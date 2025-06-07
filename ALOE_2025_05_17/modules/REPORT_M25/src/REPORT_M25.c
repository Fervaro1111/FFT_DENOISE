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

#include "REPORT_M25_interfaces.h"
#include "REPORT_M25_functions.h"
#include "REPORT_M25.h"

#define PRINTPERIOD	10

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="REPORT";
int opMODE=0; 						//O: Normal CTRL operation, 1: Read&Print CTRL file
int debugg=0;

datasourcesink_t MY_DATASOURCESINK;
MAC_t MY_MAC;
channel_t MY_CHANNEL;
uncrc_t MY_UNCRC;

unsigned long int Tslot=0;


print_t T;
int textcolor=15, backcolor=9, noflines;


/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	/* Get control parameters*/
	param_get_int("opMODE", &opMODE);
	param_get_int("debugg", &debugg);

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	sprintf(&T.text[0][0],"SPECIFIC PARAMETERS SETUP: %s\n", mname);
	sprintf(&T.text[1][0]," Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       					NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	sprintf(&T.text[2][0]," opMODE=%d [0-Normal CTRL&Create REPORT file, 1-Read&Print REPORT file]\n", opMODE);
	noflines=3;
	// printTEXT(noflines, textcolor, backgroundcolor, border_char)
	printTEXT(noflines, textcolor, backcolor, '=');
	//---------------------------------------------------------------------------------------

	

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
	int snd_samples=0;
	int rcv_samples0 = get_input_samples(0); /** number of samples at itf 0 buffer */
	int rcv_samples1 = get_input_samples(1); /** number of samples at itf 0 buffer */
	int rcv_samples2 = get_input_samples(2); /** number of samples at itf 0 buffer */
	int rcv_samples3 = get_input_samples(3); /** number of samples at itf 0 buffer */
	int rcv_samples4 = get_input_samples(4); /** number of samples at itf 0 buffer */
	int rcv_samples5 = get_input_samples(5); /** number of samples at itf 0 buffer */
	int rcv_samples6 = get_input_samples(6); /** number of samples at itf 0 buffer */
	int rcv_samples7 = get_input_samples(7); /** number of samples at itf 0 buffer */

	datasourcesink_t *DATASOURCESINK=inp;
	MAC_struc *MAC=in(inp,1);
	channel_t *CHANNEL=in(inp, 2);
	uncrc_t *UNCRC=in(inp, 3);

	// UPDATE REPORTS
	if(rcv_samples0 > 0)memcpy(&MY_DATASOURCESINK, DATASOURCESINK, sizeof(datasourcesink_t));
	if(rcv_samples1 > 0){
		MY_MAC.Coderate = MAC->coderateMCS;
		MY_MAC.MODidx = MAC->QAM_order;
		MY_MAC.IntDec = 0;
		MY_MAC.FFTsize = MAC->FFTsize;
		MY_MAC.NumPRBs = MAC->nPRB;
		MY_MAC.NID1 = MAC->NID1;
		MY_MAC.NID2 = MAC->NID2;
	}
	if(rcv_samples2 > 0)memcpy(&MY_CHANNEL, CHANNEL, sizeof(channel_t));
	if(rcv_samples3 > 0)memcpy(&MY_UNCRC, UNCRC, sizeof(uncrc_t));

	// PRINT REPORT
	if(Tslot == PRINTPERIOD){
		Tslot=0;
		printREPORT(MY_DATASOURCESINK, MY_MAC, MY_CHANNEL, MY_UNCRC);
	}
	Tslot++;
	return snd_samples;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


