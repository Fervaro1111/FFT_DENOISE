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
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "5G_SYNCHRO_functions.h"

Syncro_t SynchroINFO;
SYNC_struc Sync_MAC;

fftw_complex fftin[MAXFFTSIZE], fftout[MAXFFTSIZE];
fftw_plan fftplan;
fftw_plan iffplan;
// PSSs Sequences
_Complex float PSS_time[NofNID2*MAXFFTSIZE];
int aFFTsize[6]={512, 1024, 1536, 2048, 3072, 4096};


/**
 * @genRSsignalargerThan3RB(): Compute the PUSCH reference sequence, DMRS.
 * @params int u: According 3GPP 36.211
 * @params int v:
 * @params int m:
 * @params int M_RS_SC: Sequence length
 * @params  _Complex float *DMRSseq: Output array.
 * @params int TxRxMode: -1 (Tx Mode), 1 (Rx Mode) .
 *
 * @return On success returns 1.
 * On error returns -1.
 */

int genRSsignalargerThan3RB(int u, int v, int m, int M_RS_SC, _Complex float *DMRSseq, int TxRxMode){
	int i, sequence_length;
	double arg, rootidx;
	int N_RS_ZC = largestprime_lower_than(M_RS_SC);
	float qNfloat = ((float)(N_RS_ZC*(u+1)))/31.0;
	int qNint = qNfloat;
	float q = (float)(int)((qNfloat + 0.5)) + (float)v*(float)pow((double)(-1.0), (double)(int)(2.0*qNfloat));

	sequence_length=M_RS_SC;

	printf("u=%d, v=%d, N_RS_ZC=%d, qNfloat=%f, qNint=%d, q=%f\n", u, v, N_RS_ZC, qNfloat, qNint, q);


	for(i=0; i<sequence_length; i++){
		arg=(((double)q)*((double)(i%N_RS_ZC)*((double)(i%N_RS_ZC)+1.0)))/(double)N_RS_ZC;
		__real__ DMRSseq[i]=(float)cos(arg);
		__imag__ DMRSseq[i]=(float)sin(arg);
	}
	return sequence_length;
}


#define CARRIERS1RB	12

/**
 * @create_PUSCHspectrum(): Create PUSCH spectrum.
 * @param *MQAMsymb: Pointer to input sequence.
 * @param datalength: length of input sequence.
 * @param FFTlength: 
 * @param *out_spectrum: Pointer to output sequence.
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int create_PUSCHspectrum(_Complex float *MQAMsymb, int datalength, 
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


/**@ingroup genDMRStime_seq
 * This module generate the PSS time sequence for the different FFT size
 * \param in_DMRSfreq: DMRS sequence allocated in frequency.
 * \param out_DMRStime: FFT output of DMRS sequence allocated in frequency.
 * \param FFTsize: define the size of the OFMD symbols: 128, 256, 512, 1024, 1536 o 2048.
 * \param TxRxmode: (1)-defines the sequence for Tx, (-1) for Rx side.
 */
int genDMRStime_seq(int FFTsize, _Complex float  *in_DMRSfreq, int DMRSlength, _Complex float  *out_DMRStime, int TxRxmode){

	int s, i;
	_Complex float  AUX[2048];

	/**FFT*/
	fftw_complex DMRS_freq[2048];
	fftw_complex DMRS_aux[2048];
	fftw_plan plan128;

	// Allocate DMRS sequence in PUSCH spectrum
	create_PUSCHspectrum(in_DMRSfreq, DMRSlength, FFTsize,  AUX);
	// Copy AUX into PSS_freq[]
	for(i=0; i<FFTsize; i++)DMRS_freq[i]=AUX[i];
	// FFT->convert to time
	if(FFTsize==FFTsize){
		plan128 = fftw_plan_dft_1d(FFTsize, DMRS_freq, DMRS_aux, FFTW_BACKWARD, FFTW_ESTIMATE);
		fftw_execute(plan128);
		// TX
		if(TxRxmode == TXMODE){
			for(i=0; i<FFTsize; i++){
				*(out_DMRStime+i)=(_Complex float)*(DMRS_aux+i);
			}
		}
		// RX
		if(TxRxmode == RXMODE){
			for(i=0; i<FFTsize; i++){
				__real__ *(out_DMRStime+i)=__real__ (_Complex float)*(DMRS_aux+FFTsize-1-i);
				__imag__ *(out_DMRStime+i)=-(__imag__ (_Complex float)*(DMRS_aux+FFTsize-1-i));
			}
		}
	}
	return 0;
}


