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
extern float floatp;
extern char stringp[];
extern float samplingfreqHz;


/* oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo */
/* Configure the control parameters */
#define NOF_PARAMS	18

param_t parameters[] = {
		{"block_length", 	/* Parameter name*/
		  INT,			/* Parameter type (see skeleton.h) */
		  1},			/* Number of samples (of size the type) */
		
		{"TX_RX_mode",INT,1},
		{"coderate",FLOAT,1},
		{"mod_order",INT,1},
		{"num_layers",INT,1},
		{"Overhead_higherlayer",INT,1},
		{"nPRB",INT,1},
		{"RNTI",INT,1},
		{"CELLID",INT,1},
		{"q",INT,1},
		{"rv",INT,1},
		{"DMRS_type",INT,1},
		{"Additional_DMRS",INT,1},
		{"CORESETS_symbol_size",INT,1},
		{"Agr_lvl_DCI",INT,1},
		{NULL, 0, 0} /* need to end with null name */
};

/* This number MUST be equal or greater than the number of parameters*/
const int nof_parameters = NOF_PARAMS;
/* oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo */


#endif
