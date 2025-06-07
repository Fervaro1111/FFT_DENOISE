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

#include "DATASOURCESINK_REPORT_interfaces.h"
#include "DATASOURCESINK_REPORT_functions.h"
#include "DATASOURCESINK_REPORT.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="DATASOURCESINK_REPORT";
int block_length=36;
unsigned int Tslot=0;
unsigned int DSS_Tslot;
float gain=1.0;
float reffreq=1.0;
float tonefreq=1024.0;


//Module User Defined Parameters
char myfilename[1024]= "data/rfc793.txt.LONG"; //"../../../modules/data_source_sink/data/rfc793.txt";
char rcvfilename[1024];
int ctrl_flow = 0; 												// 0: No control data flow, 1: With control of data flow
int mode=0; 															// 0: Send File, 1: Sent Data Blocks

//Global Variables
_Complex float bufferA[2048];
float bufferB[2048];
long int myfilelength=0;
#define FILEMAXLENGTH  			50000000
char filebuffer[FILEMAXLENGTH];
char rcvfilebuffer[FILEMAXLENGTH];
unsigned int snd_flag=1;	
int debug=0; //0: NO DEBUG, 1: DEBUG
print_t T;
int textcolor=15, backcolor=208, noflines;

// Identity
#define IDNUMBER_DEFAULT	12345678
int IDnumber=12345678;
char IDfilename[1024]=".NOFILE";
char IDbuffer[256];;
long int IDlength=0;
char IDchar[STR_LEN]="44444444";

// BER
char aux[STR_LEN];
float BER=1.0;
char DATA[64*1024];
unsigned long int RECV_COUNTER=0;
unsigned int RECV_TSLOT=0;
unsigned int RECV_SUBFRAMES=0;
float RECV_THROUGHTPUT=0.0;

// ESTIMATE CODERATE
int FFTsize=256;
int NumDATARBs=4;
float CodeRate=0.0;
int MODindex=2;

// ROUND TRIP DELAY
int RoundTripDelay=0;

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	int i;
	int IDcapt[STR_LEN], numidsread, numidswrite;

//	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("block_length", &block_length);		//Initialized by hand or config file
	param_get_int("ctrl_flow", &ctrl_flow);
	param_get_int("debug", &debug);
	param_get_int("mode", &mode);
	//param_get_string("input_FILE", &myfilename[0]);



	/* Verify control parameters */
	if (block_length > get_input_max_samples()) {
		/*Include the file name and line number when printing*/
		moderror_msg("ERROR: Block length=%d > INPUT_MAX_DATA=%d\n", block_length, INPUT_MAX_DATA);
		moderror("Check your data_source_sink_interfaces.h file\n");
		return -1;
	}
	if(ctrl_flow == 1)snd_flag=0;
	else snd_flag=block_length;


	/*Include the file name and line number when printing*/
	modinfo_msg("Parameter block_length is %d\n",block_length);	//Print message and parameter 
	modinfo("Parameter block_length \n");				//Print only message

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());

