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

// Module Variables and Defines

#define SLENGTH						32
#define MODULATE					0
#define HARD_DEMODULATE		1
#define SOFT_DEMODULATE		2

#define M_BPSK						1
#define M_4QAM						2
#define M_16QAM						4
#define M_64QAM						6
#define M_256QAM					8
#define M_1024QAM					12


#define NODEBUGG					0
#define DEBUGG						1

#define max_buffer_size 120

typedef struct MODparams{
    int opMODE;									// 0: Modulate; 1: Hard Demodulate; 2: Soft Demodulate
		int modulation;							// 0: BPSK, 1: 4QAM, 2: 16QAM, 3: 64QAM
		int debugg;									// 0: No debugg, 1: Debug

}MODparams_t;

static MODparams_t oParam={MODULATE, M_16QAM, NODEBUGG};		// Initialize module params struct


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



/*************************************************************************************************/
// Functions Predefinition
int mod_16QAM (char *bits, int numbits, _Complex float *symbols);
int hard_demod_16QAM (char *bits, complex *symbols, int numsymb);
int soft_demod_16QAM (_Complex float *symbols, int numinputsymb, float * softbits);
int norm16QAM(_Complex float *inout, int length);

int mod_64QAM (char *bits, int numbits, _Complex float *symbols);
int hard_demod_64QAM (char *bits, complex *symbols, int numsymb);
int soft_demod_64QAM (_Complex float *symbols, int numinputsymb, float * softbits);
int norm64QAM(_Complex float *inout, int length);

int mod_256QAM (char *bits, int numbits, _Complex float *symbols);
int soft_demod_256QAM(_Complex float *symbols, int numinputsymb, float *softbits);
int norm256QAM(_Complex float *inout, int length);

int mod_1024QAM (char *bits, int numbits, _Complex float *symbols);
int soft_demod_1024QAM(_Complex float *symbols, int numinputsymb, float *softbits);
int norm1024QAM(_Complex float *inout, int length);
void Buffer_Manager(MAC_struc* inputMAC,int Ts,int delay,MODparams_t* oparam);
#endif
