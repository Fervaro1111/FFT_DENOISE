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
#include <string.h>

#include "MAPPING_functions.h"

Sequences_t SequencesParams;
MAPP_t MAPPinfo; 





_Complex float SSS[PSSlength];
_Complex float PSS[PSSlength];
_Complex float DMRS[240*10];
_Complex float DMRS2[240*10];
_Complex float DMRS5[240*10];
_Complex float DMRS7[240*10];
_Complex float DMRS8[240*10];
_Complex float DMRS11[240*10];










/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */

int offset_data(OFDMsymb_data){
	int data_offset; //can be 2 or 6 depending on if there's SS block
	if (OFDMsymb_data==12){
		data_offset=2;
	}else{
		data_offset=6;
	}
	return data_offset;
}



void mix_data(_Complex float *DMRSseq,_Complex float *inMQAMsymb,_Complex float *out_symb,int data_length){	

	for (int k=0;k<data_length;k++){
		__real__*(out_symb+2*k)=__real__ *(DMRSseq+k);
		__imag__*(out_symb+k*2)=__imag__ *(DMRSseq+k);
	
		__real__*(out_symb+2*k+1)=__real__ *(inMQAMsymb+k);
		__imag__*(out_symb+k*2+1)=__imag__ *(inMQAMsymb+k);

	}
}

void fill_structure_NO_DMRS(int* data_per_symbol,MAPP_t MAPPinfo,int SSblock_flag){
	
	if(SSblock_flag==0){	
		for(int i=2;i<MAPPinfo.numFFT;i++){
			data_per_symbol[i]=MAPPinfo.datalength;
		}
	

	}else{


		for(int i=2;i<6;i++){
			data_per_symbol[i]=MAPPinfo.datalength-MAPPinfo.SSblock_length;
		}
		for(int i=6;i<8;i++){
			data_per_symbol[i]=MAPPinfo.datalength;
		}
		for(int i=8;i<12;i++){
			data_per_symbol[i]=MAPPinfo.datalength-MAPPinfo.SSblock_length;
		}
		for(int i=12;i<14;i++){
			data_per_symbol[i]=MAPPinfo.datalength;
		}
	
		
	}

}



void first_symbols(int data_init_pos,int* data_per_symbol,MAPP_t MAPPinfo){
	for(int i=0;i<2;i++){
		if(i>=data_init_pos){
			data_per_symbol[i]=MAPPinfo.datalength;
	
		}

	}
}

void FILL_with_SS(int* data_per_symbol,MAPP_t MAPPinfo){
			for(int i=2;i<6;i++){
				data_per_symbol[i]=MAPPinfo.datalength-MAPPinfo.SSblock_length;
			}
			for(int i=6;i<8;i++){
				data_per_symbol[i]=MAPPinfo.datalength;
			}
			for(int i=8;i<12;i++){
				data_per_symbol[i]=MAPPinfo.datalength-MAPPinfo.SSblock_length;
			}
			for(int i=12;i<14;i++){
				data_per_symbol[i]=MAPPinfo.datalength;
			}


}



void fill_structure_1_DMRS(int* data_per_symbol,MAPP_t MAPPinfo,int SSblock_flag,int data_init_pos){
	printf("SSblock_flag=%d\n",SSblock_flag);
	printf("datalength=%d DMRS_length=%d\n",MAPPinfo.datalength,MAPPinfo.DMRS_length);
	if(SSblock_flag==0){	
		
		
		for(int i=data_init_pos;i<MAPPinfo.numFFT;i++){
			data_per_symbol[i]=MAPPinfo.datalength;
		}

		data_per_symbol[2]=MAPPinfo.datalength-MAPPinfo.DMRS_length;


	}else{
	


		first_symbols(data_init_pos,data_per_symbol, MAPPinfo);
		FILL_with_SS(data_per_symbol,MAPPinfo);


	}
}


void fill_structure_2_DMRS(int* data_per_symbol,MAPP_t MAPPinfo,int SSblock_flag,int data_init_pos){

	if(SSblock_flag==0){	
		for(int i=data_init_pos;i<MAPPinfo.numFFT;i++){
			data_per_symbol[i]=MAPPinfo.datalength;
		}
		data_per_symbol[2]=MAPPinfo.datalength-MAPPinfo.DMRS_length;
		data_per_symbol[11]=MAPPinfo.datalength-MAPPinfo.DMRS_length;

	}else{

		first_symbols(data_init_pos,data_per_symbol, MAPPinfo);
	
		FILL_with_SS(data_per_symbol,MAPPinfo);
	}
}

