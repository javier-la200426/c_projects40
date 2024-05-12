/*************************************************************************
 *
 *                     univmach.c
 *
 *     Assignment: UM
 *     Authors:  Eva Caro and Javier Laveaga (jlavea01)
 *     Date:     4/11/24
 *
 *     File containing the main function which reads in a file with 
 *     instructions and runs the um program, executing those instructions.
 *
 *************************************************************************/

#include "um_helper.h"

/********** main ********
 *
 * Runs the instructions found in the provided file using a Universal Machine.
 *
 * Parameters:
 *      int argc:         the number of arguments provided on the command line
 *      char *argv[]:     a string array containing the provided arguments
 *
 * Return: 0
 *
 * Expects
 *      Exactly two arguments given, the second being the name of a file 
 *      containing instruction words (uint32_t's) that follow the below pattern
 *      ABCD XXXXXXXXXXXXXXXXXXX TUV LMN IJK
 *      where ABCD is the instruction code (0 to 12), TUV is A, LMN is B, IJK
 *      is C, and the string of X's is a (possibly arbitrary) value. If the
 *      intended instruction is a load_val (13), then B and C are omitted:
 *      ABCD TUV XXXXXXXXXXXXXXXXXXXXXXXXX
 * 
 * Notes:
 *      Simply returns 0 if number of arguments provided is not 2 or the file
 *      cannot be opened.
 * 
 ************************/
int main(int argc, char *argv[])
{
        FILE *infile;
        if (argc != 2) {
                fprintf(stderr, "Incorrect number of arguments\n");
                exit(EXIT_FAILURE);
        } else {
                infile = fopen(argv[1], "rb");
                if (infile == NULL) { 
                        fprintf(stderr, "%s: No such file or directory\n", argv[1]);
                        exit(EXIT_FAILURE);
                }
        }
        struct stat *statbuf = malloc(sizeof(struct stat));
        stat(argv[1], statbuf);
        long total_bytes = statbuf->st_size; 
        UM my_um = UM_new();
        my_um = read_file(my_um, infile, total_bytes);
        run(my_um);

        UM_free(my_um);
        free(statbuf);
        fclose(infile);

        return 0;
}