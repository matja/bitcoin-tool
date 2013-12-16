
# I try to be C89-compliant, but I like 64-bit types too much
DISABLE_WARNINGS = -Wno-long-long

CFLAGS = -ansi -pedantic -Wall -ggdb -O2 $(DISABLE_WARNINGS)
LIBS = -lrt

OBJECTS = main.o ec.o hash.o base58.o result.o combination.o applog.o utility.o 

.PHONY : all clean

all : bitcoin-tool

clean :
	@-rm bitcoin-tool $(OBJECTS)

bitcoin-tool : $(OBJECTS)
	$(CC) -o $@ $^ -lcrypto
