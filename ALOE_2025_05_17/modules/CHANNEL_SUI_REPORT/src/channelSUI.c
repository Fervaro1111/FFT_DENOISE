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
#include "channelSUI.h"


extern int taps[];
extern int channelLength;

float max(float *vec,int lenght)		//calcula el maxim d'un vector
{	
	int i;
	float maximo=vec[0];

	for(i=0;i<lenght;i++)
	{
		if(vec[i]>maximo)
			maximo=vec[i];
	}
	return maximo;
}


void initSuiParameters(int nSui)				//inicialitza els valors dels parametres del model SUI a partir del nombre
{	
	switch(nSui)
	{
		case 1:
			P[0] = 0.0;
			P[1] = -15.0;
			P[2] = -20.0;
			K[0] = 4.0;
			K[1] = 0.0;	
			K[2] = 0.0;	
			Dop[0] = (float)0.4;
			Dop[1] = (float)0.3;
			Dop[2] = (float)0.5;		
			tau[0] = (float)0.0;
			tau[1] = (float)0.4;
			tau[2] = (float)0.9;
			Fnorm = (float)-0.1771;
			break;

		case 2:
			P[0] = 0.0;
			P[1] = -12.0;
			P[2] = -15.0;	 	
			K[0] = 2.0;
			K[1] = 0.0;	
			K[2] = 0.0;	
			Dop[0] = (float)0.2;
			Dop[1] = (float)0.15;
			Dop[2] = (float)0.25;
			tau[0] = (float)0.0;
			tau[1] = (float)0.4;
			tau[2] = (float)1.1;
			Fnorm = (float)-0.3930;	
			break;

		case 3:
			P[0]=0.0;
			P[1]=-5.0;
			P[2]=-10.0;
			K[0]=1.0;
			K[1]=0.0;
			K[2]=0.0;
			Dop[0]=(float)0.4;
			Dop[1]=(float)0.4;
			Dop[2]=(float)0.4;
			tau[0] = (float)0.0;
			tau[1] = (float)0.4;
			tau[2] = (float)0.9;
			Fnorm=(float)-1.5113;
			break;

		case 4:
			P[0] = 0.0;
			P[1] = -4.0;
			P[2] = -8.0;	 	
			K[0] = 0.0;
			K[1] = 0.0;	
			K[2] = 0.0;
			Dop[0] = (float)0.2;
			Dop[1] = (float)0.15;
			Dop[2] = (float)0.25;
			tau[0] = (float)0.0;
			tau[1] = (float)1.5;
			tau[2] = (float)4.0;
			Fnorm = (float)-1.9218;
			break;

		case 5:
			P[0] = 0.0;
			P[1] = -5.0;
			P[2] = -10.0;	 	
			K[0] = 0.0;
			K[1] = 0.0;	
			K[2] = 0.0;	
			Dop[0] = (float)2.0;
			Dop[1] = (float)1.5;
			Dop[2] = (float)2.5;
			tau[0] = (float)0.0;
			tau[1] = (float)4.0;
			tau[2] = (float)10.0;
			Fnorm = (float)-1.5113;
			break;

		case 6:
			P[0] = 0.0;
			P[1] = -10.0;
			P[2] = -14.0;	 	
			K[0] = 0.0;
			K[1] = 0.0;	
			K[2] = 0.0;	
			Dop[0] = (float)0.4;
			Dop[1] = (float)0.3;
			Dop[2] = (float)0.5;
			tau[0] = (float)0.0;
			tau[1] = (float)14.0;
			tau[2] = (float)20.0;
			Fnorm = (float)-0.5683;
			break;
	}
}


float interpolate(float value1,float value2,int samples,int number)		//interpola la posicio 'number' entre dos valors separats 'samples' mostres
{	
	return (value2-value1)/(float)samples*(float)number+value1;
}


