/*
 * cmdman_prints.c
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>. All rights reserved.
 *
 * This file is part of ALOE.
 *
 * ALOE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ALOE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ALOE.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "phal_hw_api.h"
#include "cmdman_backend.h"

#define NOF_PROCS	256

char pstr[2048];
char buffer[256];
char unstr[]={' ','k','M','G','T'};

void print_to_fd(int fd, char *str)
{
	write(fd,str,strlen(str)+1);
}

// 1000 31032020
#define MAXAVER	1000

char *formatnum(unsigned int val, const char *unit) {
	int un=0;

	while(val>MAXAVER && un<4) {
		val/=MAXAVER;
		un++;
	}

	sprintf(buffer,"%d %c%s",(int) val, unstr[un],unit);

	return buffer;
}

/** Print processes list
 *
 * Prints processes to file descriptor as parameter
 * (set to 1 for stdout)
 *
 */
int proc_print(int fd, struct proc_info *procs, int nof_procs)
{
	int i,n;
	char *t,*t2,*t3,*t4,*t5;
	int totalcpu=0,meanperiod=0,maxcpu=0, namelength=0;
	float totalmac=0,totalmaxmac=0;
	static int tsSTAMP=0;
	static struct proc_info averPROCS[NOF_PROCS];
	static int first = 0;
	static int nof_aver=0;
	static int averTotalCPU=0; 

	nof_aver++;

	if(first==0){
		averPROCS[i].cpu_usec = 0;
		averPROCS[i].max_usec = 0;
		first=1;
	}


	if(nof_procs > NOF_PROCS){
		printf("cmdman_prints.c:proc_print(): ERROR!!! nof_procs=%d > NOF_PROCS=%d\n", nof_procs, NOF_PROCS);
		exit(0);
	}

	if(procs[0].tstamp <= tsSTAMP){
		printf("WARNING!!! Try again. Still in new data capture stage\n");
		printf("WARNING!!! tsSTAMP=%d, procs[0].tstamp=%d\n", tsSTAMP, procs[0].tstamp);
		return(1);
	}
	tsSTAMP=procs[0].tstamp;








	//ORIGINAL
/*	sprintf(pstr, "\nPE\tName\t\tPRD\tTSTAMP\tRLQ\t(Max)\tCPU\t(Max)\t%% CPU\tKOPTS\t(Max)\tRTFAULT\n");
	print_to_fd(fd, pstr);
*/
	sprintf(pstr, "\033[0;31m\nCPU:\tModule\t\t\t\t\tTSlot\tCapture\tRelinquish\tCPU used (usec)\t%% CPU \tNum KOPTS\tReal Time\n");
	print_to_fd(fd, pstr);
	sprintf(pstr, "Core\tName\t\t\t\t\t(usec)\tTSlot\tAverage\tMax\tAverage\tMax\tAverage\tAverage\tMax\tFailures\n");
	print_to_fd(fd, pstr);

	n=strlen(pstr); //-3+6*7;
	n=17*8;
	for (i=0;i<n;i++)
		sprintf(&pstr[i], "=");
	sprintf(&pstr[i], "\n");
	print_to_fd(fd, pstr);
        
	//printf("nof_procs=%d\n", nof_procs);

	for (i = 0; i < nof_procs; i++) {
		if (procs[i].obj_id) {
/*			if (strlen(procs[i].name)<8)
				t="\t\t";
			else
				t="\t";
*/
			namelength=strlen(procs[i].name);
			if(namelength<24)t="\t";
			if(namelength<16)t="\t\t";
			if(namelength<8)t="\t\t\t";

			averPROCS[i].cpu_usec += procs[i].cpu_usec;
			averPROCS[i].max_usec += procs[i].max_usec;		//procs[i].max_usec,


			//sprintf(pstr, "%d:%d\t%s%s%d\t%d\t%d\t%d\t%d\t%d\t%.2f\t%.2f\t%.2f\t%d\n",
			sprintf(pstr, "%d:%d\t%s\t\%s\t%d\t%d\t%d\t%d\t%d\t%d\t%.2f\t%.1f\t%.1f\t\t%d\n",
                    procs[i].pe_id,procs[i].core_id,
					procs[i].name, t,
					procs[i].period,
					procs[i].tstamp,
					procs[i].end_usec,
					procs[i].max_end_usec,
					procs[i].cpu_usec,
					procs[i].max_usec,
					//averPROCS[i].cpu_usec/nof_aver,
					//averPROCS[i].max_usec/nof_aver,
					 ((float)procs[i].cpu_usec*100.0)/(float)procs[i].period,
					procs[i].mean_mops,
					procs[i].max_mops,
					procs[i].nof_faults);

			print_to_fd(fd, pstr);
			totalcpu+=procs[i].cpu_usec;
			maxcpu+=procs[i].max_usec;
			totalmac+=procs[i].mean_mops;
			totalmaxmac+=procs[i].max_mops;
			meanperiod+=procs[i].period;
		}
	}
        if (i)
            meanperiod/=i;
        sprintf(pstr, "%d", totalcpu);
        if (strlen(pstr)<8)
                t3="\t\t";
        else
                t3="\t";


        sprintf(pstr, "%d", maxcpu);
        if (strlen(pstr)<8)
                t5="\t\t";
        else
                t5="\t";

		averTotalCPU += totalcpu;

        sprintf(pstr, "TOTAL:\t\t\t\t\t\t\t\t\t\t%d\t%d\t%.2f%%\t%.2f\t%.2f\n\033[0m",
                        totalcpu,
						//averTotalCPU/nof_aver,
                        maxcpu,
                        (float) (100*totalcpu)/meanperiod,
                        totalmac,
                        totalmaxmac);
        print_to_fd(fd, pstr);
	for (i=0;i<n;i++)
		pstr[i] = '=';
	pstr[i++] = '\n';
	pstr[i++] = '\0';
	print_to_fd(fd,pstr);

	return 1;
}


