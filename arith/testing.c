/************************** testing.c ***************************
 *                          
 *     Authors: Javier Laveaga {jlavea01} and Prince Millidah {pmilli03}, 
 *     Date: 3/7/2024
 *     Assignment: arith
 *
 *     Summary: This file tests our bitpack interface.
 *
 ***************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include "compress40.h"
#include <seq.h>
#include "uarray2b.h"
#include "bitpack.h"
#include "compressor.h"


int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

        /*testing fitss*/
        bool bit_ss = Bitpack_fitss(-4, 3); 
        assert (bit_ss == true);
        bit_ss = Bitpack_fitss(-5, 3); 
        assert (bit_ss == false);
        bit_ss = Bitpack_fitss(3, 3); 
        assert (bit_ss == true);
        bit_ss = Bitpack_fitss(-1, 1); 
        assert (bit_ss == false);
        bit_ss = Bitpack_fitss(0, 1); 
        assert (bit_ss == true);
        bit_ss = Bitpack_fitss(-1, 1); 
        assert (bit_ss == false);
        
        
        /*testing fitsu*/
        bool bit_us = Bitpack_fitsu(8, 64);
        assert(bit_us == true);
        bit_us = Bitpack_fitsu(-1, 64);
        assert(bit_us == true);
        bit_us = Bitpack_fitsu(4, 3);
        assert(bit_us == true);
        bit_us = Bitpack_fitsu(8, 3);
        assert(bit_us == false);
        bit_us = Bitpack_fitsu(7, 3);
        assert(bit_us == true);
        bit_us = Bitpack_fitsu(0, 0);
        assert(bit_us == false);


        /*width then LSB*/
        uint64_t ans = Bitpack_getu(0x3f4, 6, 2);
        assert(ans == 61);
        ans = Bitpack_getu(0x12345678, 8, 4);
        /* knows if decimal of HEX transfers both to binary and compares*/
        assert(ans == 0x67); /*103 in decimal*/

        ans = Bitpack_getu(0x3f4, 64, 0); /*widht  + lsb is 64*/

        int64_t sign;
        sign = Bitpack_gets(0x3f4, 6, 2);
        assert(sign == -3);
        sign = Bitpack_gets(0x3f4, 3, 2);
        assert(sign == -3);
        sign = Bitpack_gets(0x5, 2 , 2); /*testing when sign is 0*/
        assert(sign == 1);
        sign = Bitpack_gets(0xf, 4 , 0);
        assert(sign == -1);
        sign = Bitpack_gets(0xf, 4 , 0);
        assert(sign == -1);

        uint64_t new_word = Bitpack_newu(0x3f4, 6, 2, 47); /*val is */
        bool my_b = (Bitpack_getu(new_word, 6, 2) == 47);

        new_word = Bitpack_newu(0x3f4, 4, 12, 14); /*val14, lsb =12, width=4*/
        assert(new_word == 0xe3f4);

        /*check lsb + width <= 64*/
        /*new_word = Bitpack_newu(0x3f4, 2, 63, 1); */ /*will throw assertion*/
        /*new_word = Bitpack_newu(0x3f4, 1, 63, 2);*/ /*test Bitpack_fitsu*/
        new_word = Bitpack_newu(0x12345678, 40, 10, 54);
        my_b = (Bitpack_getu(new_word, 40, 10) == 54);
        assert(my_b);

        bool possible_u = Bitpack_fitsu(-5, 20);
        assert(possible_u == false); /*-5 is interpreted as very large num*/
        bool possible_s = Bitpack_fitss(-5, 20);
        assert (possible_s == true);

        /*47 has width of 6 in binary*/
        new_word = Bitpack_news(0x3f4, 60, 0, -1); 
        my_b = (Bitpack_gets(new_word, 60, 0) == -1);
        assert(my_b == true);

        /*TODO: ask about wheater -1 would take 64 bites to store*/
        new_word = Bitpack_news(0x3f4, 6, 2, 4); /* width=6, lsb=2 */
        my_b = (Bitpack_gets(new_word, 6, 2) == 4);

        new_word = Bitpack_news(0x3f4, 10, 2, -47); /* width=2, lsb=2 */
        my_b = (Bitpack_gets(new_word, 10, 2) == -47);
        assert(my_b);

        float my_float = roundFloats(-0.7,-0.5, 0.5);
        assert(my_float == -0.5);

        my_float = roundFloats(0.7,-0.5, 0.5);
        assert(my_float == 0.5);

        my_float = roundFloats(0.3, -0.5, 0.5);
        assert((my_float - 0.3) < 0.0000001);

        int b_q = quantizeFloats(-0.25);
        (void) b_q;

        return EXIT_SUCCESS; 
}


