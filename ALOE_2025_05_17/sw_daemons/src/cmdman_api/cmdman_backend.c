/*
 * cmdman_backend.c
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
#include <stdio.h>
#include <string.h>
#include <mcheck.h>
#include "phal_hw_api.h"

#include "phal_daemons.h"

#include "phid.h"
#include "pkt.h"
#include "str.h"
#include "var.h"
#include "set.h"
#include "rtcobj.h"
#include "phobj.h"
#include "app.h"
#include "pe.h"
#include "daemon.h"
#include "strdata.h"

#include "stats.h"

#include "cmdman_backend.h"

#include "frontend_cmds.h"
#include "swman_cmds.h"
#include "statsman_cmds.h"
#include "cmdman_cmds.h"
#include "exec_cmds.h"
#include "hwman_cmds.h"
#include "xitf.h"
#include "phitf.h"

void *daemon_x;
pkt_o pkt;
int reports_started = 0;


#define NOF_OBJ	200		/*100 AGBABRIL20*/
#define NOF_ITF	1000	/*100 AGBABRIL20*/
#define NOF_PE	32
#define NOF_STR	5000	/*200 AGBABRIL20*/
#define NOF_VAR	500		/*50 AGBABRIL20*/
#define NOF_APP 20		/*5 AGBABRIL20*/
#define NOF_SET	1000		/*200 AGBABRIL20*/
#define NOF_SETMEM 2000	/*1000 AGBABRIL20*/

void cmdman_alloc()
{
#ifdef MTRACE
    mtrace();
#endif
    hwapi_mem_prealloc(NOF_PE, pe_sizeof);
    hwapi_mem_prealloc(NOF_ITF, phitf_sizeof);
    hwapi_mem_prealloc(NOF_OBJ, phobj_sizeof);
    hwapi_mem_prealloc(NOF_OBJ, rtcobj_sizeof);
    hwapi_mem_prealloc(NOF_STR, str_sizeof);
    hwapi_mem_prealloc(NOF_APP, app_sizeof);
    hwapi_mem_prealloc(NOF_VAR, var_sizeof);
    hwapi_mem_prealloc(NOF_SET, set_sizeof);
    hwapi_mem_prealloc(NOF_SETMEM, setmember_sizeof);
    hwapi_mem_prealloc(1, daemon_sizeof);
    hwapi_mem_prealloc(1, pkt_sizeof);
    hwapi_mem_prealloc(1, strdata_sizeof);
}

/** Initialize cmdmanager backend
 */
int cmdman_init(void)
{

    hwapi_init();
    cmdman_alloc();
    daemon_x = daemon_new(DAEMON_CMDMAN, NULL);
    if (daemon_x) {
        pkt = daemon_pkt((daemon_o) daemon_x);
        return 1;
    } else {
        return 0;
    }

}

/** De-allocates backend
 */
void cmdmand_close()
{
    daemon_delete((daemon_o*) & daemon_x);
}

int send_wait_answer_pkt(int cmd, int daemon, int max_sec)
{
    time_t t[3];
    int n, c = 0;

    do {
        n = daemon_rcvpkt((daemon_o) daemon_x, 0);
        if (n > 0) {
            c++;
        }
    } while (n > 0);
    if (c) {
        printf("CMDMAN_BACKEND: WARNING: Possible CMDMAN malfunction, discarting %d packets in queue\n", c);
    }

    daemon_sendto((daemon_o) daemon_x, cmd, 2, daemon);

    get_time(&t[1]);
    do {
        n = daemon_rcvpkt((daemon_o) daemon_x, 0);
        if (!n) {
            get_time(&t[2]);
            get_time_interval(t);
            hwapi_relinquish_daemon();
        }
    } while (!n && t[0].tv_sec < max_sec);

    return n;
}

int wait_answer_pkt(int max_sec)
{
    time_t t[3];
    int n;

    get_time(&t[1]);
    do {
        n = daemon_rcvpkt((daemon_o) daemon_x, 0);
        if (!n) {
            get_time(&t[2]);
            get_time_interval(t);
            hwapi_relinquish_daemon();
        }
    } while (!n && t[0].tv_sec < max_sec);

    return n;
}


