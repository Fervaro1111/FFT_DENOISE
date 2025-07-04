/*
 * hwman_cmds.c
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>. All rights reserved.
 *
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
#include <stdlib.h>
#include <string.h>
#include <mcheck.h>
#include "phal_hw_api.h"

#include "phid.h"
#include "phal_daemons.h"

/* common objects */
#include "set.h"
#include "str.h"
#include "var.h"
#include "app.h"
#include "rtcobj.h"
#include "phobj.h"
#include "phitf.h"
#include "pe.h"
#include "xitf.h"
#include "pkt.h"
#include "daemon.h"

#include "swload_cmds.h"
#include "swman_cmds.h"
#include "hwman_cmds.h"
#include "cmdman_cmds.h"
#include "frontend_cmds.h"
#include "bridge_cmds.h"

#include "mapper.h"


/**@TODO: Internal memory bandwidth resource modeling
 */

#define MAX_SLAVE_PE		10
pe_o peobj[MAX_SLAVE_PE];
app_o apppe[MAX_SLAVE_PE];
app_o appcores[MAX_SLAVE_PE];

void appdist_delete(void **x);

#define NOF_XITF 500
#define NOF_OBJ	200
#define NOF_ITF	1000
#define NOF_STR	5000
#define NOF_VAR	500
#define NOF_APP 20
#define NOF_SET	1000
#define NOF_SETMEM 1000
#define NOF_PE 32

#define MAX_MODULES 300
float c[MAX_MODULES];
#define MAX_PROCS 300
float C[MAX_PROCS];
int P_m[MAX_MODULES];

void hwman_alloc()
{
#ifdef MTRACE
    mtrace();
#endif
    hwapi_mem_prealloc(NOF_OBJ, phobj_sizeof);
    hwapi_mem_prealloc(NOF_OBJ, rtcobj_sizeof);
    hwapi_mem_prealloc(NOF_ITF, phitf_sizeof);
    hwapi_mem_prealloc(NOF_XITF, xitf_sizeof);
    hwapi_mem_prealloc(NOF_STR, str_sizeof);
    hwapi_mem_prealloc(NOF_APP, app_sizeof);
    hwapi_mem_prealloc(NOF_VAR, var_sizeof);
    hwapi_mem_prealloc(NOF_SET, set_sizeof);
    hwapi_mem_prealloc(NOF_SETMEM, setmember_sizeof);
    hwapi_mem_prealloc(NOF_PE, pe_sizeof);
    hwapi_mem_prealloc(1, daemon_sizeof);
    hwapi_mem_prealloc(1, pkt_sizeof);
}

struct bw_matrix_itf {
    int loc_id;
    int opp_id;
    float B;
};

struct bw_matrix_itf BW[MAX_SLAVE_PE][MAX_SLAVE_PE];

Set_o pedb = NULL;
Set_o loaded_apps = NULL;

daemon_o daemon;
pkt_o pkt;

struct platform_resources plat;
struct waveform_resources wave;
struct preprocessing preproc;
struct mapping_algorithm malg;
struct cost_function costf;
struct mapping_result result;

/** only one timeslot per platform is supported */
int tslen_usec;

struct hwapi_cpu_i local_cpu;

int hwman_init(daemon_o d)
{
    int i;

    wave.c = c;
    wave.force = calloc(1, sizeof (int) * MAX_MODULES);
    wave.b = calloc(1, sizeof (float*) * MAX_MODULES);
    for (i = 0; i < MAX_MODULES; i++) {
        wave.b[i] = calloc(1, sizeof (float) * MAX_MODULES);
    }
    plat.C = C;
    plat.B = calloc(1, sizeof (float*) * MAX_PROCS);
    for (i = 0; i < MAX_PROCS; i++) {
        plat.B[i] = calloc(1, sizeof (float) * MAX_PROCS);
    }
    result.P_m = P_m;

    tslen_usec=0;

    pedb = Set_new(0, NULL);
    loaded_apps = Set_new(0, NULL);

    hwapi_hwinfo_cpu(&local_cpu);

    daemon = d;
    pkt = daemon_pkt(daemon);

    return (pedb && loaded_apps) ? 1 : 0;
}

