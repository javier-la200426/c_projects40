/*
 *     sudoku.c
 *     by Javier Laveaga {jlavea01} and Mallika Rangan {mranga02}, 
 *     2/12/2024
 *     Assignment: iii
 *
 *     Summary: This file contains the implementation for sudoku.c. It is to
 *     be used with sudoku.h, which contains the interface. This program takes
 *     in a single portable graymap (pgm) that represents a sudoku solution
 *     to be checked for correctness. It then checks whether the sudoku is 
 *     correct for each row, column, and subgraph. It returns a 0 if the
 *     sudoku is correct, and 1 otherwise.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <uarray2.h>
#include <pnmrdr.h>
#include <bit.h>

/************************** Pnmrdr_mapdata struct **************************
*
* holds header info about our map from our image
* 
* Parameters:
*    
*        Pnmrdr_maptype type: magic number
*        unsigned width, height: maxval of header
*        unsigned denominator: denominator
*                         
*****************************************************************/
struct Pnmrdr_mapdata {
        Pnmrdr_maptype type; /* magic number */
        unsigned width, height; /*maxval*/  
        unsigned denominator; 
};

/************************** graymap_info struct **************************
*
* holds info about our graymap
* 
* Parameters:
*    
*        int Maxval - the maximum value we encounter in each input file
*        Pnmrdr_T pnmReader - pointer to our reader
*       
*                         
*****************************************************************/
struct graymap_info {
        int *Maxval;
        Pnmrdr_T pnmReader;
};

/************************** bit_info struct **************************
*
* holds info about our sudoku image
* 
* Parameters:
*    
*        int *count: count of how many times we've encountered each value
*        Bit_T bit_map: pointer to bit map
*        bool *solved - whether or not sudoku has been solved
*                         
*****************************************************************/
struct bit_info {
        int *count;
        Bit_T bit_map;
        bool *solved;
};


/************************** max_val **************************
*
* This function calculates the maximum value encountered so far.
* 
* Parameters:
*       int curr_pixel - integer of current pixel, to be evaluated to see
*       whether it is the max value or not
*       int Max: current max value
*      
*
* Return: 
*       int of the new max value
*
* Expects:
*       valid curr_pixel
*                        
*****************************************************************/
int max_val (int curr_pixel, int Max) 
{
        if (curr_pixel > Max) {
                Max = curr_pixel;
        }
        return Max;
}

/************************** get_pixel **************************
*
* This function retreives a pixel value from Pnmrdr_T reader, updates the 
* max pixel value seen, and stores the pixel value in the specified position
* in the 2D array. Returns void.
* 
* Parameters:
*       int col: Column index of the pixel in the 2D array.
*       int row: Row index of the pixel in the 2D array.
*       UArray2_T test_array: 2D array that represents sudoku board to be
*       populated with pixel values.
*       void *p1: Unused parameter.
*       void *info: Pointer to struct graymap_info, which contains info about
*       the Pnmrdr_T reader and the max pixel value seen so far.
*
* Return: 
*       Void
*
* Expects:
*       Col and row must be valid indices within test_array. Checked runtime
*       error if not.
*       test_array must be valid UArray2_T.
*       info must point to valid struct graymap_info.
*       Pixel value retrieved from Pnmrdr_T must be within valid range (0-9
*       inclusive). doublecheck
*
* Notes:
*       *p1 parameter is unused
*                        
*****************************************************************/
void get_pixel(int col, int row, UArray2_T test_array, void *p1, void *info) 
{
        assert(info != NULL);
        struct graymap_info *new_info = (struct graymap_info*) info;
        assert(new_info != NULL);
        assert(new_info->Maxval != NULL);

        Pnmrdr_T new_Reader = new_info->pnmReader;
        (void) p1;
        
        int curr_pixel = Pnmrdr_get(new_Reader);
        int curr_max = *(new_info->Maxval);
        *(new_info->Maxval) = max_val (curr_max, curr_pixel); /*Update max*/
        
        /* Set element of array to current pixel read from file
        get our slot in the array */
        int *slot = (int*)UArray2_at(test_array, col, row);
        assert(slot != NULL);
        *slot = curr_pixel; /*set element to current pixel */
}

