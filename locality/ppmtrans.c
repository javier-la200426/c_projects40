/**************************************************************
 *
 *                     ppmtrans.c
 *
 *     Assignment: Locality
 *     Authors:  Javier Laveaga (jlavea01), William Magann (wmagan01)
 *     Date:  2/22/2024
 *
 *     This file implements the rotations using the A2methods 
 *     interface. It also implements the other ones for extra 
 *     credit. Utilizes the polymorophism of the asMethods interface by 
 *     calling indirectly on the blocked or plain version of the interfaces 
 *     when the user specifies as such (with the type of major).
 *
 *    
 *
 **************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "uarray2.h"
#include "cputiming.h"

/************************** new_image struct **************************
*
* Struct that hold methods suite and rotated image array
* 
* Parameters:
*    
*       A2Methods_T methods:         The updated methods to use
*       A2Methods_UArray2 array_new: The new array where for rotated image
*                         
*****************************************************************/
struct new_image {
        A2Methods_T methods; 
        A2Methods_UArray2 array_new;
};

void rotate(Pnm_ppm ppmReader, A2Methods_T methods, A2Methods_mapfun *map, 
        bool timer_b, int rotation, FILE *timings_file, char *major);
void rotate_90(Pnm_ppm ppmReader, A2Methods_T methods, A2Methods_mapfun *map);
void apply_90(int col, int row, A2Methods_UArray2 array2,
                                A2Methods_Object *ptr, void *cl);
void apply_180(int col, int row, A2Methods_UArray2 array2,
                                A2Methods_Object *ptr, void *cl);
void apply_270(int col, int row, A2Methods_UArray2 array2,
                                A2Methods_Object *ptr, void *cl);
void apply_horitzontal(int col, int row, A2Methods_UArray2 array2,
                                A2Methods_Object *ptr, void *cl);
void apply_vertical(int col, int row, A2Methods_UArray2 array2,
                                A2Methods_Object *ptr, void *cl);
void apply_transpose(int col, int row, A2Methods_UArray2 array2,
                                A2Methods_Object *ptr, void *cl);
void free_old_arr(Pnm_ppm ppmReader, A2Methods_T methods,
                  A2Methods_UArray2 new_array);


#define SET_METHODS(METHODS, MAP, WHAT) do {                    \
        methods = (METHODS);                                    \
        assert(methods != NULL);                                \
        map = methods->MAP;                                     \
        if (map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n",               \
                                argv[0]);                       \
                exit(1);                                        \
        }                                                       \
} while (false)

static void
usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] "
                        "[-time time_file] "
                        "[filename]\n",
                        progname);
        exit(1);
}

int main(int argc, char *argv[])
{
        char *time_file_name = NULL;
        int   rotation       = 0;
        int   i;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods != NULL);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);

        char major[100]; /* Char pointer for what map major*/
        strcpy(major, "row-major");

        bool file_provided = false; /* boolean for if file in command line*/

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_col_major, 
                                    "column-major");
                        strcpy(major, "column-major");
                } else if (strcmp(argv[i], "-block-major") == 0) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                        strcpy(major, "block-major");
                } else if (strcmp(argv[i], "-rotate") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        char *endptr;
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90 ||
                            rotation == 180 || rotation == 270)) {
                                fprintf(stderr, 
                                        "Rotation must be 0, 90 180 or 270\n");
                                usage(argv[0]);
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }
                } else if (strcmp(argv[i], "-flip") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        if(strcmp(argv[++i], "horizontal") == 0) {
                                /*Horizontal Flip given abritrary int of 100*/
                                rotation = 100;  
                        } else if(strcmp(argv[i], "vertical") == 0) {
                                /*Vertical Flip given abritrary int of 200*/
                                rotation = 200;
                        } else {
                                fprintf(stderr, 
                                "Flip must be horizontal or vertical\n");
                                usage(argv[0]);   
                        }   
                } else if (strcmp(argv[i], "-transpose") == 0) {
                        rotation = 300;
                }
                else if (strcmp(argv[i], "-time") == 0) {
                        if (!(i + 1 < argc)) {      /* no time file */
                                usage(argv[0]);
                        }
                        time_file_name = argv[++i];
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
                                argv[i]);
                        usage(argv[0]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {
                        if (argc - i == 1) { /* if file provided*/
                                file_provided = true;
                        }
                        break;
                }
                
        }

        FILE *fp; /*File pointer*/

        if (file_provided == true) {
                fp = fopen(argv[argc - 1], "r");
        } else {
                fp = stdin;
        }

        
        if (fp == NULL) {
                fprintf(stderr, "Invalid File Name\n");
                exit(EXIT_FAILURE);
        }

        bool timer_b = true; /* If -time option used*/ 
        if (time_file_name == NULL) {
                timer_b = false;
        } 
        /*initialize reader with file_p and updated methods*/
        Pnm_ppm ppmReader = Pnm_ppmread(fp, methods);
        
        /* Opening timing file */
        FILE *timings_file = NULL;
        if (timer_b == true) {
                timings_file = fopen(time_file_name, "a");
                if (timings_file == NULL && timer_b == true) {
                        fprintf(stderr, "Invalid Time file\n");
                        exit(EXIT_FAILURE);
                }
        }
        rotate(ppmReader, methods, map, timer_b, rotation, 
                timings_file, major);
        /*Printing new rotated image to stdout*/
        Pnm_ppmwrite(stdout, ppmReader); 
        if (timer_b == true) {
                fclose(timings_file);
        }
        fclose(fp);
        Pnm_ppmfree(&ppmReader);
        return EXIT_SUCCESS;
}

