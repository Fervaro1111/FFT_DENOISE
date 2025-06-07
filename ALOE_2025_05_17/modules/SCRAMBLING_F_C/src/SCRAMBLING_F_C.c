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

#include "SCRAMBLING_F_C_interfaces.h"
#include "SCRAMBLING_F_C_functions.h"
#include "SCRAMBLING_F_C.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="SCRAMBLING";

//Module User Defined Parameters
int opMODE=0; 				// 0: Scrambling; 1: Descrambling
int DL_UL=0;				//0: DL,  1: UL
int RNTI=0;
int CELLID=0;
int q=0;
int Data_Control=0;			//0 is data shared channel; 1 is control shared channel



//Global Variables


extern MAC_struc MACinfo;
extern SCRinfo_t SCR_info;
extern Bufer_MAC MAC_buff;




char scrambseqCHAR[MAXSEQLENGTH];
float scrambseqFLOAT[MAXSEQLENGTH];
int sequenceLength=MAXSEQLENGTH;

int Tslot=0;
int first_sampl_in=0;
int delay;
int first_MAC_in=-1;


/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	int i, j=1;

	/* Get control parameters*/
	param_get_int("opMODE", &opMODE);		//Initialized by hand or config file
	param_get_int("DL_UL", &DL_UL);		//Initialized by hand or config file
	param_get_int("Data_Control", &Data_Control);		//Initialized by hand or config file



	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      opMODE=%d\n",opMODE);
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
	
	char *inputCHAR = in(inp,0);
	char *outputCHAR = out;
	float *inputFLOAT = in(inp,0);
	float *outputFLOAT = out;
	MAC_struc *inputMAC=in(inp,1);
	int rcv_samples_MAC = get_input_samples(1);
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


	Buffer_Manager(&SCR_info,inputMAC,Tslot,delay);
	//printf("IN %s, CELLID=%d",mname,SCR_info.CELLID);

	
		
	// SCRAMBLING PDSCH
	if(opMODE==0 && DL_UL==0 && Data_Control==0){

	//printf("SCRAMBLING FROM MAC = %d\n",inputMAC->Ts_MAC);


	create_scrambling_sequence_BITS(SCR_info.RNTI, SCR_info.CELLID, sequenceLength, scrambseqCHAR,SCR_info.q);


		scrambling(inputCHAR, outputCHAR, rcv_samples, scrambseqCHAR);
		snd_samples=rcv_samples;
		if (rcv_samples != 0) {
			//printf("Scrambling: rcv_samples=%d y snd_samples=%d\n", rcv_samples, snd_samples);
		}
	}

	// DE-SCRAMBLING PDSCH
	if(opMODE==1 && DL_UL==0 && Data_Control==0){

		create_scrambling_sequence_FLOATS(SCR_info.RNTI, SCR_info.CELLID, sequenceLength, scrambseqFLOAT,SCR_info.q);

		descrambling(inputFLOAT, outputFLOAT, rcv_samples/sizeof(float), scrambseqFLOAT);
	
		snd_samples=rcv_samples;
		if (rcv_samples != 0) {
			//printf("Descrambling: rcv_samples=%d y snd_samples=%d\n", rcv_samples, snd_samples);

		}
	}


	// SCRAMBLING PDCCH
	if(opMODE==0 && DL_UL==0 && Data_Control==1){

		create_scrambling_sequence_BITS_PDCCH(SCR_info.CELLID, sequenceLength, scrambseqCHAR,SCR_info.RNTI);

		scrambling(inputCHAR, outputCHAR, rcv_samples, scrambseqCHAR);
		snd_samples=rcv_samples;
		if (rcv_samples != 0) {
			//printf("Scrambling: rcv_samples=%d y snd_samples=%d\n", rcv_samples, snd_samples);
		}
	}


	// DE-SCRAMBLING PDCCH
	if(opMODE==1 && DL_UL==0 && Data_Control==1){
		
	//printf("IN %s, CELLID=%d",mname,SCR_info.CELLID);


		create_scrambling_sequence_FLOATS_PDCCH(SCR_info.CELLID, sequenceLength, scrambseqFLOAT,SCR_info.RNTI);

		descrambling(inputFLOAT, outputFLOAT, rcv_samples/sizeof(float), scrambseqFLOAT);
	
		snd_samples=rcv_samples;
		if (rcv_samples != 0) {
			//printf("Descrambling: rcv_samples=%d y snd_samples=%d\n", rcv_samples, snd_samples);

		}
	}



	/*for(i=1; i<snd_samples+1; i++){
		printf("%f", (outputFLOAT[i-1]));
		//if(i%80==0)printf("\n");
	}
	printf("\n");
	*/
	
	Tslot++;
	return snd_samples;
}
//
/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


