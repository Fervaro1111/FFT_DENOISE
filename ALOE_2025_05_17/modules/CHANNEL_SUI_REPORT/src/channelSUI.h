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
 * along with ALOE.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CHANNEL_SUI_H
#define CHANNEL_SUI_H

#include "CHANNEL_SUI_REPORT_interfaces.h"

#define TAPS 3				//nombre de taps del fitre SUI
#define SUI_SAMPLES 500		//nombre de mostres del canal que generem
#define M 256				//nombre de mostres del filtre doppler
#define M_PI 3.141516		//numero Pi
//#define FLOW_SIZE 320		//mida dels flows entrants
#define FLOW_SIZE INPUT_MAX_DATA		//mida dels flows entrants
#define CHANNELLENGTH	1024


float suiReal[TAPS][SUI_SAMPLES];			//vector on guardem la part real del canal SUI
float suiImag[TAPS][SUI_SAMPLES];			//vector on guardem la part imaginaria del canal SUI
float P[TAPS],K[TAPS],Dop[TAPS],tau[TAPS];	//variables del canal SUI (definides en els models)
float Fnorm;								//frequencia normalitzada (definida en els models)


float flowChannelReal[TAPS][FLOW_SIZE];		//vector on guardem el reescalat que fem a la part real del canal SUI
float flowChannelImag[TAPS][FLOW_SIZE];		//vector on guardem el reescalat que fem a la part imaginaria del canal SUI
float delayReal[FLOW_SIZE*2];				//vector que fem servir per guardar la part real dels retards entre flows
float delayImag[FLOW_SIZE*2];				//vector que fem servir per guardar la part imaginaria dels retards entre flows
int taps[TAPS];								//vector amb els retards calculats en nombre de posicions de cada tap
int suiCount;								//ultima posicio del canal SUI en la que hem fet reescalat
int posCount;								//ultima posicio entre dos punts del canal SUI que hem calculat
int resamplingRate;							//factor de reescalat 

int FLOW_SIZEvar;

_Complex float CHCoeff[CHANNELLENGTH];
int channelLength;
int stream_conv_CPLX_INT_DEC(	_Complex float *ccinput, 
											int datalength,
											_Complex float *filtercoeff, 
											int filterlength, 
											_Complex float *ccoutput);


float max(float *vec,int lenght);
void initSuiParameters(int nSui);
float interpolate(float value1,float value2,int samples,int number);
void resample();
//_Complex float resample2(int NofCoeff, int N, int nofTAP);
_Complex float resample2(int count, int N, int nofTAP);
_Complex float resample3(int N, int nofTAP);
float gauss (float mean, float sigma);
void FFT(int dir,long m,float *x,float *y);
void iFFTShift(float *x);
void FFTFilt(float *re,float *im, float *filt);
void initSui(int nSui, int symbolRate);
void addNoise(float *DataRe, float *DataIm,float SNR);
void canal(float *DataRe, float *DataIm, float SNR);
void canal2(_Complex float *ccinput, int numsamples, _Complex float *ccoutput, float snr);
void changeSymbolRate(int symbolRate);

#endif
