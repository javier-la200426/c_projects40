/**************************************************************
 *
 *                     uarray2b.c
 *
 *     Assignment: Locality
 *     Authors:  Javier Laveaga(jlavea01), William Magann (wmagan01)
 *     Date:  2/22/2024
 *
 *     This file creates the the functions that carry out the
 *     implementation of a 2D blocked array. Uses a 2D array of blocks
 *     where each block is a 1D UArray.
 *
 *    
 *
 **************************************************************/


#include "uarray2b.h"
#include "uarray2.h"
#include "uarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#define T UArray2b_T

/************************** UArray2b_T struct **************************
*
* holds info about our map from our image
* 
* Parameters:
*    
*       UArray2_T array2:  Array to hold 2D array
*       int height:       Number of rows in array 
*       int width:        Number of columns in array 
*       int size:         size of elements each cell 
*       int blocksize:   number of cells on one side of block 
*                         
*****************************************************************/
struct T {
        UArray2_T array2;  /* Array to hold 2D array*/
        int height;        /* Number of rows in array */
        int width;         /* Number of columns in array */
        int size;          /* size of elements each cell */
        int blocksize;     /* number of cells on one side of block */
};

/************************** block_info struct **************************
*
* holds header info about our map from our image
* 
* Parameters:
*    
*       int blocksize: The number of cells in one side of block size 
*       int element_size: the size of each element in cell
*                         
*****************************************************************/
struct block_info {
        int blocksize;
        int element_size;
};

/************************** UArray2b_new_64K_block **************************
*
* This function intiilaizes for maximum block size of given element sizes
* and initializes blocked array with that blocksize
*         
* 
* Parameters:
*       int width:   the width of the 2D array (# of columns of cells)
*       int height   the height of the 2D array (# of rows of cells)
*       int size     the size of each cell
*
* Return: 
        A new UArray2b_T representing the 2D array holding 1d arrays 
*
* Expects:
*       width >= 0:    Array width must be a positive integer.
*       height >= 0:   Array height must be a positive integer
*       size >= 0:     Size of each element must be a positive integer.
*
* Notes:
*      When the element in a cell is larger than the maximum blocksize,
*      blocksize is set to 1
*      once blocksize calculates, calls UArray2
*                        
*****************************************************************/
T UArray2b_new_64K_block(int width, int height, int size)
{
        double blocksize = 1;

        /* Calculates the maximum number of cells of given element
        size that can fit in 64KB */
        if (size <= (1024 * 65)) {   
                double temp = (1024 / (double)size) * 64;
                blocksize = sqrt(temp);
        }

        return UArray2b_new(width, height, size, (int)blocksize);
}

/************************** fill_array**************************
*
* This is an apply function that is called by UArray2b_new to intialize
* each block with a 1D array
*         
* 
* Parameters:
*       int col:   The desired column to access in the blocks 2d array
*       int row:   the desired row to access in the blocks 2d array
*       int size     the size of each cell
*
* Return: 
*       Nothing
*
* Expects:
*       Column and row indices to be in bounds
*       Expects cl to be an initialized block_info structure
*       An intialized 2D blocked array
*
* Notes:
*      Throws a CRE if array or cl are NULL
*      
*       Memory allocated on the heap by this function is freed
*       by the free_array() apply function. That function is called
*       by UArray_free() which in turn has to be called by the client
*       at the end of their program. The client does not call free_array()
*       directly though and thus frees the memory allocated here indirectly
*                        
*****************************************************************/
void fill_array(int col, int row, UArray2_T array2, void *elem, void *cl)
{
        assert(array2 != NULL);
        assert(cl != NULL);

        struct block_info *bl_info = (struct block_info*) cl;

        int blocksize = bl_info->blocksize;
        int size = bl_info->element_size;
        
        UArray_T *curr_element = (UArray_T*)UArray2_at(array2, col, row); 
        assert(curr_element != NULL);

        /* assigns UArray to element */
        *curr_element = UArray_new((blocksize * blocksize), size);
              
        /* assigns UArray to element */
        assert (*(UArray_T*)UArray2_at(array2, col, row) != NULL);

        (void) elem;
}

