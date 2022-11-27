CC = g++

default: pearsonCorr4

sparsematmult: pearsonCorr4.cpp
	${CC} -O0 -g -Wall -Wextra -Wno-unused-parameter -fopenmp -o $@ pearsonCorr4.cpp

clean:
	-rm -f pearsonCorr4