void calc_m_vector(Set_o objects)
{
    phobj_o obj;
    int i,j;

    assert(objects);

    memset(wave.c,0,sizeof (float) * MAX_MODULES);
    memset(wave.force,0,sizeof (int) * MAX_MODULES);

    for (i = 0; i < Set_length(objects); i++) {
        j=Set_length(objects)-1-i;
        obj = Set_get(objects, j);
        obj->exec_position=i; /* We use exec_position to save the position in the c vector */
        wave.c[i] = obj->rtc->mops_req/1000;
        wave.force[i] = obj->force_pe;
    }
}

void calc_b_matrix(Set_o objects)
{
    int i, j, k;
    phobj_o obj_r, obj_w;
    phitf_o itf;

    assert(objects);

    for (i = 0; i < MAX_MODULES; i++) {
		memset(wave.b[i],0,sizeof (float) * MAX_MODULES);
	}

    for (i = 0; i < Set_length(objects); i++) {
        obj_r = Set_get(objects, i);
        for (j = 0; j < Set_length(obj_r->itfs); j++) {
            itf = Set_get(obj_r->itfs, j);
            if (phitf_iswriteonly(itf)) {
                for (k = 0; k < Set_length(objects); k++) {
                    obj_w = Set_get(objects, k);
                    if (!str_cmp(obj_w->objname, itf->remoteobjname)) {
                        wave.b[Set_length(objects)-1-i][Set_length(objects)-1-k] += (float) itf->bw_req/1000;
                    }
                }
            }
        }
    }
}

void calc_P_vector(void)
{
    int i;
    pe_o pe;
    int len = Set_length(pedb);
    int ts=0;


    plat.nof_processors = len;

    for (i = len-1; i >= 0; i--) {
        pe = Set_get(pedb, i);
        assert(pe);
		plat.C[pe->P_idx] = pe->C;
    }
}

void calc_BW_matrix(void)
{
    int i, j, I,J,len;
    pe_o src_pe, dst_pe;
    xitf_o dst_itf;
    int update;

    memset(BW, 0, sizeof (struct bw_matrix_itf) * MAX_SLAVE_PE * MAX_SLAVE_PE);

    len = Set_length(pedb);

    for (I = 0; I < Set_length(pedb); I++) {
        src_pe = Set_get(pedb,I);
        assert(src_pe);
        i=src_pe->P_idx;
        src_pe->BW_idx = i;
        for (J = 0; J < Set_length(pedb); J++) {
			dst_pe = Set_get(pedb,J);
			assert(dst_pe);
			j=dst_pe->P_idx;
			if (i == j) {
				BW[i][i].loc_id = 0;
				BW[i][i].opp_id = 0;
				BW[i][i].B = src_pe->intBW;
				plat.B[i][i] = BW[i][i].B;
			} else {

				/* multi-core case */
				if (src_pe->id == dst_pe->id) {
					BW[i][j].loc_id = 0;
					BW[i][j].opp_id = 0;
					BW[i][j].B = src_pe->intBW;
					plat.B[i][j] = BW[i][j].B;
					BW[j][i].loc_id = 0;
					BW[j][i].opp_id = 0;
					BW[j][i].B = src_pe->intBW;
					plat.B[j][i] = BW[j][i].B;
				/* single core, remote processors */
				} else {
					/* Find if src_pe has dst_de as an attached remote processor */
					dst_itf = Set_find(src_pe->xitfs, &dst_pe->id, xitf_findremotepe);
					if (dst_itf) {
						/* Now B(j,i) indicates connection between dst->src */
						BW[j][i].loc_id = dst_itf->xitf.id;
						BW[j][i].opp_id = dst_itf->remote_id;
						BW[j][i].B = dst_itf->xitf.BW;
						plat.B[j][i] = BW[j][i].B;
						if (!plat.B[j][i])
							plat.B[j][i] = (float) dst_itf->xitf.BW;
					}
				}
			}
		}
    }
}

