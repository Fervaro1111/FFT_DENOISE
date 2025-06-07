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

#define DMRS_max_length 1024
#define Nc 1600
#define numCarriers_PRB 12

typedef struct{
	int OFDMsymbol;
	int Slotnumber;
	int SSB_counter;
	int Additional_DMRS_position;

}DMRS_t;

typedef struct{
	int nPRB;
	int FFTsize;

}info_t;

void calc_Slot_symbol(int Ts,DMRS_t* DMRS_params);
int GEN_DMRS(int Slotnumber,int OFDMsymbol,_Complex float* DMRS_seq);
void Extract_DMRS_from_symbol(info_t* Equalizer_info,_Complex float* in,_Complex float* rcv_DMRS);
void Do_equalize(_Complex float* in,int Ts,_Complex float* out);
int create_LTEspectrumNORS(_Complex float *MQAMsymb, int FFTlength, int datalength, _Complex float *out_spectrum);
int getDATAfromLTE_DOWNLINKspectrum(_Complex float *LTEspect, int FFTlength, int datalength, _Complex float *QAMsymb);
void Calc_Coeficients(_Complex float* rcv_DMRS,_Complex float* DMRS_seq,_Complex float* Coeff,int length,DMRS_t* DMRS_params,info_t* Equalizer_info);

_Complex float Average(_Complex float in,_Complex float average,int length);
float SNR(_Complex float in,_Complex float average);


void Average2(_Complex float* in,_Complex float* average,int length);
void SNR2(_Complex float* in,_Complex float* average,float* SNR);
float Calc_SNRs(DMRS_t* DMRS_params,info_t* Equalizer_info,_Complex float* DMRS_average,_Complex float* rcv_DMRS,float* SNR,int average_counter);

void Compute_MMSE_Coefficents(float SNR,_Complex float* Coeff_in,_Complex float* Coeff_out);

void Interpolate(_Complex float* Coeff_in,_Complex float* Coeff_out,int length);
void Get_Channel(_Complex float* DMRS_rcv,int length,DMRS_t* DMRS_params,info_t* Equalizer_info,_Complex float* DMRS_seq,_Complex float* Coeff);

void Average_channel(_Complex float* channel,_Complex float* channel_average,int average_counter,int length);

float SNR_average(float* snr_in, int length);

void Clean_Channel(_Complex float* in_info,_Complex float* coeff,info_t* Equalizer_info,_Complex float* out_clean);















#endif
