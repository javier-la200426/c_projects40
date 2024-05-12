/*************************************************************************
 *
 *                     um_helper.c
 *
 *     Assignment: UM
 *     Authors:  Eva Caro (ecaro01) and Javier Laveaga (jlavea01)
 *     Date:     4/11/24
 *
 *     Defines all functions that facilitate the functionality of the UM,
 *     including functions to create, fill, use, and free UM's. 
 *
 *************************************************************************/

#include "um_helper.h"

uint32_t reg[8];
struct segment *segments;



/* secret functions not to be used by client directly */
uint32_t fetch(UM my_um);
bool execute(UM my_um, int op_code, uint32_t word);
void conditional_move(uint32_t A, uint32_t B, uint32_t C);
void segmented_load(uint32_t A, uint32_t B, uint32_t C);
void load_val(uint32_t A, uint32_t val);
void segmented_store(uint32_t A, uint32_t B, uint32_t C);
void addition(uint32_t A, uint32_t B, uint32_t C);
void multiplication(uint32_t A, uint32_t B, uint32_t C);
void division(uint32_t A, uint32_t B, uint32_t C);
void nand(uint32_t A, uint32_t B, uint32_t C);
void map(UM my_um, uint32_t B, uint32_t C);
void unmap(UM my_um, uint32_t C);
void output(uint32_t C);
void input(uint32_t C);
void load_program(UM my_um, uint32_t B, uint32_t C);
void expand(UM my_um);


/********** UM_new ********
 *
 * Returns a new Universal Machine struct with counter and register array 
 * initialized to 0, segments and unmapped sequences initialized but empty.
 *
 * Parameters:
 *      N/A
 *
 * Return: the new UM as a void pointer.
 *
 * Expects
 *      N/A
 * Notes:
 *      Caller is reponsible for deallocating space for the UM.
 ************************/
void *UM_new() 
{
        UM my_um = malloc(sizeof(struct UM));
        my_um->counter = 0;
        my_um->length_seg0 = 0;
        my_um->num_segs = 0;
        my_um->segments_size = 1000;
        segments = malloc(sizeof(struct segment) * 1000);
        for (int i = 0; i < 1000; i++) {
                segments[i].length = 0; // NULL for initialization?
                segments[i].seg = NULL; // NULL   
        }
        my_um->unmapped = Seq_new(100); 
        for (int i = 0; i < 8; i++) {
                reg[i] = 0;
        }
        return my_um;
}

/********** UM_free ********
 *
 * Frees the space for a given UM, including all member sequences and their 
 * elements.
 *
 * Parameters:
 *      A UM to delete.
 *
 * Return: N/A
 *
 * Expects
 *      Given UM is not null.
 * 
 * Notes:
 *      N/A
 ************************/
void UM_free(UM my_um)
{
        long num_segs = my_um->num_segs;
        for (int i = 0; i < num_segs; i++) {
                struct segment seg_str = segments[i];
                uint32_t *seg = seg_str.seg;
                if (seg != NULL) {
                        free(seg);
                }
        }
        Seq_free(&(my_um->unmapped));
        free(segments);
        free(my_um);

}

/********** read_file ********
 *
 * Reads in the contents of a file, packing every four bytes into a uint32_t
 * instructions word and storing these as separate elements in the first 
 * sequence in the segments sequence of a given UM.
 *
 * Parameters:
 *      UM my_um:         the UM to store the file in.
 *      FILE *infile:     the file pointer to the file to read from.
 *      long total_bytes: the total amount of bytes in the file (four bytes in
 *                        every word).
 *
 * Return: the UM containing the instrucitons in segment[0[ as a void pointer.
 *
 * Expects
 *      The UM is not null, the file pointer is not null and is opened to a
 *      file containing instruction words, and total_bytes is divisible by 4.
 * 
 * Notes:
 *      Caller is reponsible for deallocating the new segment Sequence
 ************************/
