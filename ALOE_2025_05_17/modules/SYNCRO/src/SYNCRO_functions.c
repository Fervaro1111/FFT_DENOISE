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
#include <fftw3.h>
#include <math.h>

#include "SYNCRO_functions.h"

Syncro_t Syncinfo;
SYNC_struc Sync_MAC;

_Complex float PSS[PSSlength];
_Complex float PSS_time[4096];
_Complex float SSS[PSSlength];
_Complex float SSS_time[4096];
fftw_complex fftin[4096], fftout[4096];
fftw_plan fftplan;
fftw_plan iffplan;
float variance=0.0;


int NID2_found=0;
int NID1_found=0;
int checked_NIDs1=0;
_Complex float BUFFER[BUFFERMAXSIZE];
extern int TSLOT;
int Tslot=0;
int desalineacio=0;
int first_detection=-1;
int first_detection_PSS=-1;
int PSS_in_frame=0;
int PSS_alreadyfound=0;
//int ready2read=-1;
//int alreadyfound=0;
int distance;
//int FFTsize=512;
int Buff_in_pos=0;
int Buff_out_pos=0;


/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */


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
	


int create_spectrum(_Complex float *MQAMsymb, int datalength, 
							int FFTlength,  _Complex float *out_spectrum){
	int i, j;

	j=FFTlength-datalength/2;
	for(i=0; i<datalength; i++){
		*(out_spectrum+j)=*(MQAMsymb+i);
		j++;
		if(j==FFTlength) j=0;		
	}
	return(1);
}
/*
int detect_DMRS_in_subframe(_Complex float *correl, int numsamples){

	int i;
	int pMAX=-1;
	float maxval = -1000000.0, auxR;
	//Find MAX
	for(i=0; i<numsamples; i++){
		auxR=sqrt(pow(__real__ *(correl+i),2) + pow(__real__ *(correl+i),2));
		if(auxR>maxval){
			maxval = auxR;
			pMAX=i;
		}
	}
	//CHECK if DMRS: TBD may be.

	return(pMAX);
}

*/
void PSS_gen (int NID1,int NID2){
			
	int m[PSSlength];
	int x_m[PSSlength];
	
	for (int n=0;n<PSSlength;n++){
		m[n]=(n+43*NID2)%127; 
	}
	
	x_m[0]=0;x_m[1]=1;x_m[2]=1;x_m[3]=0;x_m[4]=1;x_m[5]=1;x_m[6]=1;
	
	for (int n=0;n<PSSlength-7;n++){
		x_m[n+7]=(x_m[n+4]+x_m[n])%2;
	}
	
	for (int n=0;n<127;n++){
		__real__ PSS[n] =(1-2*x_m[m[n]]);
		__imag__ PSS[n] =0.0;
	}
	PSS[128]=0.0+0.0*I;
	
}
	



int convolution(int signal_length, int filterlength,_Complex float* signal,_Complex float* correl, _Complex float* filter){

	
	int first=0;
	int i,j;
	_Complex float aux[4096*15]={0.0+0.0*I};

	for(int i=0;i<signal_length;i++){
		for (int j=0;j<filterlength;j++) {
			correl[i] += signal[j+i] * filter[j];
		}

	}

	return signal_length+filterlength;
}

void gen_SSS_time(int FFTsize,_Complex float* output,int NID1,int NID2){

	_Complex float SSS_spec[4096];
	SSS_gen (NID1,NID2);
	create_spectrum(SSS,SSSlength,FFTsize,&SSS_spec);
	
	fftplan = fftw_plan_dft_1d(FFTsize, fftin, fftout, FFTW_BACKWARD, FFTW_ESTIMATE);

	for(int i=0; i<1; i++){
		int j=i*FFTsize;
		for(int n=0; n<FFTsize; n++)*(fftin+n)=(fftw_complex) SSS_spec[j+n];
		//Execute FFT
		fftw_execute(fftplan);
		//Copy processed data to output buffer
			for(int n=0; n<FFTsize; n++)*(output+j+n)=((_Complex float) *(fftout+n))/FFTsize;
	}


}




void gen_PSS_time(int FFTsize,_Complex float* output,int NID2){

	_Complex float PSS_spec[4096];
	PSS_gen (Syncinfo.NID1,NID2);
	create_spectrum(PSS,PSSlength,FFTsize,&PSS_spec);
	
	fftplan = fftw_plan_dft_1d(FFTsize, fftin, fftout, FFTW_BACKWARD, FFTW_ESTIMATE);

	for(int i=0; i<1; i++){
		int j=i*FFTsize;
		for(int n=0; n<FFTsize; n++)*(fftin+n)=(fftw_complex) PSS_spec[j+n];
		//Execute FFT
		fftw_execute(fftplan);
		//Copy processed data to output buffer
			for(int n=0; n<FFTsize; n++)*(output+j+n)=((_Complex float) *(fftout+n))/FFTsize;
	}


}

#define FILTERLENGTH	4096
_Complex float aux[FILTERLENGTH];
//#define MAX_DATA 50*2048



