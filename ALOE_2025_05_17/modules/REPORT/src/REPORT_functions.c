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
#include <time.h> 
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>

#include "REPORT_functions.h"

extern print_t T;

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */

int randomINT(int Tslot){

	int randval;
	time_t t;

  	srand((unsigned) Tslot);
	randval=rand()%333;

	return(randval);
}

int check_username(char *A, char *B){

	int OKcheck=-1;
	//printf("A=%s, B=%s\n", A, B);
	OKcheck=strcmp(A, B); //Result should be 0 to be OK
	if(OKcheck==0)return(0);
	else return(1);
}

#define MAXTSLOTS	30

void check_received_report(int Tslot, report_t *report_SENT, report_t *report_IN){

	int OKcheck=0;
	// Compare UserNames
//	OKcheck+=check_username(report_SENT->username, report_IN->username); //Result should be 0 to be OK
//	if(OKcheck != 0)printf("check_username():ERROR!!! OKcheck=%d\n", OKcheck);
	// Compare ctrl_val
//	if(report_SENT->ctrl_val != report_IN->ctrl_val)OKcheck+=1;
/*	if(OKcheck != 0){
		printf("report_SENT->ctrl_val=%d != report_IN->ctrl_val=%d:ERROR!!! OKcheck=%d\n", report_SENT->ctrl_val, report_IN->ctrl_val, OKcheck);
		exit(0);
	}
*/	
	// Check Nof timeslots
	if(report_IN->Tslot+MAXTSLOTS < report_SENT->Tslot){
		printf("report_SENT->Tslot=%d, report_IN->Tslot=%d\n", report_SENT->Tslot, report_IN->Tslot);
	}

}

//sizeof(report_t)
void write_reportBIN(FILE *filePointer, report_t *report_IN, int datasize){

	fwrite(report_IN, datasize, 1, filePointer);
}

void read_reportBIN(FILE *filePointer, report_t *report_IN, int datasize){

	fread( report_IN, datasize, 1, filePointer);

}

