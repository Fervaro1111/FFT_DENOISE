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

#include "MAPPING_V2_UL_functions.h"

MAPP_t MAPPinfo; 

MAC_struc MACinfo;
Bufer_MAC MAC_buff;
Sequences_t SequencesParams;

int slot_num=0;
_Complex float DMRS[DMRS_max_length];

int GEN_DMRS(int Slotnumber,int OFDMsymbol,_Complex float* DMRS_seq){

	int X1[4000]={0}; //1855=1600(NC)+240(dmrslength)+32
	X1[0]=1;	
	int X2[4000]={0};
	int c[4000]={0};

	
	int d=DMRS_max_length;
	
	
	int l; //symbol number within the slot
	int n;//slot number withinthe frame
	l=OFDMsymbol;
	n=Slotnumber;
	
	//TODO:fer que aixo sigui configurable i no HARCODE

	int NIDscram=0;
	int nscid=5;
	int landa=0;

	//from 138.211 ->7.4.1.1.1
	int cinit=(int)remainder(pow(2,17)*(14*n+l+1)*(2*NIDscram+1)+pow(2,17)*(landa/2)+2*NIDscram+nscid,pow(2,31));
	//printf("%i\n",c);	
	

	for(int i=0;cinit>0;i++){    
		X2[i]=cinit%2;
		cinit=cinit/2; 
	}       
		
	for (int i=0;i<Nc+2*d;i++){
		X1[i+31]=(X1[i+3]+X1[i])%2;
		//printf("%d \n",X1[i]);
	}
	
	for (int i=0;i<Nc+2*d;i++){
		X2[i+31]=(X2[i+3]+X2[i+2]+X2[i+1]+X2[i])%2;
		//printf("%d \n",X2[i]);
	}
	
	
	for (int i=0;i<2*d;i++){
		c[i]=((X1[i+Nc]+X2[i+Nc])%2);
		//printf("%d \n",c[i]);
	}
	
	
	for (int i=0;i<d;i++){
		DMRS_seq[i]=0.707*(1.0-2.0*c[2*i])+I*0.707*(1.0-2.0*c[2*i+1]);
		//printf("%i: %f  %f I\n",i,creal(DMRS_seq[i]),cimag(DMRS_seq[i]));
	}
	
	return(0);
}





void fill_structure_1_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int data_init_pos){


	for(int i=data_init_pos;i<MAPPinfo->numFFT;i++){
		if(i==2){
			DMRS_per_symbol[i]=(MAPPinfo->datalength-2)/2;
			data_per_symbol[i]=(MAPPinfo->datalength-2)/2;
		}else{

		data_per_symbol[i]=MAPPinfo->datalength-2;
		DMRS_per_symbol[i]=0;

		}
	}
	
}



void fill_structure_2_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int data_init_pos){

	for(int i=data_init_pos;i<MAPPinfo->numFFT;i++){
		if(i==2 || i==11){
			DMRS_per_symbol[i]=(MAPPinfo->datalength-2)/2;
			data_per_symbol[i]=(MAPPinfo->datalength-2)/2;
		}else{

		data_per_symbol[i]=MAPPinfo->datalength-2;
		DMRS_per_symbol[i]=0;

		}
	}
		
}

void fill_structure_3_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int data_init_pos){

	for(int i=data_init_pos;i<MAPPinfo->numFFT;i++){
		if(i==2 || i==7 || i==11){
			DMRS_per_symbol[i]=(MAPPinfo->datalength-2)/2;
			data_per_symbol[i]=(MAPPinfo->datalength-2)/2;
		}else{

		data_per_symbol[i]=MAPPinfo->datalength-2;
		DMRS_per_symbol[i]=0;

		}
	}
		
}


void fill_structure_4_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int data_init_pos){

	for(int i=data_init_pos;i<MAPPinfo->numFFT;i++){
		if(i==2 || i==5 || i==8 || i==11){
			DMRS_per_symbol[i]=(MAPPinfo->datalength-2)/2;
			data_per_symbol[i]=(MAPPinfo->datalength-2)/2;
		}else{

		data_per_symbol[i]=MAPPinfo->datalength-2;
		DMRS_per_symbol[i]=0;

		}
	}
		
}



void generate_Subframe_structure(MAPP_t* MAPPinfo, int* data_per_symbol,int data_init_pos,int* DMRS_per_symbol){

	if (MAPPinfo->DMRS_type==1){
		switch (MAPPinfo->Additional_DMRS_position) {
			
			case 0:
				fill_structure_1_DMRS(data_per_symbol,DMRS_per_symbol,MAPPinfo,data_init_pos);
				break;
			case 1:
				fill_structure_2_DMRS(data_per_symbol,DMRS_per_symbol,MAPPinfo,data_init_pos);
				break;
			case 2:
				fill_structure_3_DMRS(data_per_symbol,DMRS_per_symbol,MAPPinfo,data_init_pos);
				break;
			case 3:
				fill_structure_4_DMRS(data_per_symbol,DMRS_per_symbol,MAPPinfo,data_init_pos);
				break;
			
			default:
				printf("ERROR\n");
				break;
		}
	}else{
		printf("NOT PROGRAMMED YET\n");
	}
	
	/*for(int i=0;i<14;i++){
		printf("data per symbol=%d\n",data_per_symbol[i]);
		printf("DMRS per symbol=%d\n",DMRS_per_symbol[i]);	
	}*/

}



