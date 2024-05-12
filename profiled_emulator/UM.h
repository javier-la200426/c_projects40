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
#include <uarray.h>
#include "bitpack.h"


struct segment {
        uint32_t *seg;
        long length;
};
typedef struct UM {
        long num_segs; /*segments that have been mapped*/
        long segments_size; /*size of array*/
        long counter;
        Seq_T unmapped;
        uint32_t length_seg0;
        //struct segment *segments;
} *UM;



