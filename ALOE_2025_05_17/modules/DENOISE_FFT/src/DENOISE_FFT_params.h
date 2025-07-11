/* 
 * Copyright (c) 2012
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


#ifndef _MOD_PARAMS_H
#define _MOD_PARAMS_H

/* oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo */
/* Define the module control parameters & control structures and configure default values*/
/* Do not initialize here the extern variables*/
extern int run_times;
extern int block_length;
extern char plot_modeIN[];
extern char plot_modeOUT[];
extern float samplingfreqHz;
extern int fftsize;
extern int numffts;
extern int op_mode;
extern int filter_mode;
extern float threshold;
extern float trans_bw;
extern float cutoff_freq;
extern int dbg_level;
/* oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo */
/* Configure the control parameters */
#define NOF_PARAMS	15

param_t parameters[] = {
		{"samplingfreqHz",FLOAT,1},
		{"fftsize",INT,1},
		{"numffts",INT,1},
		{"op_mode",INT,1},
		{"filter_mode",INT,1},
		{"threshold",FLOAT,1},
		{"trans_bw",FLOAT,1},
		{"cutoff_freq",FLOAT,1},
		{"dbg_level",INT,1},
		{NULL, 0, 0} /* need to end with null name */
};

/* This number MUST be equal or greater than the number of parameters*/
const int nof_parameters = NOF_PARAMS;
/* oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo */


#endif
