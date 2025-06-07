/* 
 * Copyright (c) 2012.
 * This file is part of ALOE (http://flexnets.upc.edu/)
 * 
 * ALOE++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ALOE++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <math.h>
#include <fftw3.h>

#include <phal_sw_api.h>
#include "skeleton.h"
#include "params.h"

#include "DENOISE_FFT_interfaces.h"
#include "DENOISE_FFT_functions.h"
#define DBG_DEFAULT 1     
#define DBG(lvl, ...)  do{ if(dbg_level >= (lvl)){                \
                              fprintf(stderr,"[%s:%d] ",          \
                                      __func__, __LINE__);        \
                              fprintf(stderr, __VA_ARGS__);       \
                          } }while(0)

char  mname[STR_LEN] = "DENOISE_FFT";
int   run_times      = 1;
int   block_length   = 111;
float samplingfreqHz = 1.0f;

int   fftsize     = 512;
int   numffts     = 1;
int   op_mode     = 1;
int   filter_mode = 0;
float cutoff_freq = 0.25f;
float trans_bw    = 0.02f;
float threshold   = 0.05f;
int dbg_level = 0;
/*────────────────────  FFTW  ────────────────────────────────*/
#define FFTMAXSZ  8192
static fftw_complex fftin [FFTMAXSZ];
static fftw_complex fftout[FFTMAXSZ];
static fftw_plan    fftplan;



int initialize()
{
    param_get_float("samplingfreqHz", &samplingfreqHz);
    param_get_int  ("fftsize",        &fftsize);
    param_get_int  ("numffts",        &numffts);
    param_get_int  ("op_mode",        &op_mode);
    param_get_int  ("filter_mode",    &filter_mode);
    param_get_float("cutoff_freq",    &cutoff_freq);
    param_get_float("trans_bw",       &trans_bw);
    param_get_float("threshold",      &threshold);
    param_get_int  ("dbg_level",      &dbg_level);
	const char *mode_str2 = (op_mode==1)? "FFT" : "IFFT";
    block_length = fftsize * numffts;
    if (block_length > get_input_max_samples()){
        moderror_msg("DENOISE_FFT: block_length=%d > MAX_DATA=%d\n",
                     block_length, get_input_max_samples());
        return -1;
    }

    printf("O----------- DENOISE_%s  init ---------------------------------------O\n", mode_str2);
    printf("O  fftsize      = %d\n",  fftsize);
    printf("O  numffts      = %d\n",  numffts);
    printf("O  op_mode      = %d  (1:FFT  2:IFFT)\n", op_mode);
    printf("O  filter_mode  = %d  (0:No Filter 1:LPF 2:THR 3:EMD)\n", filter_mode);
    printf("O  fc/tbw/Thr   = %.3f / %.3f / %.3f\n", cutoff_freq, trans_bw, threshold);
    printf("O  dbg_level    = %d\n",  dbg_level);
    printf("O----------------------------------------------------------------------O\n");

    if      (op_mode == 1)
        fftplan = fftw_plan_dft_1d(fftsize, fftin, fftout,
                                   FFTW_FORWARD , FFTW_ESTIMATE);
    else if (op_mode == 2)
        fftplan = fftw_plan_dft_1d(fftsize, fftin, fftout,
                                   FFTW_BACKWARD, FFTW_ESTIMATE);
    else {
        printf("op_mode=%d no válido (1=DENOISE_FFT,2=DENOISE_IFFT)\n", op_mode);
        return -1;
    }
    DBG(1,"Init OK  | fftsize=%d  block_length=%d  plan=%s\n",
        fftsize, block_length, (op_mode==1)?"FORWARD":"BACKWARD");
    return 0;
}

int work(input_t *inp, output_t *out)
{
    input_t  *input  = in (inp ,0);
    output_t *output = out(out,0);
    int rcv_samples  = get_input_samples(0);
	const char *mode_str = (op_mode==1)? "FFT" : "IFFT";
    if (rcv_samples == 0){
        DBG(3,"No hay muestras — retorno temprano\n");
        return 0;
    }
    if (rcv_samples % fftsize){
        printf("ERROR: rcv_samples=%d no múltiple de fftsize=%d\n",
                     rcv_samples, fftsize);
        return -1;
    }
    numffts = rcv_samples / fftsize;
    DBG(1,"%s Bloque %d muestras → %d ventanas\n", mode_str, rcv_samples, numffts);

    for (int v = 0; v < numffts; ++v){
        int base = v * fftsize;

        /*··· copiar a fftin y FFT ···*/
        for (int n = 0; n < fftsize; ++n)
           fftin[n] = (fftw_complex) input[base+n];

        if (dbg_level>=3)
            DBG(3,"%s V%02d IN[0]=%.3f%+.3fi\n", mode_str, v, crealf(fftin[0]), cimagf(fftin[0]));

		fftw_execute(fftplan);
		/*··· aplicar filtro según filter_mode ···*/
        switch (filter_mode){
        case 0: /* No Filter */
            break;
        case 1: /* Low pass Filter */
            low_pass_filter(fftout, fftsize,
                            cutoff_freq, trans_bw,
                            dbg_level);
            break;
        case 2: /* “Threshold” */
            	amplitude_threshold(fftout, fftsize,
                                threshold,
                                dbg_level);
            break;
	    case 3: /* “EMD” */
            	emd_filter(fftout, fftsize,
                       dbg_level);
            break;
        }
		if (op_mode == 1){          /* devolver espectro filtrado           */
           for (int n = 0; n < fftsize; ++n)
                output[base+n] = (_Complex float) fftout[n];
        }	
		else{                      /* IFFT al dominio tiempo               */
           for (int n = 0; n < fftsize; ++n)
                output[base+n] = (_Complex float) fftout[n] / fftsize;
        }
    	if (dbg_level >= 3)
       	DBG(3,"%s OUT V%02d[0]=%.3f%+.3fi\n", mode_str, v, crealf(output[base]), cimagf(output[base]));
	
	}
    
	return rcv_samples;
}

/*=========================================================================*/
int stop()
/*=========================================================================*/
{
    return 0;
}



