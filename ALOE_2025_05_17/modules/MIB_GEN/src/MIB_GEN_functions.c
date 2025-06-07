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

#include "MIB_GEN_functions.h"

bits_t BCH_payload;
PBCH_t BCH_info;

//Table 7.1.1-1 from 38.212
G_j[32]={16,23,18,17,8,30,10,6,24,7,0,5,3,2,1,4,9,11,12,13,14,15,19,20,21,22,25,26,27,28,29,31};


void convert_2_binary(int in,int* binary){	
	int x[16];	
	for(int i=0;i<16;i++){
		x[i]=(in >> i)&0x1;
	}
	int j=0;
	for(int i=15;i>=0;i--){
		*(binary+i)=x[j];
	j++;
	}
}

int calc_Half_frame_bit(int slot_num,int SCS){
	int real_slot_num=slot_num%10;;
	
	if(SCS==15){
		if(real_slot_num<5)return 0;
		else return 1;
	}
	/*if(SCS==30){
		if(slot_num<10)return 0;
		else return 1;

	}*/
	else printf("SCS NOT CONFIGURED");

}
void Subcarrier_spacing_Calc(int SCS,PBCH_t* BCH_info){

	if(SCS==15) BCH_info->Subcarrier_spacing=0;
	if(SCS==30) BCH_info->Subcarrier_spacing=1;
	
	//These lines are comented because FR2 is not programed.	
	//if(SCS==60) BCH_info->Subcarrier_spacing=0;
	//if(SCS==120) BCH_info->Subcarrier_spacing=1;

}


int Subcarrier_spacing_GET(PBCH_t* BCH_info){

	if(BCH_info->Subcarrier_spacing==0) return 15;
	if(BCH_info->Subcarrier_spacing==1) return 30;
	
	//if(SCS==60) BCH_info->Subcarrier_spacing=0;
	//if(SCS==120) BCH_info->Subcarrier_spacing=1;

}




int DMRS_type_A_pos(int pos_DMRS){

	if(pos_DMRS==2)return 0;
	if(pos_DMRS==3)return 1;

}
int DMRS_type_A_pos_RX(int pos_DMRS){

	if(pos_DMRS==0)return 2;
	if(pos_DMRS==1)return 3;

}


void Prepare_Payload(int pos_DMRS,int slot_num,int SCS,PBCH_t* BCH_info,int SFN,bits_t* BCH_payload){

	BCH_info->DMRS_position=DMRS_type_A_pos(pos_DMRS);
	BCH_info->Half_frame_bit=calc_Half_frame_bit(slot_num,SCS);

	BCH_info->MSBSFN_6=(SFN & 0x03F0)/16;
	BCH_info->LSBSFN_4=(SFN & 0x000F);

	Fill_Payload(BCH_payload,BCH_info);


}

void Fill_Payload(bits_t* BCH_payload,PBCH_t* BCH_info){

	BCH_payload->f1=BCH_info->MSBSFN_6;
	BCH_payload->f2=BCH_info->Subcarrier_spacing;
	BCH_payload->f3=0;//TODO: AIXO NO haria de ser 0.HARDCODED
	BCH_payload->f4=BCH_info->DMRS_position;
	BCH_payload->f5=0;//Harcoded.Look at _functions.h to check why.
	BCH_payload->f6=0;
	BCH_payload->f7=0;
	BCH_payload->f8=1;
	BCH_payload->f9=1;
	BCH_payload->f10=BCH_info->LSBSFN_4;
	BCH_payload->f11=BCH_info->Half_frame_bit;
	BCH_payload->f12=0;//TODO: AIXO NO hauria de ser 0.HARDCODED
	BCH_payload->f13=0;

}


void UNFill_Payload(bits_t* BCH_payload,PBCH_t* BCH_info){

	BCH_info->MSBSFN_6=BCH_payload->f1;
	BCH_info->Subcarrier_spacing=BCH_payload->f2;
	//BCH_payload->f3=0;//TODO: AIXO NO ES 0.HARDCODED
	BCH_info->DMRS_position=BCH_payload->f4;
	BCH_info->LSBSFN_4=BCH_payload->f10;
	//BCH_payload->f12=0;//TODO: AIXO NO ES 0.HARDCODED
	
}


void Obtain_Payload(int *SCS,PBCH_t* BCH_info,bits_t* BCH_payload){
	
	UNFill_Payload(BCH_payload,BCH_info);
	BCH_info->DMRS_position=DMRS_type_A_pos_RX(BCH_info->DMRS_position);
	BCH_info->MSBSFN_6=BCH_info->MSBSFN_6*16;
	BCH_info->SFN=BCH_info->MSBSFN_6+BCH_info->LSBSFN_4;
	*(SCS)=Subcarrier_spacing_GET(BCH_info);

}