/************************** UArray2b_new  **************************
*
* This function will initialize a 2D array of blocks with each block 
* being a 1D array.     
* 
* Parameters:
*       int width:   the width of the 2D array (# of columns)
*       int height:  the height of the 2D array (# of rows)
*       int size:    the size of an element of the array in bytes
*       int block_size: the number of cells in one side of block 
*
* Return: 
*        A new UArray2bb_T representing the 2D array of blocks
*
* Expects:
*       width >= 0:    Array width must be a positive integer.
*       height >= 0:   Array height must be a positive integer
*       size >= 0:     Size of each cell must be a positive integer.
*       block_size >= 0: blocksize is a positive integer
*
* Notes:
*       Will allocate new space for the blocks in a 2d Array 
*       Uses a 1D Uarray to represent each block
*       Failure to successfully allocate needed memory will result in a 
*       checked run-time error. CRE if width, height, or blocksize are negative
*       
*       Memory allocated in the heap by this function is freed by UArray_free()
*       and the client is responsible for accuratelt calling that function at
*       the end of their program
*                        
*****************************************************************/
T UArray2b_new (int width, int height, int size, int blocksize)
{
        assert(width >= 0);
        assert(height >= 0);
        assert(size >= 0);
        assert(blocksize >= 0);
        /*Allocate memory for struct*/
        T array2b = malloc(sizeof(struct T));
        assert(array2b != NULL);
        /*Calculate number of block columns*/
        int b_width = (width / blocksize);
        if (width % blocksize != 0) {
                b_width ++;
        }
        /*Calculate number of block rows*/
        int b_height = (height / blocksize);
        if (height % blocksize != 0) {
                b_height ++;
        }
        /*Initilaizes 2D array of blocks*/
        array2b->array2 = UArray2_new(b_width, b_height, sizeof(UArray_T));
        
        /* set array 2 elements to proper values*/
        array2b->width = width;
        array2b->height = height;
        array2b->size = size;
        array2b->blocksize = blocksize;

        /*Initialize struct for mapping function*/
        struct block_info* b_info = (struct block_info*)malloc
                (sizeof(struct block_info));
        assert(b_info != NULL);
        b_info->blocksize = blocksize; 
        b_info->element_size = size; /*size of each element*/

        /*Fill in 2dBlock array with 1D arrays, which represent one block*/
        UArray2_map_row_major(array2b->array2, fill_array, b_info);
        free(b_info);
        return array2b;
}


/************************** free_array **************************
*
* This is the apply function used for deallocating the memory taken by 
* a block in the 2d array of blocks  
* 
* Parameters:
*      int col: The current column in the 2d Blocks array
*      int row: The current row in the 2d Blocks array
*      UArray2_T array2: The 2d array of blocks
*      void *elem: The element of the array (not used)
*      void *cl: not used so NULL
*
* Return: 
*       nothing
* Expects:
*       uarray2 is a valid pointer (not NULL) to a correctly initialized 
*       2D array of blocks   
*       
* Notes:
*       Will CRE if  array to is NULL
* 
*****************************************************************/
void free_array(int col, int row, UArray2_T array2, void *elem, void *cl)
{
        assert(array2 != NULL);
        (void) elem;
        (void) cl;
        
        UArray_T *curr_element = (UArray_T*)UArray2_at(array2, col, row); 

        UArray_free(curr_element);
}

/************************** UArray2b_free  **************************
*
* This function deallocates the memory used by the blocks array by 
* majorig over the 2d blocks array and deallocating memory for each block
* 
* Parameters:
*      T *array2b: A pointer to the blocked array to be freed
*
* Return: 
*       nothing
* Expects:
*       array2b is a valid pointer (not NULL) to a correctly initialized 
*       array2b   
*       
* Notes:
* Will CRE if the array provided is NULL 
*       
*****************************************************************/
void UArray2b_free (T *array2b) 
{
        assert(array2b != NULL);

        /* freeing each element (each block) */
        UArray2_map_row_major((*array2b)->array2, free_array, NULL);

        /* frees UArray2 membeer in UArray2b struct*/
        UArray2_free(&((*array2b)->array2));

        /* frees array2b struct*/
        free(*array2b);
}

