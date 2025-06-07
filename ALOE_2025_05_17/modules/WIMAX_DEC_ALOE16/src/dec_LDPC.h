/* DECODE_LDPC.h - Interface to decode blocks of received data. */

/* Copyright (c) 2000, 2001 by Radford M. Neal 
 *
 * Permission is granted for anyone to copy, use, modify, or distribute this
 * program and accompanying programs and documents for any purpose, provided 
 * this copyright notice is retained and prominently displayed, along with
 * a note saying that the original programs are available from Radford Neal's
 * web page, and note is made of any changes made to the programs.  The
 * programs and documents are distributed without any warranty, express or
 * implied.  As the programs were written for research purposes only, they have
 * not been tested to the degree that would be advisable in any important
 * application.  All use of these programs is entirely at the user's own risk.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "rand.h"
#include "alloc.h"
#include "blockio.h"
#include "open.h"
#include "mod2sparse.h"
#include "mod2dense.h"
#include "mod2convert.h"
#include "channel.h"
#include "rcode.h"
#include "check.h"
#include "dec.h"


/* DECODING METHOD, ITS PARAMETERS, AND OTHER VARIABLES.  The global variables 
   declared here are located in decode_LDPC.c. */


extern char *pchk_file, *rfile, *dfile, *pfile, *gen_file;
extern char **meth;
extern FILE *rf, *df, *pf;

extern  char *pchk;
extern double *lratio;
extern double *bitpr;

extern double *awn_data;		/* Places to store channel data */
extern int *bsc_data;

extern unsigned iters;		/* Unsigned because can be huge for enum */
extern double tot_iter;		/* Double because can be huge for enum */
extern double chngd, tot_changed;	/* Double because can be fraction if lratio==1*/

extern int tot_valid;
extern char junk;
extern int valid;

extern int i, j, k;

extern char **argv;

extern float _EbN0;

/* PROCEDURES RELATING TO DECODING METHODS. */

void usage(void);

void Init_DeLDPC();

int * adapt_floats_to_int(float* src_block);

double * adapt_floats_to_double(float* src_block);

void read_block_from_file(void);

void run_decode_LDPC(float* src_block, char* dblk);

void open_files(char* src_file, char* dst_file);

void write_block_to_file(void);

void finish (void);
