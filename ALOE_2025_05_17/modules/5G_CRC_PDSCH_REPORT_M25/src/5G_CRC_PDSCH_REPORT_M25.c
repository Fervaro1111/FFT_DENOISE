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

#include "5G_CRC_PDSCH_REPORT_M25_interfaces.h"
#include "5G_CRC_PDSCH_REPORT_M25_functions.h"
#include "5G_CRC_PDSCH_REPORT_M25.h"

extern MAC_struc MACinfo;

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="5G_CRC_PDSCH_REPORT_M25";
int run_times=1;
int block_length=111;
char plot_modeIN[STR_LEN]="DEFAULT";
char plot_modeOUT[STR_LEN]="DEFAULT";
float samplingfreqHz=1.0;

//Module User Defined Parameters
#define ADD__CRC	0
#define CHECKCRC	1
int mode=ADD__CRC; //0: ADD CRC; 1: CHECK_CRC

//Global Variables
char bufferA[2048];
char bufferB[2048];
int numdata=11;
//float bufferB[2048];
int crc_length = 24;
unsigned int crc_poly = LTE_CRC24A; //0x1864CFB;
unsigned int crc_word;
crc_t crc_p;

int numblocksrcv=0;
int numblocksNOK=0;
float BLER=0.0;
int Tslot=0;
int CRC_Tslot;

print_t T;
int textcolor=15, backcolor=17, noflines;

#define cWIMAX 	0
#define cLTE	1
#define c5G		2

int CRANtype=cWIMAX; 

#define c5G_MAXCB_KCBg2 3840
#define c5G_MAXCB_KCBg1 8448

#define cWIMAX_MAXCB_KCBg1 1152
#define cWIMAX_MAXCB_KCBg2 1152

int Kcb=cWIMAX_MAXCB_KCBg2;


