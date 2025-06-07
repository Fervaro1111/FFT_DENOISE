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

#include <complex.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <phal_sw_api.h>
#include "skeleton.h"
#include "params.h"

#include "MAC_RX_DL_interfaces.h"
#include "MAC_RX_DL_functions.h"
#include "MAC_RX_DL.h"


extern MAC_struc MACinfo;
extern bits_t DCI;
extern DCIinfo_t DCI_info;
extern SYNC_struc Syncinfo;

#define MAXCB_KCBg1 1152
#define MAXCB_KCBg2 1152
//#define MAXCB_KCBg1 8448
//#define MAXCB_KCBg2 3840

//ALOE Module Defined Parameters. Do not delete.
char mname[STR_LEN]="MAC_LAYER";

//Module User Defined Parameters
#define TX_mode	0
#define RX_mode	1
int TX_RX;
float coderate;
float coderateMCS;


//Global Variables

int Ts=0;


int Symb_X_slot=13;// symbols per slot containing PDSCH
int Symb_X_slot_SS=5;// symbols per slot containing PDSCH when there's SS BLOCK


int Overhead_higherlayer=0;
int nPRB;


int TBS_long;
int TBS_short;

int CELLID_received=-1;


/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("TX_RX_mode", &TX_RX);
	//param_get_float("coderate", &coderate);
	//param_get_int("mod_order", &MACinfo.QAM_order);
	param_get_int("num_layers", &MACinfo.num_layers);
	param_get_int("Overhead_higherlayer", &Overhead_higherlayer);
	param_get_int("nPRB", &MACinfo.nPRB);
	param_get_int("RNTI", &MACinfo.RNTI);
	param_get_int("CELLID", &MACinfo.CELLID);
	param_get_int("q", &MACinfo.q);
	//param_get_int("rv", &MACinfo.rv);
	param_get_int("Additional_DMRS", &MACinfo.Additional_DMRS_position);
	param_get_int("DMRS_type", &MACinfo.DMRS_type);
	param_get_int("CORESETS_symbol_size", &MACinfo.CORESETS_symbol_size);
	param_get_int("Agr_lvl_DCI", &MACinfo.Agr_lvl_DCI);
		


	//Chooses the closest MCS to desired parameters accordind to 5GNR standard	
	//coderateMCS=closest_MCS(coderate,MACinfo.QAM_order);	
	//MACinfo.coderateMCS=coderateMCS;
	


	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O	num_layers=%d\n",MACinfo.num_layers);
	printf("O	Overhead_higherlayer=%d\n",MACinfo.Overhead_higherlayer);
	printf("O	nPRB=%d\n",MACinfo.nPRB);
	printf("O	RNTI=%d\n",MACinfo.RNTI);
	printf("O	q=%d\n",MACinfo.q);
	printf("O	rv=%d\n",MACinfo.rv);
	printf("O	DMRS_type=%d\n",MACinfo.DMRS_type);
	printf("O	Additional_DMRS_position=%d\n",MACinfo.Additional_DMRS_position);
	printf("O	CORESETS_symbol_size=%d\n",MACinfo.CORESETS_symbol_size);
	printf("O	Agr_lvl_DCI=%d\n",MACinfo.Agr_lvl_DCI);

	printf("O--------------------------------------------------------------------------------------------O\n");

	/* do some other initialization stuff */
	
	DCI_info.N_size_BWP=MACinfo.nPRB;

	
	int error=check_parameters(&MACinfo);
	if(error!=0) exit(0);

	GETFFT_size(&MACinfo);

	//Calc_symbols(&Symb_X_slot_SS,&Symb_X_slot,&MACinfo);
	//printf("Symb_X_slot_SS=%d, Symb_X_slot=%d\n",Symb_X_slot_SS,Symb_X_slot);

	return 0;
}



/**
 * @brief Function documentation
 *
 * @param inp Input interface buffers. Data from other interfaces is stacked in the buffer.
 * Use in(ptr,idx) to access the address. To obtain the number of received samples use the function
 * int get_input_samples(int idx) where idx is the interface index.
 *
 * @param out Input interface buffers. Data to other interfaces must be stacked in the buffer.
 * Use out(ptr,idx) to access the address.
 *
 * @return On success, returns a non-negative number indicating the output
 * samples that should be transmitted through all output interface. To specify a different length
 * for certain interface, use the function set_output_samples(int idx, int len)
 * On error returns -1.
 *
 * @code
 * 	input_t *first_interface = inp;
	input_t *second_interface = in(inp,1);
	output_t *first_output_interface = out;
	output_t *second_output_interface = out(out,1);
 *
 */