void fill_structure_3_DMRS(int* data_per_symbol,MAPP_t MAPPinfo,int SSblock_flag,int data_init_pos){

	if(SSblock_flag==0){	
		for(int i=data_init_pos;i<MAPPinfo.numFFT;i++){
			data_per_symbol[i]=MAPPinfo.datalength;
		}
		data_per_symbol[2]=MAPPinfo.datalength-MAPPinfo.DMRS_length;
		data_per_symbol[7]=MAPPinfo.datalength-MAPPinfo.DMRS_length;
		data_per_symbol[11]=MAPPinfo.datalength-MAPPinfo.DMRS_length;

	}else{
		first_symbols(data_init_pos,data_per_symbol, MAPPinfo);
	
		for(int i=2;i<6;i++){
			data_per_symbol[i]=MAPPinfo.datalength-MAPPinfo.SSblock_length;
		}

		data_per_symbol[6]=MAPPinfo.datalength;
		data_per_symbol[7]=MAPPinfo.datalength-MAPPinfo.DMRS_length;
		
		for(int i=8;i<12;i++){
			data_per_symbol[i]=MAPPinfo.datalength-MAPPinfo.SSblock_length;
		}
		for(int i=12;i<14;i++){
			data_per_symbol[i]=MAPPinfo.datalength;
		}
	
	}
}


void fill_structure_4_DMRS(int* data_per_symbol,MAPP_t MAPPinfo,int SSblock_flag,int data_init_pos){

	if(SSblock_flag==0){	
		for(int i=data_init_pos;i<MAPPinfo.numFFT;i++){
			data_per_symbol[i]=MAPPinfo.datalength;
		}
		data_per_symbol[2]=MAPPinfo.datalength-MAPPinfo.DMRS_length;
		data_per_symbol[5]=MAPPinfo.datalength-MAPPinfo.DMRS_length;
		data_per_symbol[8]=MAPPinfo.datalength-MAPPinfo.DMRS_length;
		data_per_symbol[11]=MAPPinfo.datalength-MAPPinfo.DMRS_length;
	}else{

		first_symbols(data_init_pos,data_per_symbol, MAPPinfo);
	
		FILL_with_SS(data_per_symbol,MAPPinfo);
	
	}
}





void generate_Subframe_structure(MAPP_t MAPPinfo, int* data_per_symbol,int SSblock_flag,int data_init_pos){

	printf("Additional_DMRS_position=%d  MAPPinfo.DMRS_type=%d\n",MAPPinfo.Additional_DMRS_position,MAPPinfo.DMRS_type);

	if (MAPPinfo.DMRS_type==1){
		switch (MAPPinfo.Additional_DMRS_position) {
			
			case 0:
				fill_structure_1_DMRS(data_per_symbol,MAPPinfo,SSblock_flag,data_init_pos);
				printf("In switch generate structure\n");
				break;
			case 1:
				fill_structure_2_DMRS(data_per_symbol,MAPPinfo,SSblock_flag,data_init_pos);
				break;
			case 2:
				fill_structure_3_DMRS(data_per_symbol,MAPPinfo,SSblock_flag,data_init_pos);
				break;
			case 3:
				fill_structure_4_DMRS(data_per_symbol,MAPPinfo,SSblock_flag,data_init_pos);
				break;
			
			default:
				printf("NOT PROGRAMMED YET\n");
				break;
		}
	}else{
		printf("INVALID DMRS TYpe\n");
	}
	
	for(int i=0;i<14;i++){
		printf("data per symbol=%d\n",data_per_symbol[i]);
	}

}


void demix_data(_Complex float *inMQAMsymb,_Complex float *out_symb,int data_length){	

	for (int k=0;k<data_length;k++){
			
		if(k%2!=0){
			__real__*(out_symb+k/2)=__real__ *(inMQAMsymb+k);
			__imag__*(out_symb+k/2)=__imag__ *(inMQAMsymb+k);
		}
	}
}

void REMOVE_DMRS(int data_offset,MAPP_t MAPPinfo,_Complex float* insymb,int* data_per_symbol,_Complex float* out_data){

	int out_pos=0;
	_Complex float symb_without_DMRS[2048];

	for(int i=data_offset;i<MAPPinfo.numFFT;i++){
		
		if(data_per_symbol[i]==MAPPinfo.datalength-MAPPinfo.DMRS_length){
			demix_data(insymb+MAPPinfo.datalength*i,&symb_without_DMRS,MAPPinfo.datalength);
			memcpy(out_data+out_pos,&symb_without_DMRS,data_per_symbol[i]*sizeof(_Complex float));
		}else{
		
			memcpy(out_data+out_pos,insymb+MAPPinfo.datalength*i,data_per_symbol[i]*sizeof(_Complex float));

		}



	out_pos=out_pos+data_per_symbol[i];

	}


}