/********** rotatate ********
 *
 * calls the correct apply function to rotate the image based
 * on the client provided rotation
 *
 * Parameters:
 *      Pnm_ppm ppmReader:     struct of contents of input file
 *      A2Methods_T methods:   methods suite
 *      A2Methods_mapfun *map: function pointer for which mapping fucntion
 *      bool timer_b:          boolean for if timer option selected
 *      int rotation:          int of what transformation user selected
 *      FILE *timings_file:    output file for timer writes
 *      char *major:           char array of what map major selected
 *      
 * Return: nothing
 *
 * Expects:
 *      ppmReader, methods, and *map will not be NULL
 * Notes:
 *      checked runtime error if mentioned variables are NULL
 *      Allocates and dealloates heap memory for struct
 *      
 ************************/
void rotate(Pnm_ppm ppmReader, A2Methods_T methods, A2Methods_mapfun *map,
        bool timer_b, int rotation, FILE *timings_file, char *major) 
{
        assert(ppmReader != NULL);
        assert(methods != NULL);
        assert(map != NULL);

        /* Initializes timing file and needed variables */
        (void) timings_file;
        CPUTime_T timer;
        timer = CPUTime_New(); /* Freed at end of function*/
        double time_used;
        
        /* Retrieve height width ands size of */
        int width = ppmReader->width;
        int height = ppmReader->height;
        int size = methods->size(ppmReader->pixels);

        /* initialize new_image struct */                     
        struct new_image* transform = (struct new_image*)malloc
                (sizeof(struct new_image)); /* Freed at end of function*/

        assert(transform != NULL);

        transform->methods = methods;

        if (rotation == 90) {
                A2Methods_UArray2 new_array = methods->new(height, 
                                                           width, size);
                transform->array_new = new_array;
                if (timer_b == true){
                        CPUTime_Start(timer);
                }
                (map)(ppmReader->pixels, apply_90, transform);
                if (timer_b == true){
                        time_used = CPUTime_Stop(timer);
                }
                
                free_old_arr(ppmReader, methods, new_array);
                ppmReader->width = height;
                ppmReader->height = width;
        }
        else if (rotation == 180) { /*no need to update width and height*/
                A2Methods_UArray2 new_array = methods->new(width, 
                                                           height, size);
                transform->array_new = new_array;
                if (timer_b == true){
                        CPUTime_Start(timer);
                }
                (map)(ppmReader->pixels, apply_180, transform);
                if (timer_b == true){
                        time_used = CPUTime_Stop(timer);
                }
                free_old_arr(ppmReader, methods, new_array);      
        } 
        else if (rotation == 270) {
                A2Methods_UArray2 new_array = methods->new(height,
                                                           width, size);
                transform->array_new = new_array;
                if (timer_b == true){
                        CPUTime_Start(timer);
                }
                (map)(ppmReader->pixels, apply_270, transform);
                if (timer_b == true){
                        time_used = CPUTime_Stop(timer);
                }
                
                free_old_arr(ppmReader, methods, new_array);
                ppmReader->width = height;
                ppmReader->height = width;
        } 
        else if (rotation == 100) { /* When -flip horizontal is called */
                A2Methods_UArray2 new_array = methods->new(width,
                                                           height, size);
                transform->array_new = new_array;
                if (timer_b == true){
                        CPUTime_Start(timer);
                }
                (map)(ppmReader->pixels, apply_horitzontal, transform);
                if (timer_b == true){
                        time_used = CPUTime_Stop(timer);
                }
                free_old_arr(ppmReader, methods, new_array);     
        } 
        else if (rotation == 200) { /* Vertical flip*/
                A2Methods_UArray2 new_array = methods->new(width, 
                                                           height, size);
                transform->array_new = new_array;
                if (timer_b == true){
                        CPUTime_Start(timer);
                }
                (map)(ppmReader->pixels, apply_vertical, transform);
                if (timer_b == true){
                        time_used = CPUTime_Stop(timer);
                }
                free_old_arr(ppmReader, methods, new_array);      
        } 
        else if (rotation == 300) { /* Transpose */
                A2Methods_UArray2 new_array = methods->new(height,
                                                           width, size);
                transform->array_new = new_array;
                if (timer_b == true){
                        CPUTime_Start(timer);
                }
                (map)(ppmReader->pixels, apply_transpose, transform);
                if (timer_b == true){
                        time_used = CPUTime_Stop(timer);
                }
                free_old_arr(ppmReader, methods, new_array);
                ppmReader->width = height;
                ppmReader->height = width;   
        }
        else { /* not rotation specified or -rotation 0*/
                if (timer_b == true){
                        CPUTime_Start(timer);
                        time_used = CPUTime_Stop(timer);
                }
        }
        /*Time in nanoseconds, time per pixel, rotation, map major */
        if (timer_b == true) {
                assert(timings_file != NULL);
                fprintf(timings_file, "Width: %d, Height: %d\n", width,
                        height);
                fprintf(timings_file, "Rotation: %d\n", rotation);
                fprintf(timings_file, "Major: %s\n", major);

                double tPerPix = time_used / (width * height);
                fprintf(timings_file, "Instructions per pixel: %f\n", tPerPix);
                fprintf(timings_file, "time (ns): %f\n", time_used);
                fprintf(timings_file, "time per pixel (ns): %f\n\n", tPerPix);
                
        }

        /* Frees timer and struct */
        CPUTime_Free(&timer);
        free(transform);      
}

