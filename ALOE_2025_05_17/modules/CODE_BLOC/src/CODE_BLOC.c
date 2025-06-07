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

#include "CODE_BLOC_interfaces.h"
#include "CODE_BLOC_functions.h"
#include "CODE_BLOC.h"

extern cbsegm_t CBinfoTX;
extern cbsegm_t CBinfoRX;
extern cbparams_t CBparams;
extern cbstatist_t CBstatist;
extern cbctrl_t CBctrl;
extern MAC_struc MACinfo;
extern Bufer_MAC MAC_buff;
extern LDPC_struc_t LDPC_info;


extern crc_t CBcrc_p;
extern crc_t TBcrc24_p;
extern crc_t TBcrc16_p;
extern crc_t *TBcrc_p;


//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="CODE_BLOC";
int run_times=1;
int block_length=111;
char plot_modeIN[STR_LEN]="DEFAULT";
char plot_modeOUT[STR_LEN]="DEFAULT";
float samplingfreqHz=1.0;


//Module User Defined Parameters
float floatp=111.11;
char stringp[STR_LEN]="MY_DEFAULT_INIT";
int TX_mode=0;
int RX_mode=1;
int TX_RX;

//Global Variables
_Complex float bufferA[2048];
float bufferB[2048];

int MAC_buffer_size=9;// 3-> CODE_BLOCK.app   9-> RM_chain_test 5->RM_test
int *Buffer_MAC[9];

int blocs_out=0;
static char OUTbytes[10*1024];
static char INbytes[10*1024];
int buff_out;

