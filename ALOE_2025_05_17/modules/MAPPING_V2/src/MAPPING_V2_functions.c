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

#include "MAPPING_V2_functions.h"

Sequences_t SequencesParams;
MAPP_t MAPPinfo; 

MAC_struc MACinfo;
Bufer_MAC MAC_buff;

//int ofset=0;//carrier number of frequency ofset in the GRID F-T

int slot_num=0;



_Complex float SSS[PSSlength];
_Complex float PSS[PSSlength];
_Complex float DMRS[DMRS_max_length];
_Complex float DMRS_Control[DMRS_max_length];



int Checkdelay(MAC_struc* MACin,int max_size,int Ts){
	int delay;
	delay=MACin->Ts_MAC;
	return delay;

}


void Buffer_Manager(MAPP_t* MAPPinfo,MAC_struc* inputMAC,int Ts,int delay){

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
	//printf("--------------NEW FEATURE--------- nPRB=%d Aditional DMS=%d   coderate=%f\n" ,out_buff.nPRB,out_buff.Additional_DMRS_position);

	//Advance_buffer(Bufffer,inputMAC,max_buffer_size);
	extract_MAC_info(MAPPinfo,&out_buff);



}


int extract_MAC_info(MAPP_t* MAPPinfo, MAC_struc* MACinfo){

	MAPPinfo->DMRS_type=MACinfo->DMRS_type;
	MAPPinfo->Additional_DMRS_position=MACinfo->Additional_DMRS_position;
	MAPPinfo->nPRB=MACinfo->nPRB;
	MAPPinfo->offset_data=MACinfo->CORESETS_symbol_size;
	MAPPinfo->Agreggation_lvl_DCI=MACinfo->Agr_lvl_DCI;
	SequencesParams.NID1=MACinfo->NID1;
	SequencesParams.NID2=MACinfo->NID2;
	return 0;
}


void first_symbols(int data_init_pos,int* data_per_symbol,MAPP_t* MAPPinfo,int* DMRS_per_symbol){
	for(int i=0;i<2;i++){
		if(i>=data_init_pos){
			data_per_symbol[i]=MAPPinfo->datalength;
			DMRS_per_symbol[i]=0;
	
		}else{
			data_per_symbol[i]=0;
			DMRS_per_symbol[i]=0;
		}

	}
}





void fill_structure_1_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int SSblock_flag,int data_init_pos){


	//printf("SSblock_flag=%d\n",SSblock_flag);
	//printf("datalength=%d DMRS_length=%d\n",MAPPinfo->datalength,MAPPinfo->DMRS_length);



	if(SSblock_flag==0){	
		
		for(int i=data_init_pos;i<MAPPinfo->numFFT;i++){
			if(i==2){
				DMRS_per_symbol[i]=MAPPinfo->datalength/2;
				data_per_symbol[i]=MAPPinfo->datalength-DMRS_per_symbol[i];
			}else{

			data_per_symbol[i]=MAPPinfo->datalength;
			DMRS_per_symbol[i]=0;

			}
		}
		
	}else{



	first_symbols(data_init_pos,data_per_symbol, MAPPinfo, DMRS_per_symbol);
				
	data_per_symbol[2]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;
	DMRS_per_symbol[2]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;


	for(int i=3;i<6;i++){
		data_per_symbol[i]=MAPPinfo->datalength-MAPPinfo->SSblock_length;
		DMRS_per_symbol[i]=0;
	}
	for(int i=6;i<8;i++){
		data_per_symbol[i]=MAPPinfo->datalength;
		DMRS_per_symbol[i]=0;
	}
	for(int i=8;i<12;i++){
		data_per_symbol[i]=MAPPinfo->datalength-MAPPinfo->SSblock_length;
		DMRS_per_symbol[i]=0;
	}
	for(int i=12;i<14;i++){
		data_per_symbol[i]=MAPPinfo->datalength;
		DMRS_per_symbol[i]=0;
	}




	}
}



