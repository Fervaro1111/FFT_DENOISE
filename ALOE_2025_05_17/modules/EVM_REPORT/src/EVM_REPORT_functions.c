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

#include "EVM_REPORT_functions.h"


extern print_t T;


void initCbuff(buffctrl *bufferCtrl, _Complex float *bufferC, int buffersz, int firstpcketsz){

	memset(bufferC, 0, sizeof(_Complex float)*buffersz);
	bufferCtrl->writeIndex = firstpcketsz;
	bufferCtrl->readIndex = firstpcketsz;
	bufferCtrl->buffsize = buffersz;
	bufferCtrl->occuplevel=0;
	bufferCtrl->roomlevel=buffersz;

/*	printf("buffCtrl->writeIndex=%d: \n", bufferCtrl->writeIndex);
	printf("buffCtrl->occuplevel=%d: \n", bufferCtrl->occuplevel);
	printf("buffCtrl->readIndex=%d: \n", bufferCtrl->readIndex);
	printf("bufferCtrl->buffsize=%d: \n", bufferCtrl->buffsize);
	printf("*bufferC=%u: \n", (_Complex float *)bufferC);
*/
}


/**
* void write(buffer* buffer, _Complex float value, int length)
* writes value into the buffer
* @param buffer* buffer
*   pointer to buffer to be used
* @param _Complex float value
*   value to be written in buffer
*/

int writeCbuff(buffctrl *buffer, _Complex float *buffdata, _Complex float *in, int length){
	int i;

	if(buffer->writeIndex >= buffer->readIndex){
		buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
	}
	else{
		buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
	}
	buffer->roomlevel=buffer->buffsize-buffer->occuplevel;
	if(buffer->roomlevel >= length){
		for(i=0; i<length; i++){
			buffdata[buffer->writeIndex]=*(in+i);
			buffer->writeIndex++;
			if(buffer->writeIndex==buffer->buffsize){
				buffer->writeIndex=0;
			}
		}
		if(buffer->writeIndex >= buffer->readIndex){
			buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
		}
		else{
			buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
		}
		buffer->roomlevel=buffer->buffsize-buffer->occuplevel;
		return 1;
	}else{
		printf("Error writeCbuff: Not enough space in buffer\n");
		printf("buffCtrl->writeIndex=%d: \n", buffer->writeIndex);
		printf("buffCtrl->occuplevel=%d: \n", buffer->occuplevel);
		printf("buffCtrl->readIndex=%d: \n", buffer->readIndex);
		printf("bufferCtrl->buffsize=%d: \n", buffer->buffsize);	
		return -1;
	}
}
/**
* void readn(buffer* buffer, int Xn)
* reads specified value from buffer
* @param buffer* buffer
*   pointer to buffer to be read from
* @param int Xn
*   specifies the value to be read from buffer counting backwards from the most recently written value
*   i.e. the most recently writen value can be read with readn(buffer, 0), the value written before that with readn(buffer, 1)
*/
int readCbuff(buffctrl* buffer,_Complex float *buffdata, _Complex float *out, int length){
	int i;

	if(buffer->writeIndex >= buffer->readIndex){
		buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
	}
	else{
		buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
	}
	buffer->roomlevel=buffer->buffsize-buffer->occuplevel;
	if(buffer->occuplevel >= length){
//		printf("readCbuff(): buffer->occuplevel=%d, length=%d, buffer->readIndex=%d\n", buffer->occuplevel, length, buffer->readIndex);
		for(i=0; i<length; i++){
//			if(i==0)printf("*bufferC=%u: \n", (_Complex float *)(buffdata+buffer->readIndex));//printf("buffer->readIndex=%d\n", buffer->readIndex);
			*(out+i) = *(buffdata+buffer->readIndex);
			buffer->readIndex++;

			if(buffer->readIndex==buffer->buffsize){
				buffer->readIndex=0;
			}
		}
		if(buffer->writeIndex >= buffer->readIndex){
			buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
		}
		else{
			buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
		}
		buffer->roomlevel=buffer->buffsize-buffer->occuplevel;
		return 1;
	}else{
/*		modinfo("Error readCbuff: Not enough data in buffer.\n");
		modinfo_msg("buffer->occuplevel=%d, length=%d\n", buffer->occuplevel, length);
		modinfo_msg("buffer->buffsize=%d, buffer->writeIndex=%d, buffer->readIndex=%d\n", buffer->buffsize, buffer->writeIndex, buffer->readIndex);
*/
		printf("Error readCbuff: Not enough data in buffer.\n");
		printf("buffer->occuplevel=%d, length=%d\n", buffer->occuplevel, length);
		printf("buffer->buffsize=%d, buffer->writeIndex=%d, buffer->readIndex=%d\n", buffer->buffsize, buffer->writeIndex, buffer->readIndex);

		return -1;
	}
}