void * pelist_newfrompkt(char **start, char *end)
{
    return Set_newfrompkt(start, end, pe_newfrompkt);
}

void * applist_newfrompkt(char **start, char *end)
{
    return Set_newfrompkt(start, end, app_newfrompkt);
}



/** Start/Stop Execution info logs
 *
 * Starts/stops logging of CPU time consumed by executables.
 *
 * @param app_name Name of the waveform
 * @param start 1 start, 0 stop
 * @param max_sec Timeout for receiving an answer
 */
int cmdman_execlogs(char *app_name, int start, int max_sec)
{
    int cmd = start ? SWMAN_LOGSSTART : SWMAN_LOGSSTOP;

    assert(app_name && (max_sec >= 0));

    if (!reports_started) {
        if (!cmdman_execreports(app_name, 1, max_sec)) {
            return 0;
        }
        sleep_ms(100);
    }

    pkt_clear(pkt);
    str_o appname = str_snew(app_name);
    pkt_put(pkt, FIELD_APPNAME, appname, str_topkt);
    str_delete(&appname);
    
    if (send_wait_answer_pkt(cmd, DAEMON_SWMAN, max_sec)) {
        if (pkt_getcmd(pkt) == CMDMAN_REPORTOK) {
            return 1;
        }
    }

    return -1;
}

/** Start/Stop Execution info reports
 *
 * Starts/stops reporting execution information to SWMAN.
 *
 * By default it is started as soon as the waveform is loaded. Use this function
 * to enable/disable it manually if SWMAN is compiled without the AUTOMATIC_REPORTS
 * constant.
 *
 * @param app_name Waveform name
 * @param start 1 start, 0 stop
 * @param max_sec Timeout for receiving an answer
 */
int cmdman_execreports(char *app_name, int start, int max_sec)
{
    assert(app_name && (max_sec >= 0));

    if (reports_started == start) {
        return 1;
    }

    pkt_clear(pkt);
    str_o appname = str_snew(app_name);
    pkt_put(pkt, FIELD_APPNAME, appname, str_topkt);
    str_delete(&appname);

    int cmd = start ? SWMAN_REPORTSTART : SWMAN_REPORTSTOP;

    if (send_wait_answer_pkt(cmd, DAEMON_SWMAN, max_sec)) {
        if (pkt_getcmd(pkt) == CMDMAN_REPORTOK) {
            reports_started = start;
            return 1;
        }
    }
    return -1;

}


/** List Processing Elements
 *
 * Get information of all processing elements in the platform.
 *
 * @param peinfo Structure where data will be saved (see cmdman_backend.h)
 * @param max_pe Size in elements of the structure
 * @param max_sec Timeout for receiving an answer
 */
int cmdman_pelist(struct pe_info *peinfo, int max_pe, int max_sec)
{
    Set_o pe_list;
    int i, j;
    pe_o pe;
    xitf_o xitf;

    assert(peinfo);

    pkt_clear(pkt);

    if (send_wait_answer_pkt(HWMAN_LISTCPU, DAEMON_HWMAN, max_sec)) {
        if (pkt_getcmd(pkt) == CMDMAN_LISTCPUOK) {
            pe_list = pkt_get(pkt, FIELD_PELIST, pelist_newfrompkt);
            if (!pe_list) {
                return 0;
            }
            for (i = 0; i < Set_length(pe_list) && i < max_pe; i++) {
                pe = Set_get(pe_list, i);
                assert(pe);
                peinfo[i].id = pe->id;
                peinfo[i].plat_family = pe->plat_family;
                peinfo[i].nof_cores = pe->nof_cores;
                peinfo[i].C = pe->C;
                peinfo[i].intBW = pe->intBW;
                peinfo[i].totalC = pe->totalC;
                peinfo[i].totalintBW = pe->totalintBW;
                str_get(pe->name, peinfo[i].name, CPU_NAME_LEN);
                peinfo[i].nof_xitf = Set_length(pe->xitfs);
                peinfo[i].core_id = pe->core_id;
                for (j = 0; j < Set_length(pe->xitfs); j++) {
                	xitf = Set_get(pe->xitfs, j);
                    peinfo[i].xitf[j].remote_id = xitf->remote_id;
                    peinfo[i].xitf[j].remote_pe = xitf->remote_pe;
                    peinfo[i].xitf[j].totalBW = xitf->totalBW;
                    printf("total bw is %.2f\n",peinfo[i].xitf[j].totalBW);
                    memcpy(&peinfo[i].xitf[j].xitf, &xitf->xitf, sizeof (struct hwapi_xitf_i));
                }
            }
            Set_destroy(&pe_list, pe_xdelete);
            return i;
        }
    }
    return -1;
}

