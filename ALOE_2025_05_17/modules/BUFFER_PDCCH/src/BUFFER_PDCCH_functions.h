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

int update_buffer(int desalineacio,_Complex float* inBuff, int in_length,_Complex float* out);
int DO_Buffer(_Complex float* in1, _Complex float* in2, _Complex float* out,int rcv_samp1, int rcv_samp2,int desalineacio);
int Concatenate_DCIs(_Complex float* in1, _Complex float* in2, _Complex float* out,int rcv_samp1, int rcv_samp2);


#endif