// REF: Doc TS36.100 version10.3.0 Release 10 page 216 Annex F2 and F6
float computeEVM_3GGP_LTE128(_Complex float *inputMeasured, _Complex float *inputReference, int nofsamples, int reset){

	int i, k, M=nofsamples/NOFRBsLTE;		
	double EQMdiff=0.0, EQMsignal=0.0;
	static double EVM=0.0, EVMaver=0.0;
	static int nofIterations=0;

//	printf("EVM=%f, nofiterations=%d, nofsamples=%d\n", EVM, nofIterations, nofsamples);

	if(reset==1){
		EVM=0.0, EVMaver=0.0;
		nofIterations=0;
		printf("computeEVM_3GGP_LTE(): Initializing\n");
	}

/*	if(nofsamples != 1920){
		printf("computeEVM_3GGP_LTE(): Error!!! nofsamples=%d != 1920\n", nofsamples);
//		exit(0);
	}
*/
//	printf("EVM1=%f, nofiterations=%d, nofsamples=%d, M=%d\n", EVM, nofIterations, nofsamples, M);

	// FIRST SLOT
	for(k=0; k<nofsamples/2; k++){
		EQMsignal += pow(cabsf(*(inputReference+k)), 2);
		EQMdiff += pow(cabsf(*(inputMeasured+k) - *(inputReference+k)+1.0e-9), 2);
	}
	if(EQMsignal > 0.0){
		EVM += EQMdiff/EQMsignal;
		EQMdiff=0.0;
		EQMsignal=0.0;
	}else printf("computeEVM_3GGP_LTE128().A: EQMsignal=%f\n", EQMsignal);

	//SECOND SLOT
	for(k=nofsamples/2; k<nofsamples; k++){
		EQMsignal += pow(cabsf(*(inputReference+k)), 2);
		EQMdiff += pow(cabsf(*(inputMeasured+k) - *(inputReference+k)+1.0e-9), 2);
	}
	if(EQMsignal > 0.0){
		EVM += EQMdiff/EQMsignal;
		EQMdiff=0.0;
		EQMsignal=0.0;
	}else {
		printf("computeEVM_3GGP_LTE128().B: EQMsignal=%f\n", EQMsignal);
		for(k=nofsamples/2; k<nofsamples; k++){
			printf("*(inputReference+%d)=%f+%fI\n", k, __real__ *(inputMeasured+k), __imag__ *(inputMeasured+k));
		}
	}
//	printf("EVM0=%f, nofiterations=%d, nofsamples=%d\n", EVM, nofIterations, nofsamples);
	nofIterations++;
	if(nofIterations==10){
		EVMaver=sqrt(EVM/20.0)*100.0;
		if(isnan(EVMaver))printf("ISNAN FOUND!!!, nofsamples=%d, EVM=%f\n", nofsamples, EVM);
		nofIterations=0;
		EVM=0.0;
	}
//	printf(">>>>>>>>>>EVM1=%f, nofiterations=%d, nofsamples=%d\n", EVMaver, nofIterations, nofsamples);

//	printf("computeEVM_3GGP_LTE(): EVM=%f \n", EVMaver);
	return((float)EVMaver);
}



// Return the number of samples to send

void update_CTRL(report_t *reportIN, report_t *reportOUT, float EVM, int REPORT_Tslot, unsigned int TslotDIFF){
	
	int snd_samples=0;

	memcpy(reportOUT, reportIN, sizeof(report_t));
	reportOUT->EVM=EVM;
	reportOUT->EVM_Tslot=REPORT_Tslot;
	reportOUT->TslotDIFF=TslotDIFF;

//		printf("2-EVM_REPORT(): reportOUT->CHANNEL_Tslot=%d\n", reportOUT->CHANNEL_Tslot);
	//printf("EVM_REPORT: EVM=%f, Tslot=%d, TslotDiff=%d\n", reportOUT->EVM, reportOUT->EVM_Tslot, reportOUT->TslotDIFF);

}



#define LINELENGTH	96


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
		printCOLORtext(5, T.forecolor, T.backgroundcolor,  LINELENGTH, &T.text[i][0]);
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
		printf("ERROR!!! data2print length = %d > LINELENGTHN=%d\n", strlen(data2print), LINELENGTH);
		exit(0);
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


//////////////////NORMALIZE DATA
int normalize_vs_var(_Complex float *inout, int length, float varVALUE){

	int i;
	float auxR, auxI, var2=0.0;
	float RATIO;
	// Better value for varVALUE=2.66
	
	if(length==0)return(0);

	for(i=0; i<length; i++){
		auxR=fabs(__real__ inout[i]);
		auxI=fabs(__imag__ inout[i]);
		var2 += auxI*auxI+auxR*auxR;
	}
	var2 = sqrt(var2/(2.0*(float)length));
	RATIO = (1.0)/(varVALUE*var2);
	//if(Tslot%50==0)
//	printf("var2=%f, RATIO=%f\n", var2, RATIO);

	for(i=0; i<length; i++){
		inout[i] = inout[i]*RATIO;
	}
	return(1);
}

