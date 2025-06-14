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

#include "SCRAMBLING_interfaces.h"
#include "SCRAMBLING_functions.h"
#include "SCRAMBLING.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="SCRAMBLING";

//Module User Defined Parameters
int opMODE=0; 				// 0: Scrambling; 1: Descrambling
int RNTI=0;
int CELLID=0;
int TransportChannel=0; 	//0: PDSCH, 1: PMCH
int q=0;




//Global Variables

char scrambseqCHAR[MAXSEQLENGTH];
float scrambseqFLOAT[MAXSEQLENGTH];
int sequenceLength=MAXSEQLENGTH;
int Tslot=0;
int OUTmode;

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	int i, j=1;

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("opMODE", &opMODE);		//Initialized by hand or config file
	param_get_int("RNTI", &RNTI);		//Initialized by hand or config file
	param_get_int("CELLID", &CELLID);
	param_get_int("OUTmode", &OUTmode);			//Initialized by hand or config file


	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      opMODE=%d, RNTI=%d, CELLID=%d\n", opMODE, RNTI, CELLID);
	printf("O--------------------------------------------------------------------------------------------O\n");

	/* do some other initialization stuff */
	
	if(OUTmode==0){
		create_scrambling_sequence_Oriol(RNTI, CELLID, sequenceLength, scrambseqCHAR,q);
	}

	if(OUTmode==1)create_scrambling_sequence_FLOATS(RNTI, CELLID, sequenceLength, scrambseqFLOAT,q);


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
	int i;
	input_t *input;
	output_t *output0;

	output0=out(out,0);
	input=in(inp,0);

	//float *inputFLOAT = inp;
	//float *outputFLOAT = out;

	
	// Check if data received
	if (rcv_samples == 0)return(0);
	// SCRAMBLING
	if(opMODE==0){
		/*if(Tslot%20==0)printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<SCRAMBLING\n");*/
		printf("SCRAMBLING IN =%d\n",rcv_samples);
		scrambling(input, output0, rcv_samples, scrambseqCHAR);
	
	//printf("INPUT SCRAMBLING\n");

/*	for(i=1; i<rcv_samples+1; i++){
		printf("%02x", (int)(input[i-1]&0xFF));
		if(i%80==0)printf("\n");
	}
	printf("\n");
	*/
		snd_samples=rcv_samples;
	}
	// DE-SCRAMBLING
	if(opMODE==1){
		/*if(Tslot%20==0)printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>DESCRAMBLING\n");*/		printf("DESSSCRAMBLING IN =%d\n",rcv_samples);
		scrambling(input, output0, rcv_samples, scrambseqCHAR);
	/*	printf("INPUT DESCRAMBLING\n");
	for(i=1; i<rcv_samples+1; i++){
		printf("%02x", (int)(output0[i-1]&0xFF));
		if(i%80==0)printf("\n");
	}
	printf("\n");
	*/
		snd_samples=rcv_samples;
	}

	Tslot++;
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


