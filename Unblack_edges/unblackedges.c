/*
 *     unblackedges.c
 *     by Javier Laveaga {jlavea01} and Mallika Rangan {mranga02}, 
 *     2/12/2024
 *     Assignment: iii
 *
 *     unblackedges.c is the implementation to unblacken edges of a provided
 *     file. It is to be used with the unblackedges.h interface. Unblackedges
 *     takes in a pbm file, and assesses whether something is a black edge or
 *     not. If it is, it whitens the edge and prints a bitmap to represent
 *     the whitened pbm.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <bit2.h>
#include <pnmrdr.h>
#include <seq.h>


/************************** bit_info struct **************************
*
* holds info about col and row of the bit
* 
* Parameters:
*    
*       col:          The column index of the bit 
*       row:          The row index of the bit
                    
*****************************************************************/
struct bit_info {
        int col;
        int row;
};


/************************** add_seq **************************
*
* Adds a bit with value 1 at the specified column and row coordinates to
* a sequence (stack) for further processing.
* 
* Parameters:
*       filled_array: A Bit2_T 2D bit array representing the filled image.
*                     This array is used to retrieve the value of the specified
*                     bit.
*       stack:        A Seq_T sequence (stack) to which the bit information 
*                     will be added.
*       col:          The column index of the bit to be added to the sequence.
*       row:          The row index of the bit to be added to the sequence.
* Return: 
*       Void
*
* Expects:
*       Valid array, valid stack, in bounds column and row.
*
*                        
*****************************************************************/
void add_seq (Bit2_T filled_array, Seq_T stack, int col, int row) {
        assert (filled_array != NULL);
        assert (stack != NULL);
        /* Top row, horizontal */
        int curr_element = Bit2_get(filled_array, col, row);
        /* if curr element [i, 0] is black, add to queue */
        if (curr_element == 1) { 
                struct bit_info* bit_info = (struct bit_info*)malloc
                        (sizeof(struct bit_info));
                assert(bit_info != NULL);
                bit_info->col = col;
                bit_info->row = row;
                Seq_addhi(stack, (void*)bit_info); 
        }
}


/************************** process_bit **************************
*
* Processes each bit in a filled 2D bit array using an iterative approach.
* It updates the bit array by whitening each processed bit (setting
* its value to 0) and adds neighboring bits with value 1 to a sequence (stack)
* for further processing.
* 
* Parameters:
*       A Seq_T sequence that holds pixels to be processed
*       filled_array: A Bit2_T 2D bit array representing the filled image.
*                    
*
* Return: 
*       Void
*
* Expects:
*      Valid sequence, filled array
*
*                        
*****************************************************************/
/* adds stuff to queue and whitens black one */
void process_bit (Bit2_T filled_array, Seq_T stack) {
        int count = 0;
        assert (filled_array != NULL);
        assert (stack != NULL);
        while (Seq_length(stack) != 0) {
                struct bit_info *curr_struct = 
                        (struct bit_info*)Seq_remhi(stack);
                assert(curr_struct != NULL);
                int col = curr_struct->col;
                int row = curr_struct->row;

                Bit2_put(filled_array, col, row, 0); /*whiten bit*/
                
                /* Getting neighbors value (0 or 1) */
                int up = 0;
                if (row > 0 ) { /*prevent getting out of bounds*/
                        up = Bit2_get(filled_array, col, row - 1);
                }
                int right = 0;
                if (col < Bit2_width(filled_array) - 1 ) {
                        right= Bit2_get(filled_array, col + 1, row);
                }
                int down = 0;
                if (row < Bit2_height(filled_array) - 1 ) {
                        down = Bit2_get(filled_array, col, row + 1);
                }
                int left = 0;
                if (col > 0 ) {
                        left = Bit2_get(filled_array, col - 1, row);
                }
                /*adding to stack if equal to 1*/
                if (up == 1) {
                        add_seq(filled_array, stack, col, row-1);
                }
                if (right == 1) {
                        add_seq(filled_array, stack, col + 1, row);
                } 
                if (down == 1) {
                        add_seq(filled_array, stack, col, row + 1);
                } 
                
                if (left == 1) {
                        add_seq(filled_array, stack, col - 1, row);
                }
                free(curr_struct);
                count++;
        }
}


/************************** add_edges **************************
*
* This function adds the edges of a filled 2D bit array to a sequence for
* processing.
* 
* Parameters:
*       A Seq_T sequence to which the edges will be added. Functions as a stack
*       filled_array: A Bit2_T 2D bit array representing the filled image.
*                     The edges of this array will be added to the sequence.
*
* Return: 
*       Void
*
* Expects:
*       Valid sequence, valid filled array
*                        
*****************************************************************/
void add_edges(Seq_T stack, Bit2_T filled_array) {
        assert (filled_array != NULL);
        assert (stack != NULL);
        /*iterate over top row*/
        for (int i = 0; i < Bit2_width(filled_array) - 1; i++) {
                add_seq (filled_array, stack, i, 0);
        }
        /*iterate over rightmost column*/
        for (int i = 0; i < Bit2_height(filled_array) - 1; i++) {
                add_seq (filled_array, stack, Bit2_width(filled_array) - 1, i);
        }
        /*iterate over bottom row*/
        for (int i = 0; i < Bit2_width(filled_array); i++) {
                add_seq (filled_array, stack, i, 
                        Bit2_height(filled_array) - 1);
        }
        /*iterate over leftmost column*/
        for (int i = 1; i < Bit2_height(filled_array) - 1; i++) {
                add_seq (filled_array, stack, 0, i);
        }
        /*Note: -1s are to prevent iterating over same value twice*/
}


