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

int FFT_length_calc(int rcv_samples,int numFFT);

void ADD_CP_2_symbol(_Complex float* input,_Complex float* output, int data_per_symbol,int samp_2_cpy,int rcv_samples);
int Calc_Samp2Cpy(int OFDMsymb,int rcv_samples,int numFFT);
void ADD_cyclic_Prefix(_Complex float* input,_Complex float* output,int rcv_samples,int numFFT);



void REMOVE_CP_2_symbol(_Complex float* input,_Complex float* output,int data_per_symbol,int samp_2_cpy,int rcv_samples);
int Calc_Samp2Cpy_RX(int OFDMsymb,int rcv_samples,int numFFT);
void REMOVE_cyclic_Prefix(_Complex float* input,_Complex float* output,int rcv_samples,int numFFT);

int Calc_distance_CP(int pMAX1,int pMAX2,int rcv_samples);
int GetFFT_size(int distance);












#endif




