int stream_conv_CPLX_DDC(_Complex float *ccinput,int datalength,_Complex float *filtercoeff,int filterlength,_Complex float *ccoutput,int Decimate){

	int i, j, k=0;
	static int first=0;
	static _Complex float aux[FILTERLENGTH];
	int INTER=1, DECIM=1;
	static int n=0, m=0;

	//INTER=oParam.Ninterpol;
	DECIM=Decimate;

//	printf("readCPLXfilecoeff(): real=%3.6f, imag=%3.6f\n", __real__ filtercoeff[10], __imag__ filtercoeff[10]);
//	printf("ccinput(): real=%3.6f, imag=%3.6f\n", __real__ ccinput[10], __imag__ ccinput[10]);
//	printf("INT=%d, DEC=%d, datalength=%d, n=%d, m=%d\n", INTER, DEC, datalength, n, m);

	if(first==0){
		for(j=0; j<filterlength; j++){
			aux[j]=0.0+0.0i;
		}
		first=1;
	}

	for (i=0;i<datalength*INTER;i++) {
		if(n==0){
			for (j=filterlength-2;j>=0;j--) {
				aux[j+1]=aux[j];
			}
			aux[0]=ccinput[i/INTER];
		}
		
		if(m==0){
			ccoutput[k]=0.0;
			for (j=0;j<filterlength/INTER;j++) {
				ccoutput[k]+=aux[j]*filtercoeff[j*INTER+n];
			}
			k++;
		}
		n++;							
		if(n==INTER)n=0;		
		m++;
		if(m==DECIM)m=0;
	}
	//printf("datalength*INT/DEC=%d\n", datalength*INTER/DEC);
	return (datalength*INTER/DECIM);
}





void get_signal(_Complex float* out){
	gen_SSS_time(512, out,1,2);

}


int do_sycro(_Complex float * input0,int rcv_samples,_Complex float * output,
				int* NID1, int* NID2, int FFTsize, _Complex float *graph, int *snd2GRAPH){
	_Complex float CorrResult[64*1024];
	_Complex float aux[64*1024];

	_Complex float SSS_time[4096];
	_Complex float varianze[2048*64];
	_Complex float Correl_procesed[2048*64];
	int pMAX1=-1;
	int pMAX2=-1;
	int snd_samples=0;
	
	Syncinfo.FFTsize=FFTsize;
	//printf("FFTsize in Syncro=%d\n",Syncinfo.FFTsize);

	if(NID2_found==0){
		//Iterate to find NID2
		DecideNID2(Syncinfo.FFTsize,input0,rcv_samples,&Syncinfo, graph, snd2GRAPH);

		//GENERATE PSS IN TIME DOMAIN
		gen_PSS_time(Syncinfo.FFTsize,&PSS_time,Syncinfo.NID2);
		//printf("IN PSS IF GEN TIME Syncinfo.NID2=%d\n",Syncinfo.NID2);
	}

	if(NID1_found==0 && NID2_found==1 ){
		DecideNID1(Syncinfo.FFTsize,input0,rcv_samples,Syncinfo.NID1,Syncinfo.NID2,&Syncinfo);

		if(NID1_found==1){
		*(NID2)=Syncinfo.NID2;
		*(NID1)=Syncinfo.NID1;
		}
	}



	if(NID2_found!=0){
		//FULL CORRELATION UNTIL FIRST DETECTION
		if(first_detection_PSS==-1){
			first_PSS_detection(input0,rcv_samples,&Syncinfo, graph, snd2GRAPH);
		}		
		else{
			if(PSS_alreadyfound==0){
				//CORRELATE AND PROCESATE WHERE PSS IT SHOULD BE
				int extra_size_corr=Syncinfo.FFTsize;//ES POT OPTIMITZAR FENT QUE NO ES CONVOLUCIONI EN STREAMING
				int correl_length=stream_conv_CPLX_DDC(input0+Syncinfo.pMAX1-extra_size_corr,
							Syncinfo.FFTsize+2*extra_size_corr,&PSS_time,Syncinfo.FFTsize,&CorrResult,1);
				process_correllation(&CorrResult, correl_length, &Correl_procesed);
				*snd2GRAPH=correl_length;
				memcpy(graph, Correl_procesed, sizeof(_Complex float)*correl_length);
				printf("TSLOT=%d, AAAAA_SYNCHRO, correl_length=%d\n", TSLOT, correl_length);
				//CHECK WHERE PSS IT SHOULD BE	
				int pMAX_deviation=detect_PSS(&Correl_procesed, correl_length, &varianze)-extra_size_corr;
				if(pMAX_deviation>=0){
					Syncinfo.pMAX1=Syncinfo.pMAX1+pMAX_deviation;	
					pMAX1=Syncinfo.pMAX1;
					PSS_in_frame=1;
					PSS_alreadyfound=1;
					//if(first_detection==-1)first_detection=1;
					//int distance_6=Syncinfo.FFTsize*6+ceil(Syncinfo.FFTsize*7.0/100.0)*5+ceil(Syncinfo.FFTsize*7.8/100.0);
					//Syncinfo.pMAX2=Syncinfo.pMAX1+distance_6;
					//pMAX2=Syncinfo.pMAX2;
				}

			//printf("pMAX located at %d, pMAX_deviation=%d\n",Syncinfo.pMAX1,pMAX_deviation);
			}

		}
		
	}



	///////Buffer//////////////
	//snd_samples=ManageBuffer(input0,output,pMAX1,pMAX2,rcv_samples,Syncinfo.FFTsize);

	snd_samples=ManageBuffer(input0,output,rcv_samples,&Syncinfo,Tslot);


	//Counters
	//if(Tslot==10)alreadyfound=0;

	//printf("Sendsamples=%d\n",snd_samples);
	

/*	printf("SYNCRO Tslot=%d\n",Tslot);
	printf("SYNCRO first_detection_PSS=%d\n",first_detection_PSS);
	printf("PSS_in_frame=%d\n",PSS_in_frame);
	printf("PSS_alreadyfound=%d\n",PSS_alreadyfound);
	printf("Desalineacio=%d\n",desalineacio);
	printf("distance=%d\n",distance);
	printf("Syncinfo.NID2=%d\n",Syncinfo.NID2);
	printf("Syncinfo.NID1=%d\n",Syncinfo.NID1);
	printf("Syncinfo.pMAX1=%d\n",Syncinfo.pMAX1);

*/
	if(first_detection_PSS==1){// && distance==Syncinfo.distance_6){
		Tslot=0;
		first_detection_PSS++;
	}
	PSS_in_frame=0;
	
	
	//printf("SYNCRO Tslot=%d\n",Tslot);
	Tslot++;
	if(Tslot==20)Tslot=0;
	if(Tslot==10)PSS_alreadyfound=0;

	return snd_samples;



}

