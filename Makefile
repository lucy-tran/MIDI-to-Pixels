#
# User-modifiable configuration variables:
#

SRCDIR    = src
TARGDIR   = bin
OBJDIR	  = obj
MIDILIB_DIR = midifile/lib
MIDIINC_DIR = midifile/include
# COMPILER  = LANG=C $(ENV) gcc-11 $(ARCH)
COMPILER  = g++
INC  = -I$(MIDIINC_DIR) -I${SRCDIR}
LIB = -L${MIDILIB_DIR}

# Using C++ 2011 standard:
PREFLAGS = -std=c++11

# Add -static flag to compile without dynamics libraries for better portability:
#PREFLAGS += -static

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

all: example1 example2 musicViz_seq musicViz_omp1 musicViz_omp2 musicViz_omp3

example1: ${SRCDIR}/example1.cpp 
	${COMPILER} ${PREFLAGS} ${INC} ${LIB} -o $(TARGDIR)/$@ $< -l midifile 

example2: ${SRCDIR}/example2.cpp 
	${COMPILER} ${PREFLAGS} ${INC} ${LIB} -o $(TARGDIR)/$@ $< -l midifile

# Makefile for music visualization application

utils.o: ${SRCDIR}/utils.cpp #gnuplot.o
	${COMPILER} -I${SRCDIR} -c $^ -o ${OBJDIR}/$@ 

musicViz_seq: ${SRCDIR}/musicViz_seq.cpp  utils.o 
	${COMPILER} ${PREFLAGS} ${INC} ${LIB} $< ${OBJDIR}/utils.o -o $(TARGDIR)/$@ -l midifile

musicViz_seq2: ${SRCDIR}/musicViz_seq2.cpp  utils.o 
	${COMPILER} ${PREFLAGS} ${INC} ${LIB} $< ${OBJDIR}/utils.o -o $(TARGDIR)/$@ -l midifile

musicViz_omp1: ${SRCDIR}/musicViz_omp1.cpp utils.o
	${COMPILER} ${PREFLAGS} ${INC} ${LIB} ${OMP} $< ${OBJDIR}/utils.o -o $(TARGDIR)/$@ -l midifile

musicViz_omp1.2: ${SRCDIR}/musicViz_omp1.2.cpp utils.o
	${COMPILER} ${PREFLAGS} ${INC} ${LIB} ${OMP} $< ${OBJDIR}/utils.o -o $(TARGDIR)/$@ -l midifile

musicViz_omp2: ${SRCDIR}/musicViz_omp2.cpp utils.o
	${COMPILER} ${PREFLAGS} ${INC} ${LIB} ${OMP} $< ${OBJDIR}/utils.o -o $(TARGDIR)/$@ -l midifile

musicViz_omp2.2: ${SRCDIR}/musicViz_omp2.2.cpp utils.o
	${COMPILER} ${PREFLAGS} ${INC} ${LIB} ${OMP} $< ${OBJDIR}/utils.o -o $(TARGDIR)/$@ -l midifile

musicViz_omp3: ${SRCDIR}/musicViz_omp3.cpp utils.o
	${COMPILER} ${PREFLAGS} ${INC} ${LIB} ${OMP} $< ${OBJDIR}/utils.o -o $(TARGDIR)/$@ -l midifile

musicViz_omp4: ${SRCDIR}/musicViz_omp4.cpp utils.o
	${COMPILER} ${PREFLAGS} ${INC} ${LIB} ${OMP} $< ${OBJDIR}/utils.o -o $(TARGDIR)/$@ -l midifile

clean:
	rm -f example1 example2 musicViz_seq musicViz_omp1 musicViz_omp2 musicViz_omp3 obj.*
	