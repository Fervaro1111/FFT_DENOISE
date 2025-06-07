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
#include "skeleton.h"
#include "params.h"

#include "LDPC5G_DECODER_interfaces.h"
#include "LDPC5G_DECODER_functions.h"
#include "LDPC5G_DECODER.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="LDPC5G_DECODER";

//Module User Defined Parameters
char matrices_path[256];
int BG          = 1;
int I_LS        = 0;
int Z           = 4;
int BG2          = 1;
int I_LS2        = 0;
int Z2           = 4;
int n_ite       = 10;
int enable_synd = 0;
int synd_depth  = 1;

float Timeslot_us=0.1;


//Global Variables
int K, N;
int K2, N2;

////////INIT COMPUTE LATENCY HISTOGRAM
// Compute Latency Histogram
struct timeval t_start, t_end, t_Tslot;
int pLatency_us; 	// Processing Latency in us
int Tslot=0; 	// Time slot duration in us
int NofPeriods=0;
#define HISTOGRAMSIZE	10000
int Histog[HISTOGRAMSIZE];
int Hresolution = 100; 	//Histogram resolution = 100 us
float TimeWINDOW_us = 213333.3;
int NofWINDOWsamples=4000; 
_Complex float HOUTBuffer[HISTOGRAMSIZE];
int TotalNofSamples=0;
float average=0.0;
////////END COMPUTE LATENCY HISTOGRAM

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {


	/* Get control parameters*/
	param_get_string("matrices_path", &matrices_path[0]);
	param_get_int("BG", &BG);
	param_get_int("I_LS", &I_LS);
	param_get_int("Z", &Z);
	param_get_int("BG2", &BG2);
	param_get_int("I_LS2", &I_LS2);
	param_get_int("Z2", &Z2);
	param_get_int("n_ite", &n_ite);
	param_get_int("enable_synd", &enable_synd);
	param_get_int("synd_depth", &synd_depth);

	////////INIT COMPUTE LATENCY HISTOGRAM
	// Compute Latency Histogram
	param_get_int("NofWINDOWsamples", &NofWINDOWsamples);
	param_get_float("TimeWINDOW_us", &TimeWINDOW_us);
	Hresolution = (int)(TimeWINDOW_us/(float)NofWINDOWsamples);
	////////END COMPUTE LATENCY HISTOGRAM
	

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      matrices_path=%s\n", matrices_path);
	printf("O      BG=%d, I_LS=%d, Z=%d\n", BG, I_LS, Z);
	printf("O      n_ite=%d, enable_synd=%d, synd_depth=%d\n", n_ite, enable_synd, synd_depth);

	////////INIT COMPUTE LATENCY HISTOGRAM
	printf("O      Histogram: NofWINDOWsamples=%d, TimeWINDOW_us=%3.1f us, Hresolution=%d us\n", 
							NofWINDOWsamples, TimeWINDOW_us, Hresolution);
	////////END COMPUTE LATENCY HISTOGRAM
	printf("O--------------------------------------------------------------------------------------------O\n");

////////////////////////First DECODER////////////////////////
	
	// Check BG
	if (BG == 1) {
		K = 22 * Z;
		N = 68 * Z;
	} else if (BG == 2) {
		K = 10 * Z;
		N = 52 * Z;
	} else {
		printf("%s: BG must be 1 or 2\n", mname);
		return 1;
	}
	// Check I_LS
	if (I_LS > 7 || I_LS < 0) {
		printf("%s: I_LS must be between 0 and 7\n", mname);
		return 1;
	}

	char matrix_path[256];
	sprintf(matrix_path, "%s/BG%d_ILS%d_Z%d.qc", matrices_path, BG, I_LS, Z);
	printf("O      %s\n", matrix_path);
	init_decoder(matrix_path, K, N, n_ite, enable_synd, synd_depth);
////////////////////////First DECODER////////////////////////

////////////////////////Second DECODER////////////////////////
	
	if (BG2 == 1) {
		K2 = 22 * Z2;
		N2 = 68 * Z2;
	} else if (BG2 == 2) {
		K2 = 10 * Z2;
		N2 = 52 * Z2;
	} else {
		printf("%s: BG must be 1 or 2\n", mname);
		return 1;
	}
	// Check I_LS
	if (I_LS2 > 7 || I_LS2 < 0) {
		printf("%s: I_LS must be between 0 and 7\n", mname);
		return 1;
	}

	char matrix_path2[256];
	sprintf(matrix_path2, "%s/BG%d_ILS%d_Z%d.qc", matrices_path, BG2, I_LS2, Z2);
	printf("O      %s\n", matrix_path2);
	init_decoder2(matrix_path2, K2, N2, n_ite, enable_synd, synd_depth);
	printf("O--------------------------------------------------------------------------------------------O\n");

	////////////////////////Second DECODER////////////////////////


	///////////////////////Init Histogram////////////////////////
	////////INIT COMPUTE LATENCY HISTOGRAM
	for(int i=0; i<HISTOGRAMSIZE; i++)Histog[i]=0;
	////////END COMPUTE LATENCY HISTOGRAM

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
	//////// COMPUTE LATENCY HISTOGRAM
	gettimeofday(&t_start, NULL);
	////////////////////////////////////

	int rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
	int rcv_samples1 = get_input_samples(1); /** number of samples at itf 0 buffer */
	int snd_samples=0;
	int snd_samples1=0;
	float *input0,*input1;
	unsigned char *output0,*output1;
	input0 = in(inp,0);
	output0 = out(out,0);
	input1 = in(inp,1);
	output1 = out(out,1);

	////////INIT COMPUTE LATENCY HISTOGRAM
 	unsigned char *Thistogram;
	_Complex float *CPLXThistogram;
	Thistogram = out(out,2);
	CPLXThistogram = out(out,2);
	
	////////END COMPUTE LATENCY HISTOGRAM

	
	for(int i=0;i<NOF_INPUT_ITF;i++){
		rcv_samples = get_input_samples(i);
		//printf("%s IN, Tslot=%d: itf=%d, rcv_samples=%d\n", mname, Tslot, i, rcv_samples);
		if(rcv_samples!=0){
			if (rcv_samples == N) {
				snd_samples1 = decode(input0+i*INPUT_MAX_DATA, output0+i*OUTPUT_MAX_DATA, rcv_samples);
			}else if(rcv_samples == N2){
				snd_samples1 = decode2(input0+i*INPUT_MAX_DATA,output0+i*OUTPUT_MAX_DATA, rcv_samples);
			}else{
				printf("%s Wrong number of elements at the input!!!!!\n", mname);
			}
			if(i!=0)set_output_samples(i,snd_samples1);
			else snd_samples=snd_samples1;
		}
	}
	////////INIT COMPUTE LATENCY HISTOGRAM
	gettimeofday(&t_end, NULL);
	pLatency_us = (int)((t_end.tv_sec*1e6 + t_end.tv_usec)-(t_start.tv_sec*1e6 + t_start.tv_usec));
	pLatency_us = pLatency_us/Hresolution;
	//printf("%s: Tslot=%d, pLatency=%d, rcv_samples=%d \n", mname, Tslot, pLatency_us, rcv_samples);
	if(pLatency_us > HISTOGRAMSIZE){
			printf("%s ERROR!!! pLatency=%d > HISTOGRAMSIZE=%d\n", pLatency_us, HISTOGRAMSIZE);
			exit(0);
	}
	Histog[pLatency_us] = Histog[pLatency_us]+1;
	TotalNofSamples += 1;
	Tslot++;
	if(Tslot == 10){
		Tslot=0;
		average = 0.0;
		NofPeriods = NofPeriods+1;
		for(int i=0; i<NofWINDOWsamples; i++){
			if(i<TotalNofSamples)average = average + (float)(Histog[i]*i*Hresolution);
			*(CPLXThistogram+i) = (float)(Histog[i]*100)/(float)(TotalNofSamples) + 0.0*I;
		}
		set_output_samples(2,NofWINDOWsamples*sizeof(_Complex float));
		//printf("%s: average=%3.3f us\n",mname, average/(float)TotalNofSamples);
		//set_output_samples(2,NofWINDOWsamples);
	}
	////////END COMPUTE LATENCY HISTOGRAM
	/////////////////////////////////////////////////////////////////////////////////////////////////

	//printf("%s OUT: snd_samples=%d\n", mname, snd_samples);	
	// Indicate the number of samples at output 0 with return value

	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