void print_m_vector()
{
    int i;
    xprintf("\t-- Required computing requiremenst (c vector in KOPTS) --\n\t");
    for (i = 0; i < wave.nof_tasks; i++) {
        xprintf("%.1f\t", wave.c[i]*1000);
    }
    printf("\n");
}

void print_b_matrix()
{
    int i, j;
    xprintf("\t-- Required interfaces bandwidth (b matrix in KBPTS) --\n\t");
    for (i = 0; i < wave.nof_tasks; i++) {
        for (j = 0; j < wave.nof_tasks; j++) {
       		printf("%.3f\t", wave.b[i][j]*1000);
        }
        printf("\n\t");
    }
    printf("\n");
}

void print_P_vector(int len)
{
    int i;
    xprintf("\t-- Available computing resources (C vector in KOPTS) --\n\t");
    for (i = 0; i < len; i++) {
        xprintf("%.0f\t\t", plat.C[i]*1000);
    }
    printf("\n");
}

void print_Pres_vector(int len)
{
    int i;
    xprintf("\t-- Mapping Result (P_m resulting vector) --\n");
	xprintf("\tModule Num");
	for (i = 0; i < len; i++){
		xprintf("%8d",i);
	}

	xprintf("\n\tProcessor:");
/*	xprintf("\t");*/
    for (i = 0; i < len; i++) {
        xprintf("%8d", result.P_m[i]);
    }
    printf("\n");

/*   int i;
    xprintf("\t-- Mapping Result (P_m resulting vector) --\n\t");
	xprintf("\t");
	for (i = 0; i < len; i++){
		xprintf("\tModule%d",i);
	}
	xprintf("\n\tProcessor:");
	xprintf("\t");
    for (i = 0; i < len; i++) {
        xprintf("%d\t", result.P_m[i]);
    }
    printf("\n");
*/
}

char auxstr[32];

void print_BW_matrix(int len)
{
    int i, j;
    char *t;
    xprintf("\t-- Available interfaces bandwidth (B matrix in MBPTS) --\n\t");
    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
          sprintf(auxstr,"%.2f",plat.B[i][j]);
          if (strlen(auxstr)>7) {
            t="\t";
          } else {
            t="\t\t";
          }
          xprintf("%s%s", auxstr,t);
        }
        xprintf("\n\t");
    }
    xprintf("\n");
}

void newson_error()
{
    /** @todo Handle errors of new sons
     */
}

void swmapper_error(app_o app)
{
    pkt_clear(pkt);
    pkt_put(pkt, FIELD_APPNAME, app->name, str_topkt);
    daemon_sendto(daemon, SWMAN_MAPAPPERROR, pkt_getsrcpe(pkt), DAEMON_SWMAN);
}

int check_mapping(int length)
{
    int i;

    for (i = 0; i < length; i++) {
        if (result.P_m[i] < 0) {
            return 0;
        }
    }

    return 1;
}

float mapping(app_o app, float g)
{
    int i;
    preproc.ord = no_ord;
    malg.type = tw;
    malg.w = 4;
    costf.mhop = 0;
    costf.q = g;
    plat.arch = fd;

    return mapper(&preproc, &malg, &costf, &plat, &wave, &result);
/*    for (i=0;i<Set_length(app->objects);i++) {
	if (i<Set_length(pedb)) {
		result.P_m[i] = i;
	} else {
		printf("Caution, mapping more objects (%d) than processors (%d)\n", Set_length(app->objects), Set_length(pedb));
	}
     }
 */
}

/** Perform the task-to-processor mapping. Then, send the objects to each processor, taking into account that tasks
 * allocated to a multi-core must be sent in a single packet.
 */
