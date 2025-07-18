/*
 * swload_cmds.c
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
/* standard libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* phal hwlibrary constants */
#include "phal_hw_api.h"

#include "phid.h"
#include "phal_daemons.h"

#include "swman.h"

/* common objects */
#include "set.h"
#include "str.h"
#include "rtcobj.h"
#include "phitf.h"
#include "phobj.h"
#include "app.h"
#include "pkt.h"
#include "downexe.h"
#include "daemon.h"

/* commands */
#include "swload_cmds.h"
#include "swman_cmds.h"

#include "bridge.h"
#include "bridge_cmds.h"

/* downloading queue */
Set_o down_q = NULL;

#define DEFAULT_ITF_SZ	0x1000

void load_error(downexe_o exe);
int configure_itfs(phobj_o obj);


#define NOF_OBJ	200
#define NOF_ITF	1000
#define NOF_STR	5000
#define NOF_APP 20			/*200 AGBABRIL20*/
#define NOF_SET	1000
#define NOF_SETMEM 2000

void swload_alloc() {
#ifdef MTRACE
    mtrace();
#endif
    hwapi_mem_prealloc(NOF_OBJ, phobj_sizeof);
    hwapi_mem_prealloc(NOF_OBJ, rtcobj_sizeof);
    hwapi_mem_prealloc(NOF_OBJ, downexe_sizeof);
    hwapi_mem_prealloc(NOF_ITF, phitf_sizeof);
    hwapi_mem_prealloc(NOF_STR, str_sizeof);
    hwapi_mem_prealloc(NOF_APP, app_sizeof);
    hwapi_mem_prealloc(NOF_SET, set_sizeof);
    hwapi_mem_prealloc(NOF_SETMEM, setmember_sizeof);
    hwapi_mem_prealloc(1, daemon_sizeof);
    hwapi_mem_prealloc(1, pkt_sizeof);
}


static daemon_o daemon;
static pkt_o pkt;

#define PROC_MAX_LEN	(8000*1024)	/*1100*1024 AGB CANVIAR*/
char *procBuff;

int swload_init(daemon_o d) {
    procBuff = malloc(PROC_MAX_LEN);
    if (!procBuff) {
        daemon_error(daemon, "Error allocating memory for program reception buffer\n");
        return -1;
    }
    down_q = Set_new(0, NULL);
    if (!down_q) {
        daemon_error(daemon, "Fatal error while initiating");
        return -1;
    }
    daemon = d;
    pkt = daemon_pkt(daemon);
    return 1;
}

void request_exec(int plt_family, str_o exe_name, int exeid) {

    assert(exe_name);

    if (hwapi_proc_needsrelocate()) {
        pkt_clear(pkt);
        pkt_putvalue(pkt, FIELD_PLATID, (uint) plt_family);
        pkt_putvalue(pkt, FIELD_OBJID, (uint) exeid);
        pkt_put(pkt, FIELD_EXENAME, exe_name, str_topkt);
        daemon_sendto(daemon, SWMAN_EXECINFO, 0, DAEMON_SWMAN);
    } else {
        pkt_clear(pkt);
        pkt_putvalue(pkt, FIELD_PLATID, (uint) plt_family);
        pkt_putvalue(pkt, FIELD_OBJID, (uint) exeid);
        pkt_put(pkt, FIELD_EXENAME, exe_name, str_topkt);
        daemon_sendto(daemon, SWMAN_EXECREQ, 0, DAEMON_SWMAN);
    }

}

/** ANSWER EXEC INFO
 *
 * After exec information have been requested, this command is received.
 *
 */
int swload_incmd_execinfo(cmd_t *cmd) {
    phobj_o obj;
    downexe_o exe;
    struct pinfo pinfo;
    struct hwapi_proc_launch plaunch;

    exe = Set_find(down_q, pkt_getptr(pkt, FIELD_OBJID), downexe_findid);
    if (!exe) {
        daemon_error(daemon, "Unknown executable id 0x%x.", pkt_getvalue(pkt, FIELD_OBJID));
        load_error(NULL);
        return 0;
    }

    if (pkt_getptr(pkt, FIELD_PINFO)) {
        memcpy(&pinfo, pkt_getptr(pkt, FIELD_PINFO),sizeof(struct pinfo));
    }
    /* request space */
    plaunch.mem_st = pinfo.mem_start;
    plaunch.mem_sz = pinfo.mem_sz;
    if (!hwapi_proc_request(&plaunch)) {
        daemon_error(daemon, "Error requesting space for object 0x%x.", pkt_getvalue(pkt, FIELD_OBJID));
        load_error(NULL);
        return 0;
    }
    pinfo.mem_start = plaunch.mem_st;
    pinfo.mem_sz = plaunch.mem_sz;

    daemon_sendto(daemon, SWMAN_EXECREQ, 0, DAEMON_SWMAN);

    return 1;
}

