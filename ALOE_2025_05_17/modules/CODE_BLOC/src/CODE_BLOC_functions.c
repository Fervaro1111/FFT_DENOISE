

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <complex.h>

//#include <oesr.h>
#include <params.h>
#include <skeleton.h>

//#include "5G_libs/frame_format/5Gtables.h"
//#include "5G_libs/frame_format/5Gbase.h"
//#include "5G_libs/CRC/CRC_functions.h"
#include "CODE_BLOC_functions.h"

//#include "5GCodeBlockSegmentCRC_functions.h"
//#include "base/pack.h"

cbsegm_t CBinfoTX;
cbsegm_t CBinfoRX;
cbparams_t CBparams;
cbstatist_t CBstatist;
cbctrl_t CBctrl;
MAC_struc MACinfo;
Bufer_MAC MAC_buff;
LDPC_struc_t LDPC_info;

// TABLES MCS 
int Qm51311[32]={2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,6,6,6,6,6,6,6,6,6,6,6,6,2,4,6};
float CodeRate51311[32]={0.2344, 0.3066, 0.3770, 0.4902, 0.6016, 0.7402, 0.8770, 1.0273,
						1.1758, 1.3262, 1.3281, 1.4766, 1.6953, 1.9141, 2.1602, 2.4063, 
						2.5703, 2.5664, 2.7305, 3.0293, 3.3223, 3.6094, 3.9023, 4.2129, 
						4.5234, 4.8164, 5.1152, 5.3320, 5.5547, 0.0000, 0.0000, 0.0000};

int CBS_T5321[51]={2,4,8,16,32,64,128,256,3,6,12,24,48,96,192,384,5,10,20,40,80,160,320,7,14,28,56,112,224,9,18,36,72,144,288,11,22,44,88,176,352,13,26,52,104,208,15,30,60,120,240};



int CBcrc_length = 24;
unsigned int CBcrc_poly = 0X1800063; 
unsigned int CBcrc_word;
crc_t CBcrc_p;

int TBcrc24_length = 24;
unsigned int TBcrc24_poly = 0x1864CFB; 
unsigned int TBcrc24_word;
crc_t TBcrc24_p;


int TBcrc16_length = 16;
unsigned int TBcrc16_poly = 0x11021; 
unsigned int TBcrc16_word;
crc_t TBcrc16_p;

crc_t *TBcrc_p;
int error=0;



void initCodeBlock(){
	if(crc_init(&CBcrc_p, CBcrc_poly, CBcrc_length))error++;
	if(crc_set_init( &CBcrc_p, 0x00000000))error++;
	if(crc_set_xor( &CBcrc_p, 0x00000000))error++;
	if(error){
		moderror_msg("Incorrect CRC initialization. Error=%d\n", error);
	}
	CBinfoTX.L_CB=CBcrc_length;
	CBinfoRX.L_CB=CBcrc_length;

}


void initTransportBlockCRC24(){
	if(crc_init(&TBcrc24_p, TBcrc24_poly, TBcrc24_length))error++;
	if(crc_set_init( &TBcrc24_p, 0x00000000))error++;
	if(crc_set_xor( &TBcrc24_p, 0x00000000))error++;
	if(error){
		moderror_msg("Incorrect CRC initialization. Error=%d\n", error);
	}
}

void initTransportBlockCRC16(){
	if(crc_init(&TBcrc16_p, TBcrc16_poly, TBcrc16_length))error++;
	if(crc_set_init( &TBcrc16_p, 0x00000000))error++;
	if(crc_set_xor( &TBcrc16_p, 0x00000000))error++;
	if(error){
		moderror_msg("Incorrect CRC initialization. Error=%d\n", error);
	}
}




/**
 *
 * @ingroup 5GCodeBlockSegmentCRC_functions
 *
 * This function performs the Transport Block Segmentation in N Code Blocks (inputs
 * blocks to LDPC) according 5G processing chain and specifications described at
 * ETSI TS 138212 v15.2.0 (2018-07) document in section 5.2.2 Code block segmentation and code
 * block CRC attachment.
 * Each code block size do not include the second CRC.
 *
 */