void *read_file(UM my_um, FILE *infile, long total_bytes)
{
        int num_words = total_bytes / WRDLNGTH;
        uint32_t *seg_0 = malloc(sizeof(uint32_t) * num_words);
        struct segment seg_0_str;
        seg_0_str.seg = seg_0;
        seg_0_str.length = num_words;
        uint32_t byte;
        uint32_t word;
        my_um->length_seg0 = num_words;
        /* add each word to segment, then add segment to the segments seq_t */
        for (int j = 0; j < num_words; j++) {
                word = 0;
                for (int i = 1; i <= 4; i++) { /* for each word */
                        byte = fgetc(infile);
                        byte = byte << (32 - (i * 8));
                        word = word | byte;
                }
                seg_0[j] = word;
        }
        segments[0] = seg_0_str;
        my_um->num_segs++;
        return my_um;
}

/********** fetch ********
 *
 * Returns the next instruction word from segment[0] (indexed by UM->counter).
 *
 * Parameters:
 *      UM my_um:         the UM to read from
 *
 * Return: the instruction word
 *
 * Expects
 *      counter points to a valid index in the segment[0] sequence and my_um
 *      is not null.
 * 
 * Notes:
 *      N/A
 ************************/
uint32_t fetch(UM my_um)
{
        struct segment seg_0_str = segments[0];
        uint32_t *seg_0 = seg_0_str.seg;
        uint32_t word = seg_0[my_um->counter];
        my_um->counter = my_um->counter + 1;
        return word;
}

/********** run ********
 *
 * Fetches, decodes, and executes every instruction in segment[0] until the 
 * halt instruction is reached or there are no more instructions to be read.
 *
 * Parameters:
 *      UM my_um:         the UM 
 *
 * Return: N/A
 *
 * Expects
 *      my_um is not null, program counter starts at 0, segment[0] contains at
 *      least one instruction.
 * 
 * Notes:
 *      Calls fetch, get_registers, execute, and sequence-specific functions
 *      that might include assert statements
 ************************/
void run(UM my_um)
{
        bool run = true;
        bool halt = false;
        while (run) { 
                uint32_t word = fetch(my_um);  
                uint32_t mask = 0xF0000000; /* only the first 4 bits */
                int op_code = (mask & word) >> 28;
               
                halt = execute(my_um, op_code, word);
                if (halt) {
                        break;
                }
                if (my_um->counter >= my_um->length_seg0){
                        run = false;
                }
        }
        assert(halt == true);

}

/********** execute ********
 *
 * Calls the helper function associated with the current instruction
 *
 * Parameters:
 *      UM my_um:         the UM to read from
 *      int *info:        a heap-allocated array with values extracted from
 *                        the codeword (denoting relevant register numbers 
 *                        and/or values).
 *      int op_code:      an int from 0 to 13 corresponding to an instruction
 *
 * Return: true if the HALT instruction is encountered, false otherwise.
 *
 * Expects
 *      info contains valid values, op_code is a number from 0 to 13, my_um and
 *      is not null.
 * 
 * Notes:
 *      N/A
 ************************/
bool execute(UM my_um, int op_code, uint32_t word)
{
        bool halt = false;

        uint32_t A = 0;
        uint32_t B = 0;
        uint32_t C = 0;
        uint32_t val = 0;
        uint32_t A_mask = 0;
        uint32_t B_mask = 0;
        uint32_t C_mask = 0;
        uint32_t val_mask = 0;

        if (op_code == 13) {

                A_mask = 234881024;
                A = (A_mask & word) >> 25;
                val_mask = 33554431;
                val = val_mask & word;

        } else {

                A_mask = 448;
                A = (A_mask & word) >> 6;
                B_mask = 56;
                B = (B_mask & word) >> 3;
                C_mask = 7;
                C = C_mask & word;

        }
    
        switch (op_code) {
                case 0:
                        //printf("cmov\n");
                        conditional_move(A, B, C);
                        break;
                case 1:
                        // //("seg load\n");
                        segmented_load(A, B, C);
                        break;
                case 2:
                        // printf("seg store\n");
                        segmented_store(A, B, C);
                        break;
                case 3:
                        // printf("add\n");
                        addition(A, B, C);
                        break;
                case 4:
                        //printf("mult\n");
                        multiplication(A, B, C);
                        break;
                case 5:
                        //printf("nand\n");
                        division(A, B, C);
                        break;
                case 6:
                        //printf("nand\n");
                        nand(A, B, C);
                        break;
                case 7:
                        // printf("halt\n");
                        halt = true;
                        break;
                case 8: 
                        // printf("map\n");
                        map(my_um, B, C);
                        break;
                case 9:
                        // printf("unmap\n");
                        unmap(my_um, C);
                        break;
                case 10:
                        // printf("output\n");
                        output(C);
                        break;
                case 11:
                        // printf("input\n");
                        input(C);
                        break;
                case 12:
                        // printf("load_program\n");
                        load_program(my_um, B, C);
                        break;
                case 13:
                        // printf("load_val\n");
                        load_val(A, val);
                        break;
        }

        return halt;
}

