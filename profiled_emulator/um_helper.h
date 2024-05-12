/*************************************************************************
 *
 *                     um_helper.h
 *
 *     Assignment: UM
 *     Authors:  Eva Caro (ecaro01) and Javier Laveaga (jlavea01)
 *     Date:     4/11/24
 *
 *     Declares all client-accessible functions that facilitate the 
 *     functionality of the UM. The client can create a new UM and run the 
 *     UM on the instructions in a file of their own.
 *
 *************************************************************************/

#ifndef UM_HELPER
#define UM_HELPER

#include "UM.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include <sys/stat.h>

#define WRDLNGTH 4

void *UM_new();
void UM_free(UM my_um);
void *read_file(UM my_um, FILE *infile, long total_bytes);
void run(UM my_um);


#endif