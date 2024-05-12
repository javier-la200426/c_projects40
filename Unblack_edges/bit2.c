
/*
 *     bit2.c
 *     by Javier Laveaga {jlavea01} and Mallika Rangan {mranga02}, 
 *     2/12/2024
 *     Assignment: iii
 *
 *     Summary: This file contains the implementation for bit2.c bit2.c is to
 *     be used with the bit2.h interface. The bit2.c program allows for the
 *     creation of a 2D bit array. The user of the bit2 interface can create
 *     a 2D array, get the width and height, put elements into the array,
 *     get elements from the array, use row and column mapping capabilities,
 *     and free memory for the 2D array.
 */

#include "bit2.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define T Bit2_T

/************************** T Bit2_T **************************
*
* holds info about our 2D Array
* 
* Parameters:
*    
*        UArray_T cols_array: pointer to array of columns in 2D array
*        int num_col: number of columns
*        int num_row: number of rows
*        int num_elements: number of elements
*        int size_elem: size of each element
                    
*****************************************************************/
struct T {
        UArray_T cols_array;
        int num_col;
        int num_row;
        int num_elements;
        int size_elem;
};


/************************** Bit2_new **************************
*
* This function will initialize a 2D array with the specified width, height.
*  Allocates memory for array's elements, returns a new
* Bit2_T.       
* 
* Parameters:
*       int width:   the width of the 2D array (# of columns)
*       int height   the height of the 2D array (# of rows)
*     
*
* Return: 
        A new Bit2_T representing the 2D array
*
* Expects:
*       width >= 0
*       height >= 0
*      
*
* Notes:
*       Failure to successfully allocate needed memory will result in a 
*       checked run-time error
*                        
*****************************************************************/
T Bit2_new (int col, int row) 
{
        assert (col >= 0 && row >= 0);
        /* Allocate memory for the Bit2_T struct, which reps our 2D array */
        T new_array = (T)malloc(sizeof(struct T));
        assert(new_array != NULL);

        /*  Allocate memory for the outer array. Each element is a bit vector*/
        new_array->cols_array = UArray_new(col, sizeof(Bit_T));
        assert(new_array->cols_array != NULL);
    
        
        for (int i = 0; i < col; i++) {
                Bit_T Bit_array = Bit_new(row); /*creating new bit vector */
                assert (Bit_array != NULL);
                /* casting pointer that UArray returns  */
                Bit_T *element = (Bit_T*)UArray_at(new_array->cols_array, i);
                assert(element != NULL);
                /* dereference pointer to access element */
                *element = Bit_array; 
                (void) element;
        }

        /* setting number of columns to the length of outer UArray */
        new_array->num_col = UArray_length(new_array->cols_array);

        /* accesing first element of outer Uarray */
        Bit_T *first_element = (Bit_T *)UArray_at(new_array->cols_array, 0);
        assert(first_element != NULL);
        /* getting length of first bit vector in the UArray */
        new_array->num_row = Bit_length(*first_element); 

        return new_array;
}
    

/************************** Bit2_width **************************
*
* This function returns the width (int) of the Bit2_T
* 
* Parameters:
*      T bit2:    the specified array
*
* Return: 
*       returns an int to represent the width of the array
* Expects
*       Bit2_T is a valid pointer (not NULL) and is correctly initialized
*      
* 
* Notes:
*       Will checked runtime error (CRE) if Bit2_T is NULL
*                        
*****************************************************************/      
int Bit2_width (T bit2) 
{
        assert(bit2 != NULL);
        return bit2->num_col;
}

/************************** Bit2_height **************************
*
* This function returns the height (int) of the Bit2_T
* 
* Parameters:
*      T bit2:    the specified array
*
* Return: 
*       returns an int to represent the height of the array
* Expects
*       Bit2_T is a valid pointer (not NULL) and is correctly initialized
*      
* 
* Notes:
*       Will checked runtime error (CRE) if Bit2_T is NULL
*                        
*****************************************************************/   
int Bit2_height (T bit2) 
{
        assert(bit2 != NULL);
        return bit2->num_row;
}

/************************** Bit2_put **************************
*
* This function puts a value in the array at the specified row and col, and
* returns an int of the most recently added value
* 
* Parameters:
*      T bit2:    the specified array
*      int col: column in which to put value
*      int row: row in which to put value
*      int value: value to insert
*
* Return: 
*       returns int of most recently added value
* Expects
*       Bit2_T is a valid pointer (not NULL) and is correctly initialized
*      
* 
* Notes:
*       Will checked runtime error (CRE) if Bit2_T is NULL
*                        
*****************************************************************/   
int Bit2_put(T bit2, int col, int row, int value) 
{
        assert(bit2 != NULL);
        Bit_T *bit_vector = (Bit_T *)UArray_at(bit2->cols_array, col);
        assert(bit_vector != NULL);
        int prev_value = Bit_put(*bit_vector,row, value);
        return prev_value;
}


