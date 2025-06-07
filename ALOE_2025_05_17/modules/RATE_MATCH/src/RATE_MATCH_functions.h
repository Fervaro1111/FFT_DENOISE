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

#define MAXCB_KCBg2 3840
#define MAXCB_KCBg1 8448
//#define MAXCB_KCBg1 1152
//#define MAXCB_KCBg2 1152
#define max_buffer_size 120


typedef struct RM_info{
	int TB_TBCRC; //
	float coderate;	//coderate
	int N;		//Ncb from the LDPC
	int CB_size;	//CB size at block output
	int Zc; 	//lifting size
	int graph; 	//LDPC base graph
	int C;		//num codeblocks
	int mod_order;
	int num_layers;
	int rv;
	int REs_DMRS;	   // number of REs with DMRS per Subframe
	int Symb_X_slot;   //Number of symbols for PDSCH per slot.
	int Overhead_higherlayer;
	int nPRB;
	int free_RE;		//Number of data RE in subframe


}infoRM_t;

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



}MAC_struc;

typedef MAC_struc Bufer_MAC[max_buffer_size];

int bit_recover(int *positions, char *in, char *out,int E,int rcv_samples);
int position_recover(int E, int *out, int Ncb,int rcv_samples,int k0);
int bit_selection(int E,char *in, char *out, int Ncb,int k0,int rcv_samples);
int DE_interleave(int E,int mod_order,char *in, char *out);
int interleave(int E,int mod_order,char *in, char *out);
int RateMatch_OutLength(int mod_order,int num_layers,int G, int C, int *Er);
int Select_k0(int graph,int rv_id, int Ncb,int Zc);
int Graph_Selection(int TBS,float R);
int Calc_CB(infoRM_t *RM_info);
void byte2bitsLSBF(char value, char **bits, int nof_bits);
void bits2byteLSBF(char *byte, char **bits, int nof_bits);
int extract_MAC_info(infoRM_t *RM_info, MAC_struc *MACinfo);
MAC_struc buffer(int buffer_size,MAC_struc *NEW_INFO, MAC_struc *buffer);
void Advance_buffer(MAC_struc* Buffer[],MAC_struc *MACinfo,int max_size_buffer);
MAC_struc* out_bufffer(MAC_struc* Buffer[],int max_size_buffer);
int Checkdelay(MAC_struc* MACin,int max_size,int Ts);
MAC_struc* getMACinfo(MAC_struc* Buffer[],int delay,int Ts);
MAC_struc* manageBuffer(int Ts,MAC_struc* MAC_buff[],MAC_struc* inputMAC,int delay);

int Concatenate_inputs(int max_in_interfaces,char* INbytes, char* input,int rcv_samples);
int Calc_RM_params(int in_interfaces,int G,infoRM_t* RM_info,int* Er[]);
void rate_MATCH_calc(int Er[],char INbytes[],char* output,int rcv_samples,int k0,infoRM_t RM_info);
int UNDO_Calc_RM_params(int G,infoRM_t* RM_info,int* Er[]);

void undo_rate_MATCH_calc(int Er[],char* input,char* OUTbytes,int rcv_samples,int k0,infoRM_t RM_info);
int DE_interleave_F(int E,int mod_order,float *in, float *out);
int bit_recover_F(int *positions, float *in, float *out,int E,int rcv_samples);
int Undo_rate_MATCH_calc(int Er[],float INbytes[],float* outputF,int rcv_samples,int k0,infoRM_t RM_info);
int Calc_G(infoRM_t RM_info);

void Buffer_Manager(infoRM_t* RM_info,MAC_struc* inputMAC,int Ts,int delay);



void de_puncture_initial_cols(float *in, float *out,infoRM_t RM_info,int length);
void puncture_initial_cols(char *in, char *out,infoRM_t RM_info,int length);
void rate_MATCH_calc_punct(int Er[],char INbytes[],char* output,int rcv_samples,int k0,infoRM_t RM_info);
int Undo_rate_MATCH_calc_punc(int Er[],float input[],float* outputF,int rcv_samples,int k0,infoRM_t RM_info);

int de_Trnaspose(infoRM_t RM_info,float* in,float* out);
int Trnaspose(infoRM_t RM_info,char* in,char* out);





#endif
