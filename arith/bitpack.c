/************************** bitpack.c ***************************
 *                          
 *     Authors: Javier Laveaga {jlavea01} and Prince Millidah {pmilli03}, 
 *     Date: 3/7/2024
 *     Assignment: arith
 *
 *     Summary: This file implements the bit pack interface which is 
 *     responsible for manipulating, getting, and storing unsigned and 
 *     signed integers.
 *
 ***************************************************************/

#include "bitpack.h"
#include <stdio.h>
#include "assert.h"
#include "except.h" 

/* Expection message */
Except_T Bitpack_Overflow = { "Overflow packing bits" };
const unsigned MAX_WIDTH = 64;

/* Helper function to help with shifting */
uint64_t Bitpack_shift_leftu (uint64_t val, unsigned shift);
uint64_t Bitpack_shift_rightu (uint64_t val, unsigned shift);

/******************** Bitpack_fitsu *******************
 * Description:
 *    Detremines if some input number of bits ca be used to represent
 *    the input n as an unsigned value
 * 
 * Parameters:
 *   uint64_t n:         the value to be repressented in bits 
 *   unsigned width:     the number of bit to be used
 * 
 * Returns:
 *      True if the number of bits(width) can be used to represent n,
 *      otherwise False.
 * 
 * Expects:
 *      The width to be less or equal to 64 (MAX_WIDTH)
 *
 * Notes:
 *      Will CRE if the input width is greater than 64.
 * 
 *************************************************************/
bool Bitpack_fitsu(uint64_t n, unsigned width) 
{
        assert(width <= MAX_WIDTH); /* Checking if the width is less than 64 */

        /* Return true because all unsigned can be represented with 64 bits */
        if (width == MAX_WIDTH) {
                return true;
        }
        
        if (width == 0) {
                return false;
        }

        /*Getting the max number that can be represented with the input width*/
        uint64_t num_bits = Bitpack_shift_leftu(1, width);
        uint64_t max_val = num_bits - 1;

        if (n <= max_val) {
                return true;
        }
        return false;
}


/******************** Bitpack_fitss *******************
 * Description:
 *    Determines if some input number of bits ca be used to represent
 *    the input n as an unsigned value
 * 
 * Parameters:
 *   int64_t n:         the value to be repressented in bits 
 *   unsigned width:     the number of bit to be used
 * 
 * Returns:
 *      True if the number of bits(width) can be used to represent n,
 *      otherwise False.
 * 
 * Expects:
 *      The width to be less or equal to 64 (MAX_WIDTH)
 *
 * Notes:
 *      Will CRE if the input width is greater than 64.
 * 
 *************************************************************/
bool Bitpack_fitss(int64_t n, unsigned width) 
{
        /* Checking if the input width id greater than one */
        if (width <= 1) {
                if (n == 0 && width == 1) {
                        return true;
                }
                return false;    
        }
        /* Check if width is less or equal to 64 */
        assert(width <= MAX_WIDTH);

        /* getting the max positive number */
        int64_t max_bit = (int64_t) Bitpack_shift_leftu(1, width - 1);
        return (n < max_bit && n >= -max_bit);
}


/******************** Bitpack_getu *******************
 * Description:
 *    Gets an unsigned integer of the specified width from the given word 
 *    at the given lsb 
 * 
 * Parameters:
 *   uint64_t word:         the word from which we are reading
 *   unsigned width:     the number of bits to be extracted
 *   unsigned lsb:      the least significant bit in the desired field
 * 
 * Returns:
 *      A 64 bit unsigned integer which contains the desired extracted value
 * 
 * Expects:
 *      The width and the lsb plus the width to be less that 64 
 *
 * Notes:
 *      Will CRE if the if width or width + lsb is less than 64
 * 
 *************************************************************/
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        /* Checking if the input width and lsb doesn't exceeding 64 */
        assert(width <= MAX_WIDTH);
        assert(lsb + width <= MAX_WIDTH);

        /* Making mask to change unwanted bits to zero */ 
        uint64_t mask = 0;
        mask = ~mask; 
        mask = Bitpack_shift_rightu(mask, lsb); 
        mask = Bitpack_shift_leftu(mask, lsb);
        
        /* Postioning the ones in the mask at the wanted bit location */
        unsigned shift = 64 - (lsb + width);
        mask = Bitpack_shift_leftu(mask, shift);
        mask = Bitpack_shift_rightu(mask, shift);
        
        /* Getting the result */
        uint64_t result = mask & word; 
        result = Bitpack_shift_rightu(result, lsb); 

        return (result);
}

/******************** Bitpack_gets *******************
 * Description:
 *    Gets a signed integer of the specified width from the given word 
 *    at the given lsb. Calls the Bitpack_getu.
 * 
 * Parameters:
 *   uint64_t word:         the word from which we are reading
 *   unsigned width:     the number of bits to be extracted
 *   unsigned lsb:      the least significant bit in the desired field
 * 
 * Returns:
 *      A 64 bit signed integer which contains the desired extracted value
 * 
 * Expects:
 *      The width and the lsb plus the width to be less that 64 
 *
 * Notes:
 *      Will CRE if the if width or width + lsb is less than 64
 * 
 *************************************************************/
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb) 
{
        /* Getting result using previously implemented function */
        int64_t result = Bitpack_getu(word, width, lsb);

        /* Getting the sign of the output in result */
        uint64_t sign = Bitpack_getu(result, 1, width - 1);

        /* Changing result to a negative when the sign is 1 */
        if (sign == 1) {
                
                /* Creatign mask to change number to negative */
                int64_t mask = ~0; 
                mask = Bitpack_shift_rightu(mask, width); 
                mask = Bitpack_shift_leftu(mask, width);
                
                result = result | mask;
        }
        return result;
}

