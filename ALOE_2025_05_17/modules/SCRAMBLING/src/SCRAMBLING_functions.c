/* 
 * Copyright (c) 2012
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

/* Functions that generate the test data fed into the DSP modules being developed */
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

#include "SCRAMBLING_functions.h"

#define Nc 1600


void create_scrambling_sequence_BITS(int RNTI, int CELLID, int sequencelength, char *scrambseq){
	
	int n;
	int x1[MAXSEQLENGTH*2], x2[MAXSEQLENGTH*2];

	int seed = CELLID; 	//TO BE REVISED


	for (n = 0; n < 31; n++) {
    	x2[n] = (seed >> n) & 0x1;
  	}
  	x1[0] = 1;
  	for (n = 0; n < Nc + sequencelength; n++) {
    	x1[n + 31] = (x1[n + 3] + x1[n]) & 0x1;
    	x2[n + 31] = (x2[n + 3] + x2[n + 2] + +x2[n+1] + x2[n]) & 0x1;
  	}
  	for (n = 0; n < sequencelength; n++) {
    	scrambseq[n] = (char)((x1[n + Nc] + x2[n + Nc]) & 0x1);
  	}
}

void create_scrambling_sequence_Oriol(int RNTI, int CELLID, int sequencelength, char *scrambseq, int q){
	
	int n;
	int x1[MAXSEQLENGTH*2], x2[MAXSEQLENGTH*2];
	
	int nRNTI=RNTI;
	// q=0 or 1 depending on the number of layers. CHECK definition!
	int Cell_id=CELLID;

	int Cinit=nRNTI*pow(2,15)+q*pow(2,14)+Cell_id;
	for (n = 0; n < 31; n++) {
    	x2[n] = (Cinit >> n) & 0x1;
  	}
  	x1[0] = 1;
  	for (n = 0; n < Nc + sequencelength; n++) {
    	x1[n + 31] = (x1[n + 3] + x1[n]) & 0x1;
    	x2[n + 31] = (x2[n + 3] + x2[n + 2] + +x2[n+1] + x2[n]) & 0x1;
  	}
  	for (n = 0; n < sequencelength; n++) {
    	scrambseq[n] = (char)((x1[n + Nc] + x2[n + Nc]) & 0x1);
  	}
}



void create_scrambling_sequence_FLOATS(int RNTI, int CELLID, int sequencelength, float *scrambseq,int q){

	int n;
	int x1[MAXSEQLENGTH*2], x2[MAXSEQLENGTH*2];
	int aux;

	//int seed = CELLID; 	//TO BE REVISED

	int nRNTI=RNTI;
	// q=0 or 1 depending on the number of layers. CHECK definition!
	int Cell_id=CELLID;

	int Cinit=nRNTI*pow(2,15)+q*pow(2,14)+Cell_id;
	for (n = 0; n < 31; n++) {
    	x2[n] = (Cinit >> n) & 0x1;
  	}

  	x1[0] = 1;
  	for (n = 0; n < Nc + sequencelength; n++) {
    	x1[n + 31] = (x1[n + 3] + x1[n]) & 0x1;
    	x2[n + 31] = (x2[n + 3] + x2[n + 2] + +x2[n+1] + x2[n]) & 0x1;
  	}
  	for (n = 0; n < sequencelength; n++) {
    	aux = (int)((x1[n + Nc] + x2[n + Nc]) & 0x1);
		if(aux == 0)scrambseq[n] = 1.0;
		if(aux == 1)scrambseq[n] = -1.0;
  	}
}


/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
void scrambling(char *in, char *out, int rcv_samples, char *scrambseq){
	int i;	
	char *pter;
	char bitsOUTaux[2048*50*8];
	char bits_temp[2048*50*8];
	char bits_temp_scramb[2048*50*8];
	
	for(i=0; i<rcv_samples; i++){
		pter=(char *)(bitsOUTaux+i*8);
		byte2bitsLSBF(*(in+i), &pter, 8);
	}		

	
	for(i=0; i<rcv_samples*8; i++){
		*(bits_temp_scramb+i) = (*(bitsOUTaux+i) ^ *(scrambseq+i)) & 0x01;
	}

	for(i=0; i<rcv_samples; i++){
		pter=(char *)(bits_temp_scramb+i*8);
		bits2byteLSBF(&out[i], &pter, 8);
	}
}

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
void descrambling(char *in, char *out, int rcv_samples, char *scrambseq){
	
	int i;
	for(i=0; i<rcv_samples; i++){
		*(out+i) = (*(in+i) ^ *(scrambseq+i)) & 0x01;
	}
}

void byte2bitsLSBF(char value, char **bits, int nof_bits)
{
    int i;
    for(i=0; i<8; i++) {
		if(i==nof_bits)break;
        (*bits)[i] = (value >> (8-i-1)) & 0x1;
    }
}

// Left Significant bit first
void bits2byteLSBF(char *byte, char **bits, int nof_bits)
{
    int i;
    char value=0;

	*byte = 0;
    for(i=0; i<8; i++) {
		if(i==nof_bits)break;
    	*byte |= (*bits)[i] << (8-i-1);
    }
}






void descrambling_Floats(float *in, float *out, int rcv_samples, float *scrambseq){
	int i;
	for(i=0; i<rcv_samples; i++){
		*(out+i) = (*(in+i)) * (*(scrambseq+i));
	}
	//printf("PRINTEO I:%i",i);
}


