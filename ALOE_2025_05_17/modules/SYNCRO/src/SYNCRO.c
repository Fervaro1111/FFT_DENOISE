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

#include "SYNCRO_interfaces.h"
#include "SYNCRO_functions.h"
#include "SYNCRO.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="SYNCRO";

extern Syncro_t Syncinfo;
extern SYNC_struc Sync_MAC;
int TSLOT=-1;
_Complex float buf[1024*15*100];
int already_sent_NIDs=0;
int NID1=-1;
int NID2=-1;
int FFTsize;

//Module User Defined Parameters


//Global Variables


/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {


	/* Get control parameters*/


	/* Verify control parameters */
	
	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O--------------------------------------------------------------------------------------------O\n");

	/* do some other initialization stuff */

	




//	init_functionB_FLOAT(bufferB, 1024);

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
int work(input_t *inp, output_t *out) {
	int rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
	int snd_samples=0;
	output_t *output0 = out(out,0);
	output_t *output1 = out(out,1);
	output_t *graph = out(out,2);
	output_t *output3 = out(out,3);	
	input_t *input0 = in(inp,0);
	int *input1 = in(inp,1);
//	int i,k;
	_Complex float tmp[4096*15];
	_Complex float fake[4096*15];

	int snd2graph=0;

	TSLOT++;

	// Check if data received
	if (rcv_samples == 0)return(0);
	
	printf("%s IN: rcv_samples=%d\n", mname, rcv_samples);
	FFTsize=*(input1);
	//printf("FFTSIZE IN SYNCRO=%d",*(input1));

	

	// SYNCHRONISE
	snd_samples=do_sycro(input0,rcv_samples,output0,
				&NID1, &NID2, FFTsize, graph, &snd2graph);


	if(NID1!=-1 && NID2!=-1){
		if(already_sent_NIDs==0){
			int CELLID=3*NID1+NID2;
			Sync_MAC.CELLID=CELLID;
			Sync_MAC.NID1=NID1;
			Sync_MAC.NID2=NID2;
			memcpy(output1,&Sync_MAC,1*sizeof(Sync_MAC));
			printf("CELLID=%d\n",Sync_MAC.CELLID);
			
			set_output_samples(1, sizeof(Sync_MAC));
			already_sent_NIDs++;
		}

	}


	// Indicate the number of samples at output number N
		set_output_samples(2, snd2graph);
	printf("%s OUT: snd_samples=%d\n", mname, snd_samples);
	// Indicate the number of samples at output 0 with return value

	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


