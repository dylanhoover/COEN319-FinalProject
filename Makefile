CC = g++

default: pearsonCorrWORKING

pearsonCorrWORKING: pearsonCorrWORKING.cpp
	${CC} -O0 -g -Wall -Wextra -Wno-unused-parameter -fopenmp -o $@ pearsonCorrWORKING.cpp

clean:
	-rm -f pearsonCorrWORKING