/************************** UArrayb2_width **************************
*
* This function returns the width (int) of the UArray2b.          
* 
* Parameters:
*      T array2b:    the specified UArray2b
*
* Return: 
*       returns an int to represent the width of the UArray2b (# of columns)
* Expects
*       uarray2b is a valid pointer (not NULL) to a correctly initialized 
*       UArray2b.      
* 
* Notes:
*       Will checked runtime error if uarray2b is not properly initialized
*       or NULL
*                        
*****************************************************************/  
int UArray2b_width (T array2b)
{
        assert(array2b != NULL);
        return array2b->width;
}
 
/************************** UArray2b_height **************************
*
* This function returns the height (int) of the UArray2b.          
* 
* Parameters:
*      T array2b:   the specified UArray2b
*
* Return: 
*       returns the int of height of the UArray2b.
* Expects:
*       uarray2b is a valid pointer (not NULL) to a correctly initialized 
*       UArray2b.
*      
* Notes:
*       Will checked runtime error if uarray2b is not properly initialized
*       or NULL
*                        
*****************************************************************/
int UArray2b_height (T array2b)
{
        assert(array2b != NULL);
        return array2b->height;
}

/************************** UArray2b_size **************************
*
* This function returns the size of each element in the UArray2b.
* Parameters:
*      T array2b:   the specified UArray2b
*
* Return: 
*       returns an int that represents the size of each element in the 
*       UArray2b.
* Expects:
*       uarray2b is a valid pointer (not NULL) to a correctly initialized 
*       UArray2b.
* Notes:
*       Will checked runtime error if uarray2b is not properly initialized
*       or NULL
*                        
*****************************************************************/
int UArray2b_size (T array2b)
{
        assert(array2b != NULL);
        return array2b->size;
}

/************************** UArray2b_blocksize **************************
*
* This function returns the size of one side of the blocks in the UArray2b.
* Parameters:
*      T array2b:   the specified UArray2b
*
* Return: 
*       returns an int that represents the size of the side of one block in 
*       UArray2b.
* Expects:
*       uarray2b is a valid pointer (not NULL) to a correctly initialized 
*       UArray2b.
* Notes:
*       Will checked runtime error if uarray2b is not properly initialized
*       or NULL
*                        
*****************************************************************/
int UArray2b_blocksize(T array2b)
{
        assert(array2b != NULL);
        return array2b->blocksize;
}

/************************** UArray2b_at **************************
*
* This function accesses the element of given array at the specified indices.
* 
* Parameters:
*      T array2b:   the specified UArray2b
*      int col      an integer to represent the desired column of the array
*      int row      an integer to represent the desired row of the array
*
* Return: 
*       returns the element at the specified indices (as a void pointer)
* Expects:
*       uarray2b is a valid pointer (not NULL) to a correctly initialized 
*       UArray2b.
*       Specified indices are within bounds of the specified array.
* Notes:
*       Will checked runtime error if uarray2b is not properly initialized
*       or NULL, or if specified indices are outside bounds of the array.
*                        
*****************************************************************/
void *UArray2b_at(T array2b, int col, int row)
{
        assert(array2b != NULL);
        
        /* width and height of array2b */
        int width = array2b->width;
        int height = array2b->height;

        /* check if index in array or not*/
        if((col >= width) || (row >= height) || row < 0 || col < 0) {
                fprintf(stderr, "Error: Index out of bounds\n");
                exit(EXIT_FAILURE);
        }
        
        int blocksize = array2b->blocksize;

        /* What block row and block column t*/
        int b_row = row / blocksize;
        int b_col = col / blocksize;

        /* Gets pointer to certain index og block in array*/
        UArray_T *curr_element = UArray2_at(array2b->array2, b_col, b_row);

        /* Calculates index of where cell is in given block*/
        int index = (blocksize * (row % blocksize)) + (col % blocksize);

        /* Returns a void pointer to the element at the given index*/
        return UArray_at(*curr_element, index);
}