void BCH_interleaving(char* in,char* out){

	int A=32;
	int j_sfn=0;
	int j_hrf=10;
	int j_ssb=11;
	int j_other=14;
	
	char *pter;
	char *pter2;
	char bitsIN[4*8];
	char bitsOUT[4*8]={0};
	char out_Bytes[4];
	
	// FORMAT INPUT BYTES FLOW TO BITS
	for(int i=0; i<32/8; i++){
		pter=(char *)(bitsIN+i*8);
		byte2bitsLSBF(*(in+i), &pter , 8);
	}

	printf("ORIGINAL:\n");
	for(int i=1; i<32+1; i++){
		printf("%d ",(int) bitsIN[i-1]);
	}
	printf("\n");
	


	//INTERLEAVE BITS
	for(int i=0;i<A;i++){
		//SFN BITS		
		if(i>=2 && i<8){
			bitsOUT[G_j[j_sfn]]=bitsIN[i];
			j_sfn++;
		}
		else if(i>=28){
			bitsOUT[G_j[j_sfn]]=bitsIN[i];
			j_sfn++;
		}
		//Half frame bit
		else if(i==27){
			bitsOUT[G_j[j_hrf]]=bitsIN[i];
		}
		//from A+5 to A+7 (both included) bits
		else if(i>=24 && i<27){
			bitsOUT[G_j[j_ssb]]=bitsIN[i];
			j_ssb++;
		}
		//Other bits
		else{
			bitsOUT[G_j[j_other]]=bitsIN[i];
			j_other++;
		}


	}

	//printf("j_sfn=%d\n",j_sfn);
	//printf("j_hrf=%d\n",j_hrf);
	//printf("j_ssb=%d\n",j_ssb);
	//printf("j_other=%d\n",j_other);


	printf("INTERLEAVED:\n");
	for(int i=1; i<32+1; i++){
		printf("%d ",(int) bitsOUT[i-1]);
	}
	printf("\n");
	


	//Pack to bytes
	for(int i=0; i<32/8; i++){
		pter2=(char *)(bitsOUT+i*8);
		bits2byteLSBF(&out_Bytes[i], &pter2, 8);
	}
	memcpy(out,&out_Bytes,(4)*sizeof(char));

}

void BCH_Deinterleaving(char* in,char* out){

	int A=32;
	int j_sfn=9;
	int j_hrf=10;
	int j_ssb=14;
	int j_other=31;
	
	char *pter;
	char *pter2;
	char bitsIN[4*8];
	char bitsOUT[4*8]={0};
	char out_Bytes[4];
	
	// FORMAT INPUT BYTES FLOW TO BITS
	for(int i=0; i<32/8; i++){
		pter=(char *)(bitsIN+i*8);
		byte2bitsLSBF(*(in+i), &pter, 8);
	}


	//INTERLEAVE BITS
	for(int i=A-1;i>=0;i--){
		//SFN BITS		
		if(i>=2 && i<8){
			bitsOUT[i]=bitsIN[G_j[j_sfn]];
			j_sfn--;
		}
		else if(i>=28){
			bitsOUT[i]=bitsIN[G_j[j_sfn]];
			j_sfn--;
		}
		
		else if(i==27){
			bitsOUT[i]=bitsIN[G_j[j_hrf]];
		}
		else if(i>=24 && i<27){
			bitsOUT[i]=bitsIN[G_j[j_ssb]];
			j_ssb--;
		}
		else{
			bitsOUT[i]=bitsIN[G_j[j_other]];
			j_other--;
		}


	}

	


	/*printf("DEINTERLEAVED:\n");
	for(int i=1; i<32+1; i++){
		printf("%d ",(int) bitsOUT[i-1]);
	}
	printf("\n");
	*/

	//Pack to bytes
	for(int i=0; i<32/8; i++){
		pter2=(char *)(bitsOUT+i*8);
		bits2byteLSBF(&out_Bytes[i], &pter2, 8);
	}
	memcpy(out,&out_Bytes,(4)*sizeof(char));


}


int choose_M(int Lmax,int A){

	int M;	
	switch(Lmax){
		case 4:
			M=A-3;
			break;

		case 8:
			M=A-3;
			break;

		case 10:
			M=A-4;
			break;

		case 20:
			M=A-5;
			break;

		case 64:
			M=A-6;
			break;

	}

	return M;

}

int choose_v(int third_lsb, int second_lsb){

	if(third_lsb==0 && second_lsb==0)return 0;
	if(third_lsb==0 && second_lsb==1)return 1;
	if(third_lsb==1 && second_lsb==0)return 2;
	if(third_lsb==1 && second_lsb==1)return 3;


}

void BCH_scrambling_sequence_BITS(int CELLID, char *scrambseq,int Lmax, int A, char* inseq){

	char seq_c[MAXSEQLENGTH*2];
	int n;
	int x1[MAXSEQLENGTH*2], x2[MAXSEQLENGTH*2];
	int Cinit=CELLID;	

	int M=choose_M(Lmax,A);
	int sequencelength=4*M;//????? MAYBE NO ESTA BÉ
	

	for (n = 0; n < 31; n++) {
    		x2[n] = (Cinit >> n) & 0x1;
  	}
  	x1[0] = 1;
  	for (n = 0; n < Nc + sequencelength; n++) {
    		x1[n + 31] = (x1[n + 3] + x1[n]) & 0x1;
    		x2[n + 31] = (x2[n + 3] + x2[n + 2] + +x2[n+1] + x2[n]) & 0x1;
  	}
  	for (n = 0; n < sequencelength; n++) {
    		seq_c[n] = (char)((x1[n + Nc] + x2[n + Nc]) & 0x1);
  	}

	
	//positions of 2nd LSB and 3rd LSB of SFN in MIB are 24th bit and 6th bit
	//int 2nd=(int) *(inseq+24);
	//int 3rd=(int) *(inseq+6);
	int v=choose_v((int) *(inseq+6), (int) *(inseq+24));

	for(int i=0;i<A;i++){
		
	}


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



