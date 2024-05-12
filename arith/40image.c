/*************************** 40image.c **********************
 *                         
 *     Authors: Javier Laveaga {jlavea01} and Prince Millidah {pmilli03}, 
 *     Date: 2/27/2024
 *     Assignment: arith
 *
 *     Summary: This is the main which reades in the arguments and calls 
 *     either compressor or decompressor based on the user input. It also
 *     determines whether to pass in standard input of a file pointer.
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

static void (*compress_or_decompress)(FILE *input) = compress40;

/******************** main *******************
 * Description:
 *    This function compresses or decompresses the input file 
 *    based on the user input (-c or -d)
 * 
 * Parameters:
 *   int argc:         the number of input passed to the function 
 *   char *argv:       an array of the input passed to the function
 * 
 * Returns:
 *      EXIT_SUCCESS if the program successfully executes, otherwise 
 *      return 1;
 * 
 * Expects:
 *      The input to include a command of format -c or -d
 *
 * Notes:
 *      Will throw an error if c or d does not follow - or 
 *      input doesn't include -c or -d
 * 
 *************************************************************/
int main(int argc, char *argv[])
{
        int i;

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-c") == 0) {
                        compress_or_decompress = compress40;
                } else if (strcmp(argv[i], "-d") == 0) {
                        compress_or_decompress = decompress40;
                } else if (*argv[i] == '-') {
                        /* Throwing error when input command is wrong */
                        fprintf(stderr, "%s: unknown option '%s'\n",
                                argv[0], argv[i]);
                        exit(1);
                } else if (argc - i > 2) {
                        /* Throwing error when input command is wrong */
                        fprintf(stderr, "Usage: %s -d [filename]\n"
                                "       %s -c [filename]\n",
                                argv[0], argv[0]);
                        exit(1);
                } else {
                        break;
                }
        }
        assert(argc - i <= 1);    /* at most one file on command line */

        /* Getting file input from terminal */
        if (i < argc) {
                FILE *fp = fopen(argv[i], "r");
                assert(fp != NULL);

                /* Executing command */
                compress_or_decompress(fp);
                fclose(fp);
        } else {
                /*Getting input from the standard input and executing command*/
                compress_or_decompress(stdin);
        }

        return EXIT_SUCCESS; 
}