/**
 * @brief Function documentation: detect_DMRS_in_subframe()
 * This function find the end position of DMRS inside an slot of samples length.
 * @params  _Complex float *correl: Correlation input.
 * @params int numsamples: number of samples .
 * @return On success returns correlation peak position.
 * On error returns -1.
 */
#define WINDOWSZSHORT	128
#define WINDOWPMAX		5
#define PROBThreshold	0.6

int detect_PSSs_in_subframe(_Complex float *correl, int numsamples, Syncro_t *SyncINFO){

	int i, k;
	int pMAX=-1;
	float maxval = -1000000.0, smaxval=-1000000.0, auxR;
	int pMAX0=-1, pMAX1=-1;
	int Llimit, Hlimit;
	float ratio0, ratio1, prob0, prob1, varianceSHORT;
	int distance6=SyncINFO->FFTsize*6+ceil(SyncINFO->FFTsize*7.0/100.0)*5+ceil(SyncINFO->FFTsize*7.8/100.0);
	int distance8=SyncINFO->FFTsize*8+ceil(SyncINFO->FFTsize*7.0/100.0)*7+ceil(SyncINFO->FFTsize*7.8/100.0);
	int iaux;
	float faux;

	//Find MAXs
	for(i=0; i<numsamples; i++){
		auxR=(float)(fabsf(__real__ *(correl+i)) + fabsf(__imag__ *(correl+i)));
/*		if(i==1647)printf("P1647: %3.3f+%3.3f*I\n", __real__ *(correl+i), __imag__ *(correl+i));
		if(i==1648)printf("P1648: %3.3f+%3.3f*I\n", __real__ *(correl+i), __imag__ *(correl+i));
		if(i==1649)printf("P1649: %3.3f+%3.3f*I\n", __real__ *(correl+i), __imag__ *(correl+i));
*/
		if(maxval <= auxR){
			smaxval=maxval;
			pMAX0=pMAX1;
			maxval = auxR;
			pMAX1=i;
		}else if(smaxval < auxR){
			smaxval = auxR;
			pMAX0 = i;
		}
	}
/*	printf("P4939: %3.3f+%3.3f*I\n", __real__ *(correl+4939), __imag__ *(correl+4939));
	printf("P4940: %3.3f+%3.3f*I\n", __real__ *(correl+4940), __imag__ *(correl+4940));
	printf("P4941: %3.3f+%3.3f*I\n", __real__ *(correl+4941), __imag__ *(correl+4941));
*/

//	printf("maxval=%3.3f, pMAX1=%d, smaxval=%3.3f, pMAX0=%d\n", maxval, pMAX1, smaxval, pMAX0);

	// CALCULATE SIGNAL VARIANCE AROUND THE pMAX0
	if(pMAX0 - WINDOWSZSHORT > 0)Llimit=pMAX0-WINDOWSZSHORT;
	if(pMAX0+WINDOWSZSHORT < numsamples)Hlimit=pMAX0+WINDOWSZSHORT;
	else Hlimit=numsamples;

	k=0;
	varianceSHORT=0.0;
	for(i=Llimit; i<Hlimit; i++){
		auxR=(float)(fabs(__real__ *(correl+i)) + fabs(__imag__ *(correl+i)));
		if(abs(pMAX0 - i) > WINDOWPMAX){
			varianceSHORT += auxR;
			k++;
		}
	}
	varianceSHORT = varianceSHORT/(float)k;
	if(isnan(varianceSHORT))varianceSHORT=1.0;
	if(varianceSHORT < 0.001){   //0.01
		varianceSHORT = 0.001;
	}	
	ratio0=smaxval/(varianceSHORT);
//	printf("pMAX0=%d, maxval=%3.2f, varianceSHORT=%3.2f, ratio0=%3.2f\n", pMAX0, smaxval, varianceSHORT, ratio0);
	prob0=1.0-(1.0/ratio0);

	// CALCULATE SIGNAL VARIANCE AROUND THE pMAX1
	if(pMAX1 - WINDOWSZSHORT > 0)Llimit=pMAX1-WINDOWSZSHORT;
	if(pMAX1+WINDOWSZSHORT < numsamples)Hlimit=pMAX1+WINDOWSZSHORT;
	else Hlimit=numsamples;

	k=0;
	varianceSHORT=0.0;
	for(i=Llimit; i<Hlimit; i++){
		auxR=(float)(fabs(__real__ *(correl+i)) + fabs(__imag__ *(correl+i)));
		if(abs(pMAX1 - i) > WINDOWPMAX){
			varianceSHORT += auxR;
			k++;
		}
	}
	varianceSHORT = varianceSHORT/(float)k;
	if(isnan(varianceSHORT))varianceSHORT=1.0;
	if(varianceSHORT < 0.001){   //0.01
		varianceSHORT = 0.001;
	}	
	ratio1=maxval/(varianceSHORT);
//	printf("pMAX1=%d, maxval=%3.2f, varianceSHORT=%3.2f, ratio1=%3.2f\n", pMAX1, smaxval, varianceSHORT, ratio1);
	prob1=1.0-(1.0/ratio1);

	// Exchange values if pMAX0 > pMAX1
	if(pMAX0 > pMAX1){
		iaux=pMAX1;
		pMAX1=pMAX0;
		pMAX0=iaux;

		faux=ratio1;
		ratio1=ratio0;
		ratio0=faux;
		
		faux=prob1;
		prob1=prob0;
		prob0=faux;
	}

	//printf("pMAX0=%d, ratio0=%1.3f, prob0=%1.3f, pMAX1=%d, ratio1=%1.3f, prob1=%1.3f\n", pMAX0, ratio0, prob0, pMAX1, ratio1, prob1);


	// DECIDE PMAX0: Indicates the first PSS in a subframe
	if(prob0 > PROBThreshold && prob1 > PROBThreshold){
		SyncINFO->CaptDISTANCE=pMAX1-pMAX0;
		if(SyncINFO->CaptDISTANCE == distance6)pMAX=pMAX0;
		if(SyncINFO->CaptDISTANCE == distance8)pMAX=pMAX1;
		if(SyncINFO->CaptDISTANCE != distance6 && SyncINFO->CaptDISTANCE != distance8)pMAX=-1;
	}

	if(pMAX >= 0){
		//printf("\033[1;31m pMAX0=%d, ratio0=%1.3f, prob0=%1.3f, pMAX1=%d, ratio1=%1.3f, prob1=%1.3f \033[0m\n", 
		//			pMAX0, ratio0, prob0, pMAX1, ratio1, prob1);
		//printf("\033[1;31m  CaptDISTANCE=%d, distance6=%d, distance8=%d \033[0m\n", SyncINFO->CaptDISTANCE, distance6, distance8);
	}else{
		//printf("\033[1;34m  pMAX0=%d, ratio0=%1.3f, prob0=%1.3f, pMAX1=%d, ratio1=%1.3f, prob1=%1.3f \033[0m\n", 
		//			pMAX0, ratio0, prob0, pMAX1, ratio1, prob1);
	}

	// MAYBE OTHER SCENARIOS SHOULD BE CONSIDERED

	return(pMAX);	
}

