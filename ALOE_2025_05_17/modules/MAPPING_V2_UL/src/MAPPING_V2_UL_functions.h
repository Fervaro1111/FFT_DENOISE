
#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H


#define Nc 1600
#define DMRS_max_length 1024
#define max_buffer_size 30

typedef struct{

	int DMRSlength;
	int OFDMsymbol;
	int Slotnumber;

}Sequences_t;

typedef struct{
	int datalength;
	int FFTlength;
	int numFFT;
	int OFDMsymb_data;
	int DMRS_length;
	int Additional_DMRS_position;//0: only 1 DMRs in symbol 2
				//1: 2 DMRS-> symbols 2 and 11
				//2: 3 DMRS-> symbols 2,7 and 11
				//3: 4 DMRS-> symbols 2,5,8 and 11
	
	int DMRS_type;		//All done for type A ->Type 1	
				// type B should be ->Type 2
	int nPRB;


}MAPP_t;

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
	int CORESETS_symbol_syze;   // number of OFDM symbols that contain CORESET prepared for 1 and 2 symbols
	int Carr_agr_lvl; //Carrier agregation level(1,2,4,8,16)

}MAC_struc;

typedef MAC_struc Bufer_MAC[max_buffer_size];


//int Checkdelay(MAC_struc* MACin,int max_size,int Ts);
//void Buffer_Manager(MAPP_t* MAPPinfo,MAC_struc* inputMAC,int Ts,int delay);

int GEN_DMRS(int Slotnumber,int OFDMsymbol,_Complex float* DMRS_seq);
void Calc_params(MAPP_t* MAPPinfo,Sequences_t* SequencesParams,int Slot_number);

void mix_data(_Complex float *DMRSseq,_Complex float *inMQAMsymb,_Complex float *out_symb,int data_length,int ofset);
void DEmix_data(int datalength,_Complex float* in, _Complex float* out);

int create_LTEspectrumNORS(_Complex float *MQAMsymb, int FFTlength, int datalength, _Complex float *out_spectrum);
void createSpectrum(MAPP_t* MAPPinfo,_Complex float* in,_Complex float* out);
void UNDO_Spectrum(MAPP_t* MAPPinfo,_Complex float* in, _Complex float *out);
int getDATAfromLTE_DOWNLINKspectrum(_Complex float *LTEspect, int FFTlength, int datalength, _Complex float *QAMsymb);

void generate_Subframe_structure(MAPP_t* MAPPinfo, int* data_per_symbol,int data_init_pos,int* DMRS_per_symbol);
void fill_structure_4_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int data_init_pos);
void fill_structure_3_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int data_init_pos);
void fill_structure_2_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int data_init_pos);
void fill_structure_1_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int data_init_pos);

int GET_Subframe(MAPP_t* MAPPinfo,int slot_num,_Complex float* inMQAMsymb,int* data_per_symbol,int* DMRS_per_symbol,_Complex float *out_data,int data_init_pos);
void gensymbols(MAPP_t* MAPPinfo,Sequences_t* SequencesParams,int Slotnumber,int OFDMsymbol,_Complex float *in_data,_Complex float* out,int *data_per_symbol,int* DMRS_per_symbol,int data_init_pos);


#endif