int apps_print(int fd, struct app_info *apps, int nof_apps)
{
	int i,n;
	char *t;

	sprintf(pstr, "\nId\tName\t\tStatus\tRT Faults\n");
	print_to_fd(fd, pstr);
	n=strlen(pstr)-3+3*8;
	for (i=0;i<n;i++)
		sprintf(&pstr[i], "=");
	sprintf(&pstr[i], "\n");
	print_to_fd(fd, pstr);

	for (i = 0; i < nof_apps; i++) {
		if (apps[i].app_id) {
			if (strlen(apps[i].name)<8)
				t="\t\t";
			else
				t="\t";

			sprintf(pstr, "%d\t%s%s%d\t%d\n",
					apps[i].app_id,
					apps[i].name, t,
					apps[i].status,
                                        apps[i].nof_rtfaults);
			print_to_fd(fd, pstr);
		} 
	}
	
	for (i=0;i<n;i++)
		pstr[i] = '=';
	pstr[i++] = '\n';
	pstr[i++] = '\0';
	print_to_fd(fd,pstr);
	
	return 1;
}

int pe_print(int fd, struct pe_info *pe, int nof_pe)
{
	int i,j,n;
	char *t;

	sprintf(pstr, "\nName\t\tId\tPlat family\tC\tC'\tBW\tBW'\n");
	print_to_fd(fd, pstr);
	n=strlen(pstr)-3+7*8;
	for (i=0;i<n;i++)
		sprintf(&pstr[i], "=");
	sprintf(&pstr[i], "\n");
	print_to_fd(fd, pstr);

	for (i = 0; i < nof_pe; i++) {
		if (strlen(pe[i].name)<8)
			t="\t\t";
		else
			t="\t";

		sprintf(pstr, "%s%s%d\t\t%d\t%.2f\t%.2f\t%.2f\t%.2f \n",
				pe[i].name,t,
				pe[i].id,
				pe[i].plat_family,
				pe[i].C,
				pe[i].totalC,
				pe[i].intBW,
                                pe[i].totalintBW);
		print_to_fd(fd, pstr);
	}
	
	for (i=0;i<n;i++)
		pstr[i] = '=';
	pstr[i++] = '\n';
	pstr[i++] = '\0';
	print_to_fd(fd,pstr);


	sprintf(pstr, "\nPE:ItfId\t->\tPE:ItfId\tintBW\tintBW'\n");
	print_to_fd(fd, pstr);
	n=strlen(pstr)-3+6*8;
	for (i=0;i<n;i++)
		sprintf(&pstr[i], "=");
	sprintf(&pstr[i], "\n");
	print_to_fd(fd, pstr);

	for (i = 0; i < nof_pe; i++) {
            for (j=0;j<pe[i].nof_xitf;j++) {
		sprintf(pstr, "0x%x:0x%x\t->\t0x%x:0x%x\t%.2f\t%.2f\n",
				pe[i].xitf[j].remote_pe,pe[i].xitf[j].xitf.id,
				pe[i].id,pe[i].xitf[j].remote_id,
				pe[i].xitf[j].xitf.BW, pe[i].xitf[j].totalBW);
		print_to_fd(fd, pstr);
            }
	}

	for (i=0;i<n;i++)
		pstr[i] = '=';
	pstr[i++] = '\n';
	pstr[i++] = '\0';
	print_to_fd(fd,pstr);
	
	return 1;
}

int stats_print(int fd, struct stat_info *stats, int nof_stats)
{
	int i,n;
	char *t2,*t3;


	sprintf(pstr, "\nObjName\t\tStatName\tSize\tType\n");
	print_to_fd(fd, pstr);
	n=strlen(pstr)-3+2*8;
	for (i=0;i<n;i++)
		sprintf(&pstr[i], "=");
	sprintf(&pstr[i], "\n");
	print_to_fd(fd, pstr);
	
	for (i = 0; i < nof_stats; i++) {
		if (strlen(stats[i].objname)>=8) {
			t2="\t";
		} else {
			t2="\t\t";
		}
		if (strlen(stats[i].statname)>=8) {
			t3="\t";
		} else {
			t3="\t\t";
		}
		sprintf(pstr, "%s%s%s%s%d\t%d\n", 
				stats[i].objname,t2,
				stats[i].statname,t3,
				stats[i].size,
				stats[i].type);
		print_to_fd(fd, pstr);
	}
	
	for (i=0;i<n;i++)
		pstr[i] = '=';
	pstr[i++] = '\n';
	pstr[i++] = '\0';
	print_to_fd(fd,pstr);
	
	return 1;
}