void extract_data_from_subframe(MAPP_t MAPPinfo,int* data_per_symbol,_Complex float*INspectrum,_Complex float* symbOUT,int offset_data){

	for(int i=offset_data;i<MAPPinfo.numFFT;i++){
		getDATAfromLTE_DOWNLINKspectrum(INspectrum+i*MAPPinfo.FFTlength,MAPPinfo.FFTlength, MAPPinfo.datalength,symbOUT+MAPPinfo.datalength*i);
	
	}
	

}




void do_demap(MAPP_t MAPPinfo,_Complex float *insubframe,int* data_per_symbol,int SSblock_flag,_Complex float *RX_symbolsOUT,int data_init_pos){
	
	_Complex float tmp[2048*50];

	//GENERATE PSUDO_RANDOM SEQUENCES
	calc_sequences(SequencesParams);

	//data per symbol calc
	generate_Subframe_structure(MAPPinfo,data_per_symbol, SSblock_flag,data_init_pos);


	//Extract data from subframe
	extract_data_from_subframe(MAPPinfo,data_per_symbol, insubframe,&tmp,data_init_pos);


	//REMOVE DMRS
	REMOVE_DMRS(data_init_pos,MAPPinfo,&tmp, data_per_symbol,RX_symbolsOUT);


	

}







void Do_Mapping(MAPP_t MAPPinfo,_Complex float *inMQAMsymb,int* data_per_symbol,int SSblock_flag,_Complex float *subframe,int data_init_pos){

	_Complex float mixed_buffer[3584*10]={0.0+0.0*I};



	//GENERATE PSUDO_RANDOM SEQUENCES
	calc_sequences(SequencesParams);

	//data per symbol calc
	generate_Subframe_structure(MAPPinfo,data_per_symbol, SSblock_flag,data_init_pos);
	//ADD DATA to subframe
	gen_subframe(MAPPinfo,data_per_symbol,subframe,inMQAMsymb);
	//ADD PSS AND SSS to subframe
	ADD_SSS_PSS(MAPPinfo,subframe,SSblock_flag);





}

void ADD_SSS_PSS(MAPP_t MAPPinfo,_Complex float* subframe,int SSblock_flag){
	
	_Complex float symbol[256*10];//={0.0+0.0*I};
	
	if(SSblock_flag	==1){
		//ADD PSS
		fill_symbol(&PSS,MAPPinfo.FFTlength,PSSlength,&symbol);
		cpy2subframe(subframe+MAPPinfo.FFTlength*2,&symbol,MAPPinfo.FFTlength);
		fill_symbol(&PSS,MAPPinfo.FFTlength,PSSlength,&symbol);
		cpy2subframe(subframe+MAPPinfo.FFTlength*8,&symbol,MAPPinfo.FFTlength);

		//ADD SSS
		fill_symbol(&SSS,MAPPinfo.FFTlength,PSSlength,&symbol);
		cpy2subframe(subframe+MAPPinfo.FFTlength*4,&symbol,MAPPinfo.FFTlength);
		fill_symbol(&SSS,MAPPinfo.FFTlength,PSSlength,&symbol);
		cpy2subframe(subframe+MAPPinfo.FFTlength*10,&symbol,MAPPinfo.FFTlength);


	}
	

}

void gen_subframe(MAPP_t MAPPinfo,int* data_per_symbol,_Complex float* subframe,_Complex float* insymb){
	
	int in_pos=0;
	_Complex float insymb_with_DMRS[256*10];
	_Complex float symbol[256*10];//={0.0+0.0*I};

	for(int i=0;i<MAPPinfo.numFFT;i++){
		add_DMRSanddata_to_subframe(MAPPinfo,data_per_symbol,subframe,insymb,i,&insymb_with_DMRS,&symbol,in_pos);

		in_pos=in_pos+data_per_symbol[i];

	}

}



