/* 
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

#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H


#define MMPI					3.141592653589793238462
#define CHM_BYPASS					0	//Channel Mode BYPASS
#define CHM_NOISE					1	//Channel Mode ADD GAUSSIAN NOISE POWER

#define NOISE_SNR					1
#define NOISE_VAR					2	//Channel Mode ADD GAUSSIAN NOISE VARIANCE
#define NOISE_IMP_SKEW				3	//Channel Mode ADD GAUSSIAN NOISE VARIANCE IMPULSIVITY AND SKEWENESS

#define SNRdBs_DEFAULT				20.0
#define SAMPLING_FREQ_DEFAULT			30720000.0

#define DEBUG_NONACTIVE				1	//1: NON ACTIVE, 2: ACTIVE
#define DEBUG_ACTIVE				2	//1: NON ACTIVE, 2: ACTIVE
#define DEBUG_DEFAULT				DEBUG_NONACTIVE

#define NOADactive					1
#define ADactive					2



typedef struct oParams{
	int ChannelModel;				// 0: Noise, 1: SUI, 2:??
	int NoiseModel;					// 0: Gaussian SNR, 
									// 1: Generate Gaussian Noise power, 
									// 2: Gaussian noise + Impulsivity and skeweness
	float SNR;
	float NoiseVariance;				// ??? Unitats
	float imp_prob;					// Impulsivity
	float skw_f;					// Skeweness
	int ADmodel;					// 0: AD not active Model; 1: AD active Model (generate noise data from starting execution)
	float gain;
	float samplingfreq;
	int debugg;
	
}oParams_t;


int CHANNEL(oParams_t *oCHANNEL, _Complex float *in, _Complex float *out, int rcv_samples);

////////SUI
void init_SUI_CHANNEL(int SUIchannel, float samplingfreq);
int run_channel_SUI(_Complex float *in, int nofsamples, _Complex float *out, float SNRdBs);



#define MAXTSLOT	50
#define STRLENGTH	64

typedef struct CHANNEL{
	// Provided by CHANNEL_SUI
	float SNR;
	int AD_Active;					// AD samples generation from starting simulation
	float RotatePhase;				// Channel Phase rotation
	float BandPassBw;				// Bandwidth of the bandpass signal
	int SUI_Channel;
	float PAPR;

}channel_t;






void update_CTRL(channel_t *reportOUT, float SNR, float RotatePhase, int AD_Active, float BandPassBW, int SUI, float PAPR);

// PRINT TEXT
#define MAXNOFLINES	20
#define MEM_LINELENGTH	200
#define PRINT_LINELENGTH	96

typedef struct PRINT{
	char text[MAXNOFLINES][MEM_LINELENGTH];
	int noflines;
	char borderchar;
	int forecolor;
	int backgroundcolor;
} print_t;


void printTEXT(int noflines, int textcolor, int backcolor, char borderchar);
void printCOLORtext(int position, int forecolor, char groundcolor,  int end, char *data2print);
void my_itoa(int number, char *str, int nofchars);


#define NOISELENGTH INPUT_MAX_DATA

float get_variance(float snr_db,float scale);

//float rand_gauss (void);
//void gen_noise_c(_Complex float *x, float variance, int len);
//int noise_SNRe(_Complex float *inp, _Complex float *out, float SNR, float gain, int rcv_samples);

float MYaver_power(_Complex float *in, int length);
int MYnoise_SNRe(_Complex float *inp, _Complex float *out, float SNR, int rcv_samples);
void MYgen_noise_c(_Complex float *x, float variance, int len);
float MYrand_gauss (void);


float ComputeWaveformBW(int Tslot, _Complex float *input, _Complex float *output);

float get_variance(float snr_db,float scale);
float rand_gauss (void);
void gen_noise_c(_Complex float *x, float variance, int len);
float aver_power(_Complex float *in, int length);
void add_delay(_Complex float *in, _Complex float *out, int inlength, int delay);


float gen_noise_var(_Complex float *noise, float SNR_db, int length);
void gen_skewness_and_impulsive_noise(_Complex float *noise, int rcv_samples, float imp_prob, float skw_f, float gain);

#endif