//struct s5GTransportBlockSz s5GTBsz;

int aloe_ComputeCodeBlockSegmentation(cbsegm_t *CBinfoL, cbparams_t CBparams, cbctrl_t CBctrl,LDPC_struc_t* LDPC_info){

	int i;
	int B;	// Number of received bits
	int L;				// CRC sequence length
	int C=0;			// Number of code blocks
//	int Kr=0;			// Number of bits for code block r
	int Kb=0;

	int B1=0;
	int K1;
	int D0=1000000, D1, ILS, Zc, Kcb;
	int error;
	float R=CBctrl.CodeRate;
	//float R=0.333;
	//printf("CODERATE_CODE_BLOCK=%f\n",R);
	
	//R=0.3;

/*	s5GTBsz.N_REs = 100;
	s5GTBsz.CodeRate= 0.33;
	s5GTBsz.Qm=2;
	s5GTBsz.v=1;
	s5GTBsz.N_subcarriersRBs=12;
	s5GTBsz.N_sh=14;
	s5GTBsz.N_DMRS=0;
	s5GTBsz.N_oh=0;
	s5GTBsz.N_RBs=6;


	f5G_TransportBlockSize(&s5GTBsz);

*/


	// TX SIDE
	// DETERMINE LDPC base graph: 1 or 2
	if(CBinfoL->A <= 292 || (CBinfoL->A<=MAXCB_KCBg2-16 && R<=0.67) || R<=0.25)CBinfoL->LDPCgraph=LDPCGRAPH2;
	else CBinfoL->LDPCgraph=LDPCGRAPH1;
	
	CBinfoL->B =  CBinfoL->A;

	B=CBinfoL->B;
	//printf("B=%d\n", B);

	if(CBinfoL->B>125000){
		printf("aloe_ComputeCodeBlockSegmentation(): ERROR!!! B=%d>125000\n", CBinfoL->B);
		error=-1;
	}

	// Determine Kcb &&
	Kcb=MAXCB_KCBg2;
	if(CBinfoL->LDPCgraph==LDPCGRAPH1)Kcb=MAXCB_KCBg1;
	

	// Total number of code blocks
	if(B <= Kcb){
		L=0;	// CRC length to ADD
		C=1;	// Number of Codeblocks
		B1=B;	// TOTAL DATA BLOCK
	}else{
		L=24;

		C=ceil((float)B/(float)(Kcb-L));
		B1=B+C*L;
	}
	K1=(int)ceil((float)B1/(float)C);

	//printf("B=%d, L=%d, K1=%d, R=%f,A=%d\n",B,L,K1,R,CBinfoL->A);

	// Number of bits in each code block: Determine Kb
	// LDPC base graph1
	if(CBinfoL->LDPCgraph==LDPCGRAPH1){
		Kb=22;
	}
	// LDPC base graph2
	if(CBinfoL->LDPCgraph==LDPCGRAPH2){
		Kb=6;
		if(B>192)Kb=8;
		if(B>560)Kb=9;
		if(B>640)Kb=10;
	}
	

	// Compute CodeBlock Size: Find minimum value of Z in lifting size [38.212-Table 5.3.2-1]
	for(i=0; i<51;i++){
		Zc=(int)CBS_T5321[i];
		if(Zc==0)Zc=1000;
		if(Kb*Zc >= K1){
			D1=Kb*Zc-K1;
			if(D1<D0){
				D0=D1;
				ILS=i;
			}
		}
	}

	
	Zc=(int)CBS_T5321[ILS];

	if(ILS<8)ILS=0;
	if(ILS>8 && ILS<16)ILS=1;
	if(ILS>16 && ILS<23)ILS=2;
	if(ILS>23 && ILS<29)ILS=3;
	if(ILS>29 && ILS<35)ILS=4;
	if(ILS>35 && ILS<41)ILS=5;
	if(ILS>41 && ILS<46)ILS=6;
	if(ILS>46 && ILS<51)ILS=7;

	//printf(" CB INNNN Kb=%d, ILS=%d, Zc=%d graph=%d\n", Kb, ILS, Zc,CBinfoL->LDPCgraph);

	//CBinfoL->ILS=ILS/8;
	CBinfoL->ILS=ILS;
	CBinfoL->K1=K1;
	CBinfoL->nofCBs=C;
	CBinfoL->L_CB=L;

	if(CBinfoL->LDPCgraph==LDPCGRAPH1)CBinfoL->CBsize=22*Zc;
	if(CBinfoL->LDPCgraph==LDPCGRAPH2)CBinfoL->CBsize=10*Zc;

	//printf("COMPUTE CODEBLOCKS   NofCodeBlocks=%d, CodeBlockSize=%d, CodeBlockDataSize=%d, CBCRCsize=%d, Zc=%d\n", C, CBinfoL->CBsize, CBinfoL->K1, CBinfoL->L_CB,Zc);
	Prepare_LDPC_info(LDPC_info,CBinfoL->LDPCgraph,Zc,ILS);

	return(error);	// 1 if OK, -1 if ERROR
}

