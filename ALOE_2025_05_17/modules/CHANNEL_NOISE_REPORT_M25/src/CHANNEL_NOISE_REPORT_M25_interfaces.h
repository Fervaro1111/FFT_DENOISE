/* 
 * Copyright (c) 2012, Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
 * This file is part of ALOE++ (http://flexnets.upc.edu/)
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

#ifndef ITF_TYPES_H
#define ITF_TYPES_H

#ifndef SKELETON_H
#include "skeleton.h"
#endif

/* input and output data types */
typedef _Complex float  input_t;			//_Complex float, float, int, char
#define IN_TYPE			"COMPLEXFLOAT"		//COMPLEXFLOAT, FLOAT, INT, CHAR
#define INPUT_MAX_DATA 		512*2048
#define NOF_INPUT_ITF		2

//typedef _Complex float  output_t;
typedef _Complex float  output_t;			//_Complex float, float, int, char
#define OUT_TYPE		"COMPLEXFLOAT"		//COMPLEXFLOAT, FLOAT, INT, CHAR
#define OUTPUT_MAX_DATA 	512*2048
#define NOF_OUTPUT_ITF		2


/********* do not need to modify beyond here */
#ifdef INTERFACE_CONFIG

/* Input and output buffer sizes (in number of samples) */
const int input_max_samples = INPUT_MAX_DATA;
const int output_max_samples = OUTPUT_MAX_DATA;

/* leave these two lines unmodified */
const int input_sample_sz = sizeof(input_t);
const int output_sample_sz = sizeof(output_t);

/* Number of I/O interfaces. All have the same maximum size */
const int nof_input_itf = NOF_INPUT_ITF;
const int nof_output_itf = NOF_OUTPUT_ITF;

#endif
#endif