/*returns a new word (doesnt mess with original)

replaces a field/changes field
new word is identical to original word (except that width of field is different)

*/

/******************** Bitpack_newu *******************
 * Description:
 *    Inserts the specified unsigned value into the word
 * 
 * Parameters:
 *   uint64_t word:         the word we are writing to 
 *   unsigned width:     the number of bits to be used for the value
 *   unsigned lsb:      the least significant bit in the desired field
 *   uint64_t value:   the value to be stored in the word
 * 
 * Returns:
 *      A 64 bit unsigned integer which is a new word with the new value
 * 
 * Expects:
 *      The width and the lsb plus the width to be less that 64 
 *
 * Notes:
 *      Will CRE if the if width or width + lsb is less than 64
 * 
 *************************************************************/
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                        uint64_t value) 
{
        /* Checking if the input width and lsb doesn't exceeding 64 */
        assert(width <= MAX_WIDTH);
        assert(lsb + width <= MAX_WIDTH);
        
        /* checking if the value can be represented in the word  */
        bool possible = Bitpack_fitsu(value, width);
        if (possible == false) {
                RAISE(Bitpack_Overflow);
        }
        
        /* Creating mask to modify the word */
        uint64_t mask = ~0;
        mask = Bitpack_shift_rightu(mask, (lsb + width));
        mask = Bitpack_shift_leftu(mask, (lsb + width)); 
        
        /* Shifting the mask to the make all wanted area 0s*/
        uint64_t shift = ~0;
        shift = Bitpack_shift_rightu(shift, (64 - lsb)); 
        mask = mask | shift;
       
        /* AMking the area to put the word all zeros */
        word = word & mask;
       
        /* Shifting value to the wanted region */
        value = Bitpack_shift_leftu(value, lsb); 

        /*putting value into word and returning*/
        return (word | value);
}

/******************** Bitpack_news *******************
 * Description:
 *    Inserts the specified signed value into the word. Calls the 
 *    Bitpack_newu function.
 * 
 * Parameters:
 *   uint64_t word:         the word we are writing to 
 *   unsigned width:     the number of bits to be used for the value
 *   unsigned lsb:      the least significant bit in the desired field
 *   int64_t value:   the value to be stored in the word
 * 
 * Returns:
 *      A 64 bit unsigned integer which is a new word with the new value
 * 
 * Expects:
 *      The width and the lsb plus the width to be less that 64 
 *
 * Notes:
 *      Will CRE if the if width or width + lsb is less than 64
 *      Raises Bitpack overful if value does not fit in width
 * 
 *************************************************************/
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,  
                            int64_t value) 
{
        /* Checking if the input width and lsb doesn't exceeding 64 */
        assert(width <= MAX_WIDTH); 
        assert(lsb + width <= MAX_WIDTH);
        
        /*checking if the value can be represented in the word  */
        bool possible = Bitpack_fitss(value, width);
        if (possible == false) {
                RAISE(Bitpack_Overflow);
        }

        /* Creating mask to making the value non negative */
        uint64_t mask = ~0;
        mask = Bitpack_shift_leftu(mask, (64 - width));
        mask = Bitpack_shift_rightu(mask, (64 - width));

        /*making a newValue which is unsigned*/
        uint64_t newValue = mask & value; 

        return Bitpack_newu(word, width, lsb, newValue);
}

/******************** Bitpack_shift_leftu *******************
 * Description:
 *    Shifts the spacified value to the left by the amount in shift
 * 
 * Parameters:
 *   uint64_t val:         the value to shift
 *   unsigned shift      the shift amount
 * 
 * Returns:
 *      A 64 bit unsigned integer which is the shifted value
 * 
 * Expects:
 *      The shift to be less than or equal to 64
 *
 * Notes:
 *     Will CRE if the shift amount is greater than 64
 * 
 *************************************************************/
uint64_t Bitpack_shift_leftu(uint64_t val, unsigned shift ) 
{
        /* Checking that width doesn't exceed 64 */
        assert(shift <= MAX_WIDTH);
        
        /*make sure to return 0 if shift is equal to 64 */
        if (shift == MAX_WIDTH) {
                return 0;
        }

        /* Shifting the value to the left */
        uint64_t result = val << shift;
        return result;
}

/******************** Bitpack_shift_rightu *******************
 * Description:
 *    Shifts the spacified value to the right by the shift amount
 * 
 * Parameters:
 *   uint64_t val:         the value to shift
 *   unsigned shift      the shift amount
 * 
 * Returns:
 *      A 64 bit unsigned integer which is the shifted value
 * 
 * Expects:
 *      The shift to be less than or equal to 64
 *
 * Notes:
 *     Will CRE if the shift amount is greater than 64
 * 
 *************************************************************/
uint64_t Bitpack_shift_rightu(uint64_t val, unsigned shift ) 
{
        /* Checking that width doesn't exceed 64 */
        assert(shift <= MAX_WIDTH);
        
        /*make sure to return 0 if shift is equal to 64 */
        if (shift == MAX_WIDTH) {
                return 0;
        }
        /* Shifting the value to the right */
        uint64_t result = val >> shift;
        return result;
}