int do_mapping(app_o app, float g)
{
    phobj_o obj_r, obj_w;
    phitf_o itf_r, itf_w;
    pe_o pe_r, pe_w,pe_tmp;
    int pe_r_idx, pe_w_idx;
    app_o dest_app, src_app;
    int i, j, k;
    float cost;
    xitf_o xitf;

    /* Print platform model before mapping */
    wave.nof_tasks = Set_length(app->objects);
    calc_b_matrix(app->objects);
    calc_m_vector(app->objects);

	printf("O============================================================================================O\n");
	printf("\t\033[1;34m APP Computing Requirements\033[0m\n");
    print_b_matrix();
    print_m_vector();
    printf("\n\n");
	printf("\t\033[1;34m PLATFORM Available Computing Resources\033[0m\n");
    print_BW_matrix(plat.nof_processors);
    print_P_vector(plat.nof_processors);
    printf("\n");
	

    /* Do the mapping with the q-value g */
    cost = mapping(app, g);

/*	printf("\n\tq-value=%3.1f, cost=%3.4f\n", g, cost);
*/

    /* Check mapping result */
    if (cost < 0) {
        return 0;
    } else if (cost >= infinite - 1) {
        daemon_error(daemon, "Platform does not have enough resources for the Waveform.");
        return 0;
    }
    daemon_info(daemon, "\n\t\033[1;34mMapped with cost %3.5f and using a q-value=%3.2f\033[0m", cost, g);
    if (!check_mapping(Set_length(app->objects))) {
        daemon_error(daemon, "Some objects were not mapped\n");
        return 0;
    }

    /* Print mapping */
    print_Pres_vector(Set_length(app->objects));


    /* dest_app is the set of objects that will be sent to each processor */
    for (i = 0; i < Set_length(pedb); i++) {
        dest_app = apppe[i];
        if (dest_app) Set_clear(dest_app->objects);
    }

    /* Here we perform the object-to-processor mapping */
    for (i = 0; i<Set_length(app->objects); i++) {
    	obj_w = Set_get(app->objects, i);
    	assert(obj_w);

    	/* get the processor object given the mapping result */
        pe_w = peobj[result.P_m[obj_w->exec_position]];
        assert(pe_w);

        /* Multi-cores point to the same dest_app in apppe[]. appcores[] then indicate the core-to-processor mapping */
        dest_app = appcores[pe_w->P_idx];
        assert(dest_app);

        /* Remove C resources from the processor */
		pe_w->C-=(float) obj_w->rtc->mops_req/1000;

		/* Configure object settings */
		obj_w->pe_id = pe_w->id;
		obj_w->core_idx = pe_w->core_id;

		/* Default executing position is reversed. Objects come out from the set in reverse order */
		obj_w->exec_position = pe_w->nof_objects++;

		if (pe_w->exec_order==SCHED_ORDERED) {
			daemon_info(daemon, "Warning: Processor 0x%x requests ordered execution, but is not implemented. Setting to reversed order.\n",obj_w->pe_id);
		}

		/* Put into application. Dest_app is a set of pointers, therefore we can modify the object contents after the mapping */
		Set_put(dest_app->objects, obj_w);
	}

    /* Now we have to configure interfaces in the case linked objects are in different processors */
    for (i = 0; i<Set_length(app->objects); i++) {

		/* Now go through all objects regardless of the execution order */
		obj_w = Set_get(app->objects, i);
		assert(obj_w);

		/* Get the processor */
		pe_w = Set_find2(pedb, &obj_w->pe_id, &obj_w->core_idx, pe_findid, pe_findcoreid);
		if (!pe_w) {
			daemon_error(daemon, "%d: Can't find PE 0x%x:%d.", i, obj_w->pe_id);
			return 0;
		}

		/* Go through all object interfaces */
		for (k = 0; k < Set_length(obj_w->itfs); k++) {
			itf_w = Set_get(obj_w->itfs, k);
			assert(itf_w);

			/* and if it is an output interface */
			if (phitf_iswriteonly(itf_w)) {

				/* Find remote object */
				obj_r = Set_find(app->objects, itf_w->remoteobjname, phobj_findobjname);
				if (!obj_r) {
					daemon_error(daemon, "Remote object %s not found", str_str(itf_w->remoteobjname));
					return 0;
				}
				/* And the processor where the remote object has been mapped */
				pe_r = Set_find2(pedb, &obj_r->pe_id, &obj_r->core_idx, pe_findid,pe_findcoreid);
				if (!pe_r) {
					daemon_error(daemon, "%d: Can't find PE 0x%x:%d. obj_w %s itf_w %s obj_r %s", i,obj_r->pe_id, obj_r->core_idx,str_str(obj_w->objname),str_str(itf_w->name),str_str(obj_r->objname));
					return 0;
				}

				pe_r_idx = pe_r->BW_idx;
				pe_w_idx = pe_w->BW_idx;

				/* Now if the object is in another processor, we need to configure external interfaces */
				if (!itf_w->xitf_id && obj_r->pe_id != obj_w->pe_id) {
					/* find the remote object interface */
					itf_r = Set_find(obj_r->itfs, itf_w->remotename, phitf_findname);
					if (!itf_r) {
						daemon_error(daemon, "Remote object %s does not have any itf with name %s", str_str(itf_w->remoteobjname), str_str(itf_w->remotename));
						return 0;
					}

					/* Source interface goes through external interface BW[pe_r_idx][pe_w_idx].loc_id */
					itf_w->xitf_id = BW[pe_w_idx][pe_r_idx].loc_id;

					/* And destination interfaces reads from interface BW[pe_r_idx][pe_w_idx].opp_id */
					itf_r->xitf_id = BW[pe_w_idx][pe_r_idx].opp_id;

					/* Now setup destination and source for configuring the bridge */
					itf_w->remote_obj_id = obj_r->obj_id;
					itf_w->remote_id = itf_r->id;
					itf_r->remote_obj_id = obj_w->obj_id;
					itf_r->remote_id = itf_w->id;

					/* It only remains to substract the available interface bandwidth from the writer processor and interface*/
					xitf = Set_find(pe_r->xitfs, &itf_w->xitf_id, xitf_findid);
					if (!xitf) {
						daemon_error(daemon, "Can't find itf id 0x%x (%d,%d) (0x%x,0x%x)\n", itf_w->xitf_id, pe_w_idx,pe_r_idx,pe_w->id,pe_r->id);
						return 0;
					}
					xitf->xitf.BW -= itf_w->bw_req/1000;

				} else {
					/* If the object is in the same processor (same core or not), just substract remaining internal bandwidth */
					for (j=0;j<Set_length(pedb);j++) {
						pe_tmp = Set_get(pedb,j);
						assert(pe_tmp);
						if (obj_r->pe_id==pe_w->id) {
							pe_tmp->intBW -= itf_w->bw_req/1000;
						}
					}
				}
			}
		}
    }

    /* Finally, print resulting platform model after mapping */
    printf("\n");
    calc_P_vector();
    calc_BW_matrix();
    print_BW_matrix(plat.nof_processors);
    print_P_vector(plat.nof_processors);
    printf("\n");
    return 1;
}