int ManageBuffer(_Complex float* input,_Complex float* output,int rcv_samples,Syncro_t* Syncinfo,int Tslot){

	//CALCULAR DESALINEACIÓ
	if(PSS_in_frame==1){
		int PSS_theoreticl_pos=Syncinfo->FFTsize*3+ceil(Syncinfo->FFTsize*7.0/100.0)*2+ceil(Syncinfo->FFTsize*7.8/100.0);
		desalineacio=Syncinfo->pMAX1-PSS_theoreticl_pos+1;
		printf("\033[1;30m \tDesalineacio=%d, PSS_theoretical_pos=%d, pMAX1=%d, TSLOT=%d \033[0m\n",desalineacio,PSS_theoreticl_pos,Syncinfo->pMAX1, TSLOT);
	}


	//Write samples in BUFFER
	
	if(PSS_in_frame==0){//NOT PSS FOUND
		Write_in_Buff(rcv_samples, input,Syncinfo);
	}
	if(PSS_in_frame==1){//PSS found in frame
		Write_and_Align(rcv_samples,input,Syncinfo);
	}
	

	//printf("WRITE COMPLETE\n");


	//Read samples from Buffer
	int snd_samples=Read_from_Buff(rcv_samples,output,Syncinfo);


	//int snd_samples=rcv_samples;
	return snd_samples;



}

void Write_and_Align(int rcv_samples, _Complex float* in_samples,Syncro_t* Syncinfo){

	if(first_detection_PSS==1){
		//Write_in_Buff(rcv_samples, in_samples);
		Write_in_buf_first_time(rcv_samples, in_samples,Syncinfo);
		//Align_first_detection(rcv_samples,in_samples);
	}else{
		Align_with_PSS(rcv_samples, in_samples);
	}


}

void Align_with_PSS(int rcv_samples, _Complex float* in_samples){
	if(desalineacio>=0){
		memcpy(&BUFFER[20*rcv_samples-desalineacio],in_samples,desalineacio*sizeof(_Complex float));
		memcpy(&BUFFER[0],in_samples+desalineacio,(rcv_samples-desalineacio)*sizeof(_Complex float));
		
	}
 
	if(desalineacio<0){
		//memcpy(&BUFFER[0],&BUFFER[20*rcv_samples],(-desalineacio)*sizeof(_Complex float));
		memcpy(&BUFFER[-desalineacio],in_samples,(rcv_samples)*sizeof(_Complex float));
		


//memcpy(&BUFFER[0],in_samples-desalineacio,(rcv_samples+desalineacio)*sizeof(_Complex float));
		
	}

}

void Align_first_detection(int rcv_samples, _Complex float* in_samples){
	
	if(desalineacio>=0){
		memcpy(&BUFFER[0],&BUFFER[Tslot*rcv_samples+desalineacio],(rcv_samples-desalineacio)*sizeof(_Complex float));
	}
	//if(desalineacio<0){
	//}




}

void Write_in_buf_first_time(int rcv_samples, _Complex float* in_samples,Syncro_t* Syncinfo){
	

	if(desalineacio>=0 && distance==Syncinfo->distance_6){	
		memcpy(&BUFFER[0],in_samples+desalineacio,(rcv_samples-desalineacio)*sizeof(_Complex float));
	}


	if(desalineacio<0 && distance==Syncinfo->distance_6){
		memcpy(&BUFFER[0],&BUFFER[Tslot*rcv_samples+desalineacio],(-desalineacio)*sizeof(_Complex float));
		//memcpy(&BUFFER[(Tslot-1)*rcv_samples-desalineacio],in_samples,(rcv_samples)*sizeof(_Complex float));
		memcpy(&BUFFER[-desalineacio],in_samples,(rcv_samples)*sizeof(_Complex float));

	}

	if(desalineacio>=0 && distance==Syncinfo->distance_8){	

		memcpy(&BUFFER[0],&BUFFER[(Tslot-1)*rcv_samples+desalineacio],(rcv_samples-desalineacio)*sizeof(_Complex float));
		memcpy(&BUFFER[rcv_samples-desalineacio],in_samples,(rcv_samples)*sizeof(_Complex float));


	}

	
}


