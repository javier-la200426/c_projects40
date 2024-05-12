/**************************************************************
 *
 *                     a2plain.c
 *
 *     Assignment: Locality
 *     Authors:  Javier Laveaga(jlavea01), William Magann (wmagan01)
 *     Date:  2/22/2024
 *
 *     This file contains the functions pointers which called specific 
 *     functions of the UArray2 interface. Meaning that we are defining
 *     a private version of each function in A2Methods_T
 *
 *    
 *
 **************************************************************/

#include <string.h>

#include <a2plain.h>
#include "uarray2.h"

/************************************************/
/* Define a private version of each function in */
/* A2Methods_T that we implement.               */
/************************************************/


typedef A2Methods_UArray2 A2;

/********** new ********
 *
 * initalizes new A2Methods_UArray2 of a plain array
 *
 * Parameters:
 *      int width:      width of array to be created
 *      int height:     height of array to be created
 *      int size:       size of elements to be stores
 *      
 * Return: 
        static A2Methods_UArray2 of UArray2 2D array
 *
 * Expects:
 *      nothing
 * Notes:
 *      nothing
 *      
 ************************/
static A2Methods_UArray2 new(int width, int height, int size)
{
        return UArray2_new(width, height, size);
}

/********** new_with_blocksize ********
 *
 * initalizes new A2Methods_UArray2 of a plain array
 *
 * Parameters:
 *      int width:      width of array to be created
 *      int height:     height of array to be created
 *      int size:       size of elements to be stores
 *      int blocksize:  length of one side of 1 block
 *      
 * Return: 
        static A2Methods_UArray2 of UArray2 2D array
 *
 * Expects:
 *      nothing
 * Notes:
 *      nothing
 *      
 ************************/
static A2Methods_UArray2 new_with_blocksize(int width, int height, int size,
                                            int blocksize)
{
        (void) blocksize;
        return UArray2_new(width, height, size);
}

/********** a2free ********
 *
 * calls function to free UArray2
 *
 * Parameters:
 *      A2 * array2p:   UArray2 2D array
 *      
 * Return: 
        nothing
 *
 * Expects:
 *      nothing
 * Notes:
 *      nothing
 *      
 ************************/
static void a2free(A2 * array2p)
{
        UArray2_free((UArray2_T *) array2p);
}

/************************** width **************************
*
* This function returns the width (int) of A2Methods_UArray2 (A2)        
* 
* Parameters:
*      A2 array2:    2D UArray2
*
* Return: 
*       returns the static int of width of the array
* Expects
*       Nothing     
* 
* Notes:
*      Nothing
*                        
*****************************************************************/ 
static int width(A2 array2)
{
        return UArray2_width(array2);
}

/********** height ********
 *
 * calls function to retrieve height of UArray2 2D array
 *
 * Parameters:
 *      A2 array2p:   UArray2 2D array
 *      
 * Return: 
 *      static int of 2D array height
 *
 * Expects:
 *      nothing
 * Notes:
 *      nothing
 *      
 ************************/
static int height(A2 array2)
{
        return UArray2_height(array2);
}

/********** size ********
 *
 * calls function to retrieve size of UArray2 2D array
 *
 * Parameters:
 *      A2 array2p:   UArray2 2D array
 *      
 * Return: 
 *      static int of 2D array size
 *
 * Expects:
 *      nothing
 * Notes:
 *      nothing
 *      
 ************************/
static int size(A2 array2)
{
        return UArray2_size(array2);
}

/********** blocksize ********
 *
 * calls function to retrieve blocksize of UArray2 2D array
 *
 * Parameters:
 *      A2 array2p:   UArray2 2D array
 *      
 * Return: 
        static int bloccksize of UArray2 2D array
 *
 * Expects:
 *      nothing
 * Notes:
 *      nothing
 *      
 ************************/
static int blocksize(A2 array2)
{
        (void)array2;
        return 1;
}

/********** *at ********
 *
 * calls function to element of 2D array
 *
 * Parameters:
 *      A2 array2p:   UArray2 2D array
 *      int col:      column of given index
 *      int row:      row of given index
 *      
 * Return: 
 *      static A2Methods_Object void pointer of element at an index
 *
 * Expects:
 *      nothing
 * Notes:
 *      nothing
 *      
 ************************/
static A2Methods_Object *at(A2 array2, int col, int row)
{
        return UArray2_at(array2, col, row);
}

/********** map_row_major ********
 *
 * calls the map_row_major for a U2Array
 *
 * Parameters:
 *     A2Methods_UArray2 uarray2: a Uarray2 methods 
 *     A2Methods_applyfun apply:  an apply function for A2 methods
 *     void *cl:         a closure for the apply function
 *      
 * Return: 
        nothing
 *
 * Expects:
 *      nothing
 * Notes:
 *      nothing
 *      
 ************************/
static void map_row_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_row_major(uarray2, (UArray2_applyfun*)apply, cl);
}

/********** map_col_major ********
 *
 * calls the map_col_major for a U2Array
 *
 * Parameters:
 *     A2Methods_UArray2 uarray2: a Uarray2 methods 
 *     A2Methods_applyfun apply:  an apply function for A2 methods
 *     void *cl:         a closure for the apply function
 *      
 * Return: 
        nothing
 *
 * Expects:
 *      nothing
 * Notes:
 *      nothing
 *      
 ************************/
static void map_col_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_col_major(uarray2, (UArray2_applyfun*)apply, cl);
}

/*Functions below were given to us*/
struct small_closure {
        A2Methods_smallapplyfun *apply; 
        void                    *cl;
};

static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
        struct small_closure *cl = vcl;
        (void)i;
        (void)j;
        (void)uarray2;
        cl->apply(elem, cl->cl);
}

static void small_map_row_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_row_major(a2, apply_small, &mycl);
}

static void small_map_col_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_col_major(a2, apply_small, &mycl);
}

/************************** A2Methods_T uarray2_methods_plain_struct ********
*
* holds pointers pointers to the functions in a2plain.c
* 
* 
* each name corresponds to one of the functions defined above
*                         
*****************************************************************/
static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,
        at,
        map_row_major,           
        map_col_major,           
        NULL,                   // map_block_major
        map_row_major,          // map_default
        small_map_row_major,              
        small_map_col_major,           
        NULL,
        small_map_row_major,    // small_map_default
};

/*finally the payoff: here is the exported pointer to the struct*/ 

A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