/** List waveforms
 *
 * Get info of all loaded waveforms in the platform
 *
 * @param apps Structure where info will be saved. (see cmdman_backend.h)
 * @param max_apps Maximum number of elements to save in the structure
 * @param max_sec Timeout for receiving an answer
 */
int cmdman_applist(struct app_info *apps, int max_apps, int max_sec)
{
    Set_o apps_list;

    assert(apps);

    pkt_clear(pkt);

    if (send_wait_answer_pkt(SWMAN_APPLS, DAEMON_SWMAN, max_sec)) {
        if (pkt_getcmd(pkt) == CMDMAN_APPLSOK) {
            apps_list = pkt_get(pkt, FIELD_APP, applist_newfrompkt);
            if (!apps_list) {
                return 0;
            }
            int i;
            app_o app;
            for (i = 0; i < Set_length(apps_list) && i < max_apps; i++) {
                app = Set_get(apps_list, i);
                assert(app);
                str_get(app->name, apps[i].name, PNAME_LEN);
                apps[i].status = app->cur_status;
                apps[i].app_id = app->app_id;
                apps[i].nof_rtfaults = app->nof_rtfaults;
            }
            Set_destroy(&apps_list, app_xdelete);
            return i;
        }
    }
    return -1;
}


/** List waveform processes
 *
 * Given a waveform, list objects and their execution information.
 * If obj_name is not NULL, get info for only one object.
 *
 * @param app_name Name of the waveform
 * @param obj_name (optional) If NULL list all objects, otherwise just one
 * @param proc Structure where to save info
 * @param max_procs Maximum number of elements to save in the structure
 * @param max_sec Timeout for receiving an answer
 */