int getDATAfromLTE_DOWNLINKspectrum(_Complex float *LTEspect, int FFTlength, int datalength, _Complex float *QAMsymb){
	int i, j;
	
	j=FFTlength-datalength/2;
	for(i=0; i<datalength; i++){
		*(QAMsymb+i)=*(LTEspect+j);
		j++;
		if(j==FFTlength)j=0;
	}
	return(0);
}
	


void UNDO_Spectrum(MAPP_t* MAPPinfo,_Complex float* in, _Complex float *out){
	
	for(int i=0;i<MAPPinfo->numFFT;i++){
		getDATAfromLTE_DOWNLINKspectrum(in+i*MAPPinfo->FFTlength,  MAPPinfo->FFTlength, MAPPinfo->datalength,out+i*MAPPinfo->datalength);

	}


}


void DEmix_data(int datalength,_Complex float* in, _Complex float* out){
	for(int i=0;i<datalength;i++){
		__real__ *(out+i) = __real__ *(in+i*2+1);
		__imag__ *(out+i) = __imag__ *(in+i*2+1);
	}

}


void createSpectrum(MAPP_t* MAPPinfo,_Complex float* in,_Complex float* out){

	for(int i=0;i<MAPPinfo->numFFT;i++){
		int e=create_LTEspectrumNORS(in+i*MAPPinfo->datalength,MAPPinfo->FFTlength,MAPPinfo->datalength,out+MAPPinfo->FFTlength*i);
		
	}

}


int create_LTEspectrumNORS(_Complex float *MQAMsymb, int FFTlength, int datalength, _Complex float *out_spectrum){
	int i, j;
	
	for(i=0; i<FFTlength; i++)*(out_spectrum+i)=0.0+0.0*I;

	j=(FFTlength-datalength/2);
	
	for(i=0; i<datalength; i++){
		*(out_spectrum+j)=*(MQAMsymb+i);
		//printf("out_spectrum=%f at i=%d\n",creal(out_spectrum[j]),j);	
		j++;
		if(j==FFTlength) j=0;
	}
	
	return(1);
}

void mix_data(_Complex float *DMRSseq,_Complex float *inMQAMsymb,_Complex float *out_symb,int data_length,int ofset){	
	//Fa elmateix que la configuració de la llibreta de la feina

	for (int k=0;k<data_length;k++){
		__real__ *(out_symb+2*k)=__real__ *(DMRSseq+k+ofset/2);
		__imag__ *(out_symb+k*2)=__imag__ *(DMRSseq+k+ofset/2);
	
		__real__ *(out_symb+2*k+1)=__real__ *(inMQAMsymb+k);
		__imag__ *(out_symb+k*2+1)=__imag__ *(inMQAMsymb+k);


	}

}


void Calc_params(MAPP_t* MAPPinfo,Sequences_t* SequencesParams,int Slot_number){

	
	MAPPinfo->datalength=MAPPinfo->nPRB*12;//12 carriers per PRB
	MAPPinfo->DMRS_length=MAPPinfo->datalength/2;
	
	if(MAPPinfo->nPRB<=25)MAPPinfo->FFTlength=512;
	if(MAPPinfo->nPRB>25 && MAPPinfo->nPRB<=79)MAPPinfo->FFTlength=1024;

	//printf("MAPPinfo->DMRS_length=%d datalength=%d\n",MAPPinfo->DMRS_length,MAPPinfo->datalength);

	SequencesParams->OFDMsymbol=1;
	SequencesParams->Slotnumber=Slot_number; //slot number within a frame
	SequencesParams->DMRSlength=MAPPinfo->DMRS_length;


}

void Create_Subframe(MAPP_t* MAPPinfo,int slot_num,_Complex float* inMQAMsymb,int* data_per_symbol,int* DMRS_per_symbol,_Complex float *tmp,int data_init_pos){


	//printf("\n");
	//printf("GENSYMBOLS------------------\n");

	int OFDMsym=0;
	int in_pos=0;
	int out_pos=0;
	for(int i=0;i<MAPPinfo->numFFT;i++){
	
		gensymbols(MAPPinfo,&SequencesParams,slot_num,OFDMsym,inMQAMsymb+in_pos,tmp+out_pos+1,data_per_symbol,DMRS_per_symbol,data_init_pos);
		
		
		OFDMsym++;
		in_pos=in_pos+data_per_symbol[i];
		out_pos=out_pos+MAPPinfo->datalength;

	}

}

