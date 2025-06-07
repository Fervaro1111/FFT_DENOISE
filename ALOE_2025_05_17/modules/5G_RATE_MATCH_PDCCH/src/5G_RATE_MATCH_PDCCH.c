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

#include "5G_RATE_MATCH_PDCCH_interfaces.h"
#include "5G_RATE_MATCH_PDCCH_functions.h"
#include "5G_RATE_MATCH_PDCCH.h"

extern RM_t RM_info;
extern MAC_struc MAC_info;
extern Bufer_MAC MAC_buff;

//ALOE Module Defined Parameters. Do not delete.
int TX_RX_mode=0;

//Module User Defined Parameters
float floatp=111.11;
char stringp[STR_LEN]="MY_DEFAULT_INIT";
char mname[STR_LEN]="5G_RATE_MATCH_PDCCH";

//Global Variables
_Complex float bufferA[2048];
float bufferB[2048];

int Tslot=0;
int delay=0;
int first_sampl_in=0;


/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/

	param_get_int("TX_RX_mode", &TX_RX_mode);		//Initialized by hand or config file
	//param_get_int("run_times", &run_times);			//Initialized by hand or config file

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
	int snd_samples=0;

	MAC_struc *inputMAC=in(inp,1);

	

// Check if data received
	//if (rcv_samples == 0)return(0);



	/* do DSP stuff here */

	//RM_info.Carr_agr_lvl=0;
	RM_info.K=88;


	///////// Check if data received//////
	if (rcv_samples == 0){
		MAC_buff[(Tslot%(max_buffer_size))]=*inputMAC;
		//Bufer_MAC2[(Ts%(max_buffer_size-1))]=inputMAC;
		Tslot++;
		return(0);

	}	

	//////Check delay////////
	
	if (first_sampl_in==0 && rcv_samples!=0){
		delay=0;//Checkdelay(inputMAC,max_buffer_size,Ts);
		first_sampl_in++;
	}
	//printf("-----------%s-----------Ts=%d,delay=%d------------------\n",mname,Tslot,delay);

	//int RNTI=-100;
	Buffer_Manager(&RM_info.Carr_agr_lvl,&RM_info.Control_symb_number,inputMAC,Tslot,delay);
	//printf("Agr_lvl_DCI at %s=%d Coreset symbol syze=%d\n",mname,RM_info.Carr_agr_lvl,RM_info.Control_symb_number);



if(TX_RX_mode==0){//TX	
	unsigned char *input0, *output0;
	input0 = in(inp,0);
	output0 = out(out,0);

	//RM_info.Carr_agr_lvl=1;
	//RM_info.K=88;
	RM_info.N=rcv_samples;

	//printf("%s IN: rcv_samples=%d\n", mname, rcv_samples);

	do_Rate_Match(input0, output0,&RM_info);

	snd_samples=RM_info.E;
	//printf("SND samples at RM PDCCH=%i\n",snd_samples);

}


if(TX_RX_mode==1){//RX		
	float *input0, *output0;
	input0 = in(inp,0);
	output0 = out(out,0);
	rcv_samples=rcv_samples/sizeof(float);
	//printf("%s IN: rcv_samples=%d\n", mname, rcv_samples);
		
	
	undo_Rate_Match(input0, output0,&RM_info);
	//printf("DRM snd_samp=%d\n",RM_info.N);
	snd_samples=RM_info.N*sizeof(float);
	//printf("float sizes=%d\n",sizeof(float));

	//printf("SND samples at DERM=%i\n",snd_samples);
	

}


	Tslot++;
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


