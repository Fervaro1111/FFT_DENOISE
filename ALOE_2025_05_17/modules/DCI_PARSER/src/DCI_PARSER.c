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

#include "DCI_PARSER_interfaces.h"
#include "DCI_PARSER_functions.h"
#include "DCI_PARSER.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="DCI_PARSER";
int run_times=1;
int block_length=111;
char plot_modeIN[STR_LEN]="DEFAULT";
char plot_modeOUT[STR_LEN]="DEFAULT";
float samplingfreqHz=1.0;
int opMode;

//Module User Defined Parameters
float floatp=111.11;
char stringp[STR_LEN]="MY_DEFAULT_INIT";

//Global Variables
_Complex float bufferA[2048];
float bufferB[2048];

extern bits_t DCI;
extern DCIinfo_t DCI_info;





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
	param_get_int("opMode", &opMode);			
	/* Verify control parameters */
	if (block_length > get_input_max_samples()) {
		/*Include the file name and line number when printing*/
		moderror_msg("ERROR: Block length=%d > INPUT_MAX_DATA=%d\n", block_length, INPUT_MAX_DATA);
		moderror("Check your DCI_PARSER_interfaces.h file\n");
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

	/* do DSP stuff here */

	DCI_info.N_size_BWP=50;

////////////////////////////////TX/////////////////////
if(opMode==0){
	output_t *output0;
	output0=out(out,0);

//Initialize params
	DCI_info.Lrbs=40;
	DCI_info.RBstart=0;

	DCI_info.time_domain_allocation=11;//S=1;L=13 from table 5.1.2.1.1-2 from 38.214
	DCI_info.rv=0;
	DCI_info.MCS_index=8;


//Do DCI
	do_DCI(&DCI_info,&DCI);

//Convert to char array

	char* charpointer;
	charpointer=(char*)&DCI;
	memcpy(output0,charpointer,sizeof(charpointer));
	snd_samples=sizeof(charpointer);

}
////////////////////////////////////  TX  //////////////////////////



////////////////////////////////// RX  //////////////////////////
if(opMode==1){
	input_t *input0;
	input0 = in(inp,0);
	rcv_samples = get_input_samples(0);
	if(rcv_samples==0)return 0;


//Convert to flow bits
	bits_t* DCI_rcv_bits;
	DCI_rcv_bits=(bits_t*)input0;
//GET DCI DATA
	Undo_DCI(&DCI_info,DCI_rcv_bits);


	printf("Lrbs=%d\n",DCI_info.Lrbs);
	printf("RBstart=%d\n",DCI_info.RBstart);
	printf("time_domain_allocation=%d\n",DCI_info.time_domain_allocation);
	printf("MCS_index=%d\n",DCI_info.MCS_index);
	printf("rv=%d\n",DCI_info.rv);

}
////////////////////////////////// RX  //////////////////////////





	//snd_samples=rcv_samples;

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


