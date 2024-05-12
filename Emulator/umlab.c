/*
 * Authors: Eva Caro (ecaro01) & Javier Laveaga (jlavea01)
 * Date: March 29, 2024
 * umlab.c
 *
 * Functions to generate UM unit tests. Once complete, this module
 * should be augmented and then linked against umlabwrite.c to produce
 * a unit test writing program.
 *  
 * A unit test is a stream of UM instructions, represented as a Hanson
 * Seq_T of 32-bit words adhering to the UM's instruction format.  
 * 
 * Any additional functions and unit tests written for the lab go
 * here. 
 *  
 */


#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <seq.h>
#include <bitpack.h>


typedef uint32_t Um_instruction;
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;


/* Functions that return the two instruction types */

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc);
Um_instruction loadval(unsigned ra, unsigned val);

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc)
{
        Um_instruction word = 0;
        word = Bitpack_newu(word, 3, 0, rc);
        word = Bitpack_newu(word, 3, 3, rb);
        word = Bitpack_newu(word, 3, 6, ra);
        word = Bitpack_newu(word, 4, 28, op);
        return (Um_instruction) word;
}

Um_instruction loadval(unsigned ra, unsigned val)
{
        Um_instruction word = 0;
        word = Bitpack_newu(word, 4, 28, 13); /*load command*/
        word = Bitpack_newu(word, 3, 25, ra); /*register*/
        word = Bitpack_newu(word, 25, 0, val); 
        return (Um_instruction) word;
}

/* Wrapper functions for each of the instructions */

static inline Um_instruction halt(void) 
{
        return three_register(HALT, 0, 0, 0);
}

typedef enum Um_register { r0 = 0, r1, r2, r3, r4, r5, r6, r7 } Um_register;

static inline Um_instruction cmov(Um_register a, Um_register b, Um_register c) 
{
        return three_register(CMOV, a, b, c);
}

static inline Um_instruction seg_load(Um_register a, Um_register b, 
                                      Um_register c) 
{
        return three_register(SLOAD, a, b, c);
}

static inline Um_instruction seg_store(Um_register a, Um_register b,
                                      Um_register c) 
{
        return three_register(SSTORE, a, b, c);
}


static inline Um_instruction add(Um_register a, Um_register b, Um_register c) 
{
        return three_register(ADD, a, b, c);
}

static inline Um_instruction mult(Um_register a, Um_register b, Um_register c) 
{
        return three_register(MUL, a, b, c);
}

static inline Um_instruction div(Um_register a, Um_register b, Um_register c) 
{
        return three_register(DIV, a, b, c);
}

static inline Um_instruction nand(Um_register a, Um_register b, Um_register c) 
{
        return three_register(NAND, a, b, c);
}

static inline Um_instruction map_seg(Um_register b, Um_register c) 
{
        return three_register(ACTIVATE, 0, b, c);
}

static inline Um_instruction unmap_seg(Um_register c) 
{
        return three_register(INACTIVATE, 0, 0, c);
}

static inline Um_instruction load_prog(Um_register b, Um_register c) 
{
        return three_register(LOADP, 0, b, c);
}





Um_instruction output(Um_register c);

Um_instruction output(Um_register c)
{
        return three_register(OUT, 0, 0, c);
}
Um_instruction input(Um_register c)
{
        return three_register(IN, 0, 0, c);
}

/* Functions for working with streams */

static inline void append(Seq_T stream, Um_instruction inst)
{
        assert(sizeof(inst) <= sizeof(uintptr_t));
        Seq_addhi(stream, (void *)(uintptr_t)inst); /*pointer  to pointer*/
}

const uint32_t Um_word_width = 32;

void Um_write_sequence(FILE *output, Seq_T stream)
{
        assert(output != NULL && stream != NULL);
        int stream_length = Seq_length(stream);
        for (int i = 0; i < stream_length; i++) {
                Um_instruction inst = (uintptr_t)Seq_remlo(stream);
                for (int lsb = Um_word_width - 8; lsb >= 0; lsb -= 8) {
                        fputc(Bitpack_getu(inst, 8, lsb), output);
                }
        }
      
}