/************************** store_pgm **************************
*
* This function reads a pgm file using Pnmrdr_T reader, initializes a 2D
* array, and populates the array with pixel values from the pgm file. Keeps
* track of the max pixel value seen in the image file. Returns populated 2D
* array, representing sudoku board to be validated.
* 
* 
* Parameters:
*       Pnmrdr_T newReader: A Pnmrdr_T reader for reading the PGM file
*       int *Maxval: pointer to int representing the max pixel value seen in
*       the image file.
*
* Return: 
*       A new UArray2_T, representing the 2D array populated with pixel values
*       from the image file.
*
* Expects:
*       newReader is a valid Pnmrdr_T reader.
*       Maxval is a valid pointer to an int that stores the max pixel value
*       PGM image contains valid pixel values.
*       
*
* Notes:
*       Allocates memory for graymap_info info struct; check runtime error
*       if memory allocation fails. Memory is freed before returning.
*                        
*****************************************************************/
UArray2_T store_pgm(Pnmrdr_T newReader, int *Maxval) 
{
    /*  initializing array and struct */
        UArray2_T test_array;
        test_array = UArray2_new(9, 9, sizeof(int));
        assert(test_array != NULL);
        struct graymap_info* info = (struct graymap_info*)malloc
                (sizeof(struct graymap_info));
        assert(info != NULL);

        /* initialize struct members */
        info->Maxval = Maxval;
        info->pnmReader = newReader;
        /* store numbers in array and keep track of Maxval */
        UArray2_map_row_major(test_array, get_pixel, (void*)info);

        free(info);
        return test_array;
}

/************************** check_dups **************************
*
* This function checks for duplicates within a row or column of a 2D array.
* It uses a bit map to keep track of the number of each element that is seen
* in each row and in each column. If any row/column contains a duplicate,
* boolean is set to false to indicate an invalid sudoku.
* 
* 
* Parameters:
*       int col: Column index of the pixel in the 2D array.
*       int row: Row index of the pixel in the 2D array.
*       UArray2_T test_array: 2D array that represents sudoku board to be
*       validated.
*       void *p1: Unused parameter.
*       void *info: Pointer to struct bit_info, which contains info about
*       the bit map and solved boolean flag.
* Return: 
*       void
*
* Expects:
*       Col and row must be valid indices within test_array. Checked runtime
*       error if not.
*       test_array must be valid UArray2_T.
*       info must point to valid struct bit_info
*
* Notes:
*       If dupe is encountered, solved flag is set to false
*                        
*****************************************************************/
void check_dups(int col, int row, UArray2_T test_array, void *p1, void *info) 
{
        assert(info != NULL);
        (void) p1; 
        struct bit_info *new_info = (struct bit_info*) info; /*get struct*/
        assert(new_info != NULL);

        int *count = new_info->count; /*get count from struct*/
        Bit_T bit_map = new_info->bit_map; /*get map*/
        bool *solved = new_info->solved; /*get boolean*/
        assert(new_info->count != NULL);
        assert(new_info->bit_map != NULL);
        assert(new_info->solved != NULL);

        /*get elementp*/
        int *element_p = (int*)UArray2_at(test_array, col, row); 
        assert(element_p != NULL);
        int curr_element = *element_p;
        /* to prevent access to outside of bitmap */
        int curr_bit; /*initialized to 'null' value*/
        /*checking it is in range*/
        if (curr_element > 0 && curr_element < 10) { 
                /*get bit at elementindex*/
                curr_bit = Bit_get(bit_map, curr_element); 
                if (curr_bit == 0) { /*not seen yet */
                        Bit_put(bit_map, curr_element, 1);
                /*it has been seen, thus it is a duplicate*/        
                } else if (curr_bit == 1) { 
                        *solved = false;
                        
                }
        } else { /*if outside of range */
                *solved = false;
        }
        
        (*count)++;
        /* clearing vector after end of row/column */
        if ((*count) == 9) {
                Bit_clear(bit_map, 0,9);
                *count = 0;
        }
}


/************************** valid_row_col **************************
*
* This function checks for duplicate values in rows and colums of a 2D array. 
* It initializes a bit vector to hold information about the presence of 
* duplicates, maps over rows and columns of the 2D array to check for
* duplicate values, checks and updates the bit vector accordingly. Returns true
* if no duplicates are found in each row and no duplicates are found in each
* column; returns false otherwise.
* 
* Parameters:
*       UArray2_T filled_array: 2D array that represents sudoku board to check
*
* Return: 
*       Bool to represent whether the given sudoku board is valid for the row
*       and column check (true if valid, false otherwise)
*
* Expects:
*       Valid UArray2_T 2D array that represents a sudoku board to check.
*
* Notes:
*       Allocates new space for bit info struct, frees before returning.
*       Failure to successfully allocate needed memory will result in a 
*       checked run-time error
*                        
*****************************************************************/
bool valid_row_col (UArray2_T filled_array) 
{
        struct bit_info* bit_struc = (struct bit_info*)malloc
                (sizeof(struct bit_info));
        assert(bit_struc != NULL);
        /* initialize struct members  */
        int count = 0; 
        bit_struc->count = &count; /*initialize count member*/

        Bit_T bit_map; 
        bit_map = Bit_new(10); /*vector already contains 0's*/
        bit_struc->bit_map = bit_map; /*initialize bit_map member*/

        bool solved = true;
        bool *solved_p = &solved; 
        bit_struc->solved = solved_p; /*initialize boolean member*/
        /* check duplicates in rows */
        UArray2_map_row_major(filled_array, check_dups, bit_struc);
       
        /*check duplicates in columns*/
        UArray2_map_col_major(filled_array, check_dups, bit_struc);
        Bit_free(&bit_map);

        if (*solved_p == true) {
                free(bit_struc);
                return true;
        } 
        free(bit_struc);
        return false;
}