int hwman_incmd_additf(cmd_t *cmd)
{
	int i;
    pe_o pe;
    xitf_o xitf;
    peid_t peid;

	xitf = pkt_get(pkt, FIELD_XITF, xitf_newfrompkt);
	if (!xitf) {
		daemon_error(daemon, "Invalid packet");
		return 0;
	}

	peid = (peid_t) pkt_getvalue(pkt,FIELD_PEID);

	pe = Set_find(pedb, &peid, pe_findid);
	if (!pe) {
		daemon_error(daemon, "Error can't find PE 0x%x\n", peid);
		return 0;
	}

	xitf->xitf.BW*=(float) pe->tslen_us/1000000;
	xitf->totalBW = xitf->xitf.BW;

	if (pe->nof_cores>1) {
		xitf->xitf.BW/=pe->nof_cores;
		xitf->totalBW/=pe->nof_cores;
		for (i=0;i<Set_length(pedb);i++) {
			pe = Set_get(pedb,i);
			if (pe->id == peid) {
				if (Set_find(pe->xitfs, &xitf->xitf.id, xitf_findid)==NULL) {
					Set_put(pe->xitfs, xitf);
				}
			}
	    }
	} else {
		if (Set_find(pe->xitfs, &xitf->xitf.id, xitf_findid)==NULL) {
			Set_put(pe->xitfs, xitf);
		}
	}


	daemon_info(daemon, "Processed new itf id 0x%x:0x%x->0x%x:0x%x BW=%.2f MBTS", xitf->remote_pe,xitf->xitf.id,peid,xitf->remote_id,xitf->xitf.BW);

	calc_BW_matrix();
	print_BW_matrix(plat.nof_processors);

    return 1;
}