/********** load_val ********
 *
 * Loads a given value in a given register in my_um.
 *
 * Parameters:
 *      UM my_um:         the UM being operated on
 *      int *info:        an int array holding the A, B, C, and/or value 
 *
 * Return: N/A
 *
 * Expects
 *      values in info array are valid, info is not null, and caller is 
 *      manipulating the UM in legal ways (that won't cause endless loops or
 *      other runtime issues)
 * 
 * Notes:
 *      N/A
 ************************/
void load_val(uint32_t A, uint32_t val) 
{
        reg[A] = val;
}

/********** conditional_move ********
 *
 * Replaces the value in register A with that of register B if register C is
 * not 0.
 *
 * Parameters:
 *      UM my_um:         the UM being operated on
 *      int *info:        an int array holding the A, B, C, and/or value 
 *
 * Return: N/A
 *
 * Expects
 *      values in info array are valid, info is not null, and caller is 
 *      manipulating the UM in legal ways (that won't cause endless loops or
 *      other runtime issues)
 * 
 * Notes:
 *      N/A
 ************************/
void conditional_move(uint32_t A, uint32_t B, uint32_t C)
{
        if (reg[C] != 0) {
                reg[A] = reg[B];
        }
}

/********** segmented_load ********
 *
 * Puts the instruction word at index register[C] in segment number register[B]
 * inside of register A.
 *
 * Parameters:
 *      UM my_um:         the UM being operated on
 *      int *info:        an int array holding the A, B, C, and/or value 
 *
 * Return: N/A
 *
 * Expects
 *      values in info array are valid, info is not null, caller is 
 *      manipulating the UM in legal ways (that won't cause endless loops or
 *      other runtime issues), and values inside registers B and C are not out
 *      of bounds (for number of segments and number of words in the key 
 *      segment, respectively)
 * 
 * Notes:
 *      N/A
 ************************/
void segmented_load(uint32_t A, uint32_t B, uint32_t C)
{
        struct segment seg_str = segments[reg[B]];
        uint32_t *seg = seg_str.seg;
        reg[A] = seg[reg[C]];
}

/********** segmented_store ********
 *
 * Stores the register C value in segment[register A] at index register B
 *
 * Parameters:
 *      UM my_um:         the UM being operated on
 *      int *info:        an int array holding the A, B, C, and/or value 
 *
 * Return: N/A
 *
 * Expects
 *      values in info array are valid, info is not null, caller is 
 *      manipulating the UM in legal ways (that won't cause endless loops or
 *      other runtime issues), and indices (A and B) are not out of bounds
 * 
 * Notes:
 *      N/A
 ************************/
void segmented_store(uint32_t A, uint32_t B, uint32_t C)
{
        struct segment seg_str = segments[reg[A]];
        uint32_t *seg = seg_str.seg;
        int length_seg = seg_str.length;
        assert((int)reg[B] <= length_seg);
        seg[reg[B]] = reg[C];
}

/********** addition ********
 *
 * Adds the values in registers B and C and stoes the result in register A
 *
 * Parameters:
 *      UM my_um:         the UM being operated on
 *      int *info:        an int array holding the A, B, C, and/or value 
 *
 * Return: N/A
 *
 * Expects
 *      values in info array are valid, info is not null, and caller is 
 *      manipulating the UM in legal ways (that won't cause endless loops or
 *      other runtime issues)
 * 
 * Notes:
 *      N/A
 ************************/
