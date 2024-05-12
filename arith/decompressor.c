/*************************** decompressor.c **********************
 *                         
 *     Authors: Javier Laveaga {jlavea01} and Prince Millidah {pmilli03}, 
 *     Date: 2/27/2024
 *     Assignment: arith
 *
 *     Summary: This file implements the decompressor which reads in the 
 *     the 32-bit words in a file and outputs the decompressed PPM 
 *     to standard output.
 *
 ***************************************************************/


#include "decompressor.h"

/******************* decompress40 ************************
 * Description: decompresses a compressed image file
 * 
 * Parameters: 
 *     FILE *input: file to be decompressed
 * 
 * Returns: void but write the decompressed image to stdout
 * 
 * Expects:
 *     The file to be in the compressed image format
 * 
 * Notes:
 *      Frees all memory allocated in the process of decompression
 *      which include the UArray_T codewords, Seq_T blocks, and
 *      Pnm_ppm reader
 **********************************************************/
extern void decompress40(FILE *input)
{
        assert(input != NULL);
        /* sets methods to blocked and denominator to 255 */
        Pnm_ppm reader = init_reader (255);

        /* array of words. Also updates reader width and height*/
        UArray_T codewords = readFile (input, reader); 

        /* blocks with a,b,c,d,Pb_avg,Pr_avg */
        Seq_T blocks = words_to_array (codewords);

        /* blocked array with Pb,Pr, and Y floats (per pixel) */
        A2Methods_UArray2 blocked_array = seq_to_bArray (blocks, reader);
        reader->pixels = blocked_array;

        /*getting my int rgb values*/
        float_to_int(reader); /* frees blocked_array */
        Pnm_ppmwrite(stdout, reader); /* write to stdout */

        Seq_free(&blocks);
        UArray_free(&codewords); 
        Pnm_ppmfree(&reader);
} 

/****************** init_reader ************************
 * Description: initializes a Pnm_ppm struct
 * 
 * Parameters: 
 *     int denominator: denominator for Pnm_ppm struct
 * 
 * Returns: Pnm_ppm
 * 
 * Expects:
 *    The denominator to be a positive integer
 * 
 * Notes:
 *        Sets the method to blocked
 **********************************************************/
Pnm_ppm init_reader (int denominator) 
{
        /* setting methods to blocked */
        A2Methods_T methods = uarray2_methods_blocked;

        /* creating a reader struct */
        struct Pnm_ppm *reader;
        NEW(reader);
        assert(reader != NULL);

        /* setting values for reader */
        reader->denominator = 255;
        reader->methods = methods;
        reader->width = 0;
        reader->height = 0;
        reader->pixels = NULL;
        reader->denominator = denominator; 

        return reader;
}

/****************** float_to_int ************************
 * Description: converts a Pnm_ppm struct with float values
 *              to a Pnm_ppm struct with int values
 * 
 * Parameters: 
 *     Pnm_ppm reader: Pnm_ppm struct with float values
 * 
 * Returns: void but updates the Pnm_ppm struct with int values
 * 
 * Expects:
 *       reader to have a pixels field with float values
 * 
 * Notes:
 *        Frees the float array
 **********************************************************/
/* go from r,g,b to blocked major */
void float_to_int(Pnm_ppm reader) 
{ 
        assert(reader != NULL);
        A2Methods_T methods = (A2Methods_T) reader->methods; /*for int array*/
        A2Methods_UArray2 float_Array = reader->pixels;
        
        /*Creating new float Array*/
        int int_struct_size = sizeof(struct Pnm_rgb);
        
        /* Creating the array to store the rgb values */
        A2Methods_UArray2 IntArray = methods->new_with_blocksize
                (reader->width, reader->height, int_struct_size, BLOCK_SIZE); 
        assert(IntArray != NULL);

        /* iterating through float array and updating float array with rgb */
        methods->map_block_major(IntArray, PstoInt, reader);

        /* freeing float array and updating reader */
        reader->pixels = IntArray;
        methods->free(&float_Array);
}