/*	printf("\033[1;33;40m\tO--------------------------------------------------------------------------------------------OO\t\033[0m\n");
	printf("\033[1;33;40m\tO    SPECIFIC PARAMETERS SETUP: %s                                   	\t\033[0m\n", mname);
	printf("\033[1;33;40m\tO      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s                    \t\033[0m\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("\033[1;33;40m\tO      block_length=%d, ctrl_flow=%d, snd_flag=%d                                             \t\033[0m\n", block_length, ctrl_flow, snd_flag);
	printf("\033[1;33;40m\tO      mode=%d [0: Send File, 1: Sent Data Blocks], File Name=%s		\t\033[0m\n", mode, myfilename);
	printf("\033[1;33;40m\tO      debug=%d                                                                               \t\033[0m\n", debug);
	printf("\033[1;33;40m\tO--------------------------------------------------------------------------------------------O\t\033[0m\n");
*/

	sprintf(&T.text[0][0],"SPECIFIC PARAMETERS SETUP: %s\n", mname);
	sprintf(&T.text[1][0]," Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       					NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	sprintf(&T.text[2][0]," mode=%d [0: Send File, 1: Sent Data Blocks], File Name=%s \n", mode, myfilename);
	sprintf(&T.text[3][0],"  block_length=%d, ctrl_flow=%d, snd_flag=%d \n", block_length, ctrl_flow, snd_flag);
	noflines=4;
	// printTEXT(noflines, textcolor, backgroundcolor, border_char)
	printTEXT(noflines, textcolor, backcolor, '=');



	// SEND A DATA FILE
	if(mode==0){
		myfilelength=read_file_bin(myfilename, filebuffer, FILEMAXLENGTH);
		//myfilelength=read_file(myfilename, filebuffer, FILEMAXLENGTH);
		sprintf(rcvfilename, "data/rcvDATA.bin");
		//printf("file=%s filelength=%d, rcvfilename=%s\n", myfilename, myfilelength, rcvfilename);
	}
	// SEND A DATA PATTERN
	if(mode==1){
		initFrame2send(DATA, 1024);
	}


	if(FFTsize==256)NumDATARBs=13;

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

#define TSLOTLENGTHs		1024.0/48000.0

int work(input_t *inp, output_t *out) {

	int rcv_samples0 = get_input_samples(0); /** number of samples at itf 0 buffer */
	int rcv_samples1 = get_input_samples(1);
	int rcv_samples2 = get_input_samples(2);
	int snd_samples0=block_length;
	int snd_samples1=sizeof(_Complex float);
	int snd_samples2=0;
	input_t *input = inp;
	input_t *output = out;
	unsigned int *ctrl = in(inp,1);
	_Complex float *output1 = out(out,1);
	report_t *reportIN=in(inp,2);
	report_t *reportOUT=out(out,2);

	int i;
	static long int counter=0;
	static int NUMTslotsRECV=-1;	//STORES THE NUMBER OF Tslot FROM STARTING RECEPTION

	static float IntDEC=0.0;


	int NofSymbolsOFDM=14;
	int NofSamplesAD=1024;

	static float AverNUMTslotsSubf=0, MeasSubTsLen, DevSubTsLen;
	
	float TimeSlot_us;

	static int firstTxTslot=0, firstRxTslot=0;
	static int DSS_snd_samples0;

	static int SUBFRAME_TARGET=10; 
	static int RECV2=0;
	static int Report=0;

	// DETECT REPORTING MODE
	if(rcv_samples2 > 0)Report=1;

	// MEASURE TIMESLOT
	TimeSlot_us=measure_TimeSlot_us();

//	if(rcv_samples2>0)	
//		printf("DATASOURCESINK_REPORT0: Tslot=%d, UserName=%s, ctrlVAL=%d, MAC_ID=%d\n", reportIN->Tslot, &reportIN->username[0], reportIN->ctrl_val, reportIN->MAC_ID);

	if(debug==1){
		printf("DATASOURCESINK_REPORT0: Tslot=%d || %s: rcv_samples0=%d, rcv_samples1=%d\n", Tslot, mname, rcv_samples0, rcv_samples1);
	}

	//Check control interface
	if(ctrl_flow == 1){
		snd_samples0=0;
		if(rcv_samples1 > 0){
			snd_flag=((unsigned int)(*ctrl))&0xFFFF;
			snd_samples0=snd_flag;
			if(debug==1){
				printf("DATASOURCESINK_REPORT0: Tslot=%d || %s: snd_samples0=%d\n", Tslot, mname, snd_samples0);
			}
			measure_RoundTripDelay_inTs(Tslot, SEND);
			//measure_RoundTripDelay_inTs(TimeSlot_us, SEND);
			if(firstTxTslot==0)firstTxTslot=Tslot;
		}
		if(debug==1)printf("Tslot=%d || DATA_SOURCE_SINK:rcv_samplesDATA=%d, rcv_samplesCTRL=%d, CTRL=%d\n", Tslot, rcv_samples0, rcv_samples1, snd_flag);
	}
	// SEND DATA FILE
	if(mode==0){
		if(NUMTslotsRECV>=0)NUMTslotsRECV++;
		//BER=-1.0;
		memcpy(out, (char *)(filebuffer+counter), snd_samples0);
		if(snd_samples0 > 0)DSS_snd_samples0 = snd_samples0;
		counter += snd_samples0;
		// Compare received data
		if(rcv_samples0>0){
			
			if(NUMTslotsRECV==-1)NUMTslotsRECV=1;
			memcpy((char *)(rcvfilebuffer+RECV_COUNTER), inp, rcv_samples0);
			RECV_COUNTER += rcv_samples0;
			// SAVE RECEIVED DATA INTO OUTPUT FILE
			if(RECV_COUNTER >= myfilelength)write_file_bin(rcvfilename, rcvfilebuffer, myfilelength);
			RECV_SUBFRAMES++;
			//RECV_THROUGHTPUT=((float)(RECV_COUNTER*8))/(((float)(RECV_SUBFRAMES*7))*TSLOTLENGTHs);
			
///////////////////////////////////////////////////////// ON WORK
			// COMPUTE THROUGHPUT
			//RECV_THROUGHTPUT=((float)(RECV_COUNTER*8))/(((float)NUMTslotsRECV)*TSLOTLENGTHs);	
			RECV_THROUGHTPUT=compute_Throughtput(RECV_COUNTER);			
			//printf("DATA_SOURCE_SINK_REPORT(): RECV_COUNTER=%d, RECV_SUBFRAMES=%d, RECV_THROUGHTPUT=%f\n", RECV_COUNTER, RECV_SUBFRAMES, RECV_THROUGHTPUT);
			
			// MEASURE ROUND_TRIP_DELAY
			if(firstRxTslot==0)firstRxTslot=Tslot;
			RoundTripDelay=measure_RoundTripDelay_inTs(Tslot, RECV);
			//RoundTripDelay=measure_RoundTripDelay_inTs(TimeSlot_us, RECV);

			// MEASURE DATA_PERIOD & DETECT RT-FAULTS
			computeStatisticsSubframeLength(NUMTslotsRECV, &MeasSubTsLen, &DevSubTsLen);


/////////////////////////////////////////////////////////	ON WORK
			// Estimate N/D 
			IntDEC = ((float)(NofSamplesAD)*((float)NUMTslotsRECV/(float)RECV_SUBFRAMES))/(float)(FFTsize*NofSymbolsOFDM);





			//IntDEC = (float)(NofSamplesAD*NUMTslotsSubf)/(float)(FFTsize*NofSymbolsOFDM);

			// Estimate CodeRate
			//estimateCoderate(rcv_samples0, NumDATARBs);
			estimateCoderate(rcv_samples0, NumDATARBs, &CodeRate, &MODindex);

			if(compare_data(filebuffer, myfilelength, inp, rcv_samples0, &BER)==-1){
				return(-1);
			}
		}
		
	}
	// SEND DATA PATTERN
	if(mode==1){
		BER=-1.0;

		sendDATA(DATA, out, snd_samples0);
		if(compare_data1(DATA, inp, rcv_samples0, &BER)==-1)return(-1);
/*		printf("OUTPUT0\n");
		for(i=0; i<32; i++){
			printf("%x", ((int)*(out+i))&0xFF);
		}
		printf("\n");
*/
	}
	if(debug==1){
		printf("TTslot=%d || %s: snd_samples=%d\n", Tslot, mname, snd_samples0);
		for(i=0; i<32; i++){
			printf("%c", (char) *(out+i));
		}
	}
	// OUTPUT 1
	snd_samples1=0;
	if(rcv_samples0 > 0 && BER >= 10e-6){
		*output1=BER+10e-5*I;
		snd_samples1=1;
/*		printf("Tslot=%d, pout=%p, pOUTPUT=%p, BER=%f, sizeofCOMPLEX=%d, snd_samples=%d\n", 
					Tslot, out, output1, BER, sizeof(_Complex float), snd_samples1);
		printf("OUTPUT1\n");
		printf("BER=%f\n", __real__ *output1);
		for(i=0; i<32; i++){
			printf("%x", ((int)*(output1+i))&0xFF);
		}
		printf("\n");
*/
	}
	set_output_samples(1, snd_samples1);


	// UPDATE REPORT
	if(snd_samples0>0)DSS_Tslot=Tslot;
	if(rcv_samples2>0){		
		//printf("DATASOURCESINK_REPORT: Tslot=%d, UserName=%s, ctrlVAL=%d, MAC_ID=%d\n", reportIN->Tslot, &reportIN->username[0], reportIN->ctrl_val, reportIN->MAC_ID);
		update_CTRL_DATASOURCESINK(reportIN, reportOUT, BER, RECV_THROUGHTPUT, DSS_Tslot, IntDEC, CodeRate, 
							MODindex, MeasSubTsLen, DevSubTsLen, TimeSlot_us, RoundTripDelay, DSS_snd_samples0, &myfilename[0]);
		//snd_samples2=(sizeof(report_t)/sizeof(output_t));
		snd_samples2=ceil((float)sizeof(report_t)/(float)sizeof(output_t));
		//printf("DATASOURCESINK_REPORT: Tslot=%d, ################ Received samples=%d, snd_samples2=%d\n", reportIN->Tslot, rcv_samples2, snd_samples2);
		RECV2=1;
	}
	set_output_samples(2, snd_samples2);
//	printf("3-DATASOURCESINK_REPORT.update_CTRL(): BER=%1.6f\n", reportOUT->BER);
//	printf("DATASOURCESINK_REPORT: Tslot=%d, ################ Received samples=%d, snd_samples2=%d\n", Tslot, rcv_samples2, snd_samples2);
	//if(Tslot%10==0)printf("TimeSlot_us=%f us, RounTripDelay=%d, firstRxTslot=%d, firstTxTslot=%d, diff=%d\n", TimeSlot_us, RoundTripDelay, firstRxTslot, firstTxTslot, firstRxTslot-firstTxTslot);

//	if(RECV_SUBFRAMES == SUBFRAME_TARGET && RECV2==0){
	if(Tslot%10==0 & Report==0){
		printf("#######################################################################\n\n");
		printf("	Tslot=%d, subframe=%d, BER=%.4f, Throughput=%.1f\n", Tslot, RECV_SUBFRAMES, BER, RECV_THROUGHTPUT);
		printf("\n#######################################################################\n");
		SUBFRAME_TARGET +=10;
	}
	// UPDATE TSLOT
	Tslot++;
	return snd_samples0;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


