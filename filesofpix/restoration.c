/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                     restoration.c
 * 
 * Assignment: FilesOfPix
 * Authors: Javier Laveaga {jlavea01} and Prince Millidah {pmilli03}
 * Date: January 21, 2024
 * 
 * Summary: This file contains the main function for the converting 
 *          a corrupted pgm image into the raw format.
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
        #include "readaline.h"
#include <table.h>
#include <atom.h>
#include <seq.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/* Function declarations */

void findMatch(FILE *inputfd, Seq_T *mySeq, char **sequence);
int isDigit(char inputChar);
int splitLine(FILE *inputfd, char **new_key, char **new_value, int **arr_nums);
void getOriginal(FILE *inputfd, Seq_T *mySeq, char **match);
void printRaw(Seq_T *mySeq, int width);
int getWidth(char *valueString);
void run(FILE *inputfd);

/****************************** main *******************************
* 
* Returns EXIT_SUCCESS when the corrupted file has been converted properly
* without any problems. Calls the run function responsible for starting program
* 
* Parameters:
*          int argc: the number of arguments passed
*          int argv: the array containing the name of the arguments
*
* Return: EXIT_SUCCESS if program run properly else an expection is thrown
* Expects
*       int argc to be less or equal to 2 and an input file
* Notes:
*       If the number of arguments is 3 or greater, then the program will not 
*       run. 
*                  
*****************************************************************/
int main(int argc, char *argv[]) 
{
        assert(argc <= 2);
        FILE *userInput; /* File pointer */

        if (argc == 1) {
                userInput = stdin;
        } else {
                /* Opening the input file*/
                userInput = fopen(argv[1], "r");
                assert(userInput != NULL);
        }

        run(userInput); /* Running the program */
        fclose(userInput); /* Closing the file */
        return EXIT_SUCCESS; /* Exiting the program */
}


/************************** run ***********************************
*
* Return nothing. This function calls on the necessary functions to aid
* in execute th program                
* 
* Parameters:
*      FILE *inputfd: an pointer of the input file
*
* Return: Nothing
* Expects
*       The input pointer not be null
* Notes:
*       Will CRE if inputfd is null
*                        
*****************************************************************/
void run(FILE *inputfd) 
{
        Seq_T mySeq = Seq_new(200); /* declaring a sequence */

        char *match = NULL; /* Stores a match of char from the file */

        /* Finding matching keys in opened file*/
        findMatch(inputfd, &mySeq, &match); /* this populates first 2 lines */
        
        /* If sequence is still size 0, then either file is empty or no match 
        was found */
        int seq_len = Seq_length(mySeq);
        if (seq_len != 0) {
                char *first_line = Seq_get(mySeq, 0);
                /* Getting the width of the pgm. Gets width of the first 
                line because the width should be the same for all lines */
                int width = getWidth(first_line);

                /* Getting the original values */
                getOriginal(inputfd, &mySeq, &match); 

                /* Printing out raw format of the pgm */
                printRaw(&mySeq, width);
        }
        Seq_free(&mySeq); /* Freeing memory of sequence */
}


/*************************** isDigit *******************************
*  Given a character, it determines whether or not it is a digit. 
*  If it is a digit it reuturns 1 and 0 otherwise.
*
*  Parameters:
*            char inputChar: the character that is going to be read
* 
*  Returns: 1 if it is digit, 0 if not. 
*
*  Expects: 
*         It must be a character. 
*  
*  Notes:
*   The function checks if the ASCII value of the character is between
*   48 and 57, which is the range of ASCII values for digits 0-9.
*                  
*****************************************************************/
int isDigit(char inputChar) 
{
        /* Checking if the character is a digit */
        if (inputChar >= 48 && inputChar <= 57) {
                return 1;
        }
        return 0; 
}


