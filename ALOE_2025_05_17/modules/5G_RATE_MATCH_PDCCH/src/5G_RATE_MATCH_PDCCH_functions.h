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

#define max_buffer_size 30

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

typedef MAC_struc Bufer_MAC[max_buffer_size];


typedef struct{
	int E;//bit size after rate match;
	int N; //bit size of ENCODER output;
	int K; //bit size of ENCODE input;
	int Carr_agr_lvl;// Carrier agregation level;
	int Control_symb_number;//Number of coreset OFDM symbols
}RM_t;


int calc_out_bits_max_length(int Carr_agr_lvl,int Coreset_symbol_syze);
int Calc_n(int E,int K);

void Sub_block_interleaving(char* in, char* out,int rcv_samples);
void do_Rate_Match(char* in, char* out,RM_t* RM_info);
void Bit_Selection(char* in, char* out,RM_t* RM_info);

void Bit_Recover_Floats(float* in, float* out, RM_t* RM_info);
void undo_Rate_Match(float* in, float* out,RM_t* RM_info);
void Sub_block_deinterleaving(float* in, float* out,int rcv_samples);

void Buffer_Manager(int* Agr_lvl_DCI,int* Control_symb_number, MAC_struc* inputMAC,int Ts,int delay);

#endif


