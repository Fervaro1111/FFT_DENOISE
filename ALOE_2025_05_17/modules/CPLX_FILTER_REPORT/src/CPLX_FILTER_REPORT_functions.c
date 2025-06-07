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

#include "CPLX_FILTER_REPORT_functions.h"

extern print_t T;

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int init_functionA_COMPLEX(_Complex float *input, int length){
	int i;

	printf("INIT MY FUNCTION\n");

	for (i=0;i<length;i++) {
		if(i>20) __real__ input[i]=(float)(i*i);
		__imag__ input[i]=(length-i-1)%(length);
	}
	return(1);
}


/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
void readCPLXfilecoeff(_Complex float *filtercoeff, int *filterlength, char *filtername){

	char str[128];
	float auxf;
	int num, k=0;
	double real=0.0, imag=0.0;
	char creal[32], cimag[32];
	char sign; 

	//Reading the file from .txt file 
    FILE *hFile;
	hFile = fopen(filtername, "r");
     
    if (hFile == NULL){
        printf("\033[1m\033[31mERROR!!!. FILE %s NOT FOUND\033[0m\n", filtername);// Error, file not found
    }
    else{
        fscanf(hFile, "%f", &auxf); 
		*filterlength=(int)auxf;
		while( (num = fscanf( hFile, "%lf %c %lfi\n", &real, &sign, &imag)) > 0 ) {
			if( sign == '-' )imag *= -1;
	    	filtercoeff[k] = (float)real + ((float)imag)*I;
//			printf("real=%1.9f, imag=%1.9f\n", __real__ filtercoeff[k], __imag__ filtercoeff[k]);
			k++;
		}

     	if(k != *filterlength){
			printf("\033[1m\033[31mWARNING!!!!. Please, verify the %s file. The filter length do not match the captured coefficients\033[0m\n", filtername);
         	printf("\033[1m\033[31mfilterlength=%d, number of coeefs read=%d\033[0m\n", *filterlength, k);
		}
    }
//	printf("\033[1m\033[31mfilterlength=%d, number of coeefs read=%d\033[0m\n", *filterlength, k);
    fclose(hFile);
}


void readREALfilecoeff(_Complex float *filtercoeff, int *filterlength, char *filtername){

	char str[128];
	float auxf;
	int num, k=0;
	float real=0.0, imag=0.0;
	char sign; 

	//Reading the file from .txt file 
    FILE *hFile;
	hFile = fopen(filtername, "r");
     
    if (hFile == NULL){
        printf("\033[1m\033[31mERROR!!!. FILE %s NOT FOUND\033[0m\n", filtername);// Error, file not found
    }
    else{
        fscanf(hFile, "%f", &auxf); 
		*filterlength=(int)auxf;
		while( (num = fscanf( hFile, "%f\n", &real)) > 0 ) {
	    	__real__ filtercoeff[k] = real;
				__imag__ filtercoeff[k] = 0.0;
				//__imag__ filtercoeff[k] = real;
//			printf("real=%3.6f, imag=%3.6f\n", __real__ filtercoeff[k], __imag__ filtercoeff[k]);
			k++;
		}
     	if(k != *filterlength){
			printf("\033[1m\033[31mWARNING!!!!. Please, verify the %s file. The filter length do not match the captured coefficients\033[0m\n", filtername);
         	printf("\033[1m\033[31mfilterlength=%d, number of coeefs read=%d\033[0m\n", *filterlength, k);
		}
    }
//	printf("\033[1m\033[31mfilterlength=%d, number of coeefs read=%d\033[0m\n", *filterlength, k);
    fclose(hFile);
}


int bypass(_Complex float *input, int datalength,_Complex float *output){
	int i;
	for(i=0; i<datalength; i++){
		*(output+i)=*(input+i);
	}
	return datalength;
}