/****************************** splitLine **************************      
*  The function reads a line from the file and splits it into a key and
*  a value. It returns the number of bytes read from line, and the length
* of the key and the value in an array of integers. 
*
* Parameters: 
*         FILE *inputfd:          a file pointer to read from
*       char **new_key:         a double pointer to a key
*       char **new_value:        a double pointer to a value
*       int **arr_nums:         a double pointer to an array of integers
*
* Return:
*   The number of bytes in a line, the length of the key and value in the 
*   array of integers (by reference), and the key and value (by reference). 
*
* Expects:
*      Inputfd must not be NULL. new_key, new_value, and arr_nums are NULL. 
* Notes:
*  Values are stored as characters separated by spaces. 
*  Allocates memory for the keys, values, and array which will store their 
*  respective sizes. 
*                  
*****************************************************************/
int splitLine(FILE *inputfd, char **new_key, char **new_value, int **arr_nums)
{
        int value_len = 0; /* Stores the size of the value array */
        int key_len = 0; /* Stores the size of the key array */
        
        char *datapp = NULL; /* Stores the line read from the file */
        int num_bytes = readaline(inputfd, &datapp); /*reading from file */

            /* Checking if the file is empty */
        if (num_bytes == 0) {
                return 0;
        }
            /* Allocating memory for key, values and length arrays */
        *new_key = malloc(num_bytes * sizeof(char));
        *new_value = malloc(num_bytes * sizeof(char));
        *arr_nums = malloc(sizeof(int) * 2);

        assert(*new_key); /* Checking if memory allocation was successful */
        assert(*new_value); /* Checking if memory allocation was successful */
        assert(*arr_nums); /* Checking if memory allocation was successful */

        char separator = ' '; /* Separator between values in value array */
        char curr_char; /* Current character being read from file */
        char prev_char = 'a'; /* Previous character read from file */
        for (int i = 0; i < num_bytes; i++) {
                curr_char = datapp[i];
                if (isDigit(curr_char) == 1) {
                        (*new_value)[value_len++] = curr_char; 
                        prev_char = curr_char;
                } else {
                        if (curr_char != '\n') {
                                (*new_key)[key_len++] = curr_char; 
                        }
                        if (isDigit(prev_char) == 1) {
                                /* Spaerating values in array */
                                (*new_value)[value_len++] = separator; 
                                prev_char = curr_char;
                        }
                }
        }
        free(datapp); /* Freeing memory of datapp */
        /* Setting the length of the key and value arrays */
        (*arr_nums)[0] = key_len;
        (*arr_nums)[1] = value_len;
        
        return num_bytes;
}


/************************* findMatch *******************************
*                  
* The function iterates and add keys to the declared table until 
* a key encountered already exists in the table. Stores the match key
* inside the sequence provided (our first line) as well the second.
* (current) line.                       
*                  
* Parameters:
*       FILE *inputfd:          a file pointer to read from
*       Seq_T *mySeq:           a pointer to a Hanson Sequence
*       char **sequence         a pointer to store the matching key
*
* Return: Nothing
* Expects:
*       Inputfd must not be NULL. char **sequence and Seq_T *mySeq can be NULL
* 
* Notes:
*       Will CRE if inputfd is NULL. Nothing occurs if empty file is passed.
*       Frees table once match is found. 
*                  
*****************************************************************/
void findMatch(FILE *inputfd, Seq_T *mySeq, char **sequence) 
{
        Table_T my_table = Table_new(200, NULL, NULL);

        char *key = NULL; /* Stores the key */
        char *value = NULL; /* Stores the value */
        int *arr_nums = NULL; /* Stores the length of the key and value */

        int matchFound = 0; /* Stores whether a match has been found */
        int num_bytes = 0; /* Stores the number of bytes read from the file */

        while (matchFound == 0) {  
                /* Spliting each line into a key and value */
                num_bytes = splitLine(inputfd, &key, &value, &arr_nums);
                if (num_bytes == 0) break;
                
                /* Creating atoms for the key and value */
                const char *keyString = Atom_new(key, arr_nums[0]);
                const char *valueString = Atom_new(value, arr_nums[1]);
                
                /* Checking if the key is already in the table */
                const char *match = Table_get(my_table, keyString);
                *sequence = (char*) keyString;
                
                if (match == NULL) {
                        /* Adding the key and value to the table */
                        Table_put(my_table, keyString, (void*)valueString);
                } else {
                        /* Adding the key and value to the sequence */
                        Seq_addhi(*mySeq, (void*)match);
                        Seq_addhi(*mySeq, (void*)valueString);
                        matchFound = 1;
                }
                /* Freeing allocated memory */
                free(arr_nums);
                free(key);
                free(value);   
        }
        Table_free(&my_table); /* Freeing table memory */ 
}


