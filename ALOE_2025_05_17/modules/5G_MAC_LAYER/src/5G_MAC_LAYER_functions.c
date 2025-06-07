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
#include <math.h>

#include "5G_MAC_LAYER_functions.h"

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */

MAC_struc MACinfo;
bits_t DCI;
DCIinfo_t DCI_info;

extern print_t T;

// TABLES MCS 
int Qm51311[32]={2,2,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,6,6,6,6,6,6,6,6,6,6,6,6,0,0,0};

float Spectral_efficency_51311[32]={0.2344, 0.3066, 0.3770, 0.4902, 0.6016, 0.7402, 0.8770, 1.0273,	1.1758, 1.3262, 1.3281, 1.4766, 1.6953, 1.9141, 2.1602, 2.4063, 2.5703, 2.5664, 2.7305, 3.0293, 3.3223, 3.6094, 3.9023, 4.2129, 4.5234, 4.8164, 5.1152, 5.3320, 5,5547, 0.0000, 0.0000, 0.0000};

float CodeRate51311[32]={120,157,193,251,308,379,449,526,602,679,340,378,434,490,553,616,658,438,466,517,567,616,666,719,772,822,873,910,948,0,0,0};

int TBS51321[92]={24,32,40,48,56,64,72,80,88,96,104,112,120,128,136,144,152,160,168,176,184,192,208,224,240,256,272,288,304,320,
336,352,368,384,408,432,456,480,504,528,552,576,608,640,672,704,736,768,808,848,888,928,984,1032,1064,1128,1160,1192,1224,1256,
1288,1320,1352,1416,1480,1544,1608,1672,1736,1800,1864,1928,2024,2088,2152,2216,2280,2408,2472,2536,2600,2664,2728,2792,2856,2976,3104,3240,3368,3496,
3624,3752,3824};


float closest_MCS(float coderate,int mod_order){
	int i;
	float min_dif=1000;
	int min_dif_idx=100;
	for (i=0;i<32;i++){
		if(mod_order==Qm51311[i]){
			//printf("Qm51311[i]=%d\n",Qm51311[i]);
			if (fabs((CodeRate51311[i]/1024)-coderate)<min_dif){
				min_dif=fabs((CodeRate51311[i]/1024)-coderate);
				min_dif_idx=i;				
				printf("mindif=%f min_idx=%d\n",min_dif,min_dif_idx);
				
			}
		}
	}	
	if (min_dif_idx!=100){

		return CodeRate51311[min_dif_idx]/1024;
	}else{
		printf("ERROR: INVALID MODULATION");
		return(0);
	}

}




int buffer(int buffer_size,unsigned int new_value, int *buffer){
	int buffer_out=buffer[buffer_size-1];

	//printf("Buffer_out_this_TS = %d   \n",buffer_out);
	//printf("Buffer_IN_this_TS = %d   \n",new_value);
	int i;
	for (i=buffer_size-1;i>=0;i--){
		buffer[i+1]=buffer[i];
	//	printf("Buffer data_CODE_BLOCK in pos %d is %d\n",i,buffer[i]);
	}	
	buffer[0]=new_value;

	return buffer_out;
}




int calcDMRSlength_long(MAC_struc* MACinfo){

	if(MACinfo->DMRS_type==1){
		
		MACinfo->REs_DMRS=6*(1+MACinfo->Additional_DMRS_position); 

	}


	return 0;

}

int calcDMRSlength_short(MAC_struc* MACinfo){

	if(MACinfo->DMRS_type==1){

		//Central 240 carriers
		if(MACinfo->Additional_DMRS_position==2){
			MACinfo->REs_DMRS=6;
	
		}else{
			MACinfo->REs_DMRS=0;
		}
			

	}


	return 0;

}


