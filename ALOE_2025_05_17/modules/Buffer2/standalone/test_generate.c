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

/* Functions that generate the test data fed into the DSP modules being developed */
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "params.h"
#include "Buffer2_interfaces.h"
#include "Buffer2.h"
#include "test_generate.h"

int offset=200;

/**
 * @brief Generates input signal. VERY IMPORTANT to fill length vector with the number of
 * samples that have been generated.
 * @param inp Input interface buffers. Data from other interfaces is stacked in the buffer.
 * Use in(ptr,idx) to access the address.
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 **/

int generate_input_signal(input_t *input, int *lengths)
{
	int i, data_length=0;
	char aux_c;
	int aux_i;
	float aux_f;
	_Complex float aux_cf;
	//For inputs different to 0
	//input_t *inputA, *inputB;
	
	//Module Defined Parameters
	char mname[STR_LEN]="Buffer2";
	int run_times=1;
	int block_length=11;
	char plot_modeIN[STR_LEN]="DEFAULT";
	char plot_modeOUT[STR_LEN]="DEFAULT";
	float samplingfreqHz=1.0;

	float floatp=1.1;
	char stringp[STR_LEN]="MY_DEFAULT_GEN";
	float freq=200.0;

	//Get Parameter Values
	param_get_int("block_length", &block_length);
	if (!block_length) {
		moderror("Parameter block_length is zero\n");
		return -1;
	}
	param_get_int("run_times", &run_times);
	if (!run_times) {
		moderror("Parameter run_times is zero\n");
		return -1;
	}
	param_get_float("samplingfreqHz", &samplingfreqHz);

	//Check parameter values
	if (block_length > get_input_max_samples()) {
		moderror_msg("Block length %d too large\n",block_length);
		return -1;
	}
	param_get_string("plot_modeIN", &plot_modeIN[0]);	//Initialized by hand or config file
	param_get_string("plot_modeOUT", &plot_modeOUT[0]);


	/* Print Module Init Parameters */
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: %s.generate_input_signal().\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O      block_length=%d, run_times=%d, sampligfreqHz=%3.3f\n", block_length, run_times, samplingfreqHz);
	printf("O      plot_modeIN=%s, plot_modeOUT=%s\n", plot_modeIN, plot_modeOUT);
	printf("O      floatp=%3.3f, stringp=%s\n", floatp, stringp);
	printf("O--------------------------------------------------------------------------------------------O\n");

	printf("GENERATE INPUT SIGNALooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo\n");

	/** Generate Signal */
	//Define the buffer pointer for input i: input_t *inputI = in(inp,i); 
	data_length = block_length*run_times;
	printf("GENERATE SIGNAL. standalone/test_generate(): lengths[0]=%d\n", lengths[0]);
	printf("GENERATE SIGNAL. standalone/test_generate(): data_length=%d, run_times=%d\n", data_length, run_times);

	//CHAR
	if(strcmp(IN_TYPE, "CHAR")==0){
		printf("CHAR\n");
		for (i=0;i<data_length;i++) {
			aux_c=(input_t)((i+offset)%(block_length));
			input[i]=(input_t)aux_c;
		}
	}
	//INT
	if(strcmp(IN_TYPE, "INT")==0){
		printf("INT\n");
		for (i=0;i<data_length;i++) {
			aux_i=(input_t)((i+offset)%(block_length));
			input[i]=(input_t)aux_i;
		}
	}
	//FLOAT
	if(strcmp(IN_TYPE, "FLOAT")==0){
		printf("FLOAT\n");
		for (i=0;i<data_length;i++) {
			aux_f=(input_t)((i+offset)%(block_length));
			input[i]=(input_t)aux_f;
		}
	}
	//COMPLEX
	if(strcmp(IN_TYPE, "COMPLEXFLOAT")==0){
		printf("COMPLEXFLOAT\n");
		for (i=0;i<data_length;i++) {
			__real__ aux_cf=cos(2*3.1415*((double)i)*freq/samplingfreqHz)+0.1*cos(2*3.1415*((double)i)*2*freq/samplingfreqHz);	
			__imag__ aux_cf=sin(2*3.1415*((double)i)*freq/samplingfreqHz)+0.1*sin(2*3.1415*((double)i)*2*freq/samplingfreqHz);	

			//__real__ aux_cf=-1.0 + 2.0*((i%7)%2)+(float)(i%11)/100.0;	//(i+offset)%(block_length);
			//__imag__ aux_cf=-1.0 + 2.0*((i%3)%2)+(float)(i%13)/100.0;	//(block_length-i-1+offset)%(block_length);

			input[i]=(input_t)aux_cf;
		}
	}
	//Provide the data length generated
	lengths[0] = data_length;
	return 0;
}
