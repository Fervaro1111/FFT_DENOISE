/* 
 * Copyright (c) 2012.
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

#include <complex.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <phal_sw_api.h>
#include "skeleton.h"
#include "params.h"

#include "RATE_MATCH_interfaces.h"
#include "RATE_MATCH_functions.h"
#include "RATE_MATCH.h"


extern MAC_struc MACinfo;
extern infoRM_t RM_info;
extern Bufer_MAC MAC_buff;


//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="RATE_MATCH";



//Module User Defined Parameters
int TX_RX;
#define TX 0
#define RX 1

char stringp[STR_LEN]="MY_DEFAULT_INIT";

//Global Variables

//int rv=0; //Redundanci version
//int mod_order=2;
//int num_layers=1;
int G;
int Er[50];


int max_in_interfaces=NOF_INPUT_ITF;
int max_out_interfaces=NOF_OUTPUT_ITF;

int first_sampl_in=0;
int delay;
int first_MAC_in=-1;



int Ts=0;

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	/* Get control parameters*/
	

	param_get_int("TX_RX_mode", &TX_RX);


	/* Verify control parameters */





	
	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
("O--------------------------------------------------------------------------------------------O\n");

	/* do some other initialization stuff */
	
	

	return 0;
}



/**
 * @brief Function documentation
 *
 * @param inp Input interface buffers. Data from other interfaces is stacked in the buffer.
 * Use in(ptr,idx) to access the address. To obtain the number of received samples use the function
 * int get_input_samples(int idx) where idx is the interface index.
 *
 * @param out Input interface buffers. Data to other interfaces must be stacked in the buffer.
 * Use out(ptr,idx) to access the address.
 *
 * @return On success, returns a non-negative number indicating the output
 * samples that should be transmitted through all output interface. To specify a different length
 * for certain interface, use the function set_output_samples(int idx, int len)
 * On error returns -1.
 *
 * @code
 * 	input_t *first_interface = inp;
	input_t *second_interface = in(inp,1);
	output_t *first_output_interface = out;
	output_t *second_output_interface = out(out,1);
 *
 */
int work(input_t *inp, output_t *out) {
	int rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
	int snd_samples=0;
	int i,k,k0,j;
	
	int buff_in_position;

///////////////// TX ////////////////////////
if(TX_RX==TX){
	char *input;
	char *output;
	input=in(inp,0);
	output=out(out,0);

	MAC_struc *inputMAC;
	inputMAC=in(inp,7);
	


	///////// Check if data received//////
	if (rcv_samples == 0){
		MAC_buff[(Ts%(max_buffer_size))]=*inputMAC;
		//Bufer_MAC2[(Ts%(max_buffer_size-1))]=inputMAC;
		Ts++;
		return(0);
	}	
	//printf("%s IN: rcv_samples=%d\n", mname, rcv_samples);
	//////Check delay////////
	
	if (first_sampl_in==0 && rcv_samples!=0){
		delay=Ts-1;//Checkdelay(inputMAC,max_buffer_size,Ts);
		first_sampl_in++;
	}
	//printf("-----------RMM-----------Ts=%d,delay=%d------------------\n",Ts,delay);


	Buffer_Manager(&RM_info,inputMAC,Ts,delay);
	//printf("RATE MATCH TS=%d\n",Ts);
	//printf("rv=%d, TB_sizeCRC=%d, mod_qam=%d, coderate=%f, REs_DMRS=%d,Free RE=%d /n" , RM_info.rv, RM_info.TB_TBCRC, RM_info.mod_order, RM_info.coderate, RM_info.REs_DMRS,RM_info.free_RE);

	///////CALC RATE MATCH PARAMETERS ////////////
	 
	G=Calc_G(RM_info);
	//printf("G FROM CALC=%d\n",G);
	//printf("RATE MATCH G=%d\n",G);

	Calc_CB(&RM_info);
	int in_interfaces=RM_info.C;
	k0=Calc_RM_params(in_interfaces,G,&RM_info,&Er);
	//printf("RM k0=%d\n",k0);

	/////////////////DO RM //////////////
	rate_MATCH_calc(Er,input,output,rcv_samples,k0,RM_info);
	//rate_MATCH_calc_punct(Er,input,output,rcv_samples,k0,RM_info);
	//printf("BIT output bits \n");
	
	snd_samples=G;
	//printf("%s IN: snd_samples=%d\n", mname, snd_samples);
	


}
///////////////// TX ////////////////////////77


///////////////// RX ////////////////////////77
if (TX_RX==RX){
	
	float *input;
	float *outputF;
	input=in(inp,0);
	outputF=out(out,0);


	MAC_struc *inputMAC;
	inputMAC=in(inp,1);
	int rcv_samples = get_input_samples(0)/sizeof(float);
	int rcv_samples_MAC = get_input_samples(1);


	///////// Check if data received//////
	if (rcv_samples == 0){
		MAC_buff[(Ts%(max_buffer_size))]=*inputMAC;
		if(first_MAC_in<0 && rcv_samples_MAC!=0) first_MAC_in=Ts;
		Ts++;
		return(0);

	}	

//	printf("%s IN: rcv_samples=%d\n", mname, rcv_samples);
	
	//////Check delay////////
	
	if (first_sampl_in==0 && rcv_samples!=0){
		delay=Ts-first_MAC_in;//Checkdelay(inputMAC,max_buffer_size,Ts);
		first_sampl_in++;
	}
	

	Buffer_Manager(&RM_info,inputMAC,Ts,delay);
/*	printf("UN RATE MATCH TS=%d\n",Ts-delay-1);
	printf("rv=%d, TB_sizeCRC=%d, mod_qam=%d, coderate=%f, REs_DMRS=%d,Free RE=%d /n" , 
			RM_info.rv, RM_info.TB_TBCRC, RM_info.mod_order, RM_info.coderate, RM_info.REs_DMRS,RM_info.free_RE);
*/

	/////// CALC RATE MATCH PARAMETERS ////////////
	Calc_CB(&RM_info);
	G=Calc_G(RM_info);
	
	//printf("G FROM CALC=%d\n",G);

	k0=UNDO_Calc_RM_params(G,&RM_info,&Er);
	//printf("RM k0=%d\n",k0);
	//printf("UN RATE MATCH TS=%d, G=%d,k0=%d\n",Ts,G,k0);

	/////// DO RATE MATCH ////////////
	int samp_in_RM_input=Undo_rate_MATCH_calc(Er,input,outputF,rcv_samples,k0,RM_info);
	//int samp_in_RM_input=Undo_rate_MATCH_calc_punc(Er,input,outputF,rcv_samples,k0,RM_info);


	///////SET OUTPUT INTERFACES////////////
	snd_samples=samp_in_RM_input;
	//printf("snd_samples =%d\n",snd_samples);
	//int total_interfaces=RM_info.C;
	printf("%s total_interfaces_URM_out=%d\n",mname, RM_info.C);	

	for(int i=1;i<RM_info.C;i++){
		if(i < NOF_OUTPUT_ITF)set_output_samples(i,snd_samples*sizeof(float));
		else {
			printf("%s total_interfaces_URM_out=%d, NOF_OUTPUT_ITF=%d, itf=%d\n",mname, RM_info.C, NOF_OUTPUT_ITF, i);	
		}
	}


	//printf("%s IN: snd_samples=%d\n", mname, snd_samples);
	snd_samples=snd_samples*sizeof(float);
	
	
}
	

///////////////// RX ////////////////////////77


	Ts++;
	return snd_samples;

}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


