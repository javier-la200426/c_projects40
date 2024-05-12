/************************** program_structs.h ***********************
 *                          
 *     Authors: Javier Laveaga {jlavea01} and Prince Millidah {pmilli03}, 
 *     Date: 2/27/2024
 *     Assignment: arith
 *
 *     Summary: This file defines our structures for both compressor and 
 *     decompressor.
 *
 ***************************************************************/


#ifndef PROGRAM_STRUCTS
#define PROGRAM_STRUCTS

#include <seq.h>

#define NUM_BYTE 4
#define BLOCK_ELEM 4
#define BLOCK_SIZE 2
#define BCD_WIDTH 6
#define PS_WIDTH 4
#define A_WIDTH 6
#define PB_LSB 4
#define PR_LSB 0
#define A_LSB 26
#define B_LSB 20
#define C_LSB 14
#define D_LSB 8
#define PB_LSB 4
#define PS_LSB 0
#define BCD_MAX 103.33
#define A_MAX 63
#define BYTE 8
#define PS_WIDTH 4
#define RGB_MAX 255
#define BCD_RANGE 0.3
#define PS_RANGE 0.5

/************************** P_and_Y_floats struct **************************
*
* holds the Pb, Pr, and Y floats (for an individual pixel) This is component
* video color.
* 
* Parameters:
*    
*        float Pb: the Pb value for a pixel
*        float Pr: the Pr value for a pixel
*        float Ys: the float Y which represents the brightness
*                         
*****************************************************************/
struct P_and_Y_floats {
    float Pb, Pr, Y;
};

/************************** P_avg_abcd struct **************************
*
* holds the Pb_avg, Pr_avg, and a,b,c,d floats
* 
* Parameters:
*    
*        float Pb_avg: the average of the Pb's
*        float Pr_avg: the average of the Pr's
*        float a: one of the cosine coefficients
*        float b: a cosine coefficient
*        float c: a cosine coefficient
*        float d: a cosine coefficient
*                         
*****************************************************************/
struct P_avg_abcd {
    float Pb_avg, Pr_avg, a, b, c, d;
};

/************************** compClosure struct **************************
*
* holds the information necessary for the closure in the compressor. It is
* used by the apply function to_avg_and_abcd to populate the P_avg_abcd 
* members.
* 
* Parameters:
*    struct P_avg_abcd *block_vals:      the struct holding the block_vals
*    Seq_T blocks:     The sequence which stores the         
*    int counter:
*
*        
*****************************************************************/
struct compClosure {
    struct P_avg_abcd *block_vals;
    Seq_T blocks;
    int counter;
};

/************************** decompClosure struct **************************
*
* holds the information necessary for the closure in the decompressor. It is
* used by the apply function getfloatValues to get the correct indices and 
* sequence.
* 
* Parameters:
*    Seq_T sequence:      the sequence which holds the of P_avg_abcd structs
*    int index:           the index of the sequence
*    int counter:         the counter which keeps track of the Y values
*
*        
*****************************************************************/
struct decompClosure {
    Seq_T sequence;
    int index; /*index for sequence*/
    int blockIndex; /*for get the struct position in each block */
};

#endif