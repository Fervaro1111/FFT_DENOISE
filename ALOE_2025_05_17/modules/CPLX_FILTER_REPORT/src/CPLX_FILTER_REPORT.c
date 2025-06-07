/* 
 * Copyright (c) 2012.
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

#include <complex.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <phal_sw_api.h>
#include "skeleton.h"
#include "params.h"

#include "CPLX_FILTER_REPORT_interfaces.h"
#include "CPLX_FILTER_REPORT_functions.h"
#include "CPLX_FILTER_REPORT.h"

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="CPLX_FILTER_REPORT";
float samplingfreqHz=48000.0;

//Module User Defined Parameters
int opmode=0;										//CPLX=0, ...
char filtercoefffile[STR_LEN]="FIR.coeff";
int filterlength=0;
_Complex float COEFFs[2048];
long int Tslot=0;
long int CHANNEL_Tslot=0;

MODparams_t oParam={opMODEBYPASS, READTEXT, 1, 1, "FIR.coeff"};

//Global Variables
print_t T;
_Complex float bufferA[2048];
float bufferB[2048];
int textcolor=221, backcolor=93;

//DETECT BW
#define FFTMAXSZ	8192
fftw_complex fftin[FFTMAXSZ], fftout[FFTMAXSZ];
fftw_plan fftplan;
_Complex float FFT[FFTMAXSZ];
float absFFT[FFTMAXSZ];
int fftsize=FFTMAXSZ;
float MeasBw=0.0;
int TslotBw=0;

/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	int i, noflines;
	int STAT=0;	//0-OK READING, 1-ERROR READING FILE

	/* Get control parameters*/
#ifndef STUDENTMODE
	param_get_int("opmode", &oParam.opMODE);
	param_get_int("readfileMODE", &oParam.readfileMODE);
#else
	oParam.opMODE=opMODEREAL;
	oParam.readfileMODE=READBIN;
	textcolor=15;
	backcolor=9;
#endif

	param_get_int("Ninterpol", &oParam.Ninterpol);
	param_get_int("Ndecimate", &oParam.Ndecimate);
	param_get_string("filtercoefffile", &oParam.filtercoefffile[0]);			

	/* Verify control parameters */
	if(oParam.Ninterpol == 0 || oParam.Ndecimate == 0){
		printf("%s: ERROR!!! values for Ninterpol=%d or Ndecimate=%d must be different from zero.\n", 
					mname, oParam.Ninterpol, oParam.Ndecimate);
		printf("%s: ERROR!!! values for Ninterpol=%d or Ndecimate=%d must be different from zero.\n", 
					mname, oParam.Ninterpol, oParam.Ndecimate);
		printf("%s: ERROR!!! values for Ninterpol=%d or Ndecimate=%d must be different from zero.\n", 
					mname, oParam.Ninterpol, oParam.Ndecimate);
		exit(0);
	}