int Calc_externalNRE1(MAC_struc* MACinfo){
	int Nre1=0;
	int external_nPRB=MACinfo->nPRB-nPRB_SS;
	if(external_nPRB!=0){
		MACinfo->REs_DMRS=6*(1+MACinfo->Additional_DMRS_position); 
		//OJO el 13 es un hardcode pq tot funciona a 14FFT i un canal de control
		//int Nre0=12*13-MACinfo->REs_DMRS-MACinfo->Overhead_higherlayer;
		int Nre0=12*(Symb_slot_default-MACinfo->CORESETS_symbol_size)-MACinfo->REs_DMRS-MACinfo->Overhead_higherlayer;
		Nre1=min_int(156,Nre0)*external_nPRB;
		//printf("Symb_per slot=%d,RES_DMRD=%d,Nre0=%d,NRE1=%d external_nPRB=%d\n",Symb_X_slot,MACinfo->REs_DMRS,Nre0,Nre1,external_nPRB);

	}
	return Nre1;
}
	


int Calc_TBS(int Symb_X_slot,int REs_DMRS,int Overhead_higherlayer,int nPRB,float coderate,int mod_order,int num_layers,int MAXCB_KCBg2,int MAXCB_KCBg1,MAC_struc* MACinfo){

	//internal PRB	
	int Nre0=12*Symb_X_slot-REs_DMRS-Overhead_higherlayer;
	int Nre1_1=min_int(156,Nre0)*nPRB_SS;

	//external PRB
	int Nre1_2=Calc_externalNRE1(MACinfo);
	int Nre1=Nre1_1+Nre1_2;
	MACinfo->free_RE=Nre1;
/*	printf("Number of free RE internal=%d\n",Nre1_1);
	printf("Number of free RE external=%d\n",Nre1_2);
	printf("Number of free RE total=%d\n",Nre1);
*/	

	int Ninfo=(int)((float)Nre1*coderate*(float)mod_order*(float)num_layers);
//	printf("Ninfo=%d, coderate=%f, Nre1=%d, mod_order=%d ,num_layers=%d\n",Ninfo,coderate,Nre1,mod_order,num_layers);

	float Ninfo1;
	int TBS;
	int i;
		
	//printf("Nre0 =%d, Nre1=%d, Ninfo=%d\n",Nre0,Nre1,Ninfo);

	if (Ninfo<=MAXCB_KCBg2-16){ 

		float n=max_float(3.0,floor(log2(Ninfo))-6.0);
		//printf("test=%f\n",floor(log2(Ninfo))-6.0);
		

		Ninfo1=max_int(24,pow(2,(int)n)*floor(Ninfo/pow(2,(int)n)));
		//TBS=(int)Ninfo1;
//		printf("TBS=%d  n=%f  Ninfo1=%f\n", TBS,n,Ninfo1);		

// closest bigger TBS than Ninfo1 table 5.1.3.2-1 from 138.214
	
	int min_diff=10000;
	int min_idx=10000;	
	for (i=0;i<92;i++){
		if((TBS51321[i]-Ninfo1)<min_diff && (TBS51321[i]-Ninfo1)>0){
			min_diff=(TBS51321[i]-Ninfo1);
			min_idx=i;
		}	
	
	}	
	TBS=TBS51321[min_idx];
	
	}else{

		float n=floor(log2f(Ninfo-24))-5;
		Ninfo1=max_int(MAXCB_KCBg2,pow(2,(int)n)*ceil((Ninfo-24)/ pow(2,(int)n)));
		//printf("Ninfo1=%f\n",Ninfo1);
	
				
		if (coderate<=0.25){
		
			int C=ceil((Ninfo1+24)/MAXCB_KCBg2-24);
			TBS=8*C*ceil((Ninfo1+24)/(8*C))-24;	
				
		
		}else{
			if (Ninfo1>MAXCB_KCBg1-24){
				int C=ceil((Ninfo1+24)/(MAXCB_KCBg1-24));
				TBS=8*C*ceil((Ninfo1+24)/(8*C))-24;
			}else{
				TBS=8*ceil((Ninfo1+24)/8)-24;
			}
		}
	}
	
	
//	printf("TBS=%d \n",TBS);		
	return TBS;
}	