/** LOAD OBJECT COMMAND
 *
 * This function is called when a command to load a list of objects
 * with a list of interfaces connecting them is received. The procedure
 * followed to provide such connectivity is the following:
 *
 * - Internal interfaces (the remote object is in the same processor): 
 *		A name like "w_obj_name:w_itf_name" and "r_obj_name:r_itf_name"
 * 		is associated using the HWAPI Interface Names Service with the 
 *		write and read side of the interface, created by this function.
 * - External Interfaces (the local interface is associated to a physical 
 *	interface id and a remote interface id decided by HWMAP.
 *		This function will create the object side of the interface and assign
 *		the same name for both sides. The bridge, will attach
 *		its interface when receives the order from HWMAP. 
 * 
 * @todo Configure Bridge Interfaces
 * @todo Setup interface options (delay)
 * @todo Executable Request error management
 * @todo Absolute Addressing Computers (Relocation)
 */
int swload_incmd_loadobj(cmd_t *cmd) {
    phobj_o obj;
    downexe_o exe;
    Set_o objset;
    app_o app;
    int i;
    int plt_family;
    struct hwapi_cpu_i cpu;

    hwapi_hwinfo_cpu(&cpu);
    plt_family = cpu.plat_family;

    app = (app_o) pkt_get(pkt, FIELD_APP, app_newfrompkt);
    if (!app) {
        daemon_error(daemon, "Invalid packet.");
        return 0;
    }
    objset = app->objects;
    if (!objset || !Set_length(objset)) {
        daemon_error(daemon, "Void object list.");
        app_delete(&app);
        return 0;
    }

    for (i = 0; i < Set_length(objset); i++) {
        obj = Set_get(objset, i);
        if (!obj) {
            return 0;
        }

        /** Now check if we need to request an executable.
         * If the platform needs to manually allocate programs to the memory
         * (needs to perform relocation) we will request process
         * information for every instance of the object.
         * Conversely, if the platform automatically performs relative addressing,
         * we will request one executable for all instances.
         */
        if (hwapi_proc_needsrelocate()) {
            exe = downexe_new();
            if (!exe) {
                return 0;
            }
            str_cpy(exe->name, obj->exename);
            Set_put(down_q, exe);
        } else {
            exe = Set_find(down_q, obj->exename, downexe_findname);
            if (!exe) {
                exe = downexe_new();
                if (!exe) {
                    return 0;
                }
                str_cpy(exe->name, obj->exename);
                Set_put(down_q, exe);
            }
        }
        Set_put(exe->objects, phobj_dup(obj));
    }

    exe = Set_get(down_q, 0);
    if (exe) {
        request_exec(plt_family, exe->name, exe->id);
    }
    app_delete(&app);
    return 1;
}

/** SWLOAD: Executable request error 
 * @todo what to do with errors
 */
int swload_incmd_execerr(cmd_t *cmd) {
    downexe_o exe;
    str_o exe_name;

    exe_name = pkt_get(pkt, FIELD_EXENAME, str_newfrompkt);
    if (!exe_name) {
        daemon_error(daemon, "Invalid packet");
        return 0;
    }

    exe = Set_find(down_q, exe_name, downexe_findname);
    if (!exe) {
        daemon_error(daemon, "Can't find executable at downloading queue\n\tCaution! Some executables may be still in queue.");
        str_delete(&exe_name);
        return 0;
    }

    daemon_error(daemon, "Error when requesting executable %s", str_str(exe->name));
    str_delete(&exe_name);

    load_error(exe);

    return 1;
}

/** SWLOAD: Load start command
 *
 * Now the final executable is generated and ready to be downloaded. This command
 * indicates the length (byte and word aligned) and the size of the packet that will
 * be used to slice the executable when downloading.
 * 
 * Here, as a lot of memory needs to be allocated when receiveing files, dynamic
 * memory is used.
 *
 * @todo Variable packet size
 *
 * @todo ACK del LOADSTART (per si no pot allocar)
 */
