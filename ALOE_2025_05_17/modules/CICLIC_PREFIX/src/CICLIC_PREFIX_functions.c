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

#include "CICLIC_PREFIX_functions.h"

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK


*/
int FFT_length_calc(int rcv_samples,int numFFT){
	
	int dif=0;
	for (int i=0;i<14;i++){
		dif=pow(2,i)-(rcv_samples/numFFT);
		if (dif>0){
			return pow(2,(i-1));
		}
		if (dif==0){
			return pow(2,i);
		}
		

	}


}



void ADD_cyclic_Prefix(_Complex float* input,_Complex float* output,int rcv_samples,int numFFT){

	int samp_2_cpy;
	int out_pos=0;


	int samp_per_symbol=rcv_samples/numFFT;

	
	for(int i=0;i<numFFT;i++){
		
		samp_2_cpy=Calc_Samp2Cpy(i,rcv_samples,numFFT);
		//printf("samp_2_cpy=%d at symb=%d\n",samp_2_cpy,i);
		ADD_CP_2_symbol(input+i*samp_per_symbol,output+out_pos,samp_per_symbol,samp_2_cpy,rcv_samples);

		out_pos=out_pos+samp_2_cpy+samp_per_symbol;
		//printf("out_pos=%d at symb=%d\n",out_pos,i);
	}
	

/*
	//TEST FOR FFT SIZE DETERMINATION//

	_Complex float test[512*15*4];
	memcpy(&test,output,512*15*sizeof(_Complex float));

	int j=0;
	for(int i=0; i<512*15;i++){
		output[j]=test[i];
		output[j+1]=test[i];
		output[j+2]=test[i];
		output[j+3]=test[i];	
		j=j+4;	
	}




	//TEST FOR FFT SIZE DETERMINATION//

*/

}

void ADD_CP_2_symbol(_Complex float* input,_Complex float* output,int data_per_symbol,int samp_2_cpy,int rcv_samples){

	memcpy(output+samp_2_cpy,input,data_per_symbol*sizeof(_Complex float));
	memcpy(output,input+data_per_symbol-samp_2_cpy,samp_2_cpy*sizeof(_Complex float));

}
			

int Calc_Samp2Cpy(int OFDMsymb,int rcv_samples,int numFFT){
	float tmp=0.0;
	int samp_2_cpy;
//	printf("OFDMsymb=%d \n",OFDMsymb);
	
	if(OFDMsymb!=0 && OFDMsymb!=7){
		tmp=((float)rcv_samples/((float)numFFT))*(7.0/100.0);
		samp_2_cpy=(int)ceil(tmp);
	
	}else{
		tmp=((float)rcv_samples/((float)numFFT))*(7.8/100.0);
		samp_2_cpy=(int)ceil(tmp);

	}
	return(samp_2_cpy);


}

//////////////////////////////////////////////////////////////////

void REMOVE_cyclic_Prefix(_Complex float* input,_Complex float* output,int rcv_samples,int numFFT){

	int samp_2_cpy;
	int in_pos=0;


	int samp_per_symbol=rcv_samples/(numFFT+1);

	
	for(int i=0;i<numFFT;i++){
		
		samp_2_cpy=Calc_Samp2Cpy_RX(i,rcv_samples,numFFT);
		//printf("samp_2_cpy=%d at symb=%d\n",samp_2_cpy,i);
		
		REMOVE_CP_2_symbol(input+in_pos,output+i*samp_per_symbol,samp_per_symbol,samp_2_cpy,rcv_samples);

		in_pos=in_pos+samp_per_symbol+samp_2_cpy;
		//printf("in_pos=%d at symb=%d\n",in_pos,i);
	}
	


}

int Calc_Samp2Cpy_RX(int OFDMsymb,int rcv_samples,int numFFT){
	float tmp=0.0;
	int samp_2_cpy;
	
	if(OFDMsymb!=0 && OFDMsymb!=7){
		tmp=(float)(rcv_samples/((float)numFFT+1.0))*(7.0/100.0);
		samp_2_cpy=(int)ceil(tmp);
	
	}else{
		tmp=(float)(rcv_samples/((float)numFFT+1.0))*(7.8/100.0);
		samp_2_cpy=(int)ceil(tmp);

	}
	return(samp_2_cpy);


}


void REMOVE_CP_2_symbol(_Complex float* input,_Complex float* output,int data_per_symbol,int samp_2_cpy,int rcv_samples){

	memcpy(output,input+samp_2_cpy,data_per_symbol*sizeof(_Complex float));

}
	