/* Unit tests for the UM */

void build_halt_test(Seq_T stream)
{
        append(stream, halt());
}

void build_verbose_halt_test(Seq_T stream)
{
        append(stream, loadval(r1, 'B'));
        append(stream, output(r1));
        append(stream, halt());
        append(stream, loadval(r1, 'a'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'd'));
        append(stream, output(r1));
        append(stream, loadval(r1, '!'));
        append(stream, output(r1));
        append(stream, loadval(r1, '\n'));
        append(stream, output(r1));
}



void build_add_test(Seq_T stream)
{
        append(stream, loadval(r1, 48));
        append(stream, loadval(r2, 47));
        append(stream, add(r3, r1, r2));
        append(stream, output(r1)); /*0*/
        append(stream, output(r2)); /*/*/
        append(stream, output(r3)); /*_*/
        append(stream, halt());
}

void build_multiply_test(Seq_T stream)
{
        append(stream, loadval(r1, 5));
        append(stream, loadval(r2, 6));
        append(stream, mult(r3, r1, r2));
        append(stream, halt());
        
}

void build_division_test(Seq_T stream)
{
        append(stream, loadval(r7, 6));
        append(stream, loadval(r6, 5));
        append(stream, div(r5, r7, r6));
        append(stream, halt());
        
}

void build_nand_test(Seq_T stream)
{
        append(stream, loadval(r1, 65536));
        append(stream, loadval(r2, 65535));
        append(stream, loadval(r6, 65525));
        append(stream, mult(r0,r1,r2));
        append(stream, add(r3, r0, r2));
        append(stream, add(r7, r0, r6));
        /* r3 holds 2^32 - 1 = 65536 * 65535 + 65535 = 4294967295 */
        /* r7 holds 4294967285 */
        append(stream, nand(r5, r3, r7));
        append(stream, halt());
        
}

void small_nand_test(Seq_T stream)
{
        append(stream, loadval(r1, 66));
        append(stream, loadval(r2, 66));
        /* r3 holds 2^32 - 1 = 65536 * 65535 + 65535 = 4294967295 */
        /* r7 holds 4294967285 */
        append(stream, nand(r0, r1, r2)); 
        append(stream, nand(r5, r1, r2));
        /*r0 and r5 are the same nand value*/
        append(stream, nand(r6, r0, r5));
        append(stream, output(r6));
        append(stream, halt());
        
}