/****************** roundRGB ************************
 * Description: rounds a float value to the nearest int
 * 
 * Parameters: 
 *     float *RGBValue: pointer to the float value to be rounded
 * 
 * Returns: void but updates the value of RGBValue
 * 
 * Expects:
 *      RGBValue to be a pointer to a float value and must not be NULL
 * 
 * Notes:
 *        Will CRE if RGBValue is NULL
 **********************************************************/
void roundRGB(float *RGBValue) 
{
        assert(RGBValue != NULL);
        if (*RGBValue < 0) {
                *RGBValue = 0.0;
        }
        if (*RGBValue > 1.0) {
                *RGBValue = 1.0;
        }
}


/****************** PstoInt ************************
 * Description: converts a P and Y float struct to a Pnm_rgb struct
 * 
 * Parameters: 
 *     int col: column of the current element
 *     int row: row of the current element
 *     A2Methods_UArray2 my_array: the array being iterated through
 *     A2Methods_Object *ptr: pointer to the current element
 *     void *reader: the Pnm_ppm struct with the float values
 * 
 * Returns: void but updates the Pnm_ppm struct with int values
 * 
 * Expects:
 *    reader to have a pixels field with float values
 * 
 * Notes:
 *        Updates the Pnm_ppm struct with the int values
 **********************************************************/
void PstoInt(int col, int row, A2Methods_UArray2 my_array,
                                A2Methods_Object *ptr, void *reader)
{
        assert(ptr != NULL);
        assert(reader != NULL);
        Pnm_ppm my_reader = reader; 
        struct Pnm_rgb *currElem = ptr;

        A2Methods_T methods = (A2Methods_T) my_reader->methods;
        struct P_and_Y_floats *floatValues = methods->at(my_reader->pixels, col, row);
        
        /*getting float values*/
        float y = floatValues->Y;
        float pb = floatValues->Pb;
        float pr = floatValues->Pr;

        /*converting to rgb*/
        float r = 1.0 * y + 0.0 * pb + 1.402 * pr;
        float g = 1.0 * y - 0.344136 * pb - 0.714136 * pr;
        float b = 1.0 * y + 1.772 * pb + 0.0 * pr;

        /*rounding values*/
        roundRGB(&r);
        roundRGB(&g);
        roundRGB(&b);
        
        /*updating rgb values*/
        currElem->red = (int)(r * RGB_MAX);
        currElem->green = (int)(g * RGB_MAX);
        currElem->blue = (int)(b * RGB_MAX);
        
        (void) my_array;
}

/****************** seq_to_bArray ************************
 * Description: converts a sequence of P_avg_abcd structs to a 
 *              A2Methods_UArray2 of P_and_Y_floats structs
 * 
 * Parameters: 
 *     Seq_T blocks: sequence of P_avg_abcd structs
 *     Pnm_ppm reader: Pnm_ppm struct with float values
 * 
 * Returns: A2Methods_UArray2 of P_and_Y_floats structs
 * 
 * Expects:
 *      blocks to be a sequence of P_avg_abcd structs and reader to have
 *     a pixels field with float values
 * 
 * Notes:
 *        Frees the sequence of P_avg_abcd structs
 **********************************************************/
A2Methods_UArray2 seq_to_bArray (Seq_T blocks, Pnm_ppm reader) 
{
        assert (blocks != NULL);
        assert (reader != NULL);
        /* Getting the methods from reader */
        A2Methods_T methods = (A2Methods_T) reader->methods;
        
        /* Creating the array to store the float values */
        int int_struct_size = sizeof(struct P_and_Y_floats);
        A2Methods_UArray2 blockArray = methods->new_with_blocksize
                (reader->width, reader->height, int_struct_size, BLOCK_SIZE);
        assert(blockArray != NULL);

        /* Creating a closure to pass to the map function */                 
        struct decompClosure *seq_info;
        NEW(seq_info);
        assert(seq_info != NULL);

        /* Setting the values of the closure */
        seq_info->index = 0;
        seq_info->blockIndex = 0;
        seq_info->sequence = blocks;

        /* Iterating through the sequence and updating the array */
        methods->map_block_major(blockArray, getfloatValues, seq_info); 
        FREE(seq_info); /*freeing closure*/

        return blockArray;
}


