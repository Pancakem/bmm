CC=gcc
CFLAGS= -Wall -Werror -O2

all: build

build: test.c bmm.c
	$(CC) -o test test.c bmm.c $(CFLAGS)

test:
	./test

clean:
	rm test
