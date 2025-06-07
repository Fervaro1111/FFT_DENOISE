
#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H



#define STR_LEN		128

#define LDPCGRAPH1	1
#define LDPCGRAPH2	2


//#define MAXCB_KCBg2 3840
//#define MAXCB_KCBg1 8448
#define MAXCB_KCBg1 1152
#define MAXCB_KCBg2 1152
#define MAXNOFCBs	256	

#define MAXINPUTBYTES	1000000/8

#define SEGMENT_DEBUGG	1

#define TXMODE			0x01
#define RXMODE  		0x02
#define TXRXTESTMODE	0x03

#define HISTOSZ	512

#define LDPCbaseGraph1	1
#define LDPCbaseGraph2	2

#define LTE_CRC24A	0x1864CFB
#define LTE_CRC24B	0X1800063
#define LTE_CRC16	0x11021
#define LTE_CRC8	0x19B


#define MAX_LENGTH	1024*16


#define max_buffer_size 120

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




typedef struct CBinfo{
	int A;					// A: According 3GPP notation. Transport block size
	int L_TB;				// CRC size for Transport Block
	int B;					// B: According 3GPP notation, A+TransportBlockCRCsize
	int nofCBs;				// Total number of CBs
	int L_CB;				// CRC size for CodeBlock
	int CBsize;				// Code Block size: include Data, CRC and filled bits
	int K1;					// Data size
	int ILS;
	int LDPCgraph; 			/* 1: LDPC graph1, 2: LDPC graph2*/

}cbsegm_t;


typedef struct CBparams{
	char mname[STR_LEN];	// Given Module Name in app file
	int opMODE;				// 0: TX, 1:RX
	int updown;				// 0: UPLINK, 1: DOWNLINK
	int I_MCS_TABLE; 		// 0: 5.1.3.1-1, 1: 5.1.3.1-2, 2:5.1.3.1-3, 3: 6.1.4.1-1, 4: 6.1.4.1-2
							// Only 0 available TODO
	int I_MCS; 				// MCS Index
	int TransPrecd; 		// Transform Precoding: 0: Disabled, 1: Enabled
	int nPRBs; 				// Total number of allocated PRBs for the UE
	int v; 					// Number of Layers
	
	int debugg;				// 0: No debug, 1: debug

}cbparams_t;


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

// Controls to compute the number of expected bits at RX PUSCH
typedef struct CBctrlRX{
	int Nsh_symb; 					// Number of scheduled OFDM symbols in a slot (max 14 or 12.
	int N_PRB_DMRS;					// Number of REs for DMRS per PRB in the scheduled duration including the overhead of the DMRS CDM groups indicated by DCI format 0_0/0_1
	int Noh_PRB; 					// The overhead configured by higher layer parameter xOverhead in PUSCHservingCellConfing. If xOverhead is not configured (a value from 0, 6, 12 or 18), this is set to 0.
									// For Msg3 PUSCH, thisis always set to 0.
	int Qm;							// Modulation Order: 1: BPSK, 2: 4QAM, 4: 16QAM, 6:64QAM, 8: 256QAM
	float CodeRate;		

	int TB_CRC; //TB size + TB_CRC size	

}cbctrl_t;



typedef struct CBstatist{
	int nofWrongCBCRCs; 			// Accumulated Number of Code Blocks wrong CRCs
	int wrongCBCRC[HISTOSZ];		// Indicate the Codeblock index received with wrong CRC in current subframe
	int histowrongCBCRC[HISTOSZ];	// Accumulated histogram Indicating the Codeblock index received with wrong CRC in each subframe
	int nofWrongTBCRSs;				// Accumulated Number of Transport Blocks wrong CRCs
}cbstatist_t;

typedef struct{
	int graph; //base graph 1 or 2
	int Z;
	int ILS; //Table 5.3.2.1 from 38.212

}LDPC_struc_t;



int aloe_ComputeCodeBlockSegmentation(cbsegm_t *CBinfoL, cbparams_t CBparams, cbctrl_t CBctrl,LDPC_struc_t* LDPC_info);
int aloe_CreateCodeBlocks(char *inBytes, char *outBytes, cbsegm_t *CBinfoL, cbparams_t CBparams, cbstatist_t *CBstatist, crc_t crc_p);
int aloe_5GCheckCRC(char *inBytes, char *outBytes, cbsegm_t *CBinfoL, cbstatist_t *CBstatist, crc_t crc_p);
void Byte2bit(char *bytes,char *bits);

int buffer(int buffer_size,unsigned int new_value,int *buffer);


void initCodeBlock();
void initTransportBlockCRC16();
void initTransportBlockCRC24();


unsigned int crc(unsigned int crc, char *bufptr, int len,
		int long_crc, unsigned int poly, int paste_word);

void crc_free(crc_t *crc_p);
int crc_init(crc_t *crc_par, unsigned int crc_poly, int crc_order);
int crc_set_init(crc_t *crc_par, unsigned long crc_init_value);
int crc_set_xor(crc_t *crc_par, unsigned long crc_xor_value);
unsigned int crc_attach(char *bufptr, int len, crc_t *crc_params);
void pack_bits(unsigned int value, char **bits, int nof_bits);
unsigned int unpack_bits(char **bits, int nof_bits);
int aloe_5GCheckCRC(char *inBytes, char *outBytes, cbsegm_t *CBinfoL, cbstatist_t *CBstatist, crc_t crc_p);

int add_CRC(char *bufptr, int len, crc_t *crc_params);
int check_CRC(char *bufptr, int datalen, crc_t *crc_params);


/* Byte to bits: Left Significant bit first*/
void byte2bitsLSBF(char value, char **bits, int nof_bits);
/* Bits to Byte: Left Significant bit first*/
void bits2byteLSBF(char *byte, char **bits, int nof_bits);

// STATISTICS
void init_CBstatist(cbstatist_t *CBstatist);



void Buffer_Manager(cbctrl_t* CB_ctrl,MAC_struc* inputMAC,int Ts,int delay);
void extractMCSinfo(cbparams_t CBparams, cbctrl_t *CBctrl);
int calculateNinfoPUSCH_RX(cbparams_t CBparams, cbctrl_t *CBctrl);
int min(int num1, int num2);
void Prepare_LDPC_info(LDPC_struc_t* LDPC_info,int graph, int CBsize,int ILS);


#endif