void resample()						//interpola les mostres ajustant-les a la nostra symbol rate i les guarda en els vectors flowChannel
{
	int i,j;	
	
	posCount=0;
	suiCount=0;

	if(suiCount==0)printf("resamplingRate=%d\n", resamplingRate);

	for(j=0;j<FLOW_SIZEvar;j++)
	{
		for(i=0;i<TAPS;i++)
		{
			flowChannelReal[i][j]=interpolate(suiReal[i][suiCount],suiReal[i][suiCount+1],resamplingRate,posCount);
			flowChannelImag[i][j]=interpolate(suiImag[i][suiCount],suiImag[i][suiCount+1],resamplingRate,posCount);

			//actualitzem la posicio sobre la que volem fer el zoom i vigilem que no ens sortim de rang
			posCount++;
			if(posCount>=resamplingRate)
			{
				posCount=0;
				suiCount++;
				if(suiCount+1>=SUI_SAMPLES)
					suiCount=0;
			}
		}	
	}
}



/* INTERPOLACIO 5
_Complex float resample2(int NofCoeff, int N, int nofTAP){

	static float x, x0[TAPS], x1[TAPS], x2[TAPS], x3[TAPS], x4[TAPS], y, y0[TAPS], y1[TAPS], y2[TAPS], y3[TAPS], y4[TAPS];
	float A=1.0/(float)N;
	static int idx0, idx1, idx2, idx3, idx4;
	static int numCoeff[TAPS]={0, 0, 0};
	float T0, T1, T2, T3, T4;
	static int count=0, first=0;

	//numCoeff=NofCoeff/N;


//	if(count%((5*N)/2)==0){

	


	if(count%(N*2)==0){
		
		idx0=numCoeff[nofTAP]%SUI_SAMPLES;
		idx1=(numCoeff[nofTAP]+1)%SUI_SAMPLES;
		idx2=(numCoeff[nofTAP]+2)%SUI_SAMPLES;
		idx3=(numCoeff[nofTAP]+3)%SUI_SAMPLES;
		idx4=(numCoeff[nofTAP]+4)%SUI_SAMPLES;

		x0[nofTAP]=(float)idx0;
		x1[nofTAP]=(float)idx1;
		x2[nofTAP]=(float)idx2;
		x3[nofTAP]=(float)idx3;
		x4[nofTAP]=(float)idx4;

		y0[nofTAP]=suiReal[nofTAP][idx0];
		y1[nofTAP]=suiReal[nofTAP][idx1];
		y2[nofTAP]=suiReal[nofTAP][idx2];
		y3[nofTAP]=suiReal[nofTAP][idx3];
		y4[nofTAP]=suiReal[nofTAP][idx4];

		printf("numCoeff=%d: y0=%f, y1=%f, y2=%f, y3=%f, y4=%f\n", numCoeff[nofTAP], y0[nofTAP], y1[nofTAP], y2[nofTAP], y3[nofTAP], y4[nofTAP]);

		if(first==1)numCoeff[nofTAP]+=2;
		first=1;
	}




	
	x=A*(float)(count); //+A*(float)N; 
	count++;



	T0=((x-x1[nofTAP])*(x-x2[nofTAP])*(x-x3[nofTAP])*(x-x4[nofTAP])/((x0[nofTAP]-x1[nofTAP])*(x0[nofTAP]-x2[nofTAP])*(x0[nofTAP]-x3[nofTAP])*(x0[nofTAP]-x4[nofTAP])))*y0[nofTAP];

	T1=((x-x0[nofTAP])*(x-x2[nofTAP])*(x-x3[nofTAP])*(x-x4[nofTAP])/((x1[nofTAP]-x0[nofTAP])*(x1[nofTAP]-x2[nofTAP])*(x1[nofTAP]-x3[nofTAP])*(x1[nofTAP]-x4[nofTAP])))*y1[nofTAP];

	T2=((x-x0[nofTAP])*(x-x1[nofTAP])*(x-x3[nofTAP])*(x-x4[nofTAP])/((x2[nofTAP]-x0[nofTAP])*(x2[nofTAP]-x1[nofTAP])*(x2[nofTAP]-x3[nofTAP])*(x2[nofTAP]-x4[nofTAP])))*y2[nofTAP];

	T3=((x-x0[nofTAP])*(x-x1[nofTAP])*(x-x2[nofTAP])*(x-x4[nofTAP])/((x3[nofTAP]-x0[nofTAP])*(x3[nofTAP]-x1[nofTAP])*(x3[nofTAP]-x2[nofTAP])*(x3[nofTAP]-x4[nofTAP])))*y3[nofTAP];

	T4=((x-x0[nofTAP])*(x-x1[nofTAP])*(x-x2[nofTAP])*(x-x3[nofTAP])/((x4[nofTAP]-x0[nofTAP])*(x4[nofTAP]-x1[nofTAP])*(x4[nofTAP]-x2[nofTAP])*(x4[nofTAP]-x3[nofTAP])))*y4[nofTAP];

	y=T0+T1+T2+T3+T4;

	return(y+0.0*I);

}

*/
_Complex float resample2(int count, int N, int nofTAP){

	static float x, x0[TAPS], x1[TAPS], x2[TAPS], x3[TAPS], x4[TAPS], y, y0[TAPS], y1[TAPS], y2[TAPS], y3[TAPS], y4[TAPS];
	float A=1.0/(float)N;
	static int idx0, idx1, idx2, idx3, idx4;
	//static int numCoeff[TAPS]={0, 0, 0};
	static int numCoeff=0;
	float T0, T1, T2, T3, T4;
	static int first=0;

	//numCoeff=NofCoeff/N;


//	if(count%((5*N)/2)==0){

	


	if(count%(N*2)==0){
		//if(first==1)numCoeff[nofTAP]+=2;
		if(first==3 && nofTAP==0)numCoeff+=2;
		if(first<3)first++;
	}
		
	idx0=numCoeff%SUI_SAMPLES;
	idx1=(numCoeff+1)%SUI_SAMPLES;
	idx2=(numCoeff+2)%SUI_SAMPLES;
	idx3=(numCoeff+3)%SUI_SAMPLES;
	idx4=(numCoeff+4)%SUI_SAMPLES;

	x0[nofTAP]=(float)idx0;
	x1[nofTAP]=(float)idx1;
	x2[nofTAP]=(float)idx2;
	x3[nofTAP]=(float)idx3;
	x4[nofTAP]=(float)idx4;

	y0[nofTAP]=suiReal[nofTAP][idx0];
	y1[nofTAP]=suiReal[nofTAP][idx1];
	y2[nofTAP]=suiReal[nofTAP][idx2];
	y3[nofTAP]=suiReal[nofTAP][idx3];
	y4[nofTAP]=suiReal[nofTAP][idx4];

//	if(nofTAP==0)printf("numCoeff=%d nofTAP=%d: y0=%f, y1=%f, y2=%f, y3=%f, y4=%f\n", numCoeff, nofTAP, y0[nofTAP], y1[nofTAP], y2[nofTAP], y3[nofTAP], y4[nofTAP]);


	



	//x=A*(float)(count%N)+A*((float)((3*N)/2)); 
	
	x=A*(float)(count); //+A*(float)N; 
//	count++;
//	if(x>(float)(SUISAMPLES*N))count=0;


	/*T0=((x-x1)*(x-x2)/((x0-x1)*(x0-x2)))*y0;
	T1=((x-x0)*(x-x2)/((x1-x0)*(x1-x2)))*y1;
	T2=((x-x0)*(x-x1)/((x2-x0)*(x2-x1)))*y2;
*/


	T0=((x-x1[nofTAP])*(x-x2[nofTAP])*(x-x3[nofTAP])*(x-x4[nofTAP])/((x0[nofTAP]-x1[nofTAP])*(x0[nofTAP]-x2[nofTAP])*(x0[nofTAP]-x3[nofTAP])*(x0[nofTAP]-x4[nofTAP])))*y0[nofTAP];

	T1=((x-x0[nofTAP])*(x-x2[nofTAP])*(x-x3[nofTAP])*(x-x4[nofTAP])/((x1[nofTAP]-x0[nofTAP])*(x1[nofTAP]-x2[nofTAP])*(x1[nofTAP]-x3[nofTAP])*(x1[nofTAP]-x4[nofTAP])))*y1[nofTAP];

	T2=((x-x0[nofTAP])*(x-x1[nofTAP])*(x-x3[nofTAP])*(x-x4[nofTAP])/((x2[nofTAP]-x0[nofTAP])*(x2[nofTAP]-x1[nofTAP])*(x2[nofTAP]-x3[nofTAP])*(x2[nofTAP]-x4[nofTAP])))*y2[nofTAP];

	T3=((x-x0[nofTAP])*(x-x1[nofTAP])*(x-x2[nofTAP])*(x-x4[nofTAP])/((x3[nofTAP]-x0[nofTAP])*(x3[nofTAP]-x1[nofTAP])*(x3[nofTAP]-x2[nofTAP])*(x3[nofTAP]-x4[nofTAP])))*y3[nofTAP];

	T4=((x-x0[nofTAP])*(x-x1[nofTAP])*(x-x2[nofTAP])*(x-x3[nofTAP])/((x4[nofTAP]-x0[nofTAP])*(x4[nofTAP]-x1[nofTAP])*(x4[nofTAP]-x2[nofTAP])*(x4[nofTAP]-x3[nofTAP])))*y4[nofTAP];

	y=T0+T1+T2+T3+T4;

	return(y+0.0*I);

}


