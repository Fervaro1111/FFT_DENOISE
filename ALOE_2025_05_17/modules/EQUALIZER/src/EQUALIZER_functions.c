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

#include "EQUALIZER_functions.h"

DMRS_t DMRS_params;
info_t Equalizer_info;
_Complex float DMRS[DMRS_max_length];

#define average_length 20
_Complex float DMRS_average[DMRS_max_length]={0.0+0.0*I};
_Complex float channel_average[2048]={0.0+0.0*I};

int average_counter=1;
_Complex float Coeff[2048];
_Complex float Coeff_MMSE[2048];
float snr[DMRS_max_length]={0.0};

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
	

void Do_equalize(_Complex float* in,int Ts,_Complex float* out){

	DMRS_params.Additional_DMRS_position=0;
	Equalizer_info.nPRB=24;
	Equalizer_info.FFTsize=512;

	_Complex float rcv_DMRS[2048];
	_Complex float rcv_spectrum[2048];
	_Complex float DMRS_seq[2048];
	_Complex float Coeff_out[4096];
	_Complex float channel[2048];
	_Complex float equalize_spectrum[4096];
	
	
	//Calc parameters
	calc_Slot_symbol(Ts,&DMRS_params);
	
	//Dismount the DMRS from the recived signal
	getDATAfromLTE_DOWNLINKspectrum(in+Equalizer_info.FFTsize*DMRS_params.OFDMsymbol, Equalizer_info.FFTsize, Equalizer_info.nPRB*numCarriers_PRB, &rcv_spectrum);
	Extract_DMRS_from_symbol(&Equalizer_info,&rcv_spectrum,&rcv_DMRS);
	
	// GENerate DMRS
	GEN_DMRS(DMRS_params.Slotnumber,DMRS_params.OFDMsymbol,&DMRS_seq);
	
/*	// Estimate correction Coefficients for ZF
	Calc_Coeficients(&rcv_DMRS,&DMRS_seq,&Coeff,Equalizer_info.nPRB*numCarriers_PRB/2,&DMRS_params, &Equalizer_info);
	// Estimate correction Coefficients for ZF
*/

	//Estimate channel coefficents
	Get_Channel(&rcv_DMRS,Equalizer_info.nPRB*numCarriers_PRB/2,& DMRS_params,&Equalizer_info,&DMRS_seq,&channel);
	
	Average_channel(&channel,&channel_average,average_counter, Equalizer_info.nPRB*numCarriers_PRB/2);
	

	//Estimate SNR
	float snr_aver=Calc_SNRs(&DMRS_params,&Equalizer_info,&DMRS_average,&rcv_DMRS,&snr,average_counter);
	printf("snr_aver=%f\n",10.0*log10f(snr_aver));
	

	//Compute MMSE Coefficents
	Compute_MMSE_Coefficents(snr_aver,&channel_average,&Coeff_MMSE);

	// Generate all the Coeffs
	Interpolate(&Coeff_MMSE,&Coeff_out,Equalizer_info.nPRB*numCarriers_PRB);


	//Mount spectrum again
	create_LTEspectrumNORS(&Coeff_out,Equalizer_info.FFTsize, Equalizer_info.nPRB*numCarriers_PRB,&equalize_spectrum);

	//Clean channel attenuation
	Clean_Channel(in,&equalize_spectrum,&Equalizer_info,out);

	//Counters
	if(average_counter<average_length)average_counter++;	


	//prints
/*	for(int i=0;i<Equalizer_info.nPRB*12;i++){
		
		//printf("rcv_MRS[%d]=%f\n",i,creal(rcv_DMRS[i]));
		//printf("REAL_MRS[%d]=%f\n",i,creal(DMRS_seq[i]));
		//printf("REAL_coeff[%d]=%f  IMAG_coeff=%f\n",i,creal(Coeff[i]),cimag(Coeff[i]));
		//printf("REAL_coeff channel[%d]=%f  IMAG_coeff chNNEL=%f\n",i,creal(channel[i]),cimag(channel[i]));
		//printf("REAL_coeff channel_AVERage[%d]=%f  IMAG_coeff chNNEL_AVERage=%f\n",i,creal(channel_average[i]),cimag(channel_average[i]));
		printf("SNR estimated=%f dB at i=%d\n",10.0*log10f(snr[i]),i);
		//printf("REAL_oeff interpolate[%d]=%f  IMAG_coeff interpolate=%f\n",i,creal(Coeff_out[i]),cimag(Coeff_out[i]));	
		//printf("REAL_coeff MMSE[%d]=%f  IMAG_coeff MMSE=%f\n",i,creal(Coeff_MMSE[i]),cimag(Coeff_MMSE[i]));
		//printf("REAL_coeff Coeff_out[%d]=%f  IMAG_coeff Coeff_out=%f\n",i,creal(Coeff_out[i]),cimag(Coeff_out[i]));
	
	}

*/
}