int cmdman_appinfo(char *app_name, char *obj_name, struct proc_info *proc, int max_procs, int max_sec)
{
    int i, j;
    phobj_o obj;
    phitf_o itf;
    assert(app_name && (max_sec >= 0));

    pkt_clear(pkt);
    str_o appname = str_snew(app_name);
    pkt_put(pkt, FIELD_APPNAME, appname, str_topkt);
    str_delete(&appname);
    if (obj_name) {
        str_o objname = str_snew(obj_name);
        pkt_put(pkt, FIELD_OBJNAME, objname, str_topkt);
        str_delete(&objname);
    }
    if (send_wait_answer_pkt(SWMAN_APPINFO, DAEMON_SWMAN, max_sec)) {
        if (pkt_getcmd(pkt) == CMDMAN_APPINFOOK) {
            app_o app = pkt_get(pkt, FIELD_APP, app_newfrompkt);
            if (!app) {
                return -1;
            }
            for (i = 0; i < Set_length(app->objects) && i < max_procs; i++) {
                obj = Set_get(app->objects, i);
                assert(obj);
                str_get(obj->objname, proc[i].name, PNAME_LEN);
                proc[i].obj_id = obj->obj_id;
                proc[i].status = obj->status;
                proc[i].cpu_usec = obj->rtc->cpu_usec;
                proc[i].end_usec = obj->rtc->end_usec;
                proc[i].nvcs = obj->rtc->nvcs;
                proc[i].period = obj->rtc->mean_period;
                proc[i].max_usec = obj->rtc->max_usec;
                proc[i].max_end_usec = obj->rtc->max_end_usec;
                proc[i].max_mops = obj->rtc->max_mops;
                proc[i].mean_mops = obj->rtc->mean_mops;
                proc[i].C = obj->rtc->mops_req;
                proc[i].tstamp = obj->rtc->tstamp;
                proc[i].pe_id = obj->pe_id;
                proc[i].core_id = obj->core_idx;
                proc[i].nof_faults = obj->rtc->nof_faults;
                for (j = 0; j < Set_length(obj->itfs); j++) {
                    itf = Set_get(obj->itfs, j);
                    assert(itf);
                    str_get(itf->name, (char*) proc[i].itf[j].name, PNAME_LEN);
                    proc[i].itf[j].mode = (int) itf->mode;
                    str_get(itf->remotename, (char*) proc[i].itf[j].remote_name, PNAME_LEN);
                    str_get(itf->remoteobjname, (char*) proc[i].itf[j].remote_obj_name, PNAME_LEN);
                    proc[i].itf[j].xitf_id = (int) itf->xitf_id;
                    proc[i].itf[j].fifo_usage = (int) itf->fifo_usage;
                    proc[i].itf[j].bpts = (int) itf->bw_req;
                }
                proc[i].nof_itf = j;
            }
            app_delete(&app);
            return i;
        }
    }
    return -1;
}

/** Load a waveform
 *
 * Loads a new waveform in the platform
 *
 * @param app_name Name of the waveform, as known by SWMAN
 * @param max_sec Timeout for receiving an answer
 */
int cmdman_loadapp(char *app_name, float g, int max_sec)
{
	float *x;

    assert(app_name && (max_sec >= 0));

    pkt_clear(pkt);
    str_o appname = str_snew(app_name);
    pkt_put(pkt, FIELD_APPNAME, appname, str_topkt);
    x = pkt_putptr(pkt,FIELD_GCOST,sizeof(float));
    *x = g;
    str_delete(&appname);

    if (send_wait_answer_pkt(SWMAN_LOADAPP, DAEMON_SWMAN, max_sec)) {
        if (pkt_getcmd(pkt) == CMDMAN_SWLOADAPPOK) {
            return 1;
        }
    }
    return -1;
}

/** Change waveform status
 *
 * Change the execution status for a given waveform.
 *
 * @param app_name Name of the loaded waveform
 * @param new_status New Status constant (see phal_daemons.h)
 * @param ts_ini Timeslot when status change must start (0 for now+10)
 * @param ts_len Number of TS the new status is valid e.g. to enable multiple steps (0 for ever)
 * @param max_sec Timeout for receiving an answer
 */

#define ADDTSLOTS	1000000

int cmdman_appstatus(char *app_name, int new_status, int ts_ini, int ts_len, int max_sec)
{
    assert(app_name);

    pkt_clear(pkt);
    str_o appname = str_snew(app_name);
    pkt_put(pkt, FIELD_APPNAME, appname, str_topkt);
    str_delete(&appname);
    pkt_putvalue(pkt, FIELD_STATUS, new_status);

	/*AGB	ts_ini = get_tstamp() + 1000;*/
	/*ts_ini = get_tstamp() + 1000;*/
	ts_ini = get_tstamp() + (ADDTSLOTS/get_tslot());

    pkt_putvalue(pkt, FIELD_TSTAMP, ts_ini);
    daemon_sendto((daemon_o) daemon_x, SWMAN_APPSTATUS, 2, DAEMON_SWMAN);


    if (ts_len) {
        pkt_clear(pkt);
        pkt_put(pkt, FIELD_APPNAME, str_snew(app_name), str_topkt);
        pkt_putvalue(pkt, FIELD_STATUS, PHAL_STATUS_PAUSE);
        pkt_putvalue(pkt, FIELD_TSTAMP, ts_ini + ts_len);
        daemon_sendto((daemon_o) daemon_x, SWMAN_APPSTATUS, 2, DAEMON_SWMAN);
    }

    if (wait_answer_pkt(max_sec)) {
        if (pkt_getcmd(pkt) == CMDMAN_SWSTATUSOK) {
            return 1;
        }
    }

    if (ts_len) {
        if (wait_answer_pkt(ts_len * 100)) {
            if (pkt_getcmd(pkt) == CMDMAN_SWSTATUSOK) {
                return 1;
            }
        }
    }
    return -1;
}