/*	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      opmode=%d: 0-Bypass, 1-CPLX filter, 2-Real filter\n", oParam.opMODE);
	printf("O      readfileMODE=%d: 0-Read Text file (Generate .bin coeff file), 1-Read bin file\n", oParam.readfileMODE);
	printf("O      filtercoefffile=%s, filterlength=%d\n", &oParam.filtercoefffile[0], filterlength);
	printf("O      Ninterpol=%d, Ndecimate=%d\n", oParam.Ninterpol, oParam.Ndecimate);
	printf("O--------------------------------------------------------------------------------------------O\n");
*/


	// CPLX
	//if(opmode==1)readCPLXfilecoeff(COEFFs, &filterlength, filtercoefffile);
	if(oParam.opMODE==1)readCPLXfilecoeff(COEFFs, &filterlength, &oParam.filtercoefffile[0]);
	// REAL
	//if(opmode==2)readREALfilecoeff(COEFFs, &filterlength, filtercoefffile);
	if(oParam.opMODE==2){
		// READ TEXT COEFF FILE
		if(oParam.readfileMODE==0){
			// READ TEXT COEFF FILE
			readREALfilecoeff(COEFFs, &filterlength, &oParam.filtercoefffile[0]);
			// CREATE BIN COEFF FILE
			writeBINfilterCoeffFile(COEFFs, filterlength, &oParam.filtercoefffile[0]);
		}
		// READ BIN COEFF FILE
		if(oParam.readfileMODE==1){
			STAT=readBINfilterCoeffFile(COEFFs, &filterlength, &oParam.filtercoefffile[0]);
		}
	//	for(i=0; i<filterlength; i++)printf("COEFFs[i]=%7.5f+%7.5fi\n", __real__ COEFFs[i], __imag__ COEFFs[i]);
	}

	/* Print Module Init Parameters */
	//---------------------------------------------------------------------------------------
	strcpy(mname, GetObjectName());
	sprintf(&T.text[0][0],"SPECIFIC PARAMETERS SETUP: %s\n", mname);
	sprintf(&T.text[1][0]," Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       					NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	sprintf(&T.text[2][0]," opmode=%d: 0-Bypass, 1-CPLX filter, 2-Real filter\n", oParam.opMODE);
#ifndef STUDENTMODE
	sprintf(&T.text[3][0]," readfileMODE=%d: 0-Read Text file (Generate .bin coeff file), 1-Read bin file\n", oParam.readfileMODE);
#else
	sprintf(&T.text[3][0]," readfileMODE=%d: 1-Read bin file\n", oParam.readfileMODE);
#endif
	if(oParam.readfileMODE==READTEXT){
		sprintf(&T.text[4][0]," Generated filter file: \n");
		sprintf(&T.text[5][0],"  filtercoefffile=%s, filterlength=%d\n", &oParam.filtercoefffile[0], filterlength);
	}
	if(oParam.readfileMODE==READBIN){
		sprintf(&T.text[4][0]," Readed filter file: \n");
		sprintf(&T.text[5][0],"  filtercoefffile=%s, filterlength=%d\n", &oParam.filtercoefffile[0], filterlength);
	}
	sprintf(&T.text[6][0]," Ninterpol=%d, Ndecimate=%d\n", oParam.Ninterpol, oParam.Ndecimate);
	noflines=7;
	if(STAT==1){
		sprintf(&T.text[noflines][0],"  ERROR Reading .bin file. LEAVING...\n");
		noflines++;
	}
	// printTEXT(noflines, textcolor, backgroundcolor, border_char)
	printTEXT(noflines, textcolor, backcolor, '=');
	//---------------------------------------------------------------------------------------
	if(STAT==1)exit(0);
	
	//COMPUTE BANDWIDTH
	fftplan = fftw_plan_dft_1d(fftsize, fftin, fftout, FFTW_FORWARD, FFTW_ESTIMATE);


	return 0;
}



/**
 * @brief Function documentation
 *
 * @param inp Input interface buffers. Data from other interfaces is stacked in the buffer.
 * Use in(ptr,idx) to access the address. To obtain the number of received samples use the function
 * int get_input_samples(int idx) where idx is the interface index.
 *
 * @param out Input interface buffers. Data to other interfaces must be stacked in the buffer.
 * Use out(ptr,idx) to access the address.
 *
 * @return On success, returns a non-negative number indicating the output
 * samples that should be transmitted through all output interface. To specify a different length
 * for certain interface, use the function set_output_samples(int idx, int len)
 * On error returns -1.
 *
 * @code
 * 	input_t *first_interface = inp;
	input_t *second_interface = in(inp,1);
	output_t *first_output_interface = out;
	output_t *second_output_interface = out(out,1);
 *
 */