int send_total_cpu()
{
    pe_o pe, tmppe;
    int i;
    float sum_cpu;

    struct hwapi_cpu_i cpu_i;

    pe = pe_new();
    if (!pe) {
        return 0;
    }

    sum_cpu = 0;
    for (i = 0; i < plat.nof_processors; i++) {
        sum_cpu += plat.C[i];
    }

    hwapi_hwinfo_cpu(&cpu_i);

    pe->id = cpu_i.pe_id;
    pe->C = sum_cpu;
    pe->is_hwman = 1;
    
    pkt_clear(pkt);

    pkt_put(pkt, FIELD_PE, pe, pe_topkt);
#ifdef DEB
    daemon_info(daemon, "Sending total cpu %.3f MOPS upwards, %d processors", pe->C,plat.nof_processors);
#endif
    daemon_sendto(daemon, HWMAN_UPDCPU, 1, DAEMON_HWMAN);

    pe_delete(&pe);
    return 1;
}

void send_sons_ident()
{
    pe_o pe;
    int i;

    pkt_clear(pkt);

    for (i = 0; i < Set_length(pedb); i++) {
        pe = Set_get(pedb, i);
#ifdef DEB
        daemon_info(daemon, "Sending indent packet to 0x%x", pe->id);
#endif
        daemon_sendto(daemon, BRIDGE_IDENT, pe->id, DAEMON_BRIDGE);
    }
}

int pedb_topkt(void *x, char **start, char *end)
{
    return Set_topkt(x, start, end, pe_topkt);
}

int hwman_incmd_listcpu(cmd_t *cmd)
{
    pkt_clear(pkt);
    daemon_info(daemon, "Sending cpu information");
    pkt_put(pkt, FIELD_PELIST, pedb, pedb_topkt);
    daemon_sendto(daemon, CMDMAN_LISTCPUOK, pkt_getsrcpe(pkt), DAEMON_CMDMAN);
    return 1;
}

int hwman_incmd_updcpu(cmd_t *cmd)
{
	int i;
    pe_o pe, tmppe;

    tmppe = pkt_get(pkt, FIELD_PE, pe_newfrompkt);
    if (!tmppe) {
        daemon_error(daemon, "Invalid packet");
        newson_error();
        return 0;
    }

    /* set last update for this pe  */
    for (i=0;i<Set_length(pedb);i++) {
    		pe = Set_get(pedb,i);
    		if (pe->id == tmppe->id) {
    	        tmppe->tstamp = get_tstamp();
    	        pe->tstamp = tmppe->tstamp;
    		}
    }

    pe_delete(&tmppe);

    return 1;
}