int generate_report(int Tslot, report_t *report_IN, report_t *report_OUT){


	int length, nofchars;
	//printf("A\n");
	memset(report_OUT, 0, sizeof(report_t));
	//printf("B\n");
	length=strlen(&report_IN->username[0]);
	memcpy(&report_OUT->username[0], &report_IN->username[0], length);
	//printf("C\n");
	report_OUT->Tslot=Tslot;
	//printf("D\n");
	report_OUT->ctrl_val=randomINT(Tslot);
	report_IN->ctrl_val=report_OUT->ctrl_val;
	//printf("E\n");
//	printf("generate_report()Tslot=%d: %s, ctrl_val=%d, lengthUserName=%d\n", report_OUT->Tslot, &report_OUT->username[0], report_OUT->ctrl_val, length);
	
	return(sizeof(report_t));
}
/*
#define TEXTLENGTH	96
#define TEXTLENGTH2	640

void printREPORT(report_t report_STAT){
	int i;					
	char text0[TEXTLENGTH]="-----------------------------------------------------------------------------------------------";
	char text1[TEXTLENGTH];
	char text2[TEXTLENGTH];
	char text3[TEXTLENGTH];
	char text[TEXTLENGTH2];
	char text2print[TEXTLENGTH];

//	int strl0, strl1, strl2, strl3;
	
	
	memset(text2print, 0x7E, TEXTLENGTH-1);
	text2print[TEXTLENGTH-1]='\0';
//	memcpy(text2print, text0, strlen(text0));
	printf("\033[1;32;40m%s\033[0m\n", text2print);

	memset(text2print, 0x20, TEXTLENGTH-1);
	text2print[TEXTLENGTH-1]='\0';
//	memcpy(text2print, text0, strlen(text0));
	printf("\033[1;32;40m%s\033[0m\n", text2print);

	memset(text2print, 0x20, TEXTLENGTH-1);
	text2print[TEXTLENGTH-1]='\0';
	sprintf(text1, "REPORT Tslot=%d, CHANTslot=%d, CPLXFILTTslot=%d", 
				report_STAT.Tslot, report_STAT.CHANNEL_Tslot, report_STAT.CPLXFILTER_Tslot);
	memcpy(text2print+2, text1, strlen(text1));
	printf("\033[1;32;40m%s\033[0m\n", text2print);


	memset(text2print, 0x20, TEXTLENGTH-1);
	text2print[TEXTLENGTH-1]='\0';
	sprintf(text1, "REPORT EVMTslot=%d, CRCTslot=%d, DSSTslot=%d",  
				report_STAT.EVM_Tslot, report_STAT.CRC_Tslot, report_STAT.DSS_Tslot);
	memcpy(text2print+2, text1, strlen(text1));
	printf("\033[1;32;40m%s\033[0m\n", text2print);



	memset(text2print, 0x20, TEXTLENGTH-1);
	text2print[TEXTLENGTH-1]='\0';
	sprintf(text1, "REPORT RECV: subframe=%d, UserName=%s, ctrlVAL=%d, MAC_ID=%d", 
				report_STAT.subframe, &report_STAT.username[0], report_STAT.ctrl_val, report_STAT.MAC_ID);
	memcpy(text2print+2, text1, strlen(text1));
	printf("\033[1;32;40m%s\033[0m\n", text2print);

	memset(text2print, 0x20, TEXTLENGTH-1);
	memset(text1, 0x20, TEXTLENGTH-1);
	text2print[TEXTLENGTH-1]='\0';
	sprintf(text1, "REPORT RECV: FilterName=%s, SNR=%2.1f", &report_STAT.filter_name[0], report_STAT.SNR);
	memcpy(text2print+2, text1, strlen(text1));
	printf("\033[1;32;40m%s\033[0m\n", text2print);



	memset(text2print, 0x20, TEXTLENGTH-1);
	memset(text1, 0x20, TEXTLENGTH-1);
	text2print[TEXTLENGTH-1]='\0';
	sprintf(text1, "REPORT RECV: EVM=%1.6f, TslotDIFF=%d", 
				report_STAT.EVM, report_STAT.TslotDIFF);
	memcpy(text2print+2, text1, strlen(text1));
	printf("\033[1;32;40m%s\033[0m\n", text2print);



	memset(text2print, 0x20, TEXTLENGTH-1);
	memset(text1, 0x20, TEXTLENGTH-1);
	text2print[TEXTLENGTH-1]='\0';
	sprintf(text1, "REPORT RECV: BLER=%1.6f, BER=%1.6f, Throughtput=%07.2f", 
				report_STAT.BLER, report_STAT.BER, report_STAT.throughput);
	memcpy(text2print+2, text1, strlen(text1));
	printf("\033[1;32;40m%s\033[0m\n", text2print);

	memset(text2print, 0x20, TEXTLENGTH-1);
	text2print[TEXTLENGTH-1]='\0';
//	memcpy(text2print, text0, strlen(text0));
	printf("\033[1;32;40m%s\033[0m\n", text2print);

	memset(text2print, 0x7E, TEXTLENGTH-1);
	text2print[TEXTLENGTH-1]='\0';
//	memcpy(text2print, text0, strlen(text0));
	printf("\033[1;32;40m%s\033[0m\n", text2print);


}

*/


