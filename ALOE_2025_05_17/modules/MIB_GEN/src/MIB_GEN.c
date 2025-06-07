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

#include "MIB_GEN_interfaces.h"
#include "MIB_GEN_functions.h"
#include "MIB_GEN.h"

extern bits_t BCH_payload;
extern PBCH_t BCH_info;

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="MIB_GEN";
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


int Ts=0;




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
		moderror("Check your MIB_GEN_interfaces.h file\n");
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
	BCH_info.SFN=0;



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
	output_t* output0=out(out,0);

	
	// Check if data received
	//if (rcv_samples == 0)return(0);
	//printf("%s IN: rcv_samples=%d\n", mname, rcv_samples);

	/* do DSP stuff here */




if(TX_RX==0){/////////////TX side/////////////


	//void Prepare_Payload(int pos_DMRS,int slot_num,int SCS,PBCH_t* BCH_info,int SFN,bits_t* BCH_payload);

	int First_DMRS_Pos=3;//3 contant des de 1, 2 si es comença a 0.
	int SCS=15;//KHz

	Prepare_Payload(First_DMRS_Pos,Ts,SCS,&BCH_info,BCH_info.SFN,&BCH_payload);

	//Convert to char array

	char* charpointer;
	charpointer=(char*)&BCH_payload;
	memcpy(output0,charpointer,sizeof(charpointer));
	//snd_samples=sizeof(charpointer);
	snd_samples=4;
	printf("MIB OUT:\n");
	for(int i=1; i<snd_samples+1; i++){
		printf("%02x", (int)(output0[i-1]&0xFF));
	}
	printf("\n");




	//TEST DE MOMMEnT

/*
	char out_temp[100];
	char out_temp2[100];
	BCH_interleaving(output0,&out_temp);
	*/
	/*printf("INTERLEAVED in bytes:\n");
	for(int i=1; i<4+1; i++){
		printf("%02x", (int)(out_temp[i-1]&0xFF));
	}
	printf("\n");*/

	//BCH_Deinterleaving(&out_temp,&out_temp2);


	//TEST DE MOMMEnT
	





	Ts++;
	if(Ts==10)BCH_info.SFN++;
	if(BCH_info.SFN==1024)BCH_info.SFN=0;
	if(Ts==20)Ts=0;
	
	//snd_samples=32;
	printf("%s IN: snd_samples=%d\n", mname, snd_samples);



}/////////////TX side/////////////


if(TX_RX==1){/////////////RX side/////////////

	input_t* input0=in(inp,0);

	// Check if data received
	if (rcv_samples == 0)return(0);
	printf("%s IN: rcv_samples=%d\n", mname, rcv_samples);

	for(int i=1; i<rcv_samples+1; i++){
		printf("%02x", (int)(input0[i-1]&0xFF));
	}
	printf("\n");



	//Convert to flow bits
	bits_t* BCH_payload_rcv;
	BCH_payload_rcv=(bits_t*)input0;

	int SCS;	
	Obtain_Payload(&SCS,&BCH_info,BCH_payload_rcv);
	snd_samples=0;

	printf("SCS=%d\n",SCS);
	printf("DMRS_position=%d\n",BCH_info.DMRS_position);
	printf("SFN=%d\n",BCH_info.SFN);
}


/////////////RX side/////////////


	// Indicate the number of samples at output number N
	//	set_output_samples(N, out_samples_at_N_port);
	//	printf("%s IN: snd_samples=%d\n", mname, snd_samples);
	// Indicate the number of samples at output 0 with return value
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


