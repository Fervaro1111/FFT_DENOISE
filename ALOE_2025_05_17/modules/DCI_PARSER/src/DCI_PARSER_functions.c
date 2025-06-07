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

#include "DCI_PARSER_functions.h"

bits_t DCI;
DCIinfo_t DCI_info;


// ESTRUCTURA//

//VRB-PRB 1 bits --> hardcoded at 0
//PRB bundling size indicator 0 bits
//Rate matching indicator 0 bits hardcoded
//ZP CSI-RS trigger – 0 bits hardcoded
//MCS 5 bits
//New data indicator 1bit
//redundancy version 2bits
//HARQ process number – 4 bits-->hardcoded to 0000
//Downlink assignment index  0 bits 
//TPC command for scheduled PUCCH 2 bits hardcoded to 00
//PUCCH resource indicator 3bits hardcoded to 000
//PDSCH-to-HARQ_feedback timing indicator – 0 bits hardcoded
//Antenna port(s) – 4,5,6 bits -->hardcoded to 0000
//Transmission configuration indication 3 bits ->hardoded to 000
//SRS request – 2 bits hardcoded to 00
//CBG transmission information (CBGTI) – 0 bits hardcoded
//CBG flushing out information (CBGFI) – 0 bits hardcoded
//DMRS sequence initialization 0 bits ->hardcoded


void FILL_DCI(bits_t* DCI,DCIinfo_t* DCI_info){
	DCI->f1=1;
	DCI->f2=0;
	DCI->f3=DCI_info->RIV;
	DCI->f4=DCI_info->time_domain_allocation;
	DCI->f5=0;
	DCI->f6=DCI_info->MCS_index;
	DCI->f7=1;
	DCI->f8=DCI_info->rv;
	DCI->f9=0;
	DCI->f10=0;
	DCI->f11=0;
	DCI->f12=0;
	DCI->f13=0;
	DCI->f14=0;

}

void UNFILL_DCI(bits_t* DCI,DCIinfo_t* DCI_info){
	DCI_info->RIV=DCI->f3;
	DCI_info->time_domain_allocation=DCI->f4;
	DCI_info->MCS_index=DCI->f6;
	DCI_info->rv=DCI->f8;

}


void do_DCI(DCIinfo_t* DCI_info,bits_t* DCI){

	DCI_info->RIV=Calc_RIV(DCI_info->Lrbs,DCI_info-> RBstart,DCI_info-> N_size_BWP);
	FILL_DCI(DCI,DCI_info);

}

void Undo_DCI(DCIinfo_t* DCI_info,bits_t* DCI){

	UNFILL_DCI(DCI,DCI_info);
	int Lrbs;
	int RBstart;
	GET_Lrbs_RBstart(&Lrbs,&RBstart,DCI_info->N_size_BWP,DCI_info->RIV);
	DCI_info->Lrbs=Lrbs;
	DCI_info->RBstart=RBstart;

}


int Calc_Frequencydomain_allocation_size(int N_size_BWP){
	int size=ceil(log2(N_size_BWP*(N_size_BWP+1)/2));
	return size;
}

int Calc_RIV(int Lrbs,int RBstart,int N_size_BWP){
	int RIV;
	if((Lrbs-1)<=floor(N_size_BWP/2)){
		RIV=N_size_BWP*(Lrbs-1)*RBstart;
	}else{
		RIV=N_size_BWP*(N_size_BWP-Lrbs+1)+(N_size_BWP-1-RBstart);
	}
	return RIV;
}

void GET_Lrbs_RBstart(int* Lrbs,int* RBstart,int N_size_BWP,int RIV){
	int L_rbs;
	int RB_start;	
	for(int n=1;n<=N_size_BWP;n++){
		for(int m=0;m<=N_size_BWP-n;m++){
			if((n-1)<=floor(N_size_BWP/2)){
				if(RIV==N_size_BWP*(n-1+m)){
					L_rbs=n;
					RB_start=m;
				}
			}else{
				if(RIV==N_size_BWP*(N_size_BWP-n+1)+(N_size_BWP-1-m)){
				L_rbs=n;
				RB_start=m;	
				}

			}
		}

	}

	*(Lrbs)=L_rbs;
	*(RBstart)=RB_start;

}





int Calc_SLIV(int S,int L){
	int SLIV;
	if((L-1)<=7){
		SLIV=14*(L-1)+S;
	}else{
		SLIV=14*(14-L+1)+(14-1-S);
	}
	return SLIV;
}

int GET_S_L(int* S,int* L,int SLIV_real){
	int SLIV;	
	for(int s=0;s<14;s++){
		for(int l=0;l<14;l++){
			if((l-1)<=7){
				SLIV=14*(l-1)+s;
			}else{
				SLIV=14*(14-l+1)+(14-1-s);
			}
			
			if(SLIV==SLIV_real){
				*(S)=s;
				*(L)=l;
			return 0;
			}
		}
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











