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

#include "5G_INTERLEAVER_PDCCH_functions.h"

Interl_t Interleave_info;

//Interleaving pattern from Table 5.3.1.1-1 Interleaving pattern PI_il(m) from 38.212
int pi_max_il[K_max]={0,2,4,7,9,14,19,20,24,25,26,28,31,34,42,45,49,50,51,53,54,56,58,59,61,62,65,66,67,69,70,71,72,76,77,81,82,83,87,88,89,91,93,95,98,101,104,106,108,110,111,113,115,118,119,120,122,123,126,127,129,132,134,138,139,140,1,3,5,8,10,15,21,27,29,32,35,43,46,52,55,57,60,63,68,73,78,84,90,92,94,96,99,102,105,107,109,112,114,116,121,124,128,130,133,135,141,6,11,16,22,30,33,36,44,47,64,74,79,85,97,100,103,117,125,131,136,142,12,17,23,37,48,75,80,86,137,143,13,18,38,144,39,145,40,146,41,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163};


void do_interleave(char* in, char* out,Interl_t* Interleave_info,int rcv_samples_bits){

	//int bits_data=calc_out_bits_max_length(Interleave_info);
	//printf("bits_data=%d\n",bits_data);

	int pattern[rcv_samples_bits];

	Generate_Interl_Pattern(&pattern,rcv_samples_bits,Interleave_info);
	Interleave(in, out,&pattern,rcv_samples_bits);

}

void undo_interleave(char* in, char* out,Interl_t* Interleave_info,int rcv_samples_bits){

	int pattern[rcv_samples_bits];

	Generate_Interl_Pattern(&pattern,rcv_samples_bits,Interleave_info);
	De_Interleave(in, out,&pattern,rcv_samples_bits);

}

	

void Generate_Interl_Pattern(int* pattern,int rcv_samples_bits,Interl_t* Interleave_info){

	if (Interleave_info->I_il==0){
		for (int k=0;k<rcv_samples_bits;k++){
			pattern[k]=k;	
		}
	

	}else{
		int k=0;
		for (int m=0;m<K_max;m++){
			if(pi_max_il[m]>=K_max-rcv_samples_bits){
				pattern[k]=pi_max_il[m]-(K_max-rcv_samples_bits);
				k++;
			}

		}

	}


}



void Interleave(char* in, char* out,int* pattern,int rcv_samples_bits){
	char bitsIN[K_max];
	char *pter;
	char bitsOUT[K_max];
	// FORMAT INPUT BYTES FLOW TO BITS
	for(int i=0; i<rcv_samples_bits/8; i++){
		pter=(char *)(bitsIN+i*8);
		byte2bitsLSBF(*(in+i), &pter, 8);
		
	}

	for(int i=0;i<rcv_samples_bits;i++){
		bitsOUT[i]=bitsIN[pattern[i]];
	}
	// PACK BITS TO BYTES FOR OUTPUT	
	for(int i=0; i<rcv_samples_bits/8; i++){
		pter=(char *)(bitsOUT+i*8);
		bits2byteLSBF(&out[i], &pter, 8);

	}

}

void De_Interleave(char* in, char* out,int* pattern,int rcv_samples_bits){
	char bitsIN[K_max];
	char *pter;
	char bitsOUT[K_max];
	// FORMAT INPUT BYTES FLOW TO BITS
	for(int i=0; i<rcv_samples_bits/8; i++){
		pter=(char *)(bitsIN+i*8);
		byte2bitsLSBF(*(in+i), &pter, 8);
		
	}

	for(int i=0;i<rcv_samples_bits;i++){
		bitsOUT[pattern[i]]=bitsIN[i];
	}

	// PACK BITS TO BYTES FOR OUTPUT	
	for(int i=0; i<rcv_samples_bits/8; i++){
		pter=(char *)(bitsOUT+i*8);
		bits2byteLSBF(&out[i], &pter, 8);

	}

}


// Left Significant bit first
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