int max_in_interfaces=NOF_INPUT_ITF;
int max_out_interfaces=NOF_OUTPUT_ITF;
int Ts=0;
int delay;
int first_sampl_in=0;
int first_MAC_in=-1;

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {


//const char *appname = program_invocation_name;


	printf("Init_5GCodeBlockSegmentCRC\n");
	//param_get_string_name("mname", &CBparams.mname[0]);
	strcpy(mname, GetObjectName());

	//printf("Module Name=%s, %s\n", mname, appname);

//	param_get_int_name("opMODE", &CBparams.opMODE);
//	param_get_int_name("I_MCS_TABLE", &CBparams.I_MCS_TABLE);
//	param_get_int_name("I_MCS", &CBparams.I_MCS);
//	param_get_int_name("TransPrecd", &CBparams.TransPrecd);
//	param_get_int_name("nPRBs", &CBparams.nPRBs);
//	param_get_int_name("v", &CBparams.v);
//	param_get_int_name("debugg", &CBparams.debugg);
	param_get_int("TX_RX_mode", &TX_RX);
	printf("TX_RX_mode = %d\n",TX_RX);
//	param_get_int_name("I_MCS_TABLE", &CBparams.I_MCS_TABLE);
	//CBparams.I_MCS)
	CBparams.TransPrecd=0;
	CBparams.nPRBs=20;
	CBparams.v=1;
	CBparams.debugg=0;



	//printf("%s: opMODE=%d, I_MCS=%d, TransPrecd=%d, nPRBs=%d, NofLayers=%d, debugg=%d\n", 
			//CBparams.mname, CBparams.opMODE, CBparams.I_MCS, CBparams.TransPrecd, CBparams.nPRBs, CBparams.v, CBparams.debugg);

	//extractMCSinfo(CBparams, &CBctrl);

	//printf("%s: Qm=%d, CodeRate=%f\n", mname, CBctrl.Qm, CBctrl.CodeRate);

	/* obtains a handler for fast access to the parameter */
//	gain_id = param_id("gain");
	/* In this case, we are obtaining the parameter value directly */
/*	if (param_get_int_name("block_length", &block_length)) {
		block_length = 0;
	}
*/
	/* use this function to print formatted messages */modinfo_msg("Parameter block_length is %d\n",block_length);

	/* Verify control parameters */
/*	if (block_length > input_max_samples || block_length < 0) {
		moderror_msg("Invalid block length %d\n", block_length);
		return -1;
	}
*/
	/* here you may do some other initialization stuff */

	//CBparams.LDPCgraph=LDPCGRAPH2;


	// Init Code Block CRC
	initCodeBlock();
	initTransportBlockCRC16();
	initTransportBlockCRC24();

	// Init Statistics
	init_CBstatist(&CBstatist);


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
	int rcv_samples;
	int snd_samples=0;
//	int i,k;
	int rcv_bytes, snd_bytes;
	int i, j, k, A, B;
	char *input,*input1,*input2;

	output_t *output0;//*output1,*output2,*output3,*output4,*output5,*output6;
	input = in(inp,0);
	input1 = in(inp,1);
	input2 = in(inp,2);
	/*input3 = in(inp,3);
	input4 = in(inp,4);
	input5 = in(inp,5);
	input6 = in(inp,6);*/
	
	
	
	//input1 = in(inp,1);
	output0=out(out,0);
	//output1=out(out,1);
	//output2=out(out,2);
	rcv_bytes = get_input_samples(0);
	
	
	

	float gain;
	int NumCodeBlocks=0;
	int error=0;
	//int lengthBITS=88, lengthBYTES, res;
	char *pter;
	float R;
	unsigned int crc;
	

///////////////////////////////////////TX/////////////////////////////
	if(TX_RX==TX_mode){

	rcv_samples = get_input_samples(0); 

	MAC_struc *inputMAC = in(inp,1);
	LDPC_struc_t *outputLDPC=out(out,1);
	//MAC_struc new_value=*ctrl;
	//printf("SURT del bufferRX_DEL_CODE_BLOCK_RX=%d \n",buff_out);
	rcv_samples = get_input_samples(0);

	///////// Check if data received//////
	if (rcv_samples == 0){
		MAC_buff[(Ts%(max_buffer_size))]=*inputMAC;
		//Bufer_MAC2[(Ts%(max_buffer_size-1))]=inputMAC;
		Ts++;
		return(0);

	}	

	//////Check delay////////
	
	if (first_sampl_in==0 && rcv_samples!=0){
		delay=Ts-1;//Checkdelay(inputMAC,max_buffer_size,Ts);
		first_sampl_in++;
	}
	
	Buffer_Manager(&CBctrl,inputMAC,Ts,delay);	
	
	//printf("in coderate=%f TB sise =%d\n",inputMAC->coderateMCS,inputMAC->TB_size_TBCRC);


/*
	printf("INPUT bytes aloe_5GCB\n");
	for(i=1; i<rcv_samples+1; i++){
		printf("%02x", (int)(input[i-1]&0xFF));
		//if(i%128==0)printf("\n");
	}
	printf("\n");
*/

	CBinfoTX.A=rcv_bytes*8;

//	CALCULATE CODEBLOCS
	error=aloe_ComputeCodeBlockSegmentation(&CBinfoTX, CBparams, CBctrl,&LDPC_info);
	//printf("Bytes after CB segment=%d\n",error);

//	GENERATE CODE BLOCKS	
	snd_samples=aloe_CreateCodeBlocks(input, OUTbytes, &CBinfoTX, CBparams, &CBstatist, CBcrc_p);
	//int snd_samples_TB=snd_samples;
	
	//snd_samples=CBinfoTX.CBsize/8;
	//printf("CB size CB SEGMENT=%d\n",CBinfoTX.CBsize/8);
/*
printf("OUTPUT bytes aloe_5GCheckCRC\n");
	for(i=1; i<snd_samples+1; i++){
		printf("%02x", (int)(OUTbytes[i-1]&0xFF));
		//if(i%128==0)printf("\n");
}
	printf("\n");
*/



	//SPLIT OUTPUT SAMPLES INTO DIFFERENT INTERFACES
	snd_samples=snd_samples/CBinfoTX.nofCBs;
	int total_interfaces=CBinfoTX.nofCBs;
	
	//printf("total_interfaces_CB out=%d\n",total_interfaces);


	for(i=0;i<total_interfaces;i++){
		memcpy(output0+10*2048*i,OUTbytes+snd_samples*i,snd_samples);
		if (i!=0){		
		set_output_samples(i,snd_samples);
		}
	
	}
	int itf_no_out=max_out_interfaces-i;
	for (j=0;j<itf_no_out;j++){
		set_output_samples(i,0);
		i++;
	}


	//printf("OUTPUT TX at TS=%d\n",Ts);
	/*for(int r=r;r<2;r++){
	
	for(i=1; i<snd_samples+1; i++){
		//if (i==(snd_samples/2))printf("\n");
		printf("%02x", (int)(output0[10*2048*r+i-1]&0xFF));
		//if(i%128==0)printf("\n");
	}
	printf("\n");
	}*/

	


}
//////////////////////////////////////////////////////////////////////



///////////////////////////////////////RX/////////////////////////////
if(TX_RX==RX_mode){

	MAC_struc *inputMAC = in(inp,7);
	//MAC_struc new_value=*ctrl;
	//printf("SURT del bufferRX_DEL_CODE_BLOCK_RX=%d \n",buff_out);
	rcv_samples = get_input_samples(0);
	int rcv_samples_MAC = get_input_samples(7);



	///////// Check if data received//////
	/*if (rcv_samples == 0){
		MAC_buff[(Ts%(max_buffer_size))]=*inputMAC;
		//Bufer_MAC2[(Ts%(max_buffer_size-1))]=inputMAC;

		Ts++;
		return(0);

	}*/	

	if (rcv_samples == 0){
		MAC_buff[(Ts%(max_buffer_size))]=*inputMAC;
		//Bufer_MAC2[(Ts%(max_buffer_size-1))]=inputMAC;
		if(first_MAC_in<0 && rcv_samples_MAC!=0) first_MAC_in=Ts;
		Ts++;
		return(0);

	}



	//////Check delay////////
	
	if (first_sampl_in==0 && rcv_samples!=0){
		delay=Ts-first_MAC_in;//Checkdelay(inputMAC,max_buffer_size,Ts);
		first_sampl_in++;
	}
	
	Buffer_Manager(&CBctrl,inputMAC,Ts,delay);	
	
	
	buff_out=CBctrl.TB_CRC;
//printf("in UN-CB coderate=%f TB sise =%d, BUF OUT=%d\n",inputMAC->coderateMCS,inputMAC->TB_size_TBCRC,buff_out);

	//CONCATENATE INPUT SAMPLES

	int in_interfaces=0;
	for (int i=0; i<max_in_interfaces-1;i++){//el -1 es pq l'ultima ve de la MAC i no del bloc anterior
		if(get_input_samples(i)!=0){
		in_interfaces++;
		}
	}
	
	//printf("RCV samples_Decodebloc itf0=%d  and at itf1=%d\n",rcv_samples,get_input_samples(1));
	//printf("RCV TOTAL samples_Decodebloc=%d\n",rcv_samples*(in_interfaces));


	for(i=0;i<in_interfaces;i++){
		memcpy(INbytes+rcv_samples*i,input+10*2048*i,rcv_samples);
			
	}

	

//	CALCULATE CODEBLOCS
	
	CBinfoRX.A=buff_out*8;
	//printf("bits in Decodelock=%d",CBinfoRX.A);
	error=aloe_ComputeCodeBlockSegmentation(&CBinfoRX, CBparams, CBctrl,&LDPC_info);
//	CHECK CRC OF EACH CB
	CBinfoRX.B=rcv_bytes*8;
	CBinfoRX.A=rcv_bytes*8;
	//printf("RCV samples in DSSS=%d",CBinfoRX.B);
	
	snd_samples=aloe_5GCheckCRC(INbytes,output0,&CBinfoRX, &CBstatist, CBcrc_p);
	snd_samples=buff_out;
	//printf("snd_samples_Decodebloc=%d\n",snd_samples);

	//printf("INPUT bytes UNNCB at TS=%d\n",Ts);
	/*for (int r;r<2;r++){	
	for(i=1; i<rcv_samples*in_interfaces; i++){
		//if (i==(snd_samples/2))printf("\n");
		printf("%02x", (int)(input[10*2048*r+i-1]&0xFF));
		//if(i%128==0)printf("\n");
	}
	printf("\n");
	}
	*/
}
	
	Ts++;
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


