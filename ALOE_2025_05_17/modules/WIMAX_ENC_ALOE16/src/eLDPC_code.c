/* ENCODE.C - Encode message blocks. */

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

#include "eLDPC_code.h"

void usage(void);
int Init_eeLDPC(void);
int Init_EncodedFile(void);
int Save_Encodeblock (void);
int Close_EncodeFile(void);
int Run_EncodeeLDPC (int* c, int* icw);
void Create_Source_Block(int* sourceInt, char* sourceChar);
void Create_Encode_Block(int* eBlockInt, char* eBlockChar);

char *source_file, *encoded_file;
char *pchk_file, *gen_file;
mod2dense *u, *v;

FILE *srcf, *encf;
char *sblk, *cblk, *chks;
int i, n;



/* MAIN PROGRAM. */

int Init_eLDPC(void)
{
	/* Look at arguments. */

	if ((strcmp(pchk_file,"-")==0) + (strcmp(gen_file,"-")==0) > 1)
	{
		fprintf(stderr,"Can't read more than one stream from standard input\n");
		exit(1);
	}

	/* Read parity check file */

	read_pchk(pchk_file);

	if (N<=M)
	{ 
		fprintf(stderr,
		        "Can't encode if number of bits (%d) not greater than number of checks (%d)\n",
		        N,M);
		exit(1);
	}

	/* Read generator matrix file. */

	read_gen(gen_file,0,0);

	/* Allocate needed space. */

	if (type=='d')
	{ 
		u = mod2dense_allocate(N-M,1);
		v = mod2dense_allocate(M,1);
	}

	if (type=='m')
	{
		u = mod2dense_allocate(M,1);
		v = mod2dense_allocate(M,1);
	}

	sblk = chk_alloc (N-M, sizeof *sblk);
	cblk = chk_alloc (N, sizeof *cblk);
	chks = chk_alloc (M, sizeof *chks);

}
//Source file opened is 802_16e.src
void Open_SourceFile (void)
{
	if ( !(source_file = "data/802_16e.src") || (strcmp(source_file,"-")==0)) 
		usage();
	srcf = open_file_std(source_file,"r");
	if (srcf==NULL)
	{ 
		fprintf(stderr,"Can't open source file: %s\n",source_file);
		exit(1);
	}
}
/* Create encoded output file. */
int Init_EncodedFile(void)
{
	if ( !(encoded_file = "data/802_16e.enc")) 
		usage();
	encf = open_file_std(encoded_file,"w");
	if (encf==NULL)
	{
		fprintf(stderr,"Can't create file for encoded data: %s\n",encoded_file);
		exit(1);
	}
	return 0;
}

int Run_EncodeeLDPC (int* c, int* icw)
{

/*    printf("Run_EncodeeLDPC(): INPUT\n");
    for(i=1; i<=1152; i++){
        printf("%d ", c[i-1]);
        if(i%64==0)printf("\n");
    }
    printf("\n");
 *
 *
*/


	Create_Source_Block(c, sblk);
	/* Compute encoded block. */
	switch (type)
	{ 
		case 's':
		{
                        //printf("sparse_encode\n");
			sparse_encode (sblk, cblk);
			break;
		}
		case 'd':
		{
                        //printf("dense_encode\n");
			dense_encode (sblk, cblk, u, v);
			break;
		}
		case 'm':
		{
                        //printf("mixed_encode\n");
			mixed_encode (sblk, cblk, u, v);
			break;
		}
			/* Check that encoded block is a code word. */
			mod2sparse_mulvec (H, cblk, chks);

		for (i = 0; i<M; i++) 
		{ 
		  if (chks[i]==1)
		  { 
			fprintf(stderr,"Output block %d is not a code word!  (Fails check %d)\n",n,i);
			abort(); 
		  }
		}


			
	}

	Create_Encode_Block(icw, cblk);

/*    printf("IN Run_EncodeeLDPC(): OUPUT\n");
    for(i=1; i<=2304; i++){
        printf("%d ", icw[i-1]);
        if(i%64==0)printf("\n");
    }
    printf("\n");
*/
	
	return 1;
}

void Create_Source_Block(int* sourceInt, char* sourceChar)
{
  	for(i= 0; i<N-M; i++)
	{
	    if( sourceInt[i] == 0) sourceChar[i] = 0;
		else sourceChar[i] = 1;
	}
}

void Create_Encode_Block(int* eBlockInt, char* eBlockChar)
{
  	for(i= 0; i<N; i++)
	{
	    if( eBlockChar[i] == 0) eBlockInt[i] = 0;
		else eBlockInt[i] = 1;
	}
}


/* Write encoded block to encoded output file. */
int Save_Encodeblock (void)
{
	blockio_write(encf,cblk,N);
	return 0;
} 

int Close_EncodeFile(void)
{
	if (ferror(encf) || fclose(encf)!=0)
	{ 
		fprintf(stderr,"Error writing encoded blocks to %s\n",encoded_file);
		exit(1);
	}
	return 0;
}
/* PRINT USAGE MESSAGE AND EXIT. */

void usage(void)
{ 
	fprintf(stderr,
	        "Usage:  encode pchk-file gen-file source-file encoded-file\n");
	exit(1);
}

