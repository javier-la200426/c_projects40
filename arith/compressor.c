/************************** compressor.c ***********************
 *                          
 *     Authors: Javier Laveaga {jlavea01} and Prince Millidah {pmilli03}, 
 *     Date: 2/27/2024
 *     Assignment: arith
 *
 *     Summary: This file implements the compressor which takes in a a ppm
 *     file and stores it as 32-bit words. It writes the words to stdout
 *
 ***************************************************************/
#include "compressor.h"


/************* compress40 *************
 * Description:
 *    Main driver function which compresses a PPM image to a 32-bit word
 *    and writes it to standard output.
 * 
 * Parameters:
 *    FILE *input: file pointer to the input file
 * 
 * Returns:   Nothing but frees memory allocated for storing the codeWords
 *            and frees the memory allocated for the Pnm_ppm struct.
 *
 * Expects:
 *   The input file must be a PPM image. 
 * Notes:
 *      Raises CRE if invalid image format
 *  
 * *******************************************************/
extern void compress40 (FILE *input) 
{
        assert (input != NULL);
        /* Reading the PPM file into the pixel into blocks of size 2 */
        Pnm_ppm reader = read_ppm(input);

        int_to_float (reader); /* Converting RGB value to floats */

        Seq_T blocks_seq = get_avg_Ps (reader); /* Getting the P and Y values */

        /* Packing word in array and printing it out */
        UArray_T words = seq_to_words(blocks_seq); /*frees siq*/
        print(words, reader);

        /* Freeing memory */
        UArray_free(&words); 
        Pnm_ppmfree(&reader);
}


/********************** print ***********************
 * Description:
 *    Prints the compressed image as words to standard output.
 * 
 * Parameters:
 *   UArray_T codeWords: UArray of 32-bit words
 *   Pnm_ppm reader: Pnm_ppm struct to get width and height
 * 
 * Returns:   Nothing
 * 
 * Expects:
 *      The UArray must not be empty and the Pnm_ppm struct must not be NULL.
 * Notes:
 *      Prints word to standard output
 * 
 *************************************************************/
void print(UArray_T codeWords, Pnm_ppm reader) 
{
        assert(reader != NULL);
        assert(codeWords != NULL);
        /* Getting the width and height of the image */
        int width = reader->width;
        int height = reader->height;

        
        /* Printing the header */
        printf("COMP40 Compressed image format 2\n%u %u", width, height);
        printf("\n");

        /* Printing the 32-bit words */
        for (int i = 0; i < UArray_length(codeWords); i++) {
                uint32_t *word = (uint32_t *)UArray_at(codeWords, i);
                int currlsb = 32;

                /*Printing one bytes at a time */
                for (int j = 0; j < NUM_BYTE; j++) {
                        currlsb -= BYTE; /*should start at 24*/
                        uint8_t currNum = Bitpack_getu(*word, BYTE, currlsb);
                        //(void)currNum; 
                        putchar(currNum);
                }
        }
}


/******************** populate *******************
 * Description:
 *    Apply function for block major in read_ppm
 *    which populates the new array of blocksize 2 with the old array
 *    from reader which does not have desired block size.
 * 
 * Parameters:
 *   int col:                             column index
 *   int row:                             row index
 *   A2Methods_UArray2 new_array:         new array block size of 2
 *   A2Methods_Object *ptr:               pointer to the element in old array
 *   void *reader:                        void pointer to the reader
 * 
 * Returns:   Nothing
 * 
 * Expects:
 *  The new array must not be NULL and the reader must not be NULL.
 * Notes:
 *   Will CRE if the new array is NULL or if reader is NULL
 *************************************************************/
void populate(int col, int row, A2Methods_UArray2 new_array,
                                A2Methods_Object *ptr, void *reader)
{
        /* geting reader and methods*/
        Pnm_ppm my_reader = reader; 
        A2Methods_T methods = (A2Methods_T) my_reader->methods;

        /*getting new and old_element */
        struct Pnm_rgb* new_element = ptr;
        struct Pnm_rgb* old_element = methods->at(my_reader->pixels, col, row);

        /* Copying old element into new element*/
        new_element->red = old_element->red;
        new_element->green = old_element->green;
        new_element->blue = old_element->blue;

        (void) new_array;
}


