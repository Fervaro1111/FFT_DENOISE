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

#ifndef CRC_
#define CRC_

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
//Provided by DATASOURCESINK
	int subframe;
	float BER;
	float throughput;
	float TimeSlot_us;
	float MeasSubframeTsLength;				// Meas of subframe number Tslots length 
	float StandDevSubframeLength;			// Standard Deviation 	
	unsigned int DSSRoundTripDelay; 	// Delay between receiving data from input0
	unsigned int DSSsndSamples;
	char file_name[STRLENGTH];

	// Provided by EVM
	float EVM;
	unsigned int EVMRoundTripDelay;			// Delay between receiving data form input 0 and 1
	unsigned int EVM_TXsideReceivedSamples;
	float EVM_TXDataPeriod;			// Number of Tslot between receiving samples from input0
	int FFTsize;

	// Provided by CPLX_FILTER
	char filter_name[STRLENGTH];


	// Provided by CHANNEL_SUI
	float SNR;
	int AD_Active;					// AD samples generation from starting simulation
	float RotatePhase;				// Channel Phase rotation
	float BandPassBw;				// Bandwidth of the bandpass signal
	int SUI_Channel;
	float PAPR;

	// Provided by CRC
	float BLER;

	// Provided by REPORT
	int UpDownLink;
	char username[STRLENGTH];		// Name of the user. Must be captured from .params file
	int Tslot;
	int MAC_ID;						// MAC identifier, aqain a random fix value
	int ctrl_val;
	
	
	// Calculated at REPORT
	float Coderate;
	int MODidx;								// Modulation index: 2-4QAM, 4:16QAM, 6-64QAM
	float IntDec;							// Interpolation/decimation: N/D


//	Not NEEDED
/*	int ctrl_val;					// Random value, should depen from current time
	int CHANNEL_Tslot;
	int CPLXFILTER_Tslot;
	int CRC_Tslot;
	int DSS_Tslot;
	int EVM_Tslot;*/

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
