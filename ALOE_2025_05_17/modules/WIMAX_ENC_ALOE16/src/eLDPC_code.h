
#ifndef _eLDPC_H
#define	_eLDPC_H

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
#include "rcode.h"
#include "enc.h"


void usage(void);
int Init_eLDPC(void);
int Init_EncodedFile(void);
int Save_Encodeblock (void);
int Close_EncodeFile(void);
int Run_EncodeeLDPC (int* c, int* icw);

/*char *source_file, *encoded_file;
char *pchk_file, *gen_file;
mod2dense *u, *v;

FILE *srcf, *encf;
char *sblk, *cblk, *chks;
int i, n;
int *in_array;
int *out_array;
*/
#endif