#define SUBFRAME_BUFF_SZ		FFTSIZE*(NOFOFDMSYMBPERSUBFRAME+1)
#define NOFSUBFRAMEINBUFFER		5
#define EMPTY		-1
#define NOTFULL	 	0
#define FULL	 	1
#define ACTIVE	 	1
#define NONACTIVE	0
_Complex float subframe_buffer[NOFSUBFRAMEINBUFFER][SUBFRAME_BUFF_SZ];
int stat_subframe[NOFSUBFRAMEINBUFFER]={EMPTY, EMPTY, EMPTY, EMPTY, EMPTY};	//-1: Empty, 0: Not Full, 1: Full

/**
 * @write_subframe_buffer(): Write uplink subframe in the output buffer but aligned
 *                           with the uplink LTE frame format. 
 *							 Each aligned subframe in the buffer is marked as FULL 
 *							 when ready to be read and send to the output.
 * @params	_Complex float *datain: Subframe Input sequence.
 * @params  int pMAX: Position of the last sample of the DMRS sequence.
 * @params int rcv_samples: received number of samples .
 * @params int FFTsize: SC-FDMA symbol length.
 * @return On success returns correlation 1.
 * On error returns -1.
 */
int write_subframe_buffer(_Complex float *datain, int pMAX, int rcv_samples, Syncro_t *SyncINFO, _Complex float *lastbuffer){

	int i;
	static int w_idx=0, w_subf_idx=0, first=0, firstSUBframe=0, RXActive=NONACTIVE;
	static int subframe_sz, PSS0_POS, DMRS1_POS;
	int remain;

	if(first==0){
		subframe_sz=SyncINFO->FFTsize*(NOFOFDMSYMBPERSUBFRAME+1);
		//DMRS0_POS=FFTsize*4-1;				// Final Position of DMRS0 sequence
		//DMRS1_POS=FFTsize*11-1;				// Final Position of DMRS1 sequence
		// Final Position of DMRS0 sequence
		PSS0_POS=SyncINFO->FFTsize*3+ceil(SyncINFO->FFTsize*7.0/100.0)*2+ceil(SyncINFO->FFTsize*7.8/100.0);
		printf("PSS0_POS=%d\n", PSS0_POS);
		first=1;
	}

//	printf("IN SUBFRAME BUFFERS [%d][%d][%d][%d][%d]\n", 
//		stat_subframe[0], stat_subframe[1],stat_subframe[2],stat_subframe[3],stat_subframe[4]);
//	printf("w_subf_idx=%d, w_idx=%d\n", w_subf_idx, w_idx);

	if(pMAX >= 0){
		RXActive=ACTIVE; // RX active from here
		if(pMAX == PSS0_POS){
			for(i=0; i<rcv_samples; i++){
				subframe_buffer[w_subf_idx][i]=*(datain+i);
			}
			//printf("pMAX == PSS0_POS AA stat_subframe[%d]=%d \n", w_subf_idx, stat_subframe[w_subf_idx]);
			stat_subframe[w_subf_idx]=FULL;
			w_subf_idx++;
			w_idx=0;
			if(w_subf_idx == NOFSUBFRAMEINBUFFER)w_subf_idx=0;	
		}
		if(pMAX > PSS0_POS){
			// Continue writting in current subframe for the pMAX-DMRS0_POS samples	
			//printf("pMAX > PSS0_POS INITIAL w_idx=%d\n", w_idx);
			if(w_idx != subframe_sz-(pMAX-PSS0_POS) && firstSUBframe==1){
//				printf("AB ERROR!!! w_idx=%d != subframe_sz-pMAX-DMRS0_POS=%d\n", w_idx, subframe_sz-(pMAX-DMRS0_POS));
			}
			w_idx=subframe_sz-(pMAX-PSS0_POS);		
			for(i=0; i<pMAX-PSS0_POS; i++){
				subframe_buffer[w_subf_idx][w_idx]=*(datain+i);
				w_idx++;
				if(w_idx == subframe_sz && firstSUBframe==1){
					stat_subframe[w_subf_idx]=FULL;
//					printf("AB stat_subframe[%d]=%d \n", w_subf_idx, stat_subframe[w_subf_idx]);
//					printf("i=%d, pMAX-DMRS0_POS=%d, w_idx=%d\n", i, pMAX-DMRS0_POS, w_idx);
					w_subf_idx++;
					if(w_subf_idx == NOFSUBFRAMEINBUFFER)w_subf_idx=0;	
					w_idx=0;
					break;
				}
			}
			if(firstSUBframe==0)firstSUBframe=1;
			w_idx=0;
//			if(stat_subframe[w_subf_idx]!=EMPTY)printf("B stat_subframe[%d]=%d (NO EMPTY)\n", w_subf_idx, stat_subframe[w_subf_idx]);
			for(i=0; i<rcv_samples-(pMAX-PSS0_POS); i++){
				subframe_buffer[w_subf_idx][w_idx]=*(datain+i+(pMAX-PSS0_POS));
				w_idx++;
			}
//			printf("FINAL w_idx=%d\n", w_idx);
		}
		if(pMAX < PSS0_POS){
			
			// Automatic synchronization
			w_idx=PSS0_POS-pMAX;
			// Continue writting in current subframe for the rcv_samples samples					
			for(i=0; i<rcv_samples; i++){
				subframe_buffer[w_subf_idx][w_idx]=*(datain+i);
				w_idx++;
				if(w_idx == subframe_sz){
					stat_subframe[w_subf_idx]=FULL;
//					printf("AC stat_subframe[%d]=%d \n", w_subf_idx, stat_subframe[w_subf_idx]);
					w_idx=0;
					w_subf_idx++;
					if(w_subf_idx == NOFSUBFRAMEINBUFFER)w_subf_idx=0;
//					if(stat_subframe[w_subf_idx]!=EMPTY)printf("C stat_subframe[%d]=%d (NO EMPTY)\n", w_subf_idx, stat_subframe[w_subf_idx]);
				}
			}
			if(firstSUBframe==0){
				for(i=0; i<PSS0_POS-pMAX; i++){
					subframe_buffer[w_subf_idx][i]=*(lastbuffer+i+(subframe_sz-(PSS0_POS-pMAX)));
				}
				firstSUBframe=1;
			}

		}
	}else{
		if(RXActive==ACTIVE){
			for(i=0; i<rcv_samples; i++){
				subframe_buffer[w_subf_idx][w_idx]=*(datain+i);
				w_idx++;
				if(w_idx == subframe_sz){
					stat_subframe[w_subf_idx]=FULL;
					w_idx=0;
					w_subf_idx++;
					if(w_subf_idx == NOFSUBFRAMEINBUFFER)w_subf_idx=0;
//					if(stat_subframe[w_subf_idx]!=EMPTY)printf("E stat_subframe[%d]=%d (NO EMPTY)\n", w_subf_idx, stat_subframe[w_subf_idx]);
				}
			}
		}
	}
	/*printf("OUT SUBFRAME BUFFERS [%d][%d][%d][%d][%d]\n", stat_subframe[0], stat_subframe[1],stat_subframe[2],stat_subframe[3],stat_subframe[4]);*/
	return(0);
}

