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
int init_functionA_COMPLEX(_Complex float *input, int length);
int functionA_COMPLEX(_Complex float *input, int lengths, _Complex float *output);
int init_functionB_FLOAT(float *input, int length);
int functionB_FLOAT(float *input, int lengths, float *output);

int buffer(int buffer_size,unsigned int new_value, int *buffer);
int mod_4QAM(char *bits, int numbits, _Complex float *symbols);
int mod_16QAM (char *bits, int numbits, _Complex float *symbols);
void byte2bitsLSBF(char value, char **bits, int nof_bits);
void Byte2bit(char *in, char *out, int rcv_samples);
int hard_demod_16QAM (char * bits, _Complex float * symbols, int numsymb);
void bits2byteLSBF(char *byte, char **bits, int nof_bits);
int mod_BPSK(char *bits, int numbits, _Complex float *symbols);



#endif