/********** apply_90 ********
 *
 * Apply function that rotates image 90 degrees
 *
 * Parameters:
 *      int col:                   column of original pixel
 *      int row:                   row of original pixel
 *      A2Methods_UArray2 array2:  original 2D array
 *      A2Methods_Object *ptr:     pointer to orignal 2D array element
 *      void *cl:                  void pointer used here for struct of
 *                                 with methods suite and new 2D array
 *      
 * Return: nothing
 *
 * Expects:
 *      array2, *ptr, and cl do not eqaul NULL
 * Notes:
 *      checked runtime error mentioned element NULL
 *      
 ************************/
void apply_90(int col, int row, A2Methods_UArray2 array2,
                                A2Methods_Object *ptr, void *cl)
{
        assert(array2 != NULL);
        assert(ptr != NULL);
        assert (cl != NULL);
        /*casting this to pnm_rgb, our current element*/
        struct Pnm_rgb* element = (struct Pnm_rgb*)ptr;

        /* Casts cl to new_image struct*/
        struct new_image *transform = (struct new_image*) cl;
        
        /*Accessing methods (updated from main) and new_arr from struct*/
        A2Methods_T my_methods = transform->methods;
        A2Methods_UArray2 new_array = transform->array_new;

        /* Getting height of array */
        int height = my_methods->height(array2);

        /*Calculate new position of pixel for new array*/
        int new_col = height - row - 1;
        int new_row = col;

        /*Getting the new position of curr_element from new array */
        struct Pnm_rgb *curr_new = my_methods->at(new_array, new_col, new_row);
        *curr_new = *element; /*Updating that position with old element*/
}