#define SIZEHISTO	1024

_Complex float resample3(int N, int nofTAP){

	_Complex float histo[SIZEHISTO], out=0.0+0.0*I;
	static int first=0, NofCoeff=0;
	int idx=0;
	int i, MN=N*5;

	if(first==0){
		memset(histo, 0, sizeof(_Complex float)*SIZEHISTO);
		first=1;
	}

	idx=NofCoeff/N;
	if(NofCoeff%N==0){
		histo[NofCoeff]=suiReal[nofTAP][idx%SUI_SAMPLES]+0.0*I;
	}else histo[NofCoeff]=0.0+0.0*I;
	NofCoeff++;

	for(i=0; i<MN; i++){
		out+=histo[i];
	}
	out=out/(float)(MN);

	if(NofCoeff>MN){
		for(i=1; i<MN; i++){
			histo[i-1]=histo[i];
		}
	}
	return(out);
}



float gauss (float mean, float sigma)	//calcula un nombre amb una probabilitat gaussiana de mitjana mean i desviacio sigma
{	
    /* This uses the fact that a Rayleigh-distributed random variable R, with
    the probability distribution F(R) = 0 if R < 0 and F(R) =
    1 - exp(-R^2/2*sigma^2) if R >= 0, is related to a pair of Gaussian
    variables C and D through the transformation C = R * cos(theta) and
    D = R * sin(theta), where theta is a uniformly distributed variable
    in the interval (0, 2*pi()). From Contemporary Communication Systems
    USING MATLAB(R), by John G. Proakis and Masoud Salehi, published by
    PWS Publishing Company, 1998, pp 49-50. This is a pretty good book. */
    
	double u, r;            /* uniform and Rayleigh random variables */
    /* generate a uniformly distributed random number u between 0 and 1 - 1E-6*/
    u = (double) rand() / RAND_MAX;
    if (u == 1.0) u = 0.999999999;
    /* generate a Rayleigh-distributed random number r using u */
    r = sigma * sqrt( 2.0 * log( 1.0 / (1.0 - u) ) );
    /* generate another uniformly-distributed random number u as before*/
    u = (double) rand() / RAND_MAX;
    if (u == 1.0) u = 0.999999999;
    /* generate and return a Gaussian-distributed random number using r and u */
    return( (float) ( mean + r * cos(2 * M_PI * u) ) );
}


