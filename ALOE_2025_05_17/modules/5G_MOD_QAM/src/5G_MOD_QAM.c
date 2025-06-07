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
#include <sys/time.h>
#include <phal_sw_api.h>

//Skeleton includes
#include "print_utils.h"
#include "paramsSK15.h"
#include "skeletonSK15.h"

//Module includes
#include "5G_MOD_QAM.h"
#include "5G_MOD_QAM_params.h"
#include "5G_MOD_QAM_functions.h"


extern MAC_struc MACinfo;
extern Bufer_MAC MAC_buff;

//#define DEBUG_STANDALONE									// Uncomment that line when in StandAlone Mode

/* MODULE DEFINED PARAMETERS.*/
// Do not modify here
char mname[STR_LEN]="5G_MOD_QAM";
extern MODparams_t oParam;
unsigned long int numTS;			/* Time slot number */

//Global Variables
// Module variables: Modify according your needs
_Complex float bufferA[2048];
float bufferB[2048];
int mod_order=0;


int first_sampl_in=0;
int delay=0;
int first_MAC_in=-1;
int Tslot=0;

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

#ifndef DEBUG_STANDALONE
	sprintf(mname, "%s", GetObjectName());
#endif

	/* Get control parameters value from modulename.params file*/
	get_config_params(&oParam);

	/* Verify control parameters */
	check_config_params(&oParam);

	/* Print Module Init Parameters */
	print_itfs(IN_TYPE, OUT_TYPE);					// Print interfaces information
	print_params(mname, oParam);					// Print module parameters after configuration values

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
 */

int work(void *inp, void *out) {

	int snd_samples=0, snd_samples1=0, rcv_samples=0,a=0;

	_Complex float *input0CPLX;
	char *input0CHAR;
	float *output0FLOAT;
	_Complex float *output0CPLX;
	char *output0CHAR;
	MAC_struc *inputMAC;
	inputMAC=in(inp,1);
	//printf("MOD QAM in from MAC=%d",*input1);
	
	rcv_samples=get_input_samples(0);

	
	
	if(oParam.opMODE==MODULATE){
		input0CHAR = in(inp,0);
		rcv_samples = get_input_samples(0);
		output0CPLX = out(out,0);

		///////// Check if data received//////
		if (rcv_samples == 0){
		MAC_buff[(Tslot%(max_buffer_size))]=*inputMAC;
		//Bufer_MAC2[(Ts%(max_buffer_size-1))]=inputMAC;
		Tslot++;
		return(0);

		}	

		//////Check delay////////
	
		if (first_sampl_in==0 && rcv_samples!=0){
		delay=Tslot-1;//Checkdelay(inputMAC,max_buffer_size,Ts);
		first_sampl_in++;
		}
	
		/////////////MANAGE MAC BUFFER///////////////
		Buffer_Manager(inputMAC,Tslot,delay,&oParam);

		char aux_vector_input[2048*100];
		int rcv_samples_bytes=bits2bytes(input0CHAR, aux_vector_input, rcv_samples);

		if (rcv_samples != 0) {
			//printf("MODULATE: rcv_samples=%d\n", rcv_samples_bytes);
		}
		if(oParam.debugg)printf("MOD rcv_samples(BITS)=%d, modulation=%d\n", rcv_samples, oParam.modulation);
		
		if(oParam.modulation==M_BPSK)snd_samples = mod_BPSK(input0CHAR, rcv_samples, output0CPLX);
		if(oParam.modulation==M_4QAM)snd_samples = mod_4QAM(input0CHAR, rcv_samples, output0CPLX);
		if(oParam.modulation==M_16QAM)snd_samples = mod_16QAM(input0CHAR, rcv_samples, output0CPLX);
		if(oParam.modulation==M_64QAM)snd_samples = mod_64QAM(input0CHAR, rcv_samples, output0CPLX);
		//if(*input2==M_256QAM)snd_samples = mod_256QAM(input0CHAR, rcv_samples, output0CPLX);	
		//if(*input2==M_1024QAM)snd_samples = mod_1024QAM(input0CHAR, rcv_samples, output0CPLX);	
		
		if (rcv_samples != 0) {
			//printf("MODULATE: snd_samples=%d\n", snd_samples);
		}

		if(oParam.debugg)printf("MOD snd_samples(CPLX FLOAT)=%d\n", snd_samples);
		snd_samples=snd_samples*sizeof(_Complex float);								//Send as chars
	}
	

	 /*if(oParam.opMODE==HARD_DEMODULATE){
		//printf("Not Ready Yet");
	}*/ 
 
	if(oParam.opMODE==SOFT_DEMODULATE){
		input0CPLX = in(inp,0);
		rcv_samples = get_input_samples(0)/sizeof(_Complex float);		//Received as chars
		int i=0;
		int rcv_samples_MAC = get_input_samples(1);
		output0FLOAT = out(out,0);
//		print_array("IN_DEMOD", IN_TYPE, input0CPLX, 16, 8);

			
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

		/////////////MANAGE MAC BUFFER///////////////
		Buffer_Manager(inputMAC,Tslot,delay,&oParam);
	

		//printf("DEMODULATE: rcv_samples=%d\n", rcv_samples);

		if(oParam.debugg)printf("DEMOD rcv_samples(CPLX FLOAT)=%d, modulation=%d\n", rcv_samples, oParam.modulation);
		
		if(oParam.modulation==M_BPSK)snd_samples = soft_demod_BPSK(input0CPLX, rcv_samples, output0FLOAT);
		if(oParam.modulation==M_4QAM)snd_samples = soft_demod_4QAM(input0CPLX, rcv_samples, output0FLOAT);
		if(oParam.modulation==M_16QAM)snd_samples = soft_demod_16QAM(input0CPLX, rcv_samples, output0FLOAT);
		if(oParam.modulation==M_64QAM)snd_samples = soft_demod_64QAM(input0CPLX, rcv_samples, output0FLOAT);
		//if(*input2==M_256QAM)snd_samples = soft_demod_256QAM(input0CPLX, rcv_samples, output0FLOAT);
		//if(*input2==M_1024QAM)snd_samples = soft_demod_1024QAM(input0CPLX, rcv_samples, output0FLOAT);

	/*for (int i=0; i<snd_samples; i++) {
		printf("%f ",output0FLOAT[i]);
	}*/		


		if (rcv_samples != 0) {
			//printf("DEMODULATE: snd_samples=%d\n", snd_samples);
		}

//		print_array("OUT_DEMOD", "FLOAT", output0FLOAT, 16, 8);

		//if(oParam.debugg)printf("DEMOD snd_samples(FLOATS)=%d\n", snd_samples);
		snd_samples=snd_samples*sizeof(float);					  
		//Send as chars
	}

	if (rcv_samples != 0) {
		//printf("MOD: send_samples=%d\n", snd_samples);
	}

	set_output_samples(0, snd_samples);	
							//	set_output_samples(Output_number, number_of_samples);
	if(snd_samples > OUTPUT_MAX_SAMPLES){
		printf(BOLD T_RED"MODULE %s: Tslot=%d. ERROR!!! snd_samples=%d exceeding output buffer size=%d"RESET"\n", 
					GetObjectName(), Tslot, snd_samples, OUTPUT_MAX_SAMPLES);
		return(-1);
	}
	//

	Tslot++;
	return 1;														// Always return 1 when successful
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}




