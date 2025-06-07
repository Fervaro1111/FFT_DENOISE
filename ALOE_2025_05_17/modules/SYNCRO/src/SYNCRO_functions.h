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

#define PSSlength 128
#define SSSlength 128
#define BUFFERMAXSIZE 22*4096*15


typedef struct{

	int NID1;
	int NID2;
	int pMAX1;
	int pMAX2;
	int FFTsize;
	int distance_6;
	int distance_8;

}Syncro_t;

typedef struct{
	int NID1;
	int NID2;
	int CELLID;

}SYNC_struc;

typedef struct{
	int QAM_order;
	float coderateMCS;
	int TB_size_NO_CRC;//TB size without any CRC
	int TB_size_TBCRC; //TB size with CRC from TB
	int RNTI;
	int CELLID;
	int nPRB;
	int q; 		   //parameter from scrambler Cinit
	int num_layers;
	int Overhead_higherlayer;
	int rv;		   // redundancy version for RM
	int Ts_MAC;
	int REs_DMRS;	   // number of REs with DMRS per PRB
	int Symb_X_slot;   //Number of symbols for PDSCH 
	int Additional_DMRS_position;//0: only 1 DMRs in symbol 2
				//1: 2 DMRS-> symbols 2 and 11
				//2: 3 DMRS-> symbols 2,7 and 11
				//3: 4 DMRS-> symbols 2,5,8 and 11
	int free_RE;		//Number of data RE in subframe
	int DMRS_type;		//All done for type A ->Type 1	
				// type B should be ->Type 2
	int CORESETS_symbol_size;   // number of OFDM symbols that contain CORESET prepared for 1 and 2 symbols
	int Agr_lvl_DCI; //Carrier agregation level for DCI (1,2,4,8,16)

	int NID1;//PARAMETERS to generate PSS, SSS and CELL ID
	int NID2;
	int FFTsize;

}MAC_struc;




int detect_DMRS_in_subframe(_Complex float *correl, int numsamples);
int create_spectrum(_Complex float *MQAMsymb, int datalength, 
							int FFTlength,  _Complex float *out_spectrum);
void PSS_gen (int NID1,int NID2);


//int stream_conv_CPLX(_Complex float *ccinput, int datalength,_Complex float *filtercoeff, int filterlength, _Complex float *ccoutput);


void process_correllation(_Complex float *inCorrel, int datalength, _Complex float *outCorrel);
void gen_PSS_time(int FFTsize,_Complex float* output,int NID2;);
int stream_conv_CPLX(_Complex float *ccinput, int datalength, _Complex float *filtercoeff, int filterlength, _Complex float *ccoutput);
int detect_PSS(_Complex float *inout, int length, float *varianze);


int SSS_gen (int NID1,int NID2);


//int ManageBuffer(_Complex float* input,_Complex float* output,int pMAX1,int pMAX2,int rcv_samples,int FFTsize);

int Calc_distance(int pMAX1,int pMAX2,int rcv_samples);

int Align_buffer(int desalineacio,int rcv_samples,_Complex float* input,_Complex float* output);
int Not2PSSdetected(int rcv_samples,_Complex float* input,_Complex float* output);
int Align_2(_Complex float* input,_Complex float* output,int rcv_samples,int pMAX1,int FFT_calculated);

int Calc_distance_CP(int pMAX1,int pMAX2,int rcv_samples,int FFT,_Complex float* input,_Complex float* output);



int DecideNID2_and_FFTsize(int FFTsize,_Complex float* input0,int rcv_samples);
int iterate_FFT_sizes(_Complex float* input0,int rcv_samples);
int downsample(_Complex float* in,_Complex float* out, int length,int decimate);

void Find_NID1(int NID2,_Complex float* input0,int rcv_samples,int FFTsize);
void CorrelaeSSS(int FFTsize,_Complex float* input0,int rcv_samples,int NID1);
void CorrelCp(_Complex float* input0,int rcv_samples,_Complex float* out);
_Complex float Superposar_CP(_Complex float * input,_Complex float * output,_Complex float * window,int window_size);
void gen_signal_freq(int FFTsize,_Complex float* output,_Complex float* input);
int estimateFFTsize(float* input,int length);





//DO SYNCRO
int do_sycro(_Complex float * input0,int rcv_samples,_Complex float * output,
				int* NID1, int* NID2, int FFTsize, _Complex float *graph, int *snd2GRAPH);

//PSS DETECTION
//int first_PSS_detection(_Complex float* input,int rcv_samples,Syncro_t* Syncinfo);
int first_PSS_detection(_Complex float* input,int rcv_samples,Syncro_t* Syncinfo, _Complex float *graph, int *snd2GRAPH);
void Check_PSS(_Complex float* CorrResult,int correl_length,float* varianze,Syncro_t* Syncinfo);
//void DecideNID2(int FFTsize,_Complex float* input0,int rcv_samples,Syncro_t* Syncinfo);
void DecideNID2(int FFTsize,_Complex float* input0,int rcv_samples,
				Syncro_t* Syncinfo, _Complex float *graph, int *snd2GRAPH);
void process_correllation(_Complex float *inCorrel, int datalength, _Complex float *outCorrel);
int stream_conv_CPLX_DDC(_Complex float *ccinput,int datalength,_Complex float *filtercoeff,int filterlength,_Complex float *ccoutput,int Decimate);


//SSS DETECTION
void gen_SSS_time(int FFTsize,_Complex float* output,int NID1,int NID2);
void DecideNID1(int FFTsize,_Complex float* input0,int rcv_samples,int NID1,int NID2,Syncro_t* Syncinfo);
int detect_SSS(_Complex float *inout, int length, float *varianze);
void convolute(int length,_Complex float* filter,_Complex float* in);



//BUFFER
int ManageBuffer(_Complex float* input,_Complex float* output,int rcv_samples,Syncro_t* Syncinfo,int Tslot);
void Write_in_Buff(int rcv_samples, _Complex float* in_samples,Syncro_t* Syncinfo);
int Read_from_Buff(int rcv_samples, _Complex float* out_samples,Syncro_t* Syncinfo);
void Align_Buff(Syncro_t* Syncinfo,int rcv_samples);
void Align_first_detection(int rcv_samples, _Complex float* in_samples);
void Align_with_PSS(int rcv_samples, _Complex float* in_samples);
void Write_and_Align(int rcv_samples, _Complex float* in_samples,Syncro_t* Syncinfo);
void Write_in_buf_first_time(int rcv_samples, _Complex float* in_samples,Syncro_t* Syncinfo);


void get_signal(_Complex float* out);



#endif