int min_int(int a,int b){
	if (a<b){
		return a;	
	}else{
		return b;
	}

}



float max_float(float a,float b){
	if (a>b){
		return a;	
	}else{
		return b;
	}

}

int max_int(int a,int b){
	if (a>b){
		return a;	
	}else{
		return b;
	}

}


int check_parameters(MAC_struc* MACinfo){
	int error=0;

	int MAX_REnum_for_CORESET=12*MACinfo->nPRB*MACinfo->CORESETS_symbol_size;
	int RE_for_PDCCH=12*6*MACinfo->Agr_lvl_DCI;/*12 sucarriers X 6 REG/CCE * Num CCE = carrier agregation level;	*/

	if( MACinfo->DMRS_type!=1){
		printf("ERROR!!! DMRS type not configured\n");
		error=-1;
	}
		
	if( RE_for_PDCCH> MAX_REnum_for_CORESET){
		printf("ERROR!!! With %d Symbols for CORESET such big Agragation level can't be configured. REDUCE IT!!!\n",MACinfo->CORESETS_symbol_size);
		error=-1;
	}

	if( MACinfo->rv!=0){
		printf("Redundancy version differenet from 0. Without AHRQ rv=0 is the best option\n");
		error=-1;
	}

	if(MACinfo->Additional_DMRS_position>3 || MACinfo->Additional_DMRS_position <0){
		printf("ERROR!!! Invalid Additional DMRS. Should be between 0 and 3.\n");
		error=-1;
	}
	
	if(MACinfo->nPRB<20){
		printf("ERROR!!! Minimum PRB size is 20.\n");
		error=-1;
	}
	
	if(MACinfo->QAM_order!=2 && MACinfo->QAM_order!=4 && MACinfo->QAM_order!=6){
		printf("##########################################################\n");
		printf("ERROR!!! Modulation=%d not valid.\n", MACinfo->QAM_order);
		printf("Please use only MQAM=2,4 or 6 in current implementation\n");
		printf("##########################################################\n");
		error=-1;
	}

	if(MACinfo->CORESETS_symbol_size>2 || MACinfo->CORESETS_symbol_size<=0){
		printf("ERROR!!! CORESETS symbol size should be 1 or 2. 3 is not configured YET!\n");
		error=-1;
	}
	return error;

}

void Calc_symbols(int* Symb_X_slot_SS, int* Symb_X_slot,MAC_struc* MACinfo){
	*(Symb_X_slot_SS)=Symb_slot_default-(MACinfo->CORESETS_symbol_size+8);//8 son els 2 SSB de 4 symbols cadascun.
	*(Symb_X_slot)=Symb_slot_default-MACinfo->CORESETS_symbol_size;


}




////////////////PDCCH FUNCTIONS//////////////////////


// ESTRUCTURA DCI//

//VRB-PRB 1 bits --> hardcoded at 0
//PRB bundling size indicator 0 bits
//Rate matching indicator 0 bits hardcoded
//ZP CSI-RS trigger – 0 bits hardcoded
//MCS 5 bits
//New data indicator 1bit
//redundancy version 2bits
//HARQ process number – 4 bits-->hardcoded to 0000
//Downlink assignment index  0 bits 
//TPC command for scheduled PUCCH 2 bits hardcoded to 00
//PUCCH resource indicator 3bits hardcoded to 000
//PDSCH-to-HARQ_feedback timing indicator – 0 bits hardcoded
//Antenna port(s) – 4,5,6 bits -->hardcoded to 0000
//Transmission configuration indication 3 bits ->hardoded to 000
//SRS request – 2 bits hardcoded to 00
//CBG transmission information (CBGTI) – 0 bits hardcoded
//CBG flushing out information (CBGFI) – 0 bits hardcoded
//DMRS sequence initialization 0 bits ->hardcoded