void Clean_Channel(_Complex float* in_info,_Complex float* coeff,info_t* Equalizer_info,_Complex float* out_clean){
	for(int i=0;i<14;i++){
		for(int j=0;j<Equalizer_info->FFTsize;j++){
			*(out_clean+j+Equalizer_info->FFTsize*i)=*(in_info+j+Equalizer_info->FFTsize*i) * *(coeff+j);

		}

	}


}


void Average_channel(_Complex float* channel,_Complex float* channel_average,int average_counter,int length){
	for(int i=0;i<length;i++){
		*(channel_average+i)=*(channel_average+i)+(*(channel+i)- *(channel_average+i))/average_counter;
	}
}


void Get_Channel(_Complex float* DMRS_rcv,int length,DMRS_t* DMRS_params,info_t* Equalizer_info,_Complex float* DMRS_seq,_Complex float* Coeff){
//Computes channel coefficents

	for(int i=0;i<length;i++){
		if(DMRS_params->SSB_counter!=0 && DMRS_params->SSB_counter!=1){
			*(Coeff+i) =*(DMRS_rcv+i)/ *(DMRS_seq+i);
			
		}else{//used to avoid computing coefficient when the symbol has PSS in the central carriers
			if( i<(Equalizer_info->nPRB*6-120)/2){
				*(Coeff+i) = *(DMRS_rcv+i)/ *(DMRS_seq+i);	
			}else if (i>(((Equalizer_info->nPRB*6-120)/2)+120)){
			*(Coeff+i) = *(DMRS_rcv+i)/ *(DMRS_seq+i);

			}
		}

	//printf("REAL_coeff[%d]=%f  IMAG_coeff=%f\n",i,creal(Coeff[i]),cimag(Coeff[i]));
	}

	
	if(DMRS_params->SSB_counter==0 || DMRS_params->SSB_counter==1){
		for(int i=(Equalizer_info->nPRB*6-120)/2;i<(((Equalizer_info->nPRB*6-120)/2)+60);i++){
			__real__ *(Coeff+i)=__real__ *(Coeff+i-1);
			__imag__ *(Coeff+i)=__imag__ *(Coeff+i-1);
		}

	for(int i=(((Equalizer_info->nPRB*6-120)/2)+120);i>=(((Equalizer_info->nPRB*6-120)/2)+60);i--){
			__real__ *(Coeff+i)=__real__ *(Coeff+i+1);
			__imag__ *(Coeff+i)=__imag__ *(Coeff+i+1);
		}
	}

}



void Interpolate(_Complex float* Coeff_in,_Complex float* Coeff_out,int length){
	int i;
	for(i=0;i<length;){
		__real__ *(Coeff_out+i)=__real__ *(Coeff_in+i/2);
		__imag__ *(Coeff_out+i)=__imag__ *(Coeff_in+i/2);
		i=i+2;
	}
	for(i=1;i<length-1;){
		__real__ *(Coeff_out+i)=(__real__ *(Coeff_out+i-1)+__real__ *(Coeff_out+i+1))/2.0;
		__imag__ *(Coeff_out+i)=(__imag__ *(Coeff_out+i-1)+__imag__ *(Coeff_out+i+1))/2.0;
		i=i+2;
		
	}
		__real__ *(Coeff_out+i)=__real__ *(Coeff_out+i-1);
		__imag__ *(Coeff_out+i)=__imag__ *(Coeff_out+i-1);
	

}

