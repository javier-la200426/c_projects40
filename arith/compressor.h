/************************** compressor.h ***********************
 *                          
 *     Authors: Javier Laveaga {jlavea01} and Prince Millidah {pmilli03}, 
 *     Date: 2/27/2024
 *     Assignment: arith
 *
 *     Summary: This file contains the interface for implementation of 
 *              function to compress a ppm file.
 *
 ***************************************************************/
#ifndef COMPRESSOR
#define COMPRESSOR

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <pnm.h>
#include <a2plain.h>
#include <a2blocked.h>
#include "mem.h"
#include "arith40.h"
#include "math.h"
#include "uarray.h"
#include "bitpack.h"
#include "program_structs.h"

Pnm_ppm read_ppm(FILE *fp);
void populate(int col, int row, A2Methods_UArray2 array2,
                                A2Methods_Object *ptr, void *cl);
void int_to_float (Pnm_ppm reader);
void toFloat(int col, int row, A2Methods_UArray2 my_array,
                                A2Methods_Object *ptr, void *reader);
void toCompVideo(int col, int row, A2Methods_UArray2 my_array,
                                A2Methods_Object *ptr, void *reader);

Seq_T get_avg_Ps (Pnm_ppm reader);
void to_avg_and_abcd(int col, int row, A2Methods_UArray2 my_array,
                                A2Methods_Object *ptr, void *avgClosure);

float roundFloats(float num, float lower, float upper);
UArray_T seq_to_words(Seq_T blocks);
int quantizeFloats(float num);
void print(UArray_T codeWords, Pnm_ppm reader);
float compute_rgb_float(unsigned rgb, unsigned denominator);

void divide_four (struct P_avg_abcd *block_vals);
void get_Y1(struct P_avg_abcd *block_vals, struct P_and_Y_floats *currElem);
void get_Y2(struct P_avg_abcd *block_vals, struct P_and_Y_floats *currElem);
void get_Y3(struct P_avg_abcd *block_vals, struct P_and_Y_floats *currElem);
void get_Y4(struct P_avg_abcd *block_vals, struct P_and_Y_floats *currElem);

uint32_t packWord(struct P_avg_abcd *curr_struct);


#endif