/********** apply_180 ********
 *
 * Apply function that rotates image 180 degrees
 *
 * Parameters:
 *      int col:                   column of original pixel
 *      int row:                   row of original pixel
 *      A2Methods_UArray2 array2:  original 2D array
 *      A2Methods_Object *ptr:     pointer to orignal 2D array element
 *      void *cl:                  void pointer used here for struct of
 *                                 with methods suite and new 2D array
 *      
 * Return: nothing
 *
 * Expects:
 *      array2, *ptr, and cl do not eqaul NULL
 * Notes:
 *      checked runtime error mentioned element NULL
 *      
 ************************/
void apply_180(int col, int row, A2Methods_UArray2 array2,
                                A2Methods_Object *ptr, void *cl)
{
        assert(array2 != NULL);
        assert(ptr != NULL);
        assert(cl != NULL);
        struct Pnm_rgb* element = (struct Pnm_rgb*)ptr;

        /* Casts cl to new_image struct*/
        struct new_image *transform = (struct new_image*) cl;

        /*Accessing methods (updated from main) and new_arr from struct*/
        A2Methods_T my_methods = transform->methods;
        A2Methods_UArray2 new_array = transform->array_new;

        /* Getting height of array */
        int height = my_methods->height(array2);
        int width = my_methods->width(array2);

        /* New Inidces of elements*/
        int new_col = width - col - 1;
        int new_row = height - row - 1;

        /*Getting the new position of curr_element from new array */
        struct Pnm_rgb *curr_new = my_methods->at(new_array, new_col, new_row);
        *curr_new = *element; /*Updating that position with old element*/     
}

/********** apply_270 ********
 *
 * Apply function that rotates image 270 degrees
 *
 * Parameters:
 *      int col:                   column of original pixel
 *      int row:                   row of original pixel
 *      A2Methods_UArray2 array2:  original 2D array
 *      A2Methods_Object *ptr:     pointer to orignal 2D array element
 *      void *cl:                  void pointer used here for struct of
 *                                 with methods suite and new 2D array
 *      
 * Return: nothing
 *
 * Expects:
 *      array2, *ptr, and cl do not eqaul NULL
 * Notes:
 *      checked runtime error mentioned element NULL
 *      
 ************************/
void apply_270(int col, int row, A2Methods_UArray2 array2,
                                A2Methods_Object *ptr, void *cl)
{
        assert(array2 != NULL);
        assert(ptr != NULL);
        assert(cl != NULL);
        /*casting this to pnm_rgb, our current element*/
        struct Pnm_rgb* element = (struct Pnm_rgb*)ptr;

        /* Casts cl to new_image struct*/
        struct new_image *transform = (struct new_image*) cl;
        
        /*Accessing methods (updated from main) and new_arr from struct*/
        A2Methods_T my_methods = transform->methods;
        A2Methods_UArray2 new_array = transform->array_new;

        /* Getting width of array */
        int width = my_methods->width(array2);

        /*Calculate new position of pixel for new array*/
        int new_col = row;
        int new_row = width - col - 1;

        /*Getting the new position of curr_element from new array */
        struct Pnm_rgb *curr_new = my_methods->at(new_array, new_col, new_row);
        *curr_new = *element; /*Updating that position with old element*/
}

/********** apply_horiztonal ********
 *
 * Apply function that flips image horizontally
 *
 * Parameters:
 *      int col:                   column of original pixel
 *      int row:                   row of original pixel
 *      A2Methods_UArray2 array2:  original 2D array
 *      A2Methods_Object *ptr:     pointer to orignal 2D array element
 *      void *cl:                  void pointer used here for struct of
 *                                 with methods suite and new 2D array
 *      
 * Return: nothing
 *
 * Expects:
 *      array2, *ptr, and cl do not eqaul NULL
 * Notes:
 *      checked runtime error mentioned element NULL
 *      
 ************************/
void apply_horitzontal(int col, int row, A2Methods_UArray2 array2,
                                A2Methods_Object *ptr, void *cl)
{
        assert(array2 != NULL);
        assert(ptr != NULL);
        assert(cl != NULL);
        struct Pnm_rgb* element = (struct Pnm_rgb*)ptr;

        /* Casts cl to new_image struct*/
        struct new_image *transform = (struct new_image*) cl;

        /*Accessing methods (updated from main) and new_arr from struct*/
        A2Methods_T my_methods = transform->methods;
        A2Methods_UArray2 new_array = transform->array_new;

        /* Getting width of array */
        int width = my_methods->width(array2);

        /* New Inidces of elements*/
        int new_col = width - col - 1;

        /*Getting the new position of curr_element from new array */
        struct Pnm_rgb *curr_new = my_methods->at(new_array, new_col, row);
        *curr_new = *element; /*Updating that position with old element*/     
}