void Compute_MMSE_Coefficents(float SNR,_Complex float* Coeff_in,_Complex float* Coeff_out){
	
	for(int i=0;i<Equalizer_info.nPRB*12/2;i++){
		*(Coeff_out+i)=conjf(*(Coeff_in+i))/(pow(cabsf(*(Coeff_in+i)),2)+(1.0/ (SNR)));

	}

}


float Calc_SNRs(DMRS_t* DMRS_params,info_t* Equalizer_info,_Complex float* DMRS_average,_Complex float* rcv_DMRS,float* SNR,int average_counter){
	int DMRS_processed=0;
	if(DMRS_params->SSB_counter!=0 && DMRS_params->SSB_counter!=1){
		for(int i=0;i<Equalizer_info->nPRB*numCarriers_PRB/2;i++){
			Average2(rcv_DMRS+i,DMRS_average+i,average_counter);
			SNR2(rcv_DMRS+i,DMRS_average+i,SNR+DMRS_processed);
			DMRS_processed++;
		}

	}else{
		for(int i=0;i<Equalizer_info->nPRB*numCarriers_PRB/2;i++){
			if( i<(Equalizer_info->nPRB*6-120)/2){
				Average2(rcv_DMRS+i,DMRS_average+i,average_counter);
				SNR2(rcv_DMRS+i,DMRS_average+i,SNR+DMRS_processed);
				DMRS_processed++;
			}else if (i>(((Equalizer_info->nPRB*6-120)/2)+120)){
				Average2(rcv_DMRS+i,DMRS_average+i,average_counter);
				SNR2(rcv_DMRS+i,DMRS_average+i,SNR+DMRS_processed);
				DMRS_processed++;
			}
		}

	}

	//float snr_aver=SNR_average(SNR,Equalizer_info->nPRB*numCarriers_PRB/2);	
	float snr_aver=SNR_average(SNR,DMRS_processed);
	return snr_aver;

}

float SNR_average(float* snr_in, int length){
	float SNR;
	for(int i=0;i<length;i++){
		SNR += *(snr_in+i);
	}
	printf("lengthd=%d\n",length);
	SNR=SNR/(float)(length);
	return SNR;	


}

void Average2(_Complex float* in,_Complex float* average,int length){
		_Complex float in2=fabs(__real__ *(in))+fabs(__imag__* (in))*I;	
		*(average)=*(average)+(in2- *(average))/length;
	//printf("in2 real=%f  in 2 imag=%f\n",creal(in2),cimag(in2));
		//printf("Average_real=%f  Average_imag=%f\n",__real__*(average),__imag__ *(average));


}

void SNR2(_Complex float* in,_Complex float* average,float* SNR){
		_Complex float noise=pow(fabs(__real__ *(in))-__real__ *(average),2)+pow(fabs(__imag__ *(in)) - __imag__ *(average),2)*I;	
		_Complex float signal=pow(__real__ *(average),2)+pow(__imag__ *(average),2)*I;

		float noise_total=creal(noise)+cimag(noise);
		float signal_total=creal(signal)+cimag(signal);
		if(noise_total<0.00001)noise_total=0.00001;
		*(SNR)=signal_total/noise_total;
	
		//printf("signal_real=%f  signal_imag=%f\n",creal(signal),cimag(signal));
		//printf("noise_real=%f  noise_imag=%f\n",creal(noise),cimag(noise));
		//printf("SNR estimated=%f dB\n",10.0*log10f( *(SNR) ));

}




_Complex float Average(_Complex float in,_Complex float average,int length){
		//_Complex float in2=creal(in)*creal(in)+cimag(in)*cimag(in)*I;
		_Complex float in2=fabs(creal(in))+fabs(cimag(in))*I;	
		//printf("in2 real=%f  in 2 imag=%f\n",creal(in2),cimag(in2));
		average=average+(in2-average)/length;
		//printf("Average_real=%f  Average_imag=%f\n",creal(average),cimag(average));
		return average;

}