/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {
	int i;

	strcpy(mname, GetObjectName());
	//printf("INITIALIZE oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("mode", &mode);
	param_get_int("crc_length", &crc_length);
	param_get_int("crc_poly", &crc_poly);
	param_get_int("CRANtype", &CRANtype);

	/* Verify control parameters */

	/* Print Module Init Parameters */
	
	sprintf(&T.text[0][0],"SPECIFIC PARAMETERS SETUP: %s\n", mname);
	sprintf(&T.text[1][0]," Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       					NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	sprintf(&T.text[2][0]," mode=%d [0: ADD CRC; 1: CHECK CRC]\n", mode);
	sprintf(&T.text[3][0]," crc_length=%d, crc_poly=0x%X\n", crc_length, crc_poly);
	sprintf(&T.text[4][0]," CRANtype=%d = 0:WIMAX, 1:LTE, 2:5G \n", CRANtype);
	noflines=5;
	// printTEXT(noflines, textcolor, backgroundcolor, border_char)
	printTEXT(noflines, textcolor, backcolor, '=');


	/* do some other initialization stuff */
	//Initialize CRC params and tables
	if(crc_init(&crc_p, crc_poly, crc_length))return(-1);
	if(crc_set_init( &crc_p, 0x00000000))return(-1);
	if(crc_set_xor( &crc_p, 0x00000000))return(-1);

	for (i=0;i<256;i++) {
		bufferA[i] = (char)(i&0xFF); //(char)(rand()&0xFF);
	}
//	printf("initialize(): crc_p->data0=%p\n", (void *)&(crc_p.data0));

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
	int snd_samples = 0;
	int rcv_samples = get_input_samples(0); /** number of samples at itf 0 buffer */
	//int rcv_samples1 = get_input_samples(1);
	int snd_samples0=0;
	int snd_samples1=0;
	input_t *input = inp;
	input_t *output = out;

	uncrc_t *reportOUT=out(out,1);
	
	int i,k;
	unsigned int crc=77;
	char a, b, c;
	int crcLengthBytes=(crc_p.order)/8;
	_Complex float *aux; 

	static int PrintReport=1;
	
	/*MAC_struc *inputMAC;	
	inputMAC=in(inp,1);
	printf ("inputMAC->TB_size_NO_CRC =%d",inputMAC->TB_size_NO_CRC);
	*/
	
	
	// Check if data received
	if (rcv_samples == 0){
		return(0);
	}//else printf("Tslot=%d: %s: rcv_samples=%d\n", Tslot, mname, rcv_samples);
	// Check if data exceed maximum expected data

	//if (rcv_samples1 > 0)PrintReport=0;

	if(CRANtype == 2) Kcb=c5G_MAXCB_KCBg2;
	if(CRANtype == 0) Kcb=cWIMAX_MAXCB_KCBg2;

	//ADD CRC
	if(mode==ADD__CRC){
			
		if(rcv_samples>(Kcb-16)/8){
			crc_poly=0x1864CFB;
			crc_length=24;
			crcLengthBytes=3;
		}else{
			crc_poly=0x11021;
			crc_length=16;
			crcLengthBytes=2;
		}
		if(crc_init(&crc_p, crc_poly, crc_length))return(-1);
		if(crc_set_init( &crc_p, 0x00000000))return(-1);
		if(crc_set_xor( &crc_p, 0x00000000))return(-1);
	
		//printf("RCVsamples at CRC=%i\n",rcv_samples);
		//printf("CRC length= %i\n",crc_length);
		//printf("rcv_samples at CRC= %i\n",rcv_samples);

		add_CRC(inp, rcv_samples, &crc_p);
		snd_samples=rcv_samples+crcLengthBytes;
		memcpy(out, inp, sizeof(char)*snd_samples);


		//printf("input CRC at TS=%d\n",Tslot);
	
	/*for(i=1; i<rcv_samples+1; i++){
		//if (i==(snd_samples/2))printf("\n");
		printf("%02x", (int)(inp[i-1]&0xFF));
		//if(i%128==0)printf("\n");
	}
	printf("\n");
*/
	


/*		printf("CRC: Tslot=%d, p_inp=%p, rcv_samples=%d\n", Tslot, inp, rcv_samples);
		printf("INPUTCRC\n");
		aux=inp;
		printf("BER=%f\n", __real__ *aux);
		for(i=0; i<32; i++){
			//printf("%c", (char) *(inp+i));
			printf("%x", ((int)*(inp+i))&0xFF);
		}
		printf("\n");
*/
	}
	//CHECK CRC
	if(mode==CHECKCRC){
	//printf("%s-rcv_samples=%i, Tslot=%d, rcv_samples1=%i \n",mname, rcv_samples, Tslot, rcv_samples1 );
		if(rcv_samples>Kcb/8){
			crc_poly=0x1864CFB;
			crc_length=24;
			crcLengthBytes=3;
		}else{
			crc_poly=0x11021;
			crc_length=16;
			crcLengthBytes=2;
		}
	
		if(crc_init(&crc_p, crc_poly, crc_length))return(-1);
		if(crc_set_init( &crc_p, 0x00000000))return(-1);
		if(crc_set_xor( &crc_p, 0x00000000))return(-1);
	
		numblocksrcv++;
		crc=check_CRC(inp, rcv_samples, &crc_p);
		if(crc!=0)numblocksNOK++;
		snd_samples=rcv_samples-crcLengthBytes;
		memcpy(out, inp, sizeof(char)*snd_samples);
		//if(numblocksrcv%10==0){
		if(Tslot%10==0){
			if(numblocksrcv>0)BLER=((float)numblocksNOK)/((float)numblocksrcv);
			else BLER=1.0;
			if(PrintReport==1){
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				printf("!!! %s: BLER=%1.5f, BlocksReceived=%d, BlocksNOK=%d\n", mname, BLER, numblocksrcv, numblocksNOK);
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			}
		}


		/*
		printf("OUTPUT UNCRC at TS=%d\n",Tslot);
	
	for(i=1; i<snd_samples+1; i++){
		//if (i==(snd_samples/2))printf("\n");
		printf("%02x", (int)(out[i-1]&0xFF));
		//if(i%128==0)printf("\n");
	}
	printf("\n");
*/


		// UPDATE REPORT
		if(numblocksrcv>0)BLER=((float)numblocksNOK)/((float)numblocksrcv);
		else BLER=1.0;
		//printf("UNCRC rcv_samples1=%d, BLER=%1.6f\n", rcv_samples1, BLER);	
		update_CTRL_CRC(reportOUT, BLER);
		snd_samples1=ceil((float)sizeof(uncrc_t)/(float)sizeof(output_t));
	}



	// Indicate the number of samples at at each interface
	set_output_samples(1, snd_samples1);
	//printf("!!!!!!! %s: snd_samples1=%d\n", mname, snd_samples1);



//	printf("Tslot=%d || %s: rcv_samples=%d, snd_samples=%d, crc=%u\n", Tslot, mname, rcv_samples, snd_samples, crc);
	// Indicate the number of samples at autput 0 with return value
	Tslot++;
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