void printREPORT(report_t report_STAT){

	float MeasSNR;
   	time_t curtime;
   	struct tm *loc_time;
	
	//Getting current time of system
   	curtime = time (NULL);
 
   	// Converting current time to local time
   	loc_time = localtime (&curtime);

	if(report_STAT.EVM==0.0)MeasSNR=100.0;
	else MeasSNR=-(7.26+20*log10(((double)report_STAT.EVM)/100.0));

	report_STAT.MAC_ID=4;


	sprintf(&T.text[0][0], " "); 
/*	sprintf(&T.text[1][0], "REPORT DSS: UpDownLink=%d [0: Downlink; 1: Uplink], subframe=%-5d\n", 
							report_STAT.UpDownLink, report_STAT.subframe);
*/
	sprintf(&T.text[1][0], "REPORT DSS: TimeSlot=%2.2fus (Min=21333.3us), DSSRoundTripDelay=%d\n", 
							report_STAT.TimeSlot_us, report_STAT.DSSRoundTripDelay);
/*	sprintf(&T.text[2][0], "REPORT DSS: snd_samples=%d, DataPeriod=%2.2f, RealTimeIndex=%2.2f [0.0: OK; Max = 0.5]\n", 
							report_STAT.DSSsndSamples, report_STAT.MeasSubframeTsLength, report_STAT.StandDevSubframeLength);
*/

	//sprintf(&T.text[3][0], "Tslot Difference EVM_REPORT inputs: TslotDIFF=%d\n", report_STAT.TslotDIFF); 

/*	sprintf(&T.text[4][0], "REPORT CPLX_FILTER: FilterName=%s\n", &report_STAT.filter_name[0]);
*/
	sprintf(&T.text[2][0], "REPORT:     SUI=%d [0: NOISE ONLY], SNR=%2.1f dBs, AD_Active=%d [0: NonActive, 1: Active]\n", 
						report_STAT.SUI_Channel, report_STAT.SNR, report_STAT.AD_Active);

	sprintf(&T.text[3][0], "REPORT:     RotatePhase=%2.1f, BandPassBw=%2.1fHz\n", 
						report_STAT.RotatePhase, report_STAT.BandPassBw);

	sprintf(&T.text[4][0], "REPORT EVM: DataPeriod=%2.2f, TxrcvSamples=%d, EVMRoundTripDelay=%d\n", 
								report_STAT.EVM_TXDataPeriod, report_STAT.EVM_TXsideReceivedSamples, report_STAT.EVMRoundTripDelay);  
	sprintf(&T.text[5][0], "REPORT EVM: EVM=%3.2f%, Measured SNR=%2.1f dBs, FFTsize=%d\n", report_STAT.EVM, MeasSNR, report_STAT.FFTsize);
	sprintf(&T.text[6][0], "REPORT:     DUC Interpolation/Decimation=%1.2f\n", report_STAT.IntDec);
/*	sprintf(&T.text[10][0], "REPORT:     CodeRate Options\n");

	sprintf(&T.text[11][0], "REPORT:     CodeRate[BPSK]=%2.2f, CodeRate[4QAM]=%2.2f, CodeRate[16QAM]=%2.2f,\n", 
											report_STAT.Coderate, report_STAT.Coderate/2.0, report_STAT.Coderate/4.0);
	sprintf(&T.text[12][0], "REPORT:     CodeRate[64QAM]=%2.2f, CodeRate[256QAM]=%2.2f, CodeRate[1024QAM]=%2.2f,\n", 
											report_STAT.Coderate/6.0, report_STAT.Coderate/8.0, report_STAT.Coderate/10.0);
*/	


	sprintf(&T.text[7][0], "REPORT: \n");

	sprintf(&T.text[8][0], "REPORT: subframe=%d, FileSentName=%s\n", 
				report_STAT.subframe, &report_STAT.file_name[0]);

	sprintf(&T.text[9][0], "REPORT: BLER=%1.6f, BER=%1.6f, Throughtput=%07.2f\n", report_STAT.BLER, report_STAT.BER, report_STAT.throughput);



	sprintf(&T.text[10][0], " %s", asctime (loc_time)); 
	sprintf(&T.text[11][0], " "); 
	// printTEXT(noflines, textcolor, backgroundcolor, border_char)
	//if(report_STAT.subframe < 200)printTEXT(18, 15, 202, '~');
	//else printTEXT(18, 16, 46, '~');

	if(report_STAT.subframe < 100)printTEXT(12, 16, 214, '~');
	else printTEXT(12, 214, 16, '~');

// COLORS at https://stackabuse.com/how-to-print-colored-text-in-python/

//printf("%s", asctime (loc_time));

}

