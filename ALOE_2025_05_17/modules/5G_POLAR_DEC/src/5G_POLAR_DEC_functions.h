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

#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H
#ifdef __cplusplus
extern "C" {
#endif
	int init_decoder(char *matrix_path, int K, int N);
	int decode(float *sbits_in, unsigned char *bytes_out, int nin);
	int calc_out_bits_max_length(int Carr_agr_lvl,int symb_number_Control);
	int Calc_n(int E,int K);
#ifdef __cplusplus
}
#endif
#endif
