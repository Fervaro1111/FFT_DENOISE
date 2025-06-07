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
#include <complex.h>
#include <math.h>
#include <string.h>
#include <fftw3.h>

#include <phal_sw_api.h>
#include "skeleton.h"
#include "params.h"

#include "FFT_IFFT_interfaces.h"
#include "FFT_IFFT_functions.h"
#include "FFT_IFFT.h"

#ifndef DBG_DEFAULT       
  #define DBG_DEFAULT
#endif

#define DBG(lvl, ...)                                                           \
    do{ if(dbg_level >= (lvl)){                                                 \
            fprintf(stderr,"[%s:%d] ", __func__, __LINE__);                     \
            fprintf(stderr, __VA_ARGS__);                                       \
        } }while(0)


char  mname[STR_LEN]       = "FFT_IFFT";
int   run_times            = 1;
int   block_length         = 111;
char  plot_modeIN[STR_LEN] = "DEFAULT";
char  plot_modeOUT[STR_LEN]= "DEFAULT";
float samplingfreqHz       = 1.0f;

int fftsize = 128;
int numffts = 1;
int op_mode = 1;
int dbg_level = 1;
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
	param_get_int  ("dbg_level",        &dbg_level);
    block_length = fftsize * numffts;

    if (block_length > get_input_max_samples()) {
        printf("ERROR: block_length=%d > INPUT_MAX_DATA=%d\n",
                     block_length, INPUT_MAX_DATA);
        return -1;
    }

    strcpy(mname, GetObjectName());
    printf("O---------------------------------------------------------------------------O\n");
    printf("O  Módulo \033[34m%s\033[0m  — FFT/IFFT genérico\n", mname);
	printf("O    dbg_level=%d (0=off,3=full)\n", dbg_level);
    printf("O    fftsize=%d | numffts=%d | op_mode=%d (%s)\n",
           fftsize, numffts, op_mode, (op_mode==1)?"FFT":"IFFT");
    printf("O---------------------------------------------------------------------------O\n");

    /* 4. — Crear plan FFTW (costoso ⇒ se hace una sola vez) */
    if      (op_mode == 1)
        fftplan = fftw_plan_dft_1d(fftsize, fftin, fftout,
                                   FFTW_FORWARD , FFTW_ESTIMATE);
    else if (op_mode == 2)
        fftplan = fftw_plan_dft_1d(fftsize, fftin, fftout,
                                   FFTW_BACKWARD, FFTW_ESTIMATE);
    else {
        printf("op_mode=%d no válido (1=FFT,2=IFFT)\n", op_mode);
        return -1;
    }

    DBG(1,"Init OK  | fftsize=%d  block_length=%d  plan=%s\n",
        fftsize, block_length, (op_mode==1)?"FORWARD":"BACKWARD");
    return 0;
}

int work(input_t *inp, output_t *out)
{
    input_t  *input  = in (inp ,0);     /* alias puntero entrada 0 */
    output_t *output = out(out,0);      /* alias puntero salida  0 */
    int rcv_samples  = get_input_samples(0);  /* cuántas muestras llegan */
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
    DBG(2,"%s Bloque con %d muestras → %d ventanas FFT\n", mode_str, rcv_samples, numffts);

    for (int v=0; v<numffts; ++v){
        int base = v*fftsize;

        for (int n=0; n<fftsize; ++n)
            fftin[n] = (fftw_complex) input[base+n];

        if (dbg_level>=3)
            DBG(3,"%s V%02d IN[0]=%.3f%+.3fi\n", mode_str,
                v, crealf(fftin[0]), cimagf(fftin[0]));

        fftw_execute(fftplan);

        if (op_mode == 1) {
            for (int n=0; n<fftsize; ++n)
                output[base+n] = (_Complex float) fftout[n];
        } else {
            for (int n=0; n<fftsize; ++n)
                output[base+n] = (_Complex float)(fftout[n])/fftsize;
        }

        if (dbg_level>=3)
            DBG(2,"%s V%02d OUT[0]=%.3f%+.3fi\n", mode_str,
                v, crealf(output[base]), cimagf(output[base]));
    }

    return rcv_samples;
}

int stop()
{
    fftw_destroy_plan(fftplan);
    DBG(1,"FFT_IFFT detenido y plan liberado\n");
    return 0;
}



