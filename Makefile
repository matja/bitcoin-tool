CC = clang
CFLAGS = -O0 -ggdb -ansi -pedantic -Wall
LIBS = -lrt

.PHONY : all clean

all : bitcoin-tool

clean :
	@-rm bitcoin-tool main.o ec.o hash.o utility.o base58.o result.o

bitcoin-tool : main.o ec.o hash.o utility.o base58.o result.o
	$(CC) -o $@ $^ -lcrypto