/**
 * @read_subframe_buffer():  Read uplink subframe from the output buffer to the module
 *                           output when marked as FULL. After reading buffer is marked
 * 							 EMPTY. 
 * @params	_Complex float *dataout: Pointer to output.
 * @params int FFTsize: SC-FDMA symbol length.
 * @return On success returns number of samples send.
 * On error returns -1.
 */
int read_subframe_buffer(_Complex float *dataout, Syncro_t *SyncINFO){

	int i, read_samples=0;
	static int r_subf_idx=0, first=0;
	static int subframe_sz, DMRS0_POS, DMRS1_POS;

	if(first==0){
		subframe_sz=SyncINFO->FFTsize*(NOFOFDMSYMBPERSUBFRAME+1);
		first=1;
	}

	if(stat_subframe[r_subf_idx] == FULL){
		read_samples=subframe_sz;
		for(i=0; i<read_samples; i++){
			*(dataout+i) = subframe_buffer[r_subf_idx][i];
		}
		stat_subframe[r_subf_idx] = EMPTY;
		r_subf_idx++;
		if(r_subf_idx == NOFSUBFRAMEINBUFFER)r_subf_idx=0;
	}else{
//		printf("stat_subframe[%d]=EMPTY\n",r_subf_idx);
	}
	return(read_samples);
}