int work(input_t *inp, output_t *out) {
	int rcv_samples0 = get_input_samples(0); /** number of samples at itf 0 buffer */
	int rcv_samples1 = get_input_samples(1);
	int snd_samples0=0;
	int snd_samples1=0;
	input_t *input = inp;
	input_t *output = out;

	report_t *reportIN=in(inp,1);
	report_t *reportOUT=out(out,1);

	int i, n, MINidx, MAXidx;
	float MINn=1e6, MAXn=1e-6;
	

	// ESTIMATE BW
	//COMPUTE FFT
	if(Tslot>500){
		for(n=0; n<fftsize; n++)*(fftin+n)=(fftw_complex) *(input+n);
		fftw_execute(fftplan);
		for(n=0; n<fftsize; n++){
			*(FFT+n)=(_Complex float) *(fftout+n);
			*(absFFT+n)=cabsf(*(FFT+n));
			if(*(absFFT+n) > MAXn)MAXn=*(absFFT+n);
			if(*(absFFT+n) < MINn)MINn=*(absFFT+n);
		}
	//	printf("MAXn=%f, MINn=%f\n", MAXn, MINn);
		MINidx=-1;
		MAXidx=-1;
		for(n=0; n<fftsize; n++){
			if(*(absFFT+n) > 0.4*MAXn){
				MINidx=n;
				break;
			}
		}
		for(n=0; n<fftsize/2; n++){
			if(*(absFFT+fftsize/2-n) > 0.4*MAXn){
				MAXidx=fftsize/2-n;
				break;
			}
		}
		MeasBw += (MAXidx-MINidx)*samplingfreqHz/fftsize;
		TslotBw++;
	}

//	printf("MAXidx=%d, MINidx=%d, MeasBw=%f\n", MAXidx, MINidx, MeasBw/Tslot);
	
//	printf("CPLX_FILTER: Tslot=%d, ################ Received samples=%d, snd_samples1=%d\n", Tslot, rcv_samples1, snd_samples1);
/*
#ifdef STUDENTMODE
	if(rcv_samples0>0 && Tslot<MAXTSLOT){
#endif
		if(oParam.opMODE == 1 || oParam.opMODE == 2){
			snd_samples0 = stream_conv_CPLX_INT_DEC(input, rcv_samples0, COEFFs, filterlength, output, oParam);
		}
		else snd_samples0=bypass(input, rcv_samples0, output);
#ifdef STUDENTMODE
	}
#endif
*/
//	if(rcv_samples0>0){

/*	if((rcv_samples0 != 1024) && (rcv_samples0 != 0)){
		printf("%s: ERROR!!! Received samples = %d is different from expected = 1024\n", mname, rcv_samples0);
		printf("%s: ERROR!!! Closing Execution...Bye\n");
		exit(0);
	}
*/
//	printf("%s: Tslot=%ld, rcv_samples0=%d\n", mname, Tslot, rcv_samples0);
		if(oParam.opMODE == 1 || oParam.opMODE == 2){
			snd_samples0 = stream_conv_CPLX_INT_DEC(input, rcv_samples0, COEFFs, filterlength, output, oParam);
		}
		else snd_samples0=bypass(input, rcv_samples0, output);
//	printf("%s: Tslot=%ld, snd_samples0=%d\n", mname, Tslot, snd_samples0);
//	}
	if(snd_samples0 > 0)CHANNEL_Tslot=(int)Tslot;


	// UPDATE REPORT
	if(rcv_samples1>0){		
		update_CTRL(reportIN, reportOUT, &oParam.filtercoefffile[0], CHANNEL_Tslot, MeasBw/(float)TslotBw);
		snd_samples1=ceil((float)sizeof(report_t)/(float)sizeof(output_t));
		//printf("CPLX_FILTER: Tslot=%d, ################ Received samples=%d, snd_samples1=%d\n", reportOUT->CPLXFILTER_Tslot, rcv_samples1, snd_samples1);
	}
//	printf("3-CPLX_FILTER_REPORT.update_CTRL(): filename=%s\n", &reportOUT->filter_name[0]);
//	printf("CPLX_FILTER: Tslot=%d, ################ Received samples=%d, snd_samples1=%d\n", Tslot, rcv_samples1, snd_samples1);
	Tslot++;
	// Indicate the number of samples at at each interface
	set_output_samples(1, snd_samples1);
	return snd_samples0;
}

/** @brief Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	return 0;
}