void Prepare_LDPC_info(LDPC_struc_t* LDPC_info,int graph, int CBsize,int ILS){
/*
{2,4,8,16,32,64,128,256, -->7
3,6,12,24,48,96,192,384, 8-->15
5,10,20,40,80,160,320,   16-->22
7,14,28,56,112,224,      23-->28
9,18,36,72,144,288,      29-->34
11,22,44,88,176,352,	 35-->40
13,26,52,104,208,        41-->45
15,30,60,120,240};       46-->50
*/

	if(ILS<8)LDPC_info->ILS=0;
	if(ILS>8 && ILS<16)LDPC_info->ILS=1;
	if(ILS>16 && ILS<23)LDPC_info->ILS=2;
	if(ILS>23 && ILS<29)LDPC_info->ILS=3;
	if(ILS>29 && ILS<35)LDPC_info->ILS=4;
	if(ILS>35 && ILS<41)LDPC_info->ILS=5;
	if(ILS>41 && ILS<46)LDPC_info->ILS=6;
	if(ILS>46 && ILS<51)LDPC_info->ILS=7;

	LDPC_info->graph=graph;
	LDPC_info->Z=CBsize;
}


void init_CBstatist(cbstatist_t *CBstatist){
	int i;


	CBstatist->nofWrongCBCRCs=0;
	CBstatist->nofWrongTBCRSs=0;
	for(i=0; i<HISTOSZ; i++){
		CBstatist->wrongCBCRC[i]=0;
		CBstatist->histowrongCBCRC[i]=0;
	}
}


/**
	params: CBinfoL
	return(): Number of output bytes after deleting CRCs.

*/




/**
 *
 * @ingroup lte_segmentation
 *
 * This function performs the Transport Block Segmentation in N Code Blocks (inputs
 * blocks to turbodecoder) according LTE processing chain and specifications described at
 * ETSI TS 136212 v10.6.0 (2012-07) document in section 5.1.2 Code block segmentation and code
 * block CRC attachment.
 * Each code block size do not include the second CRC.
 *
 */