void FFT(int dir,long m,float *x,float *y)		//extreta del modul FFT
{	
	/*This computes an in-place complex-to-complex FFT 
	x and y are the real and imaginary arrays of 2^m points.
	dir =  1 gives forward transform
	dir = -1 gives reverse transform */

   long n,i,i1,j,k,i2,l,l1,l2;
   double c1,c2,tx,ty,t1,t2,u1,u2,z;

   /* Calculate the number of points */
   n = 1;
   for (i=0;i<m;i++) 
      n *= 2;

   /* Do the bit reversal */
   i2 = n >> 1;
   j = 0;
   for (i=0;i<n-1;i++) {
      if (i < j) {
         tx = x[i];
         ty = y[i];
         x[i] = x[j];
         y[i] = y[j];
         x[j] = (float)tx;
         y[j] = (float)ty;
      }
      k = i2;
      while (k <= j) {
         j -= k;
         k >>= 1;
      }
      j += k;
   }

   /* Compute the FFT */
   c1 = -1.0; 
   c2 = 0.0;
   l2 = 1;
   for (l=0;l<m;l++) {
      l1 = l2;
      l2 <<= 1;
      u1 = 1.0; 
      u2 = 0.0;
      for (j=0;j<l1;j++) {
         for (i=j;i<n;i+=l2) {
            i1 = i + l1;
            t1 = u1 * x[i1] - u2 * y[i1];
            t2 = u1 * y[i1] + u2 * x[i1];
            x[i1] = x[i] - (float)t1; 
            y[i1] = y[i] - (float)t2;
            x[i] += (float)t1;
            y[i] += (float)t2;
         }
         z =  u1 * c1 - u2 * c2;
         u2 = u1 * c2 + u2 * c1;
         u1 = z;
      }
      c2 = sqrt((1.0 - c1) / 2.0);
      if (dir == 1) 
         c2 = -c2;
      c1 = sqrt((1.0 + c1) / 2.0);
   }

   /* Scaling for forward transform */
   if (dir == -1) {
      for (i=0;i<n;i++) {
         x[i] /= n;
         y[i] /= n;
      }
   }
}


