/* 
 * Copyright (c) 2012, Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
 * This file is part of ALOE++ (http://flexnets.upc.edu/)
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


//#include "utils.h"
#include "CHANNEL_NOISE_REPORT_functions.h"



oParams_t oCHANNEL={CHM_NOISE, NOISE_SNR, 0.0, NOADactive, 1.0, SAMPLING_FREQ_DEFAULT, DEBUG_DEFAULT};
extern print_t T;


float get_variance(float snr_db,float scale) {
	return sqrt(pow(10,-snr_db/10)*scale);
}

float aver_power(_Complex float *in, int length){
	int i;
	_Complex float result=0.0+0.0i;
	float aver_power=0.0;

	for(i=0; i<length; i++){
		aver_power += __real__ (*(in+i)*conjf((*(in+i))));
//		aver_power += (__real__ (*(in+i)))*(__real__ (*(in+i))) + (__imag__ (*(in+i)))*(__imag__ (*(in+i)));
	}
	aver_power=aver_power/(float)length;
	return(aver_power);
}

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
float rand_gauss (void) {
  float v1,v2,s;

  do {
    v1 = 2.0 * ((float) rand()/RAND_MAX) - 1;
    v2 = 2.0 * ((float) rand()/RAND_MAX) - 1;

    s = v1*v1 + v2*v2;
  } while ( s >= 1.0 );

  if (s == 0.0)
    return 0.0;
  else
    return (v1*sqrt(-2.0 * log(s) / s));
}

void gen_noise_c(_Complex float *x, float variance, int len) {
	int i;
	for (i=0;i<len;i++) {
		__real__ x[i] = rand_gauss();
		__imag__ x[i] = rand_gauss();
		x[i] *= variance;
	}
}



float MYaver_power(_Complex float *in, int length){
	int i;
	_Complex float result=0.0+0.0i;
	float aver_power=0.0;

	for(i=0; i<length; i++){
		aver_power += __real__ (*(in+i)*conjf((*(in+i))));
	}
	aver_power=aver_power/(float)length;
	return(aver_power);
}

float MYrand_gauss (void) {

	double r1=(double)rand() / RAND_MAX;
	double r2=(double)rand() / RAND_MAX;
	double sqrt_r1=sqrt(-2*log(r1));
	double theta=2*MMPI*r2;
	double noise=sqrt_r1*cos(theta);

	return (float)noise;
}

int MYnoise_SNRe(_Complex float *in, _Complex float *out, float SNR, int rcv_samples){
	int i;	
	float power_in, power_noise=2.0;
	_Complex float noise;
	float variance=1.0; //0.707106781;
	float gain=1.0;

	// Calculate average power of input data
	float aver_power=0.0;
	power_in=MYaver_power(in, rcv_samples/10);

	// Correct noise level according expected SNR. SNR=Psignal/(Gain*Pnoise)
	gain=power_in/(SNR*power_noise);
	gain=sqrt(gain);

	// Add Noise
	for(i=0; i<rcv_samples; i++){
		noise=(MYrand_gauss()+I*MYrand_gauss());
		*(out+i)=*(in+i)+gain*noise;
	}

	return(rcv_samples);
}


int CHANNEL(oParams_t *oCHANNEL, _Complex float *in, _Complex float *out, int rcv_samples){
	int snd_samples=0;
	char mname[128]="CHANNEL_NOISE_REPORT";

	if(oCHANNEL->ChannelModel==CHM_BYPASS && rcv_samples>0){
printf("%s: B2.1 SNR=%f, gain=%f, rcv_samples=%d\n", "BYPASS", (float)oCHANNEL->SNR, rcv_samples);
		memcpy(out, in, sizeof(_Complex float)*rcv_samples);
	}

	// Generate Noise according SNR
	if(oCHANNEL->ChannelModel==CHM_NOISE && rcv_samples>0){
//printf("%s: B2.1 SNR=%f, gain=%f, rcv_samples=%d\n", "ADD NOISE", (float)oCHANNEL->SNR, rcv_samples);
		if(oCHANNEL->NoiseModel==NOISE_SNR)snd_samples=MYnoise_SNRe(in, out, (float)oCHANNEL->SNR, rcv_samples);
	}
	// AD Active
	if(oCHANNEL->ADmodel==ADactive && rcv_samples==0){
printf("%s: B2.2 SNR=%f, gain=%f, rcv_samples=%d\n", "AD Active", (float)oCHANNEL->SNR, rcv_samples);
		snd_samples=MYnoise_SNRe(in, out, (float)oCHANNEL->SNR, 1024);
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
	//reportOUT->CHANNEL_Tslot=CHANNEL_Tslot;
//	printf("2-CHANNEL_REPORT.update_CTRL(): reportOUT->CHANNEL_Tslot=%d\n", reportOUT->CHANNEL_Tslot);

//	reportOUT->RotatePhase=RotatePhase;

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

