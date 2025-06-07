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

#include "5G_RATE_MATCH_PDCCH_functions.h"

RM_t RM_info;
MAC_struc MAC_info;
Bufer_MAC MAC_buff;

//Table 5.4.1.1-1 from 38.212
P[32]={0,1,2,4,3,5,6,7,8,16,9,17,10,18,11,19,12,20,13,21,14,22,15,23,24,25,26,28,27,29,30,31};



void Buffer_Manager(int* Agr_lvl_DCI,int* Control_symb_number, MAC_struc* inputMAC,int Ts,int delay){

	MAC_struc out_buff;

	int buff_in_position=Ts%max_buffer_size;
	//if(Ts%max_buffer_size==0)buff_in_position=0;
	int buff_out_position=buff_in_position-delay;
	MAC_buff[buff_in_position]=*inputMAC;

	if (buff_out_position<0){
		buff_out_position=buff_out_position+(max_buffer_size);
	}
	
	out_buff=MAC_buff[buff_out_position];

	//printf("TS_MAC out from CRC CONTROL %d \n",out_buff.Ts_MAC);
	//printf("--------------NEW FEATURE--------- CELLID=%d Ts=%d   coderate=%f\n" ,out_buff.CELLID,out_buff.Ts_MAC,out_buff.coderateMCS);

	//Advance_buffer(Bufffer,inputMAC,max_buffer_size);
	*(Agr_lvl_DCI)=out_buff.Agr_lvl_DCI;
	*(Control_symb_number)=out_buff.CORESETS_symbol_size;

}




void Sub_block_interleaving(char* in, char* out,int rcv_samples){
	int N=rcv_samples;	
	int i;
	int J[32];
	for(int n=0;n<N;n++){
		i=floor(32.0*(float)n /(float) N);
		J[n]=P[i]*(N/32)+(n%(N/32));
		out[n]=in[J[n]];
		//printf("J[%d]=%d\n",n,J[n]);
	}
}

void Sub_block_deinterleaving(float* in, float* out,int rcv_samples){
        int N=rcv_samples;	
	int i;
	int J[32];
        for(int n=0;n<N;n++){
                i=floor(32.0*(float)n /(float) N);
		J[n]=P[i]*(N/32)+(n%(N/32));
                out[J[n]]=in[n];
        }

}

void do_Rate_Match(char* in, char* out,RM_t* RM_info){
	
	char sub_block_interleaved[2048];
	
        RM_info->E=calc_out_bits_max_length(RM_info->Carr_agr_lvl,RM_info->Control_symb_number);

	Sub_block_interleaving(in,&sub_block_interleaved,RM_info->N);
	
	Bit_Selection(&sub_block_interleaved, out,RM_info);
}

void undo_Rate_Match(float* in, float* out,RM_t* RM_info){
        float recovered[2048]; 
       
        RM_info->E=calc_out_bits_max_length(RM_info->Carr_agr_lvl,RM_info->Control_symb_number);
        int n=Calc_n(RM_info->E,RM_info->K);
        RM_info->N=pow(2,n);

        Bit_Recover_Floats(in, &recovered, RM_info);

        Sub_block_deinterleaving( &recovered, out, RM_info->N);
}

void Bit_Recover_Floats(float* in, float* out, RM_t* RM_info){
	int E=RM_info->E;
	int K=RM_info->K;
	int N=RM_info->N;

        if(E>=N){//repetition
		for (int k=0;k<E;k++){
			out[k%N]=in[k];
		}
		
	}else{
		if((float)K/(float)E<=7.0/16.0){//puncturing
			for(int k=0;k<E;k++){
				out[k+N-E]=in[k];		
			}
			for (int k=0;k<N-E;k++){
				out[k]=0.0;
			}

		}else{//shortening
			for(int k=0;k<E;k++){
				out[k]=in[k];		
			}
			for (int k=E;k<N;k++){
				out[k]=0.0;
			}			
		}
	}
	

}

void Bit_Selection(char* in, char* out,RM_t* RM_info){
	int E=RM_info->E;
	int K=RM_info->K;
	int N=RM_info->N;

	if(E>=N){//repetition
		for (int k=0;k<E;k++){
			out[k]=in[k%N];
		}
	}else{
		if((float)K/(float)E<=7.0/16.0){//puncturing
			for(int k=0;k<E;k++){
				out[k]=in[k+N-E];		
			}
		}else{//shortening
			for(int k=0;k<E;k++){
				out[k]=in[k];		
			}			
		}
	}

}


int calc_out_bits_max_length(int Carr_agr_lvl,int Coreset_symbol_syze){
	int CCE_num=Carr_agr_lvl;
	int REG_per_CCE=6;
	int RE_DMRS_per_CCE=18;
	int RE_in_REG=12;
	int symb_num=Coreset_symbol_syze;

	int CCE_total=REG_per_CCE*CCE_num;
	int Available_RE_total=RE_in_REG*symb_num*CCE_total;
	int RE_DMRS_total=RE_DMRS_per_CCE*CCE_num;
	int RE_data=Available_RE_total-RE_DMRS_total;
	int bits_data=2*RE_data; // 2 comes from QPSK is always used in PDCCH
	//printf("E=%d\n",bits_data);
	return bits_data;

}
 

int Calc_n(int E,int K){
	float e= (float)E;
	float k=(float)K;
	int n1;
	if( e<=(9/8)*pow(2.0,ceil(log2f(e))-1)  && k/e<9/16){
		n1=ceil(log2f(e))-1;
		//printf("n1 if =%d\n",n1);
	}else{
		n1=ceil(log2f(e));
		//printf("n1 else =%d\n",n1);
	}

	float Rmin=1.0/8.0;
	int n2=ceil(log2f(k/Rmin));
	//printf("n2 =%d\n",n2);
	int nmin=5;
	int nmax=9;
	
	//Calc MIN
	int n_min=1000;
	if(n1<n_min) n_min=n1;
	if(n2<n_min) n_min=n2;
	if(nmax<n_min) n_min=nmax;
	
	//printf("RM/URM n selected=%d\n",n_min);
	//Calc MAX
	if(n_min>nmin)return n_min;
	else return nmin;
 

}

