void FILL_DCI(bits_t* DCI,DCIinfo_t* DCI_info){
	DCI->f1=1;
	DCI->f2=0;
	DCI->f3=DCI_info->RIV;
	DCI->f4=DCI_info->time_domain_allocation;
	DCI->f5=0;
	DCI->f6=DCI_info->MCS_index;
	DCI->f7=1;
	DCI->f8=DCI_info->rv;
	DCI->f9=0;
	DCI->f10=0;
	DCI->f11=0;
	DCI->f12=0;
	DCI->f13=0;
	DCI->f14=0;

}

void UNFILL_DCI(bits_t* DCI,DCIinfo_t* DCI_info){

	DCI_info->RIV=DCI->f3;
	DCI_info->time_domain_allocation=DCI->f4;
	DCI_info->MCS_index=DCI->f6;
	DCI_info->rv=DCI->f8;

}


void do_DCI(DCIinfo_t* DCI_info,bits_t* DCI){

	DCI_info->RIV=Calc_RIV(DCI_info->Lrbs,DCI_info-> RBstart,DCI_info-> N_size_BWP);
	FILL_DCI(DCI,DCI_info);

}

void Undo_DCI(DCIinfo_t* DCI_info,bits_t* DCI){

	UNFILL_DCI(DCI,DCI_info);

	int Lrbs;
	int RBstart;
	GET_Lrbs_RBstart(&Lrbs,&RBstart,DCI_info->N_size_BWP,DCI_info->RIV);
	
	DCI_info->Lrbs=Lrbs;
	DCI_info->RBstart=RBstart;

}


int Calc_Frequencydomain_allocation_size(int N_size_BWP){
	int size=ceil(log2(N_size_BWP*(N_size_BWP+1)/2));
	return size;
}

int Calc_RIV(int Lrbs,int RBstart,int N_size_BWP){
	int RIV;
	if((Lrbs-1)<=floor(N_size_BWP/2)){
		RIV=N_size_BWP*(Lrbs-1)*RBstart;
	}else{
		RIV=N_size_BWP*(N_size_BWP-Lrbs+1)+(N_size_BWP-1-RBstart);
	}
	return RIV;
}

void GET_Lrbs_RBstart(int* Lrbs,int* RBstart,int N_size_BWP,int RIV){
	int L_rbs;
	int RB_start;	

	for(int n=1;n<=N_size_BWP;n++){
		for(int m=0;m<=N_size_BWP-n;m++){
			if((n-1)<=floor(N_size_BWP/2)){
				if(RIV==N_size_BWP*(n-1+m)){
					L_rbs=n;
					RB_start=m;
				}
			}else{
				if(RIV==N_size_BWP*(N_size_BWP-n+1)+(N_size_BWP-1-m)){
				L_rbs=n;
				RB_start=m;	
				}

			}
		}

	}

	*(Lrbs)=L_rbs;
	*(RBstart)=RB_start;

}





int Calc_SLIV(int S,int L){
	int SLIV;
	if((L-1)<=7){
		SLIV=14*(L-1)+S;
	}else{
		SLIV=14*(14-L+1)+(14-1-S);
	}
	return SLIV;
}

