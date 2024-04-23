CC = /usr/lib64/openmpi/bin/mpicxx

all: main

main: main.cpp
	$(CC) -o main main.cpp

seq: extra_seq.c
	$(CC) -o seq extra_seq.c

open: main_openmp.cpp
	$(CC) -fopenmp -o open main_openmp.cpp
clean:
	rm -f main
	rm -f open