void gensymbols(MAPP_t* MAPPinfo,Sequences_t* SequencesParams,int Slotnumber,int OFDMsymbol,_Complex float *in_data,_Complex float* out,int *data_per_symbol,int* DMRS_per_symbol,int data_init_pos){
	

	///////////////GENERATE 2D DUBFRAME GRID///////////////////

	int SS_in_symb=0;

	
	if(DMRS_per_symbol[OFDMsymbol]==0 && data_per_symbol[OFDMsymbol]!=0){
		memcpy(out,in_data,data_per_symbol[OFDMsymbol]*sizeof(_Complex float));	
	}

	
	if(DMRS_per_symbol[OFDMsymbol]!=0 && data_per_symbol[OFDMsymbol]!=0){
		GEN_DMRS(Slotnumber,OFDMsymbol,&DMRS);
		mix_data(&DMRS,in_data,out,data_per_symbol[OFDMsymbol],0);
	}

/*
	//if(OFDMsymbol<data_init_pos){
	if(DMRS_per_symbol[OFDMsymbol]==0 && data_per_symbol[OFDMsymbol]==0){
		for(int i=0;i<MAPPinfo->datalength;i++){
			out[i]=0.0+0.0*I;
		}
	}*/

	
}

int GET_Subframe(MAPP_t* MAPPinfo,int slot_num,_Complex float* inMQAMsymb,int* data_per_symbol,int* DMRS_per_symbol,_Complex float *out_data,int data_init_pos){

	int OFDMsym=0;
	int in_pos=0;
	int out_pos=0;
	for(int i=0;i<MAPPinfo->numFFT;i++){

		GETsymbols(MAPPinfo,slot_num,OFDMsym,inMQAMsymb+in_pos+1,out_data+out_pos, data_per_symbol,DMRS_per_symbol,data_init_pos);
		
		OFDMsym++;
		in_pos=in_pos+MAPPinfo->datalength;
		out_pos=out_pos+data_per_symbol[i];
				
	}
	return out_pos;
	
}


void GETsymbols(MAPP_t* MAPPinfo,int Slotnumber,int OFDMsymbol,_Complex float *in_data,_Complex float* out,int *data_per_symbol,int* DMRS_per_symbol,int data_init_pos){
		
		
	//printf("SSin_symb=%d at symb=%d\n",SS_in_symb,OFDMsymbol);

	if(DMRS_per_symbol[OFDMsymbol]==0 && data_per_symbol[OFDMsymbol]!=0){
		memcpy(out,in_data,data_per_symbol[OFDMsymbol]*sizeof(_Complex float));		
	}
		
	if(DMRS_per_symbol[OFDMsymbol]!=0 && data_per_symbol[OFDMsymbol]!=0){
		DEmix_data(MAPPinfo->datalength/2, in_data,out); 
	}

}


int Do_Mapping(MAPP_t* MAPPinfo, _Complex float *inMQAMsymb, int* data_per_symbol, _Complex float *subframe, int data_init_pos, int* DMRS_per_symbol){


	_Complex float tmp[2048*20]={0.0+0.0*I};
	int OFDMsymbol=0;
	int snd_samples=0;


	//calcsubframe params
	Calc_params(MAPPinfo,&SequencesParams,slot_num);	


	//Gen subframe structure
	generate_Subframe_structure(MAPPinfo,data_per_symbol, data_init_pos, DMRS_per_symbol);


	//crate subframe
	Create_Subframe(MAPPinfo,slot_num,inMQAMsymb,data_per_symbol,DMRS_per_symbol,&tmp,data_init_pos);
	

	//CREATE SPECTRUM
	createSpectrum(MAPPinfo,&tmp,subframe);

	snd_samples=MAPPinfo->numFFT*MAPPinfo->FFTlength;

	slot_num++;
	if (slot_num==10)slot_num=0;
	return snd_samples;

}



int UNDO_Mapping(MAPP_t* MAPPinfo,_Complex float *inspectrum,int* data_per_symbol,_Complex float *dataout,int data_init_pos,int* DMRS_per_symbol){

	int snd_samples;
	_Complex float tmp[2048*50]={0.0+0.0*I};

	//calcsubframe params
	Calc_params(MAPPinfo,&SequencesParams,slot_num);	


	//Gen subframe structure
	generate_Subframe_structure(MAPPinfo,data_per_symbol, data_init_pos, DMRS_per_symbol);


	//UNDO_Spectrum
	UNDO_Spectrum(MAPPinfo,inspectrum, &tmp);
	

	//GET DATA FROM GRID
	snd_samples=GET_Subframe(MAPPinfo,slot_num, &tmp, data_per_symbol, DMRS_per_symbol,dataout,data_init_pos);

	
for(int i=0;i<5000;i++){
		printf("data=%f i=%d\n",creal(dataout[i]),i);			
	}



	//Update slot num WITHIN FRAME
	slot_num++;
	if (slot_num==10)slot_num=0;
	
	
	return snd_samples;


}