void iFFTShift(float *x)				//intercambia els M/2 primers valors d'un vector per els M/2 del final
{	
	int i=0;
	float v[M];

	for(i=0;i<(M/2);i++)
	{		
		v[i]=x[i+(M/2)];
		v[i+(M/2)]=x[i];
	}
	for(i=0;i<M;i++)
	{
		x[i]=v[i];
	}
}


void FFTFilt(float *re,float *im, float *filt)		//aplica un filtre FFT a partir de dos vectors d'entrada en temps i un filtre en frequencia
{	
	float vecReal[SUI_SAMPLES+M-1],vecImag[SUI_SAMPLES+M-1];	
	int i,j;
	
	for(i=0;i<SUI_SAMPLES+M-1;i++)
	{
		vecReal[i]=(float)0;
		vecImag[i]=(float)0;
	}
		
	for(i=0;i<SUI_SAMPLES;i++)
	{
		for(j=0;j<M;j++)
		{
			vecReal[i+j]=filt[j]*re[i]+vecReal[i+j];
			vecImag[i+j]=filt[j]*im[i]+vecImag[i+j];
		}
	}

	for(i=0;i<SUI_SAMPLES;i++)
	{
		re[i]=vecReal[i+M/2];
		im[i]=vecImag[i+M/2];
	}
}


