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



#ifndef _MOD_NOISE_H
#define _MOD_NOISE_H

#include "CHANNEL_SUI_REPORT_interfaces.h"


#define NOISELENGTH INPUT_MAX_DATA



float get_variance(float snr_db,float scale);
float aver_power(_Complex float *in, int length);
float rand_gauss (void);
void gen_noise_c(_Complex float *x, float variance, int len);
int noise_SNR(_Complex float *inp, _Complex float *out, float SNR, float gain, int rcv_samples);









#endif