int get_nof_cb(int recv_bits, int *nof_short_cb, int *nof_long_cb,
		int *len_short_cb, int *len_long_cb, int *nof_filler_bits) {

	int num_cb;
/*	int i, Bp, Ak;
*/
	/** Calculate Number of output code blocks*/
/*	if (recv_bits <= Z) {
		num_cb = 1;
		*nof_long_cb = 1;
		*nof_short_cb = 0;
		*len_short_cb = 0;
		if (recv_bits < 40) {
			*len_long_cb = 40;
			*nof_filler_bits = Bmin - recv_bits;
		} else {
			*len_long_cb = recv_bits;
			*nof_filler_bits = 0;
		}
	}

	if (recv_bits > Z) {
		num_cb = CEILING((float)recv_bits/(float)(Z - L));
		Bp = recv_bits + num_cb * L;
*/
		/** Calculate code block sizes*/
/*		for (i = 1; i < 189; i++) {*/
			/** First Segmentation size: K+*/
/*			*len_long_cb = Table5133[i];*/
			/** Second Segmentation Size: K-*/
/*			*len_short_cb = Table5133[i - 1];
			if (Table5133[i] * num_cb >= Bp) {
				break;
			}
		}
		if (num_cb == 1) {
*/
			/** C+ :Number of segments of size K+ (Kp)*/
//			*nof_long_cb = 1;
//			*len_short_cb = 0; /** K- */
			/** C- :Number of segments of size K- (Km)*/
//			*nof_short_cb = 0;
			/** Number of Filler Bits*/
/*			*nof_filler_bits = Bmin - recv_bits;
		}
		if (num_cb > 1) {
			Ak = *len_long_cb - *len_short_cb;
			*nof_short_cb = (int) floor(
					((float) (num_cb * *len_long_cb) - Bp) / (float) Ak);
			*nof_long_cb = num_cb - *nof_short_cb;
*/
			/** Number of Filler Bits*/
/*			*nof_filler_bits = *nof_long_cb * *len_long_cb
					+ *nof_short_cb * *len_short_cb - Bp;
		}
		*len_long_cb = *len_long_cb - 24;
		if (*len_short_cb > 0)
			*len_short_cb = *len_short_cb - 24;
	}
*/
	return num_cb;
}



/**
 *
 * @ingroup calculateNinfoPUSCH_RX 
 * https://www.sharetechnote.com/html/5G/5G_MCS_TBS_CodeRate.html#PUSCH_TBS
 * This function calculates the number of transmitted bits without 
 * redundancy neither CRC according received DCI, Transfrom Precoding and...
 *
 */


int calculateNinfoPUSCH_RX(cbparams_t CBparams, cbctrl_t *CBctrl){



	int Ninfo=0;
	int N1_RE=0, N_RE=0;
	int N_RB_sc=12;




	// Calculate N1_RE
	N1_RE=N_RB_sc*CBctrl->Nsh_symb-CBctrl->N_PRB_DMRS-CBctrl->Noh_PRB;

	// Calculate the total number of REs for PUSCH
	N_RE=(min(156, N1_RE))*CBparams.nPRBs;

	// Calculate Ninfo
	Ninfo=N_RE*CBctrl->CodeRate*CBctrl->Qm*CBparams.v;

	if(CBparams.debugg==SEGMENT_DEBUGG)	printf("calculateNinfoPUSCH_RX(): Ninfo=%d\n", Ninfo);
	return(Ninfo);

}



void Buffer_Manager(cbctrl_t* CB_ctrl,MAC_struc* inputMAC,int Ts,int delay){

	MAC_struc out_buff;

	int buff_in_position=Ts%max_buffer_size;
	//if(Ts%max_buffer_size==0)buff_in_position=0;
	int buff_out_position=buff_in_position-delay;
	MAC_buff[buff_in_position]=*inputMAC;

	//printf("Ts=%d and delay=%d\n", Ts,delay);
	/*for(int i=0;i<30;i++){
		printf("coderateMCS=%f and TB_size_TBCRC=%d at position=%d\n", MAC_buff[i].coderateMCS,MAC_buff[i].TB_size_TBCRC,i);

	}*/

	if (buff_out_position<0){
		buff_out_position=buff_out_position+(max_buffer_size);
	}
	//printf("Tbuff_out_position=%d\n",buff_out_position);
	 
	out_buff=MAC_buff[buff_out_position];

	CB_ctrl->CodeRate=out_buff.coderateMCS;
	CB_ctrl->TB_CRC=out_buff.TB_size_TBCRC;

}


