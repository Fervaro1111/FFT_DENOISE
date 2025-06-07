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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>

#include "REPORT_M25_functions.h"

extern print_t T;



void printREPORT(datasourcesink_t DATASOURCESINK, MAC_t MAC, channel_t CHANNEL, uncrc_t UNCRC){

	float MeasSNR;
   	time_t curtime;
   	struct tm *loc_time;
	
	//Getting current time of system
   	curtime = time (NULL);
 
   	// Converting current time to local time
   	loc_time = localtime (&curtime);

	sprintf(&T.text[0][0], " "); 
	sprintf(&T.text[1][0], "REPORT DataSourceSink: TimeSlot=%2.2fus (Min=21333.3us)\n", DATASOURCESINK.TimeSlot_us);
	sprintf(&T.text[2][0], "REPORT Channel: SUI=%d [0: NOISE ONLY], SNR=%2.1f dBs, AD_Active=%d [0: NonActive, 1: Active]\n", 
						CHANNEL.SUI_Channel, CHANNEL.SNR, CHANNEL.AD_Active);
	sprintf(&T.text[3][0], "REPORT Channel: RotatePhase=%2.1f, BandWidth=%2.1fHz\n", 
						CHANNEL.RotatePhase, CHANNEL.BandPassBw);
	sprintf(&T.text[4][0], "REPORT MAC: FFTsize=%d, NumPRBs=%d\n", MAC.FFTsize, MAC.NumPRBs);  
	sprintf(&T.text[5][0], "REPORT MAC: CodeRate=%3.2f%, MOD_IDX=%d, NID1=%d, NID2=%d \n", MAC.Coderate, MAC.MODidx, MAC.NID1, MAC.NID2);
	sprintf(&T.text[6][0], "REPORT MAC: DUC Interpolation/Decimation=%1.2f\n", MAC.IntDec);
	sprintf(&T.text[7][0], "REPORT: \n");
	sprintf(&T.text[8][0], "REPORT: subframe=%d, FileSentName=%s\n", 
						DATASOURCESINK.subframe, &DATASOURCESINK.file_name[0]);

	sprintf(&T.text[9][0], "REPORT: BLER=%1.6f, BER=%1.6f, Throughtput=%07.2f bps\n", UNCRC.BLER, DATASOURCESINK.BER, DATASOURCESINK.throughput);


	sprintf(&T.text[10][0], " %s", asctime (loc_time)); 
	sprintf(&T.text[11][0], " "); 
	// printTEXT(noflines, textcolor, backgroundcolor, border_char)
	//if(report_STAT.subframe < 200)printTEXT(18, 15, 202, '~');
	//else printTEXT(18, 16, 46, '~');

	if(DATASOURCESINK.subframe < 100)printTEXT(12, 16, 214, '~');
	else printTEXT(12, 214, 16, '~');

// COLORS at https://stackabuse.com/how-to-print-colored-text-in-python/

//printf("%s", asctime (loc_time));

}



/////////////////////PRINTING
#define LINELENGTH	PRINT_LINELENGTH


void printTEXT(int noflines, int textcolor, int backcolor, char borderchar){

	int i, length;
	char border[LINELENGTH];
	char text2print[LINELENGTH];

	T.borderchar=borderchar;
	T.noflines=noflines;
	T.forecolor=textcolor%256;
	T.backgroundcolor=backcolor%256;


	// Border UP
	memset(border, T.borderchar, LINELENGTH-1);
	border[LINELENGTH-1]='\0';
//	printf("\033[1;%s;%sm%s\033[0m\n", textcolor, backcolor, border);

//	printf("%s\n", backcolor); 
/*	printCOLORtext(0, "208", backcolor,  LINELENGTH, border);
	printCOLORtext(5, "208", "010",  LINELENGTH, &T.text[i][0]);
*/
	printCOLORtext(0, T.forecolor, T.backgroundcolor,  LINELENGTH, border);
	for(i=0; i<T.noflines; i++){
		printCOLORtext(2, T.forecolor, T.backgroundcolor,  LINELENGTH, &T.text[i][0]);
	}
	printCOLORtext(0, T.forecolor, T.backgroundcolor,  LINELENGTH, border);
}


//http://web.archive.org/web/20131009193526/http://bitmote.com/index.php?post/2012/11/19/Using-ANSI-Color-Codes-to-Colorize-Your-Bash-Prompt-on-Linux
//char format[BACKGROUNDLEN]="\33[1m\33[38;5;021;48;5;226m";
//char reset[10]="\033[0m";


void printCOLORtext(int position, int forecolor, char groundcolor,  int end, char *data2print){
	
	int i;		
	char background[LINELENGTH]; 
	int length=(int)LINELENGTH;
	char formatt[LINELENGTH]="\33[1m\33[38;5;021;48;5;226m";
	char reset[10]="\033[0m";

	char fore[4];
	char ground[4];

	my_itoa(groundcolor, ground, 1);
	my_itoa(forecolor, fore, 1);

	if(strlen(data2print)>LINELENGTH-1){
		printf("ERROR!!! data2print length = %d > LINELENGTH=%d\n", strlen(data2print), LINELENGTH);
		//exit(0);
	}

	memset(background, 0x20, LINELENGTH-1);
//	background[BACKGROUNDLEN-1]='\0';
/*	printf("A_");
	for(i=0; i<strlen(formatt)-1; i++)printf("%c", formatt[i]);
	printf("\n");
*/
	memcpy(formatt+11, fore, strlen(fore));
	memcpy(formatt+20, ground, strlen(ground));
/*	printf("B_");
	for(i=0; i<strlen(formatt)-1; i++)printf("%c", formatt[i]);
	printf("\n");
*/
	length=strlen(data2print)-1;
	if(length>(LINELENGTH-1-position))length=(LINELENGTH-1-position);
	memcpy(background+position, data2print, length);
	background[end-1]=NULL;
	printf("%s%s%s\n", formatt, background, reset);
}


void my_itoa(int number, char *str, int nofchars) {
   sprintf(str, "%.3d", number&0xFF); //make the number into string using sprintf function
}

///////////////////////////////////////////////////////////////////////////////





