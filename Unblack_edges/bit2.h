/*
 *     bit2.h
 *     by Javier Laveaga {jlavea01} and Mallika Rangan {mranga02}, 
 *     2/12/2024
 *     Assignment: iii
 *
 *     Summary: This file contains the interface for bit2.h, which is to be
 *     used with the bit2.c implementation. Bit2.h allows for the creation of
 *     a 2D bit array.
 */

#ifndef BIT2
#define BIT2
#include "bit.h"
#include "uarray.h"
#define T Bit2_T
typedef struct T *T;


T Bit2_new (int col, int row);
int Bit2_width(T bit2);
int Bit2_height(T bit2);


/* puts element into the specified col, row. Returns previous value */
int Bit2_put(T bit2, int col, int row, int value);

/* Get gets the value at the specified col, row. Returns bit you get */
int Bit2_get(T bit2, int col, int row);

void Bit2_map_col_major(T bit2, 
        void (*function_name)(int col, int row, 
        T bitarray, int correct_val, void *cl), void *cl);

void Bit2_map_row_major(T bit2, 
        void (*function_name)(int col, int row, 
        T bitarray, int correct_val, void *cl), void *cl);

void Bit2_free(T *bit2);

#undef T
#endif

