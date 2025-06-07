/* 
 * Copyright (c) 2012
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

/* Functions that generate the test data fed into the DSP modules being developed */
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

#include "RATE_MATCH_functions.h"

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */


infoRM_t RM_info;
MAC_struc MACinfo;
Bufer_MAC MAC_buff;



int CBS_T5321[51]={2,4,8,16,32,64,128,256,3,6,12,24,48,96,192,384,5,10,20,40,80,160,320,7,14,28,56,112,224,9,18,36,72,144,288,11,22,44,88,176,352,13,26,52,104,208,15,30,60,120,240};





int Checkdelay(MAC_struc* MACin,int max_size,int Ts){
	int delay;
	delay=MACin->Ts_MAC;
	return delay;

}


void Buffer_Manager(infoRM_t* RM_info,MAC_struc* inputMAC,int Ts,int delay){

	MAC_struc out_buff;

	int buff_in_position=Ts%max_buffer_size;
	//if(Ts%max_buffer_size==0)buff_in_position=0;
	int buff_out_position=buff_in_position-delay;
	MAC_buff[buff_in_position]=*inputMAC;

	if (buff_out_position<0){
		buff_out_position=buff_out_position+(max_buffer_size);
	}
	 
	out_buff=MAC_buff[buff_out_position];

	//printf("TS_MAC out from RM %d \n",out_buff.Ts_MAC);
	//printf("--------------NEW FEATURE--------- rv=%d mod_qam=%d   coderate=%f\n" ,out_buff.TB_size_TBCRC,out_buff.QAM_order,out_buff.coderateMCS);

	//Advance_buffer(Bufffer,inputMAC,max_buffer_size);
	extract_MAC_info(RM_info,&out_buff);



}


int extract_MAC_info(infoRM_t *RM_info, MAC_struc *MACinfo){
	RM_info->coderate=MACinfo->coderateMCS;
	RM_info->TB_TBCRC=MACinfo->TB_size_TBCRC;
	RM_info->mod_order=MACinfo->QAM_order;
	RM_info->num_layers=MACinfo->num_layers;
	RM_info->REs_DMRS=MACinfo->REs_DMRS;
	RM_info->rv=MACinfo->rv;
	RM_info->Symb_X_slot=MACinfo->Symb_X_slot;
	RM_info->nPRB=MACinfo->nPRB;
	RM_info->free_RE=MACinfo->free_RE;
	return 0;
}

int Select_k0(int graph,int rv_id, int Ncb,int Zc){
	int k0;
	if (graph==1){
		switch (rv_id){
		case 0:
			k0=0;
			break;
		case 1:
			k0=Zc*floor(17*Ncb/(66*Zc));
			break;
		case 2:
			k0=Zc*floor(33*Ncb/(66*Zc));
			break;
		case 3:
			k0=Zc*floor(56*Ncb/(66*Zc));
			break;
		}
	}else{
		switch (rv_id){
		case 0:
			k0=0;
			break;
		case 1:
			k0=Zc*floor(13*Ncb/(50*Zc));
			break;
		case 2:
			k0=Zc*floor(25*Ncb/(50*Zc));
			break;
		case 3:
			k0=Zc*floor(43*Ncb/(50*Zc));
			break;
		}
	}
	return k0;
}

int RateMatch_OutLength(int mod_order,int num_layers,int G, int C, int *Er){
	int j=0;
	for(int r=0;r<C;r++){
		if (j<=(C-((G/(num_layers*mod_order))%C))-1){
			*(Er+r)=num_layers*mod_order*floor(G/(num_layers*mod_order*C));
		}else{
			*(Er+r)=num_layers*mod_order*ceil(G/(num_layers*mod_order*C));
		}
		j++;
	}
	return 0;
	
}


