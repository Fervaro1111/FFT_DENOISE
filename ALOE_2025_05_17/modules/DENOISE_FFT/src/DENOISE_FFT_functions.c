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
#include "DENOISE_FFT_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <fftw3.h>

#include "DENOISE_FFT_functions.h"

// Definición de PI si no está ya definida
#ifndef M_PI
#define M_PI 3.1415926535897932384
#endif

// ------------------- UTILIDADES INTERNAS -------------------

// Calcula el valor RMS (Root Mean Square) de un vector complejo X de N puntos
static double rms(const fftw_complex *X, size_t N){
    double acc=0.0; 
    for(size_t k=0; k<N; ++k) 
        acc += pow(cabs(X[k]),2.0);  // Suma el cuadrado del módulo
    return sqrt(acc/N);              // Raíz cuadrada del promedio
}

// Calcula la mediana de las magnitudes de X (para estadísticas robustas)
static double median_mag(const fftw_complex *X, size_t N){
    double *tmp = malloc(N * sizeof(double));
    for(size_t k=0; k<N; ++k) 
        tmp[k] = cabs(X[k]);     // Llena el array con módulos
    size_t m = N/2;              // Índice central (mediana)
    size_t l = 0, r = N-1; 
    double med;
    // Algoritmo quickselect para hallar la mediana sin ordenar todo
    while(1){
        double piv=tmp[m]; size_t i=l, j=r;
        do{
            while(tmp[i]<piv) ++i;
            while(tmp[j]>piv) --j;
            if(i<=j){ double t=tmp[i]; tmp[i]=tmp[j]; tmp[j]=t; ++i; --j; }
        }while(i<=j);
        if(j<m) l=i; 
        if(i>m) r=j;
        if(l>=r){ med=tmp[m]; break; }
    }
    free(tmp); 
    return med;  // Devuelve la mediana
}

// Ganancia de Wiener para filtrado adaptativo (opcional)
static inline double wiener_gain(double mag2, double N0){
    return mag2/(mag2+N0);   // mag2=|X[k]|^2, N0=potencia de ruido
}

// =================== 0) FILTRO LOW PASS ===================
void low_pass_filter(fftw_complex *X, size_t N, float fc_norm, float tb_norm, int dbg)
{
    // fc_norm: frecuencia de corte (normalizada, 0…0.5)
    // tb_norm: transición de banda (ancho normalizado)
    if(fc_norm<=0.f || fc_norm>=0.5f || tb_norm<=0.f){ 
        return; // Chequeo de parámetros
    }

    const double fc = fc_norm;
    const double tb = tb_norm;
    const double invN = 1.0/(double)N;
    const double N0 = pow(median_mag(X,N),2.0);  // Estima el nivel de ruido

    if(dbg>=2)
        fprintf(stderr,"[LPF] fc=%.3f  tb=%.3f  N0=%.3e\n",
                fc, tb, N0);

    // Bucle por cada bin de frecuencia
    for(size_t k=0; k<N; ++k){
        size_t kpos = (k<=N/2) ? k : (N-k); // Simetría espectral
        double f = kpos * invN;             // Frecuencia normalizada (0…0.5)
        double H;
        // Forma de ventana tipo raised-cosine:
        if(f<=fc)        H = 1.0;                                 // Paso-bajo ideal
        else if(f<fc+tb) H = 0.5*(1.0+cos(M_PI*(f-fc)/tb));       // Transición suave
        else             H = 0.0;                                 // Banda suprimida

        double G = H;
        if(H>0.0){
            double mag2 = pow(cabs(X[k]),2.0);
            G *= wiener_gain(mag2,N0);    // Filtrado adaptativo: menor ganancia donde hay menos señal
        }
        if(dbg>=3 && (k==0 || k==N/2))
            fprintf(stderr,"[LPF]  k=%zu  |X|=%.2e  G=%.2f\n",
                    k, cabs(X[k]), G);
        X[k] *= G;    // Aplica la ganancia final
    }
}

// ================ 1) FILTRO AMPLITUDE-THRESHOLD ===============
void amplitude_threshold(fftw_complex *X, size_t N,
                         float threshold,
                         int dbg)
{
    // Encuentra el máximo módulo del espectro
    double maxMag=0.0;
    for(size_t k=0; k<N; ++k){
        double m = cabs(X[k]); 
        if(m > maxMag) maxMag = m;
    }
    if(maxMag==0.0) return;   // Si la señal es toda cero, sale

    // Cálculo del umbral:
    // Si threshold >= 1.0 → interpreta como dB por debajo del máximo
    // Si threshold < 1.0  → interpreta como fracción lineal del máximo
    double thr_lin = (threshold >= 1.0f) ? maxMag * pow(10.0, -threshold/20.0) : maxMag * threshold;
    double thr2 = thr_lin * 2.0; // Zona de transición

    if(dbg>=2)
        fprintf(stderr,"[THR] max|X|=%.3e  thr=%.3e\n", maxMag, thr_lin);

    // Filtrado coeficiente a coeficiente
    for(size_t k=0; k<N; ++k){
        double mag = cabs(X[k]);
        if(mag<=thr_lin){
            X[k] = 0.0 + 0.0*I;  // Anula los débiles
        }else if(mag < thr2){
            double a = (mag-thr_lin)/(thr2-thr_lin);             // Progresión en la ventana de transición
            double w = 0.5*(1.0+cos(M_PI*(1.0-a)));              // Ventana cónica
            X[k] *= w;                                          // Atenúa suavemente
        }
        if(dbg>=3 && (k==0 || k==N/2))
            fprintf(stderr,"[THR] k=%zu  mag=%.2e → %.2e\n",k, mag, cabs(X[k]));
    }
}

// ================ 2) FILTRO “EMD-lite” ========================
void emd_filter(fftw_complex *X, size_t N, int dbg)
{
    // Calcula la potencia RMS y el nivel de ruido estimado (mediana)
    double p_rms   = rms(X,N);
    double p_noise = median_mag(X,N);

    if(dbg>=2)
        fprintf(stderr,"[EMD] rms=%.3e  med=%.3e\n", p_rms, p_noise);

    // Si la SNR es alta (señal >> ruido), NO se filtra (bypass)
    if(p_rms > 2.0*p_noise){
        if(dbg>=2) fprintf(stderr,"[EMD] bypass\n");
        return;
    }
    // Si no, aplica un low-pass filter adaptativo
    low_pass_filter(X, N, 0.45f, 0.05f, dbg);
}

