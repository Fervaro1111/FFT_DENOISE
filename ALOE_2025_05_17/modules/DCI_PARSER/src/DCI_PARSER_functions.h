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


// ESTRUCTURA//

//Primer bit a 1 pq es DL
//0 bits de carrier indicator
// BWP indicator 1 bit =0
/*Frequency domain resource assignment=RIV
Resource allocation type 1 -->
	-->Lrbs=contiguously allocated blocks
	-->RBstart=Fisrt RB with that particular UE data
	-->N_size_BWP=number of PRB of the BWP

	bit size of the field is  ceil(log2(N_size_BWP*(N_size_BWP+1)/2))
	-> es hardcodeja la longitud a 16 bits pq aloe no permet fer el valor dinamic a l'hora de crear lestructura. 16 bits es el length per 270 PRB que es el maxim d'aquesta configuracio de SCS=15KHz

	
*/
// ------->>>OPTION NOT USED-------> no quadren els bits que has d'utilitzar amb el que has d'enviar
/*Time domain allocation
	k0=slot numbre from DCI slot and data slot -> 0 pq venen en el mateix slot(subframe)
	mapping  type A ->0
 	SLIV=start i duracioen un mateix numero
		L=length in symbols
		S=start symbol

	length del camp=5 bits(k0)+1 bit(mapping type)+7 bits(SLIV)=13bits

*/


//Option used
/*Time domain allocation
	taula 5.1.2.1.1-2 del 38.214
	length=ceil(log2(16)=4bits;
	per a S=1 i L=13 amb k0=0 es l'index 11 (si es conta de 0 a 15)
	11 en bits es --> 1011
	longitud=4bits
*/


//VRB-PRB 1 bits --> hardcoded at 0
//PRB bundling size indicator 0 bits
//Rate matching indicator 0 bits hardcoded
//ZP CSI-RS trigger – 0 bits hardcoded
//MCS 5 bits
//New data indicator 1bit -> hardcode 1
//redundancy version 2bits
//HARQ process number – 4 bits-->hardcoded to 0000
//Downlink assignment index  0 bits 
//TPC command for scheduled PUCCH 2 bits hardcoded to 00
//PUCCH resource indicator 3bits hardcoded to 000
//PDSCH-to-HARQ_feedback timing indicator – 0 bits hardcoded
//Antenna port(s) – 4,5,6 bits -->hardcoded to 0000
//Transmission configuration indication 3 bits ->hardoded to 000
//SRS request – 2 bits hardcoded to 00
//CBG transmission information (CBGTI) – 0 bits hardcoded
//CBG flushing out information (CBGFI) – 0 bits hardcoded
//DMRS sequence initialization 0 bits ->hardcoded







typedef struct {
	unsigned int f1:1;//DL 1 bit
	unsigned int f2:1;// BWP indicator 1 bit
	unsigned int f3:16;//RIV ceil(log2(N_size_BWP*(N_size_BWP+1)/2)) bits
	unsigned int f4:4;//Time domain allocation 4 bits
	unsigned int f5:1;//VRB-to-PRB 1 bit
	unsigned int f6:5;//MCS 5 bits
	unsigned int f7:1;//New data indicator 1bit
	unsigned int f8:2;//redundancy version 2bits
	unsigned int f9:4;//HARQ process number
	unsigned int f10:2;//TPC command for scheduled PUCCH 2 bits
	unsigned int f11:3;//PUCCH resource indicator 3bits
	unsigned int f12:4;//Antenna port(s) – 4,5,6 bits hardcode 0000
	unsigned int f13:3;//Transmission configuration indication 3 bits
	unsigned int f14:2;//SRS request 2bits
}bits_t;

typedef struct{
	int Lrbs;
	int RBstart;
	int N_size_BWP;
	int RIV;
	int MCS_index;
	int rv;
	int time_domain_allocation;

}DCIinfo_t;


void GET_Lrbs_RBstart(int* Lrbs,int* RBstart,int N_size_BWP,int RIV);
int Calc_RIV(int Lrbs,int RBstart,int N_size_BWP);
int Calc_Frequencydomain_allocation_size(int N_size_BWP);

int GET_S_L(int* S,int* L,int SLIV_real);
int Calc_SLIV(int S,int L);

void FILL_DCI(bits_t* DCI,DCIinfo_t* DCI_info);
void do_DCI(DCIinfo_t* DCI_info,bits_t* DCI);
void byte2bitsLSBF(char value, char **bits, int nof_bits);
void Undo_DCI(DCIinfo_t* DCI_info,bits_t* DCI);
void UNFILL_DCI(bits_t* DCI,DCIinfo_t* DCI_info);






#endif
