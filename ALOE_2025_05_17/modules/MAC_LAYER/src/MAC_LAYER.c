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

#include "MAC_LAYER_interfaces.h"
#include "MAC_LAYER_functions.h"
#include "MAC_LAYER.h"


extern MAC_struc MACinfo;
extern bits_t DCI;
extern DCIinfo_t DCI_info;

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



/*
 * Function documentation
 *
 * @returns 0 on success, -1 on error
 */
int initialize() {

	printf("INITIALIZEoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooP\n");
	/* Get control parameters*/
	param_get_int("TX_RX_mode", &TX_RX);
	param_get_float("coderate", &coderate);
	param_get_int("mod_order", &MACinfo.QAM_order);
	param_get_int("num_layers", &MACinfo.num_layers);
	param_get_int("Overhead_higherlayer", &Overhead_higherlayer);
	param_get_int("nPRB", &MACinfo.nPRB);
	param_get_int("RNTI", &MACinfo.RNTI);
	param_get_int("NID1", &MACinfo.NID1);
	param_get_int("NID2", &MACinfo.NID2);
	param_get_int("q", &MACinfo.q);
	param_get_int("rv", &MACinfo.rv);
	param_get_int("Additional_DMRS", &MACinfo.Additional_DMRS_position);
	param_get_int("DMRS_type", &MACinfo.DMRS_type);
	param_get_int("CORESETS_symbol_size", &MACinfo.CORESETS_symbol_size);
	param_get_int("Agr_lvl_DCI", &MACinfo.Agr_lvl_DCI);
		


	//Chooses the closest MCS to desired parameters accordind to 5GNR standard	
	coderateMCS=closest_MCS(coderate,MACinfo.QAM_order);	
	MACinfo.coderateMCS=coderateMCS;
	MACinfo.CELLID=3*MACinfo.NID1+MACinfo.NID2;
	

	/* Print Module Init Parameters */
	strcpy(mname, GetObjectName());
	printf("O--------------------------------------------------------------------------------------------O\n");
	printf("O    SPECIFIC PARAMETERS SETUP: \033[1;34m%s\033[0m\n", mname);
	printf("O      Nof Inputs=%d, DataTypeIN=%s, Nof Outputs=%d, DataTypeOUT=%s\n", 
		       NOF_INPUT_ITF, IN_TYPE, NOF_OUTPUT_ITF, OUT_TYPE);
	printf("O	CODERATE =%f\n",coderate);
	printf("O	CODERATEMCS =%f\n",MACinfo.coderateMCS);
	printf("O	QAM_order=%d\n",MACinfo.QAM_order);
	printf("O	num_layers=%d\n",MACinfo.num_layers);
	printf("O	Overhead_higherlayer=%d\n",MACinfo.Overhead_higherlayer);
	printf("O	nPRB=%d\n",MACinfo.nPRB);
	printf("O	RNTI=%d\n",MACinfo.RNTI);
	printf("O	NID1=%d\n",MACinfo.NID1);
	printf("O	NID2=%d\n",MACinfo.NID2);
	printf("O	CELLID=%d\n",MACinfo.CELLID);
	printf("O	q=%d\n",MACinfo.q);
	printf("O	rv=%d\n",MACinfo.rv);
	printf("O	DMRS_type=%d\n",MACinfo.DMRS_type);
	printf("O	Additional_DMRS_position=%d\n",MACinfo.Additional_DMRS_position);
	printf("O	CORESETS_symbol_size=%d\n",MACinfo.CORESETS_symbol_size);
	printf("O	Agr_lvl_DCI=%d\n",MACinfo.Agr_lvl_DCI);

	printf("O--------------------------------------------------------------------------------------------O\n");

	/* do some other initialization stuff */
	
	DCI_info.N_size_BWP=MACinfo.nPRB;
	
	if(TX_RX==0){
		int error=check_parameters(&MACinfo);
		if(error!=0) exit(0);
	}

	Calc_symbols(&Symb_X_slot_SS,&Symb_X_slot,&MACinfo);
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

	if(TX_RX==0){
	
		int *output0;
		MAC_struc *output1;
		MAC_struc *output2;
		MAC_struc *output3;	
		MAC_struc *output4;	
		MAC_struc *output5;
		MAC_struc *output6;
		MAC_struc *output7;
		MAC_struc *output8;
		MAC_struc *output9;
		MAC_struc *output10;
		MAC_struc *output11;
		MAC_struc *output12;
		MAC_struc *output13;
		MAC_struc *output14;

		int* output15;
		int* output16;
		MAC_struc *output17;
		MAC_struc *output18;
		int *output19;
		int *output20;	
		MAC_struc *output21;
		MAC_struc *output22;
		MAC_struc *output23;
		char *output24;
		//MAC_struc *output25;

	


	
		output0=out(out,0);
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
		output14=out(out,14);
		output15=out(out,15);
		output16=out(out,16);
		output17=out(out,17);
		output18=out(out,18);
		output19=out(out,19);
		output20=out(out,20);
		output21=out(out,21);
		output22=out(out,22);
		output23=out(out,23);
		output24=out(out,24);
		//output25=out(out,25);
	

	
		//if(Ts==10)MACinfo.QAM_order=4;
		//if(Ts==10)MACinfo.coderateMCS=120.0/1024.0;

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
	

	

		/////////////////Distribute information across PHYSICAL Layer

		*output0=MACinfo.TB_size_NO_CRC/8;
		*output1=MACinfo;
		set_output_samples(1,sizeof(MACinfo));


		*output3=MACinfo;
		set_output_samples(3,sizeof(MACinfo));

		*output4=MACinfo;
		set_output_samples(4,sizeof(MACinfo));

		*output5=MACinfo;
		set_output_samples(5,sizeof(MACinfo));

		*output6=MACinfo;
		set_output_samples(6,sizeof(MACinfo));

		*output7=MACinfo;
		set_output_samples(7,sizeof(MACinfo));
	
		*output8=MACinfo;
		set_output_samples(8,sizeof(MACinfo));
	
		*output9=MACinfo;
		set_output_samples(9,sizeof(MACinfo));

		*output10=MACinfo;
		set_output_samples(10,sizeof(MACinfo));

		*output11=MACinfo;
		set_output_samples(11,sizeof(MACinfo));

		*output12=MACinfo;
		set_output_samples(12,sizeof(MACinfo));
	
		*output13=MACinfo;
		set_output_samples(13,sizeof(MACinfo));
	
		*output14=MACinfo;
		set_output_samples(14,sizeof(MACinfo));
	
	

		//printf("MACinfo ASK SAMPLES TO DATASOURCE = %d\n",*output0);
	
		snd_samples=1*sizeof(int);
	



		/////////////PDCCH/////////////////

		//Initialize params

		Init_parameters_PDCCH(&DCI_info,&MACinfo);
		//DCI_info.MCS_index=MCS_selected(MACinfo.coderateMCS);
		//DCI_info.Lrbs=MACinfo.nPRB;

		//DCI_info.RBstart=0;//First PRB for UE
		//DCI_info.time_domain_allocation=11;//S=1;L=13 from table 5.1.2.1.1-2 from 38.214
		//DCI_info.rv=MACinfo.rv;

		//Build DCI
		do_DCI(&DCI_info,&DCI);

		char* charpointer;
		charpointer=(char*)&DCI;
		//printf("Size of PDCCH is %d\n",sizeof(charpointer));

		memcpy(output24,charpointer,sizeof(charpointer));
		set_output_samples(24,sizeof(charpointer));
		//printf("snd_samp_PDCCH=%d\n",sizeof(charpointer));


		*output2=MACinfo;
		set_output_samples(2,sizeof(MACinfo));

		*output23=MACinfo;
		set_output_samples(23,sizeof(MACinfo));
	
		//*output25=MACinfo;
		//set_output_samples(25,sizeof(MACinfo));
	
		*output15=MACinfo.CORESETS_symbol_size;
		set_output_samples(15,sizeof(int));

		*output16=MACinfo.CORESETS_symbol_size;
		set_output_samples(16,sizeof(int));

		*output19=MACinfo.Agr_lvl_DCI;
		set_output_samples(19,sizeof(int));

		*output20=MACinfo.Agr_lvl_DCI;
		set_output_samples(20,sizeof(int));

		*output17=MACinfo;
		set_output_samples(17,sizeof(MACinfo));

		*output18=MACinfo;
		set_output_samples(18,sizeof(MACinfo));


		//HARDCODE FOR PDCCH MODULATORS. THEY ALWAYS USE 4QAM. 
		int QAM_real=MACinfo.QAM_order;
		MACinfo.QAM_order=2;

		*output22=MACinfo;
		set_output_samples(22,sizeof(MACinfo));

		*output21=MACinfo;
		set_output_samples(21,sizeof(MACinfo));
		MACinfo.QAM_order=QAM_real;


		/////////////PDCCH/////////////////
		Ts++;


	}
	if(TX_RX==1){

		/////////////PDCCH/////////////////
		input_t* input0;
		input0 = in(inp,0);
	
		int rcv_samples = get_input_samples(0);
		if(rcv_samples==0)return 0;


		//Convert to flow bits
		bits_t* DCI_rcv_bits;
		DCI_rcv_bits=(bits_t*)input0;


		//GET DCI DATA
		Undo_DCI(&DCI_info,DCI_rcv_bits);

		GET_params_from_DCI(&DCI_info,&MACinfo);


		printf("coderateMCS=%f\n",MACinfo.coderateMCS);
		printf("rv=%d\n",MACinfo.rv);
		printf("MOD_QAM=%d\n",MACinfo.QAM_order);

	
		printf("Lrbs=%d\n",DCI_info.Lrbs);
		printf("RBstart=%d\n",DCI_info.RBstart);
		printf("time_domain_allocation=%d\n",DCI_info.time_domain_allocation);
		//printf("MCS_index=%d\n",DCI_info.MCS_index);
		//printf("rv=%d\n",DCI_info.rv);
	
		snd_samples=0;

			/////////////PDCCH/////////////////

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