/********************** read_ppm ***********************
 * Description:
 *    Reads the PPM image from the input file, trims it, saves
 *    it into a new blocked array of blocksize 2 and returns the 
 *    updated Pnm_ppm reader.
 * 
 * Parameters:
 *   FILE *fp: file pointer to the input file
 * 
 * Returns:   Pnm_ppm struct
 * 
 * Expects:
 *  The input file pointer must point to a valid PPM image.
 * Notes:
 *    Will CRE if invalid file pointer. Trims the image if odd width or height.
 *    Frees the array in which the imiage is originally stored (old array)
 *************************************************************/
Pnm_ppm read_ppm(FILE *fp) 
{  
        assert (fp != NULL);
        /* Initialising methids to blocked */
        A2Methods_T methods = uarray2_methods_blocked;

        /* Reading the PPM image */
        Pnm_ppm reader = Pnm_ppmread(fp, methods);
        int size = methods->size(reader->pixels);

        /* Making sure the width and height are even */
        if (reader->width % 2 != 0) { 
                reader->width -= 1;
        } 
        if (reader->height % 2 != 0) {
                reader->height -= 1;
        }

        /* Creating a new array with the same size as the old array */
        A2Methods_UArray2 oldArray = reader->pixels;
        A2Methods_UArray2 newArray = methods->new_with_blocksize(reader->width, 
                                        reader->height, size, BLOCK_SIZE);

        assert(newArray != NULL);
        /*iterate over new array blocksize 2 and populate it with blocked array 
        from before*/
        methods->map_block_major(newArray, populate, reader);

        /*update reader pixels array with new array*/
        reader->pixels = newArray; 
        methods->free(&oldArray); /*Free old array*/

        return reader;
}


/********************** int_to_float ***********************
 * Description:
 *    Converts the unsigned RGB values in the Pnm_ppm struct to float
 *    values and saves them in the an array of P_and_Y_floats structs. 
 * 
 * Parameters:
 *   Pnm_ppm reader: Pnm_ppm reader which has the blocked array of blocksize 2.
 * 
 * Returns:   Nothing but frees the old array of RGB values from the reader
 * 
 * Expects:
 *  The Pnm_ppm struct must not be NULL.
 * Notes:
 *   It frees the old array of RGB values from the reader
 *************************************************************/
void int_to_float (Pnm_ppm reader) 
{
        /*getting methods from reader*/
        A2Methods_T methods = (A2Methods_T) reader->methods;
        A2Methods_UArray2 oldArray = reader->pixels;

        /*Creating new float Array*/
        int float_struct_size = sizeof(struct P_and_Y_floats);
        A2Methods_UArray2 floatArray = methods->new_with_blocksize
                (reader->width, reader->height, float_struct_size, BLOCK_SIZE);

        assert(floatArray != NULL);
        /* Iterating through float array and updating float array */
        methods->map_block_major(floatArray, toCompVideo, reader);

        /* Freeing the old array and updating the reader pixels */
        reader->pixels = floatArray;
        methods->free(&oldArray);
}


/****************** compute_rgb_float ******************
 * Description:
 *    Computes the float value from the RGB unsigned value.
 * 
 * Parameters:
 *   unsigned rgb:            Red, Green, or Blue value
 *   unsigned denominator:    denominator
 * 
 * Returns:   float which represents the new r,g, or b value as a 
 *  decimal which really s a percentage. 
 * 
 * Expects:
 *   The denominator must not be 0 and is expected to be 255.
 * Notes:
 *   The denominator is within the criteria specified. FOr us is 255
 * 
 *************************************************************/
float compute_rgb_float(unsigned rgb, unsigned denominator) 
{
        float new_value = (rgb * 1.0) / denominator;
        return new_value;
}


