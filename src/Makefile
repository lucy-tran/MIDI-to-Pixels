#
# User-modifiable configuration variables:
#

SRCDIR    = src
TARGDIR   = ../bin
OBJDIR	  = obj
MIDILIB_DIR = ../midifile/lib
MIDIINC_DIR = ../midifile/include
# COMPILER  = LANG=C $(ENV) gcc-11 $(ARCH)
COMPILER  = g++ 
INC  = -I$(MIDIINC_DIR) -I.
LIB = -L${MIDILIB_DIR}
# Add -static flag to compile without dynamics libraries for better portability:
#PREFLAGS += -static

# Using C++ 2011 standard:
PREFLAGS = -std=c++11

#                                                                         #
# End of user-modifiable variables.                                       #
#                                                                         #
###########################################################################


# Code prefix
PROGRAM_PREFIX=musicViz

# Makefile for midifile examples 
#
CLANG=clang++ -Xpreprocessor
OMP=-fopenmp

SRCS1      = utils.c
# object files from source files
# OBJS1      = $(SRCS1:.c=.o)

CFLAGS = -std=gnu99 
WFLAGS = -Wno-format-overflow -Wno-unused-result

all: example1 example2 musicViz_seq #musicViz_omp


example1: example1.cpp 
	${COMPILER} ${PREFLAGS} ${INC} ${LIB} -o $(TARGDIR)/$@ $< -l midifile 

example2: example2.cpp 
	${COMPILER} ${PREFLAGS} ${INC} ${LIB} -o $(TARGDIR)/$@ $< -l midifile


# Makefile for music visualization application
gnuplot.o: gnuplot_i.c
	gcc $(CFLAGS) $(WFLAGS) -c $< -o $@ 

utils.o: utils.cpp 
	${COMPILER} -c $^ -o $@ 

# draw2D.o: draw2D.c
# 	gcc -c $^ -o $@ 

# source files for larger dynamically sized plates
# SRCS1      = draw2D.c gnuplot_i.c
# # object files from source files
# OBJS1      = $(SRCS1:.c=.o)

# .c.o: 
# 	gcc -c $(CFLAGS) $(WFLAGS) $<

# musicViz_seq: musicViz_seq.cpp  utils.o #gnuplot.o draw2D.o
# 	${COMPILER} ${PREFLAGS} ${INC} ${LIB} $< utils.o -o $(TARGDIR)/$@ -l midifile
musicViz_seq: utils.o
	${COMPILER} ${PREFLAGS} ${CFLAGS} ${INC} ${LIB} $^ -o $(TARGDIR)/$@ -l midifile

musicViz_omp: musicViz_omp.cpp utils.o
	${COMPILER} ${PREFLAGS} ${INC} ${LIB} ${OMP} $^ -o $(TARGDIR)/$@ -l midifile

clean:
	rm -f example1 example2 musicViz_omp musicViz_seq *.o
	