void extractMCSinfo(cbparams_t CBparams, cbctrl_t *CBctrl){

	if(CBparams.I_MCS_TABLE==0){
		CBctrl->Qm=Qm51311[CBparams.I_MCS];
		CBctrl->CodeRate=CodeRate51311[CBparams.I_MCS];
		//CBctrl->CodeRate=MAC_data[]	
}
	//TODO: Fixed values now
	CBctrl->Nsh_symb=14;
	CBctrl->N_PRB_DMRS=0;
	CBctrl->Noh_PRB=0;
	if(CBparams.debugg==SEGMENT_DEBUGG)	printf("extractMCSinfo(): Qm=%d, CodeRate=%f\n", CBctrl->Qm, CBctrl->CodeRate);

}


int min(int num1, int num2) 
{
    return (num1 > num2 ) ? num2 : num1;
}

int buffer(int buffer_size,unsigned int new_value, int *buffer){
	int buffer_out=buffer[buffer_size-1];

	int i;
	for (i=buffer_size-1;i>=0;i--){
		buffer[i+1]=buffer[i];
	//	printf("Buffer data_CODE_BLOCK in pos %d is %d\n",i,buffer[i]);
	}	
	buffer[0]=new_value;

	return buffer_out;
}


int aloe_CreateCodeBlocks(char *inBytes, char *outBytes, cbsegm_t *CBinfoL, cbparams_t CBparams, cbstatist_t *CBstatist, crc_t crc_p){

	int i,j, k;
	int M=CBinfoL->K1-CBinfoL->L_CB;
	int K=CBinfoL->CBsize;
	int K1=CBinfoL->K1;
	int F=K-CBinfoL->K1;
	int C=CBinfoL->nofCBs;
	int L=CBinfoL->L_CB;
	int inBytesLength=CBinfoL->B/8;

//	unsigned int crc_poly = LTE_CRC24B; //0X1800063
	unsigned int crc;
	int error=0;
	int outNumBytes=0;



	char *pter;
	char bitsIN[MAXINPUTBYTES*8];
	char bitsCRC[MAXCB_KCBg1];
	char bitsOUT[MAXINPUTBYTES*8];

	char bytesOUT[MAXINPUTBYTES];


	char bitsT[MAXINPUTBYTES*8];

	char value;


	// FORMAT INPUT BYTES FLOW TO BITS
	for(i=0; i<inBytesLength; i++){
		pter=(char *)(bitsIN+i*8);
		byte2bitsLSBF(*(inBytes+i), &pter, 8);
		
}


	// DO FOR EACH CODEBLOCK
	if(C>1){
		
		for(k=0; k<C; k++){
			// FILL ZEROS
			memset(bitsOUT+K*k+M, '\0', K-M);
			// ADD DATA
			memcpy(bitsOUT+K*k, bitsIN+M*k, M);
			// ADD CRC
			crc=crc_attach(bitsOUT+K*k, M, &crc_p);
		}

		// PRINT CODEBLOCK
		//j=0;
		//for(i=0; i<K*C; i++){
			//printf("%d", bitsOUT[i]);
			//j++;
			//if(j==256){
				//j=0;
			//	printf("\n");
		//	}
	//	}
		//printf("\n");
	//	printf("A\n");


		// PACK BITS TO BYTES FOR OUTPUT
		outNumBytes=(K*C)/8;
		//printf("PACK BITS TO BYTES FOR OUTPUT_SEGMENTATION: K=%d, C=%d, outNumBytes=%d\n", K, C, outNumBytes);
		for(i=0; i<outNumBytes; i++){
			pter=(char *)(bitsOUT+i*8);
			bits2byteLSBF(&outBytes[i], &pter, 8);
			//bits2byteLSBF(&outBytes[i], &pter, 8);	
		}
	}
	// ONLY ONE CODE BLOCK
	if(C==1){
		//printf("C==1: K1=%d, K=%d\n", K1, K);
		memcpy(bitsCRC, bitsIN, K1);
//printf("B0\n");
		// FILL ZEROS
		for(i=K1; i<K; i++)bitsCRC[i]='\0';
//printf("B1\n");
		// PACK BITS TO BYTES FOR OUTPUT
		outNumBytes=K/8;
		for(i=0; i<outNumBytes; i++){
			pter=(char *)(bitsCRC+i*8);
			bits2byteLSBF(&outBytes[i], &pter, 8);
			//bits2byteLSBF(&outBytes[i], &pter, 8);
		}



//printf("B2\n");
	}

/*
printf("B3\n");
	for(i=1; i<=outNumBytes; i++){
		//(*(out+k-1)=k+32;
		printf("%c ", (char)(*(bytesOUT+i-1))&0xFF);
		//printf("%c ", (char)(*(outBytes+i-1))&0xFF);
		if(i%64==0)printf("\n");
	}
	printf("\n");
printf("B4\n");
*/

/*
	printf("OUTPUT bytes aloe_CreateCodeBlocks\n");
	for(i=1; i<outNumBytes+1; i++){
		printf("%02x", (int)(outBytes[i-1]&0xFF));
		if(i%80==0)printf("\n");
	}
	printf("\n");	
	
*/
	return(outNumBytes);
}



