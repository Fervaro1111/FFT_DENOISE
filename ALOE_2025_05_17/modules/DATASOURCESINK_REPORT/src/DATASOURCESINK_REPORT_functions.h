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
long int read_file(char *filename, char *databuff, long int size);
long int read_file_bin(char *filename, char *databuff, long int size);
long int write_file(char *filename, char *databuff, long int size);
long int write_file_bin(char *filename, char *databuff, long int size);
long int addIDbin(int *databuff);
int compare_data(char *file, long int filelength, char *data, int datalength, float *BER);
void writemy(FILE *f, int* buf, int size);
FILE *datafile_open(char * name, char *mode);
void datafile_write_float(FILE *f,  float* buf, int size);

void initFrame2send(char *databuff, int MaxLength);
int sendDATA(char *databuff, char *data_out, int datalength);
int compare_data1(char *DATA, char *data_in, int datalength, float *BER);


#define MAXTSLOT	50
#define STRLENGTH	64
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

//void update_CTRL_DATASOURCESINK(report_t *reportIN, report_t *reportOUT, float BER, float throughtput, int DSS_Tslot);
//void update_CTRL_DATASOURCESINK(report_t *reportIN, report_t *reportOUT, float BER, float throughput, int DSS_Tslot, float INTDEC, float CodeRate, int MODindex);
/*void update_CTRL_DATASOURCESINK(report_t *reportIN, report_t *reportOUT, float BER, float throughput, 
								int DSS_Tslot, float INTDEC, float CodeRate, int MODindex, float dataPeriod, 
								float Deviation, float TimeSlot_us, unsigned int RoundTripDelay, int DSS_snd_samples0);*/
void update_CTRL_DATASOURCESINK(report_t *reportIN, report_t *reportOUT, float BER, float throughput, 
								int DSS_Tslot, float INTDEC, float CodeRate, int MODindex, float dataPeriod, 
								float Deviation, float TimeSlot_us, unsigned int RoundTripDelay, int DSS_snd_samples0, char *filename);
void estimateCoderate(int rcv_bytes, int NumRBs, float *coderate, int *MODindex);
void computeStatisticsSubframeLength(int NofTslot, float *Meas, float *Deviation);

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


#define SEND	0
#define RECV	1

double compute_elapsedTimeFromFirstExec();
float compute_Throughtput(int NumOfRcvTotalBytes);
float measure_TimeSlot_us();
//int measure_RoundTripDelay_inTs(float TimeSlot_us, int SENDRECV);
int measure_RoundTripDelay_inTs(int numTslot, int SENDRECV);

#endif
