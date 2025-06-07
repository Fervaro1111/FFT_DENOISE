/* DECODE_LDPC.C - Decode blocks of received data. */

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

#include "dec_LDPC.h"
//Queda fijado el tipo de canal y el metodo de decodificación
//El resto de modos soportados no se han testeado, las funciones de los diferentes modos de funcionamiento
//permanecen en el codigo por si es necesario usarlas.
#define CHANNEL_TYPE AWGN
#define METHOD Prprp

/* GLOBAL VARIABLES.  Declared in decode_LDPC.h. */

char *pchk_file, *rfile, *dfile, *pfile, *gen_file;
FILE *rf, *df, *pf;

//char *dblk, *pchk;
char *pchk, *gen_file;
double *lratio;
double *bitpr;

double *awn_data;		/* Places to store channel data */
int *bsc_data;

unsigned iters;		/* Unsigned because can be huge for enum */
double tot_iter;		/* Double because can be huge for enum */
double chngd, tot_changed;	/* Double because can be fraction if lratio==1*/

int tot_valid;
char junk;
int valid;

float mean, es, sn_ratio;

float _EbN0;
int i, j, k;

void Init_DeLDPC ()
{	
      
	mean = 0;
	es = 1;	
	dec_method = METHOD;
	channel = CHANNEL_TYPE;	
	
	//Se añade la función de lectura de la matriz generadora. El codigo original corrige los errores
	//pero no nos devuelve la palabra original eliminando la redundancia. Con la lectura de la matriz
	//generadora, se inicializa el puntero que almacena las posiciones de los bit que pertenecen a la
	//palabra original ( cols).
	
	read_pchk(pchk_file);
	read_gen(gen_file,1,0);

	if (N<=M)
	{ fprintf(stderr,
	          "Number of bits (%d) should be greater than number of checks (%d)\n",N,M);
		exit(1);
	}

	/* Create file for bit probabilities, if specified. */

	
	/* Allocate space for data from channel. */
	switch (channel)
	{ case BSC:
		{ bsc_data = chk_alloc (N, sizeof *bsc_data);
			break;
		}
		case AWGN: case AWLN:
		{ awn_data = chk_alloc (N, sizeof *awn_data);
			break;
		}
		default:
		{ abort();
		}
	}

	/* Allocate other space. */

	//dblk   = chk_alloc (N, sizeof *dblk);
	lratio = chk_alloc (N, sizeof *lratio);
	pchk   = chk_alloc (M, sizeof *pchk);
	bitpr  = chk_alloc (N, sizeof *bitpr);
}


void open_files(char* src_file, char* dst_file){

	rfile=src_file;

	dfile=dst_file;
	
	/* Open file of received data. */

	rf = open_file_std(rfile,"r");
	if (rf==NULL)
	{ fprintf(stderr,"Can't open file of received data: %s\n",rfile);
		exit(1);
	}

	/* Create file for decoded data. */

	df = open_file_std(dfile,"w");
	if (df==NULL)
	{ fprintf(stderr,"Can't create file for decoded data: %s\n",dfile);
		exit(1);
	}

}


int * adapt_floats_to_int(float* src_block){

	int* adapted_block = chk_alloc (N, sizeof *adapted_block); 

	for (i = 0; i<N; i++){
		if(src_block[i]>=0.0)
			adapted_block[i]=0;
		else
			adapted_block[i]=1;
	}

	return  adapted_block;
}


double * adapt_floats_to_double(float* src_block){

	double* adapted_block = chk_alloc (N, sizeof *adapted_block); 

	for (i = 0; i<N; i++){
		adapted_block[i]=-src_block[i]/1.0;
		
	}

	return  adapted_block;
}


