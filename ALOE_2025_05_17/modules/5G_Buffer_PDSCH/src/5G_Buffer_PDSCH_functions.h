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

#define Buffer_Size 80*4096
#define rcv_samp_Buffer_size 10


int GET_from_buffer_rcv_sampl(int* Buff);
void Add_to_buffer_rcv_sampl(int rcv_samp_in,int* Buff);
void GET_from_buffer(_Complex float* out,_Complex float* Buff,int snd_samples);
void Add_to_buffer(_Complex float* in,_Complex float* Buff,int rcv_samples);


#endif
