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

#include <complex.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <phal_sw_api.h>
#include "skeleton.h"
#include "params.h"

#include "WIMAX_DEC_ALOE16_interfaces.h"
#include "WIMAX_DEC_ALOE16_functions.h"
#include "WIMAX_DEC_ALOE16.h"
#include "dec_LDPC.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="WIMAX_DEC_ALOE16";
char pchk_fileH[100];
char gen_fileH[100];
float coderate = 3.0;
extern int max_iter=3;
char info_out_chars[INPUT_MAX_DATA];
char outAUX[INPUT_MAX_DATA];

int input[INPUT_MAX_DATA];
int output[INPUT_MAX_DATA];

extern char *pchk_file, *gen_file;
extern int *cols;


/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	//printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	//pchk_file = "/home/antoni//DATA0/DeNOU_DADES/DEVELOPMENT/ALOE/ALOE_DOCENCIA/ALOE-1.6_DOCENCIA_WORKING_MAY2020/APPs/WAVEForms/WIMAXLDPC_TEST/data/802_16e.pchk";
	pchk_file = "data/802_16e.pchk";

    //gen_file = "/home/antoni//DATA0/DeNOU_DADES/DEVELOPMENT/ALOE/ALOE_DOCENCIA/ALOE-1.6_DOCENCIA_WORKING_MAY2020/APPs/WAVEForms/WIMAXLDPC_TEST/data/802_16e.gen";
    gen_file = "data/802_16e.gen";

	/* Get control parameters*/
/*	param_get_string("pchk_file", &pchk_file[0]);	
	param_get_string("gen_fileH", &gen_fileH[0]);*/
//	param_get_float("coderate", &coderate);
	param_get_int("max_iter", &max_iter);



	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      coderate=%f\n", coderate);
	printf("O      max_iter=%i\n", max_iter);
	printf("O      path pchk_file=%s\n", pchk_file);
	printf("O      path gen_file=%s\n", gen_file);
	printf("O--------------------------------------------------------------------------------------------O\n");


    Init_DeLDPC();
    printf("%s: INIT_DeLDPC: data/802_16e.pchk OK\n", GetObjectName());
	printf("%s: IF ERROR CHECK DATA FILE REPOSITORY\n", GetObjectName());

	return 0;
}



/**
 * @brief Function documentation
 *
 * @param inp Input interface buffers. Data from other interfaces is stacked in the buffer.
 * Use in(ptr,idx) to access the address. To obtain the number of received samples use the function
 * int get_input_samples(int idx) where idx is the interface index.
 *
 * @param out Input interface buffers. Data to other interfaces must be stacked in the buffer.
 * Use out(ptr,idx) to access the address.
 *
 * @return On success, returns a non-negative number indicating the output
 * samples that should be transmitted through all output interface. To specify a different length
 * for certain interface, use the function set_output_samples(int idx, int len)
 * On error returns -1.
 *
 * @code
 * 	input_t *first_interface = inp;
	input_t *second_interface = in(inp,1);
	output_t *first_output_interface = out;
	output_t *second_output_interface = out(out,1);
 *
 */
int work(input_t *inp, output_t *out) {
	int rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
	//printf("LDPC DECODER in rcv_samples=%d\n",rcv_samples);
	int snd_samples=0;
//	int i,k;
	
	// Check if data received
	if (rcv_samples == 0)return(0);

	for(i=0; i<rcv_samples; i++){
		*(inp+i) = -*(inp+i);
		/*if(*(inp+i)>=0.0)*(inp+i)=-1.0;
		else *(inp+i)=1.0;*/
	}
//	printf("\n%s rcv_samples = %d\n", mname, rcv_samples);
//	for(i=0; i<32; i++)printf("%1.1f ", *(inp+i));

   /** Decode word Input_i to output_i*/
 	//for(i=0; i<rcv_samples; i++)*(inp+i)=(float)(1-(2**(inp+i)));
    run_decode_LDPC(inp,info_out_chars);

    /** Only Relevant Data out*/

/*	printf("      \n\n%s A snd_samples = %d, N=%d\n", mname, snd_samples, N);
	for(i=0; i<1152*3; i++){
		printf("%x ", *(info_out_chars+i));
		if((i+1)%64==0)printf("\n");
	}

	  for(i=1152; i<N; i++){
        if (info_out_chars[cols[i]] == 0) outAUX[i-1152]=0;
		else outAUX[i-1152]=1;
    }
*/

	snd_samples=((int)(((float)rcv_samples)/coderate));
   	for(i=1152; i<N; i++){
        if (info_out_chars[cols[i]] == 0) outAUX[i-1152]=0;
		else outAUX[i-1152]=1;
    }


	
/*	printf("      \n\n%s A snd_samples = %d, N=%d\n", mname, snd_samples, N);
	for(i=0; i<snd_samples; i++){
		printf("%x ", *(outAUX+i));
		if((i+1)%64==0)printf("\n");
	}
*/
	char2bin (outAUX, out, snd_samples);
	snd_samples=snd_samples/8;
	//printf("LDPC DECODER in snd_samples=%d\n",snd_samples);

	/*printf("\n%s B snd_samples = %d, N=%d\n", mname, snd_samples, N);
	for(i=0; i<16; i++)printf("%x ", *(out+i));
*/
   // snd_samples=((int)(((float)rcv_samples)/coderate))/8;
	//bin2char (outAUX, out, snd_samples);




//	printf("%s snd_samples = %d\n", mname, snd_samples);
	// Indicate the number of samples at output 0 with return value
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


