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


#define Nc 1600
#define PSSlength 128
#define SSSlength 128

typedef struct{
	int NID1;
	int NID2;
	int DMRSlength;
	int OFDMsymbol;
	int Slotnumber;


}Sequences_t;

typedef struct{
	int datalength;
	int FFTlength;
	int numFFT;
	int OFDMsymb_data;
	int offset_data;
	int SSblock_length;
	int DMRS_length;
	int Additional_DMRS_position;//0: only 1 DMRs in symbol 2
				//1: 2 DMRS-> symbols 2 and 11
				//2: 3 DMRS-> symbols 2,7 and 11
				//3: 4 DMRS-> symbols 2,5,8 and 11
	
	int DMRS_type;		//All done for type A ->Type 1	
				// type B should be ->Type 2


}MAPP_t;





void calc_sequences(Sequences_t SequencesParams);
int SSS_gen (int NID1,int NID2);
int PSS_gen (int NID1,int NID2);
int GEN_DMRS(int Slotnumber,int DMRSlength,int OFDMsymbol,_Complex float* DMRS_seq);
int create_LTEspectrumNORS(_Complex float *MQAMsymb, int FFTlength, int datalength, _Complex float *out_spectrum);
void fill_symbol(_Complex float *inMQAMsymb,int FFTlength,int datalength,_Complex float *symbol);
void Do_Mapping(MAPP_t MAPPinfo,_Complex float *inMQAMsymb,int* data_per_symbol,int SSblock_flag,_Complex float *subframe,int data_init_pos);
void fill_subframe(_Complex float *inMQAMsymb,int FFTlength,int datalength,_Complex float *subframe,int numFFT,int offset);
void cpy2subframe(_Complex float *subframe,_Complex float *symbol,int FFTsize);
int offset_data(OFDMsymb_data);



void fill_structure_4_DMRS(int* data_per_symbol,MAPP_t MAPPinfo,int SSblock_flag,int data_init_pos);
void fill_structure_3_DMRS(int* data_per_symbol,MAPP_t MAPPinfo,int SSblock_flag,int data_init_pos);
void fill_structure_2_DMRS(int* data_per_symbol,MAPP_t MAPPinfo,int SSblock_flag,int data_init_pos);
void fill_structure_1_DMRS(int* data_per_symbol,MAPP_t MAPPinfo,int SSblock_flag,int data_init_pos);
void fill_structure_NO_DMRS(int* data_per_symbol,MAPP_t MAPPinfo,int SSblock_flag);
void mix_data(_Complex float *DMRSseq,_Complex float *inMQAMsymb,_Complex float *out_symb,int data_length);
void generate_Subframe_structure(MAPP_t MAPPinfo, int* data_per_symbol,int SSblock_flag,int data_init_pos);
void gen_subframe(MAPP_t MAPPinfo,int* data_per_symbol,_Complex float* subframe,_Complex float* insymb);
void ADD_SSS_PSS(MAPP_t MAPPinfo,_Complex float* subframe,int SSblock_flag);
void first_symbols(int data_init_pos,int* data_per_symbol,MAPP_t MAPPinfo);
void FILL_with_SS(int* data_per_symbol,MAPP_t MAPPinfo);
void add_DMRSanddata_to_subframe(MAPP_t MAPPinfo,int* data_per_symbol,_Complex float* subframe,_Complex float* insymb,int i,_Complex float* insymb_with_DMRS,_Complex float* symbol,int in_pos);





int getDATAfromLTE_DOWNLINKspectrum(_Complex float *LTEspect, int FFTlength, int datalength, _Complex float *QAMsymb);
void extract_data_from_subframe(MAPP_t MAPPinfo,int* data_per_symbol,_Complex float*INspectrum,_Complex float*symbOUT,int offset_data);
void do_demap(MAPP_t MAPPinfo,_Complex float *insubframe,int* data_per_symbol,int SSblock_flag,_Complex float *RX_symbolsOUT,int data_init_pos);
void demix_data(_Complex float *inMQAMsymb,_Complex float *out_symb,int data_length);
void REMOVE_DMRS(int data_offset,MAPP_t MAPPinfo,_Complex float* insymb,int* data_per_symbol,_Complex float* out_data);


#endif