void Write_in_Buff(int rcv_samples, _Complex float* in_samples,Syncro_t* Syncinfo){

	if(distance==0){
		printf("IN WRITE NOTHIING FOUND YET!!!\n");
		memcpy(&BUFFER[Tslot*rcv_samples],in_samples,rcv_samples*sizeof(_Complex float));
	}


	if(distance==Syncinfo->distance_6 && desalineacio>=0){	
		memcpy(&BUFFER[Tslot*rcv_samples-desalineacio],in_samples,rcv_samples*sizeof(_Complex float));
		//printf("Buff_in_pos=%d\n",Tslot*rcv_samples+desalineacio);
	}
	
	if(distance==Syncinfo->distance_6 && desalineacio<0){	
		if(Tslot!=19){
			memcpy(&BUFFER[(Tslot)*rcv_samples-desalineacio],in_samples,rcv_samples*sizeof(_Complex float));
			//printf("Buff_in_pos=%d at Tslot=%d with desalineacio=%d\n",(Tslot)*rcv_samples-desalineacio,Tslot,desalineacio);
		
		}else{
			memcpy(&BUFFER[(Tslot)*rcv_samples-desalineacio],in_samples,(rcv_samples+desalineacio)*sizeof(_Complex float));
			memcpy(&BUFFER[0],in_samples+rcv_samples+desalineacio,(-desalineacio)*sizeof(_Complex float));
		}

	}
	if(distance==Syncinfo->distance_8 && desalineacio>=0){	
		memcpy(&BUFFER[(Tslot+1)*rcv_samples-desalineacio],in_samples,rcv_samples*sizeof(_Complex float));
		//printf("Buff_in_pos=%d\n",(Tslot+1)*rcv_samples-desalineacio);
	}

	
}


/*
void Write_in_Buff(int rcv_samples, _Complex float* in_samples){

		
	if(Buff_in_pos+rcv_samples<=BUFFERMAXSIZE){

		memcpy(&BUFFER[Buff_in_pos],in_samples,rcv_samples*sizeof(_Complex float));
		
	}else{
		Buff_in_pos=0;
		memcpy(&BUFFER[Buff_in_pos],in_samples,rcv_samples*sizeof(_Complex float));
	}

	Buff_in_pos+=rcv_samples;
	printf("Buff_in_pos=%d\n",Buff_in_pos);
}

*/
int Read_from_Buff(int rcv_samples, _Complex float* out_samples,Syncro_t* Syncinfo){
	
	int snd_samples=0;	

	if(first_detection_PSS==1 && desalineacio==0){

		Buff_out_pos=Tslot*rcv_samples;
		memcpy(out_samples,&BUFFER[Buff_out_pos],rcv_samples*sizeof(_Complex float));
		snd_samples=rcv_samples;
	
	}

	else if(first_detection_PSS>1 && desalineacio==0){

		Buff_out_pos=Tslot*rcv_samples;
		//printf("Buff_out_pos=%d\n",Buff_out_pos);
		memcpy(out_samples,&BUFFER[Buff_out_pos],rcv_samples*sizeof(_Complex float));
	snd_samples=rcv_samples;

	}

	else if(first_detection_PSS==1 && desalineacio>0){

		if(distance==Syncinfo->distance_6) snd_samples=0;	
		if(distance==Syncinfo->distance_8){
			memcpy(out_samples,&BUFFER[0],rcv_samples*sizeof(_Complex float));
		
			snd_samples=rcv_samples;	
		}
	
	}

	else if(first_detection_PSS>1 && desalineacio>0){
		if(distance==Syncinfo->distance_6){		
			if(Tslot!=0){
				Buff_out_pos=(Tslot-1)*rcv_samples;
			}else{
				Buff_out_pos=19*rcv_samples;
			}
			//printf("Buff_out_pos=%d\n",Buff_out_pos);
			memcpy(out_samples,&BUFFER[Buff_out_pos],rcv_samples*sizeof(_Complex float));
			snd_samples=rcv_samples;
		
		}else{

			Buff_out_pos=(Tslot)*rcv_samples;
			//printf("Buff_out_pos=%d\n",Buff_out_pos);
			memcpy(out_samples,&BUFFER[Buff_out_pos],rcv_samples*sizeof(_Complex float));
			snd_samples=rcv_samples;

		}

	}

	else if(first_detection_PSS==1 && desalineacio<0){

		if(distance==Syncinfo->distance_6){

			memcpy(out_samples,&BUFFER[0],rcv_samples*sizeof(_Complex float));
			snd_samples=rcv_samples;	
		}
	}

	else if(first_detection_PSS>1 && desalineacio<0){
		//if(distance==Syncinfo->distance_6){		
			//if(Tslot!=0){
				Buff_out_pos=(Tslot)*rcv_samples;
			//}else{
			//	Buff_out_pos=19*rcv_samples;
			//}
			//printf("Buff_out_pos=%d\n",Buff_out_pos);
			memcpy(out_samples,&BUFFER[Buff_out_pos],rcv_samples*sizeof(_Complex float));
			snd_samples=rcv_samples;
		//}

	}

	return snd_samples;
	//Buff_out_pos+=rcv_samples;
	//if(Buff_out_pos>=BUFFERMAXSIZE)Buff_out_pos=0;

}