int interleave(int E,int mod_order,char *in, char *out){
	for(int j=0;j<E/mod_order;j++){
		for (int i=0;i<mod_order;i++){
			*(out+i+j*mod_order)=in[(i*E/mod_order)+j];
			//printf("%f",*(out+i+j*mod_order));
		}
	}
	return 0;
	
}
int DE_interleave(int E,int mod_order,char *in, char *out){
	for(int j=0;j<E/mod_order;j++){
		for (int i=0;i<mod_order;i++){
			*(out+(i*E/mod_order)+j)=in[i+j*mod_order];
			//printf("%02x",(int) *(out+i+j*mod_order)&0xFF);
		}
	}
	return 0;
	
}
int DE_interleave_F(int E,int mod_order,float *in, float *out){
	for(int j=0;j<E/mod_order;j++){
		for (int i=0;i<mod_order;i++){
			*(out+(i*E/mod_order)+j)=in[i+j*mod_order];
			//printf("%02x",(int) *(out+i+j*mod_order)&0xFF);
		}
	}
	return 0;
	
}

void puncture_initial_cols(char *in, char *out,infoRM_t RM_info,int length){
	if(RM_info.graph==1){
		memcpy(out,in+RM_info.Zc*2,length-RM_info.Zc*2);
	}else{
		memcpy(out,in+RM_info.Zc*2,length-RM_info.Zc*2);
	}	

}

void de_puncture_initial_cols(float *in, float *out,infoRM_t RM_info,int length){
	if(RM_info.graph==1){
		memcpy(out+RM_info.Zc*2,in,(length-RM_info.Zc*2)*sizeof(float));
		for(int i=0;i<RM_info.Zc*2;i++){
			out[i]=0.0;
		}
	}else{
		memcpy(out+RM_info.Zc*2,in,(length-RM_info.Zc*2)*sizeof(float));
		for(int i=0;i<RM_info.Zc*2;i++){
			out[i]=0.0;
		}
	}	
}
	
int bit_selection(int E,char *in, char *out, int Ncb,int k0,int rcv_samples){
	int k=0;
	int j=0;
	int j2=0;
	
	

	//printf("-----------Ncb in selection------------=%d\n",Ncb);

	while (k<E){
		if ((k0+j) <rcv_samples){
			*(out+k)=in[(k0+j)%Ncb];
			//int a=(k0+j)%Ncb;
  			//printf("(k0+j)%Ncb = %d    j=%d      ",(k0+j)%Ncb,j);
			j++;		
		}else{
			*(out+k)=in[(j2)%Ncb];
			//printf("(k0+j2)%Ncb = %d    j2=%d     ",(k0+j2)%Ncb,j2);
			j2++;
			//j++;
			//printf("BitSelection");
			//printf("%d",out+k)
			

		}
	//printf("j= %d\n",j);	
	k++;
	}	
	
	
	return 0;
}
	
int position_recover(int E, int *out, int Ncb,int rcv_samples,int k0){
	int k=0;
	int j=0;
	int j2=0;

	while (k<E){
		if ((k0+j) <rcv_samples){
			*(out+k)=(k0+j)%Ncb;
			//int a=(k0+j)%Ncb;
  			//printf("(k0+j)%Ncb = %d    j=%d     ",j);
			j++;		
		}else{
			*(out+k)=j2%Ncb;
			//printf("(k0+j2)%Ncb = %d    j2=%d     ",(k0+j2)%Ncb);
			j2++;
			//j++;
			//printf("BitSelection");
			//printf("%d",out+k)
			

		}
	//printf("j= %d\n",j);	
	k++;
	}	
	
	return 0;
	
}
	
int bit_recover(int *positions, char *in, char *out,int E,int rcv_samples){
	
	for(int i=0;i<E;i++){
		out[positions[i]]=in[i];
	}
	

	
	return 0;
}

int bit_recover_F(int *positions, float *in, float *out,int E,int rcv_samples){
	
	for(int i=0;i<E;i++){
		out[positions[i]]=in[i];
	}
	for(int i=E;i<rcv_samples;i++){
		out[i]=0.0;
	}

	
	
	return 0;
}
	
int Trnaspose(infoRM_t RM_info,char* in,char* out){
	int cols;
	//char tmp[2048*24];
	if(RM_info.graph==1)cols=68;	
	if(RM_info.graph==1)cols=52;	
	for(int i=0;i<cols;i++){
		for(int j=0;j<RM_info.Zc;j++){
			out[j+i*RM_info.Zc]=in[ i+j*RM_info.Zc];
		}
	}


}