void read_blocks_from_file(void){
	/* Read block from received file, exit if end-of-file encountered. */

	for (i = 0; i<N; i++)
	{ int c;
		switch (channel)
		{ case BSC:  
			{ c = fscanf(rf,"%1d",&bsc_data[i]); 
				break;
			}
			case AWGN: case AWLN:
			{ c = fscanf(rf,"%lf",&awn_data[i]); 
				break;
			}
				default: abort();
		}
		if (c==EOF) 
		{ if (i>0)
			{ fprintf(stderr,
			          "Warning: Short block (%d long) at end of received file ignored\n",i);
			}
			finish();
		}
		if (c<1 || channel==BSC && bsc_data[i]!=0 && bsc_data[i]!=1)
		{ fprintf(stderr,"File of received data is garbled\n");
			exit(1);
		}
	}
}


void run_decode_LDPC(float* src_block, char* dblk){
  
	//Se calcula la sigma a partir de la EbNo introducida por el canal. Se inicializa
	//cada vez que se va a decodificar una palabra
	
  	sn_ratio = (float) pow(10, ( _EbN0 / 10) );
	std_dev =  (float) sqrt (es / ( 2 * sn_ratio ) );

	switch (channel)
	{ 	case BSC:
		{ 
			bsc_data=adapt_floats_to_int(src_block);
			
			break;
		}
		case AWGN:
		{ 
			awn_data=adapt_floats_to_double(src_block);
			
			break;
		}
		case AWLN:
		{ 
			awn_data=adapt_floats_to_double(src_block);
			
			break;
		}
			default: abort();
	}

	/* Read received blocks, decode, and write decoded blocks. */

	/* Find likelihood ratio for each bit. */

	switch (channel)
	{ case BSC:
		{ for (i = 0; i<N; i++)
			{ lratio[i] = bsc_data[i]==1 ? (1-error_prob) / error_prob
					: error_prob / (1-error_prob);
			}
			break;
		}
		case AWGN:
		{ 
		  
		  for (i = 0; i<N; i++)
			{ lratio[i] = exp(2*awn_data[i]/(std_dev*std_dev));
			  
			}
			break;
		}
		case AWLN:
		{ for (i = 0; i<N; i++)
			{ double e, d1, d0;
				e = exp(-(awn_data[i]-1)/lwidth);
				d1 = 1 / ((1+e)*(1+1/e));
				e = exp(-(awn_data[i]+1)/lwidth);
				d0 = 1 / ((1+e)*(1+1/e));
				lratio[i] = d1/d0;
			}
			break;
		}
			default: abort();
	}

	/* Try to decode using the specified method. */


	switch (dec_method)
	{ 	
	    case Prprp:
		{ iters = prprp_decode (H, lratio, dblk, pchk, bitpr);
		
			break;
		}
	    case Enum_block: case Enum_bit:
		{ iters = enum_decode (lratio, dblk, bitpr, dec_method==Enum_block);
			break;
		}
			default: abort();
	}
      
}

//void write_block_to_file(void){
	/* Write decoded block. */

	//blockio_write(df,dblk,N);

//}

void finish (void){

	/* Finish up. */

	
		fprintf(stderr,
		        "Decoded %d blocks, %d valid.  Average %.1f iterations, %.0f%% bit changes\n",
		        block_no, tot_valid, (double)tot_iter/block_no, 
		        100.0*(double)tot_changed/(N*block_no));

		if (ferror(df) || fclose(df)!=0)
	{ fprintf(stderr,"Error writing decoded blocks to %s\n",dfile);
		exit(1);
	}

		if (pfile)
	{ if (ferror(pf) || fclose(pf)!=0)
		{ fprintf(stderr,"Error writing bit probabilities to %s\n",dfile);
			exit(1);
		}
	}

		exit(0);
}


/* PRINT USAGE MESSAGE AND EXIT. */


void usage(void)
{ fprintf(stderr,"Usage:\n");
	fprintf(stderr,
	        "  decode [ -t ] pchk-file received-file decoded-file [ bp-file ] channel method\n");
	channel_usage();
	fprintf(stderr,
	        "Method:  enum-block gen-file | enum-bit gen-file | prprp [-]max-iterations\n");
	exit(1);
}