/****************** getfloatValues ************************
 * Description: gets the float values from the sequence of P_avg_abcd
 *              structs and updates the array of P_and_Y_floats structs
 * 
 * Parameters: 
 *     int col:                 column of the current element
 *     int row:                 row of the current element
 *     A2Methods_UArray2 my_array: the array being iterated through
 *     A2Methods_Object *ptr:      pointer to the current element
 *     void *cl:                   the closure with the sequence of P_avg_abcd 
 *                                 structs
 * 
 * Returns: void
 * 
 * Expects:
 *     The closure to have a sequence of P_avg_abcd structs
 * 
 * Notes:
 *        Updates the array of P_and_Y_floats structs
 **********************************************************/
void getfloatValues(int col, int row, A2Methods_UArray2 my_array,
                                A2Methods_Object *ptr, void *decompClosure)
{
        assert(ptr != NULL);
        assert(decompClosure != NULL);
        /* Getting the closure */
        struct decompClosure *currClosure = decompClosure;
        struct P_avg_abcd *currDisComp = Seq_get(currClosure->sequence, 
                                        currClosure->index);

        /* Setting the values of the current element */
        struct P_and_Y_floats *currElem = ptr;
        currElem->Pb = currDisComp->Pb_avg;
        currElem->Pr = currDisComp->Pr_avg;

        currClosure->blockIndex += 1;  /*index of block*/
        

        /* Getting the Y values from the sequence based on the blockIndex */
        if (currClosure->blockIndex == 1) { /* Y1 */ 
                currElem->Y = currDisComp->a - currDisComp->b - currDisComp->c 
                                + currDisComp->d;
        } else if (currClosure->blockIndex == 3) { /* Y2 */
                currElem->Y = currDisComp->a - currDisComp->b + currDisComp->c 
                                - currDisComp->d;
        } else if (currClosure->blockIndex == 2) { /* Y3 */ 
                currElem->Y = currDisComp->a + currDisComp->b - currDisComp->c 
                                - currDisComp->d;
        } else { /* Y4 */ 
                currElem->Y = currDisComp->a + currDisComp->b + currDisComp->c 
                                + currDisComp->d;
                currClosure->index += 1; /*index of sequence*/
                FREE(currDisComp);
                currClosure->blockIndex = 0;
        }
        (void) col;
        (void) row;
        (void) my_array;
}

/****************** words_to_array ************************
 * Description: converts a UArray_T of codewords to a sequence of 
 *              P_avg_abcd structs
 * 
 * Parameters: 
 *     UArray_T codewords: UArray_T of codewords
 * 
 * Returns: Seq_T of P_avg_abcd structs
 * 
 * Expects:
 *       UArray_T codewords to be a UArray_T of uint32_t codewords
 * 
 * Notes:
 *        Frees the UArray_T of codewords
 **********************************************************/
Seq_T words_to_array (UArray_T codewords) 
{
        assert(codewords != NULL);
        /* Creating a sequence to store the P_avg_abcd structs */
        Seq_T blocks = Seq_new(0);
        assert(blocks != NULL);

        /* Iterating through the UArray_T and updating the sequence */
        for (int i = 0; i < UArray_length(codewords); i++) {

                /* Getting the current word */
                uint32_t *word = (uint32_t *)UArray_at(codewords, i);

                /* Creating a P_avg_abcd struct to store the float values */
                struct P_avg_abcd *currFloats;
                NEW(currFloats);
                assert(currFloats != NULL);

                /* unpacking word and adding it to sequence */
                unpackWord(*word, currFloats);
                Seq_addhi(blocks, currFloats); 
        }

        return blocks;
}


