/*
 *     uarray2.h
 *     by Javier Laveaga {jlavea01} and Mallika Rangan {mranga02}, 
 *     2/12/2024
 *     Assignment: iii
 *
 *     Summary: This file contains the interface for the uarray2. Has methods
 *     where you can do a row and column major as well as accessing elements.
 *     Includes function for freeing one of the elements.
 */

#ifndef UARRAY2
#define UARRAY2
#include "uarray.h"
#define T UArray2_T
typedef struct T *T;


T UArray2_new(int col, int row, int size);
int UArray2_width (T uarray2);
int UArray2_height (T uarray2);
int UArray2_size (T uarray2);
void *UArray2_at(T uarray2, int col, int row);
void UArray2_map_col_major(T uarray2, 
                void (*function_name)(int i, int j, T a, void *p1, void *cl), 
                void *cl);
void UArray2_map_row_major(T uarray2, 
                void (*function_name)(int col, int row, T a, void *p1, 
                void *cl), void *cl);
void UArray2_free(T *test_array);


#undef T
#endif