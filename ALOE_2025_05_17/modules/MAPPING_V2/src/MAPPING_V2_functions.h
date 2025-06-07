
#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H


#define Nc 1600
#define PSSlength 127
#define SSSlength 127
#define DMRS_max_length 1024
#define SS_block_length 240
#define PSS_offset 56 //offset relative to the start of the SSblock in the frequency axis

#define DMRS_SEPARATION_SSB 4 //Number of carriers between DMRS inside SSB
#define fisrt_DMRS_SSB //Number of carrier within SSB that contains first DMRS
#define DMRS_SEPARATION_Control_Channel 4
#define first_DMRS_Control_channel 0


#define max_buffer_size 120

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
	int nPRB;
	int Agreggation_lvl_DCI;


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
	int CORESETS_symbol_size;   // number of OFDM symbols that contain CORESET prepared for 1 and 2 symbols
	int Agr_lvl_DCI; //Carrier agregation level for DCI (1,2,4,8,16)

	int NID1;//PARAMETERS to generate PSS, SSS and CELL ID
	int NID2;
	int FFTsize;

}MAC_struc;

typedef MAC_struc Bufer_MAC[max_buffer_size];


//Strucure 
void generate_Subframe_structure(MAPP_t* MAPPinfo, int* data_per_symbol,int SSblock_flag,int data_init_pos,int* DMRS_per_symbol);
void first_symbols(int data_init_pos,int* data_per_symbol,MAPP_t* MAPPinfo,int* DMRS_per_symbol);
void fill_structure_1_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int SSblock_flag,int data_init_pos);
void fill_structure_2_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int SSblock_flag,int data_init_pos);
void fill_structure_3_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int SSblock_flag,int data_init_pos);
void fill_structure_4_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int SSblock_flag,int data_init_pos);


//Sequences
int SSS_gen (int NID1,int NID2);
int PSS_gen (int NID1,int NID2);
int GEN_DMRS(int Slotnumber,int OFDMsymbol,_Complex float* DMRS_seq);
void calc_sequences(Sequences_t* SequencesParams,MAPP_t* MAPPinfo);



//TX
void mix_data(_Complex float *DMRSseq,_Complex float *inMQAMsymb,_Complex float *out_symb,int data_length,int ofset);
int Do_Mapping(MAPP_t* MAPPinfo,_Complex float *inMQAMsymb,int* data_per_symbol,int SSblock_flag,_Complex float *subframe,int data_init_pos,int* DMRS_per_symbol,int rcv_samples_controlchann,_Complex float* control_data);
void gensymbols(MAPP_t* MAPPinfo,Sequences_t* SequencesParams,int Slotnumber,int OFDMsymbol,_Complex float *in_data,_Complex float* out,int *data_per_symbol,int* DMRS_per_symbol,int data_init_pos,int SS_block_flag);

void Create_Subframe(MAPP_t* MAPPinfo,int slot_num,_Complex float* inMQAMsymb,int* data_per_symbol,int* DMRS_per_symbol,_Complex float *tmp,int data_init_pos,int SS_block_flag);
int create_LTEspectrumNORS(_Complex float *MQAMsymb, int FFTlength, int datalength, _Complex float *out_spectrum);
void createSpectrum(MAPP_t* MAPPinfo,_Complex float* in,_Complex float* out);



void Calc_params(MAPP_t* MAPPinfo,Sequences_t* SequencesParams,int Slot_number);
int FFT_length_calc(MAPP_t* MAPPinfo);
void fill_zeros(MAPP_t* MAPPinfo,_Complex float* out,int ofset);


//RX
void UNDO_Spectrum(MAPP_t* MAPPinfo,_Complex float* in, _Complex float *out);
int UNDO_Mapping(MAPP_t* MAPPinfo,_Complex float *inspectrum,int* data_per_symbol,int SSblock_flag,_Complex float *dataout,int data_init_pos,int* DMRS_per_symbol,_Complex float* out_control_data, int* snd_samples_control);
void GETsymbols(MAPP_t* MAPPinfo,int Slotnumber,int OFDMsymbol,_Complex float *in_data,_Complex float* out,int *data_per_symbol,int* DMRS_per_symbol,int data_init_pos,int SS_block_flag);
int GET_Subframe(MAPP_t* MAPPinfo,int slot_num,_Complex float* inMQAMsymb,int* data_per_symbol,int* DMRS_per_symbol,_Complex float *out_data,int data_init_pos,int SS_block_flag);
void DEmix_data(int datalength,_Complex float* in, _Complex float* out);

//Buffer
int Checkdelay(MAC_struc* MACin,int max_size,int Ts);
void Buffer_Manager(MAPP_t* MAPPinfo,MAC_struc* inputMAC,int Ts,int delay);


//PDCCH TX
int mix_control_data(_Complex float *DMRSseq,_Complex float *inMQAMsymb,_Complex float *out_symb,int control_data_length);
void Fill_PDCCH(_Complex float *DMRSseq,_Complex float *inMQAMsymb,_Complex float *inout_frame,int control_data_length, int data_init_pos,MAPP_t* MAPPinfo,int Slotnumber);
int GEN_DMRS_Control(int Slotnumber,int OFDMsymbol,_Complex float* DMRS_seq);

//PDCCH RX
int calc_out_bits_max_length(int Carr_agr_lvl,int CORESET_symb_num);
int Calc_symb_out_control(int aggregation_level_DCI,int CORESET_symb_num);
int demix_control_data(_Complex float *inOFDMsymb,_Complex float *out_symb,int length);
int GET_PDCCH(_Complex float *outMQAMsymb,_Complex float *in_frame,int data_init_pos,MAPP_t* MAPPinfo);

#endif
