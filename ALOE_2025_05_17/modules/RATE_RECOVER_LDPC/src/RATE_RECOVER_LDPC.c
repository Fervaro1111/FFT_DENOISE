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

#include "RATE_RECOVER_LDPC_interfaces.h"
#include "RATE_RECOVER_LDPC_functions.h"
#include "RATE_RECOVER_LDPC.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="RATE_RECOVER_LDPC";

//Parametros del Rate_Recover
int rv=0;			//redundancy version (1,2,3,4)
int NBG=1;			//1 or 2
int modulation=2;	//1 if BPSK, 2 if QPSK, 4 if 16 QAM, 6 if 64 QAM, 8 if 256 QAM
int nlayers=1;		//PDSCH (1,2...8) in PUSCH (1,2,3,4)
int TBSLBRM=0;		//Always 0, con 1 no se ha programado
int trblklen=0;		//Longitud original del transport block sin CRC

//Global Variables
int Tslot = 0;

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */

int initialize() {

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("rv", &rv);
	param_get_int("NBG", &NBG);
	param_get_int("modulation", &modulation);
	param_get_int("nlayers", &nlayers);
	param_get_int("TBSLBRM", &TBSLBRM);
	param_get_int("trblklen", &trblklen);

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      rv=%d, NBG=%d, modulation=%d, nlayers=%d, TBSLBRM=%d\n", rv, NBG, modulation, nlayers, TBSLBRM, trblklen);
	printf("O--------------------------------------------------------------------------------------------O\n");

	return 0;
}
//
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
	input_t *input0;
	output_t *output0;

	input0 = in(inp,0);	//Por este llega el stream de datos (samples)
	output0 = out(out,0);


	printf("IN RATE_RECOVER\n");
	/*for (int i=0; i<rcv_samples; i++) {
		printf("%f ",input0[i]);
	}*/

	//Tengo que pensar en qué necesito que me entre para poder llevar a cabo el procesamiento

	/* GET THE NUMBER OF SAMPLES RECEIVED */

	printf("Rate recover entro en la función: rcv_samples=%d\n", rcv_samples);
	
	// Check if data received
	if (rcv_samples == 0)return(0);

	

	int output_len;
	double CB_num, CB_len;
	float out_aux[512*2048];

	h5gRateRecoverLDPC(input0, output0, &output_len, &CB_num, &CB_len, rcv_samples, trblklen, rv, NBG, modulation, nlayers);
	

	//printf("RATERECOVER OUT LENGTH =%d\n",output_len);

	/*printf("OUT_AUX RATE_RECOVER\n");
	for (int i=0; i<output_len; i++) {
		printf("%f ",out_aux[i]);
	}*/

	//Por último lo pasamos a un vector con la medida exacta de CB_num*CB_len
	//output0[output_len];
	/*for (int i = 0; i < output_len; i++) {
		*(output0+i) = out_aux[i];
		//printf("%f ",output0[i]);
		//if(out[i] =! 0) printf("%d \n", out[i]);
	}*/

	//memcpy(output0,out_aux,sizeof(out_aux));

	//int n = sizeof(output_len) / sizeof(int);
	//printf("Resultado del vector es: %d \n", n);
	/*
	int z = 0;
	for(int i=0; i<output_len; i++){
	if(out_final[0+i] =! 0){
	    z += 1;
	    printf("%d", out_final[i]);
	}
	}
	printf("\n");
	printf("%d", z);*/

	snd_samples=rcv_samples;

	if (rcv_samples != 0) {
		printf("Rate recover: rcv_samples=%d y snd_samples=%d\n", rcv_samples, snd_samples);
		//printf("Algun output=%d",output0[0]);
	}

	printf("OUTPUT RATE_RECOVER\n");
	for (int i=0; i<snd_samples; i++) {
		printf("%f ",output0[i]);
	}

	


	Tslot++;
	set_output_samples(0, snd_samples);	//	set_output_samples(Output_number, number_of_samples);
	return 1;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


