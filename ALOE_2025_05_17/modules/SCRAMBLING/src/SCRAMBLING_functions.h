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

#define MAXSEQLENGTH	2048*50
void create_scrambling_sequence_BITS(int RNTI, int CELLID, int sequencelength, char *scrambseq);
void create_scrambling_sequence_Oriol(int RNTI, int CELLID, int sequencelength, char *scrambseq,int q);
void scrambling(char *in, char *out, int rcv_samples, char *scrambseq);
void descrambling(char *in, char *out, int rcv_samples, char *scrambseq);
void bits2byteLSBF(char *byte, char **bits, int nof_bits);
void byte2bitsLSBF(char value, char **bits, int nof_bits);

void descrambling_Floats(float *in, float *out, int rcv_samples, float *scrambseq);
void create_scrambling_sequence_FLOATS(int RNTI, int CELLID, int sequencelength, float *scrambseq,int q);
#endif