/*
void Align_Buff(Syncro_t* Syncinfo,int rcv_samples){

	int PSS_theoreticl_pos=Syncinfo->FFTsize*3+ceil(Syncinfo->FFTsize*7.0/100.0)*2+ceil(Syncinfo->FFTsize*7.8/100.0);
	desalineacio=Syncinfo->pMAX1-PSS_theoreticl_pos+1;
	printf("desalineacio=%d, PSS_theoreticl_pos=%d, pMAX1=%d\n",desalineacio,PSS_theoreticl_pos,Syncinfo->pMAX1);
	memcpy(&BUFFER[0],&BUFFER[Buff_in_pos-rcv_samples+desalineacio],rcv_samples*sizeof(_Complex float));
	Buff_in_pos=rcv_samples;

}*/



int first_PSS_detection(_Complex float* input,int rcv_samples,Syncro_t* Syncinfo, _Complex float *graph, int *snd2GRAPH){

	_Complex float CorrResult[2048*32];
	_Complex float Correl_procesed[2048*32];
	_Complex float varianze[2048*32];

	//CORRELATE AND PROCESATE
	int correl_length=stream_conv_CPLX_DDC(input,rcv_samples,&PSS_time, Syncinfo->FFTsize,&CorrResult,1);
	process_correllation(&CorrResult, correl_length, &Correl_procesed);

	*snd2GRAPH=correl_length;
	memcpy(graph, Correl_procesed, sizeof(_Complex float)*correl_length);
	printf("TSLOT=%d, BBBBB_SYNCHRO, correl_length=%d\n", TSLOT, correl_length);

	//CHECK FOR PSSs
	Check_PSS(&Correl_procesed,correl_length,&varianze,Syncinfo);
	if(Syncinfo->pMAX1>0 && Syncinfo->pMAX2>0){
		first_detection_PSS=1;
		PSS_in_frame=1;
		PSS_alreadyfound=1;
		//printf("First Double PSS DETECTED\n");

		//CALC DISTANCES BETWEEN PSSs.	
		int distance_6=Syncinfo->FFTsize*6+ceil(Syncinfo->FFTsize*7.0/100.0)*5+ceil(Syncinfo->FFTsize*7.8/100.0);
		int distance_8=Syncinfo->FFTsize*8+ceil(Syncinfo->FFTsize*7.0/100.0)*7+ceil(Syncinfo->FFTsize*7.8/100.0);

		Syncinfo->distance_6=distance_6;
		Syncinfo->distance_8=distance_8;	
	

		distance=Syncinfo->pMAX2-Syncinfo->pMAX1;
		//printf("first time calc-->distance=%d\n",distance);
		if(distance==distance_8){
			int tmp;
			tmp= Syncinfo->pMAX1;
			Syncinfo->pMAX1=Syncinfo->pMAX2;
			Syncinfo->pMAX2=tmp;
		}
	}

}




void DecideNID1(int FFTsize,_Complex float* input0,int rcv_samples,int NID1,int NID2,Syncro_t* Syncinfo){
	
	_Complex float CorrResult[64*1024];
	_Complex float aux[64*1024];
	_Complex float SSS_time[2048];
	_Complex varianze[2048*64];
	int pMAX1=-1;
	int pMAX2=-1;


	for(int i=0;i<355;i++){
		gen_SSS_time(Syncinfo->FFTsize, &SSS_time,i,NID2);
		int correl_length=stream_conv_CPLX_DDC(input0+Syncinfo->pMAX1,2*FFTsize,&SSS_time, FFTsize,&CorrResult,1);
		process_correllation(&CorrResult, correl_length, &aux);
		pMAX1=detect_SSS(&CorrResult,correl_length,&varianze);
		if(pMAX1>0){
			Syncinfo->NID1=1;
			NID1_found=1;	
			//printf("NID1 found =%d\n",Syncinfo->NID1);
			break;
		}	
		
		
	}



}

void convolute(int length,_Complex float* filter,_Complex float* in){
	
	_Complex float out;
	for(int i=0;i<length;i++){
		out+=filter[i]*in[i];

	}
		//printf("SSS efficient Correl real=%f, imag=%f\n",creal(out),cimag(out));

}


void DecideNID2(int FFTsize,_Complex float* input0,int rcv_samples,
				Syncro_t* Syncinfo, _Complex float *graph, int *snd2GRAPH){
	
	_Complex float CorrResult[64*1024];
	_Complex float aux[64*1024];
	_Complex float PSS_time[2048];
	_Complex varianze[2048*64];
	int pMAX1=-1;
	int pMAX2=-1;

/*	for(int i=0;i<3;i++){
		gen_PSS_time(FFTsize,&PSS_time,i);
		int correl_length=stream_conv_CPLX_DDC(input0,rcv_samples,&PSS_time, FFTsize,&CorrResult,1);
		process_correllation(&CorrResult, correl_length, &aux);
		//Check_PSS(&aux,correl_length,&varianze,&pMAX1,&pMAX2);
		pMAX1=detect_PSS(&CorrResult,correl_length,&varianze);
		if(pMAX1>0){
			Syncinfo->NID2=i;
			NID2_found=1;	
			printf("NID2 found =%d\n",Syncinfo->NID2);
			printf("Pmax located at=%d\n",pMAX1);
			break;		
		}	
*/

	for(int i=0;i<3;i++){
		gen_PSS_time(FFTsize,&PSS_time,i);
		int correl_length=stream_conv_CPLX_DDC(input0,rcv_samples,&PSS_time, FFTsize,&CorrResult,1);
		process_correllation(&CorrResult, correl_length, &aux);
		Check_PSS(&CorrResult,correl_length,&varianze,Syncinfo);
		*snd2GRAPH=correl_length;
		memcpy(graph, aux, sizeof(_Complex float)*correl_length);
		printf("TSLOT=%d, CCCCC_SYNCHRO, correl_length=%d\n", TSLOT, correl_length);
		//pMAX1=detect_PSS(&CorrResult,correl_length,&varianze);
		if(Syncinfo->pMAX1>0 && Syncinfo->pMAX2>0){
			Syncinfo->NID2=i;
			NID2_found=1;	
			//printf("NID2 found =%d\n",Syncinfo->NID2);
			//printf("Pmax located at=%d\n",Syncinfo->pMAX1);
			break;		
		}		
	}
}