/********************** toCompVideo ***********************
 * Description:
 *    Maps block wise over the array of floats and computes
 *    the Pb, Pr, and Y values for each pixel and updates the
 *    P_and_Y_floats struct.
 * 
 * Parameters:
 *   int col:                             column index of pixel
 *   int row:                             row index of pixel
 *   A2Methods_UArray2 my_array:          array of floats
 *   A2Methods_Object *ptr:               pointer to the element
 *   void *reader:                        void pointer to the reader
 * 
 * Returns:   Nothing
 * 
 * Expects:
 *  The array of floats must not be NULL and the reader must not be NULL.
 * Notes:
 *      assertion raised if either array or reader is NULL
 *************************************************************/
void toCompVideo(int col, int row, A2Methods_UArray2 my_array,
                                A2Methods_Object *ptr, void *reader)
{
        Pnm_ppm my_reader = reader; 
        struct P_and_Y_floats *currElem = ptr;

        A2Methods_T methods = (A2Methods_T) my_reader->methods;
        struct Pnm_rgb *intValues = methods->at(my_reader->pixels, col, row);

        /*computing floats rgb*/
        float r = compute_rgb_float(intValues->red, my_reader->denominator);
        float g = compute_rgb_float(intValues->green, my_reader->denominator);
        float b = compute_rgb_float(intValues->blue, my_reader->denominator);

        /*computing Pb,Pr, Y floats from rgb*/
        currElem->Pb = (-0.168736 * r) - (0.331264 * g) + (0.5 * b); 
        currElem->Pr = (0.5 * r) - (0.418688 * g) - (0.081312 * b);
        currElem->Y = (0.299 * r) + (0.587 * g) + (0.114 * b);
        (void) my_array;
}


/********************** get_avg_Ps ***********************
 * Description:
 *    Maps block wise over array of floats Pb,Pr,Y for each pixel
 *    and calculates the average Pb, PR, and abcd (per 1 block (2*2 pixels))
 *    It then saves these average float values Pb, PR, and abcd in a struct.
 *    Struct is passed into sequence. Thus we get a sequence of structs/blocks.
 * 
 * Parameters:
 *   Pnm_ppm reader:                      reader with blocked array of floats
 *  
 * 
 * Returns:   Nothing
 * 
 * Expects:
 *  A reader which is not NULL
 * Notes:
 *     Seq needs to be freed outside of this function. Also the P_avg_abcd 
 *     structs inside the sequence also need to be freed.
 *************************************************************/
Seq_T get_avg_Ps (Pnm_ppm reader) 
{
        /*getting methods and float array. Methods is initialized to blocked*/
        A2Methods_T methods = (A2Methods_T) reader->methods;
        /*blocked array*/
        A2Methods_UArray2 float_array = reader->pixels; /*contains Pb,Pr,Y*/

        /*allocate memory for new closure struct and initialize it */
        struct compClosure *avgClosure;
        NEW(avgClosure);
        assert(avgClosure != NULL);

        /*create new plain array where each slot contains info for 1 block */
        int new_width = reader->width / BLOCK_SIZE;
        int new_height = reader->height / BLOCK_SIZE;
        avgClosure->blocks = Seq_new(new_width * new_height);
        avgClosure->counter = 0;
        /*block major in old array and when counter is 4, insert at new array*/
        methods->map_block_major(float_array, to_avg_and_abcd, avgClosure);

        Seq_T mySeq = avgClosure->blocks;
        FREE(avgClosure); 
        return mySeq;
}

/********************** to_avg_and_abcd***********************
 * Description:
 *    Apply function called in get_avg_Ps which populates the P_avg_abcd 
 *     structs and adds them the the sequence. 
 * 
 * Parameters:
 *   int col:                             column index of pixel
 *   int row:                             row index of pixel
 *   A2Methods_UArray2 my_array:          array of P avg and abcd floats
 *   A2Methods_Object *ptr:               pointer to the element
 *   void *avgClosure:                    pointer to an avgClosure                   
 *  
 * 
 * Returns:   Nothing
 * 
 * Expects:
 *  A reader which is not NULL
 * Notes:
 *     Seq needs to be freed outside of this function. Also the P_avg_abcd 
 *     structs inside the sequence also need to be freed.
 *************************************************************/