void initSui(int nSui, int symbolRate)		//inicialitzem el vector del canal a partir del nombre de SUI i la tasa de simbols que hi posarem
{
	int i,j;							
	float s[TAPS], m[TAPS];						//s es la varianca del senyal i m la part constant de la potencia
	float PSD[(M/2)+1],filtReal[M],filtImag[M];	//PSD es el vector amb la distribucio espectral de potencies i filtReal i filtImag els valors reals i imaginaris del filtre doppler
	float maxDop,sumFilt,D,aux;					//maxDop es el valor maxim del filtre doppler, sumFilt la suma de tots els valors del filtre, D el valor normalitzat del filtre doppler i aux un valor auxiliar

	//inicialitzem les variables al model SUI que volem calcular
	initSuiParameters(nSui);
	suiCount=0;
	posCount=0;

	//calculem caracteristiques generals del canal
	for(i=0;i<TAPS;i++)
	{  
		P[i]=(float)pow((float)10.0,(P[i]/10));		//passem la potencia de cada tap a lineal
		s[i]=P[i]/(K[i]+1);							//calculem la variança
		m[i]=(float)sqrt(P[i]*(K[i]/(K[i]+1)));		//calculem la part constant de la potencia

		printf("P[%d]=%f, s[i]=%f, m[i]=%f\n", i, P[i], s[i], m[i]);
	}
	
	for(i=0;i<TAPS;i++)								//calculem els retards en mostres per cada tap
	{
		taps[i]=(int)(tau[i]*(float)symbolRate/1000000.0);

		printf("taps[%i]=%d, tau[i]=%f, symbolRate=%d\n", i, taps[i], tau[i], symbolRate);
		
//		if(taps[i]>=FLOW_SIZEvar)						//per si un cas, vigilem que el retard no sigui superior a la mida de dos flows
//			taps[i]=FLOW_SIZEvar-1;
	}
	// COMPUTE THE CHANNEL IMPULSE RESPONSE LENGTH
	channelLength=taps[TAPS-1]+1;
	printf("channelLength=%d, taps[%i]=%d\n", channelLength, 0, taps[0]);
	printf("channelLength=%d, taps[%i]=%d\n", channelLength, 1, taps[1]);
	printf("channelLength=%d, taps[%i]=%d\n", channelLength, 2, taps[2]);

	maxDop=max(Dop,TAPS);									//calculem el doppler mes alt
	resamplingRate=(int)((float)symbolRate/(2.0*maxDop));	//calculem el factor de reescalat que necessitarem per la nostra tasa

	Fnorm=(float)pow(10,Fnorm/20);							//passem la potencia normalitzada a lineal

	//generem la distribucio de Rice
	for(i=0;i<TAPS;i++)
	{
		for(j=0;j<SUI_SAMPLES;j++)
		{
			suiReal[i][j]=(float)sqrt(0.5)*gauss(0,1)*s[i];
			suiImag[i][j]=(float)sqrt(0.5)*gauss(0,1)*s[i];
		}
	}

	//afegim l'efecte doppler
	for(i=0;i<TAPS;i++)
	{
		//normalitzem al doppler mes alt					
		D=Dop[i]/maxDop/(float)2.0;

		//realitzem l'amproximacio de distribucio de potencia espectral
		for(j=0;j<(int)(M*D)+1;j++)				
		{
			aux=(float)j/(M*D);
			PSD[j]=(float)0.785*(float)pow(aux,(float)4)-(float)1.72*(float)pow(aux,(float)2)+(float)1.0;		
		}

		//Creem H(f)
		for(j=0;j<M;j++)						
		{
			filtReal[j]=(float)0;
			filtImag[j]=(float)0;				//creem la part imaginaria del filtre perque la necessita la FFT, pero no la fem servir
		}
		
		for(j=0;j<(int)(M*D)+1;j++)						
		{
			filtReal[j]=(float)sqrt(PSD[j]);
			filtReal[M-j-1]=(float)sqrt(PSD[j]);
		}

		//obtenim el valor de la resposta impulsional
		FFT(-1,(long)(log((float)M)/log((float)2)),filtReal,filtImag);		

		iFFTShift(filtReal);

		//normalitzem el filtre
		sumFilt=0;								
		for(j=0;j<M;j++)					
		{
			sumFilt=sumFilt+(float)pow(filtReal[j],(float)2);
		}
		
		sumFilt=sqrt(sumFilt);

		for(j=0;j<M;j++)					
		{
			filtReal[j]=filtReal[j]/sumFilt;
		}

		//apliquem el filtre doppler al canal Rice
		FFTFilt(suiReal[i],suiImag[i],filtReal);
	}	

	for(i=0;i<TAPS;i++)					//apliquem el factor de normalitzacio
	{
		for(j=0;j<SUI_SAMPLES;j++)
		{
				suiReal[i][j]=(suiReal[i][j]+m[i])*Fnorm;
				suiImag[i][j]=(suiImag[i][j]+m[i])*Fnorm;
		}
	}	

	for(i=0;i<FLOW_SIZEvar*2;i++)			//inicialitzem el vector que guarda els retards del flow anterior
	{
		delayReal[i]=0;
		delayImag[i]=0;
	}

	// CREATE IMPULSE RESPONSE
	for(j=0; i<channelLength; i++)CHCoeff[i]=0.0+0.0*I;
	for(i=0; i<TAPS; i++){
		CHCoeff[taps[i]]=suiReal[i][0]+suiImag[i][0]*I;
		printf("CHCoeff[%d]=%f+%f*I\n", taps[i], __real__ CHCoeff[taps[i]], __imag__ CHCoeff[taps[i]]);
	}



	////sense multipath
	/*for(j=0;j<SUI_SAMPLES;j++)
	{
		suiReal[0][j]=0.5;
		suiImag[0][j]=0.5;
	}
	for(i=0;i<TAPS-1;i++)
	{
		for(j=0;j<SUI_SAMPLES;j++)
		{
			suiReal[i+1][j]=0;
			suiImag[i+1][j]=0;
		}
	}*/
}


