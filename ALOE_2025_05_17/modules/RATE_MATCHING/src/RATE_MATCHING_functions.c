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
#include "RATE_MATCHING_functions.h"

/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */

/* Rate-matching: includes stages of bit selection, interleaving defined for LDPC, and code block concatenation
Subclauses: 5.4.2 y 5.5 de TS 38.212
*/

void ratematchfunction(char *in, char *out, int rcv_samples, int rv, int NBG, int modulation, int nlayers, int TBSLBRM, int num_codeblocks) {

	/*Suponiendo que todos los codeblock son del mismo tamaño, extraemos la longitud de cada codeblock a partir de los rcv_samples y 
	el número de codeblocks en los cuáles se había dividido la información*/
	int long_codeblock=rcv_samples/num_codeblocks;
	
	// Imprime los valores del array
    /*for (int i = 0; i < 50; i++) {
        printf("%d\n", in[i]);
    }*/

	/*A partir del array que nos llega, convertimos este array en una matriz, donde cada codeblock de longitud long_codeblock será una columna*/
	char matrix[long_codeblock][num_codeblocks];
	for (int i=0; i<long_codeblock; i++) {
		for (int j=0; j<num_codeblocks; j++) {
			matrix[i][j]=*(in+i+j*long_codeblock);
		}	
	}

	//printf("%d\n", matrix[0][0]);
	//printf("%d\n", matrix[0][1]);
	
	//Suponiendo que todos los codeblocks son del mismo tamaño extraemos el 'code block soft buffer size'
	int N=long_codeblock;	//Cantidad de informacion en cada code block
	//printf("%d\n", N);
	int C=num_codeblocks;	//Cantidad de codeblocks que llegan
	//printf("%d\n", C);

	int Ncb;
	if (TBSLBRM!=0) {		//Esta opción no está programada (opcional)
		int Nref=floor(3*TBSLBRM/4);
		if (Nref>N) {
			Ncb=N;
		}
		else {
			Ncb=Nref;
		}	
	}
	else { 					//Esta opción es la única programada, TBSLBRM=0
		Ncb=N;
	}
	//printf("Ncb=%d\n", Ncb);

	//Get starting position in circular buffer
	int Zc;
	int k0;
	if (NBG==1) {
		Zc=N/66;
		if (rv==0) {
			k0=0;
		}
		else if (rv==1) {
			k0=floor(17*Ncb/(66*Zc))*Zc;
			//printf("%d\n", k0);
		}
		else if (rv==2) {
			k0=floor(33*Ncb/(66*Zc))*Zc;
		}
		else if (rv==3) {
			k0=floor(56*Ncb/(66*Zc))*Zc;
		}
	}
	else {
		Zc=N/50;
		if (rv==0) {
			k0=0;
		}
		else if (rv==1) {
			k0=floor(13*Ncb/(50*Zc))*Zc;
		}
		else if (rv==2) {
			k0=floor(25*Ncb/(50*Zc))*Zc;
		}
		else if (rv==3) {
			k0=floor(43*Ncb/(50*Zc))*Zc;
		}
	}

	//Validate the input data size
	int Ncw;
	if (floor(Zc) != Zc) {
		if (NBG==1) {
			Ncw=66;
		}
		else {
			Ncw=50;
		}
	}

	/*Get rate matching output for all the code blocks and perform code block
	concatenation according to 38.212 5.4.2 and 5.5*/
	int index=0;
	for (int r=0; r<C; r++) {
		int E;
		if (r<=(C-fmod(rcv_samples/(nlayers*modulation),C)-1)) {
			E=nlayers*modulation*floor((rcv_samples/(nlayers*modulation*C)));
		}
		else {
			E=nlayers*modulation*ceil((rcv_samples/(nlayers*modulation*C)));
		}
		//printf("E=%d\n", E);
		char e[E];

		//printf("%d\n", N);
		char codeblock_array[N];	
		for (int i=0; i<N; i++) {
			codeblock_array[i]=matrix[i][r];
			//printf("%d ", matrix[i][r]);
		}

		/*for (int i = 0; i < N; i++) {
            printf("%d\n", codeblock_array[i]);
        }*/
		//printf("%d\n", codeblock_array[0]);

		char f[E];
		cbsRateMatch(codeblock_array, E, k0, Ncb, modulation, f);

		/*if (r==1) {
    		for (int i = 0; i < 3300; i++) {
                printf("%d ", f[i]);
            }
		}*/
		
		//Perform concatenation
		for (int i=0; i<E; i++) {
			*(out+index)=f[i];
			//printf("%d ",f[i]);
			index++;
			//printf("%d\n",index);
		}
	}
}

void cbsRateMatch(char *d, int E, int k0, int Ncb, int modulation, char *f) {
	
	//printf("%d",E);
	//Get the rate matching output bit sequence e_k, k=1,2,3,4...E-1
	int k=0;
	int j=0;
	char e[E];
	int i=0;

	//inicializar el vector "e" con valores de cero
	for (int i=0; i<E; i++) {
		e[i]=0;
	}

	while (k<E) {
	    //printf("%d\n",d[(k0+j)%Ncb]);
		if (d[((k0+j)%Ncb)] != -1) {	//NULL
			e[k]=d[((k0+j)%Ncb)];
			//printf("%d\n",e[k]);
			k++;
		}
		j++;
	}

	/*for (int i=0; i<E; i++) {
	    printf("%d\n",e[i]);
	}*/
	//printf("%d\n",e[2600]);
	//printf("%d\n",e[1457]);

	//Perform bit interleaving
	for (int j=0; j<(E/modulation); j++) {
		for (int i=0; i<(modulation); i++) {
			*(f+i+j*modulation)=e[i*E/(modulation+j)];
		}
	}

	//printf("%d\n",f[1457]);
	
	/*int f_rows=E/modulation;
	int f_cols=modulation;
	//printf("%d\n",f_cols);
	
	// Realiza el reshape del vector "f"
    int* f_reshaped = malloc(f_rows * f_cols * sizeof(int));
    for (int i = 0; i < f_rows; i++) {
        for (int j = 0; j < f_cols; j++) {
            f_reshaped[i*f_cols + j] = f[i*f_cols + j];
        }
    }
    printf("%d\n",f_reshaped[3566]);
    
    // Realiza la traspuesta del vector "f"
    int* f_transposed = malloc(f_rows * f_cols * sizeof(int));
    for (int i = 0; i < f_rows; i++) {
        for (int j = 0; j < f_cols; j++) {
            f_transposed[i*f_cols + j] = f_reshaped[j*f_rows + i];
        }
    }
    
    // Aplana el vector "f"
    int* f_unrolled = malloc(E * sizeof(int));
    for (int i = 0; i < E; i++) {
        f_unrolled[i] = f_transposed[i];
    }
    
    //printf("%d\n",f_unrolled[2]);
    
    // Libera la memoria asignada a los vectores temporales
    free(f_reshaped);
    free(f_transposed);
    free(f_unrolled);
    */
}