int work(input_t *inp, output_t *out) {
	int rcv_samples=0; 
	int snd_samples=0;


	input_t* input0;
	input0 = in(inp,0);
	SYNC_struc* input_Syncro= in(inp,1);  
	int rcv_samples_syncro = get_input_samples(1);
	rcv_samples = get_input_samples(0);
	//printf("rcv_samples MAC RX DL =%d\n",rcv_samples);
	snd_samples=0;


	MAC_struc *output1;
	int *output2;
	int *output3;	
	MAC_struc *output4;	
	MAC_struc *output5;
	int *output6;
	MAC_struc *output7;
	MAC_struc *output8;
	MAC_struc *output9;
	MAC_struc *output10;
	MAC_struc *output11;
	MAC_struc *output12;
	int *output13;

	//output0=out(out,0);
	output1=out(out,1);
	output2=out(out,2);
	output3=out(out,3);
	output4=out(out,4);
	output5=out(out,5);
	output6=out(out,6);
	output7=out(out,7);
	output8=out(out,8);
	output9=out(out,9);
	output10=out(out,10);
	output11=out(out,11);
	output12=out(out,12);
	output13=out(out,13);
	

	/*TODO*/
	*output13=MACinfo.FFTsize; 
	set_output_samples(13,sizeof(int));

	

	//printf("FFTsize FROM SYNCRO=%d\n",MACinfo.FFTsize);
	//Read CELL ID VALUE FROM 
	if(CELLID_received==-1 && rcv_samples_syncro!=0){
		Syncinfo=*input_Syncro;
		MACinfo.NID1=Syncinfo.NID1;
		MACinfo.NID1=Syncinfo.NID2;
		MACinfo.CELLID=Syncinfo.CELLID;


		CELLID_received++;

	}

	//printf("CELL ID FROM SYNCRO=%d\n",Syncinfo.CELLID);


	//////////////Parameters to process PDCCH//////////////

	if(CELLID_received!=-1){

		*output1=MACinfo; 
		set_output_samples(1,sizeof(MACinfo));

		*output2=MACinfo.Agr_lvl_DCI; //TO POLAR DECODER PDCCH input1
		set_output_samples(2,sizeof(int));

		*output3=MACinfo.CORESETS_symbol_size; //TO POLAR DECODER PDCCH input2
		set_output_samples(3,sizeof(int));

		*output4=MACinfo; 
		set_output_samples(4,sizeof(MACinfo));

		MACinfo.QAM_order=2;
		*output5=MACinfo; 
		set_output_samples(5,sizeof(MACinfo));

		*output11=MACinfo; 
		set_output_samples(11,sizeof(MACinfo));
	
		*output12=MACinfo; 
		set_output_samples(12,sizeof(MACinfo));

	}
	//////////////Parameters to process PDCCH//////////////




	//////////////////////Parse PDCCH//////////////////////

	if(rcv_samples!=0){

		//Convert to flow bits
		bits_t* DCI_rcv_bits;
		DCI_rcv_bits=(bits_t*)input0;


		//GET DCI DATA
		Undo_DCI(&DCI_info,DCI_rcv_bits);

		GET_params_from_DCI(&DCI_info,&MACinfo);


		/*printf("RX MAC coderateMCS=%f\n",MACinfo.coderateMCS);
		printf("RX MAC rv=%d\n",MACinfo.rv);
		printf("RX MAC MOD_QAM=%d\n",MACinfo.QAM_order);

		printf("RX MAC Lrbs=%d\n",DCI_info.Lrbs);
		printf("RX MAC RBstart=%d\n",DCI_info.RBstart);
		printf("RX MAC time_domain_allocation=%d\n",DCI_info.time_domain_allocation);*/

		*output6=1; 
		set_output_samples(6,sizeof(int));	

		//printf("RX MAC flag to buffer=%d\n",*(output6));
		//////////////////////PArse PDCCH//////////////////////


		//////////////Parameters to process PDSCH//////////////

		Calc_symbols(&Symb_X_slot_SS,&Symb_X_slot,&MACinfo);
		//printf("Symb_X_slot_SS=%d, Symb_X_slot=%d\n",Symb_X_slot_SS,Symb_X_slot);


	
	//SS block 1st and 2nd Subframe from 1 out of 2 frames
		if(Ts==20)Ts=0;	
		if (Ts==0 || Ts==1 ){
			
				calcDMRSlength_short(&MACinfo);	

				//Calculate TBS for "short" subframe 
	
				TBS_short=Calc_TBS(Symb_X_slot_SS,MACinfo.REs_DMRS,MACinfo.Overhead_higherlayer,MACinfo.nPRB,
					MACinfo.coderateMCS,MACinfo.QAM_order,MACinfo.num_layers,MAXCB_KCBg2,MAXCB_KCBg1, &MACinfo);
		
				MACinfo.TB_size_NO_CRC=TBS_short;
				MACinfo.Symb_X_slot=Symb_X_slot_SS;

			}else{
		
				//Calculate TBS for "long" subframe .
			
				calcDMRSlength_long(&MACinfo);
				TBS_long=Calc_TBS(Symb_X_slot, MACinfo.REs_DMRS, MACinfo.Overhead_higherlayer, MACinfo.nPRB, 
					MACinfo.coderateMCS, MACinfo.QAM_order, MACinfo.num_layers, MAXCB_KCBg2, MAXCB_KCBg1, &MACinfo);
				MACinfo.TB_size_NO_CRC=TBS_long;
				MACinfo.Symb_X_slot=Symb_X_slot;
			}
	


		MACinfo.Ts_MAC=Ts;


		if (MACinfo.TB_size_NO_CRC>(MAXCB_KCBg2-16)){
			MACinfo.TB_size_TBCRC=(MACinfo.TB_size_NO_CRC/8)+3;
		}else{
			MACinfo.TB_size_TBCRC=(MACinfo.TB_size_NO_CRC/8)+2;	
			
		}
	


		//////////////Parameters to process PDSCH//////////////

		*output7=MACinfo; 
		set_output_samples(7,sizeof(MACinfo));
		*output8=MACinfo; 
		set_output_samples(8,sizeof(MACinfo));
		*output9=MACinfo; 
		set_output_samples(9,sizeof(MACinfo));
		*output10=MACinfo; 
		set_output_samples(10,sizeof(MACinfo));
	

		Ts++;


		/*printf("RX MAC rv=%d\n",MACinfo.rv);
		printf("RX MAC MOD_QAM=%d\n",MACinfo.QAM_order);
		printf("RX MAC TB_size_NO_CRC=%d\n",MACinfo.TB_size_NO_CRC);
		printf("RX MAC TB_size_TBCRC=%d\n",MACinfo.TB_size_TBCRC);
		printf("RX MAC RNTI=%d\n",MACinfo.RNTI);
		printf("RX MAC nPRB=%d\n",MACinfo.nPRB);
		printf("RX MAC num_layers=%d\n",MACinfo.num_layers);
		printf("RX MAC Overhead_higherlayer=%d\n",MACinfo.Overhead_higherlayer);
		printf("RX MAC REs_DMRS=%d\n",MACinfo.REs_DMRS);
		printf("RX MAC Symb_X_slot=%d\n",MACinfo.Symb_X_slot);
		printf("RX MAC Additional_DMRS_position=%d\n",MACinfo.Additional_DMRS_position);
		printf("RX MAC Agr_lvl_DCI=%d\n",MACinfo.Agr_lvl_DCI);
		printf("RX MAC CORESETS_symbol_size=%d\n",MACinfo.CORESETS_symbol_size);
		printf("RX MAC DMRS_type=%d\n",MACinfo.DMRS_type);

	*/

	}

	

	// Indicate the number of samples at output number N
	//set_output_samples(N, out_samples_at_N_port);
	//printf("%s IN: snd_samples=%f\n", mname, *out);
	// Indicate the number of samples at output 0 with return value
	return snd_samples;
}


int stop() {
	return 0;
}


