CC=m68k-palmos-gcc
CCFLAGS=-Wall

PimeKernel.o: PimeKernel.c PimeKernel.h
	$(CC) $(CCFLAGS) -c -o PimeKernel.o PimeKernel.c

.PHONY: clean

clean:
	rm -f PimeKernel.o *~
