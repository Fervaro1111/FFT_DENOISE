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

#include "eLDPC_code.h"

#include "WIMAX_ENC_ALOE16_interfaces.h"
#include "WIMAX_ENC_ALOE16_functions.h"
#include "WIMAX_ENC_ALOE16.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="WIMAX_ENC_ALOE16";
#define BSIZE	100*1024
extern char *pchk_file, *gen_file;

char pchk_fileH[100];
char gen_fileH[100];
float coderate = 3.0;
int inputINT[INPUT_MAX_DATA], outputINT[INPUT_MAX_DATA];
char inputBIT[INPUT_MAX_DATA];
/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	//pchk_file = "/home/antoni//DATA0/DeNOU_DADES/DEVELOPMENT/ALOE/ALOE_DOCENCIA/ALOE-1.6_DOCENCIA_WORKING_MAY2020/APPs/WAVEForms/WIMAXLDPC_TEST/data/802_16e.pchk";
	pchk_file = "data/802_16e.pchk";

    //gen_file = "/home/antoni//DATA0/DeNOU_DADES/DEVELOPMENT/ALOE/ALOE_DOCENCIA/ALOE-1.6_DOCENCIA_WORKING_MAY2020/APPs/WAVEForms/WIMAXLDPC_TEST/data/802_16e.gen";

    gen_file = "data/802_16e.gen";

	/* Get control parameters*/
/*	param_get_string("pchk_file", &pchk_file[0]);	
	param_get_string("gen_fileH", &gen_fileH[0]);*/
//	param_get_float("coderate", &coderate);


	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      coderate=%f\n", coderate);
	printf("O      path pchk_file=%s\n", pchk_file);
	printf("O      path gen_fileH=%s\n", gen_file);
	printf("O--------------------------------------------------------------------------------------------O\n");


    Init_eLDPC();
    printf("%s: INIT_eLDPC: data/802_16e.pchk OK\n", GetObjectName());
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
	int snd_samples=0;
	int i,k;
	//printf("INIT LDPC ENCODING rcv_samples=%d\n",rcv_samples);
	

	// Check if data received
	if (rcv_samples == 0)return(0);

/*	printf("A %s rcv_samples = %d\n", mname, rcv_samples);
	for(i=0; i<rcv_samples; i++){
		printf("%x ", *(inp+i));
		if((i+1)%64==0)printf("\n");
	}
	printf("\n");
*/
	bin2char (inp, inputBIT, rcv_samples*8);
	rcv_samples=rcv_samples*8;

/*	printf("A %s rcv_samples = %d\n", mname, rcv_samples);
	for(i=0; i<rcv_samples; i++){
		printf("%x ", *(inputBIT+i));
		if((i+1)%64==0)printf("\n");
		//if(i>800)*(inputBIT+i)=0x0;
	}
*/
    for(i=0; i<rcv_samples; i++){
        inputINT[i]=(int)inputBIT[i];
		//printf("%x\n", input[i]);
    }

    //N=2304;
    Run_EncodeeLDPC (inputINT, outputINT);
    
  	snd_samples=(int)((float)rcv_samples*coderate);
    for(i=0; i<snd_samples; i++)out[i]=(char)outputINT[i];
/*	printf("\nOUT %s snd_samples = %d\n", mname, snd_samples);
	for(i=0; i<16; i++)printf("%x ", *(out+i));
*/
	//printf("END LDPC ENCODING snd_samples=%d\n",snd_samples);
	// Indicate the number of samples at output 0 with return value
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