int aloe_5GCheckCRC(char *inBytes, char *outBytes, cbsegm_t *CBinfoL, cbstatist_t *CBstatist, crc_t crc_p){

	int i, k, j=0, l;
	unsigned int crc;
	char *pter;
	char bitsOUT[MAXINPUTBYTES*8];
	char bitsOUTaux[MAXINPUTBYTES*8];
	int K=CBinfoL->CBsize;
	int C=CBinfoL->nofCBs;
	int K1=CBinfoL->K1;
	int inBitsLength=CBinfoL->B;
	int L=CBinfoL->L_CB;
	int M=CBinfoL->K1-CBinfoL->L_CB;
	int F=K-CBinfoL->K1;
	int nofOutBytes, nofOutBits=0;
/*
#ifdef SEGMENT_DEBUGG
	if(inBitsLength>MAXINPUTBYTES*8){
		printf("5GCodeBlocksSegmentCRC.aloe_5GCheckCRC(): ERROR!!! inBitsLength=%d>MAXINPUTBYTES*8=%d\n", inBitsLength, MAXINPUTBYTES*8);
	}
	printf("aloe_5GCheckCRC(): C=%d, K=%d, K1=%d, M=%d, F=%d, inBitsLength=%d\n", CBinfoL->nofCBs, K, K1, M, F, inBitsLength);
	printf("aloe_5GCheckCRC(): C=%d, K=%3.1f(B), K1=%3.1fb, M=%3.1fb, F=%3.1fb\n", CBinfoL->nofCBs, ((float)K)/8.0, ((float)K1), ((float)M), ((float)F));
#endif
*/
	// FORMAT INPUT BYTES FLOW TO BITS
	for(i=0; i<K*C; i++){
		pter=(char *)(bitsOUT+i*8);
		byte2bitsLSBF(*(inBytes+i), &pter, 8);
	}

	// PRINT RECEIVED DATA IN BIT FORMAT
/*		printf("PRINT RECEIVED DATA IN BIT FORMAT\n");
		l=0;
		for(i=0; i<K*C; i++){
			printf("%d", bitsOUT[i]);
			l++;
			if(l==256){
				l=0;
				printf("\n");
			}
		}
		printf("\n");
*/

	// CHECK CRC OF EACH CODEBLOCK
	//CBinfoL->nofWrongCRCs=0;
	//printf("C=%d, K=%d, K1=%d\n", C, K, K1);
	if(C>1){
		for(k=0; k<C; k++){
			//crc=crc_attach(bitsOUT+k*K, K1, &crc_p);
		  	crc=check_CRC(bitsOUT+k*K,K1, &crc_p);
			//CBstatist->wrongCBCRC[j]=-1;
			//printf("CHECK: CB CRC%d=%x\n", k, crc);
			if(crc != 0x00){
				CBstatist->wrongCBCRC[j]=k;
				CBstatist->nofWrongCBCRCs++;
				CBstatist->histowrongCBCRC[k] +=1;
				j++;
			}
			memcpy(bitsOUTaux+k*M, bitsOUT+k*K, M);
			nofOutBits += M;
		}
	}
	if(C==1){
		memcpy(bitsOUTaux, bitsOUT, M);
		nofOutBits = M;
	}



	
	// PACK BITS TO BYTES FOR OUTPUT

	nofOutBytes=(int)ceil((double)((float)nofOutBits/8.0));
	//printf("NOF_OUT_BITS_UNCRC_CB=%d, nofOutBytes=%d\n", nofOutBits, nofOutBytes);
	for(i=0; i<nofOutBytes; i++){
		pter=(char *)(bitsOUTaux+i*8);
		bits2byteLSBF(&outBytes[i], &pter, 8);
	}


/*
	printf("OUTPUT bytes aloe_5GCheckCRC\n");
	for(i=1; i<nofOutBytes+1; i++){
		printf("%02x", (int)(outBytes[i-1]&0xFF));
		if(i%128==0)printf("\n");
	}
	printf("\n");	
*/
	return(nofOutBytes); 

}



