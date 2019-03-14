CC = gcc
CFLAGS = --std=c99 -Wall -O3
SRC = $(wildcard *.c)
EXECUTABLE = pawnmind

all: $(EXECUTABLE)

$(EXECUTABLE):
	$(CC) $(SRC) $(CFLAGS) -o $@

clean:
	rm $(EXECUTABLE)
