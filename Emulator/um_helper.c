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

/* secret functions not to be used by client directly */
int *get_registers(uint32_t word, bool loadval);
uint32_t fetch(UM my_um);
bool execute(UM my_um, int *info, int op_code);
void conditional_move(UM my_um, int *info);
void segmented_load(UM my_um, int *info);
void load_val(UM my_um, int *info);
void segmented_store(UM my_um, int *info);
void addition(UM my_um, int *info);
void multiplication(UM my_um, int *info);
void division(UM my_um, int *info);
void nand(UM my_um, int *info);
void map(UM my_um, int *info);
void unmap(UM my_um, int *info);
void output(UM my_um, int *info);
void input(UM my_um, int *info);
void load_program(UM my_um, int *info);

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
    my_um->segments = Seq_new(1000); 
    my_um->counter = 0;
    my_um->unmapped = Seq_new(100); 
    for (int i = 0; i < 8; i++) {
        my_um->reg[i] = 0;
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
    int length = Seq_length(my_um->segments);
    for (int i = 0; i < length; i++) {
        Seq_T seg = Seq_remlo(my_um->segments);
        Seq_free(&seg);
    }
    Seq_free(&(my_um->segments));
    Seq_free(&(my_um->unmapped));
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
    Seq_T seg_0 = Seq_new(num_words);
    uint8_t byte;
    uint32_t word;
    /* add each word to segment, then add segment to the segments seq_t */
    for (int i = 0; i < num_words; i++) {
        word = 0;
        for (int i = 1; i <= 4; i++) { /* for each word */
            byte = fgetc(infile);
            word = Bitpack_newu(word, 8, 32 - (i * 8), byte);
        }
        Seq_addhi(seg_0, (void *)(uintptr_t)word);
    }
    Seq_addlo(my_um->segments, seg_0);
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
    Seq_T curr_seg = Seq_get(my_um->segments, 0);
    uint32_t word = (uint32_t)(uintptr_t)Seq_get(curr_seg, my_um->counter);
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
        int op_code = Bitpack_getu(word, 4, 28);
        int *info;
        if (op_code == 13) { /* loadval has different encoding */
            info = get_registers(word, true);
        } else {
            info = get_registers(word, false);
        }

        halt = execute(my_um, info, op_code);
        free(info);
        if (halt) {
            break;
        }
        if (my_um->counter >= Seq_length(Seq_get(my_um->segments, 0))) {
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
bool execute(UM my_um, int *info, int op_code)
{
    bool halt = false;
 
    switch (op_code) {
        case 0:
            conditional_move(my_um, info);
            break;
        case 1:
            segmented_load(my_um, info);
            break;
        case 2:
            segmented_store(my_um, info);
            break;
        case 3:
            addition(my_um, info);
            break;
        case 4:
            multiplication(my_um, info);
            break;
        case 5:
            division(my_um, info);
            break;
        case 6:
            nand(my_um, info);
            break;
        case 7:
            halt = true;
            break;
        case 8: 
            map(my_um, info);
            break;
        case 9:
            unmap(my_um, info);
            break;
        case 10:
            output(my_um, info);
            break;
        case 11:
            input(my_um, info);
            break;
        case 12:
            load_program(my_um, info);
            break;
        case 13:
            load_val(my_um, info);
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
void load_val(UM my_um, int *info) 
{
    int A = info[0];
    unsigned val = info[1];
    my_um->reg[A] = val;
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
void conditional_move(UM my_um, int *info)
{
    int A = info[0];
    int B = info[1];
    int C = info[2];
    if (my_um->reg[C] != 0) {
        my_um->reg[A] = my_um->reg[B];
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
void segmented_load(UM my_um, int *info)
{
    int A = info[0];
    int B = info[1];
    int C = info[2];
    Seq_T seg = (Seq_T)Seq_get(my_um->segments, my_um->reg[B]);
    my_um->reg[A] = (uint32_t)(uintptr_t)Seq_get(seg, my_um->reg[C]);
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
void segmented_store(UM my_um, int *info)
{
    int A = info[0];
    int B = info[1];
    int C = info[2];
    Seq_T seg = (Seq_T)Seq_get(my_um->segments, my_um->reg[A]);
    assert((int)my_um->reg[B] <= Seq_length(seg));
    if ((int)my_um->reg[B] == Seq_length(seg)) {
        Seq_addhi(seg, (void *)(uintptr_t)my_um->reg[C]);
    } else { /* seq_put can only replace existing index, not create new ones */
        Seq_put(seg, my_um->reg[B], (void *)(uintptr_t)my_um->reg[C]);
    } 
    
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
void addition(UM my_um, int *info)
{
    int A = info[0];
    int B = info[1];
    int C = info[2];
    uint64_t regB = my_um->reg[B];
    uint64_t regC = my_um->reg[C];
    uint64_t regA = regB + regC;
    my_um->reg[A] = regA;
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
void multiplication(UM my_um, int *info) 
{
    int A = info[0];
    int B = info[1];
    int C = info[2];
    my_um->reg[A] = my_um->reg[B] * my_um->reg[C];
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
void division(UM my_um, int *info) 
{
    int A = info[0];
    int B = info[1];
    int C = info[2];
    my_um->reg[A] = my_um->reg[B] / my_um->reg[C];
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
void nand(UM my_um, int *info)
{
    int A = info[0];
    int B = info[1];
    int C = info[2];
    my_um->reg[A] = ~(my_um->reg[B] & my_um->reg[C]);
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
void map(UM my_um, int *info)
{
    int B = info[1];
    int C = info[2];
    Seq_T new_seg = Seq_new(C);
    uint32_t seg_id;
    /* checking if unmapped sequences are available */
    if (Seq_length(my_um->unmapped) != 0) {
        seg_id = (uint32_t)(uintptr_t)Seq_remhi(my_um->unmapped);
        Seq_T buffer = Seq_get(my_um->segments, seg_id);
        Seq_free(&buffer); /* freeing small size 1 sequence */
        Seq_put(my_um->segments, seg_id, (void *)new_seg);
    } else { 
        seg_id = Seq_length(my_um->segments);
        Seq_addhi(my_um->segments, new_seg);
    }

    /* intializing words in new segment to 0 */
    for (int i = 0; i < (int)my_um->reg[C]; i++) {
       Seq_addhi(new_seg, (void*)(uintptr_t)0);
    }
    my_um->reg[B] = seg_id;
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
void unmap(UM my_um, int *info) 
{
    int C = info[2];
    uint32_t seg_id = my_um->reg[C];
    Seq_T unmapped_seg = Seq_get(my_um->segments, seg_id);
    Seq_addhi(my_um->unmapped, (void *)(uintptr_t)seg_id);
    Seq_free(&unmapped_seg);
    Seq_put(my_um->segments, seg_id, (void*)Seq_new(1));
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
void output(UM my_um, int *info)
{
    int C = info[2];
    assert(my_um->reg[C] < 256);
    printf("%c", my_um->reg[C]);
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
void input(UM my_um, int *info)
{
    int C = info[2];
    char input = fgetc(stdin);
    my_um->reg[C] = input;
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
void load_program(UM my_um, int *info)
{
    int B = info[1]; /* segment id to be duplicated */
    int C = info[2]; /* program counter new position */
    my_um->counter = my_um->reg[C];
    if (my_um->reg[B] != 0) {

        /* og is segment 0 */
        Seq_T og_seg_B = Seq_get(my_um->segments, my_um->reg[B]);
        Seq_T duplicate = Seq_new(Seq_length(og_seg_B));
        for (int i = 0; i < Seq_length(og_seg_B); i++) {
            uint32_t word = (uintptr_t)Seq_get(og_seg_B, i);
            Seq_addhi(duplicate, (void *)(uintptr_t)word);
        }
     
        Seq_T dum = Seq_put(my_um->segments, 0, (void *)duplicate);
        Seq_free(&dum);
    }
}

/********** get_registers ********
 *
 * Helper function for fetch that stores the key values in a heap-allocated int
 * array. If loadval is true, then only register A and the 25-bit value in the
 * codeword are stored; otherwise, A, B, and C are stored.
 *
 * Parameters:
 *      uint32_t word:    the word to unpack
 *      bool loadval:     whether the op_code is 13
 *
 * Return: a heap-allocated int array
 *
 * Expects
 *      N/A
 * 
 * Notes:
 *      Caller must delete info array after use.
 * 
 ************************/
int *get_registers(uint32_t word, bool loadval)
{
    int *info;
    if (loadval) {
        info = malloc(sizeof(int) * 2);
        unsigned regA = Bitpack_getu(word, 3, 25);
        unsigned val = Bitpack_getu(word, 25, 0);
        info[0] = regA;
        info[1] = val;
    } else {
        info = malloc(sizeof(int) * 3);
        unsigned regA = Bitpack_getu(word, 3, 6);
        unsigned regB = Bitpack_getu(word, 3, 3);
        unsigned regC = Bitpack_getu(word, 3, 0);
        info[0] = regA;
        info[1] = regB;
        info[2] = regC;
    }
    
    return info;
}
