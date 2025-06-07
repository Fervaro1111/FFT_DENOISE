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

#define K_max 164

typedef struct{
	//int Car_agr_lvl; //Carrier agregation level it can be 1,2,4,8 and 16
	int I_il; //1 for Interleaving and 0 to don't interleave;
}Interl_t;

//int calc_out_bits_max_length(Interl_t* Interleave_info);
void do_interleave(char* in, char* out,Interl_t* Interleave_info,int rcv_samples_bits);
void undo_interleave(char* in, char* out,Interl_t* Interleave_info,int rcv_samples_bits);
void Generate_Interl_Pattern(int* pattern,int rcv_samples_bits,Interl_t* Interleave_info);
void Interleave(char* in, char* out,int* pattern,int rcv_samples_bits);
void De_Interleave(char* in, char* out,int* pattern,int rcv_samples_bits);
void byte2bitsLSBF(char value, char **bits, int nof_bits);
void bits2byteLSBF(char *byte, char **bits, int nof_bits);



#endif
