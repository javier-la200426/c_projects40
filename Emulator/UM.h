/*************************************************************************
 *
 *                     UM.h
 *
 *     Assignment: UM
 *     Authors:  Eva Caro (ecaro01) and Javier Laveaga (jlavea01)
 *     Date:     4/11/24
 *
 *     Defines the UM struct, which operates as a Universal Machine interface.
 *
 *************************************************************************/
#include <seq.h>
#include "bitpack.h"



typedef struct UM {
    Seq_T segments;
    long counter;
    Seq_T unmapped;
    uint32_t reg[8];
} *UM;