int stream_conv_CPLX(_Complex float *ccinput, int datalength,_Complex float *filtercoeff, int filterlength, _Complex float *ccoutput){

	int i, j;
	static int first=0;
	static _Complex float aux[FILTERLENGTH];

	if(first==0){
		for(j=0; j<filterlength; j++){
			aux[j]=0.0+0.0i;
			//printf("FIRST CCONV real=%3.6f, imag=%3.6f\n", __real__ filtercoeff[j], __imag__ filtercoeff[j]);
		}
		first=1;
	}

	for (i=0;i<datalength;i++) {
		for (j=filterlength-2;j>=0;j--) {
			aux[j+1]=aux[j];
		}
		aux[0]=ccinput[i];
		ccoutput[i]=0.0;
		for (j=0;j<filterlength;j++) {
			ccoutput[i]+=aux[j]*filtercoeff[j];
		}
	}
	return datalength;
}

/**
 * @stream_conv_CPLX_DDC(): Perform continous convolution and interpolate by INTER and 
 *                          decimate by DECIM
 * @param.
 * @oParaml: Refers to the struct that containts the module parameters
 * @return: Number of output samples
 */

int stream_conv_CPLX_INT_DEC(	_Complex float *ccinput, 
											int datalength,
											_Complex float *filtercoeff, 
											int filterlength, 
											_Complex float *ccoutput, 
											MODparams_t oParam){

	int i, j, k=0;
	static int first=0;
	static _Complex float aux[FILTERLENGTH];
	int INTER=9, DECIM=3;
	static int n=0, m=0;

	INTER=oParam.Ninterpol;
	DECIM=oParam.Ndecimate;

//	printf("readCPLXfilecoeff(): real=%3.6f, imag=%3.6f\n", __real__ filtercoeff[10], __imag__ filtercoeff[10]);
//	printf("ccinput(): real=%3.6f, imag=%3.6f\n", __real__ ccinput[10], __imag__ ccinput[10]);
//	printf("INT=%d, DEC=%d, datalength=%d, n=%d, m=%d\n", INTER, DEC, datalength, n, m);

	if(first==0){
		for(j=0; j<filterlength; j++){
			aux[j]=0.0+0.0i;
		}
		first=1;
	}

	for (i=0;i<datalength*INTER;i++) {
		if(n==0){
			for (j=filterlength-2;j>=0;j--) {
				aux[j+1]=aux[j];
			}
			aux[0]=ccinput[i/INTER];
		}
		
		if(m==0){
			ccoutput[k]=0.0;
			for (j=0;j<filterlength/INTER;j++) {
				ccoutput[k]+=aux[j]*filtercoeff[j*INTER+n];
			}
			k++;
		}
		n++;							
		if(n==INTER)n=0;		
		m++;
		if(m==DECIM)m=0;
	}
	//printf("datalength*INT/DEC=%d\n", datalength*INTER/DEC);
	return (datalength*INTER/DECIM);
}


void update_CTRL(report_t *reportIN, report_t *reportOUT, char *filename, int CHANNEL_Tslot, float Bw){
//		printf("CPLX_FILTER_REPORT: Tslot=%d, UserName=%s, ctrlVAL=%d, MAC_ID=%d\n", reportIN->Tslot, &reportIN->username[0], reportIN->ctrl_val, reportIN->MAC_ID);
		memcpy(reportOUT, reportIN, sizeof(report_t));
//		printf("1-CPLX_FILTER_REPORT.update_CTRL(): filename=%s\n", filename);
		strcpy(&reportOUT->filter_name[0], filename);
		reportOUT->CPLXFILTER_Tslot=CHANNEL_Tslot;
//		printf("2-CPLX_FILTER_REPORT.update_CTRL(): filename=%s\n", &reportOUT->filter_name[0]);
		reportOUT->BandPassBw=Bw;
}


// BYNARY FILTER FORMAT

