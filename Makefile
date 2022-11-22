CC = g++

default: pearsonCorr

sparsematmult: pearsonCorr.cpp
	${CC} -O0 -g -Wall -Wextra -Wno-unused-parameter -fopenmp -o $@ pearsonCorr.cpp

clean:
	-rm -f pearsonCorr
