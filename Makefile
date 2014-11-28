
ifeq ($(OS),Windows_NT)
	OS_FAMILY = Windows
else
	OS_FAMILY = UNIX
endif

# I try to be C89-compliant, but I like 64-bit types too much
DISABLE_WARNINGS = -Wno-long-long

CFLAGS = -ansi -Wall -ggdb -O2 $(DISABLE_WARNINGS) -I /usr/include -DOS_FAMILY=$(OS_FAMILY)

LIBS = -lcrypto -lssl
ifeq ($(OS_FAMILY),Windows)
	LIBS += -lgdi32
endif

OBJECTS = main.o keys.o hash.o base58.o result.o combination.o applog.o \
	utility.o prefix.o

.PHONY : all clean

all : bitcoin-tool

clean :
	@-rm bitcoin-tool $(OBJECTS)

bitcoin-tool : $(OBJECTS)
	$(CC) -o $@ $^ -L /usr/lib $(LIBS)