void gen_crc_table(crc_t *crc_params) {

	int i, j, ord=(crc_params->order-8);
	unsigned long bit, crc;

	for (i=0; i<256; i++) {
		crc = ((unsigned long)i)<<ord;
		for (j=0; j<8; j++) {
			bit = crc & crc_params->crchighbit;
			crc<<= 1;
			if (bit) crc^= crc_params->polynom;
		}
		crc_params->table[i]=crc & crc_params->crcmask;
	}
}


unsigned long  crctable (unsigned long length, crc_t *crc_params) {

	// Polynom order 8, 16, 24 or 32 only.
	int ord=crc_params->order-8;
	unsigned long crc = crc_params->crcinit;
	unsigned char* data = crc_params->data0;

//	printf("crctable():crc_par->data0=%p\n", (void *)crc_params->data0);

	while (length--){
		 crc = (crc << 8) ^ crc_params->table[ ((crc >> (ord)) & 0xff) ^ *data++];
	}
	return((crc ^ crc_params->crcxor) & crc_params->crcmask);
}

unsigned long reversecrcbit(unsigned int crc, int nbits, crc_t *crc_params) {

	unsigned long m, rmask=0x1;

	for(m=0; m<nbits; m++){
		if((rmask & crc) == 0x01 )crc = (crc ^ crc_params->polynom)>>1;
		else crc = crc >> 1;
	}
	return((crc ^ crc_params->crcxor) & crc_params->crcmask);
}

int crc_set_init(crc_t *crc_par, unsigned long crc_init_value){

	crc_par->crcinit=crc_init_value;
	if (crc_par->crcinit != (crc_par->crcinit & crc_par->crcmask)) {
		printf("ERROR, invalid crcinit in crc_set_init().\n");
		return(-1);
	}
	return(0);
}
int crc_set_xor(crc_t *crc_par, unsigned long crc_xor_value){

	crc_par->crcxor=crc_xor_value;
	if (crc_par->crcxor != (crc_par->crcxor & crc_par->crcmask)) {
		printf("ERROR, invalid crcxor in crc_set_xor().\n");
		return(-1);
	}
	return(0);
}

int crc_init(crc_t *crc_par, unsigned int crc_poly, int crc_order){

	// Set crc working default parameters
	crc_par->polynom=crc_poly;
	crc_par->order=crc_order;
	crc_par->crcinit=0x00000000;
	crc_par->crcxor=0x00000000;

	// Compute bit masks for whole CRC and CRC high bit
	crc_par->crcmask = ((((unsigned long)1<<(crc_par->order-1))-1)<<1)|1;
	crc_par->crchighbit = (unsigned long)1<<(crc_par->order-1);

	// check parameters
	if (crc_par->order%8 != 0) {
		printf("ERROR, invalid order=%d, it must be 8, 16, 24 or 32.\n", crc_par->order);
		return(-1);
	}
	if(crc_set_init( crc_par, crc_par->crcxor))return(-1);
	if(crc_set_xor( crc_par, crc_par->crcxor))return(-1);

	// generate lookup table
	gen_crc_table(crc_par);

	// Alloocate memory
/*	crc_par->data0 = (unsigned char *)malloc(sizeof(*crc_par->data0) * (MAX_LENGTH+crc_par->order));
	printf("crc_init(): crc_par->data0=%p\n", (void *)crc_par->data0);
	if (!crc_par->data0) {
		perror("malloc ERROR: Allocating memory for data pointer in crc() function");
		return(-1);
	}
*/
	return(0);
}

