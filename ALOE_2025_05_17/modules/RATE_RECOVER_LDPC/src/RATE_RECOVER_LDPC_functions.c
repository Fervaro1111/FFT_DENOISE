/* 
 * Copyright (c) 2012
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

/* Functions that generate the test data fed into the DSP modules being developed */
#include <complex.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <phal_sw_api.h>
#include "skeleton.h"
#include "RATE_RECOVER_LDPC_functions.h"

//#define MAXCB_KCBg2 3840
//#define MAXCB_KCBg1 8448
#define MAXCB_KCBg1 1152
#define MAXCB_KCBg2 1152

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */

//A continuación encontraremos todas las funciones desarrolladas para el módulo del Rate Recovering

void h5gRateRecoverLDPC(float *in, float *output, int *output_len, double *CB_num, double *CB_len, int rcv_samples, double trblklen, double rv, double NBG, double modulation, double Nlayers){

	

	
	
	//int cbsbuffers[0];
	
	float in_loc[rcv_samples];

	//printf("Los valores del in_loc:");
	for(int i=0;i<rcv_samples;i++){
		in_loc[i] = in[i];
		//printf("%f ", in[i]);
	}
	//printf("\n");

	int G = sizeof(in_loc) / sizeof(float);

	//printf("El tamaño de la entrada es de %d \n", G);

	double B;

	//Get transport block size after CRC attachement according to 38.212 6.2.1 and 7.2.1

	//if(trblklen > MAXCB_KCBg2-16) B = trblklen + 24;
	//else B = trblklen + 16;

	B=trblklen;

	double chsinfo[5];

	//Get code block segementation parameters
	getCBSParams(B,NBG,chsinfo);
	int C = chsinfo[0];
	double Kd = chsinfo[1];
	double K = chsinfo[2];
	int N = chsinfo[3];
	double zc = chsinfo[4];


	//Get code block soft buffer size, asumiremos siempre que Ncb = N
	double Ncb = N; 
	//printf("Ncb:%d\n",Ncb);

	(*CB_len) = N;
	(*CB_num) = C;
	printf("CB num=%d  CB length =%d\n",N,C);

	// Get starting position in circular buffer
	double k0;

	if (NBG == 1) {
		if (rv == 0) {
			k0 = 0;
		} else if (rv == 1) {
			k0 = floor(17*Ncb/(66*zc))*zc;
		} else if (rv == 2) {
			k0 = floor(33*Ncb/(66*zc))*zc;
		} else if (rv == 3) {
			k0 = floor(56*Ncb/(66*zc))*zc;
		}
	}
	else {
		if (rv == 0) {
			k0 = 0;
		} else if (rv == 1) {
			k0 = floor(13*Ncb/(50*zc))*zc;
		} else if (rv == 2) {
			k0 = floor(25*Ncb/(50*zc))*zc;
		} else if (rv == 3) {
			k0 = floor(43*Ncb/(50*zc))*zc;
		}
	}

	int gIdx = 1;

	float out[C][N];  // Declaramos la matriz con C code blocks, de longitud N.

	for (int i = 0; i < C; i++) {
		for (int j = 0; j < N; j++) {
			out[i][j] = 0;
		}
	}

	int E;
	int dec;
	float vector_intermedio[N];

	double C_d = C;

	for (int r = 0; r < C; r++) {

		double G_d = (double)G;

		double k = G_d / (Nlayers * modulation);

		double resto = C_d-fmod(k, C_d)-1;

		if (r <= resto) {
			E = Nlayers * modulation * floor(G_d / (Nlayers * modulation * C_d));
		} else {
			E = Nlayers * modulation * ceil(G_d / (Nlayers * modulation * C_d));
		}
		printf("E:%d\n",E);

		int lower = gIdx-1;
		int upper = gIdx+E-1;

		/*
		printf("El valor del lower es: %d \n", lower);
		printf("El valor del upper es: %d \n", upper);
		*/

		//Creamos el vector deconcatenated
		float deconcatenated[E];

		dec = 0;
		for (int x = lower; x < upper; x++) {
			deconcatenated[dec] = in_loc[x];
			dec += 1;
		}
		
		//int printeo = sizeof(deconcatenated) / sizeof(float);
		/*printf("DECONCATENATED .....\n");		
		for(int i=0; i<printeo; i++){
		    printf("%f", deconcatenated[i]);
		}
		*/printf("\n");
		/*
		int printeo = sizeof(deconcatenated) / sizeof(int);

		printf("La longitud de deconcatenated es: %d \n", printeo);

		for(int i=0; i<printeo; i++){
		    printf("%d", deconcatenated[i]);
		}

		printf("\n");*/

		gIdx += E;

		cbsRateRecover(C, Kd, K, N, zc, deconcatenated, vector_intermedio, k0, Ncb, modulation, (double)E);

		int long_calc_cbs = sizeof(vector_intermedio)/sizeof(float);
		
		for (int j = 0; j < N; j++) {
			out[r][j] = vector_intermedio[j];
		}
	

		/*printf("VECT INTERMEDIO .....\n");		
		for(int i=0; i<long_calc_cbs; i++){
		    printf("%f", vector_intermedio[i]);
		}
		printf("\n");
		*/

		/*for(int i=0; i<long_calc_cbs; i++){
		    printf("%f ", vector_intermedio[i]);
		}*/
	}
	//Metemos todos los codeblocks en un gran vector, para luego separarlo fuera de la función
	(*output_len) = (*CB_num)*(*CB_len);
	int output_i = 0;
	for(int r = 0; r < C; r++){
		for (int j = 0; j < N; j++) {
			output[output_i] = out[r][j];
			output_i += 1;
		}
	}
	/*
	for (int i = 0; i < 10; i++) {
	printf("%d", output[i]);
	}
	for (int i = N; i < N+10; i++) {
	printf("%d", output[i]);
	}--> Comprovacion hecha, está haciendo correctamente los cambios que tocan */ 
}

