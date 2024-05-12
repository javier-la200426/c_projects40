/************************** decompressor.h ***********************
 *                          
 *     Authors: Javier Laveaga {jlavea01} and Prince Millidah {pmilli03}, 
 *     Date: 2/27/2024
 *     Assignment: arith
 *
 *     Summary: This file contains the interface for implementation of 
 *              function to decompress an input file with coded words.
 *
 ***************************************************************/

#ifndef DECOMPRESSOR
#define DECOMPRESSOR

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <pnm.h>
#include <a2plain.h>
#include "mem.h"
#include "uarray.h"
#include "bitpack.h"
#include "arith40.h"
#include <a2blocked.h>
#include "program_structs.h"


void PstoInt(int col, int row, A2Methods_UArray2 my_array,
                                A2Methods_Object *ptr, void *reader);
void float_to_int(Pnm_ppm reader);
void getfloatValues(int col, int row, A2Methods_UArray2 my_array,
                                A2Methods_Object *ptr, void *decompClosure);
                                
A2Methods_UArray2 seq_to_bArray (Seq_T blocks, Pnm_ppm reader);
Seq_T words_to_array (UArray_T codewords);
UArray_T readFile (FILE *in, Pnm_ppm reader);
Pnm_ppm init_reader (int denominator);
void unpackWord(uint32_t word, struct P_avg_abcd *currFloats);


#endif