/************************** valid_subgraphs **************************
*
* This function checks for duplicate values in each 3x3 subgraph a 2D array. 
* Returns true if no duplicates are found in each row and no duplicates are 
* found in each subgraph.
* 
* 
* Parameters:
*       UArray2_T filled_array: 2D array that represents sudoku board to check
*
* Return: 
*       Bool to represent whether the given sudoku board is valid for all
*       subgraphs.
*
* Expects:
*       Valid UArray2_T 2D array that represents a sudoku board to check.
*
* Notes:
*       Assuming we have elements in range (0-9) inclusive
*       
*                        
****************************************************************/

bool valid_subgraphs (UArray2_T filled_array) 
{
        /*traverse in a left right, then down manner*/
        /*start at top left*/
        int row = 0;
        int col = 0;
        int curr_bit;
        bool solved = true;
        int *element_p;
        Bit_T bit_map = Bit_new(10);
        for (int i = row; i < 9; i += 3) { /*iterate over start of row */
                for (int j = col; j < 9; j += 3) { /*iterate over start col*/ 
                        /*iterate over subgrid (from left to right first)*/
                        for (int row_s = 0; row_s < 3; row_s++) {
                                for (int col_s = 0; col_s < 3; col_s++) {
                                        element_p = (int*)UArray2_at
                                                (filled_array, 
                                                j + col_s, 
                                                i + row_s);
                                        curr_bit = Bit_get(bit_map, 
                                                *element_p);
                                        if (curr_bit == 0) {/*not seen yet */
                                                Bit_put(bit_map, 
                                                        *element_p, 1);
                                        /*it has been seen*/
                                        } else if (curr_bit == 1) { 
                                                solved = false;
                                        }
                                }
                        }
                Bit_clear(bit_map, 0,9);
                }   
        }
    Bit_free(&bit_map);
    return solved;
}


/************************** driver **************************
*
* This function drives our Sudoku checker. Valid sudoku boards return 0, and
* invalid boards return 1. Our suduoku checker uses a 2D array to represent
* the board, and checks each column, row, and subgraph for duplicates.     
* 
* Parameters:
*       FILE *fp file pointer of a valid pgm file, representing a sudoku
*       solution that may be invalid or valid
*
* Return: 
*       Bool to represent whether the given sudoku board is valid
*
* Expects:
*       Valid file pointer
*
* Notes:
*       If not a valid PGM, Pnmrdr will throw an exception.
*       
*                        
****************************************************************/

bool driver(FILE *fp) 
{
        assert(fp != NULL);
        Pnmrdr_T newReader = Pnmrdr_new(fp);
        Pnmrdr_mapdata newMapData = Pnmrdr_data(newReader);
       
        /* check pgm header to make sure it fits sudoku format */
        /*Sudoku: P1 and P5*/
        assert(newMapData.type == 2); /*Check it is valid type*/
        if(newMapData.height != 9 || newMapData.width != 9) {
                Pnmrdr_free(&newReader);
                return false; /*not solved if dimensions are wrong*/
        }
        int MaxVal = 0;
        /*store numbers of the PGM into the array*/
        UArray2_T filled_array = store_pgm(newReader, &MaxVal);
        Pnmrdr_free(&newReader);

        bool solved_row_col = true; 
        solved_row_col = valid_row_col (filled_array);
        if (MaxVal != 9) { /*Might do it in check dups*/
                solved_row_col = false;
        }

        /*checking subgraphs*/
        /*if it is true, it means that all elements are b/t 1 and 9 inclusive*/
        if (solved_row_col == true) { 
                solved_row_col = valid_subgraphs(filled_array);
        }

        if (solved_row_col == true) {
                UArray2_free(&filled_array); 
                return true;
        }

        UArray2_free(&filled_array);  
        return false;
}


/************************** main **************************
*
* This function verifies that the correct args are passed, and handles whether
* the file pointer should read from stdin or a given file.
* 
* Parameters:
*       User can either input a file, or run the program and then input 
*       contents to stdin
* Return: 
*       Exit success if successful sudoku board, exit failure otherwise
*
* Expects:
*       either file or stdin
*            
****************************************************************/
int main(int argc, char *argv[]) 
{
        FILE *fp;
        
        assert(argc <= 2);
        
        if (argc == 2) {
                /*read file*/
                fp = fopen(argv[1], "r");
                assert(fp != NULL);
        } else if (argc == 1) {
                fp = stdin;
        }
        bool solved = driver(fp);
        assert(solved == true || solved == false);

        assert(fp != NULL);
        fclose(fp);
        if (solved == true) {
                exit(EXIT_SUCCESS);
        } 
        exit(EXIT_FAILURE);
}

