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

#define PI 3.14159265358979323846

#define SLENGTH					32
#define NUMRB					15
#define RBCARRIERS				12
#define NOFOFDMSYMBPERSUBFRAME	14
#define NOFOFDMSYMBPERSLOT		7
#define NOFSLOTSPERSUBFRAME		2


#define TXMODE					1
#define RXMODE					-1



int genRSsignalargerThan3RB(int u, int v, int m, int M_RS_SC, _Complex float *DMRSseq, int TxRxMode);
int genDMRStime_seq(int FFTsize, _Complex float  *in_DMRSfreq, int DMRSlength, _Complex float  *out_DMRStime, int TxRxmode);

int detect_DMRS_in_subframe(_Complex float *correl, int numsamples);


//int write_subframe_buffer(_Complex float *datain, int pMAX, int rcv_samples, int FFTsize);

//int read_subframe_buffer(_Complex float *datain, int FFTsize);

int largestprime_lower_than(int number);
int check_if_prime(int number);
int stream_conv_CPLX(_Complex float *ccinput, int datalength, _Complex float *filtercoeff, int filterlength, _Complex float *ccoutput);
int createDMRS(_Complex float *DMRS);
//int setPSS(int phylayerID, _Complex float *PSSsymb, int TxRxMode);

void rotateCvector(_Complex float *in, _Complex float *out, int length, float Adegrees);
float checkPhaseOffset(_Complex float *DMRScorrelation);


///////////////////////////////////////////////////////////////////////////////////////////////////

#define PSSlength 	128
#define SSSlength 	128
#define NofNID2		3
#define MAXFFTSIZE	4096
#define FFTSIZE		512

typedef struct{

	int NID1;
	int NID2;
	int pMAX1;
	int pMAX2;
	int FFTsize;
	int distance_6;
	int distance_8;
	int CaptDISTANCE;
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


int create_spectrum(_Complex float *MQAMsymb, int datalength, int FFTlength,  _Complex float *out_spectrum);
void PSS_gen (int NID2, _Complex float *PSS);
void gen_PSS_time(int NID2, int FFTsize,_Complex float* output);
int detect_PSSs_in_subframe(_Complex float *correl, int numsamples, Syncro_t *SyncINFO);



int write_subframe_buffer(_Complex float *datain, int pMAX, int rcv_samples, Syncro_t *SyncINFO, _Complex float *lastbuffer);
int read_subframe_buffer(_Complex float *dataout, Syncro_t *SyncINFO);







#endif