void build_print_test(Seq_T stream)
{
        append(stream, loadval(r1, 48));
        append(stream, loadval(r2, 6));
        append(stream, add(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

void test_load_val (Seq_T stream)
{
        append(stream, loadval(r0, 87));
        append(stream, loadval(r1, 94));
        append(stream, loadval(r2, 47));
        append(stream, loadval(r3, 51));
        append(stream, loadval(r4, 98));
        append(stream, loadval(r5, 64));
        append(stream, loadval(r6, 65));
        append(stream, loadval(r7, 43));
        append(stream, output(r0)); /*W*/
        append(stream, output(r1));/*^*/
        append(stream, output(r2)); /*/*/
        append(stream, output(r3)); /*3*/
        append(stream, output(r4)); /*b*/
        append(stream, output(r5)); /*@*/
        append(stream, output(r6)); /*A*/
        append(stream, output(r7)); /*+*/
        append(stream, halt());
        /*register 1 should now be */

}

void build_conditional_test(Seq_T stream)
{
        /*take A = r1, B = r2, C = r3*/
        append(stream, loadval(r1, 23)); 
        append(stream, loadval(r2, 101));
        append(stream, loadval(r3, 113));
        /*r1 is supposed to be 101 now e */
        append(stream, cmov(r1,r2,r3));
        append(stream, output(r1));
        append(stream, halt());
        
}

void build_cond_test_0(Seq_T stream)
{
        /*take A = r1, B = r2, C = r3*/
        append(stream, loadval(r1, 0));
        append(stream, loadval(r2, 101));
        append(stream, loadval(r3, 1));
        /*r1 is supposed to still be 0 */
        append(stream, cmov(r1,r2,r3));
        append(stream, output(r1));  
        append(stream, output(r2));  
        append(stream, halt());

}

/* cmov, which has a word equal to 83, worked, but loadval resulted in the 
register holding the same word value with three other numbers at the end */
void test_seg_load (Seq_T stream)
{
        append(stream, loadval(r1, 1)); /*A result here*/
        append(stream, loadval(r2, 0)); /*B seg number*/
        append(stream, cmov(r1, r2, r3));
        append(stream, loadval(r3, 3)); /*C offset number*/
        append(stream, seg_load(r1,r2,r3)); 
        append(stream, halt());

}

/*aqui*/
void test_seg_store(Seq_T stream)
{
        /*seg store test*/
        append(stream, loadval(r0, 74));
        append(stream, map_seg(r7, r0));
        append(stream, loadval(r1, 1)); /*segment ID*/
        append(stream, loadval(r2, 3)); /*offset*/
        append(stream, loadval(r3, 65)); /*value*/
        append(stream, seg_store(r1,r2,r3));
        /*seg load test*/
        append(stream, seg_load(r4, r1, r2)); /*r4 should have A*/
        append(stream, output(r4));
        append(stream, halt());

}
/*load program.. */
/* passes ! */
void test_seg_load2(Seq_T stream)
{
        append(stream, loadval(r0, 0)); /*segment number is 0*/
        append(stream, loadval(r1, 1)); 
        append(stream, loadval(r1, 2));
        append(stream, loadval(r1, 3));
        append(stream, loadval(r1, 4));
        append(stream, loadval(r1, 5)); /*offset is 5*/
        append(stream, seg_load(r2, r0, r1)); /*putting above word in r2*/
        append(stream, halt());
}

void test_seg_load3(Seq_T stream)
{
        append(stream, loadval(r0, 0)); /* segment number is 0*/
        append(stream, loadval(r1, 1)); 
        append(stream, loadval(r1, 2));
        append(stream, loadval(r1, 3));
        append(stream, loadval(r1, 4));
        append(stream, loadval(r1, 7)); /*offset is 5*/
        printf("********word: %u\n", loadval(r1, 5));
        append(stream, seg_load(r2, r0, r1)); /*putting above word in r2*/
}

void test_map_segment(Seq_T stream)
{
        append(stream, loadval(r3, 7)); /*7 words*/
        append(stream, map_seg(r2, r3)); /*7 words*/
        append(stream, halt());
        /*r2 shoul have seg id =1*/    

}

void test_unmap_segment(Seq_T stream)
{
        append(stream, loadval(r3, 7));
        append(stream, map_seg(r2,r3));
        append(stream, unmap_seg(r2)); /*7 words*/
        append(stream, halt());
        /*r2 shoul have seg id =1*/    

}


void test_unmap_segment_2(Seq_T stream)
{
        append(stream, loadval(r3, 7));
        append(stream, map_seg(r2,r3));  
        append(stream, loadval(r1, 1)); /*segment ID*/
        append(stream, loadval(r5, 2)); /*offset*/
        append(stream, loadval(r3, 67));
        append(stream, seg_store(r1,r5,r3));

        append(stream, unmap_seg(r2)); /*7 words*/
        append(stream, halt());
        /*should have 67 in index 1,1*/

}

void test_output(Seq_T stream)
{
        append(stream, loadval(r3, 81));
        append(stream, output(r3));
        append(stream, halt());
}

void test_output_bad(Seq_T stream)
{
        append(stream, loadval(r3, 256));
        /*commented out because 256 will throw exception when printing*/
        /*append(stream, output(r3));*/
        append(stream, halt());
}



void test_load_program(Seq_T stream)
{
        /*creating new segment*/
        append(stream, loadval(r3, 2));
        append(stream, map_seg(r2,r3)); /*2 is where id is put r3 is size*/

        /*storing values in new segment*/
        append(stream, loadval(r1, 1)); /*segment ID*/
        append(stream, loadval(r5, 0)); /*offset*/
        append(stream, loadval(r3, 67)); 
        append(stream, loadval(r0, 0)); /*this is counter*/
        append(stream, loadval(r2, 1)); /*this is segment to be duplicated*/
        append(stream, seg_store(r1,r5,r3));
        
        /*Commented out because it will fail as the duplicated segment
        does not contain halt command */
        /*append(stream, load_prog(r2,r0));*/
        append(stream, halt());
}

void test_load_program3(Seq_T stream)
{

        
        append(stream, loadval(r3, 3));
        append(stream, map_seg(r2,r3)); /*2 is where id is put r3 is size*/

        /*storing values in new segment*/
        append(stream, loadval(r0, 0));
        append(stream, loadval(r4, 97));
        append(stream, seg_store(r2, r0, r4));
        append(stream, loadval(r0, 1));
        append(stream, loadval(r4, 98));
        append(stream, seg_store(r2, r0, r4));
        append(stream, loadval(r0, 2));
        append(stream, loadval(r7, 29360128)); /* this*64 is halt val*/
        append(stream, loadval(r4, 64));
        append(stream, mult(r4, r4, r7));
        append(stream, seg_store(r2, r0, r4));
        /* now seg0 holds 97, 98, HALT in that order */
        append(stream, loadval(r5, 0)); /*using to iterate thorugh segment*/
        append(stream, seg_load(r6, r2, r5));
        append(stream, output(r6));
        append(stream, loadval(r5, 1)); /*using to iterate thorugh segment*/
        append(stream, seg_load(r6, r2, r5));
        append(stream, output(r6));
        append(stream, loadval(r5, 2)); /*using to iterate thorugh segment*/
        append(stream, seg_load(r6, r2, r5));

        append(stream, loadval(r0, 1));
        append(stream, loadval(r1, 0));
        append(stream, load_prog(r0, r1));
}

void test_seg_store2(Seq_T stream)
{
        append(stream, loadval(r0, 2));
        append(stream, map_seg(r3,r0));
        append(stream, loadval(r0, 2));
        append(stream, map_seg(r3,r0));
        append(stream, loadval(r0, 2));
        append(stream, map_seg(r3,r0));
        append(stream, loadval(r0, 2));
        append(stream, map_seg(r3,r0));
        append(stream, loadval(r0, 2));
        append(stream, map_seg(r3,r0));
        append(stream, loadval(r0, 2));
        append(stream, map_seg(r3,r0));
        /*so 6 segments*/
        append(stream, loadval(r7, 69)); /*E*/
        append(stream, loadval(r4, 5) ); /*A*/
        append(stream, loadval(r5, 2) ); /*B*/
        append(stream, seg_store(r4,r5,r7));
        append(stream, seg_load(r0,r4,r5));
        append(stream, output(r0)); /*output should be is 100037*/
        append(stream, halt());
}

void test_input_output(Seq_T stream)
{
        append(stream, input(r7));
        append(stream, output(r7));
        append(stream, halt());

}

/*jav*/
void advanced_nand (Seq_T stream)
{
        append(stream, input(r1));
        append(stream, loadval(r2, 0xf));
        append(stream, nand(r0, r1, r2));
        append(stream, nand(r3, r0, r0));
        append(stream, loadval(r4, 48));
        append(stream, add(r5, r4, r3)); /*adding 48 and input after nand*/
        append(stream, output(r5));
        append(stream, halt());
}

void test_time (Seq_T stream)
{
        append(stream, loadval(r0, 3));
        append(stream, loadval(r5, 0));
        for (int i = 0; i < 100000; i++) {
                append(stream, map_seg(r1,r0));
                append(stream, map_seg(r4, r0));
                append(stream, seg_store(r1, r5, r0)); /*storing 3*/
                append(stream, unmap_seg(r1));
                append(stream, unmap_seg(r4));
        }
        append(stream, halt());
}