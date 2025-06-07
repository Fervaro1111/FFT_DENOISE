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




#define MAXTSLOT	50
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


void update_CTRL_CRC(report_t *reportIN, report_t *reportOUT, float BLER, int subframe, int CRC_Tslot);

// PRINT TEXT
#define MAXNOFLINES	20
#define MEM_LINELENGTH	200
#define PRINT_LINELENGTH	96

typedef struct PRINT{
	char text[MAXNOFLINES][MEM_LINELENGTH];
	int noflines;
	char borderchar;
	int forecolor;
	int backgroundcolor;
} print_t;


void printTEXT(int noflines, int textcolor, int backcolor, char borderchar);
void printCOLORtext(int position, int forecolor, char groundcolor,  int end, char *data2print);
void my_itoa(int number, char *str, int nofchars);

#endif
