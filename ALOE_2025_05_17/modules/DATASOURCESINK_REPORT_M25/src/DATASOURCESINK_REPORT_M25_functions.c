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
#include <time.h> 

#include "DATASOURCESINK_REPORT_M25_functions.h"

char f[32]="logs/.log";

extern char mname[];
extern print_t T;


#define NUMMIN_ERRORS		0
#define NUMTS2PRINT_BER		5

// Mode 1
#define MAXTSLOTSEXEC	1000
#define MAXLENGTH		32
int sentDataLength[MAXLENGTH];		// Saves the length of sent packets

void initFrame2send(char *databuff, int MaxLength){
	int i, first=0, second=1, next=1;
	
	for(i=0; i<MaxLength; i++){
		//*(databuff+i)=(char)(rand()&0xFF);

		*(databuff+i)=(char)(next&0xFF);
		next = first+second;
		first=second;
		second=next;
	}
}

int sendDATA(char *databuff, char *data_out, int datalength){
	int i;
	static int w_idx=0;

	if(datalength == 0)return(0);
	
	for(i=0; i<datalength; i++){
		*(data_out+i)=*(databuff+i);
	}
	sentDataLength[w_idx]=datalength;
	w_idx++;
	if(w_idx==MAXLENGTH)w_idx=0;
	return(1);
}


#define TSLOT_s		0.021333333

