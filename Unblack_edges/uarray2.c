/*
 *     uarray2.c
 *     by Javier Laveaga {jlavea01} and Mallika Rangan {mranga02}, 
 *     2/12/2024
 *     Assignment: iii
 *
 *     uarray2.c is the uarray2 implementation for a 2D array. It is to be 
 *     used with the uarray2.h interface. This program allows the client to
 *     create and populate a 2D array, get the width and height, 
 *     use row and column mapping capabilities, and free memory for the 2D 
 *     array.
 */



#include "uarray2.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define T UArray2_T

struct T {
        UArray_T cols_array;
        int num_col;
        int num_row;
        int num_elements;
        int size_elem;
};


/************************** UArray2_new **************************
*
* This function will initialize a 2D array with the specified width, height,
* and element size. Allocates memory for array's elements, returns a new
* UArray2_T.         
* 
* Parameters:
*       int width:   the width of the 2D array (# of columns)
*       int height   the height of the 2D array (# of rows)
*       int size     the size of an element of the array in bytes
*
* Return: 
        A new UArray2_T representing the 2D array
*
* Expects:
*       width >= 0:    Array width must be a positive integer.
*       height >= 0:   Array height must be a positive integer
*       size >= 0:     Size of each element must be a positive integer.
*
* Notes:
*       Will allocate new space for the elements of the array. 
*       Uses a 1D Uarray to represent a 2D array. 
*       Failure to successfully allocate needed memory will result in a 
*       checked run-time error
*                        
*****************************************************************/
T UArray2_new(int col, int row, int size) {
    /*  check to see that array dimensions are >= 0 */
        assert (col >= 0 && row >= 0);
    /*  Allocate memory for the UArray2_T struct, which reps our 2D array */
        T new_array = (T)malloc(sizeof(struct T));
        assert(new_array != NULL);

        new_array->num_row = row;
        new_array->size_elem = size;
        
    /*  Allocate memory for an array of UArray_T instances, each of which
        represent a column of our 2D Array */
        new_array->cols_array = UArray_new(col, sizeof(UArray_T));
        assert(new_array->cols_array != NULL);

    /*  Create new UArray_T instance for each column of our 2D Array
        Filling each element with a new (inner) array */
        for (int i = 0; i < col; i++) {
                UArray_T inner_array = UArray_new(row, size);
                assert(inner_array != NULL);
                UArray_T *element = 
                        (UArray_T*)UArray_at(new_array->cols_array, i);
                assert(element != NULL);
                *element = inner_array;
        }
        new_array->num_col = UArray_length(new_array->cols_array);
        return new_array;
}
    

/************************** UArray2_width **************************
*
* This function returns the width (int) of the UArray2.          
* 
* Parameters:
*      T uarray2:    the specified UArray2
*
* Return: 
*       returns an int to represent the width of the UArray2 (# of columns)
* Expects
*       uarray2 is a valid pointer (not NULL) to a correctly initialized 
*       UArray2.*       
* 
* Notes:
*       Will checked runtime error (CRE) if uarray2 is not properly initialized
*       or NULL
*                        
*****************************************************************/  
int UArray2_width (T uarray2) 
{
        assert(uarray2 != NULL);
        return uarray2->num_col;
}


/************************** UArray2_height **************************
*
* This function returns the height (int) of the UArray2.          
* 
* Parameters:
*      T uarray2:   the specified UArray2
*
* Return: 
*       returns the height of the UArray2.
* Expects:
*       uarray2 is a valid pointer (not NULL) to a correctly initialized 
*       UArray2.
*      
* Notes:
*       Will checked runtime error (CRE) if uarray2 is not properly initialized
*       or NULL
*                        
*****************************************************************/
int UArray2_height (T uarray2) 
{
        assert(uarray2 != NULL);
        return uarray2->num_row;
}



/************************** UArray2_size **************************
*
* This function returns the size of each element in the UArray2.
* Parameters:
*      T uarray2:   the specified UArray2
*
* Return: 
*       returns an int that represents the size of each element in the 
*       UArray2.
* Expects:
*       uarray2 is a valid pointer (not NULL) to a correctly initialized 
*       UArray2.
* Notes:
*       Will checked runtime error (CRE) if uarray2 is not properly initialized
*       or NULL
*                        
*****************************************************************/
int UArray2_size (T uarray2) 
{
        assert(uarray2 != NULL);
        return uarray2->size_elem;
}