void addition(uint32_t A, uint32_t B, uint32_t C)
{
        uint64_t regB = reg[B];
        uint64_t regC = reg[C];
        uint64_t regA = regB + regC;
        reg[A] = regA;
}

/********** multiplication ********
 *
 * Multiplies the values in registers B and C and stores the result in register
 * A.
 *
 * Parameters:
 *      UM my_um:         the UM being operated on
 *      int *info:        an int array holding the A, B, C, and/or value 
 *
 * Return: N/A
 *
 * Expects
 *      values in info array are valid, info is not null, and caller is 
 *      manipulating the UM in legal ways (that won't cause endless loops or
 *      other runtime issues)
 * 
 * Notes:
 *      N/A
 ************************/
void multiplication(uint32_t A, uint32_t B, uint32_t C) 
{
        reg[A] = reg[B] * reg[C];
}

/********** division ********
 *
 * Divides the values in registers B and C and stores the result in register
 * A.
 *
 * Parameters:
 *      UM my_um:         the UM being operated on
 *      int *info:        an int array holding the A, B, C, and/or value 
 *
 * Return: N/A
 *
 * Expects
 *      values in info array are valid, info is not null, and caller is 
 *      manipulating the UM in legal ways (that won't cause endless loops or
 *      other runtime issues)
 * 
 * Notes:
 *      N/A
 ************************/
void division(uint32_t A, uint32_t B, uint32_t C) 
{
        reg[A] = reg[B] / reg[C];
}

/********** nand ********
 *
 * Performs a bitwise NOT AND on the values in registers B and C and stores the
 * result in register A.
 *
 * Parameters:
 *      UM my_um:         the UM being operated on
 *      int *info:        an int array holding the A, B, C, and/or value 
 *
 * Return: N/A
 *
 * Expects
 *      values in info array are valid, info is not null, and caller is 
 *      manipulating the UM in legal ways (that won't cause endless loops or
 *      other runtime issues)
 * 
 * Notes:
 *      N/A
 ************************/
void nand(uint32_t A, uint32_t B, uint32_t C)
{
        reg[A] = ~(reg[B] & reg[C]);
}

/********** map ********
 *
 * Creates a new segment, and puts its segment ID in register B. If there are
 * any unmapped segments, then one of those segment IDs is used; otherwise, 
 * the segment is placed at the end of the segments sequence.
 *
 * Parameters:
 *      UM my_um:         the UM being operated on
 *      int *info:        an int array holding the A, B, C, and/or value 
 *
 * Return: N/A
 *
 * Expects
 *      values in info array are valid, info is not null, and caller is 
 *      manipulating the UM in legal ways (that won't cause endless loops or
 *      other runtime issues)
 * 
 * Notes:
 *      N/A
 ************************/
void map(UM my_um, uint32_t B, uint32_t C)
{
        uint32_t *new_seg = malloc(sizeof(uint32_t) * reg[C]);
        /* intializing words in new segment to 0 */
        for (long i = 0; i < reg[C]; i++) {
                new_seg[i] = 0;
        }
        struct segment new_seg_str;
        new_seg_str.seg = new_seg;
        new_seg_str.length = reg[C];
        uint32_t seg_id;
        /* checking if unmapped sequences are available */
        if (Seq_length(my_um->unmapped) != 0) {
                seg_id = (uint32_t)(uintptr_t)Seq_remhi(my_um->unmapped);
                segments[seg_id] = new_seg_str;
        } else { 
                seg_id = my_um->num_segs;
                if (my_um->num_segs == my_um->segments_size) {
                        expand(my_um);
                }
                segments[my_um->num_segs] = new_seg_str; 
                my_um->num_segs++;
        }
        
        reg[B] = seg_id;
}