void writeBINfilterCoeffFile(_Complex float *COEFFs, int filterlength, char *filtername){
	char FILENAME[STRLENGTH];
	FILE *fp;
	sprintf(FILENAME, "%s.bin", filtername);
	fp=fopen(FILENAME, "wb");
	file_t binflow;
	int writelength, size;

	binflow.numCoeffs=filterlength;
	memcpy(&binflow.filter_name, FILENAME, STRLENGTH);
	memcpy(filtername, FILENAME, STRLENGTH);
	memset(&binflow.COEFFS, 0, (sizeof(_Complex float))*COEFFsz);
	memcpy(&binflow.COEFFS, COEFFs, (sizeof(_Complex float))*filterlength);

//	printf("writeBINfilterCoeffFile(): filterlength=%d, filtername=%s\n", filterlength, FILENAME);
//	printf("writeBINfilterCoeffFile(): filterlength=%d, filtername=%s, re=%f\n", binflow.numCoeffs, binflow.filter_name, __real__ binflow.COEFFS[0]);

	size=sizeof(file_t);
	writelength=fwrite(&binflow,1,size,fp);
	if(size != writelength){
		printf("writeBINfilterCoeffFile(): Writing filename %s: wrote data=%ld, expected=%ld\n", FILENAME, writelength, size);
	}
	fclose(fp);
}


int readBINfilterCoeffFile(_Complex float *COEFFs, int *filterlength, char *filtername){
	long int readlength, size;
	FILE *fp;
	fp=fopen(filtername, "rb");
	file_t binflow;
	int STAT=0;			//0-OK, 1-ERROR READING FILE

//	printf("readBINfilterCoeffFile(): filtername=%s\n", filtername);

	// CHECK IF FILE EXIST
	if(fp==NULL)STAT=1;
	
	// READ FILE
	if(STAT==0){
		size=sizeof(file_t);
		readlength=fread(&binflow,1,size,fp);
		//printf("size=%d, readlength=&d\n", size, readlength);
		if(size != readlength){
			STAT=1;
			printf("STAT=1");
	/*		printf("\033[1;31;47m\t-------------------------------------------------------------------------------------------------------------------\t\033[0m\n");
			printf("\033[1;31;47m\t     readBINfilterCoeffFile(): Reading filename %s: read data=%ld, expected=%ld      \t\033[0m\n", filtername, readlength, size);
			printf("\033[1;31;47m\t     ERROR!!! FILTER COEFFICIENTS FILENAME %s NOT FOUND OR NOT A .bin FILE           \t\033[0m\n", filtername);
			printf("\033[1;31;47m\t     PLEASE, CHECK THAT SUCH FILE EXIST. 			                                                              \t\033[0m\n");
			printf("\033[1;31;47m\t-------------------------------------------------------------------------------------------------------------------\t\033[0m\n");
	*/
/*			sprintf(&T.text[0][0],"  readBINfilterCoeffFile(): \n");
			sprintf(&T.text[1][0],"  Reading filename %s: read data=%ld, expected=%ld\n", filtername, readlength, size);
			sprintf(&T.text[2][0],"  ERROR!!! FILTER FILENAME %s NOT FOUND OR NOT A .bin FILE \n", filtername);
			sprintf(&T.text[3][0],"  PLEASE, CHECK THAT SUCH FILE EXIST. \n");
			// printTEXT(noflines, textcolor, backgroundcolor, border_char)
			printTEXT(4, 196, 15, '|');
*/
		}else{
			*filterlength=binflow.numCoeffs;
			memcpy(filtername, &binflow.filter_name, STRLENGTH);
			memcpy(COEFFs, &binflow.COEFFS, (sizeof(_Complex float))*(*filterlength));
		
		}

		//printf("readBINfilterCoeffFile():  readlength=%d, filterlength=%d, filtername=%s, re=%f\n", readlength, *filterlength, filtername, __real__ binflow.COEFFS[0]);
		//printf("readBINfilterCoeffFile():  readlength=%d, filterlength=%d, filtername=%s, re=%f\n", readlength, *filterlength, filtername, __real__ COEFFs[0]);
		fclose(fp);
	}
	if(STAT==1){
		// PRINT IF ERROR
		sprintf(&T.text[0][0]," \n");
		sprintf(&T.text[1][0],"  READING FILENAME %s: read data=%ld, expected=%ld\n", filtername, readlength, size);
		sprintf(&T.text[2][0],"  ERROR!!! FILTER FILENAME %s NOT FOUND OR NOT A .bin FILE \n", filtername);
		sprintf(&T.text[3][0],"  PLEASE, CHECK THAT SUCH FILE EXIST. \n");
		// printTEXT(noflines, textcolor, backgroundcolor, border_char)
		printTEXT(4, 196, 15, '/');
	}


	return(STAT);
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

