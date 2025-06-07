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

#include "EVM_REPORT_interfaces.h"
#include "EVM_REPORT_functions.h"
#include "EVM_REPORT.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="EVM_REPORT";
//Module User Defined Parameters
int opMODE=0; 	
int debugg=0;
int Standalone=0;
print_t T;

//Global Variables
unsigned int Tslot=0;
unsigned int TslotRECV0=0, TslotRECV1=0; 
float EVM=0.0;

// Circular buffer
#define CBUFFER_SZ	128*1024
_Complex float CBuffer[CBUFFER_SZ];
buffctrl buffCtrl;

// EVM
#define EVMBUFFER_SZ	2048*16
_Complex float EVMbuffer[EVMBUFFER_SZ];

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	/* Get control parameters*/
	param_get_int("opMODE", &opMODE);		//Initialized by hand or config file
	param_get_int("debugg", &debugg);
	param_get_int("Standalone", &Standalone);

	/* Print Module Init Parameters */
	//---------------------------------------------------------------------------------------
	strcpy(mname, GetObjectName());
	sprintf(&T.text[0][0],"SPECIFIC PARAMETERS SETUP: %s\n", mname);
	sprintf(&T.text[1][0],"  Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       					NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	sprintf(&T.text[2][0],"  opMODE=%d: 0-BYPASS, 1-EVM Check&Report\n", opMODE);
	sprintf(&T.text[3][0],"  debugg=%d: 0) No debugg; 1) Debugg active\n", debugg);

	// printTEXT(noflines, textcolor, backgroundcolor, border_char)
	printTEXT(4, 221, 93, '=');
	//---------------------------------------------------------------------------------------
	//Initialize Circular Buffer
	initCbuff(&buffCtrl, CBuffer, CBUFFER_SZ, 0);

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
	int rcv_samples0 = get_input_samples(0); 
	int rcv_samples1 = get_input_samples(1); 
	int rcv_samples2 = get_input_samples(2); 
	int snd_samples0=0, snd_samples1=0, snd_samples2=0;
	input_t *input0 = inp;
	input_t *input1 = in(inp,1);;
	input_t *output0 = out;
	input_t *output1 = out(out,1);
	report_t *reportIN=in(inp,2);
	report_t *reportOUT=out(out,2);

	int flagError=0;

	// BYPASS ONLY
	if(opMODE==0){
		//printf("%s: rcv_samples0=%d, rcv_samples1=%d\n", mname, rcv_samples0, rcv_samples1);
		memcpy(output0, input0, sizeof(input_t)*rcv_samples0);
		snd_samples0=rcv_samples0;
		memcpy(output1, input1, sizeof(input_t)*rcv_samples1);
		snd_samples1=rcv_samples1;
	}

	// EVM TEST AND REPORT
	if(opMODE==1){

		// RECEIVE DATA IN0: SAVE IN CBUFFER & BYPASS TO OUT0
		// Copy internal working buffer to output Circular buffer
		if(rcv_samples0>0){
			if(TslotRECV0==0)TslotRECV0=Tslot;
			if(writeCbuff(&buffCtrl, CBuffer, input0, rcv_samples0)==-1){
				printf("%s ERROR!!! in writing C buffer\n", mname);
				return -1;
			}
			// BYPASS
			memcpy(output0, input0, sizeof(input_t)*rcv_samples0);
			snd_samples0=rcv_samples0;
		}

		// RECEIVE DATA IN1: READ CBUFFER & VME INO-IN1 & BYPASS TO OUT1
		// Read rcv_samples1 from Cbuffer
		if(rcv_samples1>0){
			if(TslotRECV1==0)TslotRECV1=Tslot;
			if(buffCtrl.occuplevel >= rcv_samples1){
				readCbuff(&buffCtrl, CBuffer, EVMbuffer, rcv_samples1);
			}else {
				printf("%s WARNING!!! rcv_samples1=%d but not enough in Cbuffer.occuplevel=%d\n", mname, rcv_samples1, buffCtrl.occuplevel);
				printf("%s PROBABLE LOSS OF SYNCHRONIZATION\n", mname, rcv_samples1, buffCtrl.occuplevel);
				flagError=1;
				//return -1;
			}
			// Compute EVM between CBuffer samples and input1 received ones
			if(flagError==0){
				// NORMALIZE MAPPING OUTPUT
				normalize_vs_var(EVMbuffer, rcv_samples1, 3.0);
				// NORMALIZE DEMAPPING INPUT
				normalize_vs_var(input1, rcv_samples1, 3.0);
				// COMPUTE EVM
				EVM=computeEVM_3GGP_LTE128(input1, EVMbuffer, rcv_samples1, 0);
			}else {
				EVM=133.0;
				flagError=0;
			}			
			// BYPASS
			memcpy(output1, input1, sizeof(input_t)*rcv_samples1);
			snd_samples1=rcv_samples1;
			if(Standalone)printf("EVM_REPORT: EVM=%3.2f\n", EVM);
		}

		// RECEIVE REPORT IN1: ADD VME, TSLOTS DELAY, TSLOT & BYPASS TO OUT2
		// UPDATE REPORT
		if(rcv_samples2>0){	

			/*printf("%s: rcv_samples2=%d, rcv_samples1=%d\n", mname, rcv_samples2, rcv_samples1);
			printf("%s: Tslot=%d, rcv_samples=%d, snd_samples=%d, UserName=%s, ctrlVAL=%d, MAC_ID=%d\n", 
				mname, Tslot, rcv_samples1, snd_samples1, &reportOUT->username[0], reportOUT->ctrl_val);
			*/	
			update_CTRL(reportIN, reportOUT, EVM, Tslot, TslotRECV1-TslotRECV0);
			snd_samples2=ceil((float)sizeof(report_t)/(float)sizeof(output_t));

			if(debugg)printf("%s: Tslot=%d, rcv_samples2=%d, snd_samples2=%d, UserName=%s, ctrlVAL=%d, MAC_ID=%d\n", 
							mname, Tslot, rcv_samples2, snd_samples2, &reportOUT->username[0], reportOUT->ctrl_val);
		}
	}

	
	Tslot++;
	set_output_samples(2, snd_samples2);
	set_output_samples(1, snd_samples1);
	return snd_samples0;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


