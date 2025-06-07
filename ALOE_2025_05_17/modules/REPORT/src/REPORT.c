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

#include "REPORT_interfaces.h"
#include "REPORT_functions.h"
#include "REPORT.h"

#define PRINTPERIOD	10

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="REPORT";
int opMODE=0; 						//O: Normal CTRL operation, 1: Read&Print CTRL file
int debugg=0;
report_t report_SENT, report_RECV, report_STAT;
unsigned long int Tslot=0;
int first=0;

FILE *filePointer, *filePointerSAVE;
char FILENAME[STR_LEN];

print_t T;
int textcolor=15, backcolor=9, noflines;

char APPname[128]; 
APP_t APP; 
TOT_t TOT;

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	/* Get control parameters*/
	param_get_int("opMODE", &opMODE);
	param_get_string("UserName", &report_SENT.username[0]);
	param_get_int("debugg", &debugg);

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	sprintf(&T.text[0][0],"SPECIFIC PARAMETERS SETUP: %s\n", mname);
	sprintf(&T.text[1][0]," Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       					NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	sprintf(&T.text[2][0]," opMODE=%d [0-Normal CTRL&Create REPORT file, 1-Read&Print REPORT file]\n", opMODE);
	sprintf(&T.text[3][0]," Report File=%s.bin \n", &report_SENT.username[0]);
	noflines=4;
	// printTEXT(noflines, textcolor, backgroundcolor, border_char)
	printTEXT(noflines, textcolor, backcolor, '=');
	//---------------------------------------------------------------------------------------

#ifndef STUDENTMODE
	printf("ATTENTION PLEASE!!! NON STUDENT MODE\n");
	printf("	ATTENTION PLEASE!!! NON STUDENT MODE\n");
	printf("		ATTENTION PLEASE!!! NON STUDENT MODE\n");
	printf("			ATTENTION PLEASE!!! NON STUDENT MODE\n");
#endif


	sprintf(FILENAME, "%s.bin", &report_SENT.username[0]);
	
//	filePointer = fopen(&report_SENT.username[0], "wb");
	if(opMODE==0){
		filePointer = fopen(&FILENAME[0], "wb");
	   	if (filePointer == NULL){
			printf("ERROR opening write file: fopen() failed for '%s'\n", &FILENAME[0]);
			exit(0);		
	   	}
		// CAPTURE APP INFO
		captureAPPinfo(filePointer);
	}

	if(opMODE==1){
		filePointer = fopen(&FILENAME[0], "rb");
	//	filePointerSAVE=filePointer+sizeof(APP_t);
	   	if (filePointer == NULL){
			printf("ERROR opening read file: fopen() failed for '%s'\n", &FILENAME[0]);
			exit(0);		
	   	}
		// READ FILE APP INFO
		readAPPinfo(filePointer);
	}




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

	report_t *reportIN=inp;
	report_t *reportOUT=out;

	static int COUNTER=0, LASTsubframe;

	//printf("############################################3 REPORT: Tslot=%d------rcv_samples=%d, snd_samples=%d\n", Tslot, rcv_samples, snd_samples);
	// Perform the CTRL and generate CTRL file
	if(opMODE==0){

		if(rcv_samples>0){
	
//		printf("Tslot=%d-rcv_samples=%d--------------------------------------------------------------------------------------------------\n", Tslot, rcv_samples);

/*			printf("\033[1;31;47m ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\t\033[0m\n");
			printf("\033[1;31;47m    REPORT RECV: Tslot=%d, UserName=%s, ctrlVAL=%d, MAC_ID=%d                            \t\033[0m\n", 
						reportIN->Tslot, &reportIN->username[0], reportIN->ctrl_val, reportIN->MAC_ID);
			printf("\033[1;31;47m    REPORT RECV: FilerName=%s, SNR=%2.1f                               \t\033[0m\n", 
						&reportIN->filter_name[0], reportIN->SNR);
			printf("\033[1;31;47m    REPORT RECV: BLER=%1.6f, BER=%1.6f, Throughtput=%07.2f                                      \t\033[0m\n", 
						reportIN->BLER, reportIN->BER, reportIN->throughput);
			printf("\033[1;31;47m ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\t\033[0m\n");
*/
			//check_received_report(Tslot, &report_SENT, reportIN);
			memcpy(&report_STAT, reportIN, sizeof(report_t));
//			if((COUNTER%PRINTPERIOD==0) && (report_STAT.throughput > 0.0)){
			if((report_STAT.subframe%PRINTPERIOD==0) && (report_STAT.throughput > 0.0) && LASTsubframe!=report_STAT.subframe){
//			if(report_STAT.subframe%PRINTPERIOD==0){
				write_reportBIN(filePointer, reportIN, sizeof(report_t));
				printREPORT(report_STAT);
				LASTsubframe=report_STAT.subframe;
			}
			COUNTER++;
		}

		// First time we need to a first send
		if(first==0){
			first=1;
			rcv_samples=1;
		}
		// If Received report send a new report
		if(rcv_samples>0){
			snd_samples=generate_report(Tslot, &report_SENT, reportOUT);
			if(debugg)printf("%s: Tslot=%d, rcv_samples=%d, snd_samples=%d, UserName=%s, ctrlVAL=%d, MAC_ID=%d\n", 
							mname, Tslot, rcv_samples, snd_samples, &reportOUT->username[0], reportOUT->ctrl_val);
		}


		//printf("-----------------------\n");

	}
	if(opMODE==1){
		read_reportBIN(filePointer, reportIN, sizeof(report_t));
		printREPORT(*reportIN);
/*		printf("ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
		printf("Tslot=%d, UserName=%s, ctrlVAL=%d, MAC_ID=%d                             \n", reportIN->Tslot, &reportIN->username[0], reportIN->ctrl_val, reportIN->MAC_ID);
		printf("FilerName=%s, SNR=%2.1f                       \n", &reportIN->filter_name[0], reportIN->SNR);
		printf("BLER=%1.6f, BER=%1.6f, Throughtput=%5.5f\n", reportIN->BLER, reportIN->BER, reportIN->throughput);
		printf("ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");
*/
	}


	if(snd_samples>0){
//		printf("REPORT SEND: Tslot=%d, UserName=%s, ctrlVAL=%d, MAC_ID=%d\n", reportOUT->Tslot, &reportOUT->username[0], reportOUT->ctrl_val, reportOUT->MAC_ID);
//		printf("REPORT SEND: Tslot=%d------rcv_samples=%d, snd_samples=%d\n", Tslot, rcv_samples, snd_samples);
	}
/*	printf("REPORT SEND: FilerName=%s, SNR=%2.1f, BLER=%1.6f, BER=%1.6f\n", &reportOUT->filter_name[0], reportOUT->SNR, reportOUT->BLER, reportOUT->BER);
	printf("REPORT: Tslot=%d------rcv_samples=%d, snd_samples=%d\n", Tslot, rcv_samples, snd_samples);
*/

/*	if((Tslot%PRINTPERIOD==0) && (report_STAT.throughput > 0.0)){
			printREPORT(report_STAT);
			printf("\033[1;31;47m ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\t\033[0m\n");
			printf("\033[1;31;47m    REPORT RECV: Tslot=%d, subframe=%d, UserName=%s, ctrlVAL=%d, MAC_ID=%d                            \t\033[0m\n", 
						report_STAT.Tslot, report_STAT.subframe, &report_STAT.username[0], report_STAT.ctrl_val, report_STAT.MAC_ID);
			printf("\033[1;31;47m    REPORT RECV: FilerName=%s, SNR=%2.1f                               \t\033[0m\n", 
						&report_STAT.filter_name[0], report_STAT.SNR);
			printf("\033[1;31;47m    REPORT RECV: BLER=%1.6f, BER=%1.6f, Throughtput=%07.2f                                      \t\033[0m\n", 
						report_STAT.BLER, report_STAT.BER, report_STAT.throughput);
			printf("\033[1;31;47m ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\t\033[0m\n");

	}
*/

	Tslot++;
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