void fill_structure_2_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int SSblock_flag,int data_init_pos){
	//printf("SSblock_flag=%d\n",SSblock_flag);
	//printf("datalength=%d DMRS_length=%d\n",MAPPinfo->datalength,MAPPinfo->DMRS_length);
   if(SSblock_flag==0){	
		
		for(int i=data_init_pos;i<MAPPinfo->numFFT;i++){
			if(i==2 || i==11){
				DMRS_per_symbol[i]=MAPPinfo->datalength/2;
				data_per_symbol[i]=MAPPinfo->datalength-DMRS_per_symbol[i];
			}else{

			data_per_symbol[i]=MAPPinfo->datalength;
			DMRS_per_symbol[i]=0;

			}
		}
		
   }else{



	first_symbols(data_init_pos,data_per_symbol, MAPPinfo,DMRS_per_symbol);
				
	data_per_symbol[2]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;
	DMRS_per_symbol[2]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;


	for(int i=3;i<6;i++){
		data_per_symbol[i]=MAPPinfo->datalength-MAPPinfo->SSblock_length;
		DMRS_per_symbol[i]=0;
	}
	for(int i=6;i<8;i++){
		data_per_symbol[i]=MAPPinfo->datalength;
		DMRS_per_symbol[i]=0;
	}
	for(int i=8;i<11;i++){
		data_per_symbol[i]=MAPPinfo->datalength-MAPPinfo->SSblock_length;
		DMRS_per_symbol[i]=0;
	}

	data_per_symbol[11]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;
	DMRS_per_symbol[11]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;
	

	for(int i=12;i<14;i++){
		data_per_symbol[i]=MAPPinfo->datalength;
		DMRS_per_symbol[i]=0;
	}


    }
}


void fill_structure_3_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int SSblock_flag,int data_init_pos){
	//printf("SSblock_flag=%d\n",SSblock_flag);
	//printf("datalength=%d DMRS_length=%d\n",MAPPinfo->datalength,MAPPinfo->DMRS_length);
   if(SSblock_flag==0){	
					
		for(int i=data_init_pos;i<MAPPinfo->numFFT;i++){
			if(i==2 || i==11 ||i==7){
				DMRS_per_symbol[i]=MAPPinfo->datalength/2;
				data_per_symbol[i]=MAPPinfo->datalength-DMRS_per_symbol[i];
			}else{

			data_per_symbol[i]=MAPPinfo->datalength;
			DMRS_per_symbol[i]=0;

			}
		}
		
   }else{



	first_symbols(data_init_pos,data_per_symbol, MAPPinfo,DMRS_per_symbol);
				
	data_per_symbol[2]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;
	DMRS_per_symbol[2]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;

	for(int i=3;i<6;i++){
		data_per_symbol[i]=MAPPinfo->datalength-MAPPinfo->SSblock_length;
		DMRS_per_symbol[i]=0;
	}

	data_per_symbol[6]=MAPPinfo->datalength;
		DMRS_per_symbol[6]=0;
	data_per_symbol[7]=(MAPPinfo->datalength)/2;
		DMRS_per_symbol[7]=(MAPPinfo->datalength)/2;


	for(int i=8;i<11;i++){
		data_per_symbol[i]=MAPPinfo->datalength-MAPPinfo->SSblock_length;
		DMRS_per_symbol[i]=0;
	}

	data_per_symbol[11]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;
	DMRS_per_symbol[11]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;
	

	for(int i=12;i<14;i++){
		data_per_symbol[i]=MAPPinfo->datalength;
		DMRS_per_symbol[i]=0;
	}


    }
}