int compare_data1(char *DATA, char *data_in, int datalength, float *BER){

    static long int count=0;
	static int numbitserror=0;
    unsigned char byte;
    int i, n;
    static int Tslot=-1;
	static int r_idx=0;
	float throughtput=0.0;

	Tslot++;
	*BER=10e-6;
	if(datalength<=0)return(1);
/*	if(datalength != sentDataLength[r_idx]){
		printf("data_source_sink.compare_data1(): datalength=%d != sentDataLength[%d]=%d\n", 
				datalength, r_idx, sentDataLength[r_idx]);
		return(-1);
	}*/
	r_idx++;
	if(r_idx==MAXLENGTH)r_idx=0;

	for (i=0; i<datalength; i++){
		byte=*(data_in+i)^*(DATA+i);
		if(byte!=0){
			for(n=0; n<8; n++){
				numbitserror=numbitserror+(byte&0x01);
				byte=byte>>1;
			}
		}
	}
	count += datalength*8;
	*BER=(float)(numbitserror)/(float)(count);
	throughtput = (float)count/(((float)Tslot)*TSLOT_s);

//	if(Tslot==0)printf("\n");
	if(Tslot%NUMTS2PRINT_BER == 0){
		*BER=(float)(numbitserror)/(float)(count);
		printf("\033[1;31;47m \t==========================================================================\t\033[0m\n");
		printf("\033[1;31;47m \t!!!!TSLOT=%d: CURRENT BER = %1.6f, numbitserror=%d, throughtput=%6.1f         \t\033[0m\n",Tslot, *BER, numbitserror, throughtput);
		printf("\033[1;31;47m \t==========================================================================\t\033[0m\n");
		numbitserror=0;
		count=0;
	}
	if(Tslot==MAXTSLOTSEXEC){
		printf("\033[1;31;47m \tO   ################################################################     O\t\033[0m\n");
		printf("\033[1;31;47m \tO     APP EXECUTION FINISHED!!!: DATA BLOCKS SENT & RECEIVED              \t\033[0m\n");
		printf("\033[1;31;47m \tO     DATA RECEIVED WITH BER = %1.6f, numbitserror=%d                     \t\033[0m\n",*BER, numbitserror);
		printf("\033[1;31;47m \tO   ################################################################     O\t\033[0m\n");
		return(-1);
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
long int read_file(char *filename, char *databuff, long int size){
	long int readlength=0;
	FILE *fp;

//	printf("%s\n", filename);
	fp=fopen(filename, "r");
	readlength=fread(databuff,1,size,fp);
	if(size != readlength){
		//printf("Reading filename %s: read data=%ld, expected=%ld\n", filename, readlength, size);
	}
	fclose(fp);
	return(readlength);
}


long int read_file_bin(char *filename, char *databuff, long int size){
	long int readlength=0;
	FILE *fp;

//	printf("%s\n", filename);
	fp=fopen(filename, "rb");
	readlength=fread(databuff,1,size,fp);
	if(size != readlength){
		//printf("Reading filename %s: read data=%ld, expected=%ld\n", filename, readlength, size);
	}
	fclose(fp);
	return(readlength);
}


long int write_file(char *filename, char *databuff, long int size){
	long int writelength=0;
	FILE *fp;

	fp=fopen(filename, "w");
	writelength=fwrite(databuff,1,size,fp);
	if(size != writelength){
		printf("Writing filename %s: wrote data=%ld, expected=%ld\n", filename, writelength, size);
	}
	fclose(fp);
	return(writelength);
}

long int write_file_bin(char *filename, char *databuff, long int size){
	long int writelength=0;
	FILE *fp;

	fp=fopen(filename, "wb");
	writelength=fwrite(databuff,1,size,fp);
	if(size != writelength){
		printf("Writing filename %s: wrote data=%ld, expected=%ld\n", filename, writelength, size);
	}
	fclose(fp);
	return(writelength);
}


/**
 * @brief Compare the received bytes with the reference file.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int compare_data(char *file, long int filelength, char *data_in, int datalength, float *BER){

    static long int count=0;
	static int numbitserror=0;
    unsigned char byte;
    int i, n;
    static int Tslot=0;
	float throughtput=0.0;

	if(count<filelength){		//Check the file length
		for (i=0; i<datalength; i++){
			byte=*(data_in+i)^*(file+count);
			if(byte!=0){
				for(n=0; n<8; n++){
//					printf("byte=%x\n",byte&0x01);
					numbitserror=numbitserror+(byte&0x01);
					byte=byte>>1;
//					printf("numbitserror=%d\n",numbitserror);
				}
			}
//			printf("count=%d\n", count);
			count++;	
		}
		//printf("numbitserror=%d\n",numbitserror);
	}
	else {
		printf("Data bytes received = %ld longer than file length=%ld\n", count, filelength);
		if(numbitserror>=NUMMIN_ERRORS){
			//printf("AAnumbitserror=%d\n",numbitserror);
			*BER=(float)(numbitserror)/(float)(count*8);
			printf("\033[1;31m O       #####################################################        O\033[0m\n");
			printf("\033[1;31m O       APP EXECUTION FINISHED!!!: FILE SENT & RECEIVED \033[0m\n");
			printf("\033[1;31m O       FILE RECEIVED WITH BER = %1.6f, numbitserror=%d\033[0m\n",*BER, numbitserror);
			printf("\033[1;31m O       #####################################################        O \033[0m\n");
		}else{
			printf("FILE RECEIVED: numerrors=%d < NUMMIN_ERRORS=%d and BER can not be calculated\n",numbitserror, NUMMIN_ERRORS);
		}
		return(-1);
	}
//	if(Tslot==0)printf("\n");
	if(Tslot%NUMTS2PRINT_BER == 0 && Tslot>0){
		*BER=(float)(numbitserror)/(float)(count*8);
		throughtput = (float)count/(((float)Tslot)*TSLOT_s);

/*		printf("\033[1;31;47m =====================================================================================================\t\033[0m\n");
		printf("\033[1;31;47m %s!!!! TSLOT=%d: CURRENT BER = %1.6f, numbitserror=%d, throughtput=%6.1f bps        \t\033[0m\n",
					mname, Tslot, *BER, numbitserror, throughtput);
		printf("\033[1;31;47m =====================================================================================================\t\033[0m\n");
*/
	}

	Tslot++;
    return(1);
}

