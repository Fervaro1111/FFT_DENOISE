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

#include <phal_sw_api.h>
#include <swapi_utils.h>
#include <assert.h>
#include <stdio.h>

#include "skeleton.h"
#include "params.h"

#include "RATE_MATCHING_params.h"

#define INTERFACE_CONFIG
#include "RATE_MATCHING_interfaces.h"
#undef INTERFACE_CONFIG

#include "stats.h"

#define MAX_STATS 100
#define MAX_INPUTS 10
#define MAX_OUTPUTS 10

extern const int input_sample_sz;
extern const int output_sample_sz;
extern const int nof_parameters;
extern const int nof_input_itf;
extern const int nof_output_itf;
extern const int input_max_samples;
extern const int output_max_samples;
extern const int nof_extra_vars;

struct utils_itf input_itfs[MAX_INPUTS], output_itfs[MAX_OUTPUTS];
struct utils_param params =  {NULL, 0, 0, 0};
struct utils_stat stats[MAX_STATS];
int stat_ids[MAX_STATS];
char input_itf_str[MAX_INPUTS][64], output_itf_str[MAX_OUTPUTS][64];

char ctrl_itf_name[] = "control";

char *input_buffer;
char *output_buffer;
int input_nsamples[MAX_INPUTS];
int output_nsamples[MAX_INPUTS];

extern struct utils_variables vars_extra[];
extern int currentInterfaceIdx;

#define addStat(a,b,c,d,e) \
		stats[statcnt].name = a; \
		stats[statcnt].type = b; \
		stats[statcnt].size = c; \
		stats[statcnt].id = &stat_ids[statcnt]; \
		stats[statcnt].value = d; \
		stats[statcnt].update_mode = e; \
		statcnt++;

#define endStats stats[statcnt].name=NULL;

int process_control(int len);
int rcv_input(int len);

void allocate_memory() {
	/**@TODO: use static memory allocator */
	input_buffer = malloc(nof_input_itf*input_max_samples*input_sample_sz);
	assert(input_buffer);
	output_buffer = malloc(nof_output_itf*output_max_samples*output_sample_sz);
	assert(output_buffer);
}

void free_memory() {
	free(input_buffer);
	free(output_buffer);
}