/** List statistics
 *
 * List stats of an object or a waveform. If no object is given (NULL), all stats
 * of all objects in the waveform are listed.
 * Only those stats that have been initialized will be listed.
 *
 * @param app_name Name of the waveform
 * @param obj_name (optional) List only one object (non-NULL), or all (NULL)
 * @param stats Structure to save stat information (see cmdman_backend.h)
 * @param max_stats Maximum number of elements to save in the structure
 * @param max_sec Timeout for receiving an answer
 */
int cmdman_statlist(char *app_name, char *obj_name, struct stat_info *stats, int max_stats, int max_sec)
{
    app_o app;
    phobj_o obj;
    var_o var;
    int i, j, k = 0;

    assert(stats && app_name);

    pkt_clear(pkt);
    str_o appname = str_snew(app_name);
    pkt_put(pkt, FIELD_APPNAME, appname, str_topkt);
    str_delete(&appname);
    if (obj_name) {
        str_o objname = str_snew(obj_name);
        pkt_put(pkt, FIELD_OBJNAME, objname, str_topkt);
        str_delete(&objname);
    }

    if (send_wait_answer_pkt(STATSMAN_STATLS, DAEMON_STATSMAN, max_sec)) {
        if (pkt_getcmd(pkt) == CMDMAN_STATSLSOK) {
            app = pkt_get(pkt, FIELD_STATLIST, app_newfrompkt);
            if (!app) {
                return -1;
            }
            for (j = 0; j < Set_length(app->objects) && k < max_stats; j++) {
                obj = Set_get(app->objects, j);
                assert(obj);
                for (i = 0; i < Set_length(obj->stats) && k < max_stats; i++) {
                    var = Set_get(obj->stats, i);
                    assert(var);
                    str_get(obj->objname, stats[k].objname, PNAME_LEN);
                    str_get(var->name, stats[k].statname, PNAME_LEN);
                    stats[k].type = (int) var_type(var);
                    stats[k].size = var_length(var);
                    k++;
                }
            }
            app_delete(&app);
            return k;
        }
    }
    return -1;
}

/** Get an statistic value
 *
 * Obtain an stat value. Values are saved in the value pointer. Type of the statistic
 * will be saved into the type pointer so the user can process data which type is
 * not known a priori.
 *
 * @param app_name Name of the waveform
 * @param obj_name Name of the object who registered the stat
 * @param stat_name Name of the variable as the object registered it
 * @param value Pointer to a buffer where data will be saved
 * @param type Pointer to an integer where statistic type will be saved
 * @param tstamp Pointer to an integer where tstamp will be saved
 * @param max_size Size of the user buffer, in bytes
 * @param max_sec Timeout for receiving an answer
 */
int cmdman_statget(char *app_name, char *obj_name, char *stat_name, int window,
        void *value, int *type, int *tstamp, int max_size, int max_sec)
{
    int n;
    var_o var;

    assert(app_name && obj_name && stat_name);

    pkt_clear(pkt);
    str_o appname = str_snew(app_name);
    pkt_put(pkt, FIELD_APPNAME, appname, str_topkt);
    str_delete(&appname);
    str_o objname = str_snew(obj_name);
    pkt_put(pkt, FIELD_OBJNAME, objname, str_topkt);
    str_delete(&objname);
    str_o statname = str_snew(stat_name);
    pkt_put(pkt, FIELD_STNAME, statname, str_topkt);
    str_delete(&statname);
    pkt_putvalue(pkt, FIELD_STREPORTWINDOW, window);

    if (send_wait_answer_pkt(STATSMAN_STGET, DAEMON_STATSMAN, max_sec)) {
        if (pkt_getcmd(pkt) == CMDMAN_STVALOK) {
            var = pkt_get(pkt, FIELD_STAT, var_newfrompkt);
            if (value) {
                var_getvalue(var,value,max_size);
            } 
            if (tstamp) {
                *tstamp = var->tstamp;
            }
            if (type) {
                *type = var_type(var);
            }
            n = var_length(var);
            var_delete(&var);
            return n;
        } 
    }
    return -1;
}

