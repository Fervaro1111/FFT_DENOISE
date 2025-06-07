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

#include "LDPC5G_ENCODER_interfaces.h"
#include "LDPC5G_ENCODER_functions.h"
#include "LDPC5G_ENCODER.h"


//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="LDPC5G_ENCODER";

//Module User Defined Parameters
char matrices_path[256];
int BG   = 1;
int I_LS = 0;
int Z    = 4;
int BG2   = 1;
int I_LS2 = 0;
int Z2    = 4;

//Global Variables
int K, N;
int K2, N2;
struct timeval  tv1, tv2;

int ILSTable[8][8]={ 2,  4,  8,  16,  32,  64, 128, 256,
					 3,  6, 12,  24,  48,  96, 192, 384,
					 5, 10, 20,  40,  80, 160, 320,   0,
					 7, 14, 28,  56, 112, 224,   0,   0,
					 9, 18, 36,  72, 144, 288,   0,   0,
					11, 22, 44,  88, 176, 352,   0,   0,
					13, 26, 52, 104, 208,   0,   0,   0,
					15, 30, 60, 120, 240,   0,   0,   0};


/*
 * Function documentation
 *
 * @returns 0 on success, 1 on error
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

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      matrices_path=%s\n", matrices_path);
	printf("O      BG=%d, I_LS=%d, Z=%d\n", BG, I_LS, Z);
	printf("O      BG2=%d, I_LS2=%d, Z2=%d\n", BG2, I_LS2, Z2);

////////////////////////First encoder////////////////////////





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
	printf("%s\n", mname);
	sprintf(matrix_path, "%s/BG%d_ILS%d_Z%d.qc", matrices_path, BG, I_LS, Z);
	printf("O      %s\n", matrix_path);

gettimeofday(&tv1, NULL);
	init_encoder(matrix_path, K, N);
gettimeofday(&tv2, NULL);
printf ("Total time = %f seconds\n",
         (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
         (double) (tv2.tv_sec - tv1.tv_sec));

////////////////////////First encoder////////////////////////

////////////////////////Second encoder////////////////////////
	
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
	printf("%s\n", mname);
	sprintf(matrix_path2, "%s/BG%d_ILS%d_Z%d.qc", matrices_path, BG2, I_LS2, Z2);
	printf("O      %s\n", matrix_path2);
	init_encoder2(matrix_path2, K2, N2);



////////////////////////Second encoder////////////////////////
	printf("O--------------------------------------------------------------------------------------------O\n");

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
	int rcv_samples1 = get_input_samples(1); /** number of samples at itf 0 buffer */
	int snd_samples=0;
	int snd_samples1=0;
	unsigned char *input0, *output0,*input1, *output1,*input2, *output2,*input3, *output3;
	
	input0 = in(inp,0);
	output0 = out(out,0);
	input1 = in(inp,1);
	output1 = out(out,1);
	//input2 = in(inp,2);
	//output2 = out(out,2);
	//input3 = in(inp,3);
	//output3 = out(out,3);

	//printf("%s IN: rcv_samples=%d, K=%d, K2=%d\n", mname, rcv_samples*8, K, K2);

	for(int i=0;i<NOF_INPUT_ITF;i++){
		rcv_samples = get_input_samples(i)*8;
		if(rcv_samples!=0){
			if (rcv_samples == K) {
				snd_samples1 = encode(input0+INPUT_MAX_DATA*i, output0+OUTPUT_MAX_DATA*i, rcv_samples);
			}else if(rcv_samples == K2){
				snd_samples1 = encode2(input0+INPUT_MAX_DATA*i, output0+OUTPUT_MAX_DATA*i, rcv_samples);
			}else{
				printf("%s Wrong number of elements at the input 1 !!!!!\n", mname);
			}
			if(i!=0)set_output_samples(i,snd_samples1);
			else snd_samples=snd_samples1; 
				//printf("%s, output%d, snd_samples=%d\n", mname, i, snd_samples1);
		}
	}



	/*for(i=1; i<rcv_samples*in_interfaces; i++){
		//if (i==(snd_samples/2))printf("\n");
		printf("%02x", (int)(input[10*2048*r+i-1]&0xFF));
		//if(i%128==0)printf("\n");
	}
	printf("\n");
	*/
	


	
	//printf("snd_samples ENCODER======%d\n",snd_samples);
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

/*void CreateILSMatrix(int *ILS){
	
	ILS=2;


}


void CodeBlocksSize(){

}*/





