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

#include "DEMODULATOR_functions.h"

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






int mod_4QAM (char *bits, int numbits, _Complex float *symbols){	
	int i;
	int j=0;
	for(i=0;i<numbits;){
		if (bits[i]==0 && bits[i+1]==0){
		symbols[j]=QAM4_LEVEL+QAM4_LEVEL*I;
		}if (bits[i]==0 && bits[i+1]==1){
		symbols[j]=QAM4_LEVEL-QAM4_LEVEL*I;
		}if (bits[i]==1 && bits[i+1]==1){
		symbols[j]=-QAM4_LEVEL-QAM4_LEVEL*I;
		}if (bits[i]==1 && bits[i+1]==0){
		symbols[j]=-QAM4_LEVEL+QAM4_LEVEL*I;
		}
		i=i+2;
		j++;
	
	}

	return j;
}


int mod_16QAM (char *bits, int numbits, _Complex float *symbols)
{
	int i, j=0;

	if(numbits==0)return(0);

	for (i=0;i<numbits;i+=4)
	{
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
		pter=(in+i*8);
		bits2byteLSBF(&out[i], &pter, 8);
	}


}



int demod_4QAM (char * bits, _Complex float * symbols, int numsymb)
{
	int i, j=0;
	float in_real, in_imag;

	for (i=0;i<numsymb*2;i+=2)
	{
		in_real = creal(symbols[j]);
		in_imag = cimag(symbols[j]);
		if(in_real>0 && in_imag>0){
		bits[i]=0;
		bits[i+1]=0;
		}if(in_real>0 && in_imag<0){
		bits[i]=0;
		bits[i+1]=1;
		}if(in_real<0 && in_imag<0){
		bits[i]=1;
		bits[i+1]=1;
		}if(in_real<0 && in_imag>0){
		bits[i]=1;
		bits[i+1]=0;
		}
		j++;
	}
	//printf("DEMOD4QAM = %d\n",j);
	return i;
}

int demod_BPSK (char * bits, _Complex float * symbols, int numsymb)
{
	int i,j=0;
	float in_real;

	for (i=0;i<numsymb;i++){
		in_real = creal(symbols[j]);
	//printf("inreal =%f\n,inreal)
		if(in_real>0){
			bits[i]=0;
		}if(in_real<0){
			bits[i]=1;
		}
		j++;
	}
	//printf("DEMOD4QAM = %d\n",j);
	return j;
}

int soft_demod_BPSK(_Complex float *symbols, int numinputsymb, float *softbits){
	int i, j=0;
	float in_real, in_imag;

	// Normalize input
	normBPSK(symbols, numinputsymb);


	for (i=0;i<numinputsymb;i++){
		softbits[j+0]=creal(*(symbols+i));
		softbits[j+1]=cimag(*(symbols+i));
		j++;
	}
	return(j);
}

int normBPSK(_Complex float *inout, int length){
	int i;
	float auxR, auxI, averg=0.0, Q=-0.0; 
	static float ratio=0.0;

	for(i=0; i<length; i++){
		auxR=fabs(__real__ inout[i]);
		auxI=fabs(__imag__ inout[i]);
		averg = averg + auxR + auxI;
	}

	averg=averg/(2.0*(float)length+0.00000001);
	ratio = (BPSK_LEVEL)/(averg+Q);		
	for(i=0; i<length; i++){
		inout[i] = inout[i]*ratio;
	}
	return(1);
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

	printf("Buffer_out_this_TS_DEM = %d   \n",buffer_out);
	printf("Buffer_IN_this_TS_DEM = %d   \n",new_value);
	int i;
	for (i=buffer_size-1;i>=0;i--){
		buffer[i+1]=buffer[i];
	//	printf("Buffer data_CODE_BLOCK in pos %d is %d\n",i,buffer[i]);
	}	
	buffer[0]=new_value;

	return buffer_out;
}




