/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                  readaline.c
 * 
 * Assignment: FilesOfPix
 * Authors: Javier Laveaga {jlavea01} and Prince Millidah {pmilli03}
 * Date: January 21, 2024
 * 
 * Summary: This file contains the implementation for the readaline
 * function, which reads a line from a file and returns the number
 * of bytes read.
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/*****************************************************************
*                  
*                          readaline
*                  
*   This function reads a line from a file and returns the number
*   of bytes read. It also allocates memory for the line read.
*   If the file is empty, it returns 0.
*                  
*****************************************************************/
size_t readaline(FILE *inputfd , char **datapp) 
{
        /* Checking if input are null */
        assert(inputfd != NULL);
        assert(datapp != NULL);

        int num_bytes = 0; /* Number of bytes read */
        int curr_char = fgetc(inputfd); /* Current character */
        char char_version = curr_char; /* Character version of curr_char */

        if (curr_char == EOF) { /* Checking for empty file */
                *datapp = NULL;
                return 0;     
        }
        int currSize = 1000;

        /* Allocating memory and chackinf if it was successful */
        *datapp = malloc(currSize * sizeof(char));
        assert(*datapp); 

        /* Counting number of bytes until end of file or newline */
        while (curr_char != EOF) {
                char_version = curr_char;
                (*datapp)[num_bytes++] = char_version;
                
                if (char_version == '\n') {
                        break;
                }
                /* Reallocating memory when index equal current size */
                if (num_bytes == currSize) {
                        currSize = (currSize * 2) + 2;
                        *datapp = realloc(*datapp, currSize * sizeof(char));
                        assert(*datapp);
                }
                curr_char = fgetc(inputfd); /* Getting next character */
        }   
        return num_bytes;
}