/** Start/Stop statistics report
 *
 * Starts/stops reports of statistics value into a text file. Reports are by default
 * saved at %repository%/reports/waveformName/objName.statName.stat
 *
 * @param app_name Name of the waveform
 * @param obj_name Name of the object who registered the stat
 * @param stat_name Name of the variable
 * @param action 1 start 0 stop
 * @param window Number of samples to capture
 * @param period Period of trigger
 * @param max_sec Timeout for receiving an answer
 */
int cmdman_statreport(char *app_name, char *obj_name, char *stat_name,
        int action, int window, int period, int max_sec)
{
    assert(app_name && obj_name && stat_name);

    pkt_clear(pkt);
    str_o appname = str_snew(app_name);
    pkt_put(pkt, FIELD_APPNAME, appname, str_topkt);
    str_delete(&appname);
    str_o objname = str_snew(obj_name);
    pkt_put(pkt, FIELD_OBJNAME, objname, str_topkt);
    str_delete(&objname);
    str_o statname = str_snew(stat_name);
    pkt_put(pkt, FIELD_STNAME, statname, str_topkt);
    str_delete(&statname);

    pkt_putvalue(pkt, FIELD_STREPORTACT, action);
    pkt_putvalue(pkt, FIELD_STREPORTWINDOW, window);
    pkt_putvalue(pkt, FIELD_STREPORTPERIOD, period);

    if (send_wait_answer_pkt(STATSMAN_STREPORT, DAEMON_STATSMAN, max_sec)) {
        if (pkt_getcmd(pkt) == CMDMAN_STREPORTOK) {
            return 1;
        }
    }
    return -1;
}

/** Set statistics variable value
 *
 *
 * @param app_name Name of the waveform
 * @param obj_name Name of the object who registered the stat
 * @param stat_name Name of the variable
 * @param value Pointer to a buffer where data will be read
 * @param size Size (in elements) of the variable value
 * @param type Type of the statistic
 * @param max_sec Timeout for setting value
 */
int cmdman_statset(char *app_name, char *obj_name, char *stat_name,
        void *value, int size, int type, int max_sec)
{
    int bsize;

    assert(app_name && obj_name && stat_name && value);

    pkt_clear(pkt);

    switch(type) {
    case STAT_TYPE_INT:
    case STAT_TYPE_FLOAT:
        bsize=(size<<2);
        break;
    default:
        printf("CMDMAN_BACKEND: Error setting stat. Type %d not supported\n", type);
        return -1;
    }

    str_o appname = str_snew(app_name);
    pkt_put(pkt, FIELD_APPNAME, appname, str_topkt);
    str_delete(&appname);
    str_o objname = str_snew(obj_name);
    pkt_put(pkt, FIELD_OBJNAME, objname, str_topkt);
    str_delete(&objname);
    str_o statname = str_snew(stat_name);
    pkt_put(pkt, FIELD_STNAME, statname, str_topkt);
    str_delete(&statname);
    pkt_putvalue(pkt, FIELD_STSIZE, size);
    memcpy(pkt_putptr(pkt, FIELD_STVALUE, bsize), value, bsize);
    if (send_wait_answer_pkt(STATSMAN_STSET, DAEMON_STATSMAN, max_sec)) {
        if (pkt_getcmd(pkt) == CMDMAN_STSETOK) {
            return 1;
        }
    }
    return -1;
}








