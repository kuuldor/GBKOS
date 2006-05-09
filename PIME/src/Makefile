CC=m68k-palmos-gcc
CCFLAGS=-Wall -g

.PHONY: all

all: PimeKernel.o

PimeKernel.o: PimeKernel.c PimeKernel.h
	$(CC) $(CCFLAGS) -c -o PimeKernel.o PimeKernel.c

.PHONY: clean

clean:
	rm -f PimeKernel.o *~
