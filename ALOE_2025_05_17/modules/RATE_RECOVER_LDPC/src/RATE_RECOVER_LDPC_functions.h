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

void getCBSParams(double B, double nbg, double *info);
void h5gRateRecoverLDPC(float *in, float *output, int *output_len, double *CB_num, double *CB_len, int rcv_samples, double trblklen, double rv, double NBG, double modulation, double Nlayers);
void cbsRateRecover(int C, int Kd, int K, int N, int ZC, float in[], float *out, int k0, int Ncb, int modulation, double E);
void reshape_2(float *output1, float *output2, float input[], int lon_input, int lon_output);

#endif
