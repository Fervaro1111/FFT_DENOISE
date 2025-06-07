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
#define FILTERLENGTH	2048
#define CCINIT			0
#define CCSTREAM		1

#define opMODEBYPASS	0
#define opMODECPLX		1
#define opMODEREAL		2

#define STR_LENGTH		128

#define READTEXT		0
#define READBIN			1


//#define STUDENTMODE


typedef struct MODparams{
    int opMODE;
	int readfileMODE;			//0: Text; 1: Bin
    int Ninterpol;
	int Ndecimate;
	char filtercoefffile[STR_LENGTH];
}MODparams_t;




int init_functionA_COMPLEX(_Complex float *input, int length);
int functionA_COMPLEX(_Complex float *input, int lengths, _Complex float *output);
int init_functionB_FLOAT(float *input, int length);
int functionB_FLOAT(float *input, int lengths, float *output);
void readCPLXfilecoeff(_Complex float *filtercoeff, int *filterlength, char *filtername);
void readREALfilecoeff(_Complex float *filtercoeff, int *filterlength, char *filtername);
int stream_conv_CPLX(_Complex float *ccinput, int datalength,_Complex float *filtercoeff, int filterlength, _Complex float *ccoutput);
int stream_conv_CPLX_INT_DEC(	_Complex float *ccinput, 
											int datalength,
											_Complex float *filtercoeff, 
											int filterlength, 
											_Complex float *ccoutput, 
											MODparams_t oParam);
int bypass(_Complex float *input, int datalength,_Complex float *output);



#define MAXTSLOT	50
#define STRLENGTH	64
typedef struct REPORT{
	char username[STRLENGTH];		// Name of the user. Must be captured from .params file
	int ctrl_val;					// Random value, should depen from current time
	int MAC_ID;						// MAC identifier, aqain a random fix value
	char filter_name[STRLENGTH];
	float SNR;
	int AD_Active;					// AD samples generation from starting simulation
	float RotatePhase;				// Channel Phase rotation
	float BandPassBw;				// Bandwidth of the bandpass signal
	int subframe;
	float BLER;
	float BER;
	float throughput;
	float Coderate;
	int MODidx;						// Modulation index: 2-4QAM, 4:16QAM, 6-64QAM
	float IntDec;					// Interpolation/decimation: N/D
	float EVM;
	unsigned int TslotDIFF; 		// Delay between receiving data form input 0 and 1
	float PAPR;
	int EVM_Tslot;
	int CHANNEL_Tslot;
	int CPLXFILTER_Tslot;
	int CRC_Tslot;
	int DSS_Tslot;
	int Tslot;
	float MeasSubframeTsLength;		// Meas of subframe number Tslots length 
	float StandDevSubframeLength;	// Standard Deviation 
	int SUI_Channel;
}report_t;


void update_CTRL(report_t *reportIN, report_t *reportOUT, char *filename, int CHANNEL_Tslot, float Bw);


// FILE BINARY FORMAT

#define COEFFsz	2048
typedef struct FILEFORMAT{
	int numCoeffs;
	char filter_name[STRLENGTH];
	_Complex float COEFFS[COEFFsz];
}file_t;


void writeBINfilterCoeffFile(_Complex float *COEFFs, int filterlength, char *filtername);
int readBINfilterCoeffFile(_Complex float *COEFFs, int *filterlength, char *filtername);



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
