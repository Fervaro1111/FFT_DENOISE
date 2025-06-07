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
#define CHM_NOISE					1	//Channel Mode ADD NOISE

#define NOISE_SNR					1
#define NOISE_VAR					2

#define SNRdBs_DEFAULT				20.0
#define SAMPLING_FREQ_DEFAULT			30720000.0

#define DEBUG_NONACTIVE				1	//1: NON ACTIVE, 2: ACTIVE
#define DEBUG_ACTIVE				2	//1: NON ACTIVE, 2: ACTIVE
#define DEBUG_DEFAULT				DEBUG_NONACTIVE

#define NOADactive					1
#define ADactive					2



typedef struct oParams{
	int ChannelModel;				// 0: Noise, 1: SUI, 2:??
	int NoiseModel;					// 0: SNR, 1: Generate Noise power
	float SNR;
	float NoiseVariance;				// ??? Unitats
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
typedef struct REPORT{
//Provided by DATASOURCESINK
	int subframe;
	float BER;
	float throughput;
	float TimeSlot_us;
	float MeasSubframeTsLength;				// Meas of subframe number Tslots length 
	float StandDevSubframeLength;			// Standard Deviation 	
	unsigned int DSSRoundTripDelay; 	// Delay between receiving data from input0
	unsigned int DSSsndSamples;
	char file_name[STRLENGTH];

	// Provided by EVM
	float EVM;
	unsigned int EVMRoundTripDelay;			// Delay between receiving data form input 0 and 1
	unsigned int EVM_TXsideReceivedSamples;
	float EVM_TXDataPeriod;			// Number of Tslot between receiving samples from input0
	int FFTsize;

	// Provided by CPLX_FILTER
	char filter_name[STRLENGTH];


	// Provided by CHANNEL_SUI
	float SNR;
	int AD_Active;					// AD samples generation from starting simulation
	float RotatePhase;				// Channel Phase rotation
	float BandPassBw;				// Bandwidth of the bandpass signal
	int SUI_Channel;
	float PAPR;

	// Provided by CRC
	float BLER;

	// Provided by REPORT
	int UpDownLink;
	char username[STRLENGTH];		// Name of the user. Must be captured from .params file
	int Tslot;
	int MAC_ID;						// MAC identifier, aqain a random fix value
	int ctrl_val;
	
	
	// Calculated at REPORT
	float Coderate;
	int MODidx;								// Modulation index: 2-4QAM, 4:16QAM, 6-64QAM
	float IntDec;							// Interpolation/decimation: N/D


//	Not NEEDED
/*	int ctrl_val;					// Random value, should depen from current time
	int CHANNEL_Tslot;
	int CPLXFILTER_Tslot;
	int CRC_Tslot;
	int DSS_Tslot;
	int EVM_Tslot;*/

}report_t;


void update_CTRL(report_t *reportIN, report_t *reportOUT, float SNR, int CHANNEL_Tslot, float RotatePhase, int AD_Active);

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




float get_variance(float snr_db,float scale);
float rand_gauss (void);
void gen_noise_c(_Complex float *x, float variance, int len);
float aver_power(_Complex float *in, int length);
void add_delay(_Complex float *in, _Complex float *out, int inlength, int delay);

#endif
