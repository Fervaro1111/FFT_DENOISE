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

#include "channelSUI.h"
#include "CHANNEL_SUI_REPORT_interfaces.h"
#include "CHANNEL_SUI_REPORT_functions.h"
#include "CHANNEL_SUI_REPORT.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="CHANNEL_SUI_REPORT";
unsigned int Tslot=0;
print_t T;
int textcolor=15, backcolor=30, noflines;

//Module User Defined Parameters
unsigned int CHANNEL_Tslot;




// Global variables
extern oParams_t oCHANNEL;
float SNRdBs=0;
		

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("ChannelModel", &oCHANNEL.ChannelModel);
	param_get_int("NoiseModel", &oCHANNEL.NoiseModel);
	param_get_float("gain", &oCHANNEL.gain);
	param_get_float("SNR", &oCHANNEL.SNR);
	param_get_float("NoiseVariance", &oCHANNEL.NoiseVariance);
	param_get_int("debugg", &oCHANNEL.debugg);
	param_get_int("AD_Active", &oCHANNEL.ADmodel);

	param_get_int("SUIchannel", &oCHANNEL.SUIchannel);
//	param_get_float("samplingfreq", &oCHANNEL.samplingfreq);

	oCHANNEL.SNR=oCHANNEL.SNR-10.0;


	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());

	if((oCHANNEL.ChannelModel==CHM_SUI) && ((oCHANNEL.SUIchannel < 1 ) || (oCHANNEL.SUIchannel > 6 ))){
		printf("\n\tmname.initialize(): ERROR!!! If SUI Channel Mode Selected, Channel SUI number must be between 1-6 included\n");
		printf("\tPlease, modify the .params file. ...Now stopping\n\n");
		exit(0);
	}


	sprintf(&T.text[0][0],"SPECIFIC PARAMETERS SETUP: %s\n", mname);
	sprintf(&T.text[1][0]," Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       					NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	sprintf(&T.text[2][0]," ChannelModel=%d: [1) Noise only; 2) SUI+Noise]\n", oCHANNEL.ChannelModel);
	sprintf(&T.text[3][0]," NoiseModel=%d: [1) Calculate Noise Power; 2) Variance], gain=%1.1f, SNR=%3.1fdBs\n", oCHANNEL.NoiseModel, oCHANNEL.gain, oCHANNEL.SNR);
	sprintf(&T.text[4][0]," SUIchannel=SUI%d [SUI1 to SUI6], samplingfreq=%1.1f\n", oCHANNEL.SUIchannel, oCHANNEL.samplingfreq);
	sprintf(&T.text[5][0]," debugg=%d: [1) No debugg; 2) Debugg active] \n", oCHANNEL.debugg);
	sprintf(&T.text[6][0]," AD_Active=%d: [1) No AD active; 2) AD active] \n",oCHANNEL.ADmodel);
	noflines=7;
	// printTEXT(noflines, textcolor, backgroundcolor, border_char)
	printTEXT(noflines, textcolor, backcolor, '=');

	/* INITIALIZATION NOISE FUNCTIONS*/
	oCHANNEL.NoiseVariance=get_variance(oCHANNEL.SNR,1.0);
	SNRdBs=oCHANNEL.SNR;
	//SNR=(pow(10,-SNR/10));
	oCHANNEL.SNR=(pow(10,oCHANNEL.SNR/10.0));

//	printf("oCHANNEL.SNR=%f\n", oCHANNEL.SNR);

	/*Channel SUI Inicialization*/
//	initSui(3, 27700);   /*SUI number & symbol rate*/
//	initSui(3, 30720000);   /*SUI number & symbol rate*/
	if(oCHANNEL.ChannelModel==CHM_NOISE)oCHANNEL.SUIchannel=SUI_DEFAULT;
	if(oCHANNEL.ChannelModel==CHM_SUI)init_SUI_CHANNEL(oCHANNEL.SUIchannel, oCHANNEL.samplingfreq);
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
	int rcv_samples0 = get_input_samples(0); /** number of samples at itf 0 buffer */
	int rcv_samples1 = get_input_samples(1);
	int snd_samples0=0;
	int snd_samples1=0;
	input_t *input = in(inp,0);
	output_t *output = out(out,0);

	report_t *reportIN=in(inp,1);
	report_t *reportOUT=out(out,1);

	output_t *graph = out(out,2);

	int i;
	//printf("%s: rcv_samples=%d \n", mname, rcv_samples0);
	if(rcv_samples0 > INPUT_MAX_DATA)printf("%s: ERROR!!! rcv_samples=%d > INPUT_MAX_DATA\n", mname, rcv_samples0);
	// If AD Active rcv_samples=1024
	if(oCHANNEL.ADmodel==ADactive)rcv_samples0=1024;


	if(oCHANNEL.debugg==DEBUG_ACTIVE){
		rcv_samples0=1024;
		for(i=0; i<rcv_samples0; i++){
			*(input+i)=0.0+0.0*I;
		}
		*(input+10)=1.0+0.0*I;
	}

	// ADD CHANNEL BEHAVIOUR
	snd_samples0=CHANNEL(&oCHANNEL, input, output, rcv_samples0);
	//memcpy(output, input, sizeof(output_t)*rcv_samples0);


	// UPDATE REPORT
	if(snd_samples0>0)CHANNEL_Tslot=Tslot;
	if(rcv_samples1>0){		
		update_CTRL(reportIN, reportOUT, SNRdBs, CHANNEL_Tslot, 0.0, oCHANNEL.ADmodel);
		snd_samples1=ceil((float)sizeof(report_t)/(float)sizeof(output_t));
	}

	// Indicate the number of samples at interface 1
	set_output_samples(1, snd_samples1);


	if(snd_samples0 > OUTPUT_MAX_DATA)printf("%s: ERROR!!! snd_samples=%d > OUTPUT_MAX_DATA\n", mname, snd_samples0);
//	printf("%s: snd_samples=%d \n", mname, snd_samples0);
	Tslot++;
	// Indicate the number of samples at output 0 with return value
	return snd_samples0;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}


