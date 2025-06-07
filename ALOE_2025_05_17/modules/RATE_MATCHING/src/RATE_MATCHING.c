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

#include "RATE_MATCHING_interfaces.h"
#include "RATE_MATCHING_functions.h"
#include "RATE_MATCHING.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="RATE_MATCHING";

//Module User Defined Parameters
int rv=0;			//redundancy version (1,2,3,4)
int NBG=1;			//1 or 2
int modulation=2;	//1 if BPSK, 2 if QPSK, 4 if 16 QAM, 6 if 64 QAM, 8 if 256 QAM
int nlayers=1;		//PDSCH (1,2...8) in PUSCH (1,2,3,4)
int TBSLBRM=0;		//Always 0, con 1 no se ha programado

//Global Variables
int Tslot=0;

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("rv", &rv);
	param_get_int("NBG", &NBG);
	param_get_int("modulation", &modulation);
	param_get_int("nlayers", &nlayers);
	param_get_int("TBSLBRM", &TBSLBRM);

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      rv=%d, NBG=%d, modulation=%d, nlayers=%d, TBSLBRM=%d\n", rv, NBG, modulation, nlayers, TBSLBRM);
	printf("O--------------------------------------------------------------------------------------------O\n");

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
	int snd_samples0=0, rcv_samples0=0, rcv_samples1=0;
	input_t *input0, *input1;
	output_t *output0;
		
	input0 = in(inp,0);	//Por este llega el stream de datos (samples)
	input1 = in(inp,1);	//Se supone que solo llega un sample que es el numero de codeblocks
	output0 = out(out,0);

	/* GET THE NUMBER OF SAMPLES RECEIVED */
	rcv_samples0=get_input_samples(0);
	rcv_samples1=get_input_samples(1);

	//printf("rcv_samples=%d\n", rcv_samples0);
	
	// Check if data received
	if (rcv_samples0 == 0)return(0);
	
	//input1=2;
	//input1[0]=3;					//Activar para hacer pruebas con el módulo sin tener en cuenta la input 2
	//int num_codeblocks=input1[0];	//Como sabemos, por el input1 recibimos el numero de codeblocks con formato tipo int, recogemos este valor
	int num_codeblocks=1;	

	//int num_codeblocks=(int)input1;	//Como sabemos, por el input1 recibimos el numero de codeblocks con formato tipo int, recogemos este valor

	//printf("Número de codeblocks=%d\n", num_codeblocks);
	
	//printf("%s IN: rcv_samples=%d\n", mname, rcv_samples0);

	/* do DSP stuff here */

	/*for(int i=1; i<rcv_samples0+1; i++){
		printf("%02x", (int)(input0[i-1]&0xFF));
		if(i%80==0)printf("\n");
	}	
*/
	ratematchfunction(input0, output0, rcv_samples0, rv, NBG, modulation, nlayers, TBSLBRM, num_codeblocks);
	snd_samples0=rcv_samples0;

	if (rcv_samples0 != 0) {
		printf("Rate matching: rcv_samples=%d y snd_samples=%d\n", rcv_samples0, snd_samples0);
		//printf("Algun output=%d",output0[0]);
	}
	//
	/*printf("INPUT RATE_MATCHING\n");
	for (int i=0; i<rcv_samples0; i++) {
		printf("%d ",input0[i]);
	}

	printf("OUTPUT RATE_MATCHING\n");
	for (int i=0; i<rcv_samples0; i++) {
		printf("%c ",output0[i]);
	}*/
/*
	for(int i=1; i<rcv_samples0+1; i++){
		printf("%02x", (int)(input0[i-1]&0xFF));
		if(i%80==0)printf("\n");
	}
*/
	Tslot++;
	return snd_samples0;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


