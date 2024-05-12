#include <stdio.h>
#include <stdlib.h>
#include "pnm.h"
#include <a2plain.h>
#include "a2methods.h"
#include "uarray2.h"
#include "assert.h"
#include "mem.h"
#include "math.h"

int compareDimension(Pnm_ppm original, Pnm_ppm compressed);
void iterate_array(Pnm_ppm Reader_o, Pnm_ppm Reader_c, A2Methods_T methods);
void apply_fun(int col, int row, A2Methods_UArray2 compressed, 
                A2Methods_Object *ptr, void *cl);

struct closure {
    A2Methods_UArray2 original;
    A2Methods_T methods;
    float sumSquared;
};

int main(int argc, char *argv[])
{
    FILE *original, *compressed;
    if (argc != 3){
        fprintf(stderr, " Too many or too few arguments\n");
        exit(EXIT_FAILURE);
    } 

    original = fopen(argv[1], "r");
    if (original == NULL) {
        fprintf(stderr, "Invalid File Name\n");
        exit(EXIT_FAILURE);
    }

    compressed = fopen(argv[2], "r");
    if (compressed == NULL) {
        fprintf(stderr, "Invalid File Name\n");
        exit(EXIT_FAILURE);
    }

    A2Methods_T methods = uarray2_methods_plain;
    Pnm_ppm Reader_o = Pnm_ppmread(original, methods);

    Pnm_ppm Reader_c = Pnm_ppmread(compressed, methods);
    int compVal = compareDimension(Reader_o, Reader_c);

    int width_o = Reader_o->width;
    int height_o = Reader_o->height;
    int width_c = Reader_c->width;
    int height_c = Reader_c->height;

    /*Smaller image should be second argument in iterate array*/
    if(compVal == 0 ) {
        if (width_o <= width_c && height_o <= height_c) {
            iterate_array(Reader_c, Reader_o, methods);
        } else {
            iterate_array(Reader_o, Reader_c, methods);
        }
    }
    Pnm_ppmfree(&Reader_o);
    Pnm_ppmfree(&Reader_c);
    fclose(original);
    fclose(compressed);

    return EXIT_SUCCESS; /*Do we have to do this*/
}

/*TODO:Define small apply*/
void apply_fun(int col, int row, A2Methods_UArray2 compressed, 
                A2Methods_Object *ptr, void *cl) 
{
    (void)compressed;
    struct closure *curr = cl;
    A2Methods_UArray2 origArray = curr->original;
    A2Methods_T methods = curr->methods;
    
    struct Pnm_rgb* compPixel = ptr;
    struct Pnm_rgb* origPixel = methods->at(origArray, col, row);

    /*Casting to int so that difference can be negative*/
    int redDiff = (int)origPixel->red - (int)compPixel->red;
    int greenDiff = (int)origPixel->green - (int)compPixel->green; 
    int blueDiff = (int)origPixel->blue - (int)compPixel->blue;

    curr->sumSquared += pow(redDiff , 2);
    curr->sumSquared += pow(greenDiff, 2);
    curr->sumSquared += pow(blueDiff, 2);
    
}

void iterate_array(Pnm_ppm Reader_o, Pnm_ppm Reader_c, A2Methods_T methods) 
{
    assert(Reader_o != NULL);
    assert(Reader_c != NULL);
    assert(methods != NULL);
    A2Methods_UArray2 array_o = Reader_o->pixels;
    A2Methods_UArray2 array_c = Reader_c->pixels;
    (void) array_o;
    (void) array_c;

    /*TODO: Use NEW instead of malloc*/
    struct closure* info;
    NEW(info);
    info->methods = methods;
    info->original = array_o;
    info->sumSquared = 0;
    /*Compressed file is always smaller, so we use its width and height*/
    /*TODO: Which height dowe use?*/
    methods->map_default(array_c, apply_fun, info);

    info->sumSquared  = info->sumSquared / 
                        (3 * Reader_c->width * Reader_c->height);
    info->sumSquared =  sqrt(info->sumSquared); 
    info->sumSquared = info->sumSquared / Reader_c->denominator;;
    /*Multiply SQRT part of formula by 1/255 at the end*/
    printf("Final result: %.4f\n", info->sumSquared);
    FREE(info);

}

int compareDimension(Pnm_ppm original, Pnm_ppm compressed) 
{

    if ((int)original->height - (int)compressed->height > 1) {
            fprintf(stderr, "height differ by more than one\n");
            printf("%.1f\n", 1.0);
            return 1;
            
    }
    if ((int)original->width - (int)compressed->width > 1) {
            fprintf(stderr, "width differ by more than one\n");
            printf("%.1f\n", 1.0);
            return 1;
    }
    return 0;
}