//***********************************************************************************************
//***********************************************************************************************
//***********************************************************************************************

//Rate recovery for a single code block segment
void cbsRateRecover(int C, int Kd, int K, int N, int ZC, float in[], float *out, int k0, int Ncb, int modulation, double E){

	int E_i = E;
	float in_loc[E_i];

	//printf("Los valores del in_loc:");
	for(int i=0;i<E_i;i++){
		in_loc[i] = in[i];
		//printf("%f", in[i]);
	}

	/*//Recorremos el in a ver qué hay:

	int in_len = sizeof(in_loc) / sizeof(int);

	printf("La longitud del in es: %d \n", in_len);

	for(int i=0; i<in_len; i++){
	printf("%d", in[i]);
	}

	printf("\n");*/

	//Definimos el vector que lleva todas las muestras:
	double Qm = modulation;
	int longitud_salida = E;
	float output1[longitud_salida];
	float output2[longitud_salida];
	float result[(int)E];

	//printf("El tamaño de los vectores generados es de %d \n", longitud_salida);
/*
	if(Qm == 2){ 
		reshape_2(output1, output2, in, E, longitud_salida);
		for (int i = 0; i < Qm*longitud_salida; i++) {
			//Printeamos los resultados:
			printf("%d ", output1[i]);
			printf("%d ", output2[i]);
			printf("\n");
		}
*/
		//printf("El vector que va en cada CodeBlock es: "); //--> están genial
		//Ahora concatenamos los vectores transpuestos:
		
/*for (int i = 0; i < E; i++) {
			if(i<longitud_salida) result[i] = output1[i];
			else result[i] = output2[i-longitud_salida];
			//Printeamos los resultados:
			//printf("%d ", result[i]);
		}
		//printf("\n");
	}
*/
	//Recorremos el result a ver qué hay:

	/*int result_len = sizeof(result) / sizeof(float);

	printf("La longitud del result es: %d \n", result_len);

	for(int i=0; i<result_len; i++){
	printf("%d", result[i]);
	}

	printf("\n");*/

	//Pruncture systematic bits
	//K = K - 2*ZC;
	//Kd = Kd - 2*ZC;

	//printf("Printeamos Kd: %d y K: %d \n", Kd, K);

	// Perform reverse of bit selection according to 38.212 5.4.2.1
	int k = 0; 
	int j = 0;
	int indices[E_i];

	for (int i = 0; i < E; i++) {
		indices[i] = 0;
	}

	int idx = 0;

	while (k < E) {
		idx = (k0+j) % Ncb;
		if (!(idx >= Kd && idx < K)) { // Avoid <NULL> filler bits
			indices[k] = idx;
			k++;
		}
		j++;
	}

	
	//Echamos un vistazo al array indices:

	int result_ind = sizeof(indices) / sizeof(int);

	printf("La longitud del indices es: %d \n", result_ind);
/*
	for(int i=0; i<E_i; i++){
	printf("%f", indices[i]);
	}

	printf("\n");
*/
	float out_loc[N];

	//Recover circular buffer
	for (int i=0; i<N; i++) {
		out_loc[i] = 0;
	}

	//Filler bits are treated as 0 bits when perform encoding, 0 bits
	//corresponding to Inf in received soft bits, this step improves
	//error-correction performance in the LDPC decoder

	/*for (int i = Kd + 1; i < K; i++) {
		out_loc[i] = INFINITY;
	}
*/
	int index_loop = 0;

	//printf("Printeamos los valores del out_loc: ");
	for(int n=0; n<E; n++){
		index_loop = indices[n];
		out_loc[index_loop] = out_loc[index_loop] + in[n];
		//printf("%f", out_loc[index_loop]);
	}
	//printf("\n");

	memcpy(out,out_loc,sizeof(out_loc));

	//Recorremos el out a ver qué hay: --> genial

	/*for(int i=0; i<E; i++){
	printf("%f", out[i]);
	}

	printf("\n");
*/
}