void add_DMRSanddata_to_subframe(MAPP_t MAPPinfo,int* data_per_symbol,_Complex float* subframe,_Complex float* insymb,int i,_Complex float* insymb_with_DMRS,_Complex float* symbol,int in_pos){

		if(data_per_symbol[i]==MAPPinfo.datalength-MAPPinfo.DMRS_length){
			mix_data(&DMRS,insymb+in_pos,insymb_with_DMRS,MAPPinfo.datalength);
			fill_symbol(insymb_with_DMRS,MAPPinfo.FFTlength,MAPPinfo.datalength,symbol);
		}else{
		fill_symbol(insymb+in_pos,MAPPinfo.FFTlength,data_per_symbol[i],symbol);
		}

		cpy2subframe(subframe+i*MAPPinfo.FFTlength,symbol,MAPPinfo.FFTlength);

		/*for(int i=0;i<256;i++){	
		printf("insymb_with_DMRS=%f at i=%d\n",creal(symbol[i]),i);	
		}
		printf("\n");
*/
}




void cpy2subframe(_Complex float *subframe,_Complex float *symbol,int FFTsize){
	memcpy(subframe,symbol,FFTsize*sizeof(_Complex float));
}


void fill_symbol(_Complex float *inMQAMsymb,int FFTlength,int datalength,_Complex float *symbol){
	
	
	int e=create_LTEspectrumNORS(inMQAMsymb,FFTlength,datalength,symbol);
	/*for(int i=0;i<256;i++){	
	printf("output0=%f at i=%d\n",creal(symbol[i]),i);	
	}
	printf("\n");
	*/
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
	














































void calc_sequences(Sequences_t SequencesParams){

	SSS_gen (SequencesParams.NID1,SequencesParams.NID2);
	PSS_gen (SequencesParams.NID1,SequencesParams.NID2);
	GEN_DMRS(SequencesParams.Slotnumber,SequencesParams.DMRSlength,2,&DMRS);	
	GEN_DMRS(SequencesParams.Slotnumber,SequencesParams.DMRSlength,2,&DMRS2);
	GEN_DMRS(SequencesParams.Slotnumber,SequencesParams.DMRSlength,5,&DMRS5);
	GEN_DMRS(SequencesParams.Slotnumber,SequencesParams.DMRSlength,7,&DMRS7);
	GEN_DMRS(SequencesParams.Slotnumber,SequencesParams.DMRSlength,8,&DMRS8);
	GEN_DMRS(SequencesParams.Slotnumber,SequencesParams.DMRSlength,11,&DMRS11);





	//printf("DMRS\n");
	/*for(int i=0;i<240;i++){
		output[i]=DMRS[i];
		printf("%f + %f*I \n",creal(DMRS[i]),cimag(DMRS[i]));
	}

	printf("DMRS\n");*/
}




int GEN_DMRS(int Slotnumber, int DMRSlength,int OFDMsymbol,_Complex float* DMRS_seq){
	
	int X1[4000]={0}; //1855=1600(NC)+240(dmrslength)+32
	X1[0]=1;	
	int X2[4000]={0};
	int c[4000]={0};

	
	int d=DMRSlength;
	
	
	int l; //symbol number within the slot
	int n;//slot number withinthe frame
	l=OFDMsymbol;
	n=Slotnumber;
	
	//TODO:fer que aixo sigui configurable i no HARCODE

	int NIDscram=0;
	int nscid=5;



	int cinit=pow(2,17)*(14*n+l+1)*(2*NIDscram+1)+2*NIDscram+nscid;
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
		//printf("%i: %f  %f I\n",i,creal(r[i]),cimag(r[i]));
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
		
		for (int n=0;n<128;n++){
			__real__ PSS[n] =(1-2*x_m[m[n]]);
			__imag__ PSS[n] =0.0;
		}
		
		
		return(0);
	}
		
		

int SSS_gen (int NID1,int NID2){
	

	
	int x_0[128];
	int x_1[128];
		

	int m0=3*round(NID1/112)+NID2;
	int m1=(NID1%112)+m0+1;

	
	x_0[0]=1;x_0[1]=0;x_0[2]=0;x_0[3]=0;x_0[4]=0;x_0[5]=0;x_0[6]=0;
	x_1[0]=1;x_1[1]=0;x_1[2]=0;x_1[3]=0;x_1[4]=0;x_1[5]=0;x_1[6]=0;
	
	for (int n=0;n<128-7;n++){
		x_0[n+7]=(x_0[n+4]+x_0[n])%2;
		x_1[n+7]=(x_1[n+1]+x_1[n])%2;
		
	}
	
	for (int n=0;n<128;n++){
		__real__ SSS[n] =(float)(1-2*(x_0[(n+m0)%127]))*(1-2*(x_1[(n+m1)%127]));
		__imag__ SSS[n]=0.0;
	}
	
	
	return(0);
}
	