int swload_incmd_loadstart(cmd_t *cmd) {
    phobj_o obj;
    downexe_o exe;

    exe = Set_find(down_q, pkt_getptr(pkt, FIELD_OBJID), downexe_findid);
    if (!exe) {
        daemon_error(daemon, "Unknown executable id 0x%x.", pkt_getvalue(pkt, FIELD_OBJID));
        load_error(NULL);
        return 0;
    }

    memcpy(&exe->pinfo, pkt_getptr(pkt, FIELD_PINFO), sizeof (struct pinfo));
    exe->data = procBuff;
    exe->checksum = 0;
    if (exe->pinfo.blen > PROC_MAX_LEN) {
        daemon_error(daemon, "Error program too big: %d>%d", exe->pinfo.blen, PROC_MAX_LEN);
        obj = Set_get(exe->objects, 0);
        pkt_put(pkt, FIELD_APPNAME, obj->appname, str_topkt);
    }

    return 1;
}

int counter = 0;
int prog_cnt = 0;

int save_packet(void *x, void *data, fieldsz_t size) {
    downexe_o downexe = (downexe_o) x;
    int i;
    unsigned char *p;

    assert(x && data && size);

    if (downexe->rpm + size > downexe->pinfo.blen) {
        daemon_error(daemon, "Error with size at pkt %d", counter);
        return -1;
    }

    memcpy(&downexe->data[downexe->rpm], data, size);

    p = (unsigned char *) &downexe->data[downexe->rpm];

    /* compute checksum */
    for (i = 0; i < size; i++) {
        downexe->checksum += p[i];
    }

    downexe->rpm += size;

    return 1;
}

void load_error(downexe_o exe) {
    phobj_o obj;
    pkt_clear(pkt);
    if (exe) {
        obj = Set_get(exe->objects, 0);
        pkt_put(pkt, FIELD_APPNAME, obj->appname, str_topkt);
        Set_remove(down_q, exe);
        downexe_delete(&exe);
    }
    daemon_sendto(daemon, SWMAN_EXECERR, 0, DAEMON_SWMAN);
}

/** SWLOAD: Loading progress command
 *
 * With this function the actual body of the executable is 
 * saved into the buffer allocated during the loadstart function.
 *
 * @todo Errors when downloading
 */
int swload_incmd_loading(cmd_t *cmd) {
    downexe_o exe;

    exe = Set_find(down_q, pkt_getptr(pkt, FIELD_OBJID), downexe_findid);
    if (!exe) {
        daemon_error(daemon, "Unknown executable id 0x%x.", pkt_getvalue(pkt, FIELD_OBJID));
        load_error(NULL);
        return 0;
    }
    if (!exe->data) {
        daemon_error(daemon, "Start packet not received");
        load_error(exe);
        return 0;
    }
    if (!save_packet(exe, pkt_getptr(pkt, FIELD_BINDATA),
            pkt_getvaluesz(pkt, FIELD_BINDATA))) {
        daemon_error(daemon, "Error saving data");
        load_error(exe);
        return 0;
    }

    /*    if (!test_save(exe, pkt_getptr(pkt, FIELD_BINDATA),
                pkt_getvaluesz(pkt, FIELD_BINDATA))) {
            daemon_error(daemon, "Error saving data");
            load_error(exe);
            return 0;
        }
     */ return 1;
}

/** Launch an instance (object) for every process 
 */
int swload_launchall(downexe_o downexe) {
    Set_o objset;
    phobj_o obj;

    int i;
    struct hwapi_proc_launch plaunch;
    struct hwapi_cpu_i cpu;

    hwapi_hwinfo_cpu(&cpu);

    assert(downexe);

    objset = downexe->objects;

    for (i = 0; i < Set_length(objset); i++) {
        obj = Set_get(objset, i);

        plaunch.app_id = obj->app_id;
        plaunch.obj_id = obj->obj_id;
        plaunch.obj_name = str_str(obj->objname);
        plaunch.app_name = str_str(obj->appname);
        plaunch.exe_name = str_str(downexe->name);
        plaunch.blen = downexe->pinfo.blen;
        plaunch.mem_st = downexe->pinfo.mem_start;
        plaunch.mem_sz = downexe->pinfo.mem_sz;

        if (hwapi_proc_create((char *) downexe->data, &plaunch, obj->exec_position, obj->core_idx) < 0) {
            daemon_error(daemon, "Error creating process.");
            return 0;
        }
    }
    return 1;
}

/** SWLOAD: Loading finish command
 *
 * When the transmission ends, this packet is received with the 
 * checksum to check errors. 
 *
 * If all is ok, we continue launching the process.
 */
