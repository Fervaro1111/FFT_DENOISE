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

#define MAXSEQLENGTH	2048*50
#define Nc		1600

//Estructura MIB
/*
->System frame number--> 10 bits, 6 MSB included in MIB
->Subcarrier spacing--> 1bit, 15/30 KHz for FR1 and 60/120 KHz for FR2.
->SSB subcarrier offset --> 5 bits for FR1 and 4 for FR2. Only 4 are included in MIB
->DMRS type A position--> 1 bit. Indicates if fiest DMRS is located in 2nd or 3rd OFDM symbol.
->PDCCH-Config-SIB1 -->8 bits-->Harcoded to 0 as no CORESET#0 nor SIB1 is transmitted.
->Cell barrer -->1 bit. Indicates if UEs can acces or not the cell.
->Intra frequency selection--> 1bit
->SSB index--> 3bits for FR2 and 0 for FR1.
->BCCH-BCH-MessageType indication-->1 bit

->Spare bits-->1bit



->4th,3rd,2nd,1st LSB bits of SFN 
->->Half frame bit--> 1 bit. 0 if BCH is sent in the first half of a 10ms frame. 1 otherwise.
->MSB of Kssb and 0 0 for Sub-6GHz ot 6th,5th,4th MSB of Kssb

TOTAL==> 32 bits + 24bits(CRC) ==56 bits.
QPSK modulated.

*/

typedef struct {
	unsigned int f1:6;//6 MSB of SFN
	unsigned int f2:1;//Subcarrier spacing-> 0 because it's FR1 and 15KHz SCS. HARCODED
	unsigned int f3:4;//KSSb offset and FR1
	unsigned int f4:1;//First DM-RS position
	unsigned int f5:8;//SSIB1-Config-->Harcoded to 0 as no CORESET#0 nor SIB1 is configured.
	unsigned int f6:1;//Cell barrer->0 as not-barrered cell ->HARCODED
	unsigned int f7:1;//Intra frequency selection ->0 ->HARCODED
	unsigned int f8:1;//Spare bit -> 1 HARCODED
	unsigned int f9:1;//BCCH-BCH-MessageType ->1 hardcoded

//Fins aqui el MIB propiament ->24 bits

	unsigned int f10:4;//4 LSB of SFN
	unsigned int f11:1;//Half frame bit
	unsigned int f12:1;//Kssb offset MSB
	unsigned int f13:2;//Reserved --> 00

//Fins aqui son 24 bits + 8 bits = 32 bits.
}bits_t;

typedef struct {
	int SFN;
	int SCS;
	int Kssb;
	int DMRS_position;
	int Cell_barrer;
	int Intra_freq_sel;
 	int Half_frame_bit;
	int MSBSFN_6;
	int LSBSFN_4;
	int Subcarrier_spacing //value for f2.

}PBCH_t;


void convert_2_binary(int in,int* binary);
int calc_Half_frame_bit(int slot_num,int SCS);
int DMRS_type_A_pos(int pos_DMRS);
void Subcarrier_spacing_Calc(int SCS,PBCH_t* BCH_info);
void Prepare_Payload(int pos_DMRS,int slot_num,int SCS,PBCH_t* BCH_info,int SFN,bits_t* BCH_payload);
void Fill_Payload(bits_t* BCH_payload,PBCH_t* BCH_info);

void UNFill_Payload(bits_t* BCH_payload,PBCH_t* BCH_info);
int DMRS_type_A_pos_RX(int pos_DMRS);
int Subcarrier_spacing_GET(PBCH_t* BCH_info);
void Obtain_Payload(int *SCS,PBCH_t* BCH_info,bits_t* BCH_payload);
void byte2bitsLSBF(char value, char **bits, int nof_bits);
void BCH_interleaving(char* in,char* out);
void BCH_Deinterleaving(char* in,char* out);

int choose_M(int Lmax,int A);
int choose_v(int third_lsb, int second_lsb);

#endif