void crc_free(crc_t *crc_p){
//	free(crc_p->data0);
//	crc_p->data0=NULL;
}



unsigned int crc_attach(char *bufptr, int len, crc_t *crc_params) {

	int i, len8, res8, a=0;
	unsigned int crc;
	char *pter;

	if(len > MAX_LENGTH){
		perror("Data lenght ERROR: Input data lenght exceeds available memory (MAX_LENGTH)");
		return(-1);
	}

	//# Pack bits into bytes
	len8=(len>>3);
	res8=(len - (len8<<3));
	if(res8>0)a=1;

	// Move to char format
	for(i=0; i<len8; i++){
		pter=(char *)(bufptr+8*i);
		crc_params->data0[i]=(unsigned char)(unpack_bits(&pter, 8)&0xFF);
	}
	crc_params->data0[len8]=0x00;
	for(i=0; i<res8; i++){
		crc_params->data0[len8] |= ((unsigned char)*(pter+i))<<(7-i);
	}

	// Calculate CRC
	crc=crctable(len8+a, crc_params);

	// Reverse CRC res8 positions
	if(a==1)crc=reversecrcbit(crc, 8-res8, crc_params);

	// Add CRC
	pter=(char *)(bufptr+len);
	pack_bits(crc, &pter, crc_params->order);

	//Return CRC value
	return crc;
}

void pack_bits(unsigned int value, char **bits, int nof_bits)
{
    int i;

    for(i=0; i<nof_bits; i++) {
        (*bits)[i] = (value >> (nof_bits-i-1)) & 0x1;
    }
    *bits += nof_bits;
}

unsigned int unpack_bits(char **bits, int nof_bits)
{
    int i;
    unsigned int value=0;

    for(i=0; i<nof_bits; i++) {
    	value |= (*bits)[i] << (nof_bits-i-1);
    }
    *bits += nof_bits;
    return value;
}

int add_CRC(char *bufptr, int datalen, crc_t *crc_params){
	int i;
	unsigned int crc;
	char a, b, c;
	int order=(crc_params->order)/8;
	int order2=(order-1)*8;

	memcpy(crc_params->data0, bufptr, sizeof(char)*datalen);
	crc=crctable((unsigned long)datalen, crc_params);
//	printf("ADD_CRC(): crc=%x\n", crc);
	for(i=0; i<order; i++){
		bufptr[datalen+i]=(char)((crc>>order2-i*8)&0xFF);
	}

	return(1);
}

int check_CRC(char *bufptr, int datalen, crc_t *crc_params){
	int crc;
	//CHECK CRC
	memcpy(crc_params->data0, bufptr, sizeof(char)*datalen);
	crc=crctable((unsigned long)datalen, crc_params);
//	printf("CHECK_CRC(): crc=%x\n", crc);
	return(crc);
}

// Left Significant bit first
void byte2bitsLSBF(char value, char **bits, int nof_bits)
{
    int i;
    for(i=0; i<8; i++) {
		if(i==nof_bits)break;
        (*bits)[i] = (value >> (8-i-1)) & 0x1;
    }
}

// Left Significant bit first
void bits2byteLSBF(char *byte, char **bits, int nof_bits)
{
    int i;
    char value=0;

	*byte = 0;
    for(i=0; i<8; i++) {
		if(i==nof_bits)break;
    	*byte |= (*bits)[i] << (8-i-1);
    }
}