//////////////////////////////////////////////LIBRARY/////////////////////////////////////////////////////////
/**@ingroup rotateCvector()
 * This module introduce a phase change of Adegrees in the data sequence.
 * \param *inout: Input/output complex vector pointer
 * \param length: Length of data sequence.
 * \param Adegrees: Degrees of phase variation.
 * \param Pdegrees: Accuracy in degrees of phasortable.
 * Return -1 if error, 1 if OK.
 */
void rotateCvector(_Complex float *in, _Complex float *out, int length, float Adegrees){
	int i;
	_Complex float phasor;
	float degrees;

	degrees = Adegrees;
	phasor=cos(degrees*PI/180.0)+(sin(degrees*PI/180.0))*I;
	for(i=0; i<length; i++){
		*(out+i)=*(in+i)*phasor;
	}
}

/**@checkCFO()
 * This function estimates CFO acording: "An efficient CFO Estimation Algorithm for the Downlink of 3GPP-LTE"
 * \param *DMRScorrelation: Correlation of the received DMRS signal with the original DMRS
 * \param fftsize: FFT size.
 * \param CFO: CFO estimation in .
 * Return -1 if error, 1 if OK.
 */

float checkPhaseOffset(_Complex float *DMRScorrelation){
	_Complex float PhaseOffset, aux=(180.0/PI); 
	double PhaseOffsetdegrees=0.0;
	
	PhaseOffset=*DMRScorrelation;	
	PhaseOffsetdegrees=atan2f((double)__imag__ PhaseOffset, (double)__real__ PhaseOffset)*aux;
//	printf("AOOocheckCFO(): CFOdegree=%2.3f\n", (float)PhaseOffsetdegrees);
	return((float)PhaseOffsetdegrees);
}