/************************** getOriginal ****************************
*                  
* The function iterates and gets remaining lines from the input file pointer
* and compares the key to our match from the findMatch function. The iteration
* continues until EOF. Nothing is returned.                       
*                  
* Parameters:
*       FILE *inputfd:          a file pointer to read from
*       Seq_T *mySeq:           a pointer to a Hanson Sequence
*       char **sequence         a pointer that store the matching key  
*
* Returns: Nothing
* Expects:
*       Inputfd and match must not be NULL. Seq_T *mySeq can be NULL
*       
* Notes:
*       Will CRE if inputfd is NULL. The sequence is populated with 
*       the original lines                 
*****************************************************************/
void getOriginal(FILE *inputfd, Seq_T *mySeq, char **match) 
{
        char *key = NULL; /* Stores the key */
        char *value = NULL; /* Stores the value */
        int *arr_nums = NULL; /* Stores the length of the key and value */
        
        /* Reading a line from the file and splitting it into a key and value*/
        int num_bytes = splitLine(inputfd, &key, &value, &arr_nums);

        /* Reading the rest of the file */
        while (num_bytes != 0) {
                /* Creating atoms for the key and value */
                const char *keyString = Atom_new(key, arr_nums[0]);
                const char *valueString = Atom_new(value, arr_nums[1]);

                /* Checking if current key is the same as the first match */
                if (keyString == *match) {
                        Seq_addhi(*mySeq, (void*)valueString);
                }

                /* Freeing allocated memory */
                free(arr_nums);
                free(key);
                free(value);
                num_bytes = splitLine(inputfd, &key, &value, &arr_nums);
        }
}

/***************************** printRaw ****************************
*                  
* The function prints out the raw format of the pgm after getting
* the height and using the width provided. It loop through the 
* sequence and prints out the corresponding ASCII characters.
*
* Parameters:
*       Seq_T *mySeq:      a pointer to a sequence where the lines are stored
*       int width:         the width of one of the lines
*
* Return: Nothing.  
* 
* Expects:
*       a pointer to a filled sequence and the width of each line, which 
*       should be the same for all lines. 
* Notes:
*   Prints the magic number for raw pgm files, the width, height, and max
*   number. It then loops through the sequence and prints out
*   the corresponding ASCII characters in the sequence.             
*****************************************************************/
void printRaw(Seq_T *mySeq, int width) 
{
        int height = Seq_length(*mySeq); /* Getting the height of the pgm */

        /* Printing pgm format, width, height and maximum value */
        printf("P%u\n", 5);
        printf("%d %d\n", width , height); 
        printf("%d\n", 255);

        char *currAtom; /* Stores the current atom */
        for (int i = 0; i < Seq_length(*mySeq); i++) {
                /* Getting the current atom */
                currAtom = (void*) Seq_get(*mySeq, i); 
                char currChar = currAtom[0]; 
                int charValue = 0;
                for (int j = 1; currChar != '\0'; j++) {
                        if (isDigit(currChar)) {
                                /* Converting character to integer */
                                charValue *= 10;
                                charValue += currChar - '0';
                        } else {
                                /* Printing ASCII character of integers */
                                printf("%c", charValue);
                                charValue = 0;
                        }
                        /* Getting the next character */
                        currChar = currAtom[j];
                }
        }
}


/************************** getWidth *******************************
*                  
* This function computes the width by counting the collections of chararcters 
* in the input character pointer. It does this by trying to convert the 
* characters to an integer and increment the counter once a space character is
* encountered. It returns the number of numbers counted.                    
*                  
* Parameters:
*      char *valueString:      a character pointer to a value array
*       
* Return: an integer that represent the width of the pgm
* 
* Expects:
*       valuestring pointer not to be NULL.
* Notes:  
*       If valueString is NULL, the function will return a width of zero
*****************************************************************/
int getWidth (char *valueString) 
{
        int width = 0; /* Stores the width of the pgm */
        char curr_char = ' '; /* Stores the current character */
        int index = 0; /* Stores the index of the current character */
        int currVal = 0; /* Stores the current integer value */

        /* Looping through the character */
        while (curr_char != '\0') {
                /* Getting the current character */
                curr_char = valueString[index++]; 
                if (curr_char == ' ') {
                        width++; /* Incrementing width for each integer */
                        currVal = 0;
                } else {
                        /* Converting character to integer */
                        currVal *= 10;
                        currVal += curr_char - '0';
                }
        }
        /* Incrementing width for the last integer */
        if (currVal > 0) {
                width++; 
        }
        return width;
}
