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

#include "SCRAMBLING_F_C_functions.h"

#define Nc 1600

SCRinfo_t SCR_info;
MAC_struc MACinfo;
Bufer_MAC MAC_buff;


void Buffer_Manager(SCRinfo_t* SCR_info,MAC_struc* inputMAC,int Ts,int delay){

	MAC_struc out_buff;

	int buff_in_position=Ts%max_buffer_size;
	//if(Ts%max_buffer_size==0)buff_in_position=0;
	int buff_out_position=buff_in_position-delay;
	MAC_buff[buff_in_position]=*inputMAC;

	if (buff_out_position<0){
		buff_out_position=buff_out_position+(max_buffer_size);
	}
	 
	/*printf("Ts=%d and delay=%d\n", Ts,delay);
	for(int i=0;i<30;i++){
		printf("CELLID=%d and RNTI=%d at position=%d\n", MAC_buff[i].CELLID,MAC_buff[i].RNTI,i);

	}*/




	out_buff=MAC_buff[buff_out_position];

	//printf("TS_MAC out from SCR %d \n",out_buff.Ts_MAC);
	//printf("--------------NEW FEATURE--------- CELLID=%d Ts=%d   coderate=%f\n" ,out_buff.CELLID,out_buff.Ts_MAC,out_buff.coderateMCS);

	//Advance_buffer(Bufffer,inputMAC,max_buffer_size);
	extract_MAC_info(SCR_info,&out_buff);


}

void extract_MAC_info(SCRinfo_t* SCR_info, MAC_struc *MACinfo){
	SCR_info->q=MACinfo->q;
	SCR_info->CELLID=MACinfo->CELLID;
	SCR_info->RNTI=MACinfo->RNTI;


}




void UL_create_scrambling_sequence_BITS(int RNTI, int CELLID, int sequencelength, char *scrambseq,int q){

	int mgs_A=0;	//TODO Make dynamic


	int n;
	int x1[MAXSEQLENGTH*2], x2[MAXSEQLENGTH*2];
	int Cinit;	

	int nRNTI=RNTI;
	// q=0 or 1 depending on the number of layers. CHECK definition!
	int Cell_id=CELLID;


	if(mgs_A==1){
		int nRAPID=0; //NSE A QUE ES!!! El 0 esta malament! MIRAR A 5.1.3A of [11, TS 38.321] pero no s'utilitza de moment.
		Cinit=nRNTI*pow(2,16)+nRAPID*pow(2,10)+Cell_id;
	}else{
		//138.211->6.3.1.1
		Cinit=nRNTI*pow(2,15)+Cell_id;
	
	}

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



void UL_create_scrambling_sequence_FLOATS(int RNTI, int CELLID, int sequencelength, float *scrambseq,int q){

	int mgs_A=0;	//TODO Make dynamic

	int aux;
	int n;
	int x1[MAXSEQLENGTH*2], x2[MAXSEQLENGTH*2];
	int Cinit;	

	int nRNTI=RNTI;
	// q=0 or 1 depending on the number of layers. CHECK definition!
	int Cell_id=CELLID;


	if(mgs_A==1){
		int nRAPID=0; //NSE A QUE ES!!! El 0 esta malament! MIRAR A 5.1.3A of [11, TS 38.321] pero no s'utilitza de moment.
		Cinit=nRNTI*pow(2,16)+nRAPID*pow(2,10)+Cell_id;
	}else{
		//138.211->6.3.1.1
		Cinit=nRNTI*pow(2,15)+Cell_id;
	
	}

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





void create_scrambling_sequence_BITS(int RNTI, int CELLID, int sequencelength, char *scrambseq,int q){
	//from 38.211-> 7.3.1.1
	
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

void create_scrambling_sequence_BITS_PDCCH(int CELLID, int sequencelength, char *scrambseq,int RNTI){
	//from 38.211-> 7.3.2.3
	
	int n;
	int x1[MAXSEQLENGTH*2], x2[MAXSEQLENGTH*2];
	int aux;


	int nRNTI=RNTI;
	// q=0 or 1 depending on the number of layers. CHECK definition!
	int Cell_id=CELLID;

	int Cinit=(int)remainder(nRNTI*pow(2,16)+Cell_id,pow(2,31));
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


void create_scrambling_sequence_FLOATS_PDCCH(int CELLID, int sequencelength, float *scrambseq,int nRNTI){

	
	int n;
	int x1[MAXSEQLENGTH*2], x2[MAXSEQLENGTH*2];
	int aux;


	//int nRNTI=RNTI;
	// q=0 or 1 depending on the number of layers. CHECK definition!
	int Cell_id=CELLID;

	int Cinit=(int)remainder(nRNTI*pow(2,16)+Cell_id,pow(2,31));
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



void create_scrambling_sequence_FLOATS(int RNTI, int CELLID, int sequencelength, float *scrambseq, int q){

	
	int n;
	int x1[MAXSEQLENGTH*2], x2[MAXSEQLENGTH*2];
	int aux;

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
	for(i=0; i<rcv_samples; i++){
		*(out+i) = (*(in+i) ^ *(scrambseq+i)) & 0x01;
		//*(out+i)=(*(in+i) + *(scrambseq+i)) % 2;
	}
}


//
/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */


void descrambling(float *in, float *out, int rcv_samples, float *scrambseq){
	int i;
	for(i=0; i<rcv_samples; i++){
		*(out+i) = (*(in+i)) * (*(scrambseq+i));
		//*(out+i) = ((*(in+i)) - (*(scrambseq+i))) %2;	
}
	//printf("PRINTEO I:%i",i);
}

