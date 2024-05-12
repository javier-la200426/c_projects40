/*************************************************************************
 *
 *                     umlabwrite.c
 *
 *     Assignment: UM
 *     Authors:  Eva Caro and Javier Laveaga (jlavea01)
 *     Date:     4/11/24
 *
 *     File responsible for building tests as well as expected input and 
 *     output files used for those tests. 
 *
 *************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "fmt.h"
#include "seq.h"

extern void Um_write_sequence(FILE *output, Seq_T instructions);

extern void build_halt_test(Seq_T instructions);
extern void build_verbose_halt_test(Seq_T instructions);
extern void build_add_test(Seq_T stream);
extern void build_print_test(Seq_T stream);
void build_conditional_test(Seq_T stream);
void build_cond_test_0(Seq_T stream);
void test_seg_store(Seq_T stream);
void build_multiply_test(Seq_T stream);
void build_division_test(Seq_T stream);
void build_nand_test(Seq_T stream);
void test_map_segment(Seq_T stream);
void test_unmap_segment(Seq_T stream);
void test_unmap_segment_2(Seq_T stream);
void test_output( Seq_T stream);
void test_load_program(Seq_T stream);
void test_seg_load2(Seq_T stream);
void test_seg_store2(Seq_T stream);
void test_output_bad(Seq_T stream);
void test_input_output(Seq_T stream);
void small_nand_test(Seq_T stream);
void test_load_val (Seq_T stream);
void test_load_program3(Seq_T stream);
void advanced_nand (Seq_T stream);
void test_time (Seq_T stream);

/* The array `tests` contains all unit tests for the lab. */

static struct test_info {
        const char *name;
        const char *test_input;          /* NULL means no input needed */
        const char *expected_output;
        /* writes instructions into sequence */
        void (*build_test)(Seq_T stream);
} tests[] = {
        { "halt",         NULL, "", build_halt_test },
        { "halt-verbose", NULL, "B", build_verbose_halt_test },
        { "add",          NULL, "0/_", build_add_test },
        { "print-six",    NULL, "6",  build_print_test },
        { "condNOT0",     NULL, "e",  build_conditional_test },
        { "cond0",        NULL, "ee",  build_cond_test_0 },
        { "seg_store",    NULL, "A", test_seg_store },
        { "multiply",     NULL, "", build_multiply_test },
        { "div",          NULL, "", build_division_test },
        { "nand",         NULL, "", build_nand_test },
        { "mapseg",       NULL, "", test_map_segment },
        { "unmapseg",     NULL, "", test_unmap_segment },
        { "unmapseg2",    NULL, "", test_unmap_segment_2 },
        { "output",       NULL, "Q", test_output },
        { "load_program", NULL, "", test_load_program },
        { "segstore2",    NULL, "E", test_seg_store2 },
        { "seg_load2",    NULL, "", test_seg_load2 }, /*do this*/
        { "output_bad",   NULL, "", test_output_bad }, /*do this*/
        { "input_output", "F", "F", test_input_output}, /*input 70*/
        { "small_nand",   NULL, "B", small_nand_test},
        { "load_val",     NULL, "W^/3b@A+", test_load_val},
        { "load_prog3",   NULL, "ab", test_load_program3 },
        { "hard_nand",   "5", "5", advanced_nand},
        { "time",         NULL, "", test_time }
       
};

  
#define NTESTS (sizeof(tests)/sizeof(tests[0]))

/*
 * open file 'path' for writing, then free the pathname;
 * if anything fails, checked runtime error
 */
static FILE *open_and_free_pathname(char *path);

/*
 * if contents is NULL or empty, remove the given 'path', 
 * otherwise write 'contents' into 'path'.  Either way, free 'path'.
 */
static void write_or_remove_file(char *path, const char *contents);

static void write_test_files(struct test_info *test);


int main (int argc, char *argv[])
{
        bool failed = false;
        if (argc == 1)
                for (unsigned i = 0; i < NTESTS; i++) {
                        printf("***** Writing test '%s'.\n", tests[i].name);
                        write_test_files(&tests[i]);
                }
        else
                for (int j = 1; j < argc; j++) {
                        bool tested = false;
                        for (unsigned i = 0; i < NTESTS; i++)
                                if (!strcmp(tests[i].name, argv[j])) {
                                        tested = true;
                                        write_test_files(&tests[i]);
                                }
                        if (!tested) {
                                failed = true;
                                fprintf(stderr,
                                        "***** No test named %s *****\n",
                                        argv[j]);
                        }
                }
        return failed; /* failed nonzero == exit nonzero == failure */
}


static void write_test_files(struct test_info *test)
{
        FILE *binary = open_and_free_pathname(Fmt_string("%s.um", test->name));
        Seq_T instructions = Seq_new(0);
        test->build_test(instructions);
        Um_write_sequence(binary, instructions);
        Seq_free(&instructions);
        fclose(binary);

        write_or_remove_file(Fmt_string("%s.0", test->name),
                             test->test_input);
        write_or_remove_file(Fmt_string("%s.1", test->name),
                             test->expected_output);
}


static void write_or_remove_file(char *path, const char *contents)
{
        if (contents == NULL || *contents == '\0') {
                remove(path);
        } else {
                FILE *input = fopen(path, "wb");
                assert(input != NULL);

                fputs(contents, input);
                fclose(input);
        }
        free(path);
}


static FILE *open_and_free_pathname(char *path)
{
        FILE *fp = fopen(path, "wb");
        assert(fp != NULL);

        free(path);
        return fp;
}