void ConfigInterfaces() {
	int i;
	int j=0;
	int statcnt=0;
	param_t *param;
	char itf_str[64];
	int tmp;

	allocate_memory();

	if (param_init(parameters,nof_parameters) == -1) {
		printf("Error configuring module_template_parameters.h\n");
		return;
	}

	if (nof_parameters+nof_extra_vars+1 > MAX_STATS) {
		printf("Error too many parameters and variables.\nIncrease MAX_STATS in skeleton.c\n");
		return;
	}

	if (nof_input_itf > MAX_INPUTS) {
		printf("Error too many input interfaces.\nIncrease MAX_INPUTS in skeleton.c\n");
		return;
	}

	if (nof_output_itf > MAX_OUTPUTS) {
		printf("Error too many input interfaces.\nIncrease MAX_INPUTS in skeleton.c\n");
		return;
	}

	if (!InitParamFile()) {
		printf("Error initiating parameters file\n");
	}

	/* configure control interface */
	input_itfs[0].name = ctrl_itf_name;
	input_itfs[0].sample_sz = param_packet_size();
	input_itfs[0].max_buffer_len = 1;
	input_itfs[0].buffer = param_packet_addr();
	input_itfs[0].process_fnc = process_control;
	input_itfs[0].get_block_sz = NULL;

	/** configure inputs */
	for (i = 1; i < nof_input_itf+1; i++) {
		snprintf(input_itf_str[i-1], 64, "input_%d", i-1);

		input_itfs[i].name = input_itf_str[i-1];

		input_itfs[i].sample_sz = input_sample_sz;
		input_itfs[i].max_buffer_len = input_max_samples;
		input_itfs[i].buffer = &input_buffer[(i-1)*input_max_samples*input_sample_sz];	//MODIFIED 11/03/2015
		input_itfs[i].process_fnc = rcv_input;
		input_itfs[i].get_block_sz = NULL;
	}
	input_itfs[i].name = NULL;


	/** configure outputs */
	for (i = 0; i < nof_output_itf; i++) {
		snprintf(output_itf_str[i], 64, "output_%d", i);

		output_itfs[i].name = output_itf_str[i];

		output_itfs[i].sample_sz = output_sample_sz;
		output_itfs[i].max_buffer_len = output_max_samples;
		output_itfs[i].buffer = &output_buffer[i*output_max_samples*output_sample_sz];	//MODIFIED 11/03/2015
		output_itfs[i].process_fnc = NULL;
		output_itfs[i].get_block_sz = NULL;
	}
	output_itfs[i].name = NULL;


	for (i = 0; i < param_nof() && i<nof_parameters; i++) {
		param = param_get_i(i);
		assert(param);

		/** get param */
		if (!GetParam(param->name, param_get_addr_i(i),
				param_get_aloe_type_i(i),param->size)) {
			//printf("Caution parameter %s not initated\n", param->name);
			printf("\033[1m\033[31mCaution parameter %s not initiated for %s\033[0m\n", param->name, GetObjectName());
		}

		/* copy stats structure, will be initialized by the skeleton */
		addStat(param->name, param_get_aloe_type_i(i),
				param->size, param_get_addr_i(i), READ);
	}

	/* copy extra variables */
	for (i=0;vars_extra[i].name && i<nof_extra_vars;i++) {
		if (vars_extra[i].value) {
			addStat(vars_extra[i].name,vars_extra[i].type,vars_extra[i].size,
					vars_extra[i].value,vars_extra[i].update_mode);
		} else {
			tmp=0;
			/* if value is NULL, try if it is an interface */
			for (j=0;j<nof_input_itf;j++) {
				if (!strcmp(input_itf_str[j],vars_extra[i].name)) {
					addStat(vars_extra[i].name,vars_extra[i].type,vars_extra[i].size,
						&input_buffer[j*input_max_samples],vars_extra[i].update_mode);
					break;
				}
			}
			if (j==nof_input_itf) {
				tmp=1;
			}
			for (j=0;j<nof_output_itf;j++) {
				if (!strcmp(output_itf_str[j],vars_extra[i].name)) {
					addStat(vars_extra[i].name,vars_extra[i].type,vars_extra[i].size,
						&output_buffer[j*output_max_samples],vars_extra[i].update_mode);
					break;
				}
			}
			if (!tmp && j==nof_output_itf) {
				printf("Caution could not find interface %s for extra variable\n",
						vars_extra[i].name);
			}
		}
	}

	endStats;

}

/** Called after data is received, we save the number of bytes that have been received.
 */
int rcv_input(int len) {
	assert(currentInterfaceIdx>0 && currentInterfaceIdx<nof_input_itf+1);
	input_nsamples[currentInterfaceIdx-1] = len;
	return 1;
}

/** Returns the number of samples at input interface idx
 */
int get_input_samples(int idx) {
	assert(idx>=0 && idx<nof_input_itf);
	return input_nsamples[idx];
}

int get_input_max_samples() {
	return input_max_samples;
}

/** Sends nsamples through output interface idx
 * By default, the number of samples to send is the value returned by the work() function
 */
void set_output_samples(int idx, int nsamples) {
	assert(idx>=0 && idx<nof_output_itf);
	output_nsamples[idx] = nsamples;
}

/** RunCustom() function process the received data by calling the user processing function.
 *  return 1 if ok, 0 if error.
 */
int RunCustom() {
	int n, i;

	/** set output length to a default value */
	for (i = 0; i < nof_output_itf; i++) {
			output_nsamples[i] = 0;
	}


	n = work(input_buffer, output_buffer);
	if (n == -1) {
		return 0;
	}
	output_nsamples[0] = n;

	/** set output length to a default value */
/*	for (i = 0; i < nof_output_itf; i++) {
		if (!output_nsamples[i]) {
			output_nsamples[i] = n;
		}
	}
*/
	/* send output data */
	for (i = 0; i < nof_output_itf; i++) {
		SendItf(i, output_nsamples[i]);
//		printf("module_template. output_nsamples[%d]=%d\n", i, output_nsamples[i]);
	}

	/* reset len counters */
	memset(output_nsamples, 0, sizeof(int) * nof_output_itf);
	memset(input_nsamples, 0, sizeof(int) * nof_input_itf);

	return 1;
}

/** function called by background skeleton after all the initialization process.
 * Here we call User init function to initialize object-specific variables.
 */
int InitCustom() {
	if (initialize()) {
		return -1;
	}
	return 1;
}

int StopCustom() {
	stop();
	free_memory();
	return 1;
}

int process_control(int len) {
	param_packet_read(len);
	return initialize();
}