void fill_structure_4_DMRS(int* data_per_symbol,int* DMRS_per_symbol,MAPP_t* MAPPinfo,int SSblock_flag,int data_init_pos){
	//printf("SSblock_flag=%d\n",SSblock_flag);
	//printf("datalength=%d DMRS_length=%d\n",MAPPinfo->datalength,MAPPinfo->DMRS_length);
   if(SSblock_flag==0){	
					
		for(int i=data_init_pos;i<MAPPinfo->numFFT;i++){
			if(i==2 || i==11 ||i==8 || i==5){
				DMRS_per_symbol[i]=MAPPinfo->datalength/2;
				data_per_symbol[i]=MAPPinfo->datalength-DMRS_per_symbol[2];
			}else{

			data_per_symbol[i]=MAPPinfo->datalength;
			DMRS_per_symbol[i]=0;

			}
		}
		
   }else{



	first_symbols(data_init_pos,data_per_symbol, MAPPinfo,DMRS_per_symbol);
				
	data_per_symbol[2]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;
	DMRS_per_symbol[2]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;


	for(int i=3;i<5;i++){
		data_per_symbol[i]=MAPPinfo->datalength-MAPPinfo->SSblock_length;
		DMRS_per_symbol[i]=0;
	}
	data_per_symbol[5]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;
	DMRS_per_symbol[5]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;

	data_per_symbol[6]=MAPPinfo->datalength;
	DMRS_per_symbol[6]=0;

	data_per_symbol[7]=MAPPinfo->datalength;
	DMRS_per_symbol[7]=0;
	
	data_per_symbol[8]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;
	DMRS_per_symbol[8]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;


	for(int i=9;i<11;i++){
		data_per_symbol[i]=MAPPinfo->datalength-MAPPinfo->SSblock_length;
		DMRS_per_symbol[i]=0;
	}

	data_per_symbol[11]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;
	DMRS_per_symbol[11]=(MAPPinfo->datalength-MAPPinfo->SSblock_length)/2;
	

	for(int i=12;i<14;i++){
		data_per_symbol[i]=MAPPinfo->datalength;
		DMRS_per_symbol[i]=0;
	}


    }
}