//***********************************************************************************************
//***********************************************************************************************
//***********************************************************************************************

void getCBSParams(double B, double nbg, double *info){  
	double Kcb, L, C, Bd, cbz, Kd, Kb, Zc;
	int Zlist[51] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18, 20, 22, 24, 26, 28, 30, 32, 36, 40, 44, 48, 52, 56, 60, 64, 72, 80, 88, 96, 104, 112, 120, 128, 144, 160, 176, 192, 208, 224, 240, 256, 288, 320, 352, 384};

	double vect[5];    

	// Get the maximum code block size
	if(nbg == 1) Kcb = MAXCB_KCBg1;
	else Kcb = MAXCB_KCBg2;

	// Get number of code blocks and length of CB-CRC coded block
	if (B <= Kcb){
		C = 1;
		Bd = B;
	}
	else{
		L = 24; // Length of the CRC bits attached to each code block
		C = ceil(B/(Kcb-L));
		Bd = B+C*L;
	}

	// Obtain the number of bits per code block (excluding CB-CRC bits)
	cbz = B/C;

	// Get number of bits in each code block (excluding Filler bits)
	Kd = Bd/C;

	//Find the minimum value of Z in all sets of lifting sizes in Table
	//5.3.2-1, denoted as Zc, such that Kb*Zc>=Kd
	if (nbg == 1) Kb = 22;
	else {
		if (B > 640) {
			Kb = 10;
		} else if (B > 560) {
			Kb = 9;
		} else if (B > 192) {
			Kb = 8;
		} else {
			Kb = 6;
		}
	}
	for (int i = 0; i < 51; i++) {
		Zc = Zlist[i];
		if (Kb*Zc >= Kd) {
		break;
		}
	}

	// Get number of bits (including Filler bits) to be encoded by LDPC encoder
	double K, N;
	if (nbg == 1) {
		K = 22 * Zc;
		N = 66 * Zc;
	} else {
		K = 10 * Zc;
		N = 50 * Zc;
	}

	vect[0] = C;         // Number of code block segements
	vect[1] = Kd;       // Number of bits in each code block (may include CB-CRC bits but excluding Filler bits)
	vect[2] = K;         // Number of bits in each code block (including Filler bits)
	vect[3] = N;         // Circular buffer length (with 2*Zc bits punctured)
	vect[4] = Zc;       // Selected lifting size

	printf("IN FUNCTION RATE RECOVER ->  Num_codeblocs=%f, data+allCRC=%f,CB size=%f,lifting size=%f",C,Kd,K,Zc);


	memcpy(info,vect,sizeof(vect));
}
//
//***********************************************************************************************
//***********************************************************************************************
//***********************************************************************************************

// Reshape plus transpose para Qm (modulation) de 2 (QPSK)
void reshape_2(float *output1, float *output2, float input[], int lon_input, int lon_output){
	// Copy the data from the old array to the new array
	int j = 0;
	for (int i = 0; i < lon_output; i++) {
		//printf("El valor de input es de: %d \n", input[j+1]);
		output1[i] = input[j];
		output2[i] = input[j+1];
		j = j + 2;
	}
}