void Check_PSS(_Complex float* CorrResult,int correl_length,float* varianze,Syncro_t* Syncinfo){
	int pMAX1;
	int pMAX2;

	int pMAX=detect_PSS(CorrResult,correl_length,varianze);
	//varianze=0.0;
	pMAX1=pMAX;
	if(pMAX>0){
		CorrResult[pMAX+1]=0.0+0.0*I;
		CorrResult[pMAX+2]=0.0+0.0*I;
		CorrResult[pMAX+3]=0.0+0.0*I;
		CorrResult[pMAX]=0.0+0.0*I;
		CorrResult[pMAX-1]=0.0+0.0*I;
		CorrResult[pMAX-2]=0.0+0.0*I;
		CorrResult[pMAX-3]=0.0+0.0*I;
		pMAX2=detect_PSS(CorrResult,correl_length,varianze);
		//pMAX2=pMAX;

	}
	if(pMAX1>pMAX2){
		int tmp;
		tmp= pMAX1;
		pMAX1=pMAX2;
		pMAX2=tmp;
	}

	Syncinfo->pMAX1=pMAX1;
	Syncinfo->pMAX2=pMAX2;

}



#define PSS_THRESHOLD		6.0
#define PSS_MAXTHRESHOLD	600.0
#define PSS_RATIO2THRESHOLD	2.0

float EXTthreshold=PSS_THRESHOLD;//PSS_MAXTHRESHOLD; //PSS_THRESHOLD;
float ExtAverRatio3=3.0;

#define PSS_RATIO3THRESHOLD	600.0
#define WINDOWSZSHORT	128
#define WINDOWSZWIDE	2048
#define WINDOWSZWIDE2	1024

