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

#define NOFRBsLTE	15				//6: 128; 15: 256; 

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


void update_CTRL(report_t *reportIN, report_t *reportOUT, float EVM, int REPORT_Tslot, unsigned int TslotDIFF);


//CIRBUFFER
typedef struct buffer{
	int readIndex;
	int writeIndex;
	int buffsize;
	int occuplevel;	//Occupancy level of buffer for reading
	int roomlevel;	//Available room in buffer for writing
}buffctrl;

void initCbuff(buffctrl *bufferCtrl, _Complex float *bufferC, int buffersz, int firstpcketsz);
int writeCbuff(buffctrl *buffer, _Complex float *buffdata, _Complex float *in, int length);
int readCbuff(buffctrl *buffer,_Complex float *buffdata, _Complex float *out, int length);
int lookCbuff(buffctrl* buffer,_Complex float *buffdata, _Complex float *out, int length);

// EVM

float computeEVM_3GGP_LTE128(_Complex float *inputMeasured, _Complex float *inputReference, int nofsamples, int reset);


// PRINT TEXT
#define MAXNOFLINES	10
#define LINELENGTH	96

typedef struct PRINT{
	char text[MAXNOFLINES][LINELENGTH];
	int noflines;
	char borderchar;
	int forecolor;
	int backgroundcolor;
} print_t;


void printTEXT(int noflines, int textcolor, int backcolor, char borderchar);
void printCOLORtext(int position, int forecolor, char groundcolor,  int end, char *data2print);
void my_itoa(int number, char *str, int nofchars);


// NORMALIZE DATA

int normalize_vs_var(_Complex float *inout, int length, float varVALUE);

#endif
