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

#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H
#define STRLENGTH	64

#define STUDENTMODE

#define DOWNLINK	0
#define UPLINK 		1


typedef struct REPORT{
//Provided by DATASOURCESINK
	int subframe;
	float BER;
	float throughput;
	float TimeSlot_us;
	float MeasSubframeTsLength;				// Meas of subframe number Tslots length 
	float StandDevSubframeLength;			// Standard Deviation 	
	unsigned int DSSRoundTripDelay; 	// Delay between receiving data from input0
	unsigned int DSSsndSamples;
	char file_name[STRLENGTH];

	// Provided by EVM
	float EVM;
	unsigned int EVMRoundTripDelay;			// Delay between receiving data form input 0 and 1
	unsigned int EVM_TXsideReceivedSamples;
	float EVM_TXDataPeriod;			// Number of Tslot between receiving samples from input0
	int FFTsize;

	// Provided by CPLX_FILTER
	char filter_name[STRLENGTH];


	// Provided by CHANNEL_SUI
	float SNR;
	int AD_Active;					// AD samples generation from starting simulation
	float RotatePhase;				// Channel Phase rotation
	float BandPassBw;				// Bandwidth of the bandpass signal
	int SUI_Channel;
	float PAPR;

	// Provided by CRC
	float BLER;

	// Provided by REPORT
	int UpDownLink;
	char username[STRLENGTH];		// Name of the user. Must be captured from .params file
	int Tslot;
	int MAC_ID;						// MAC identifier, aqain a random fix value
	int ctrl_val;
	
	
	// Calculated at REPORT
	float Coderate;
	int MODidx;								// Modulation index: 2-4QAM, 4:16QAM, 6-64QAM
	float IntDec;							// Interpolation/decimation: N/D


//	Not NEEDED
/*	int ctrl_val;					// Random value, should depen from current time
	int CHANNEL_Tslot;
	int CPLXFILTER_Tslot;
	int CRC_Tslot;
	int DSS_Tslot;
	int EVM_Tslot;*/
}report_t;


int randomINT(int Tslot);
int check_username(char *A, char *B);
void check_received_report(int Tslot, report_t *report_SENT, report_t *report_IN);
//void write_reportBIN(FILE *filePointer, report_t *report_IN);
void write_reportBIN(FILE *filePointer, report_t *report_IN, int datasize);
//void read_reportBIN(FILE *filePointer, report_t *report_IN);
void read_reportBIN(FILE *filePointer, report_t *report_IN, int datasize);
int generate_report(int Tslot, report_t *report_IN, report_t *report_OUT);
void printREPORT(report_t report_IN);
void readAPPname(char *filename);

void estimateSTATparameters(report_t *report_STAT);

#define APPSIZE	1024*32
typedef struct APP{
	char name[128];
	int numofbytes;
	char localtime[128];
	char contens[APPSIZE];
}APP_t;

void readAPPfileContens(char *filename, APP_t *appcontents);


typedef struct TOT{
	APP_t APP;
	report_t report;
}TOT_t;


void captureAPPinfo(FILE *filePointer);
void printAPPinfoT(TOT_t TOT);


// PRINT TEXT
#define MAXNOFLINES	20
#define MEM_LINELENGTH	200
#define PRINT_LINELENGTH	96

typedef struct PRINT{
	char text[MAXNOFLINES][MEM_LINELENGTH];
	int noflines;
	char borderchar;
	int forecolor;
	int backgroundcolor;
} print_t;


void printTEXT(int noflines, int textcolor, int backcolor, char borderchar);
void printCOLORtext(int position, int forecolor, char groundcolor,  int end, char *data2print);
void my_itoa(int number, char *str, int nofchars);

#endif