void addNoise(float *DataRe, float *DataIm,float snr)		//afegeix soroll blanc gaussia a una senyal d'entrada
{
	/*float sigma, mean = 0.0;
	int i;

	sigma = pow(10,-snr/10);							//calculem la desviacio estandard a partir de la SNR
	sigma = sqrt(sigma);*/
	
	
    int i;
    float mean, es, sn_ratio, sigma;
 
    /* given the desired Es/No (for BPSK, = Eb/No - 3 dB), calculate the
    standard deviation of the additive white gaussian noise (AWGN). The
    standard deviation of the AWGN will be used to generate Gaussian random
    variables simulating the noise that is added to the signal. */

    mean = 0.0;
    es = 1.0;
    sn_ratio = (float) pow(10, ( snr / 10) );
    sigma = (float) sqrt (es / sn_ratio);
   
	
	for(i = 0; i < FLOW_SIZEvar; i++)
	{
		DataRe[i] = DataRe[i] + gauss(mean, sigma); 
		DataIm[i] = DataIm[i] + gauss(mean, sigma);
	}
}




void canal2(_Complex float *ccinput, int numsamples, _Complex float *ccoutput, float snr){

	int K, i, NofBlocks, NumSamplesBlock;

	NofBlocks=1;
	NumSamplesBlock=numsamples/NofBlocks;

	static int NCoeff=0;

	//redimensionem la part del vector del canal amb la que treballarem per adequar-la a la nostra tasa d'observacio
	//resample();

	memset(CHCoeff, 0, sizeof(_Complex float)*channelLength);
	for(K=0; K<NofBlocks; K++){
		//UPDATE COEFF

		for(i=0; i<TAPS; i++){
			//CHCoeff[taps[i]]=suiReal[i][NCoeff]+suiImag[i][NCoeff]*I;flowChannelReal[i][j]
			//CHCoeff[taps[i]]=flowChannelReal[i][NCoeff]+flowChannelImag[i][NCoeff]*I;
			CHCoeff[taps[i]]=resample2(NCoeff, 50, i);
			//CHCoeff[taps[i]]=resample3(20, i);

			//if(NCoeff==SUI_SAMPLES)NCoeff=0;
			//printf("CHCoeff[%d]=%f+%f*I\n", taps[i], __real__ CHCoeff[taps[i]], __imag__ CHCoeff[taps[i]]);	
		}
		NCoeff++;
		//*ccoutput=CHCoeff[taps[0]];
		//CONVOLUTION
		stream_conv_CPLX_INT_DEC(ccinput+K*NumSamplesBlock, NumSamplesBlock, CHCoeff, channelLength, ccoutput+K*NumSamplesBlock);

	}


}