/*
void printREPORT(report_t report_STAT){

	float MeasSNR;
   	time_t curtime;
   	struct tm *loc_time;
	
	//Getting current time of system
   	curtime = time (NULL);
 
   	// Converting current time to local time
   	loc_time = localtime (&curtime);

	if(report_STAT.EVM==0.0)MeasSNR=100.0;
	else MeasSNR=-(7.26+20*log10(((double)report_STAT.EVM)/100.0));

	report_STAT.MAC_ID=4;


	sprintf(&T.text[0][0], " "); 
	sprintf(&T.text[1][0], "REPORT Tslot=%d, CHANTslot=%d, CPLXFILTTslot=%d\n", 
				report_STAT.Tslot, report_STAT.CHANNEL_Tslot, report_STAT.CPLXFILTER_Tslot);
	sprintf(&T.text[2][0], "REPORT EVMTslot=%d, CRCTslot=%d, DSSTslot=%d\n",  
				report_STAT.EVM_Tslot, report_STAT.CRC_Tslot, report_STAT.DSS_Tslot);
	//sprintf(&T.text[3][0], "Tslot Difference EVM_REPORT inputs: TslotDIFF=%d\n", report_STAT.TslotDIFF); 
	sprintf(&T.text[3][0], "EVM_REPORT inputs: EVMRoundTripDelay=%d\n", report_STAT.EVMRoundTripDelay);   
	sprintf(&T.text[4][0], "REPORT: subframe=%d, UserName=%s, ctrlVAL=%d, MAC_ID=%d\n", 
				report_STAT.subframe, &report_STAT.username[0], report_STAT.ctrl_val, report_STAT.MAC_ID);
	sprintf(&T.text[5][0], "REPORT: FilterName=%s, SNR=%2.1f\n", &report_STAT.filter_name[0], report_STAT.SNR);

	sprintf(&T.text[6][0], "REPORT: SUI=%d, AD_Active=%d, RotatePhase=%2.1f, BandPassBw=%2.1fHz\n", report_STAT.SUI_Channel, report_STAT.AD_Active, report_STAT.RotatePhase, report_STAT.BandPassBw);

	sprintf(&T.text[7][0], "REPORT: EVM=%3.2f%, Measured SNR=%2.1f dBs\n", report_STAT.EVM, MeasSNR);
	sprintf(&T.text[8][0], "REPORT: CodeRate=%2.2f, MOD_Index=%d, IntDec=%1.2f\n", report_STAT.Coderate, report_STAT.MODidx, report_STAT.IntDec);

	sprintf(&T.text[9][0], "REPORT: dataPeriod=%2.2f, RealTimeIndicator=%2.2f [0.0: OK]\n", report_STAT.MeasSubframeTsLength, report_STAT.StandDevSubframeLength);

	sprintf(&T.text[10][0], "REPORT: BLER=%1.6f, BER=%1.6f, Throughtput=%07.2f\n", report_STAT.BLER, report_STAT.BER, report_STAT.throughput);



	sprintf(&T.text[11][0], " %s", asctime (loc_time)); 
	sprintf(&T.text[12][0], " "); 
	// printTEXT(noflines, textcolor, backgroundcolor, border_char)
	if(report_STAT.subframe < 100)printTEXT(13, 10, 196, '~');
	else printTEXT(13, 10, 0, '~');

//printf("%s", asctime (loc_time));

}
*/



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

///////////////////////////////////////////////////////////////////////////////


void readAPPname(char *filename){		/*AGBAPRIL20*/

	FILE *fp;
	int k, z;



	//printf("writeAPPname()\n");

	fp = fopen("APPname.bin", "rb");
	if (fp == NULL){
		printf("ERROR opening read file: fopen() failed for '%s'\n", "APPname.bin");
		exit(0);		
	}

	fread(filename, 128, 1, fp);
	//printf("READ FUNC: %s\n", filename);
	fclose(fp);


}


void readAPPfileContens(char *filename, APP_t *app){

	FILE *fp;
	int k, z;
	char APPname[128];
	char COMMAND[128];


//	printf("READ FUNC: %s\n", filename);
	sprintf(APPname, "%s.app", filename);

	
	memcpy(&app->name[0], APPname, strlen(APPname));

	fp = fopen(APPname, "rb");
	if (fp == NULL){
		printf("ERROR opening read file: fopen() failed for '%s'\n", "APPname.bin");
		exit(0);		
	}

	k=fread(&app->contens, 1, sizeof(APP_t), fp);
	app->numofbytes=k;

	fclose(fp);
	//DELETE FILE
//	sprintf(COMMAND, "rm %s", "APPname.bin");
	strcpy(COMMAND, "rm APPname.bin");
	system(COMMAND);

}



