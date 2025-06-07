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

#include "MODULATOR_functions.h"



#define BPSK_LEVEL      0.7071
#define QAM4_LEVEL      0.7071    ///< QAM4 amplitude (RMS=1)

#define QAM16_LEVEL_1   0.3162    ///< Low 16-QAM amplitude (RMS=1)
#define QAM16_LEVEL_2   0.9487    ///< High 16-QAM amplitude (RMS=1)
#define QAM16_THRESHOLD	0.6324    ///< 16-QAM threshold for RMS=1 signal

#define QAM64_LEVEL_1	0.1543    ///< Low 16-QAM amplitude (RMS=1)
#define QAM64_LEVEL_2	0.4629    ///< High 16-QAM amplitude (RMS=1)
#define QAM64_THRESHOLD_1	0.3086    ///< 16-QAM threshold for RMS=1 signal
#define QAM64_THRESHOLD_2	0.6172    ///< 16-QAM threshold for RMS=1 signal


/* INIT PHASE FUNCTIONS #####################################################################################################*/



/* WORK PHASE FUNCTIONS #####################################################################################################3*/
//
//  1011    1001    0001    0011
//  1010    1000    0000    0010
//  1110    1100    0100    0110
//  1111    1101    0101    0111
//



int mod_16QAM (char *bits, int numbits, _Complex float *symbols)
{
	int i, j=0;

	//if(numbits==0)return(0);

	for (i=0;i<numbits;i+=4){

		symbols[j]  =   ((bits[i+0]==1)?(-1):(+1))*((bits[i+2]==1)?(QAM16_LEVEL_2):(QAM16_LEVEL_1));
		symbols[j] += I*((bits[i+1]==1)?(-1):(+1))*((bits[i+3]==1)?(QAM16_LEVEL_2):(QAM16_LEVEL_1));
		j++;
	}
	
if((numbits/4) != j){
		printf("mod_16QAM(): ERROR numbits=%d, j=%d\n", numbits, j);
		exit(0);
	}

//	for(i=0; i<numbits/4; i++)*(symbols+i)=(float)i-((float)i)*I;

	return j;
}

void Byte2bit(char *in, char *out, int rcv_samples){
	int i;	
	char *pter;

	
	for(i=0; i<rcv_samples; i++){
		pter=(out+i*8);
		byte2bitsLSBF(*(in+i), &pter, 8);
	}		

/*	
	for(i=0; i<rcv_samples*8; i++){
		*(bits_temp_scramb+i) = (*(bitsOUTaux+i) ^ *(scrambseq+i)) & 0x01;
	}



	for(i=0; i<rcv_samples; i++){
		pter=(char *)(bits_temp_scramb+i*8);
		bits2byteLSBF(&out[i], &pter, 8);
	}
*/
}

void bit2Byte(char *in, char *out, int rcv_samples){
	char *pter;
	int i;	
	for(i=0; i<rcv_samples; i++){
		pter=(char *)(in+i*8);
		bits2byteLSBF(&out[i], &pter, 8);
	}


}



int hard_demod_16QAM (char * bits, _Complex float * symbols, int numsymb)
{
	int i, j=0;
	float in_real, in_imag;

	for (i=0;i<numsymb;i++)
	{
		in_real = creal(symbols[i]);
		in_imag = cimag(symbols[i]);
		bits[j++] = (in_real<0)?1:0;
		bits[j++] = (in_imag<0)?1:0;
		bits[j++] = (fabs(in_real)-QAM16_THRESHOLD<0)?1:0;
		bits[j++] = (fabs(in_imag)-QAM16_THRESHOLD<0)?1:0;
	}
	return j;
}
int mod_4QAM(char *bits, int numbits, _Complex float *symbols){
    

	int i, k, j=0;
	
	if(numbits==0)return(0);
	
	for (i=0;i<numbits;i+=2){
		k = ((bits[i] == 1) ? 1 : 0) <<1 | ((bits[i+1] == 1) ? 1 : 0);
		//printf("K=%d\n",k);
		switch (k) {
		case 0:
			symbols[j]=QAM4_LEVEL+QAM4_LEVEL*I;
			break;
		case 1:
			symbols[j]=QAM4_LEVEL-QAM4_LEVEL*I;
			break;
		case 2:
			symbols[j]=-QAM4_LEVEL+QAM4_LEVEL*I;
			break;
		case 3:
			symbols[j]=-QAM4_LEVEL-QAM4_LEVEL*I;
			break;
		default:
			printf("ERROR: Unknown symbol for QAM4: %d\n",k);
			exit(0);
		}
		j++;
		
	}
	
	return j;
}

int mod_BPSK(char *bits, int numbits, _Complex float *symbols){
    

	int i, k, j=0;
	
	if(numbits==0)return(0);
	
	for (i=0;i<numbits;i++){
		k =(int)bits[i];
		//printf("K=%d\n",k);
		switch (k) {
		case 0:
			symbols[j]=BPSK_LEVEL+0*I;
			break;
		case 1:
			symbols[j]=-BPSK_LEVEL+0*I;
			break;
		
		default:
			printf("ERROR: Unknown symbol for BPSK: %d\n",k);
			exit(0);
		}
		j++;
		
	}
	
	return j;
}



void byte2bitsLSBF(char value, char **bits, int nof_bits)
{
    int i;
    for(i=0; i<8; i++) {
		if(i==nof_bits)break;
        (*bits)[i] = (value >> (8-i-1)) & 0x1;
    }
}

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

int buffer(int buffer_size,unsigned int new_value, int *buffer){
	int buffer_out=buffer[buffer_size-1];

	printf("Buffer_out_this_TS = %d   \n",buffer_out);
	printf("Buffer_IN_this_TS = %d   \n",new_value);
	int i;
	for (i=buffer_size-1;i>=0;i--){
		buffer[i+1]=buffer[i];
	//	printf("Buffer data_CODE_BLOCK in pos %d is %d\n",i,buffer[i]);
	}	
	buffer[0]=new_value;

	return buffer_out;
}


