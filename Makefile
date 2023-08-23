CC=gcc
CFLAGS= -Wall -Werror -O2

all: build


build: test.c bmm.c
	$(CC) -o test test.c bmm.c $(CFLAGS)

build-debug: test.c bmm.c
	$(CC) -o test test.c bmm.c $(CFLAGS) -D DEBUG

test: build
	./test

test-debug: build-debug
	./test

clean:
	rm test
