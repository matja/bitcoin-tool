CFLAGS = -ansi -pedantic -Wall -ggdb -O0
LIBS = -lrt

OBJECTS = main.o ec.o hash.o utility.o base58.o result.o applog.o

.PHONY : all clean

all : bitcoin-tool

clean :
	@-rm bitcoin-tool $(OBJECTS)

bitcoin-tool : $(OBJECTS)
	$(CC) -o $@ $^ -lcrypto