/************************** map_help **************************
*
* This function accesses the element of given array at the specified indices. 
* 
* Parameters:

void apply(int col, int row, T array2b, void *elem, void *cl):

*       T array2b:    an initialized array of blocks
*       void *l:      the closer for the apply function
*       int b_length: the number of cells in a block 
*       blocksize:    The number of cells on one side of block
*       int col_num:  the column number fo the cell
*       int row_num:  the row number of the cell
*       int height:   height of the two 2 array
*       int width:    width of the two array 
*       UArray_T *curr_block: A pointer to the current block
*     
* Return: 
*      Nothing
* Expects:
*       An initialized array of blocks and apply function with specified 
*       parameters
* Notes:
*       Will checked runtime error if array2b, apply, or l are NULL
*                        
*****************************************************************/
void map_help(void apply(int col, int row, T array2b, void *elem, void *cl),
              T array2b, void *l, int b_length, int blocksize, 
              int col_num, int row_num, int height, int width, 
              UArray_T *curr_block) 
{
        assert(curr_block != NULL); 
        assert(apply != NULL);
        
        /* iterates through all cells in block*/
        for (int i = 0; i < b_length; i++) {

                /* finds cells block level index */
                int col_i = col_num + (i % blocksize);
                int row_i = row_num + (i / blocksize);
                
                /* ignores empty cells on the bottom of the last block row */
                if (row_i < height) {
                        /* ignores empty cells in the rightmost block column */
                        if (col_i < width) {
                                void *elem = UArray_at(*curr_block, i);
                                apply(col_i, row_i, array2b, elem, l);
                        }
                }              
        }
}


/************************** UArray2b_map **************************
*
* This function accesses the element of given array at the specified indices.  
* 
* Parameters:
*      T array2b:                         an initialized array of blocks
*      void apply(int col, int row, 
*       T array2b, void *elem, void *cl): apply function to be used for every
*                                         cell in all blocks
*      void *l:                          void pointer to pass to apply function
*      
*
* Return: 
*      Nothing
* Expects:
*       An initialized array of blocks and apply function with specified 
*       parameters
* Notes:
*       Will checked runtime error if array2b, apply, or l are NULL
*                        
*****************************************************************/
void UArray2b_map(T array2b, void apply(int col, int row, T array2b, 
                                        void *elem, void *cl), void *l) 
{
        assert(array2b != NULL);
        assert(apply != NULL);

        /* Gets height width and blocksize from array2b*/
        int height = array2b->height;
        int width = array2b->width;
        int blocksize = array2b->blocksize;

        /* Finds number of block rows */
        int b_width = (width / blocksize);
        if (width % blocksize != 0) {
                b_width ++;
        }

        /* Finds number of block columns */
        int b_height = (height / blocksize);
         if (height % blocksize != 0) {
                b_height ++;
        }

        /* number of elements in one block*/
        int b_length = blocksize * blocksize;

        int row_num  = 0; /*counter variable for row of cells*/

        /* iterates through number of block rows*/
        for (int b_row = 0; b_row < b_height; b_row ++) {
                int col_num = 0; /*counter variable for column of cells*/

                /* iterates through number of block columns*/
                for (int b_col = 0; b_col < b_width; b_col ++) {

                        /* finds current block*/
                        UArray_T *curr_block = (UArray_T *)UArray2_at(
                                                array2b->array2, b_col, b_row);

                        /* helper function to map through blocks*/
                        map_help(apply, array2b, l, b_length, blocksize,
                                 col_num, row_num, height, width, curr_block);
                        
                        col_num += blocksize;
                }
                row_num += blocksize;
        }
}

