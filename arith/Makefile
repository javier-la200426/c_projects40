# 	           Makefile for CS 40 Homework 4: arith
#
#     Author: Javier Laveaga {jlavea01} and Prince Millidah {pmilli03}
#
#  Maintenance targets:
#
#
#

############## Variables ###############

CC = gcc # The compiler being used

# Updating include path to use Comp 40 .h files and CII interfaces
IFLAGS = -I/comp/40/build/include -I/usr/sup/cii40/include/cii

CFLAGS = -g -std=gnu99 -Wall -Wextra -Werror -Wfatal-errors -pedantic $(IFLAGS)

LDFLAGS = -g -L/comp/40/build/lib -L/usr/sup/cii40/lib64

INCLUDES = $(shell echo *.h)

# Libraries needed for linking
# All programs cii40 (Hanson binaries) and *may* need -lm (math)
# 40locality is a catch-all for this assignment, netpbm is needed for pnm
# rt is for the "real time" timing library, which contains the clock support
LDLIBS = -l40locality -larith40 -lnetpbm -lcii40 -lm -lrt

all: 40image-6

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@

40image-6: 40image.o compressor.o a2plain.o uarray2.o decompressor.o a2blocked.o uarray2b.o bitpack.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

ppmdiff: ppmdiff.o a2plain.o uarray2.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

testing: testing.o compressor.o a2plain.o uarray2.o decompressor.o a2blocked.o uarray2b.o bitpack.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)



clean:
	rm -f ppmtrans a2test timing_test *.o