float SNR(_Complex float in,_Complex float average){
		_Complex float noise=pow(fabs(creal(in))-creal(average),2)+pow(fabs(cimag(in))-cimag(average),2)*I;	
		_Complex float signal=pow(creal(average),2)+pow(cimag(average),2)*I;

		float noise_total=creal(noise)+cimag(noise);
		float signal_total=creal(signal)+cimag(signal);
		if(noise_total<0.0001)noise_total=0.0001;
		float SNR=signal_total/noise_total;
	
		//printf("signal_real=%f  signal_imag=%f\n",creal(signal),cimag(signal));
		//printf("noise_real=%f  noise_imag=%f\n",creal(noise),cimag(noise));
		//printf("SNR estimated =%f\n",SNR);

		return SNR;

}



void Calc_Coeficients(_Complex float* rcv_DMRS,_Complex float* DMRS_seq,_Complex float* Coeff,int length,DMRS_t* DMRS_params,info_t* Equalizer_info){
//en aquesta funcio es calculen els coeficients de nateja per la part real i la part imaginaria i no el nombre complexe invers del canal.

	for(int i=0;i<length;i++){
		if(DMRS_params->SSB_counter!=0 && DMRS_params->SSB_counter!=1){
			__real__ *(Coeff+i) = __real__ *(DMRS_seq+i)/__real__ *(rcv_DMRS+i);
			__imag__ *(Coeff+i) = __imag__ *(DMRS_seq+i)/__imag__ *(rcv_DMRS+i);
		}else{//used to avoid computing coefficient when the symbol has PSS in the central carriers
			if( i<(Equalizer_info->nPRB*6-120)/2){
				__real__ *(Coeff+i) = __real__ *(DMRS_seq+i)/__real__ *(rcv_DMRS+i);
				__imag__ *(Coeff+i) = __imag__ *(DMRS_seq+i)/__imag__ *(rcv_DMRS+i);

			}else if (i>(((Equalizer_info->nPRB*6-120)/2)+120)){
				__real__ *(Coeff+i) = __real__ *(DMRS_seq+i)/__real__ *(rcv_DMRS+i);
				__imag__ *(Coeff+i) = __imag__ *(DMRS_seq+i)/__imag__ *(rcv_DMRS+i);

			}
		}

	}
	if(DMRS_params->SSB_counter==0 || DMRS_params->SSB_counter==1){
		for(int i=(Equalizer_info->nPRB*6-120)/2;i<(((Equalizer_info->nPRB*6-120)/2)+60);i++){
			__real__ *(Coeff+i)=__real__ *(Coeff+i-1);
			__imag__ *(Coeff+i)=__imag__ *(Coeff+i-1);
		}

	for(int i=(((Equalizer_info->nPRB*6-120)/2)+120);i>=(((Equalizer_info->nPRB*6-120)/2)+60);i--){
			__real__ *(Coeff+i)=__real__ *(Coeff+i+1);
			__imag__ *(Coeff+i)=__imag__ *(Coeff+i+1);
		}
	}


}
	
void calc_Slot_symbol(int Ts,DMRS_t* DMRS_params){

	if(DMRS_params->Additional_DMRS_position==0){
		DMRS_params->OFDMsymbol=2;
		DMRS_params->Slotnumber=Ts%10;
		DMRS_params->SSB_counter=Ts%20;	

	}
	//TODO:Configure equalizer and params for Additional_DMRS==1,2,3
	
	
}


void Extract_DMRS_from_symbol(info_t* Equalizer_info,_Complex float* in,_Complex float* rcv_DMRS){

	int nPRB=Equalizer_info->nPRB;
	for(int i=0;i<nPRB*numCarriers_PRB/2;i++){
		__real__ *(rcv_DMRS+i) = __real__ *(in+i*2);
		__imag__ *(rcv_DMRS+i) = __imag__ *(in+i*2);
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


