int hwman_incmd_addcpu(cmd_t *cmd)
{
    pe_o pe, tmppe,tmppe2;
    char buff[8];
    int i;

    tmppe = pkt_get(pkt, FIELD_PE, pe_newfrompkt);
    if (!tmppe) {
        daemon_error(daemon, "Invalid packet");
        newson_error();
        return 0;
    }

    /* overwrite existing one with same id, if any */
    tmppe->tstamp = get_tstamp();
    pe = Set_find(pedb, &tmppe->id, pe_findid);
    if (pe) {
    	tmppe->C*=tmppe->tslen_us/1000000;
    	tmppe->intBW*=tmppe->tslen_us/1000000;
        daemon_info(daemon, "CPU updated for PE 0x%x (%.3f MOPTS, TS=%d)", pe->id, tmppe->C, tmppe->tstamp);
        Set_remove(pedb, pe);
        pe_delete(&pe);
    } else {
    	tmppe->C*=(float) tmppe->tslen_us/1000000;
    	tmppe->intBW*=(float) tmppe->tslen_us/1000000;
        tmppe->totalC = tmppe->C;
        tmppe->totalintBW = tmppe->intBW;
        tmppe->P_idx=Set_length(pedb);
        tmppe->core_id=0;
        if (!tslen_usec) {
        	tslen_usec=tmppe->tslen_us;
        } else {
        	if (tslen_usec!=tmppe->tslen_us) {
        		daemon_error(daemon, "Only one timeslot length per platform is supported");
        	}
        }


        Set_clear(tmppe->xitfs);
        daemon_info(daemon, "\n\t\033[1;34mNew CPU: PE 0x%x, %.3f MOPTS, BW %.1f MBPTS, %d cores, TS=%d us\033[0m", tmppe->id, tmppe->C, tmppe->intBW, tmppe->nof_cores,tmppe->tslen_us);

        apppe[Set_length(pedb)] = app_new();
		assert(apppe[Set_length(pedb)]);
		appcores[Set_length(pedb)]=apppe[Set_length(pedb)];
		peobj[Set_length(pedb)] = tmppe;
        Set_put(pedb, tmppe);

        for (i=1;i<tmppe->nof_cores;i++) {
			tmppe2 = pe_dup(tmppe);
			tmppe2->core_id=i;
			tmppe2->P_idx = Set_length(pedb);
			sprintf(buff,"_%d",i);
			str_cat(tmppe2->name,buff);
			appcores[Set_length(pedb)]=apppe[tmppe->P_idx];
			peobj[Set_length(pedb)] = tmppe2;
			Set_put(pedb,tmppe2);
		}
        if (tmppe->nof_cores>1) {
        	str_cat(tmppe->name,"_0");
        }

    }

    calc_P_vector();
    calc_BW_matrix();

    print_P_vector(plat.nof_processors);

    pkt_clear(pkt);

    /* send new info to parent */
    send_total_cpu();

    /**@todo Really need this sleep? */
    sleep_ms(500);

    /* send ident broadcast to sons */
    send_sons_ident();

    return 1;
}

void appdist_delete(void **x)
{
    app_o *app = (app_o*) x;
    Set_clear((*app)->objects);
    app_delete(&(*app));
}

int hwman_incmd_swmapper(cmd_t *cmd)
{
	float g,*x;
    app_o app, tmpapp;
    phobj_o tmpobj;
    pe_o pe;
    int i,j;

    app = pkt_get(pkt, FIELD_APP, app_newfrompkt);
    if (!app) {
        daemon_error(daemon, "Invalid packet.");
        swmapper_error(NULL);
        return 0;
    }

    x = pkt_getptr(pkt, FIELD_GCOST);
    g = *x;

/*printf("hwman_cmds.c:hwman_incmd_swmapper(): g=%3.1f \n", g);*/

    if (!do_mapping(app,g)) {
        swmapper_error(app);
        app_delete(&app);
        return 0;
    }

    Set_put(loaded_apps, app);

    for (i = 0; i < Set_length(pedb); i++) {
        tmpapp = apppe[i];
        if (tmpapp) {
			for (j=0;j<Set_length(tmpapp->objects);j++) {
				tmpobj = Set_get(tmpapp->objects,j);
			}
			if (Set_length(tmpapp->objects)) {
				tmpobj = Set_get(tmpapp->objects, 0);
				if (tmpobj) {
					pe = Set_find(pedb, &tmpobj->pe_id, pe_findid);
					if (!pe) {
						daemon_error(daemon, "Can't find pe 0x%x", tmpobj->pe_id);
					}

					pkt_clear(pkt);
					pkt_put(pkt, FIELD_APP, tmpapp, app_topkt);

					if (pe->is_hwman) {
						daemon_sendto(daemon, HWMAN_MAPOBJ, pe->id, DAEMON_HWMAN);
					} else {
						daemon_sendto(daemon, SWLOAD_LOADOBJ, pe->id, DAEMON_SWLOAD);
					}
				}
			}
        }
    }
    send_total_cpu();
    return 1;
}

