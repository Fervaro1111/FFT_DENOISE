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

#include "5G_Buffer_PDSCH_functions.h"

int in_pos=0;
int out_pos=0;


int in_pos_rcv_samp=0;
int out_pos_rcv_samp=0;


void Add_to_buffer(_Complex float* in,_Complex float* Buff,int rcv_samples){

	for(int i=0;i<rcv_samples;i++){
		*(Buff+in_pos)=*(in+i);
		in_pos++;
		if(in_pos==Buffer_Size) in_pos=0;
	}

}

void GET_from_buffer(_Complex float* out,_Complex float* Buff,int snd_samples){

	for(int i=0;i<snd_samples;i++){
		*(out+i)=*(Buff+out_pos);
		out_pos++;
		if(out_pos==Buffer_Size) out_pos=0;
	}

}

void Add_to_buffer_rcv_sampl(int rcv_samp_in,int* Buff){

	Buff[in_pos_rcv_samp]=rcv_samp_in;
	in_pos_rcv_samp++;
	if(in_pos_rcv_samp==rcv_samp_Buffer_size) in_pos_rcv_samp=0;

}

int GET_from_buffer_rcv_sampl(int* Buff){

	int rcv_samp_out=Buff[out_pos_rcv_samp];
	out_pos_rcv_samp++;
	if(out_pos_rcv_samp==rcv_samp_Buffer_size) out_pos_rcv_samp=0;
	return rcv_samp_out;	
}