/**@largestprime_lower_than(): Computes the largest prime lower than number.
 * \param number: 
 * Return -1 if error, 1 if OK.
 */

int largestprime_lower_than(int number){

	int i; 

	for(i=1; i<number; i++){
		if(check_if_prime(number-i)==1) break;
	}
	return(number-i);
}

/**@check_if_prime(): Check if "number" is prime.
 * \param number: 
 * Return 1 if primer, 0 if not.
 */
int check_if_prime(int number){

    int n, i, flag = 1;

    for(i = 2; i <= number/2; ++i)
    {
        // condition for nonprime number
        if(number%i == 0)
        {
            flag = 0;
            break;
        }
    }
	// flag 1 if prime
	// flag 0 if not
	return flag;
	
}



#define FILTERLENGTH	1024
_Complex float aux[FILTERLENGTH];
#define MAX_DATA 50*2048

int stream_conv_CPLX(_Complex float *ccinput, int datalength, _Complex float *filtercoeff, int filterlength, _Complex float *ccoutput){

	int i, j;
	static int first=0;
	_Complex float CCinput[MAX_DATA];
	_Complex float CCoutput[MAX_DATA];

	if(datalength>MAX_DATA){
		printf("ERROR!!!.In stream_conv_CPLX() datalength=%d exceeding INPUT_MAX_DATA=%d\n", datalength, MAX_DATA);
		exit(0);
	}
	if(filterlength>FILTERLENGTH){
		printf("ERROR!!!.In stream_conv_CPLX() filterlength=%d exceeding FILTERLENGTH=%d\n",filterlength, FILTERLENGTH);
		exit(0);
	}
//	printf("datalength=%d, filterlength=%d\n", datalength, filterlength);

	memcpy(CCinput, ccinput, sizeof(_Complex float)*datalength);

	if(first==0){
		for(j=0; j<filterlength; j++){
			aux[j]=0.0+0.0i;
			//printf("FIRST CCONV real=%3.6f, imag=%3.6f\n", __real__ filtercoeff[j], __imag__ filtercoeff[j]);
		}
		first=1;
	}
	for (i=0;i<datalength;i++) {
		for (j=filterlength-2;j>=0;j--) {
			aux[j+1]=aux[j];
		}
		aux[0]=CCinput[i];
		CCoutput[i]=0.0;
		for (j=0;j<filterlength;j++) {
			CCoutput[i]+=aux[j]*filtercoeff[j];
		}
	}
	memcpy(ccoutput, CCoutput, sizeof(_Complex float)*datalength);
	return datalength;
}