long int addIDbin(int *databuff){
	long int writelength=0;
	int readbuff[32];
	int readlength=0;
	int i, flag=0;
	FILE *fp;

	fp=fopen(f, "ab+");
	readlength=fread(readbuff,sizeof(int),32,fp);
	for(i=0; i<readlength; i++){
		if(readbuff[i] == (*databuff))flag=1;
	}
	if(flag==0){
		writelength=fwrite(databuff,1,sizeof(int),fp);
	} else{
		fseek(fp, sizeof(int), SEEK_END);
		fread(databuff, sizeof(int), 1, fp);
	}
	fclose(fp);
	return(writelength);
}

int readIDfilebin(int *databuff){
	long int readlength=0;
	FILE *fp;

	//printf("%s\n", filename);
	fp=fopen(f, "rb");
	if(fp == NULL)return(-1);
	readlength=fread(databuff,sizeof(int),16,fp);
	fclose(fp);
	return(readlength);
}


void writemy(FILE *f, int* buf, int size) {
	int i;
	char aux[16];
	int a[8]={7,2,0,1,5,4,3,6};

	for (i=0;i<size;i++) {
		sprintf(aux, "%d", buf[i]);
		if(buf[i] > 0)fprintf(f,"0.%c%c%c%c%c%c%c%c\n",
				aux[a[0]], aux[a[1]],aux[a[2]],aux[a[3]],
				aux[a[4]],aux[a[5]],aux[a[6]],aux[a[7]]);
		else fprintf(f,"0.000000\n");
	}
}




FILE *datafile_open(char * name, char *mode) {
	return fopen(name, mode);
}


void datafile_write_float(FILE *f,  float* buf, int size) {
	int i;
	fprintf(f,"%1.6f\n", buf[0]);

}

void computeStatisticsSubframeLength(int NofTslot, float *Meas, float *Deviation){

	static int LastRecvNofTslot;
	static float lMeas=0.0, lDev=0.0;
	static int NofRecv=0;


	

	if(NofRecv>0){
			lMeas += (float)(NofTslot-LastRecvNofTslot);
			*Meas=lMeas/(float)NofRecv;
			lDev += ((float)(NofTslot-LastRecvNofTslot) - *Meas)*((float)(NofTslot-LastRecvNofTslot) - *Meas);
			if(NofRecv>2)*Deviation = (float)sqrt((double)(lDev/(float)(NofRecv-1)));
	}
	NofRecv++;

//	printf("                                   Current SUBframeTslotsLength=%d, Meas=%f, Deviation=%f\n", NofTslot-LastRecvNofTslot, *Meas, *Deviation);
	LastRecvNofTslot=NofTslot;

}


///////////////////////////////////////////////REPORTING///////////////////////

void update_CTRL_DATASOURCESINK(datasourcesink_t *reportOUT, float BER, float throughput, 
								int subframe, float TimeSlot_us, char *filename){
//		printf("DATASOURCESINK_REPORT_O24: Tslot=%d, UserName=%s, ctrlVAL=%d, MAC_ID=%d\n", reportIN->Tslot, &reportIN->username[0], reportIN->ctrl_val, reportIN->MAC_ID);
		
//		printf("1-DATASOURCESINK_REPORT_O24.update_CTRL(): BER=%3.1f\n", reportOUT->BER);
		reportOUT->subframe=subframe;
		reportOUT->BER=BER;
		reportOUT->throughput=throughput;
		//reportOUT->MeasSubframeTsLength=dataPeriod;
		//reportOUT->StandDevSubframeLength=Deviation;
		reportOUT->TimeSlot_us=TimeSlot_us;
		strcpy(&reportOUT->file_name[0], filename);

//	printf("2-DATASOURCESINK_REPORT_O24(): reportOUT->CHANNEL_Tslot=%d\n", reportOUT->CHANNEL_Tslot);
//		printf("2-DATASOURCESINK_REPORT_O24.update_CTRL(): BER=%3.1f\n", reportOUT->BER);
}


#define NOFDATAOFDMSYMBOLSSUBFRAME	12
#define CRC_BITS					24
#define NOFCODERATES				5
#define NUMCARRIERSRB				12