int swload_incmd_loadend(cmd_t *cmd) {
    downexe_o exe;
    int i;
    int plt_family;
    struct hwapi_cpu_i cpu;
    unsigned char *p;
    unsigned int c=0;
    int rcvTstamp;

    hwapi_hwinfo_cpu(&cpu);
    plt_family = cpu.plat_family;

    exe = Set_find(down_q, pkt_getptr(pkt, FIELD_OBJID), downexe_findid);
    if (!exe) {
        daemon_error(daemon, "Unknown executable id 0x%x.", pkt_getvalue(pkt, FIELD_OBJID));
        load_error(NULL);
        return 0;
    }
    
    if (exe->checksum ^ ((uint) pkt_getvalue(pkt, FIELD_CHKSUM))) {
        daemon_error(daemon, "Error with checksum (0x%x!=0x%x)",
                exe->checksum,
                pkt_getvalue(pkt, FIELD_CHKSUM));
        load_error(exe);
        return 0;
    }

    rcvTstamp = get_tstamp();

    if (!swload_launchall(exe)) {
        daemon_error(daemon, "Internal error");
        load_error(exe);
        return 0;
    }
    
/*    printf("Take %d tslots to load\n",get_tstamp()-rcvTstamp);
 */

    for (i = 0; i < Set_length(exe->objects); i++) {
        if (!configure_itfs(Set_get(exe->objects, i))) {
            daemon_error(daemon, "Error configuring itfs.");
            load_error(exe);
            return 0;
        }
    }

    Set_remove(down_q, exe);
    downexe_delete(&exe);

    /* request next one */
    exe = Set_get(down_q, 0);
    if (exe) {
        request_exec(plt_family, exe->name, exe->id);
    }

    return 1;
}

int add_bridge_itf(hwitf_t itf_id, objid_t obj_id, phitf_o itf) {
    data_header_t header;
    struct hwapi_cpu_i cpu;
    void *x;

    if (phitf_iswriteonly(itf)) {
        header.ItfId = itf->remote_id;
        header.ObjId = itf->remote_obj_id;
        header.obj_tstamp = 0;
    } else {
        header.ItfId = itf->id;
        header.ObjId = obj_id;
        header.obj_tstamp = 0;
    }
    
    pkt_clear(pkt);

    x=pkt_putptr(pkt, FIELD_DATAHEADER, sizeof (data_header_t));

    if (!x) {
        printf("SWLOAD: Error could not add header to packet\n");
        return 0;
    }

    memcpy(x, &header, sizeof (data_header_t));
    pkt_putvalue(pkt, FIELD_OBJITFID, (int) itf_id);
    pkt_putvalue(pkt, FIELD_OBJITFMODE, (int) itf->mode);
    pkt_putvalue(pkt, FIELD_OBJXITFID, (int) itf->xitf_id);

    hwapi_hwinfo_cpu(&cpu);

    return daemon_sendto(daemon, BRIDGE_ADDITF, cpu.pe_id, DAEMON_BRIDGE);
}

int configure_itfs(phobj_o obj) {
    int j;
    phitf_o itf;
    int n;
    hwitf_t itf_id;
    char *w_obj, *r_obj, *w_itf, *r_itf;

    /* for every interface in the object */
    for (j = 0; j < Set_length(obj->itfs); j++) {
        itf = Set_get(obj->itfs, j);

        /* create the interface just for one side */
        if (phitf_iswriteonly(itf) || !phitf_isinternal(itf)) {
            itf_id = hwapi_itf_create(0, DEFAULT_ITF_SZ);
            if (!itf_id) {
                daemon_error(daemon, "Error creating interface");
                return 0;
            }

            if (phitf_iswriteonly(itf)) {
                w_obj = str_str(obj->objname);
                w_itf = str_str(itf->name);
                r_obj = str_str(itf->remoteobjname);
                r_itf = str_str(itf->remotename);
            } else {
                r_obj = str_str(obj->objname);
                r_itf = str_str(itf->name);
                w_obj = str_str(itf->remoteobjname);
                w_itf = str_str(itf->remotename);
            }

            /* Set system-default delay */
            hwapi_itf_setdelay(itf_id, itf->delay);

            /* assign same interface id for read/write side */
            if (hwapi_itf_match(itf_id, w_obj, w_itf, /* write itf name */
                    r_obj, r_itf) /* reader itf name */
                    < 0) {
                daemon_error(daemon, "Error assigning interface");
                return 0;
            }

            if (!phitf_isinternal(itf)) {
                if (!add_bridge_itf(itf_id, obj->obj_id, itf)) {
                    daemon_error(daemon, "Error sending itf to bridge");
                    return 0;
                }
                hwapi_itf_setexternal(itf_id);
            }
        }
    }

    return 1;
}

/** SWLOAD: Executable Information Answer
 *
 * This command is received after a request for memory information, 
 * here we need to obtain processor memory resources and request
 * the executable addressed to such memory.
 *
 * As memory has to be allocated, it may happen that not enough is 
 * available. In such case, an error should be reported indicating 
 * the impossibiliness of loading the executable.
 */

