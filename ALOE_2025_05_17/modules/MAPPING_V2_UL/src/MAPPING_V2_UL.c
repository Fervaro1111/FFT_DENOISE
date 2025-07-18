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

#include "MAPPING_V2_UL_interfaces.h"
#include "MAPPING_V2_UL_functions.h"
#include "MAPPING_V2_UL.h"

extern MAPP_t MAPPinfo; 

extern MAC_struc MACinfo;
extern Bufer_MAC MAC_buff;


//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="MAPPING_V2_UL";
int run_times=1;
int block_length=111;
char plot_modeIN[STR_LEN]="DEFAULT";
char plot_modeOUT[STR_LEN]="DEFAULT";
float samplingfreqHz=1.0;

int TX_RX=0;

//Module User Defined Parameters
float floatp=111.11;
char stringp[STR_LEN]="MY_DEFAULT_INIT";

//Global Variables
_Complex float bufferA[2048];
float bufferB[2048];

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("TX_RX", &TX_RX);		//Initialized by hand or config file


	/* Verify control parameters */
	if (block_length > get_input_max_samples()) {
		/*Include the file name and line number when printing*/
		moderror_msg("ERROR: Block length=%d > INPUT_MAX_DATA=%d\n", block_length, INPUT_MAX_DATA);
		moderror("Check your MAPPING_V2_UL_interfaces.h file\n");
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
//	int i,k;
	input_t *input0=in(inp,0);
	MAC_struc *inputMAC;
	inputMAC=in(inp,1);
	output_t *output0=out(out,0);



	MAPPinfo.nPRB=24;
	MAPPinfo.DMRS_type=1;
	MAPPinfo.Additional_DMRS_position=0;
	MAPPinfo.numFFT=14;

	int data_init_pos=0;

	int data_per_symbol[14]={0};
	int DMRS_per_symbol[14]={0};


if(TX_RX==0){//MAPP_UL

	_Complex float input_test[5000];
	for(int i=0;i<5000;i++){
		input_test[i]=((float)i)+((float)i*I);			
	}


	snd_samples=Do_Mapping(&MAPPinfo,&input_test,&data_per_symbol, output0, data_init_pos, &DMRS_per_symbol);

	

}

if(TX_RX==1){//DEMAPP_UL

	/*for(int i=0;i<rcv_samples;i++){
	printf("data=%f i=%d\n",creal(input0[i]),i);			
	}*/

	snd_samples=UNDO_Mapping(&MAPPinfo, input0, &data_per_symbol, output0, data_init_pos, &DMRS_per_symbol);



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


