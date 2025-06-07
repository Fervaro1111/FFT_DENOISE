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

#include "MAPPING_V2_interfaces.h"
#include "MAPPING_V2_functions.h"
#include "MAPPING_V2.h"


extern Sequences_t SequencesParams;
extern MAPP_t MAPPinfo;
extern MAC_struc MACinfo;
extern Bufer_MAC MAC_buff;


//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="MAPPING";
int run_times=1;
int block_length=111;
char plot_modeIN[STR_LEN]="DEFAULT";
char plot_modeOUT[STR_LEN]="DEFAULT";
float samplingfreqHz=1.0;

//Module User Defined Parameters
float floatp=111.11;
char stringp[STR_LEN]="MY_DEFAULT_INIT";

//Global Variables
_Complex float bufferA[2048];
float bufferB[2048];
int Tslot=0;
int subframe_num=0;
int first_sampl_in=0;
int delay;
int first_MAC_in=-1;
int TX_RX;

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {


	/* Get control parameters*/
	
	param_get_int("TX_RX", &TX_RX);	


	/* Verify control parameters */
	if (block_length > get_input_max_samples()) {
		/*Include the file name and line number when printing*/
		moderror_msg("ERROR: Block length=%d > INPUT_MAX_DATA=%d\n", block_length, INPUT_MAX_DATA);
		moderror("Check your MAPPING_interfaces.h file\n");
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
	printf("O     TX_RX=%d\n",TX_RX);
	printf("O      plot_modeIN=%s, plot_modeOUT=%s\n", plot_modeIN, plot_modeOUT);
	
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
	output_t *outputControlchann=out(out,1);
	input_t *inputCONTROLchann=in(inp,2);
	int rcv_samples_controlchann = get_input_samples(2);
	int rcv_samples_MAC = get_input_samples(1);
	 //Check if data received
	// if (rcv_samples == 0)return(0);
	


	//printf("%s IN: rcv_samples=%d\n", mname, rcv_samples);
	
	///////// Check if data received//////
	if (rcv_samples == 0){
	MAC_buff[(Tslot%(max_buffer_size))]=*inputMAC;
	if(first_MAC_in<0 && rcv_samples_MAC!=0) first_MAC_in=Tslot;
	Tslot++;
	return(0);

	}	

	//printf("%s IN: rcv_samples=%d\n", mname, rcv_samples);

	//////Check delay////////

	if (first_sampl_in==0 && rcv_samples!=0){
	delay=Tslot-first_MAC_in;//Checkdelay(inputMAC,max_buffer_size,Ts);
	first_sampl_in++;
	}


	Buffer_Manager(&MAPPinfo,inputMAC,Tslot,delay);


	/* do DSP stuff here */
	

	MAPPinfo.numFFT=14;
	
	int data_per_symbol[14]={0};
	int DMRS_per_symbol[14]={0};

	int data_init_pos=MAPPinfo.offset_data;
	//printf("DATA INIT POS IN MAPPPPING=%d\n",data_init_pos);

	//Aqui fins que es legeixi el MAC buffer
	int SSblock_flag=0;

	if(subframe_num==0 || subframe_num==1){
		SSblock_flag=1;
	}else{
		SSblock_flag=0;
	}	
	
	
	if(TX_RX==0){

		// MAKE INPUT RAMP
		/*_Complex float tmp[2048*14];
		for (int i=0;i<rcv_samples;i++){
			tmp[i]=((float)i)+(float)i*I;
		}*/

		// NO DATA AT INPUT
		//for (int i=0;i<rcv_samples;i++)*(input0+i)=0.0+0.0*I;
		//for (int i=0;i<rcv_samples_controlchann;i++)*(inputCONTROLchann+i)=0.0+0.0*I;

		// printf("%s IN: rcv_samples=%d\n", mname, rcv_samples);
		snd_samples=Do_Mapping(&MAPPinfo, input0, &data_per_symbol, SSblock_flag, output0, data_init_pos, 
								&DMRS_per_symbol, rcv_samples_controlchann, inputCONTROLchann);
		//snd_samples=Do_Mapping(&MAPPinfo,&tmp[0],&data_per_symbol,SSblock_flag,output0,data_init_pos,&DMRS_per_symbol);
		
		
	
	}else{
		//printf("%s IN: rcv_samples=%d\n", mname, rcv_samples);
		int snd_ctrl_samples=0;		
		snd_samples=UNDO_Mapping(&MAPPinfo, input0, &data_per_symbol, SSblock_flag, output0, data_init_pos, 
									&DMRS_per_symbol, outputControlchann, &snd_ctrl_samples);
		set_output_samples(1, snd_ctrl_samples);
		
		/*for(int i=0;i<snd_ctrl_samples;i++){	
		printf("outdemap control=%f at i=%d\n",creal(outputControlchann[i]),i);
		}*/

	}
	
	///////////////////////////////////////////////////////////


	//printf("Tslot=%d\n",Tslot);
	Tslot++;
	subframe_num++;
	if(subframe_num==20)subframe_num=0;
	
	//snd_samples=128;

	// Indicate the number of samples at output number N
	//	set_output_samples(N, out_samples_at_N_port);
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


