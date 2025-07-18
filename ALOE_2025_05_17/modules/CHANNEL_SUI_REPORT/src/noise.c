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
 * along with ALOE.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "noise.h"

float get_variance(float snr_db,float scale) {
	return sqrt(pow(10,-snr_db/10)*scale);
}

float aver_power(_Complex float *in, int length){
	int i;
	_Complex float result=0.0+0.0i;
	float aver_power=0.0;

	for(i=0; i<length; i++){
		aver_power += __real__ (*(in+i)*conjf((*(in+i))));
	}
	aver_power=aver_power/(float)length;
	return(aver_power);
}

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
float rand_gauss (void) {
  float v1,v2,s;

  do {
    v1 = 2.0 * ((float) rand()/RAND_MAX) - 1;
    v2 = 2.0 * ((float) rand()/RAND_MAX) - 1;

    s = v1*v1 + v2*v2;
  } while ( s >= 1.0 );

  if (s == 0.0)
    return 0.0;
  else
    return (v1*sqrt(-2.0 * log(s) / s));
}

void gen_noise_c(_Complex float *x, float variance, int len) {
	int i;
	for (i=0;i<len;i++) {
		__real__ x[i] = rand_gauss();
		__imag__ x[i] = rand_gauss();
		x[i] *= variance;
	}
}


int noise_SNR(_Complex float *inp, _Complex float *out, float SNR, float gain, int rcv_samples){

	int i;	
	float power_in, power_noise;
	_Complex float noise[NOISELENGTH];

	// Calculate average power of input data
	power_in=aver_power(inp, rcv_samples);
	// Generate Noise
	gen_noise_c(noise, 1.0, rcv_samples);
	// Calculate power generated noise
	power_noise=aver_power(noise, rcv_samples);
	// Correct noise level according expected SNR. SNR=Psignal/(Gain*Pnoise)
	gain=power_in/(SNR*power_noise);
	gain=sqrt(gain);
	// Add Noise
	for(i=0; i<rcv_samples; i++){
		*(out+i)=*(inp+i)+gain*(*(noise+i));
	}
	return(rcv_samples);
}