/************************** Bit2_get **************************
*
* This function retrieves the value of the bit at the specified row and col.
* Returns specified bit.
* 
* Parameters:
*      T bit2:    the specified array
*      int col: column of desired bit
*      int row: row of desired bit
*
* Return: 
*       returns the int of the desired bit
* Expects
*       Bit2_T is a valid pointer (not NULL) and is correctly initialized
*      
* 
* Notes:
*       Will checked runtime error (CRE) if Bit2_T is NULL
*                        
*****************************************************************/   
int Bit2_get(T bit2, int col, int row) {
        assert(bit2 != NULL);
        Bit_T *bit_vector = (Bit_T *)UArray_at(bit2->cols_array, col);
        assert(bit_vector != NULL);
        int desired_bit = Bit_get(*bit_vector, row);
        return desired_bit;
}

/************************** Bit2_map_col_major **************************
*
* This function will traverse the array in a column major order 
* for every element in the array, applying the apply function to each element.
* 
* Parameters:
*      T bit2        the desired bit 2 array
*      void (*function_name)(int col, int row, T bit2, int correct_val, 
*            void *cl
*            a function pointer to the function to be applied to each element.
*               This function takes five parameters:
*               a: the 2D array
*               p1: a pointer
*               cl: a closure pointer
*      void *cl    The closure pointer
*
* Return: nothing
* Expects
* an apply function with the specified parameters.      
*       
* Notes:
* cl is a closure and p1 is pointer to an element. 
*       
*****************************************************************/
void Bit2_map_col_major(T bit2, 
            void (*function_name)(int col, int row, 
            T bit2, int correct_val, void *cl), void *cl) {
        
        assert(bit2 != NULL);
        assert(function_name != NULL);
        assert(cl != NULL);
        for (int i = 0; i < bit2->num_col; i++) {
                for (int j = 0; j < bit2->num_row; j++) {
                        int correct_val = Bit2_get(bit2, i, j);
                        function_name(i, j, bit2, correct_val, cl);
                    }
        }
}

/************************** Bit2_map_row_major **************************
*
* This function will traverse the array in a column major order 
* for every element in the array, applying the apply function to each element.
* 
* Parameters:
*      T bit2        the desired bit 2 array
*      void (*function_name)(int col, int row, T bit2, int correct_val, 
*      void *cl
*       a function pointer to the function to be applied to each element.
*               This function takes five parameters.
*               T bit2: the 2D array
*               int correct_val: current element
*               void *cl: a closure pointer
*      void *cl    The closure pointer 
*
* Return: nothing
* Expects
* an apply function with the specified parameters.      
*       
* Notes:
* cl is a closure.
*       
*****************************************************************/
void Bit2_map_row_major(T bit2, 
            void (*function_name)(int col, int row, 
            T bit2, int correct_val, void *cl), void *cl) 
            {
        assert(bit2 != NULL);
        assert(function_name != NULL);
        assert(cl != NULL);
        for (int i = 0; i < bit2->num_row; i++) {
                for (int j = 0; j < bit2->num_col; j++) {
                        int correct_val = Bit2_get(bit2, j, i);
                        function_name(j, i, bit2, correct_val, cl);
            }
        }
}


/************************** Bit2_free **************************
*
* This function deallocates the memory used by the given bit2, including
* nested elements.     
* 
* Parameters:
*      T *bit2:  a pointer to the array for which to free memory
*
* Return: 
*       nothing
* Expects:
*       *bit2 is a valid pointer (not NULL) to a correctly initialized 
*       bit2.     
*       
* Notes:
* If bit2 is NULL, then it will CRE
*       
*****************************************************************/
void Bit2_free(T *bit2) 
{
        assert(bit2 != NULL);
        /* freeing each inner bit vector */
        for (int i = 0; i < (*bit2)->num_col; i++) {
                Bit_T *current_element = 
                                (Bit_T *)UArray_at((*bit2)->cols_array, i);
                assert(current_element != NULL);
                Bit_free(current_element);
        }
        /*free outer array */
        assert((*bit2)->cols_array != NULL); 
        UArray_free(&((*bit2)->cols_array));
        free(*bit2); 
}