void to_avg_and_abcd(int col, int row, A2Methods_UArray2 my_array,
                                A2Methods_Object *ptr, void *avgClosure)
{
        (void) col;
        (void) row;
        (void) my_array;
        /*struct with info in it*/
        struct P_and_Y_floats *currElem = ptr; /*blocked array element*/
        struct compClosure *currClosure = avgClosure;
        currClosure->counter ++;

        if (currClosure->counter == 1) { /*1st block */
                /*get the struct P_avg_abcd*/
                /*initialize block_vals to update it later*/
                struct P_avg_abcd *block_vals;
                currClosure->block_vals = NEW(block_vals);
                get_Y1(currClosure->block_vals, currElem);
        } else if (currClosure->counter == 3) { /*2nd block*/
                get_Y2(currClosure->block_vals, currElem); /*new*/
        } else if (currClosure->counter == 2) {
                get_Y3(currClosure->block_vals, currElem);
        } else if (currClosure->counter == 4) {
                get_Y4(currClosure->block_vals, currElem);
                currClosure->counter = 0;
                divide_four(currClosure->block_vals);
                Seq_addhi(currClosure->blocks, currClosure->block_vals); 
        }
}


/******************** get_Y1 *******************
 * Description:
 *    Gets the Y1 value and updates the P_avg_abcd struct by adding or 
 *    substracting Y1 to a,b,c,d.
 * 
 * Parameters:
 *   struct P_avg_abcd *block_vals:         struct with P_avg and abcd values
 *   struct P_and_Y_floats *currElem:       struct with P and Y floats
 * 
 * Returns:   Nothing but updates the P_avg_abcd struct to contain the 
 *           Y1 value.
 * 
 * Expects:
 *  The P_avg_abcd struct must not be NULL and the P_and_Y_floats struct
 *  must not be NULL.
 * Notes:
 *   updates the values in the block_vals struct
 * 
 *************************************************************/
void get_Y1(struct P_avg_abcd *block_vals, struct P_and_Y_floats *currElem)
{
        assert(block_vals != NULL);
        assert(currElem != NULL);
        /* initialize block_vals to update it later*/
        block_vals->Pb_avg = 0;
        block_vals->Pr_avg = 0;
        block_vals->a = 0; 
        block_vals->b = 0; 
        block_vals->c = 0; 
        block_vals->d = 0; 

        /*Computations with Y1*/
        block_vals->a += currElem->Y; 
        block_vals->b -= currElem->Y; 
        block_vals->c -= currElem->Y; 
        block_vals->d += currElem->Y; 

        /* GEtting the P values */
        block_vals->Pb_avg += currElem->Pb;
        block_vals->Pr_avg += currElem->Pr;
}


/******************** get_Y2 *******************
 * Description:
 *    Gets the Y2 value and updates the P_avg_abcd struct by adding or 
 *    substracting Y2 to a,b,c,d.
 * 
 * Parameters:
 *   struct P_avg_abcd *block_vals:         struct with P_avg and abcd values
 *   struct P_and_Y_floats *currElem:       struct with P and Y floats
 * 
 * Returns:   Nothing but updates the P_avg_abcd struct to contain the 
 *           Y2 value.
 * 
 * Expects:
 *  The P_avg_abcd struct must not be NULL and the P_and_Y_floats struct
 *  must not be NULL.
 * Notes:
 *   updates the values in the block_vals struct
 * 
 *************************************************************/
void get_Y2(struct P_avg_abcd *block_vals, struct P_and_Y_floats *currElem)
{
        assert(block_vals != NULL);
        assert(currElem != NULL);
        /* Getting the abcd value for the second blocks */
        block_vals->a += currElem->Y; 
        block_vals->b -= currElem->Y; 
        block_vals->c += currElem->Y; 
        block_vals->d -= currElem->Y;
        
        /*Pb2*/
        block_vals->Pb_avg += currElem->Pb;
        block_vals->Pr_avg += currElem->Pr;
}