// Return length
// FROM MATLAB
int createDMRS(_Complex float *DMRS){
	*(DMRS)=1.0000 + 0.0000*I;
	*(DMRS+1)=-0.0810 + 0.9967*I;
  	*(DMRS+2)=-0.9610 + 0.2766*I;
  	*(DMRS+3)=-0.8839 - 0.4677*I;
  	*(DMRS+4)=-0.6886 - 0.7251*I;
  	*(DMRS+5)=-0.7692 - 0.6390*I;
  	*(DMRS+6)=-0.9912 - 0.1324*I;
  	*(DMRS+7)=-0.6447 + 0.7645*I;
   	*(DMRS+8)=0.6779 + 0.7352*I;
   	*(DMRS+9)=0.4872 - 0.8733*I;
  	*(DMRS+10)=-0.9687 + 0.2481*I;
  	*(DMRS+11)=0.9930 + 0.1177*I;
  	*(DMRS+12)=-0.9990 - 0.0442*I;
   	*(DMRS+13)=0.8907 - 0.4546*I;
  	*(DMRS+14)=-0.1397 + 0.9902*I;
  	*(DMRS+15)=-0.9524 - 0.3048*I;
   	*(DMRS+16)=0.0369 - 0.9993*I;
   	*(DMRS+17)=0.7785 - 0.6276*I;
   	*(DMRS+18)=0.9650 - 0.2624*I;
   	*(DMRS+19)=0.9568 - 0.2907*I;
   	*(DMRS+20)=0.7200 - 0.6940*I;
  	*(DMRS+21)=-0.1104 - 0.9939*I;
  	*(DMRS+22)=-0.9947 - 0.1031*I;
   	*(DMRS+23)=0.1250 + 0.9922*I;
   	*(DMRS+24)=0.6992 - 0.7149*I;
  	*(DMRS+25)=-0.9430 + 0.3328*I;
   	*(DMRS+26)=0.9478 - 0.3188*I;
  	*(DMRS+27)=-0.7302 + 0.6833*I;
  	*(DMRS+28)=-0.0516 - 0.9987*I;
   	*(DMRS+29)=0.9788 + 0.2050*I;
  	*(DMRS+30)=-0.0221 + 0.9998*I;
  	*(DMRS+31)=-0.8227 + 0.5685*I;
  	*(DMRS+32)=-0.9947 + 0.1031*I;
  	*(DMRS+33)=-0.9990 + 0.0442*I;
  	*(DMRS+34)=-0.9099 + 0.4148*I;
  	*(DMRS+35)=-0.3118 + 0.9501*I;
   	*(DMRS+36)=0.8142 + 0.5806*I;
   	*(DMRS+37)=0.4612 - 0.8873*I;
  	*(DMRS+38)=-0.9947 + 0.1031*I;
   	*(DMRS+39)=0.9037 + 0.4282*I;
  	*(DMRS+40)=-0.8548 - 0.5190*I;
   	*(DMRS+41)=0.9788 + 0.2050*I;
  	*(DMRS+42)=-0.8391 + 0.5440*I;
  	*(DMRS+43)=-0.2266 - 0.9740*I;
   	*(DMRS+44)=0.9478 - 0.3188*I;
   	*(DMRS+45)=0.7597 + 0.6503*I;
   	*(DMRS+46)=0.2695 + 0.9630*I;
   	*(DMRS+47)=0.1250 + 0.9922*I;

	return(48);
}


