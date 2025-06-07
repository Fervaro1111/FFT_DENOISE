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

#include "RECONCATENATION_interfaces.h"
#include "RECONCATENATION_functions.h"
#include "RECONCATENATION.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="RECONCATENATION";
int run_times=1;
int block_length=111;
char plot_modeIN[STR_LEN]="DEFAULT";
char plot_modeOUT[STR_LEN]="DEFAULT";
float samplingfreqHz=1.0;

//Module User Defined Parameters
float floatp=111.11;
char stringp[STR_LEN]="MY_DEFAULT_INIT";
int TX_RX;


//Global Variables
_Complex float bufferA[2048];
float bufferB[2048];
char reconcatenation[2048*8*10];
float reconcatenationf[2048*8];
int num_codeblocks=0;
int codebloc_counter=0;



/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("block_length", &block_length);		//Initialized by hand or config file
	param_get_int("run_times", &run_times);			//Initialized by hand or config file
	param_get_string("plot_modeIN", &plot_modeIN[0]);	//Initialized by hand or config file
	param_get_string("plot_modeOUT", &plot_modeOUT[0]);
	param_get_float("samplingfreqHz", &samplingfreqHz);
	param_get_float("floatp", &floatp);
	param_get_string("stringp", &stringp[0]);
	param_get_int("TX_RX", &TX_RX);

	/* Verify control parameters */
	if (block_length > get_input_max_samples()) {
		/*Include the file name and line number when printing*/
		moderror_msg("ERROR: Block length=%d > INPUT_MAX_DATA=%d\n", block_length, INPUT_MAX_DATA);
		moderror("Check your RECONCATENATION_interfaces.h file\n");
		return -1;
	}
	/*Include the file name and line number when printing*/
	modinfo_msg("Parameter block_length is %d\n",block_length);	//Print message and parameter 
	modinfo("Parameter block_length \n");				//Print only message

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      block_length=%d, run_times=%d, sampligfreqHz=%3.3f\n", block_length, run_times, samplingfreqHz);
	printf("O      plot_modeIN=%s, plot_modeOUT=%s\n", plot_modeIN, plot_modeOUT);
	printf("O      floatp=%3.3f, stringp=%s\n", floatp, stringp);
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
	int snd_samples=0;
	int i,k;
	
	char *input;	
	input = in(inp,0);
	char *output;	
	output = out(out,0);
	int *input1=in(inp,1);
	
	if(get_input_samples(1)!=0){
		num_codeblocks=*input1;
	}

	
	// Check if data received
	if (rcv_samples == 0)return(0);
	// Check if data exceed maximum expected data
	if (rcv_samples > block_length) {
		/* ... */
	}
	//printf("%s IN: rcv_samples=%d\n", mname, rcv_samples);

	/* do DSP stuff here */
	
	//printf("RECONCATENATION in \n");
	//for(i=1; i<rcv_samples+1; i++){
	//	printf("%02x", (int)(input[i-1]&0xFF));
//		if(i%80==0)printf("\n");
//	}
//	printf("\n");


if (TX_RX==0){
	


	/*printf("RECONCATENATION in TX RCV_samples=%d\n",rcv_samples);
	for(i=1; i<rcv_samples+1; i++){
		//printf("%02x", (int)(output[i-1]&0xFF));
		printf("%02x", (int)(input[i-1]&0xFF));
		if(i%80==0)printf("\n");
	}
	printf("\n");
*/

	memcpy(reconcatenation+codebloc_counter*rcv_samples,input,rcv_samples);
	codebloc_counter++;
	if (codebloc_counter==num_codeblocks){
		snd_samples=rcv_samples*num_codeblocks;
		memcpy(output,reconcatenation,snd_samples);
		codebloc_counter=0;


	}else{

		snd_samples=0;
	}

	/*printf("RECONCATENATION OUTTTT TX snd_samples=%d\n",snd_samples);
	for(i=1; i<snd_samples+1; i++){
		//printf("%02x", (int)(output[i-1]&0xFF));
		printf("%02x", (int)(output[i-1]));
		if(i%80==0)printf("\n");
	}
	printf("\n");

*/

}else{

	/*printf("RECONCATENATION INNN RX rcv_samples=%d\n",rcv_samples);
	for(i=1; i<rcv_samples+1; i++){
		printf("%02x", (int)(input[i-1]&0xFF));
		if(i%80==0)printf("\n");
	}
	printf("\n");
*/
	memcpy(reconcatenation+codebloc_counter*rcv_samples,input,rcv_samples);
	codebloc_counter++;
	if (codebloc_counter==num_codeblocks){
		snd_samples=rcv_samples*num_codeblocks;
		memcpy(output,reconcatenation,snd_samples);
		codebloc_counter=0;


	}else{

		snd_samples=0;
	}

/*	printf("RECONCATENATION OUTTTT RX snd_samples=%d\n",snd_samples);
	for(i=1; i<snd_samples+1; i++){
		//printf("%02x", (int)(output[i-1]&0xFF));
		printf("%02x", (int)(output[i-1]&0xFF));
		if(i%80==0)printf("\n");
	}
	printf("\n");
*/

}

	// Indicate the number of samples at output number N
	//	set_output_samples(N, out_samples_at_N_port);
	printf("%s IN: snd_samples=%d\n", mname, snd_samples);
	// Indicate the number of samples at output 0 with return value
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