/******************** get_Y3 *******************
 * Description:
 *    Gets the Y3 value and updates the P_avg_abcd struct by adding or 
 *    substracting Y3 to a,b,c,d.
 * 
 * Parameters:
 *   struct P_avg_abcd *block_vals:         struct with P_avg and abcd values
 *   struct P_and_Y_floats *currElem:       struct with P and Y floats
 * 
 * Returns:   Nothing but updates the P_avg_abcd struct to contain the 
 *           Y3 value.
 * 
 * Expects:
 *  The P_avg_abcd struct must not be NULL and the P_and_Y_floats struct
 *  must not be NULL.
 * Notes:
 *   updates the values in the block_vals struct
 * 
 *************************************************************/
void get_Y3(struct P_avg_abcd *block_vals, struct P_and_Y_floats *currElem)
{
        assert(block_vals != NULL);
        assert(currElem != NULL);
        /* Getting the abcd value for the third blocks */
        block_vals->a += currElem->Y; 
        block_vals->b += currElem->Y; 
        block_vals->c -= currElem->Y; 
        block_vals->d -= currElem->Y;

        /* Getting the P values */
        block_vals->Pb_avg += currElem->Pb;
        block_vals->Pr_avg += currElem->Pr;
}


/******************** get_Y4 *******************
 * Description:
 *    Gets the Y4 value and updates the P_avg_abcd struct by adding or 
 *    substracting Y4 to a,b,c,d.
 * 
 * Parameters:
 *   struct P_avg_abcd *block_vals:         struct with P_avg and abcd values
 *   struct P_and_Y_floats *currElem:       struct with P and Y floats
 * 
 * Returns:   Nothing but updates the P_avg_abcd struct to contain the 
 *           Y4 value.
 * 
 * Expects:
 *  The P_avg_abcd struct must not be NULL and the P_and_Y_floats struct
 *  must not be NULL.
 * Notes:
 *   updates the values in the block_vals struct
 * 
 *************************************************************/
void get_Y4(struct P_avg_abcd *block_vals, struct P_and_Y_floats *currElem)
{
        assert(block_vals != NULL);
        assert(currElem != NULL);
        /* Getting the abcd value for the fourth blocks */
        block_vals->a += currElem->Y; 
        block_vals->b += currElem->Y; 
        block_vals->c += currElem->Y; 
        block_vals->d += currElem->Y; 

        /* Getting the P values */
        block_vals->Pb_avg += currElem->Pb;
        block_vals->Pr_avg += currElem->Pr;
}


/********************** divide_four ***********************
 * Description:
 *    Divides the a, b, c, and d, Pb_avg, and Pr_avg values by 4.
 *    It then updates those variables with those values.
 * 
 * Parameters:
 *   struct P_avg_abcd *block_vals:        struct with P's avg & a,b,c,d floats
 * 
 * Returns:   Nothing but updates the P_avg_abcd struct to contain the
 *           average values.
 * Expects:
 *         The P_avg_abcd struct must not be NULL.
 * 
 * Notes:
 *       updates the block_vals struct with their new average values
 *  
 *************************************************************/
void divide_four (struct P_avg_abcd *block_vals)
{
        assert(block_vals != NULL);
        /* Getting the average values */
        block_vals->a = block_vals->a / BLOCK_ELEM;
        block_vals->b = block_vals->b / BLOCK_ELEM;
        block_vals->c = block_vals->c / BLOCK_ELEM;
        block_vals->d = block_vals->d / BLOCK_ELEM;
        block_vals->Pb_avg = block_vals->Pb_avg / BLOCK_ELEM;
        block_vals->Pr_avg = block_vals->Pr_avg / BLOCK_ELEM;
}


/*************************** roundFloats ************************
 * Description:
 *    Makes the float value num fit within the range if it is not within it.
 * 
 * Parameters:
 *   float num:            value representing the current float
 *   float lower:          the lowest the num can be 
 *   float upper:          the highest the num can be
 * 
 * Returns:   a float that is within the range specified by lower and upper
 * 
 * Expects:
 *  The lower bound must be less than the upper bound.
 * Notes:
 *  returns original value if it is within the range.
 * 
 *************************************************************/