void captureAPPinfo(FILE *filePointer){

	char APPname[128]; 
//	char ASCtime[128];
	int length;

	APP_t APP;
	TOT_t TOT;

   	time_t curtime;
   	struct tm *loc_time;
	
	//Getting current time of system
   	curtime = time (NULL);
   	// Converting current time to local time
   	loc_time = localtime (&curtime);

//	printf("RW0:%s", asctime (loc_time)); 
	length=strlen(asctime(loc_time));
//	printf("length=%d\n", length);
//	sprintf(&APP.localtime[0], "%s", asctime(loc_time)); 
	memcpy(&APP.localtime[0], asctime(loc_time), length-1);
	APP.localtime[length-1]='\0';
//	printf("RW1:%s\n", &APP.localtime[0]);


	readAPPname(APPname);
//	printf("REPORT %s\n", APPname);
	// Read the contens of APP
	readAPPfileContens(APPname, &APP);
//	printf("REPORT2 %s\n", &APP.name[0]);
//	printf("READ FUNC: %d\n", APP.numofbytes);

	memcpy(&TOT.APP, &APP, sizeof(APP_t));
//	pchar=&APP.contens[0];
//	for(i=0; i<256; i++)printf("%c", *(pchar+i));
	write_reportBIN(filePointer, &TOT, sizeof(TOT_t));

}



void readAPPinfo(FILE *filePointer){

	TOT_t TOT;
	int i;
	char *pchar;

	read_reportBIN(filePointer, &TOT, sizeof(TOT));

//	printf("READING %s\n", &TOT.APP.name[0]);
	printAPPinfoT(TOT);

#ifndef STUDENTMODE
	pchar=&TOT.APP.contens[0];
	for(i=0; i<TOT.APP.numofbytes; i++)printf("%c", *(pchar+i));
	printf("\n");
#endif
}


void printAPPinfoT(TOT_t TOT){


	sprintf(&T.text[0][0], " "); 
	sprintf(&T.text[1][0], "APPname=%s, APPsize=%d ", 
				&TOT.APP.name[0], TOT.APP.numofbytes);
	sprintf(&T.text[2][0], "ExecTime=%s ", &TOT.APP.localtime[0]); 

	// printTEXT(noflines, textcolor, backgroundcolor, border_char)
	printTEXT(3, 15, 6, '~');

//printf("%s", asctime (loc_time));

}


void estimateSTATparameters(report_t *report_STAT){
	
	int numCarriers;
	int MappingExpectedSamples;
	float GlobalCodeRate;
	int ModIdx; 		// 1: BPSK, 2:4QAM, 4: 16QAM, 6:64QAM, 8:256QAM: 10:1024QAM

	if(report_STAT->UpDownLink == UPLINK){
		if(report_STAT->EVM_TXsideReceivedSamples == 128*14)numCarriers=72-24;
		if(report_STAT->EVM_TXsideReceivedSamples == 256*14)numCarriers=180-24;
		if(report_STAT->EVM_TXsideReceivedSamples == 512*14)numCarriers=300-24;
		if(report_STAT->EVM_TXsideReceivedSamples == 1024*14)numCarriers=600-24;
		if(report_STAT->EVM_TXsideReceivedSamples == 1536*14)numCarriers=900-24;
		if(report_STAT->EVM_TXsideReceivedSamples == 2048*14)numCarriers=1200-24;
		MappingExpectedSamples=numCarriers*12;		//DMRS REMOVE 2 OFDM SYMBOLS
	}

	if(report_STAT->UpDownLink == DOWNLINK){
		if(report_STAT->EVM_TXsideReceivedSamples == 128*14)numCarriers=72;
		if(report_STAT->EVM_TXsideReceivedSamples == 256*14)numCarriers=180;
		if(report_STAT->EVM_TXsideReceivedSamples == 512*14)numCarriers=300;
		if(report_STAT->EVM_TXsideReceivedSamples == 1024*14)numCarriers=600;
		if(report_STAT->EVM_TXsideReceivedSamples == 1536*14)numCarriers=900;
		if(report_STAT->EVM_TXsideReceivedSamples == 2048*14)numCarriers=1200;
		MappingExpectedSamples=numCarriers*13;		//PSS ONLY REMOVE ONE OF EACH 5
	}

//	printf("DSSsndSamples_BITS=%d, MappingExpectedSamples=%d\n", (report_STAT->DSSsndSamples+3)*8, MappingExpectedSamples);


	GlobalCodeRate = ((float)(report_STAT->DSSsndSamples+3)*8.0)/(float)MappingExpectedSamples;
	report_STAT->Coderate = GlobalCodeRate;

	report_STAT->IntDec = (report_STAT->EVM_TXDataPeriod*1024.0)/report_STAT->EVM_TXsideReceivedSamples;

}