/****************** unpackWord ************************
 * Description: unpacks a word and updates a P_avg_abcd struct
 * 
 * Parameters: 
 *     uint32_t word: the word to be unpacked
 *     struct P_avg_abcd *currFloats: the struct to be updated
 * 
 * Returns: void but updates the struct with the float values
 * 
 * Expects:
 *      word to be a 32-bit word and currFloats to be a struct with
 *     float values. currFloat must not be NULL
 * 
 * Notes:
 *        Frees the UArray_T of codewords. Will CRE if currFloats is NULL
 **********************************************************/
void unpackWord(uint32_t word, struct P_avg_abcd *currFloats)
{
        assert(currFloats != NULL);
        /* unpacking values*/
        unsigned Pr_int = Bitpack_getu(word, PS_WIDTH, PR_LSB);
        unsigned Pb_int = Bitpack_getu(word, PS_WIDTH, PB_LSB);
        unsigned a_q = Bitpack_getu(word, A_WIDTH, A_LSB);
        signed b_q = Bitpack_gets(word, BCD_WIDTH, B_LSB);
        signed c_q = Bitpack_gets(word, BCD_WIDTH, C_LSB);
        signed d_q = Bitpack_gets(word, BCD_WIDTH, D_LSB);

        /* unquantize values */
        float Pb = Arith40_chroma_of_index(Pb_int);
        float Pr = Arith40_chroma_of_index(Pr_int);
        float a = (a_q * 1.0/ A_MAX);
        float b = (b_q * 1.0/ BCD_MAX);
        float c = (c_q * 1.0/ BCD_MAX);
        float d = (d_q * 1.0/ BCD_MAX);

        /* updating struct */
        currFloats->a = a;
        currFloats->b = b;
        currFloats->c = c;
        currFloats->d = d;
        currFloats->Pb_avg = Pb;
        currFloats->Pr_avg = Pr;
}


/****************** readFile ************************
 * Description: reads a compressed image file and updates a Pnm_ppm
 *              struct with the width and height of the image
 * 
 * Parameters: 
 *     FILE *in: the file to be read
 *     Pnm_ppm reader: the Pnm_ppm struct to be updated
 * 
 * Returns: UArray_T of codewords
 * 
 * Expects:
 *      The file to be in the compressed image format
 * 
 * Notes:
 *        Frees the UArray_T of codewords
 **********************************************************/
UArray_T readFile (FILE *in, Pnm_ppm reader) 
{
        assert(in != NULL);
        assert(reader != NULL);
        unsigned height, width;  /* Store width and height of image */
        /*read in width and height*/
        int read = fscanf(in, "COMP40 Compressed image format 2\n%u %u", &width, 
                                        &height);
        assert(read == 2); /*asserting it read in 2 values*/
        int c = getc(in); /*getting next thing, which should be a new line*/
        assert(c == '\n');

        /*updating reader with the width and height */ 
        reader->width = width;
        reader->height = height;

        /*number of words in file*/
        int num_words = width * height / BLOCK_ELEM;

        /*creating array of codewords*/
        UArray_T codeWords = UArray_new(num_words, sizeof(uint32_t));
        assert(codeWords != NULL);

        /*reading in words from file*/
        for (int i = 0; i < UArray_length(codeWords); i++) {
                int currlsb = 32;
                uint32_t word;
                int byte;
                /*read in 4 bytes to create a word*/
                for (int j = 0; j < 4; j++) {
                        currlsb -= BYTE;
                        /*reads 8 bites.call it 4 times for each word*/
                        byte = getc(in); 
                        word = Bitpack_newu(word, BYTE, currlsb, byte);
                }
                /*save word into array*/
                uint32_t *element = (uint32_t *)UArray_at(codeWords, i);
                *element = word;
        }

        return codeWords;
}
