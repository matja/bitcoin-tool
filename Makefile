
# I try to be C89-compliant, but I like 64-bit types too much
DISABLE_WARNINGS = -Wno-long-long

CFLAGS = -ansi -Wall -ggdb -O2 $(DISABLE_WARNINGS)
LIBS = -lrt

OBJECTS = main.o keys.o hash.o base58.o result.o combination.o applog.o \
	utility.o prefix.o

.PHONY : all clean

all : bitcoin-tool

clean :
	@-rm bitcoin-tool $(OBJECTS)

bitcoin-tool : $(OBJECTS)
	$(CC) -o $@ $^ -lcrypto