int de_Trnaspose(infoRM_t RM_info,float* in,float* out){
	int cols;
	//char tmp[2048*24];
	if(RM_info.graph==1)cols=68;	
	if(RM_info.graph==1)cols=52;	
	for(int i=0;i<cols;i++){
		for(int j=0;j<RM_info.Zc;j++){
			out[RM_info.Zc*j+i]=in[ i*RM_info.Zc+j];
		}
	}


}

int Calc_CB(infoRM_t *RM_info){
	int graph,k1,B1,Kb,C,Zc,L,D1,ILS2,i,K1;
	int D0=1000000;
	float R=RM_info->coderate;
	int B=RM_info->TB_TBCRC*8;
	
	//printf("UN RATEMATCH TB_TBCRC in =%d coderate in =%f\n",B,R);

//Necessito saber C,TBS,R,

	
	if(B<= 292 || (B<=MAXCB_KCBg2-16 && R<=0.67) || R<=0.25){
		graph=2;
	}else{
 		graph=1;
	}
	//printf("graph=%d\n",graph);

	// Determine Kcb
	int Kcb=MAXCB_KCBg2;
	if(graph==1)Kcb=MAXCB_KCBg1;

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

	//printf("B1=%d,K1=%d\n",B1,K1);
	if(graph==1){
		Kb=22;
	}
	// LDPC base graph2
	if(graph==2){
		Kb=6;
		if(B>192)Kb=8;
		if(B>560)Kb=9;
		if(B>640)Kb=10;
	}

	//printf("Kb=%d",Kb);

	// Compute CodeBlock Size: Find minimum value of Z in lifting size [38.212-Table 5.3.2-1]
	for(i=0; i<51;i++){
		Zc=(int)CBS_T5321[i];
		if(Zc==0)Zc=1000;
		if(Kb*Zc >= K1){
			D1=Kb*Zc-K1;
			if(D1<D0){
				D0=D1;
				ILS2=i;
			}
		}
	}

	
	//printf("ILS = %d\n",ILS2);
	Zc=(int)CBS_T5321[ILS2];
	

	if(ILS2<8)ILS2=0;
	if(ILS2>8 && ILS2<16)ILS2=1;
	if(ILS2>16 && ILS2<23)ILS2=2;
	if(ILS2>23 && ILS2<29)ILS2=3;
	if(ILS2>29 && ILS2<35)ILS2=4;
	if(ILS2>35 && ILS2<41)ILS2=5;
	if(ILS2>41 && ILS2<46)ILS2=6;
	if(ILS2>46 && ILS2<51)ILS2=7;


	//printf("Zc=%d\n",Zc);
	if(graph==1){
		RM_info->CB_size=22*Zc;
		//RM_info->N=68*Zc;
		RM_info->N=3*RM_info->CB_size;
	}
	if(graph==2){
		RM_info->CB_size=10*Zc;
		//RM_info->N=52*Zc;
		RM_info->N=3*RM_info->CB_size;
	}
	
	//printf("COMPUTE CODEBLOCKS IN UN-RM   NofCodeBlocks=%d, CodeBlockSize=%d, CodeBlockDataSize=%d, Zc=%d, ILS=%d, graph=%d\n", C, RM_info->CB_size, K1, Zc,ILS2,graph);

	RM_info->Zc=Zc;
	RM_info->graph=graph;
	RM_info->C=C;

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


int Concatenate_inputs(int max_in_interfaces,char* INbytes, char* input,int rcv_samples){

int in_interfaces=0;
	
	for (int i=0; i<max_in_interfaces-1;i++){//el -1 es pq l'ultima ve de la MAC i no del bloc anterior
		if(get_input_samples(i)!=0){
		in_interfaces++;
		}
	}
	//int rcv_samples = get_input_samples(0);
	//printf("RCV TOTAL sRATE MATCH=%d\n",rcv_samples*(in_interfaces));
	
	for(int i=0;i<in_interfaces;i++){
		memcpy(INbytes+rcv_samples*i,input+80*2048*i,rcv_samples);
			
	}

	return in_interfaces;

}


int Calc_RM_params(int in_interfaces,int G,infoRM_t* RM_info,int* Er[]){
	//int Er[50];
 //scheduled bits for transmission
	int k0=Select_k0(RM_info->graph,RM_info->rv, RM_info->N,RM_info->Zc);
	RateMatch_OutLength(RM_info->mod_order,RM_info->num_layers,G,RM_info->C,Er);
		//printf("RM.info-rv =%d   G=%d\n",RM_info->rv,G );
	//printf("garph=%d, N=%d, Zc=%d,Er[0]=%d,C=%d,k0=%d,G%d\n",RM_info->graph,RM_info->N,RM_info->Zc,Er[0],RM_info->C,k0,G);
	



	return k0;

}


int UNDO_Calc_RM_params(int G,infoRM_t* RM_info,int* Er[]){

	int k0=Select_k0(RM_info->graph,RM_info->rv, RM_info->N,RM_info->Zc);
	RateMatch_OutLength(RM_info->mod_order,RM_info->num_layers,G,RM_info->C,Er);	
	return k0;


}



int Calc_G(infoRM_t RM_info){
	
	//int Nre0=12*RM_info.Symb_X_slot-RM_info.REs_DMRS-RM_info.Overhead_higherlayer;

		
	int Nre1 = RM_info.free_RE;
	//printf("Number of free RE=%d\n",RM_info.free_RE);
	//printf("Free RE=%d\n",Nre1);

	//int G=(int)((float)Nre0*(float)RM_info.mod_order*(float)RM_info.num_layers)*RM_info.nPRB;
	int G=(int)((float)Nre1*(float)RM_info.mod_order*(float)RM_info.num_layers);
	return G;

}


int Undo_rate_MATCH_calc(int Er[],float input[],float* outputF,int rcv_samples,int k0,infoRM_t RM_info){
//////////////DO RM //////////////7	
	
	float out_deinterleave[2048*8*10];
	int r;
	int out_position[2048*8*10];
	float pre_output[2048*8*10];
	int rcv_at_RM=RM_info.N;
	//printf("rcv_at_RM=%d\  CB_size=%dn",rcv_at_RM,RM_info.CB_size);

	for (r=0;r<RM_info.C;r++){
		int i;
	
		//printf("Er[r] in URM=%d",Er[r]);

		DE_interleave_F(Er[r],RM_info.mod_order,input+r*rcv_samples/(RM_info.C),out_deinterleave);
		position_recover(Er[r],out_position,RM_info.N,rcv_at_RM,k0);
		
		bit_recover_F(out_position,out_deinterleave, pre_output,Er[r],rcv_at_RM);

		/*for(i=1; i<(rcv_at_RM)+1; i++){
		printf("%0.1f  ", (pre_output[i-1]));
		}
		printf("\n");		
		*/
		//memcpy(floats_out+Er[r]*r,output_casi,Er[r]*sizeof(float));
		memcpy(outputF+80*2048*r/sizeof(float),pre_output,rcv_at_RM*sizeof(float));


		/*for(i=1; i<(rcv_at_RM); i++){
		printf("%0.1f  ", (outputF[i-1+20*2048*r/sizeof(float)]));
		}
		printf("\n");		
		*/

								
		
	
	}
	return rcv_at_RM;

}

void rate_MATCH_calc(int Er[],char INbytes[],char* output,int rcv_samples,int k0,infoRM_t RM_info){

	int r;
	int i;

		//printf("RATE MATCH E[r]=%d\n",Er[r]);
		char *pter;
		char out_selection[2048*8*10];
		char out_interleave[2048*8*10];
		char output_casi[2048*8*10];

	for (r=0;r<RM_info.C;r++){
		
		bit_selection(Er[r],INbytes+80*2048*r,out_selection,RM_info.N,k0,rcv_samples);
		//printf("RATE MATCH NCB CIRC_BUFFER%d\n",RM_info.N);
		interleave(Er[r],RM_info.mod_order,out_selection,output_casi);
		memcpy(output+Er[r]*r,output_casi,Er[r]);

	}
	
}