int free_resources(app_o app, phobj_o obj)
{
    int i,j, itf_id, obj_id;
    phobj_o robj;
    phitf_o itf;
    pe_o pe,pe2;
    xitf_o xitf;

    pe = Set_find2(pedb, &obj->pe_id, &obj->core_idx, pe_findid, pe_findcoreid);
    assert(pe);
    pe->C+= (float) obj->rtc->mops_req/1000;
    pe->nof_objects--;

    for (j = 0; j < Set_length(obj->itfs); j++) {
        itf = Set_get(obj->itfs, j);
        assert(itf);

        if (!phitf_isinternal(itf)) {
            if (phitf_iswriteonly(itf)) {
            	xitf=NULL;
            	for (i=0;i<Set_length(pedb);i++) {
            		pe=Set_get(pedb,i);
            		assert(pe);
            		xitf = Set_find(pe->xitfs, &itf->xitf_id, xitf_findid);
                	if (xitf) {
                        xitf->xitf.BW += itf->bw_req/1000;
                        break;
                	}
            	}
            	if (!xitf) {
					daemon_error(daemon, "Can't find xitf 0x%x\n",itf->xitf_id);
            	}
            	obj_id=itf->remote_obj_id;
            	itf_id=itf->remote_id;
            } else {
                obj_id = obj->obj_id;
                itf_id = itf->id;
            }

            pkt_clear(pkt);
            pkt_putvalue(pkt, FIELD_OBJID, obj_id);
            pkt_putvalue(pkt, FIELD_OBJITFID, itf_id);
            pkt_putvalue(pkt, FIELD_OBJITFMODE, itf->mode);
            daemon_sendto(daemon, BRIDGE_RMITF, obj->pe_id, DAEMON_BRIDGE);

        } else {
            if (phitf_iswriteonly(itf)) {
                /* remove all core int bw */
				for (i=0;i<Set_length(pedb);i++) {
					pe = Set_get(pedb,i);
					assert(pe);
					if (obj->pe_id==pe->id) {
						pe->intBW += itf->bw_req/1000;
					}
				}

            }
        }
    }

    return 1;
}

int hwman_incmd_swunmap(cmd_t *cmd)
{
    app_o app;
    str_o app_name;
    phobj_o obj;
    int i;

    app_name = pkt_get(pkt, FIELD_APPNAME, str_newfrompkt);
    if (!app_name) {
        daemon_error(daemon, "Invalid packet");
        return 0;
    }

    app = Set_find(loaded_apps, app_name, app_findname);
    if (!app) {
        return 0;
    }

    daemon_info(daemon, "Restoring resources for Waveform %s", str_str(app_name));

    str_delete(&app_name);

    for (i = 0; i < Set_length(app->objects); i++) {
        obj = Set_get(app->objects, i);
        assert(obj);

        free_resources(app, obj);
    }

    calc_P_vector();
    calc_BW_matrix();
    print_P_vector(plat.nof_processors);
    print_BW_matrix(plat.nof_processors);
    send_total_cpu();

    Set_remove(loaded_apps, app);
    app_delete(&app);
    return 1;
}



int check_processors_last = 0;

void hwman_background(int call_tstamp)
{
    int i, tstamp;
    pe_o pe;

    if (call_tstamp<0) {
		tstamp = get_tstamp();
		if (check_processors_last > (tstamp - CHECK_PROCESSORS_PERIOD)) {
			return;
		}
    } else {
    	tstamp=call_tstamp;
    }

    check_processors_last = tstamp;
    for (i = 0; i < Set_length(pedb); i++) {
        pe = Set_get(pedb, i);
        assert(pe);
        if (pe->tstamp < tstamp - CHECK_PROCESSORS_TIMEOUT
                && pe->id != local_cpu.pe_id
                && pe->tstamp > 0) {

            xprintf("Caution Processor 0x%x is not responding, de-allocating resources (pets=%d,ts=%d)\n", pe->id, pe->tstamp, tstamp);
            Set_remove(pedb, pe);
            pe_delete(&pe);
            calc_P_vector();
            pkt_clear(pkt);
            send_total_cpu();
            send_sons_ident();
        }
    }
}





