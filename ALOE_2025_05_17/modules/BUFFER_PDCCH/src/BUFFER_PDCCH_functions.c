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

#include "BUFFER_PDCCH_functions.h"



#define len_buf_pos 14
#define slot_buf 4096
#define len_buf 14*4096 //len_buf_pos X slot_buf


_Complex float BUFFER[len_buf]={0.0+0.0*I};
int lengths_BUFFER[len_buf_pos]={0};
int samp_in_buff=0;


int update_buffer(int desalineacio,_Complex float* inBuff, int in_length,_Complex float* out){

	int out_length;
	int buf_pos=desalineacio*slot_buf;


	//ADD to BUFF
	memcpy(&BUFFER[buf_pos],inBuff,in_length*sizeof(_Complex float));
	lengths_BUFFER[desalineacio]=in_length;

	//GET from BUFF
	
	out_length=lengths_BUFFER[0];
	memcpy(out,&BUFFER[0],out_length*sizeof(_Complex float));

	//UPDATE BUFFER

	memcpy(&lengths_BUFFER[0],&lengths_BUFFER[1],len_buf_pos*sizeof(int));
	memcpy(&BUFFER[0],&BUFFER[slot_buf],len_buf*sizeof(_Complex float));

	return  out_length;

}



int DO_Buffer(_Complex float* in1, _Complex float* in2, _Complex float* out,int rcv_samp1, int rcv_samp2,int desalineacio){

	_Complex float concatenated[4096];

	int total_length=Concatenate_DCIs(in1, in2, &concatenated, rcv_samp1, rcv_samp2);
	int snd_samples=update_buffer(desalineacio,&concatenated,total_length,out);

	return snd_samples;


}




int Concatenate_DCIs(_Complex float* in1, _Complex float* in2, _Complex float* out,int rcv_samp1, int rcv_samp2){

	memcpy(out,in1,rcv_samp1*sizeof(_Complex float));
	memcpy(out+rcv_samp1,in2,rcv_samp2*sizeof(_Complex float));

	return rcv_samp1+rcv_samp2;
}