void generate_Subframe_structure(MAPP_t* MAPPinfo, int* data_per_symbol,int SSblock_flag,int data_init_pos,int* DMRS_per_symbol){

	//printf("Additional_DMRS_position=%d  MAPPinfo.DMRS_type=%d\n",MAPPinfo->Additional_DMRS_position,MAPPinfo->DMRS_type);

	if (MAPPinfo->DMRS_type==1){
		switch (MAPPinfo->Additional_DMRS_position) {
			
			case 0:
				fill_structure_1_DMRS(data_per_symbol,DMRS_per_symbol,MAPPinfo,SSblock_flag,data_init_pos);
				//printf("In switch generate structure\n");
				break;
			case 1:
				fill_structure_2_DMRS(data_per_symbol,DMRS_per_symbol,MAPPinfo,SSblock_flag,data_init_pos);
				break;
			case 2:
				fill_structure_3_DMRS(data_per_symbol,DMRS_per_symbol,MAPPinfo,SSblock_flag,data_init_pos);
				break;
			case 3:
				fill_structure_4_DMRS(data_per_symbol,DMRS_per_symbol,MAPPinfo,SSblock_flag,data_init_pos);
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
	


int GEN_DMRS_Control(int Slotnumber,int OFDMsymbol,_Complex float* DMRS_seq){

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
	int cinit=(int)remainder(pow(2,17)*(14*n+l+1)*(2*NIDscram+1)+2*NIDscram,pow(2,31));
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
	


int PSS_gen (int NID1,int NID2){
			
	int m[128];
	int x_m[128];
	
	for (int n=0;n<128;n++){
		m[n]=(n+43*NID2)%127; 
	}
	
	x_m[0]=0;x_m[1]=1;x_m[2]=1;x_m[3]=0;x_m[4]=1;x_m[5]=1;x_m[6]=1;
	
	for (int n=0;n<128-7;n++){
		x_m[n+7]=(x_m[n+4]+x_m[n])%2;
	}
	
	for (int n=0;n<127;n++){
		__real__ PSS[n] =(1-2*x_m[m[n]]);
		__imag__ PSS[n] =0.0;
	}
	PSS[128]=0.0+0.0*I;
	
	return(0);
}
		
		

int SSS_gen (int NID1,int NID2){
	

	
	int x_0[SSSlength];
	int x_1[SSSlength];
		

	int m0=15*floor(NID1/112)+5*NID2;
	int m1=(NID1%112);

	
	x_0[0]=1;x_0[1]=0;x_0[2]=0;x_0[3]=0;x_0[4]=0;x_0[5]=0;x_0[6]=0;
	x_1[0]=1;x_1[1]=0;x_1[2]=0;x_1[3]=0;x_1[4]=0;x_1[5]=0;x_1[6]=0;
	
	for (int n=0;n<SSSlength-7;n++){
		x_0[n+7]=(x_0[n+4]+x_0[n])%2;
		x_1[n+7]=(x_1[n+1]+x_1[n])%2;
		
	}
	
	for (int n=0;n<SSSlength;n++){
		__real__ SSS[n] =(float)(1-2*(x_0[(n+m0)%127]))*(1-2*(x_1[(n+m1)%127]));
		__imag__ SSS[n]=0.0;
	}
	
	
	return(0);
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


int UNDO_Mapping(MAPP_t* MAPPinfo,_Complex float *inspectrum,int* data_per_symbol,int SSblock_flag,_Complex float *dataout,int data_init_pos,int* DMRS_per_symbol,_Complex float* out_control_data, int* snd_samples_control){

	int snd_samples;
	_Complex float tmp[2048*50]={0.0+0.0*I};

	//calcsubframe params
	Calc_params(MAPPinfo,&SequencesParams,slot_num);	


	//Gen subframe structure
	generate_Subframe_structure(MAPPinfo,data_per_symbol, SSblock_flag, data_init_pos,DMRS_per_symbol);


	//UNDO_Spectrum
	UNDO_Spectrum(MAPPinfo,inspectrum, &tmp);


	//GET DATA FROM GRID PDSCH
	snd_samples=GET_Subframe(MAPPinfo,slot_num,&tmp,data_per_symbol, DMRS_per_symbol,dataout,data_init_pos,SSblock_flag);


	//GET CONTROL DATA FROM PDCCH

	//TEST
	//_Complex float test[20*2048];
	*(snd_samples_control)=GET_PDCCH(out_control_data,&tmp,data_init_pos,MAPPinfo);
	
	/*for(int i=0;i<280;i++){	
		printf("control symbols=%f at i=%d\n",creal(test[i]),i);	
		
	}*/



	/*
		printf("OUTSDATA------------------\n");
	for(int j=0;j<MAPPinfo->numFFT;j++){
		for(int i=0;i<MAPPinfo->datalength;i++){	
		//printf("outdemap=%f at i=%d\n",creal(dataout[i+j*MAPPinfo->datalength]),i);	
		printf("Real in[%d %d]=%f IMAG in =%f\n",i,j,creal(dataout[i+24*12*j]),cimag(dataout[i+24*12*j]));
		}
	printf("\n");
	}*/

	//Update slot num WITHIN FRAME
	slot_num++;
	if (slot_num==10)slot_num=0;
	
	
	return snd_samples;


}



int GET_Subframe(MAPP_t* MAPPinfo,int slot_num,_Complex float* inMQAMsymb,int* data_per_symbol,int* DMRS_per_symbol,_Complex float *out_data,int data_init_pos,int SS_block_flag){

	int OFDMsym=0;
	int in_pos=0;
	int out_pos=0;
	for(int i=0;i<MAPPinfo->numFFT;i++){

		GETsymbols(MAPPinfo,slot_num,OFDMsym,inMQAMsymb+in_pos,out_data+out_pos, data_per_symbol,DMRS_per_symbol,data_init_pos,SS_block_flag);
		
	
		OFDMsym++;
		in_pos=in_pos+MAPPinfo->datalength;
		out_pos=out_pos+data_per_symbol[i];
				

	}


	return out_pos;
	


}

/*
int isSSinSymb(int SS_block_flag){
	if(SS_block_flag==1){

		if(OFDMsymbol==2 || OFDMsymbol==3 ||OFDMsymbol==4 ||OFDMsymbol==5 ||OFDMsymbol==8 ||OFDMsymbol==9 ||OFDMsymbol==10 ||OFDMsymbol==11 ){
			SS_in_symb=1;
		}
		
	}


}
*/
void GETsymbols(MAPP_t* MAPPinfo,int Slotnumber,int OFDMsymbol,_Complex float *in_data,_Complex float* out,int *data_per_symbol,int* DMRS_per_symbol,int data_init_pos,int SS_block_flag){
	//printf("DMRS_per_symbol[OFDMsymbol]=%d\n",DMRS_per_symbol[OFDMsymbol]);	

	///////////////GENERATE 2D DUBFRAME GRID///////////////////

	int SS_in_symb=0;
	if(SS_block_flag==1){

		if(OFDMsymbol==2 || OFDMsymbol==3 ||OFDMsymbol==4 ||OFDMsymbol==5 ||OFDMsymbol==8 ||OFDMsymbol==9 ||OFDMsymbol==10 ||OFDMsymbol==11 ){
			SS_in_symb=1;
		}
		
	}

	int ofset=(MAPPinfo->datalength-SS_block_length)/2;
	
	//printf("SSin_symb=%d at symb=%d\n",SS_in_symb,OFDMsymbol);

	if(DMRS_per_symbol[OFDMsymbol]==0 && data_per_symbol[OFDMsymbol]!=0){
		if(SS_in_symb==0){			
					
			memcpy(out,in_data,data_per_symbol[OFDMsymbol]*sizeof(_Complex float));
			
		}else{
			memcpy(out,in_data,data_per_symbol[OFDMsymbol]*sizeof(_Complex float)/2);
		

			memcpy(out+data_per_symbol[OFDMsymbol]/2,in_data+SS_block_length+ofset,data_per_symbol[OFDMsymbol]*sizeof(_Complex float)/2);
			
		
		}
	}
		
	

	if(DMRS_per_symbol[OFDMsymbol]!=0 && data_per_symbol[OFDMsymbol]!=0){
		
		//int datalength2mix=2*data_per_symbol[OFDMsymbol];

		if(SS_in_symb==0){
			DEmix_data(MAPPinfo->datalength/2, in_data,out);
		//	printf("INIFFFFFFFFFFFF\n");
	
		
		}else{
		
		//OJO AMB AIXÓ
			DEmix_data(ofset, in_data,out);
			DEmix_data(ofset, in_data+SS_block_length+ofset,out+ofset/2);
		}
		
 			
	}
	


}

void DEmix_data(int datalength,_Complex float* in, _Complex float* out){
	for(int i=0;i<datalength;i++){
		__real__ *(out+i) = __real__ *(in+i*2+1);
		__imag__ *(out+i) = __imag__ *(in+i*2+1);
	}

}


//#define ONLY_PSS


int Do_Mapping(MAPP_t* MAPPinfo, _Complex float *inMQAMsymb, int *data_per_symbol, int SSblock_flag, _Complex float *subframe, 
							int data_init_pos,int *DMRS_per_symbol, int rcv_samples_controlchann, _Complex float *control_data){

	_Complex float tmp[2048*20]={0.0+0.0*I};
	int OFDMsymbol=0;
	int snd_samples=0;
	int i;

	//printf("Do_Mapping: slot_num=%d\n", slot_num);

#ifndef ONLY_PSS
	//FILL SUBFRAME with zeros
	for(i=0; i<MAPPinfo->numFFT*MAPPinfo->FFTlength; i++)*(subframe+i)=0.0+0.0*I;
#endif

	//calcsubframe params
	Calc_params(MAPPinfo,&SequencesParams,slot_num);	


	//Gen subframe structure
	generate_Subframe_structure(MAPPinfo,data_per_symbol, SSblock_flag, data_init_pos,DMRS_per_symbol);


	//crate subframe
	Create_Subframe(MAPPinfo,slot_num,inMQAMsymb,data_per_symbol,DMRS_per_symbol,&tmp,data_init_pos,SSblock_flag);
	


	//ADD Control RE to subframe


	//TEST ONLY DATA
/*
	_Complex float fake_Control[20*2048]={0.0+0.0*I};
	//_Complex float Control_symbol[24*12]={0.0+0.0*I};
	int control_length=rcv_samples_controlchann;
	for(int i=0;i<control_length;i++){
		fake_Control[i]=((float)i)+(float)i*I;
	}
*/	
	//printf("rcv_samples_controlchann=%d\n",rcv_samples_controlchann);
	//Fill_PDCCH(&DMRS_Control,&fake_Control,&tmp,control_length, data_init_pos,MAPPinfo,slot_num);


#ifndef ONLY_PSS	
	Fill_PDCCH(&DMRS_Control, control_data, &tmp, rcv_samples_controlchann, data_init_pos, MAPPinfo,slot_num);
#endif	



	
	//CREATE SPECTRUM
	createSpectrum(MAPPinfo,&tmp,subframe);

	//printf("FFTlength=%d \n",MAPPinfo->FFTlength);
	
	/*for(int j=0;j<14;j++){
		for(int i=0;i<MAPPinfo->FFTlength;i++){	
			printf("outdemap=%f at i=%d\n",creal(subframe[i+j*MAPPinfo->FFTlength]),i);	
		}
		printf("\n");
	}
	printf("\n");

*/
	
	//Update slot num WITHIN FRAME
	slot_num++;
	if (slot_num==10)slot_num=0;

	snd_samples=MAPPinfo->numFFT*MAPPinfo->FFTlength;
	return snd_samples;
	
	
}




void Create_Subframe(MAPP_t* MAPPinfo,int slot_num,_Complex float* inMQAMsymb,int* data_per_symbol,int* DMRS_per_symbol,_Complex float *tmp,int data_init_pos,int SS_block_flag){


	//printf("\n");
	//printf("GENSYMBOLS------------------\n");

	int OFDMsym=0;
	int in_pos=0;
	int out_pos=0;
	for(int i=0;i<MAPPinfo->numFFT;i++){
	
		gensymbols(MAPPinfo,&SequencesParams,slot_num,OFDMsym,inMQAMsymb+in_pos,tmp+out_pos,data_per_symbol,DMRS_per_symbol,data_init_pos,SS_block_flag);
		
		
		OFDMsym++;
		in_pos=in_pos+data_per_symbol[i];
		out_pos=out_pos+MAPPinfo->datalength;
		

	}

}


void gensymbols(MAPP_t* MAPPinfo,Sequences_t* SequencesParams,int Slotnumber,int OFDMsymbol,_Complex float *in_data,_Complex float* out,int *data_per_symbol,int* DMRS_per_symbol,int data_init_pos,int SS_block_flag){
	

	///////////////GENERATE 2D DUBFRAME GRID///////////////////

	int SS_in_symb=0;
	int ofset=(MAPPinfo->datalength-SS_block_length)/2;

	//TODO: fer aixo una funció
	if(SS_block_flag==1){

		if(OFDMsymbol==2 || OFDMsymbol==3 ||OFDMsymbol==4 ||OFDMsymbol==5 ||OFDMsymbol==8 ||OFDMsymbol==9 ||OFDMsymbol==10 ||OFDMsymbol==11 ){
			SS_in_symb=1;
		}
		
	}

#ifndef ONLY_PSS	
	if(DMRS_per_symbol[OFDMsymbol]==0 && data_per_symbol[OFDMsymbol]!=0){
		if(SS_in_symb==0){			
			memcpy(out,in_data,data_per_symbol[OFDMsymbol]*sizeof(_Complex float));
		

		}else{

			memcpy(out,in_data,data_per_symbol[OFDMsymbol]*sizeof(_Complex float)/2);
			memcpy(out+ofset+SS_block_length,in_data+data_per_symbol[OFDMsymbol]/2,data_per_symbol[OFDMsymbol]*sizeof(_Complex float)/2);
			//(MAPPinfo,out,ofset);
		}
		
	}

	
	if(DMRS_per_symbol[OFDMsymbol]!=0 && data_per_symbol[OFDMsymbol]!=0){
		
		//int datalength2mix=2*data_per_symbol[OFDMsymbol];
		GEN_DMRS(Slotnumber,OFDMsymbol,&DMRS);
		//printf("data_per_symbol[OFDMsymbol]=%d\n",data_per_symbol[OFDMsymbol]);
		if(SS_in_symb==0){
			mix_data(&DMRS,in_data,out,(MAPPinfo->datalength)/2,0);
		}else{
			mix_data(&DMRS,in_data,out,(MAPPinfo->datalength-SS_block_length)/2,0);

			mix_data(&DMRS,in_data+data_per_symbol[OFDMsymbol]/2,out+ofset+SS_block_length,(MAPPinfo->datalength-SS_block_length)/2,ofset+SS_block_length);

			fill_zeros(MAPPinfo,out,ofset);

		}
					
	}

	//if(OFDMsymbol<data_init_pos){
	if(DMRS_per_symbol[OFDMsymbol]==0 && data_per_symbol[OFDMsymbol]==0){
		for(int i=0;i<MAPPinfo->datalength;i++){
			out[i]=0.0+0.0*I;
		}
	}
#endif

	//ADD PSS AND SSS
	if(SS_block_flag==1){
		if(OFDMsymbol==2||OFDMsymbol==8)memcpy(out+ofset+PSS_offset,PSS,PSSlength*sizeof(_Complex float));
//#ifndef ONLY_PSS	
	//	if(OFDMsymbol==4||OFDMsymbol==10)memcpy(out+ofset+PSS_offset,SSS,SSSlength*sizeof(_Complex float));
//#endif
	}
	
}


/*

//out_symbol son les portadores d'un symbol OFDM que conté SSB sense incloure les de PDSCH. Aquestes es monten a la trama en un altre lloc.
void fill_SSB_symbol(_Complex float* in_PDCCH,_Complex float* out_symbol,int OFDMsymbol,int ofset){
	switch (OFDMsymbol) {
			case 2:
				memcpy(out+PSS_offset,PSS,PSSlength*sizeof(_Complex float));
				break;
			case 3:
				memcpy(out+PSS_offset,PSS,PSSlength*sizeof(_Complex float));
				break;	




			case 8:
				memcpy(out+PSS_offset,PSS,PSSlength*sizeof(_Complex float));
				break;	
		


	
	if(OFDMsymbol==4||OFDMsymbol==10)memcpy(out+ofset+PSS_offset,SSS,SSSlength*sizeof(_Complex float));



}
*/

/*
void create_SSB(_Complex float* DMRS_SSB, _Complex float* in_BCH,_Complex float* out_SSB){




}*/


void Fill_PDCCH(_Complex float *DMRSseq,_Complex float *inMQAMsymb,_Complex float *inout_frame,int control_data_length, int data_init_pos,MAPP_t* MAPPinfo,int Slotnumber){
//Control_data_length is the number of QAM symbols from control channels

	int DMRS_RE_num_perSymbol=ceil(((float)MAPPinfo->nPRB*12)/4.0);
	//int RE_Control_data_perSymbol=MAPPinfo->nPRB*12-DMRS_RE_num_perSymbol;
	//int symb_necessaris=ceil((float)control_data_length/(float)RE_Control_data_perSymbol);

	float RE_data_and_DMRS=(float)DMRS_SEPARATION_Control_Channel/(float)(DMRS_SEPARATION_Control_Channel-1);

	//printf("symb_necessaris=%d, DMRS_RE_num_perSymbol=%d, RE_Control_data_perSymbol=%d, MAPPinfo->nPRB=%d, RE_data_and_DMRS=%d \n",symb_necessaris,DMRS_RE_num_perSymbol,RE_Control_data_perSymbol,MAPPinfo->nPRB*12,RE_data_and_DMRS);	

	int data_already_in=0;

	//Case only one Symbol for control
	if(data_init_pos==1){
		GEN_DMRS_Control(Slotnumber,1,DMRSseq);
		data_already_in=mix_control_data(DMRSseq, inMQAMsymb, inout_frame, ceil((float)control_data_length*RE_data_and_DMRS));
	}

	//Case TWO Symbol for control
	int DMRS_already_in=DMRS_RE_num_perSymbol;
	if(data_init_pos==2){
		//First symbol
		GEN_DMRS_Control(Slotnumber,1,DMRSseq);
		data_already_in=mix_control_data(DMRSseq, inMQAMsymb, inout_frame, MAPPinfo->nPRB*12);
		
		//Second symbol
		GEN_DMRS_Control(Slotnumber,2,DMRSseq);
		mix_control_data(DMRSseq, inMQAMsymb+data_already_in, inout_frame+MAPPinfo->datalength,ceil((float)(control_data_length-data_already_in))*RE_data_and_DMRS+1);
	}


	/*for(int i=0;i<12*24*3;i++){	
		printf("control symbols=%f at i=%d\n",creal(inout_frame[i]),i);	
		
	}*/
	

}

int mix_control_data(_Complex float *DMRSseq,_Complex float *inMQAMsymb,_Complex float *out_symb,int control_data_length){	

	int pos_data_in=0;
	
	for(int i=0;i<control_data_length;i++){

		if(i%DMRS_SEPARATION_Control_Channel!=0){		
			__real__ *(out_symb+i)=__real__ *(inMQAMsymb+pos_data_in);
			__imag__ *(out_symb+i)=__imag__ *(inMQAMsymb+pos_data_in);
		pos_data_in++;

		}else{
			__real__ *(out_symb+i)=__real__ *(DMRSseq+i);
			__imag__ *(out_symb+i)=__imag__ *(DMRSseq+i);
		}
			

	}
	return pos_data_in;
	

}

int GET_PDCCH(_Complex float *outMQAMsymb,_Complex float *in_frame,int data_init_pos,MAPP_t* MAPPinfo){

	int out_symbols = Calc_symb_out_control(MAPPinfo->Agreggation_lvl_DCI, data_init_pos);
	
	//Variable de test
	//int length=296;


	float RE_data_and_DMRS=(float)DMRS_SEPARATION_Control_Channel/(float)(DMRS_SEPARATION_Control_Channel-1);

	int RE_to_demix=ceil(RE_data_and_DMRS*(float)out_symbols);

	demix_control_data(in_frame,outMQAMsymb,RE_to_demix);
	
	/*for(int i=0;i<out_symbols+5;i++){	
		printf("control symbols=%f at i=%d\n",creal(outMQAMsymb[i]),i);	
		
	}*/
	return out_symbols;


}


int demix_control_data(_Complex float *inOFDMsymb,_Complex float *out_symb,int length){	

	int data_pos=0;
	for(int i=0;i<length;i++){
		if(i%DMRS_SEPARATION_Control_Channel!=0){		
			__real__ *(out_symb+data_pos)=__real__ *(inOFDMsymb+i);
			__imag__ *(out_symb+data_pos)=__imag__ *(inOFDMsymb+i);
			data_pos++;
		}
	}
}




int Calc_symb_out_control(int aggregation_level_DCI,int CORESET_symb_num){

		int K=88;//size in bits of DCI on MAC
		int E=calc_out_bits_max_length(aggregation_level_DCI, CORESET_symb_num); // bits out of rate match
	return E/2;
}



int calc_out_bits_max_length(int Carr_agr_lvl,int CORESET_symb_num){
	int CCE_num=Carr_agr_lvl;
	int REG_per_CCE=6;
	int RE_DMRS_per_CCE=18;
	int RE_in_REG=12;
	int symb_num=CORESET_symb_num;

	int CCE_total=REG_per_CCE*CCE_num;
	int Available_RE_total=RE_in_REG*symb_num*CCE_total;
	int RE_DMRS_total=RE_DMRS_per_CCE*CCE_num;
	int RE_data=Available_RE_total-RE_DMRS_total;
	int bits_data=2*RE_data; // 2 comes from QPSK is always used in PDCCH
	//printf("E=%d\n",bits_data);
	return bits_data;

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


void fill_zeros(MAPP_t* MAPPinfo,_Complex float* out,int ofset){

		for(int i=ofset;i<ofset+SS_block_length;i++){
			out[i]=0.0+0.0*I;
		}
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

/*
int FFT_length_calc(MAPP_t* MAPPinfo){
	
	
	int dif;
	for (int i=0;i<32;i++){
		dif=pow(2,i)-MAPPinfo->datalength;
		if (dif>0){
			return pow(2,i);
		}

	}


}*/

void Calc_params(MAPP_t* MAPPinfo,Sequences_t* SequencesParams,int Slot_number){

	
	MAPPinfo->datalength=MAPPinfo->nPRB*12;//12 carriers per PRB
	MAPPinfo->DMRS_length=MAPPinfo->datalength/2;
	
	if(MAPPinfo->nPRB<=25)MAPPinfo->FFTlength=512;
	if(MAPPinfo->nPRB>25 && MAPPinfo->nPRB<=79)MAPPinfo->FFTlength=1024;

	MAPPinfo->SSblock_length=SS_block_length;
	//printf("MAPPinfo->DMRS_length=%d datalength=%d\n",MAPPinfo->DMRS_length,MAPPinfo->datalength);
	
	//SequencesParams->NID1=1;
	//SequencesParams->NID2=2;
	SequencesParams->OFDMsymbol=1;
	SequencesParams->Slotnumber=Slot_number; //slot number within a frame
	SequencesParams->DMRSlength=MAPPinfo->DMRS_length;


	SSS_gen (SequencesParams->NID1,SequencesParams->NID2);
	PSS_gen (SequencesParams->NID1,SequencesParams->NID2);

}