int detect_PSS(_Complex float *inout, int length, float *varianze){
	int i, k, pMAX=-1, pMAX2=0, block;
	float maxval, smaxval=1000000.0, auxR, auxI, ratio, ratio2, ratio3, ratio4;
	float vmedio=0.0;
	static float threshold=PSS_THRESHOLD;
	static float thresholdratio3=PSS_RATIO3THRESHOLD;
	float varianceSHORT=0.0, varianceWIDE=0.0, variance2=0.0;
	int Llimit=0, Hlimit;
	int P=0;
	static float ratio3MAXave=PSS_RATIO3THRESHOLD, ratio3ave=1.0;
	static int numave=1, numMAXave=1;

	float MAX5=0, ratio5;
	float varianceWIDE5=0.0;
	float auxR5, auxI5;

	static float Ratio3MEM[5]={1000000.0, 1000000.0, 1000000.0, 1000000.0, 1000000.0};
	static int CountRatio3=0;
	float AverRatio3;
	float percentage=0.0;
	

	//*variance=0.0;
	maxval = -1000000.0;
	for(i=0; i<length; i++){
		auxR=(float)(fabs(__real__ *(inout+i)) + fabs(__imag__ *(inout+i)));
			if(maxval < auxR){
					smaxval=fabs(maxval);
				maxval = fabs(auxR);
				pMAX=i;
			}
	}
	//printf("PMAX=%d\n", pMAX);

	// CHECK IF PSS
	k=0;
	auxR5=(float)(fabs(__real__ *(inout+pMAX)) + fabs(__imag__ *(inout+pMAX)));
	auxI5=(float)(fabs(__real__ *(inout+pMAX)) - fabs(__imag__ *(inout+pMAX)));
	MAX5=fabs(auxR5*auxI5);
	if(pMAX - WINDOWSZWIDE2 > 0)Llimit=pMAX-WINDOWSZWIDE2;
	else Llimit=0;
	if(pMAX+WINDOWSZWIDE2 < length)Hlimit=pMAX+WINDOWSZWIDE2;
	else Hlimit=length;
	for(i=Llimit; i<Hlimit; i++){
		auxR5=(float)(fabs(__real__ *(inout+i)) + fabs(__imag__ *(inout+i)));
		auxI5=(float)(fabs(__real__ *(inout+i)) - fabs(__imag__ *(inout+i)));
		auxR=fabs(auxR5*auxI5);
		if(abs(pMAX - i) > 5){
			varianceWIDE5 += auxR;
			k++;
		}
	}
	varianceWIDE5 = varianceWIDE5/(float)k;
	if(isnan(varianceWIDE5))varianceWIDE5=1.0;
	if(varianceWIDE5 < 0.001){   //0.01
		varianceWIDE5 = 0.001;
	}
	ratio5=fabs(1.0 - (varianceWIDE5/MAX5))*1000.0; 


	// CALCULATE SIGNAL VARIANCE
	k=0;
	if(pMAX - WINDOWSZWIDE > 0)Llimit=pMAX-WINDOWSZWIDE;
	if(pMAX+WINDOWSZWIDE < length)Hlimit=pMAX+WINDOWSZWIDE;
	for(i=Llimit; i<Hlimit; i++){
		auxR=(float)(fabs(__real__ *(inout+i)) + fabs(__imag__ *(inout+i)));
		if(abs(pMAX - i) > 5){
			varianceWIDE += auxR/maxval;
			k++;
		}
	}
	varianceWIDE = varianceWIDE/(float)k;
	if(isnan(varianceWIDE))varianceWIDE=1.0;
	if(varianceWIDE < 0.001){   //0.01
		varianceWIDE = 0.001;
	}	
	// CALCULATE SIGNAL VARIANCE AROUND THE pMAX
	k=0;
	if(pMAX - WINDOWSZSHORT > 0)Llimit=pMAX-WINDOWSZSHORT;
	if(pMAX+WINDOWSZSHORT < length)Hlimit=pMAX+WINDOWSZSHORT;
	for(i=Llimit; i<Hlimit; i++){
		auxR=(float)(fabs(__real__ *(inout+i)) + fabs(__imag__ *(inout+i)));
		if(abs(pMAX - i) > 5){
			varianceSHORT += auxR/maxval;
			k++;
		}
	}
	varianceSHORT = varianceSHORT/(float)k;
	if(isnan(varianceSHORT))varianceSHORT=1.0;
	if(varianceSHORT < 0.001){   //0.01
		varianceSHORT = 0.001;
	}	
	*varianze=varianceSHORT;
	ratio=maxval/(varianceSHORT);
	ratio4=maxval/(varianceWIDE);
	ratio2=maxval/smaxval;
	ratio3=ratio*ratio2;
	


//	printf("PSS probability=%f\n",pss);

	// RECOMPUTE THRESHOLD
	numMAXave++;
	ratio3MAXave += ratio3;	
	

	if(ratio3 > thresholdratio3){
		//printf("\033[1;35m: detect_PSS(): \033[0m\n");
		numMAXave++;
		ratio3MAXave += ratio3;
	}
	if(numave+numMAXave > 10)thresholdratio3=(0.4*(ratio3ave/(float)numave)+0.6*(ratio3MAXave/(float)numMAXave))/2.0;
	else thresholdratio3=varianceWIDE*10.0;

	// DECIDIR UTILITZANT PROBABILITAT
	float pssmin = 0.3;
	float pss = 1.0 - (1.0/ratio4);
	if(pss > pssmin){
		printf("\033[1;31m \tPSS probability=%.3f, PSS DETECTED at sample=%d \033[0m\n",pss, pMAX-1);
		printf("\033[1;31m \tratio=%-10.3f, ratio2=%-10.3f, ratio3=%-10.3f, ratio4=%-10.3f, thresholdratio3=%-10.3f, TSLOT=%d \033[0m\n", 
					ratio, ratio2, ratio3, ratio4, thresholdratio3, TSLOT);
		
	}else{
/*		printf("\033[1;34m \tPSS probability=%.3f, PSS NOT DETECTED at sample=%d \033[0m\n",pss, pMAX-1);
		printf("\033[1;34m \tratio=%-10.3f, ratio2=%-10.3f, ratio3=%-10.3f, ratio4=%-10.3f, thresholdratio3=%-10.3f, TSLOT=%d \033[0m\n", 
					ratio, ratio2, ratio3, ratio4, thresholdratio3, TSLOT);
*/
		pMAX=-1;
	}




	return(pMAX-1);
}