/********** apply_vertical ********
 *
 * Apply function that flips image vertically
 *
 * Parameters:
 *      int col:                   column of original pixel
 *      int row:                   row of original pixel
 *      A2Methods_UArray2 array2:  original 2D array
 *      A2Methods_Object *ptr:     pointer to orignal 2D array element
 *      void *cl:                  void pointer used here for struct of
 *                                 with methods suite and new 2D array
 *      
 * Return: nothing
 *
 * Expects:
 *      array2, *ptr, and cl do not eqaul NULL
 * Notes:
 *      checked runtime error mentioned element NULL
 *      
 ************************/
void apply_vertical(int col, int row, A2Methods_UArray2 array2,
                                A2Methods_Object *ptr, void *cl)
{
        assert(array2 != NULL);
        assert(ptr != NULL);
        assert(cl != NULL);
        struct Pnm_rgb* element = (struct Pnm_rgb*)ptr;

        /* Casts cl to new_image struct*/
        struct new_image *transform = (struct new_image*) cl;

        /*Accessing methods (updated from main) and new_arr from struct*/
        A2Methods_T my_methods = transform->methods;
        A2Methods_UArray2 new_array = transform->array_new;

        /* Getting height of array */
        int height = my_methods->height(array2);

        /* New Inidces of elements*/
        int new_row = height - row - 1;

        /*Getting the new position of curr_element from new array */
        struct Pnm_rgb *curr_new = my_methods->at(new_array, col, new_row);
        *curr_new = *element; /*Updating that position with old element*/     
}

/********** apply_transpore ********
 *
 * Apply function that flips image across y = x
 *
 * Parameters:
 *      int col:                   column of original pixel
 *      int row:                   row of original pixel
 *      A2Methods_UArray2 array2:  original 2D array
 *      A2Methods_Object *ptr:     pointer to orignal 2D array element
 *      void *cl:                  void pointer used here for struct of
 *                                 with methods suite and new 2D array
 *      
 * Return: nothing
 *
 * Expects:
 *      array2, *ptr, and cl do not eqaul NULL
 * Notes:
 *      checked runtime error mentioned element NULL
 *      
 ************************/
void apply_transpose(int col, int row, A2Methods_UArray2 array2,
                                A2Methods_Object *ptr, void *cl)
{
        assert(array2 != NULL);
        assert(ptr != NULL);
        assert(cl != NULL);
        struct Pnm_rgb* element = (struct Pnm_rgb*)ptr;

        /* Casts cl to new_image struct*/
        struct new_image *transform = (struct new_image*) cl;

        /*Accessing methods (updated from main) and new_arr from struct*/
        A2Methods_T my_methods = transform->methods;
        A2Methods_UArray2 new_array = transform->array_new;

        (void)array2;

        /* New Inidces of elements*/
        int new_row = col;
        int new_col = row;

        /*Getting the new position of curr_element from new array */
        struct Pnm_rgb *curr_new = my_methods->at(new_array, new_col, new_row);
        *curr_new = *element; /*Updating that position with old element*/     
}

/********** free_old_arr ********
 *
 * Given a pnm_ppm, methods, and a new initialized array, it will free the 
 * old array held by pnm_ppm using methods and update it with the new array 
 *
 * Parameters:
 *      Pnm_ppm ppmReader:         The reader that the rotation is working with
 *      A2Methods_T methods:       The updated methods used by rotation
 *      A2Methods_UArray2 new_array  The new array to be be assigned to ppm
 *      
 * Return: nothing
 *
 * Expects:
 *      An initialized methods and new_array and an old array
 * Notes:
 *      checked runtime error mentioned when any parameters are NULL
 *      
 ************************/
void free_old_arr(Pnm_ppm ppmReader, A2Methods_T methods, 
                  A2Methods_UArray2 new_array) 
{
        assert(ppmReader != NULL);
        assert(methods != NULL);
        assert(new_array != NULL);

        /*Saving old array before updating it to the new*/
        A2Methods_UArray2 old_array = ppmReader->pixels;
        ppmReader->pixels = new_array;

        /*freeing old array*/
        methods->free(&old_array);
}