/************************** UArray2_at **************************
*
* This function accesses the element of given array at the specified indices. 
* 
* Parameters:
*      T uarray2:   the specified UArray2
*      int col      an integer to represent the desired column of the array
*      int row      an integer to represent the desired row of the array
*
* Return: 
*       returns the element at the specified indices (as a void pointer)
* Expects:
*       uarray2 is a valid pointer (not NULL) to a correctly initialized 
*       UArray2.
*       Specified indices are within bounds of the specified array.
* Notes:
*       Will checked runtime error (CRE) if uarray2 is not properly initialized
*       or NULL, or if specified indices are outside bounds of the array.
*                        
*****************************************************************/
void *UArray2_at(T uarray2, int col, int row) 
{
        assert(uarray2 != NULL);
        UArray_T *row_array = (UArray_T*) UArray_at(uarray2->cols_array, col);
        assert(row_array != NULL);
        void *element = UArray_at(*row_array, row);
        assert(element != NULL);
        return element;
}

/************************** UArray2_free **************************
*
* This function deallocates the memory used by the given array, including
* nested elements.     
* 
* Parameters:
*      T uarray2:   the array for which to free memory
*
* Return: 
*       nothing
* Expects:
*       uarray2 is a valid pointer (not NULL) to a correctly initialized 
*       UArray2.     
*       
* Notes:
* 
*       
*****************************************************************/
void UArray2_free(T *test_array) 
{
        assert(test_array != NULL);
        for (int i = 0; i < (*test_array)->num_col; i++) {
                UArray_T *inner_array = 
                        (UArray_T*) UArray_at((*test_array)->cols_array, i);
                assert(inner_array != NULL);
                UArray_free(inner_array); 
        }

        UArray_free(&((*test_array)->cols_array));
        free(*test_array);  
}

/************************** UArray2_map_row_major **************************
*
* This function will traverse the array in a row major order 
* for every element in the array, applying the apply function to each element.
*     
* 
* Parameters:
*      T a        the 2D array to traverse
*      void (*function_name)(int i, int j, T a, void *p1, void *cl) - 
*       a function pointer to the function to be applied to each element.
*               This function takes five parameters:
*               i: the column index
*               j: the row index
*               a: the 2D array
*               p1: a pointer to the current element
*               cl: a closure pointer
*      void *cl    The closure pointer to boolean variable OK
* Return: nothing
* Expects
*   an apply function with the specified parameters.      
*       
* Notes:
*   p2 is a closure and p1 is pointer to an element. 
*       
*****************************************************************/
void UArray2_map_row_major(T uarray2, 
        void (*function_name)(int col, int row, 
        T a, void *p1, void *cl), void *cl) 
{
        assert(uarray2 != NULL);
        assert(cl != NULL);
        assert(function_name != NULL);
        for (int i = 0; i < uarray2->num_row; i++) {
                for (int j = 0; j < uarray2->num_col; j++) {
                        void *p1 = UArray2_at(uarray2, j, i);
                        assert(p1 != NULL);
                        function_name(j, i, uarray2, p1, cl);
                }
        }
}

/************************** UArray2_map_col_major **************************
*
* This function will traverse the array in a column major order 
* for every element in the array, applying the apply function to each element.
* 
* Parameters:
*      T uarray2        the desired UArray2_t
*      void (*function_name)(int i, int j, T a, void *p1, void *cl) - 
*       a function pointer to the function to be applied to each element.
*               This function takes five parameters:
*               i: the column index
*               j: the row index
*               a: the 2D array
*               p1: a pointer to the current element
*               cl: a closure pointer
*      void *cl    The closure pointer to boolean variable OK
*
* Return: nothing
* Expects
* an apply function with the specified parameters.      
*       
* Notes:
* cl is a closure and p1 is pointer to an element. 
*       
*****************************************************************/
void UArray2_map_col_major(T uarray2, 
    void (*function_name)(int col, int row, 
    T a, void *p1, void *cl), void *cl) 
{
        assert(uarray2 != NULL);
        assert(cl != NULL);
        assert(function_name != NULL);
        
        for (int i = 0; i < uarray2->num_col; i++) {
                for (int j = 0; j < uarray2->num_row; j++) {
                        void *p1 = UArray2_at(uarray2, i, j); 
                        assert(p1 != NULL);
                        function_name(i, j, uarray2, p1, cl);
            }
        }       
}
