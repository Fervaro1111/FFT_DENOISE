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
#include <string.h>
#include <phal_sw_api.h>
#include "skeleton.h"
#include "params.h"

#include "5G_SYNCHRO_interfaces.h"
#include "5G_SYNCHRO_functions.h"
#include "5G_SYNCHRO.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="5G_SYNCHRO";
int TSLOT=0;


//Module User Defined Parameters
const int FFTsize=FFTSIZE;

//Global Variables
// DMRS sequence
#define BUFFER_SZ	2048
_Complex float DMRS_SEQ[BUFFER_SZ];
_Complex float DMRS_TX_TIME[BUFFER_SZ];
  DMRS_RX_TIME[BUFFER_SZ];
int DMRS_length=0;
int M_RS_SC = (NUMRB-2)*RBCARRIERS;



//Global Variables
//PSS
extern _Complex float PSS_time[0];
extern Syncro_t SynchroINFO;
extern SYNC_struc Sync_MAC;
_Complex float *pPSS0, *pPSS1, *pPSS2;
int NID1=-1, NID2=-1; //Default value while not detected NIDs

// CORRELATION process
#define BUFFERSIZE	64*1024
_Complex float CorrResult[BUFFERSIZE];




/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {
	int i, j;


	/* Get control parameters*/

	/* Verify control parameters */

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      FFTsize=%d\n", FFTsize);
	printf("O--------------------------------------------------------------------------------------------O\n");

/*	// CALCULATE DMRS sequence 
	//M_RS_SC = 48;
	DMRS_length=genRSsignalargerThan3RB(1, 0, 10, M_RS_SC, DMRS_SEQ, 0);
	//DMRS_length=genRSsignalargerThan3RB(0, 1, 10, M_RS_SC, DMRS_SEQ, 0);
	//createDMRS(DMRS_SEQ);
	// Generate Transmitter DMRS_PUSCH time sequence
	genDMRStime_seq(FFTsize, DMRS_SEQ, M_RS_SC, DMRS_TX_TIME, TXMODE);
	// Generate Receiver DMRS_PUSCH time sequence
	genDMRStime_seq(FFTsize, DMRS_SEQ, M_RS_SC, DMRS_RX_TIME, RXMODE);
*/



	//GENERATE PSSs SEQUENCES: Only for FFT 512
	for(i=0;i<NofNID2;i++){
		gen_PSS_time(i, FFTsize, PSS_time+i*MAXFFTSIZE);
	}
	/*pPSS0=PSS_time;
	pPSS1=PSS_time+MAXFFTSIZE;
	pPSS2=PSS_time+2*MAXFFTSIZE;
*/
	printf("%s END Init\n", mname);

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
	input_t *input0 = in(inp,0);
	input_t *MAC_RX_IN = in(inp,1);
	output_t *output0 = out(out,0);
	output_t *output1 = out(out,1);
	output_t *output2 = out(out,2);

	int rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
	int snd_samples0=0, snd_samples1=0, snd_samples2=0;
	int i,k;
	static int pMAX=-1;
	static int NID2=-1;
	static int NID1=0;	//TODO
	static int already_sent_NIDs=0;


	float Dphase=0.0;
	int pMAXA[3];
	_Complex float auxbuffer[INPUT_MAX_DATA];
	static int WRBUFFERIDX=-1;
	
	
	// Check if data received
	if (rcv_samples == 0)return(0);


	//SynchroINFO.FFTsize=*(MAC_RX_IN);
	SynchroINFO.FFTsize=512;	//TODO

	//printf("%s IN Tslot=%d: rcv_samples=%d\n", mname, TSLOT, rcv_samples);

	// TRACKING MODE
	if(NID2 >= 0){
		stream_conv_CPLX(input0, rcv_samples, &PSS_time[0]+NID2*MAXFFTSIZE, FFTsize, CorrResult);
		pMAX=detect_PSSs_in_subframe(CorrResult, rcv_samples, &SynchroINFO);
		//if(pMAX >= 0)printf("TRACKING MODE NEW PSS sequence found: pMAX=%d, NID2=%d\n", pMAX, NID2);
	}



	// FIRST PSS DETECTION: One of three
	if(NID2 == -1){
		// SAVE LAST RECEIVED SUBFRAME
		memcpy(auxbuffer, input0, sizeof(_Complex float)*rcv_samples);
		// DO&CHECK CORRELATION


		stream_conv_CPLX(input0, rcv_samples, &PSS_time[0]+2*MAXFFTSIZE, FFTsize, CorrResult);
		pMAXA[2]=detect_PSSs_in_subframe(CorrResult, rcv_samples, &SynchroINFO);

		stream_conv_CPLX(input0, rcv_samples, &PSS_time[0]+MAXFFTSIZE, FFTsize, CorrResult);
		pMAXA[1]=detect_PSSs_in_subframe(CorrResult, rcv_samples, &SynchroINFO);

		stream_conv_CPLX(input0, rcv_samples, &PSS_time[0], FFTsize, CorrResult);
		pMAXA[0]=detect_PSSs_in_subframe(CorrResult, rcv_samples, &SynchroINFO);

		printf("pMAX0=%d, pMAX1=%d, pMAX2=%d\n", pMAXA[0], pMAXA[1], pMAXA[2]);
		for(i=0; i<3; i++){
			if(pMAXA[i] > 0){
				NID2=i;
				SynchroINFO.NID2=i;
				pMAX=pMAXA[i];
			}
		}
		if(pMAX >= 0)printf("PSS sequence found: pMAX=%d, NID2=%d\n", pMAX, NID2);
	}

	// UPDATE MAC RX
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







/*
	// DETECT CFO
	Dphase=checkPhaseOffset(&CorrResult[0]+pMAX);
	printf("DetectedPhase=%f\n", Dphase);
	// CORRECT CFO
	rotateCvector(input0, input0, rcv_samples, -Dphase);
*/
	// WRITE INTO ALIGNED SUBFRAME BUFFER
	if(pMAX>=0)WRBUFFERIDX=pMAX;
	else WRBUFFERIDX=-1;
	write_subframe_buffer(input0, WRBUFFERIDX, rcv_samples, &SynchroINFO, auxbuffer);
	
	// READ FROM ALIGNED SUBFRAME AND COPY TO OUTPUT
	snd_samples0=read_subframe_buffer(output0, &SynchroINFO);



	// OUTPUT 2
	memcpy(output2, CorrResult, sizeof(_Complex float)*rcv_samples);
	snd_samples2=rcv_samples;

/*	// OUTPUT 2
	memcpy(output2, &PSS_time[0], sizeof(_Complex float)*512);
	snd_samples2=512;
*/
	// TIMESLOT COUNT
	TSLOT++;
	// Indicate the number of samples at output number N
//	set_output_samples(1, snd_samples1);
	set_output_samples(2, snd_samples2);

//	printf("%s OUT: snd_samples0=%d\n", mname, snd_samples0);
	// Indicate the number of samples at output 0 with return value
	return snd_samples0;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


