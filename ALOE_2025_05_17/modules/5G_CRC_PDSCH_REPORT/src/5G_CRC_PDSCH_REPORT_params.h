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
extern int mode;
extern unsigned int crc_poly; //0x1864CFB;
extern unsigned int crc_length;

/* oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo */
/* Configure the control parameters */
#define NOF_PARAMS	5

param_t parameters[] = {
		{"mode",INT,1},
		{"crc_poly",INT,1},
		{"crc_length",INT,1},
		{"CRANtype",INT,1},
		{NULL, 0, 0} /* need to end with null name */
};

/* This number MUST be equal or greater than the number of parameters*/
const int nof_parameters = NOF_PARAMS;
/* oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo */


#endif
