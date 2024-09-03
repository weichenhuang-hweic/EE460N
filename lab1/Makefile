CC = gcc # Flag for implicit rules
CFLAGS = -std=c99 -Wall -g # Flag for implicit rules. Turn on debug info

objects = assembler.o
all: $(objects)
	$(CC) $(CFLAGS) $^ -o all

assembler.o: assembler.c
	$(CC) $(CFLAGS) -c assembler.c -o assembler.o

# clean:
# 	rm -f *.o all