/*#define PSSLENGTH 	62	//Number of PSS Symbols in Zadoff-Chu sequence
#define PSSCELLID0 	25.0
#define PSSCELLID1 	29.0
#define PSSCELLID2 	34.0
*/

/** PRIMARY SYNCH SIGNALS: PSS*/
/**
 * @brief Function documentation: setPSS()
 * This function calculates the Zadoff-Chu sequence.
 * @params
 * @params int phylayerID:(0, 1, 2) Physical Layer Identity within the
 * Physical Layer cell-Identity Group.
 * @params  _Complex float *PSSsymb: Output array.
 * @params int TxRxMode: -1 (Tx Mode), 1 (Rx Mode) .
 *
 * @return On success returns 1.
 * On error returns -1.
 */

/*int setPSS(int phylayerID, _Complex float *PSSsymb, int TxRxMode){
	int i;
	double arg, rootidx;

	if(phylayerID == 0)rootidx = PSSCELLID0;
	if(phylayerID == 1)rootidx = PSSCELLID1;
	if(phylayerID == 2)rootidx = PSSCELLID2;

	for(i=0; i<PSSLENGTH/2; i++){
		arg=(((double)TxRxMode)*PI*rootidx*((double)i*((double)i+1.0)))/63.0;
		__real__ PSSsymb[i]=(float)cos(arg);
		__imag__ PSSsymb[i]=(float)sin(arg);
	}
	for(i=PSSLENGTH/2; i<PSSLENGTH; i++){
		arg=(((double)TxRxMode)*PI*rootidx*(((double)i+2.0)*((double)i+1.0)))/63.0;
		__real__ PSSsymb[i]=(float)cos(arg);
		__imag__ PSSsymb[i]=(float)sin(arg);
	}
	return 1;
}*/

//////////////////////////////////////////////////////////////////////////////5G/////////////////////////////////////////////////////////////////////////////

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


void PSS_gen (int NID2, _Complex float *PSS){
			
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
	

void gen_PSS_time(int NID2, int FFTsize,_Complex float* output){

	_Complex float PSS_seq[128];
	_Complex float PSS_spectrum[4096];


	printf("gen_PSS_time A\n");
	PSS_gen (NID2, &PSS_seq[0]);
	//ALLOCATE THE PSS IN THE CENTER OF THE BANDWIDTH
	create_spectrum(&PSS_seq[0], PSSlength, FFTsize, &PSS_spectrum[0]);
	
	fftplan = fftw_plan_dft_1d(FFTsize, fftin, fftout, FFTW_BACKWARD, FFTW_ESTIMATE);

	for(int n=0; n<FFTsize; n++)*(fftin+n)=(fftw_complex) PSS_spectrum[n];
	//Execute FFT
	fftw_execute(fftplan);
	//Copy processed data to output buffer
	for(int n=0; n<FFTsize; n++)*(output+n)=(_Complex float) *(fftout+n)/FFTsize;
	printf("gen_PSS_time Z\n");
}