/************************** get_bit **************************
*
* Callback function used to read each bit from a portable bitmap (PBM) image 
* represented by a Pnmrdr_T object and store it in a Bit2_T (2D bit array)
* 
* Parameters:
*       int col: Column index of the pixel in the 2D array.
*       int row: Row index of the pixel in the 2D array.
*       bitarray: The Bit2_T object (2D bit array) in which to store the bit.
*       void *p1: Unused parameter.
*       the Pnmrdr_T reader
*
* Return: 
*       Void
*
* Expects:
*       Valid reader, col and int that are in bounds, valid bit array
* Notes:
*       *p1 parameter is unused
*                        
*****************************************************************/
/* apply function */
void get_bit(int col, int row, Bit2_T bitarray, int p1, void *reader) {
        assert (reader != NULL);
        assert (bitarray != NULL);
        
        Pnmrdr_T *new_Reader_p = (Pnmrdr_T *)reader;
        Pnmrdr_T new_Reader = *new_Reader_p;
        (void) p1;

        int curr_bit = Pnmrdr_get(new_Reader);
        /* put current bit read into our bit array */
        Bit2_put(bitarray, col, row, curr_bit);  
}


/************************** store_pbm **************************
*
* Reads a portable bitmap (PBM) image from a Pnmrdr_T object, stores it in a 
* Bit2_T (2D bit array), and returns the resulting bit array.
* 
* Parameters:
*        newReader: The Pnmrdr_T object representing the PBM image to be read.
* Return: 
*       A Bit2_T object containing the original PBM image data
*
* Expects:
*       Valid reader
*
*                        
*****************************************************************/
Bit2_T store_pbm(Pnmrdr_T newReader) {
        assert (newReader != NULL);
        Pnmrdr_mapdata newMapData = Pnmrdr_data(newReader);
        /* get col and row */
        int col = newMapData.width;
        int row = newMapData.height;
        Bit2_T bit_array = Bit2_new (col, row);

        /* store in the bit array */
        Bit2_map_row_major(bit_array, get_bit, &newReader);
        return bit_array;
}


/************************** run **************************
*
* This function runs unblack edges. It creates a new pnmrdr reader, 
* allocates memory for the needed data structures needed by unblack edges,
* runs unblack edges, and frees all allocated memory. It prints out a 
* unblackened map.
* 
* Parameters:
*       FILE *fp file pointer of picture to be unblackened
*
* Return: 
*       Void
*
* Expects:
*       Valid file pointer
*                        
*****************************************************************/
void run (FILE *fp) {
        assert(fp != NULL);
        Pnmrdr_T newReader = Pnmrdr_new(fp);
        assert (newReader != NULL);
        /* verify pgm is correctly formatted */
        Pnmrdr_mapdata newMapData = Pnmrdr_data(newReader);
        
        assert(newMapData.type == 1);
        assert(newMapData.height != 0);
        assert(newMapData.width != 0);

        Seq_T stack = Seq_new(1000);
        
        
        Bit2_T filled_array = store_pbm(newReader); /*obtain populated array */
        assert (filled_array != NULL);
        assert(stack != NULL);
        add_edges(stack, filled_array); /*add edges to stack*/
        
        assert (filled_array != NULL);
        assert(stack != NULL);
        process_bit(filled_array, stack); /*clean up the black edges*/

        /* print the cleaned up array bit map */
        printf("P%d\n", newMapData.type);
        printf("%d %d\n", newMapData.width, newMapData.height);
        for (int i = 0; i < (int) newMapData.height; i++) {
                for (int j = 0; j < (int) newMapData.width; j++) {
                        int element = Bit2_get(filled_array, j, i);
                        printf("%d ", element);
                }
        printf("\n");
        }
        
        Bit2_free(&filled_array);
        Seq_free(&stack); 
        Pnmrdr_free(&newReader); 
}


/************************** main **************************
*
This function verifies that the correct args are passed, and handles whether
* the file pointer should read from stdin or a given file. Closes file pointer.
* 
* Parameters:
*       User can either input a file, or run the program and then input 
*       contents to stdin*
* Return: 
*       Void
*
* Expects:
*       Valid file pointer.
*
*                        
*****************************************************************/
int main(int argc, char *argv[]) {
        FILE *fp;
        
        assert(argc <= 2);

        if (argc == 2) {
                fp = fopen(argv[1], "r");
                assert(fp != NULL);
        } else if (argc == 1) {
                fp = stdin;
        }
        run(fp);
        
        fclose(fp);
}