float roundFloats(float num, float lower, float upper) 
{
        if (num < lower) {
                return lower;
        } else if (num > upper) {
                return upper;
        }
        return num;
}


/*************************** quantizeFloats ************************
 * Description:
 *    Quantizes the float values to the nearest integer value between
 *    -15 and 15
 * 
 * Parameters:
 *   float num:            current float value to quantize
 * 
 * Returns:   a quantized integer
 * 
 * Expects:
 *     The float value must not be NULL.
 * Notes:
 *     First makes sure that num is within the valid range.
 * 
 *************************************************************/
int quantizeFloats(float num)
{
        num = roundFloats(num, -BCD_RANGE, BCD_RANGE);
        int value = round(num * BCD_MAX);

        return value;
}


/********************** seq_to_words ***********************
 * Description:
 *    Converts the sequence of P_avg_abcd structs to a UArray of
 *    32-bit words.
 * 
 * Parameters:
 *   Seq_T blocks:         sequence of P_avg_abcd structs
 * 
 * Returns:   UArray_T which contains each word 
 * 
 * Expects:
 *      The sequence of P_avg_abcd structs must not be NULL.
 * Notes:
 *   Allocates memory for a new array of words. The memory for this array
 *   must be deallocated outside of this function
 * 
 *************************************************************/
UArray_T seq_to_words (Seq_T blocks)
{
        int size_array = Seq_length(blocks);
        UArray_T codeWords = UArray_new(size_array, sizeof(uint32_t));
        assert(codeWords != NULL);

        for (int i = 0; i < size_array; i++) {
                struct P_avg_abcd *curr_struct = Seq_get(blocks, i);
                uint32_t word = packWord(curr_struct);
                *(uint32_t *)UArray_at(codeWords, i) = word;
                FREE(curr_struct);  
        }
        Seq_free(&blocks);
        return codeWords;
}


/*************************** packWord ************************
 * Description:
 *    Packs the P_avg_abcd struct into a 32-bit word. It first rounds and
 *    quantizes the floats to either signed or unsigned. Then it packes them 
 *    into a 32-bit word.
 * 
 * Parameters:
 *   struct P_avg_abcd *curr_struct:        struct with P's avg and floats abcd
 * 
 * Returns:   a uint32_t word 
 * 
 * Expects:
 *     The P_avg_abcd struct must not be NULL.
 * Notes:
 *     it populates each word with the values in the P_avg_abcd struct
 * 
 *************************************************************/
uint32_t packWord(struct P_avg_abcd *curr_struct) 
{
        assert (curr_struct != NULL);
        uint32_t word = 0; /* initialize word */

        /* rounding the P and a values */
        curr_struct->Pb_avg = roundFloats(curr_struct->Pb_avg, -PS_RANGE, 
                                                                PS_RANGE); 
        curr_struct->Pr_avg = roundFloats(curr_struct->Pr_avg, -PS_RANGE, 
                                                                PS_RANGE);
        curr_struct->a = roundFloats(curr_struct->a, 0.0, 1.0);


        /* Quantizing  Pb_avg and Pr_avg */
        unsigned Pb_int = Arith40_index_of_chroma(curr_struct->Pb_avg);
        unsigned Pr_int = Arith40_index_of_chroma(curr_struct->Pr_avg);

        /* Quantizing a, b, c and d values */
        unsigned a_q = A_MAX * curr_struct->a;
        signed b_q = quantizeFloats(curr_struct->b);
        signed c_q = quantizeFloats(curr_struct->c);
        signed d_q = quantizeFloats(curr_struct->d);

        /* storing values in word */
        word = Bitpack_newu(word, PS_WIDTH, PS_LSB, Pr_int);
        word = Bitpack_newu(word, PS_WIDTH, PB_LSB, Pb_int);
        word = Bitpack_newu(word, A_WIDTH, A_LSB, a_q);
        word = Bitpack_news(word, BCD_WIDTH, B_LSB, b_q);
        word = Bitpack_news(word, BCD_WIDTH, C_LSB, c_q);
        word = Bitpack_news(word, BCD_WIDTH, D_LSB, d_q);

        return word;
}