void canal(float *DataRe, float *DataIm, float snr)
{
	int i,j;



	






	/*Channel SUI Inicialization*/
//	initSui(3, 27700);   /*SUI number & symbol rate*/
//	initSui(3, 30720000);   /*SUI number & symbol rate*/


	
	//redimensionem la part del vector del canal amb la que treballarem per adequar-la a la nostra tasa d'observacio
	//resample();				




	for(j=0;j<FLOW_SIZEvar;j++)
	{


		delayReal[j]=(float)creal(CHCoeff[j]);
		delayImag[j]=(float)cimag(CHCoeff[j]);



	}





/*	//apliquem els retards del flow anterior a l'inici del flow actual
	printf("FLOW_SIZEvar=%d\n", FLOW_SIZEvar);
	for(i=0;i<FLOW_SIZEvar;i++)
	{
		delayReal[i]=delayReal[i+FLOW_SIZEvar];
		delayReal[i+FLOW_SIZEvar]=0;
		delayImag[i]=delayImag[i+FLOW_SIZEvar];
		delayImag[i+FLOW_SIZEvar]=0;
	}
	
	//afegim les mostres al flow actual
	for(j=0;j<FLOW_SIZEvar;j++)
	{
		for(i=0;i<TAPS;i++)
		{		
//			delayReal[j+taps[i]]=delayReal[j+taps[i]]+DataRe[j]*flowChannelReal[i][j];
//			delayImag[j+taps[i]]=delayImag[j+taps[i]]+DataIm[j]*flowChannelImag[i][j];

			delayReal[j]=suiReal[0][j];
			delayImag[j]=suiImag[0][j];


//			delayReal[j]=flowChannelReal[0][j];
//			delayImag[j]=flowChannelImag[0][j];


		}
	}
*/	
	//retornem la sortida i afegim soroll al canal
	for(i=0;i<FLOW_SIZEvar;i++)
	{
		DataRe[i]=delayReal[i];
		DataIm[i]=delayImag[i];

	}


//	addNoise(DataRe,DataIm,snr);
	
}


void changeSymbolRate(int symbolRate)		//recalcula el factor de reescalat a partir d'una nova symbolRate
{
	int i;

	resamplingRate=(int)((float)symbolRate/(2.0*max(Dop,TAPS)));	//calculem el factor de reescalat que necessitarem per la nostra tasa
	
	for(i=0;i<TAPS;i++)								//calculem els retards en mostres per cada tap
	{
		taps[i]=(int)(tau[i]/1000000*symbolRate);
		
		if(taps[i]>=FLOW_SIZEvar)						//per si un cas, vigilem que el retard no sigui superior a la mida de dos flows
			taps[i]=FLOW_SIZEvar-1;
	}

	for(i=0;i<FLOW_SIZEvar*2;i++)					//inicialitzem el vector que guarda els retards del flow anterior
	{
		delayReal[i]=0;
		delayImag[i]=0;
	}

	posCount=0;										//retornem a 0 la posicio dels contadors
}







/**
 * @stream_conv_CPLX_DDC(): Perform continous convolution and interpolate by INTER and 
 *                          decimate by DECIM
 * @param.
 * @oParaml: Refers to the struct that containts the module parameters
 * @return: Number of output samples
 */

int stream_conv_CPLX_INT_DEC(	_Complex float *ccinput, int datalength, _Complex float *filtercoeff, int filterlength, _Complex float *ccoutput){

	int i, j, k=0;
	static int first=0;
	static _Complex float aux[CHANNELLENGTH];
	int INTER=1, DECIM=1;
	static int n=0, m=0;


	if(first==0){
		for(j=0; j<filterlength; j++){
			aux[j]=0.0+0.0i;
		}
		first=1;
	}

	for (i=0;i<datalength*INTER;i++) {
		if(n==0){
			for (j=filterlength-2;j>=0;j--) {
				aux[j+1]=aux[j];
			}
			aux[0]=ccinput[i/INTER];
		}
		
		if(m==0){
			ccoutput[k]=0.0;
			for (j=0;j<filterlength/INTER;j++) {
				__real__ ccoutput[k]+=(__real__ aux[j])*(__real__ filtercoeff[j*INTER+n]);
				__imag__ ccoutput[k]+=(__imag__ aux[j])*(__imag__ filtercoeff[j*INTER+n]);
			}
			k++;
		}
		n++;							
		if(n==INTER)n=0;		
		m++;
		if(m==DECIM)m=0;
	}
	//printf("datalength*INT/DEC=%d\n", datalength*INTER/DEC);
	return (datalength*INTER/DECIM);
}
















