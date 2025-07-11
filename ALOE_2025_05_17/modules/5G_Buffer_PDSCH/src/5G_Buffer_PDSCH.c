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

#include "5G_Buffer_PDSCH_interfaces.h"
#include "5G_Buffer_PDSCH_functions.h"
#include "5G_Buffer_PDSCH.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="5G_Buffer_PDSCH";
int opMode=0;


//Global Variables
_Complex float buffer_PDSCH[Buffer_Size];
int buffer_rcv_samp[rcv_samp_Buffer_size];
int output_flag=0; //Mac flag to begin outputing data.



/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("opMode", &opMode);

	
	/*Include the file name and line number when printing*/

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	
	printf("O--------------------------------------------------------------------------------------------O\n");

	/* do some other initialization stuff */

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
	//int rcv_samples_flag = get_input_samples(1); 
	int snd_samples=0;
//	int i,k;
	output_t *output0;
	input_t *input_PDSCH;
	int *in_flag;

	input_PDSCH = in(inp,0);
	in_flag = in(inp,1);
	output0 = out(out,0);

	//printf("RX MAC flag to buffer=%d\n",*(in_flag));

	////////////////////////////////////////////////////////
	////////////////////Buffer mode ////////////////////////


if(opMode==1){

	// Check if data received
	if (rcv_samples == 0)return(0);
	if(*(in_flag)!=0) output_flag=*(in_flag);	



	///////// Store received data  //////

	Add_to_buffer(input_PDSCH,&buffer_PDSCH,rcv_samples);
	Add_to_buffer_rcv_sampl(rcv_samples,&buffer_rcv_samp);

	//////Check delay////////
	

	if (output_flag!=0){
		snd_samples=GET_from_buffer_rcv_sampl(&buffer_rcv_samp);
		GET_from_buffer(output0,&buffer_PDSCH,snd_samples);
	}


}


	////////////////////Buffer mode ////////////////////////
	////////////////////////////////////////////////////////




	////////////////////////////////////////////////////////
	////////////////////Bypass mode ////////////////////////
	if(opMode==0){
		if (rcv_samples == 0)return(0);
		snd_samples=rcv_samples;
		memcpy(output0,input_PDSCH,snd_samples*sizeof(_Complex float));
		/*for(int i=0;i<snd_samples;i++){	
			printf("out=%f at i=%d\n",creal(output0[i]),i);
		}*/
	
	}	

	////////////////////Bypass mode ////////////////////////
	////////////////////////////////////////////////////////



	//printf("%s IN: snd_samples=%d\n", mname, snd_samples);
	// Indicate the number of samples at output 0 with return value
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


