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
#define LTE_CRC24A	0x1864CFB
#define LTE_CRC24B	0X1800063
#define LTE_CRC16	0x11021
#define LTE_CRC8	0x19B

#define max_buffer_size 30
#define MAX_LENGTH	1024*16

typedef struct {
	unsigned long table[256];
	unsigned char data0[MAX_LENGTH];
	int polynom;
	int order;
	unsigned long crcinit;
	unsigned long crcxor;
	unsigned long crcmask;
	unsigned long crchighbit;
	unsigned int crc_out;
} crc_t;


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

void Buffer_Manager(int* RNTI,MAC_struc* inputMAC,int Ts,int delay);

unsigned int crc(unsigned int crc, char *bufptr, int len,
		int long_crc, unsigned int poly, int paste_word);

void crc_free(crc_t *crc_p);
int crc_init(crc_t *crc_par, unsigned int crc_poly, int crc_order);
int crc_set_init(crc_t *crc_par, unsigned long crc_init_value);
int crc_set_xor(crc_t *crc_par, unsigned long crc_xor_value);
unsigned int crc_attach(char *bufptr, int len, crc_t *crc_params);
void pack_bits(unsigned int value, char **bits, int nof_bits);
unsigned int unpack_bits(char **bits, int nof_bits);

int add_CRC(char *bufptr, int len, crc_t *crc_params);
int check_CRC(char *bufptr, int datalen, crc_t *crc_params);

void byte2bitsLSBF(char value, char **bits, int nof_bits);
void bits2byteLSBF(char *byte, char **bits, int nof_bits);
int prepare_to_calc_CRC(char* inBytes,int rcv_samples,char* outBytes);
void extract_ones(char* inout,int length);
void convert_2_binary(int in,int* binary);
void scrambling(char *in, char *out, int snd_samples, int *scrambseq);


#endif
