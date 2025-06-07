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

#include "5G_CODE_BLOCK_interfaces.h"
#include "5G_CODE_BLOCK_functions.h"
#include "5G_CODE_BLOCK.h"

extern cbsegm_t CBinfoTX;
extern cbsegm_t CBinfoRX;
extern cbparams_t CBparams;
extern cbstatist_t CBstatist;
extern cbctrl_t CBctrl;
extern MAC_struc MACinfo;
extern MAC_struc MAC_buff[];
extern LDPC_struc_t LDPC_info;


extern crc_t CBcrc_p;
extern crc_t TBcrc24_p;
extern crc_t TBcrc16_p;
extern crc_t *TBcrc_p;


//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="CODE_BLOC";

//Module User Defined Parameters
int TX_mode=0;
int RX_mode=1;
int TX_RX;

//Global Variables
_Complex float bufferA[2048];
float bufferB[2048];

int blocs_out=0;
static char OUTbytes[OUTPUT_MAX_DATA];
static char INbytes[INPUT_MAX_DATA];
int buff_out;

int Ts=0;
int TsMAC=0;
int delay;
int first_sampl_in=0;
int first_MAC_in=-1;

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

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
	int rcv_samples, rcv_MACsamples;
	int snd_samples=0, aux_snd_samples;
	int rcv_bytes, snd_bytes;
	int i, j, k, A, B;

	output_t *output0 = out;
	
	output0=out(out,0);
	
	
	int error=0;

///////////////////////////////////////TX/////////////////////////////
	if(TX_RX==TX_mode){
		// Define input buffer data pointers
		char * datafromCRC = in(inp,0);
		MAC_struc *inputMAC = in(inp,1);

		///////// Check if data received //////
		rcv_bytes = get_input_samples(0);
		///////// Check if MAC data received and save it//////
		rcv_MACsamples = get_input_samples(1);

/*		// Manage MAC buffer
		if (rcv_MACsamples){
			//MAC_buff[TsMAC]=*inputMAC;
			memcpy(&MAC_buff[TsMAC], inputMAC, sizeof(MAC_struc));
			printf("OOOO %s rcv_bytes=%d, rcv_MACsamples=%d, MAC_buff[TsMAC=%d].Ts_MAC=%d\n", 
						mname, rcv_bytes, rcv_MACsamples,  TsMAC, MAC_buff[TsMAC].Ts_MAC);
			//Bufer_MAC2[(Ts%(max_buffer_size-1))]=inputMAC;
			TsMAC++;
			if(TsMAC == MAC_BUFFER)TsMAC=0;
		}	
		if(rcv_bytes == 0)return(0);
		// Check if MAC buffer allow operation
		if(MAC_Buffer_Manager(&CBctrl, &MAC_buff[Ts], Ts))return(0);
*/
		//MAC_Buffer_Manager(&CBctrl, &MAC_buff[Ts], Ts);

		// ALIGN MAC COMMANDS AND PIPELINED EXECUTION
		if(AlignPipelineExec(rcv_MACsamples, rcv_bytes, &CBctrl, inputMAC)==0)return(0);
		//printf("%s            TX continue          \n", mname);
		
		//  TRANSPORT BLOCK
		CBinfoTX.A=rcv_bytes*8;


		//	CALCULATE CODEBLOCS
		error=aloe_ComputeCodeBlockSegmentation(&CBinfoTX, CBparams, CBctrl,&LDPC_info);
		//printf("Bytes after CB segment=%d\n",error);

		//	GENERATE CODE BLOCKS	
		snd_samples=aloe_CreateCodeBlocks(datafromCRC, OUTbytes, &CBinfoTX, CBparams, &CBstatist, CBcrc_p);


		//PUT EACH CODEWORDS INTO A DIFFERENT OUTPUT INTERFACES
		snd_samples=snd_samples/CBinfoTX.nofCBs;
		int NofCodewords=CBinfoTX.nofCBs;
	
		//printf("%s: NofCodewordst=%d\n",mname, NofCodewords);
		if(NofCodewords > NOF_OUTPUT_ITF){
			printf("ERROR!!! %s.work(): NofCodewords=%d > NOF_OUTPUT_ITF=%d\n", mname, NofCodewords, NOF_OUTPUT_ITF);
			printf("You can correct by increasing NOF_OUTPUT_ITF value in %s_interfaces.h\n", mname);
			printf("The current max value for NOF_OUTPUT_ITF = 16\n");
		}

		for(j=0; j<NOF_OUTPUT_ITF; j++){
			if(j<NofCodewords){
				memcpy(out+OUTPUT_MAX_DATA*j,OUTbytes+snd_samples*j,snd_samples);
				aux_snd_samples=snd_samples;
			}else {
				aux_snd_samples=0;
			}
			if(j!=0)set_output_samples(j, aux_snd_samples);
			//if(aux_snd_samples>0)printf("%s output%d, snd_samples=%d\n", mname, j, aux_snd_samples);
		}

	}
//////////////////////////////////////////////////////////////////////



///////////////////////////////////////RX/////////////////////////////
	if(TX_RX==RX_mode){
		char * datafromLDPC = in(inp,0);
		MAC_struc *inputMAC = in(inp,7);
		rcv_bytes = get_input_samples(0);
		int rcv_MACsamples = get_input_samples(7);

		// ALIGN MAC COMMANDS AND PIPELINED EXECUTION
		if(AlignPipelineExecRX(rcv_MACsamples, rcv_bytes, &CBctrl, inputMAC)==0)return(0);

	//	printf("               RX continue         \n");

	
		buff_out=CBctrl.TB_CRC;
		//printf("in UN-CB coderate=%f TB sise =%d, BUF OUT=%d\n",inputMAC->coderateMCS,inputMAC->TB_size_TBCRC,buff_out);

		//CONCATENATE INPUT SAMPLES
		for (int i=0; i<NOF_INPUT_ITF-1;i++){
			rcv_bytes=get_input_samples(i);
			memcpy(INbytes+rcv_bytes*i, datafromLDPC+INPUT_MAX_DATA*i, rcv_bytes);
		}
	

		//	CALCULATE CODEBLOCKS
		CBinfoRX.A=buff_out*8;
		//printf("bits in Decodelock=%d",CBinfoRX.A);
		error=aloe_ComputeCodeBlockSegmentation(&CBinfoRX, CBparams, CBctrl,&LDPC_info);

		//	CHECK CRC OF EACH CB
		CBinfoRX.B=rcv_bytes*8;
		CBinfoRX.A=rcv_bytes*8;
		snd_samples=aloe_5GCheckCRC(INbytes,output0,&CBinfoRX, &CBstatist, CBcrc_p);
		snd_samples=buff_out;


		//	printf("INPUT bytes UNNCB at TS=%d\n",Ts);
		/*for (int r;r<2;r++){	
		for(i=1; i<rcv_samples*in_interfaces; i++){
			//if (i==(snd_samples/2))printf("\n");
			printf("%02x", (int)(input[10*2048*r+i-1]&0xFF));
			//if(i%128==0)printf("\n");
		}
		printf("\n");
		}
		*/

		//printf("%s snd_samples=%d\n", mname, snd_samples);
	}


	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