/********** unmap ********
 *
 * Unmaps and frees a segment whose ID is stored in register C.
 *
 * Parameters:
 *      UM my_um:         the UM being operated on
 *      int *info:        an int array holding the A, B, C, and/or value 
 *
 * Return: N/A
 *
 * Expects
 *      values in info array are valid, info is not null, and caller is 
 *      manipulating the UM in legal ways (that won't cause endless loops or
 *      other runtime issues)
 * 
 * Notes:
 *      The segment is replaced with an empty sequence. 
 ************************/
void unmap(UM my_um, uint32_t C) 
{
        uint32_t seg_id = reg[C];
        
        struct segment unmapped_seg = segments[seg_id];
        Seq_addhi(my_um->unmapped, (void *)(uintptr_t)seg_id);
        free(unmapped_seg.seg);

        struct segment dummy_str;
        dummy_str.seg = NULL;
        segments[seg_id] = dummy_str;
}

/********** output ********
 *
 * Prints the character associated with the ASCII value in register C to stdout
 *
 * Parameters:
 *      UM my_um:         the UM being operated on
 *      int *info:        an int array holding the A, B, C, and/or value 
 *
 * Return: N/A
 *
 * Expects
 *      values in info array are valid, info is not null, and caller is 
 *      manipulating the UM in legal ways (that won't cause endless loops or
 *      other runtime issues). Register C should contain a value between 0
 *      and 255.
 * 
 * Notes:
 *      N/A
 ************************/
void output(uint32_t C)
{
        assert(reg[C] < 256);
        printf("%c", reg[C]);
}

/********** input ********
 *
 * Takes a character as input from stdin and stores it in register C.
 *
 * Parameters:
 *      UM my_um:         the UM being operated on
 *      int *info:        an int array holding the A, B, C, and/or value 
 *
 * Return: N/A
 *
 * Expects
 *      values in info array are valid, info is not null, and caller is 
 *      manipulating the UM in legal ways (that won't cause endless loops or
 *      other runtime issues). Input should be a value between 0 and 255 or a
 *      character.
 * 
 * Notes:
 *      N/A
 ************************/
void input(uint32_t C)
{
        char input = getchar();
        reg[C] = input;
}

/********** load_program ********
 *
 * Replaces segment[0] with segment[ register[B] ]. The program counter is set
 * to the value in register C.
 *
 * Parameters:
 *      UM my_um:         the UM being operated on
 *      int *info:        an int array holding the A, B, C, and/or value 
 *
 * Return: N/A
 *
 * Expects
 *      values in info array are valid, info is not null, and caller is 
 *      manipulating the UM in legal ways (that won't cause endless loops or
 *      other runtime issues). Register C should contain a value that 
 *      corresponds to the segment ID of a mapped segment.
 * 
 * Notes:
 *      Original of duplicated segment remains intact. If register B contains 
 *      0, then only the program counter is updated.
 * 
 ************************/
void load_program(UM my_um, uint32_t B, uint32_t C)
{
        my_um->counter = reg[C];
        if (reg[B] != 0) {
            /* og is segment 0 */
                struct segment og_seg_B_str = segments[reg[B]];
                uint32_t *og_seg_B = og_seg_B_str.seg;
                int length_og_seg_B = og_seg_B_str.length;
                uint32_t *dup_seg = malloc(sizeof(uint32_t) * length_og_seg_B);
                for (int i = 0; i < length_og_seg_B; i++) {
                        uint32_t word = og_seg_B[i];
                        dup_seg[i] = word;
                }
                my_um->length_seg0 = length_og_seg_B;
                free(segments[0].seg);
                struct segment dup_seg_str;
                dup_seg_str.seg = dup_seg;
                dup_seg_str.length = length_og_seg_B;
                segments[0] = dup_seg_str;
                
        }
        
}  

/*TODO: make um struct not be pointer. Also make reg array a global*/
void expand(UM my_um)
{
        int new_size = 2 * my_um->segments_size; 
        segments = realloc(segments, sizeof(struct segment) * new_size);
        int num_segs = my_um->num_segs;

        for (int i = num_segs; i < new_size; i++) {
                struct segment dummy_str;
                dummy_str.seg = NULL;
                segments[i] = dummy_str;
        }
        my_um->segments_size = new_size;
        
}