int GET_S_L(int* S,int* L,int SLIV_real){
	int SLIV;	
	for(int s=0;s<14;s++){
		for(int l=0;l<14;l++){
			if((l-1)<=7){
				SLIV=14*(l-1)+s;
			}else{
				SLIV=14*(14-l+1)+(14-1-s);
			}
			
			if(SLIV==SLIV_real){
				*(S)=s;
				*(L)=l;
			return 0;
			}
		}
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


void Init_parameters_PDCCH(DCIinfo_t* DCI_info,MAC_struc* MACinfo){

	//DE MOMENT SON VALORS HARDCODEJATS
	DCI_info->RBstart=0;//First PRB for UE
	DCI_info->time_domain_allocation=11;//S=1;L=13 from table 5.1.2.1.1-2 from 38.214

	DCI_info->Lrbs=MACinfo->nPRB;
	DCI_info->rv=MACinfo->rv;
	DCI_info->MCS_index=MCS_selected(MACinfo->coderateMCS,MACinfo->QAM_order);
	
}

int MCS_selected(float coderate,int Mod_order){

	for(int i=0;i<32;i++){//32 is number of entries in MCS table
		if((float)CodeRate51311[i]/1024.0==coderate){
			if(Mod_order==Qm51311[i]) return i;
		}
	}

}

void GET_params_from_DCI(DCIinfo_t* DCI_info,MAC_struc* MACinfo){
	
	MACinfo->coderateMCS=CodeRate51311[DCI_info->MCS_index]/1024.0;
	MACinfo->QAM_order=Qm51311[DCI_info->MCS_index];
	MACinfo->rv=DCI_info->rv;
}



/////////////////////PRINTING
#define LINELENGTH	PRINT_LINELENGTH


void printTEXT(int noflines, int textcolor, int backcolor, char borderchar){

	int i, length;
	char border[LINELENGTH];
	char text2print[LINELENGTH];

	T.borderchar=borderchar;
	T.noflines=noflines;
	T.forecolor=textcolor%256;
	T.backgroundcolor=backcolor%256;


	// Border UP
	memset(border, T.borderchar, LINELENGTH-1);
	border[LINELENGTH-1]='\0';
//	printf("\033[1;%s;%sm%s\033[0m\n", textcolor, backcolor, border);

//	printf("%s\n", backcolor); 
/*	printCOLORtext(0, "208", backcolor,  LINELENGTH, border);
	printCOLORtext(5, "208", "010",  LINELENGTH, &T.text[i][0]);
*/
	printCOLORtext(0, T.forecolor, T.backgroundcolor,  LINELENGTH, border);
	for(i=0; i<T.noflines; i++){
		printCOLORtext(2, T.forecolor, T.backgroundcolor,  LINELENGTH, &T.text[i][0]);
	}
	printCOLORtext(0, T.forecolor, T.backgroundcolor,  LINELENGTH, border);
}


//http://web.archive.org/web/20131009193526/http://bitmote.com/index.php?post/2012/11/19/Using-ANSI-Color-Codes-to-Colorize-Your-Bash-Prompt-on-Linux
//char format[BACKGROUNDLEN]="\33[1m\33[38;5;021;48;5;226m";
//char reset[10]="\033[0m";


void printCOLORtext(int position, int forecolor, char groundcolor,  int end, char *data2print){
	
	int i;		
	char background[LINELENGTH]; 
	int length=(int)LINELENGTH;
	char formatt[LINELENGTH]="\33[1m\33[38;5;021;48;5;226m";
	char reset[10]="\033[0m";

	char fore[4];
	char ground[4];

	my_itoa(groundcolor, ground, 1);
	my_itoa(forecolor, fore, 1);

	if(strlen(data2print)>LINELENGTH-1){
		printf("ERROR!!! data2print length = %d > LINELENGTH=%d\n", strlen(data2print), LINELENGTH);
		//exit(0);
	}

	memset(background, 0x20, LINELENGTH-1);
//	background[BACKGROUNDLEN-1]='\0';
/*	printf("A_");
	for(i=0; i<strlen(formatt)-1; i++)printf("%c", formatt[i]);
	printf("\n");
*/
	memcpy(formatt+11, fore, strlen(fore));
	memcpy(formatt+20, ground, strlen(ground));
/*	printf("B_");
	for(i=0; i<strlen(formatt)-1; i++)printf("%c", formatt[i]);
	printf("\n");
*/
	length=strlen(data2print)-1;
	if(length>(LINELENGTH-1-position))length=(LINELENGTH-1-position);
	memcpy(background+position, data2print, length);
	background[end-1]=NULL;
	printf("%s%s%s\n", formatt, background, reset);
}


void my_itoa(int number, char *str, int nofchars) {
   sprintf(str, "%.3d", number&0xFF); //make the number into string using sprintf function
}

///////////////////////////////////////////////////////////////////////////////


















