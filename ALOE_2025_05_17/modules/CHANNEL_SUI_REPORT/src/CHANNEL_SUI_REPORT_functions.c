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
#include <string.h>
#include <math.h>

#include "CHANNEL_SUI_REPORT_functions.h"
#include "noise.h"
#include "channelSUI.h"


oParams_t oCHANNEL={CHM_SUI, SUI_DEFAULT, NOISE_SNR, SNRdBs_DEFAULT, 0.0, ADactive, 1.0, SAMPLING_FREQ_DEFAULT, DEBUG_DEFAULT};
extern print_t T;



void init_SUI_CHANNEL(int SUIchannel, float samplingfreq){

	/* INITIALIZATION CHANNEL SUI FUNCTIONS*/
	initSui(SUIchannel, samplingfreq);   /*SUI number & symbol rate*/

}

int run_channel_SUI(_Complex float *in, int nofsamples, _Complex float *out, float SNRdBs){
	int i;
	float real[FLOW_SIZE], imag[FLOW_SIZE];
	FLOW_SIZEvar=nofsamples;

	canal2(in, nofsamples, out, SNRdBs);


	
/*	for(i=0; i<FLOW_SIZEvar; i++){
		real[i]=crealf(*(in+i));
		imag[i]=cimagf(*(in+i));
	}
	printf("A\n");
	canal(real, imag, SNRdBs);

	for(i=0; i<FLOW_SIZEvar; i++){
		*(out+i)=real[i]+imag[i]*I;
	}*/
	return(nofsamples);
}


int CHANNEL(oParams_t *oCHANNEL, _Complex float *in, _Complex float *out, int rcv_samples){
	int snd_samples=0;

	// Channel SUI
	if(oCHANNEL->ChannelModel==CHM_SUI && rcv_samples>0){
		snd_samples=run_channel_SUI(in, rcv_samples, out, (float)oCHANNEL->SNR);
		if(oCHANNEL->NoiseModel==NOISE_SNR){
			//printf("SUI+NOISE\n");
			snd_samples=noise_SNR(out, out, (float)oCHANNEL->SNR, (float)oCHANNEL->gain, rcv_samples);
		}
	}
	// Generate Noise according SNR
	if(oCHANNEL->ChannelModel==CHM_NOISE && rcv_samples>0){
		if(oCHANNEL->NoiseModel==NOISE_SNR)snd_samples=noise_SNR(in, out, (float)oCHANNEL->SNR, (float)oCHANNEL->gain, rcv_samples);
	}
	// AD Active
	if(oCHANNEL->ADmodel==ADactive && rcv_samples==0){
		snd_samples=noise_SNR(in, out, (float)oCHANNEL->SNR, (float)oCHANNEL->gain, 1024);
	}

//	printf("NOISE_SNR: oCHANNEL->NoiseModel=%d, snd_samples=%d, SNR=%f\n", oCHANNEL->NoiseModel, snd_samples, oCHANNEL->SNR);
	return(snd_samples);
}








// Return the number of samples to send

void update_CTRL(report_t *reportIN, report_t *reportOUT, float SNR, int CHANNEL_Tslot, float RotatePhase, int AD_Active){
	
	int snd_samples=0;

//	printf("CHANNEL_REPORT: Tslot=%d, UserName=%s, ctrlVAL=%d, MAC_ID=%d\n", reportIN->Tslot, &reportIN->username[0], reportIN->ctrl_val, reportIN->MAC_ID);
	memcpy(reportOUT, reportIN, sizeof(report_t));
//		printf("1-CHANNEL_REPORT.update_CTRL(): SNR=%3.1f\n", reportOUT->SNR);
	reportOUT->SNR=SNR;
	reportOUT->CHANNEL_Tslot=CHANNEL_Tslot;
//	printf("2-CHANNEL_REPORT.update_CTRL(): reportOUT->CHANNEL_Tslot=%d\n", reportOUT->CHANNEL_Tslot);

	reportOUT->RotatePhase=RotatePhase;
	reportOUT->AD_Active=AD_Active-1;
	if(oCHANNEL.ChannelModel == CHM_NOISE)reportOUT->SUI_Channel = SUI_DEFAULT;
	if(oCHANNEL.ChannelModel == CHM_SUI)reportOUT->SUI_Channel=oCHANNEL.SUIchannel;

//	snd_samples=ceil((float)sizeof(report_t)/(float)sizeof(output_t));
//	if(snd_samples*sizeof(output_t) < sizeof(report_t))snd_samples
//	return();
}


/////////////////////PRINTING
#define LINELENGTH	PRINT_LINELENGTH


void printTEXT(int noflines, int textcolor, int backcolor, char borderchar){

	int i, length;
	char border[LINELENGTH];
	char text2print[LINELENGTH];

	T.borderchar=borderchar;
	T.noflines=noflines;
	T.forecolor=textcolor%256;
	T.backgroundcolor=backcolor%256;


	// Border UP
	memset(border, T.borderchar, LINELENGTH-1);
	border[LINELENGTH-1]='\0';
//	printf("\033[1;%s;%sm%s\033[0m\n", textcolor, backcolor, border);

//	printf("%s\n", backcolor); 
/*	printCOLORtext(0, "208", backcolor,  LINELENGTH, border);
	printCOLORtext(5, "208", "010",  LINELENGTH, &T.text[i][0]);
*/
	printCOLORtext(0, T.forecolor, T.backgroundcolor,  LINELENGTH, border);
	for(i=0; i<T.noflines; i++){
		printCOLORtext(2, T.forecolor, T.backgroundcolor,  LINELENGTH, &T.text[i][0]);
	}
	printCOLORtext(0, T.forecolor, T.backgroundcolor,  LINELENGTH, border);
}


//http://web.archive.org/web/20131009193526/http://bitmote.com/index.php?post/2012/11/19/Using-ANSI-Color-Codes-to-Colorize-Your-Bash-Prompt-on-Linux
//char format[BACKGROUNDLEN]="\33[1m\33[38;5;021;48;5;226m";
//char reset[10]="\033[0m";


void printCOLORtext(int position, int forecolor, char groundcolor,  int end, char *data2print){
	
	int i;		
	char background[LINELENGTH]; 
	int length=(int)LINELENGTH;
	char formatt[LINELENGTH]="\33[1m\33[38;5;021;48;5;226m";
	char reset[10]="\033[0m";

	char fore[4];
	char ground[4];

	my_itoa(groundcolor, ground, 1);
	my_itoa(forecolor, fore, 1);

	if(strlen(data2print)>LINELENGTH-1){
		printf("ERROR!!! data2print length = %d > LINELENGTH=%d\n", strlen(data2print), LINELENGTH);
		//exit(0);
	}

	memset(background, 0x20, LINELENGTH-1);
//	background[BACKGROUNDLEN-1]='\0';
/*	printf("A_");
	for(i=0; i<strlen(formatt)-1; i++)printf("%c", formatt[i]);
	printf("\n");
*/
	memcpy(formatt+11, fore, strlen(fore));
	memcpy(formatt+20, ground, strlen(ground));
/*	printf("B_");
	for(i=0; i<strlen(formatt)-1; i++)printf("%c", formatt[i]);
	printf("\n");
*/
	length=strlen(data2print)-1;
	if(length>(LINELENGTH-1-position))length=(LINELENGTH-1-position);
	memcpy(background+position, data2print, length);
	background[end-1]=NULL;
	printf("%s%s%s\n", formatt, background, reset);
}


void my_itoa(int number, char *str, int nofchars) {
   sprintf(str, "%.3d", number&0xFF); //make the number into string using sprintf function
}
