int detect_SSS(_Complex float *inout, int length, float *varianze){
	int i, k, pMAX=-1, pMAX2=0, block;
	float maxval, smaxval=1000000.0, auxR, auxI, ratio, ratio2, ratio3, ratio4;
	float vmedio=0.0;
	static float threshold=PSS_THRESHOLD;
	static float thresholdratio4=PSS_RATIO3THRESHOLD;
	float varianceSHORT=0.0, varianceWIDE=0.0, variance2=0.0;
	int Llimit=0, Hlimit=length;
	int P=0;
	static float ratio3MAXave=PSS_RATIO3THRESHOLD, ratio3ave=1.0;
	static int numave=1, numMAXave=1;

	float MAX5=0, ratio5;
	float varianceWIDE5=0.0;
	float auxR5, auxI5;

	static float Ratio3MEM[5]={1000000.0, 1000000.0, 1000000.0, 1000000.0, 1000000.0};
	static int CountRatio3=0;
	float AverRatio3;
	float percentage=0.0;
	

	//*variance=0.0;
	maxval = -1000000.0;
	for(i=0; i<length; i++){
		auxR=(float)(fabs(__real__ *(inout+i)) + fabs(__imag__ *(inout+i)));
			if(maxval < auxR){
					smaxval=fabs(maxval);
				maxval = fabs(auxR);
				pMAX=i;
			}
	}


	// CHECK IF PSS
	k=0;
	auxR5=(float)(fabs(__real__ *(inout+pMAX)) + fabs(__imag__ *(inout+pMAX)));
	auxI5=(float)(fabs(__real__ *(inout+pMAX)) - fabs(__imag__ *(inout+pMAX)));
	MAX5=fabs(auxR5*auxI5);
	if(pMAX - WINDOWSZWIDE2 > 0)Llimit=pMAX-WINDOWSZWIDE2;
	else Llimit=0;
	if(pMAX+WINDOWSZWIDE2 < length)Hlimit=pMAX+WINDOWSZWIDE2;
	else Hlimit=length;
	for(i=Llimit; i<Hlimit; i++){
		auxR5=(float)(fabs(__real__ *(inout+i)) + fabs(__imag__ *(inout+i)));
		auxI5=(float)(fabs(__real__ *(inout+i)) - fabs(__imag__ *(inout+i)));
		auxR=fabs(auxR5*auxI5);
		if(abs(pMAX - i) > 5){
			varianceWIDE5 += auxR;
			k++;
		}
	}
	varianceWIDE5 = varianceWIDE5/(float)k;
	if(isnan(varianceWIDE5))varianceWIDE5=1.0;
	if(varianceWIDE5 < 0.001){   //0.01
		varianceWIDE5 = 0.001;
	}
	ratio5=fabs(1.0 - (varianceWIDE5/MAX5))*1000.0; 


	// CALCULATE SIGNAL VARIANCE
	k=0;
	if(pMAX - WINDOWSZWIDE > 0)Llimit=pMAX-WINDOWSZWIDE;
	if(pMAX+WINDOWSZWIDE < length)Hlimit=pMAX+WINDOWSZWIDE;
	for(i=Llimit; i<Hlimit; i++){
		auxR=(float)(fabs(__real__ *(inout+i)) + fabs(__imag__ *(inout+i)));
		if(abs(pMAX - i) > 5){
			varianceWIDE += auxR/maxval;
			k++;
		}
	}
	varianceWIDE = varianceWIDE/(float)k;
	if(isnan(varianceWIDE))varianceWIDE=1.0;
	if(varianceWIDE < 0.001){   //0.01
		varianceWIDE = 0.001;
	}	
	// CALCULATE SIGNAL VARIANCE AROUND THE pMAX
	k=0;
	if(pMAX - WINDOWSZSHORT > 0)Llimit=pMAX-WINDOWSZSHORT;
	if(pMAX+WINDOWSZSHORT < length)Hlimit=pMAX+WINDOWSZSHORT;
	for(i=Llimit; i<Hlimit; i++){
		auxR=(float)(fabs(__real__ *(inout+i)) + fabs(__imag__ *(inout+i)));
		if(abs(pMAX - i) > 5){
			varianceSHORT += auxR/maxval;
			k++;
		}
	}
	varianceSHORT = varianceSHORT/(float)k;
	if(isnan(varianceSHORT))varianceSHORT=1.0;
	if(varianceSHORT < 0.001){   //0.01
		varianceSHORT = 0.001;
	}	
	*varianze=varianceSHORT;
	ratio=maxval/(varianceSHORT);
	ratio4=maxval/(varianceWIDE);
	ratio2=maxval/smaxval;
	ratio3=ratio*ratio2;
	

	float pssmin = 0.3;
	float pss = 1.0 - (1.0/ratio4);
	//printf("PSS probability=%f\n",pss);
	if(pss<pssmin){
		pMAX=-1;
			
 	}
	if(pMAX>0){
		//printf("PSS DETECTED at sample=%d\n",pMAX-1);
		}	

	numMAXave++;
	ratio3MAXave += ratio3;	
	
	
	if(ratio3 > thresholdratio4){
		//printf("\033[1;35m: detect_PSS(): \033[0m\n");
		numMAXave++;
		ratio3MAXave += ratio3;
	}
	
	if(numave+numMAXave > 10)thresholdratio4=(0.4*(ratio3ave/(float)numave)+0.6*(ratio3MAXave/(float)numMAXave))/2.0;
	else thresholdratio4=varianceWIDE*10.0;

	if(pMAX>=0){
		printf("\033[1;32m \tSSS probability=%.3f, SSS DETECTED at sample=%d \033[0m\n",pss, pMAX-1);
		printf("\033[1;32m \tratio=%-10.3f, ratio2=%-10.3f, ratio3=%-10.3f, ratio4=%-10.3f, thresholdratio4=%-10.3f \033[0m\n", 
					ratio, ratio2, ratio3, ratio4, pss);
	}else{
		printf("\033[1;33m \tSSS probability=%.3f, SSS DETECTED at sample=%d \033[0m\n",pss, pMAX-1);
		printf("\033[1;33m \tratio=%-10.3f, ratio2=%-10.3f, ratio3=%-10.3f, ratio4=%-10.3f, thresholdratio4=%-10.3f \033[0m\n", 
					ratio, ratio2, ratio3, ratio4, pss);
	}

	return(pMAX-1);
}




void process_correllation(_Complex float *inCorrel, int datalength, _Complex float *outCorrel){

	int i;

	for(i=0; i<datalength; i++){
		__real__ *(outCorrel+i)=(float)(fabs(__real__ *(inCorrel+i)) + fabs(__imag__ *(inCorrel+i)));
		__imag__ *(outCorrel+i)=(float)(fabs(__real__ *(inCorrel+i)) - fabs(__imag__ *(inCorrel+i)));
	}

}