void estimateCoderate(int rcv_bytes, int NumRBs, float *coderate, int *MODindex){
	
	int i;	
	int ModIndex=2; 	// Modulation Index: 2: 4QAM, 4: 16QAM, 6: 64QAM
	float CodeRate[NOFCODERATES];

	int MappingBits;

	for(i=0; i<NOFCODERATES; i++){
		MappingBits =  NumRBs*NUMCARRIERSRB*NOFDATAOFDMSYMBOLSSUBFRAME*ModIndex;
		//printf("NumRBs=%d, MappingBits=%d, rcv_bytes*8+CRC_BITS=%d\n", NumRBs, MappingBits, rcv_bytes*8+CRC_BITS);
		CodeRate[i]=(float)(MappingBits)/(float)(rcv_bytes*8+CRC_BITS);
		if((CodeRate[i]-floor(CodeRate[i]))==0.0 && CodeRate[i] <=10.0){
			//printf("CodeRate=%f, ModIndex=%d\n", CodeRate[i], ModIndex);
			*coderate=1.0/CodeRate[i];
			*MODindex=ModIndex;
			break;
		}
		ModIndex+=2;
	}
}

#define BILLION  1000000000.0

double compute_elapsedTimeFromFirstExec(){

	static int first=0;

    static struct timespec start;
	struct timespec end;
 
    if(first==0){
		clock_gettime(CLOCK_REALTIME, &start);
		first=1;
	}
 
    clock_gettime(CLOCK_REALTIME, &end);
    // time_spent = end - start
    double time_spent = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / BILLION;
 
    //printf("Time elpased is %f seconds", time_spent);
 
    return time_spent;

}

float measure_TimeSlot_us(){

	static int first=0, Tslot=0;

    static struct timespec start;
	struct timespec end;

	float TimeSlot_us;
	double time_spent = 0.0;

	if(first==1){
		clock_gettime(CLOCK_REALTIME, &end);
		// time_spent = end - start
		time_spent = (end.tv_sec - start.tv_sec) +
		                    (end.tv_nsec - start.tv_nsec) / BILLION;
	}
    if(first==0){
		clock_gettime(CLOCK_REALTIME, &start);
		first=1;
	}
 
	TimeSlot_us=(float)(time_spent/(double)Tslot);
 
    //printf("Time elpased is %f seconds", time_spent);
 
	Tslot++;
    return TimeSlot_us*1000000.0;


}


float compute_Throughtput(int NumOfRcvTotalBytes){
	
	float throughtput;
	double elapsedSeconds;

	elapsedSeconds=compute_elapsedTimeFromFirstExec();
	throughtput=(float)(((double)(NumOfRcvTotalBytes*8))/(double)elapsedSeconds);

	return(throughtput);
}



int measure_RoundTripDelay_inTs(int numTslot, int SENDRECV){

	static int firstSEND=0, firstRECV=0;
	static int NofTslots;
 
    if(firstSEND==0 && SENDRECV==SEND){
		NofTslots=0;
		firstSEND=numTslot;
	}

    if(firstRECV==0 && SENDRECV==RECV){
		NofTslots= numTslot-firstSEND;
		firstRECV=1;
	}
 
    //printf("Time elpased is %f seconds", time_spent);
 
    return NofTslots;

}




/*int measure_RoundTripDelay_inTs(float TimeSlot_us, int SENDRECV){

	static int firstSEND=0, firstRECV;
    static struct timespec start;
	struct timespec end;
	static int NofTslots;
 
    if(firstSEND==0 && SENDRECV==SEND){
		clock_gettime(CLOCK_REALTIME, &start);
		NofTslots=0;
		firstSEND=1;
	}

    if(firstRECV==0 && SENDRECV==RECV){
		clock_gettime(CLOCK_REALTIME, &end);
    	// time_spent = end - start
   		double time_spent = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / BILLION;
		NofTslots= (float)((time_spent*1000000.0)/(double)TimeSlot_us);
		firstRECV=1;
	}
 
    //printf("Time elpased is %f seconds", time_spent);
 
    return NofTslots;

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

