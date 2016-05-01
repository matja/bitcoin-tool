
ifeq ($(OS),Windows_NT)
	OS_FAMILY = Windows
	# need to have explicit include path on Cygwin
	INCLUDE = -I /usr/include
else
	OS_FAMILY = UNIX
endif

CFLAGS_OPTIMISE = -O2
CFLAGS_DEBUG =
# I try to be C89-compliant, but I like 64-bit types too much
CFLAGS_DISABLE_WARNINGS = -Wno-long-long
LIBS = -lcrypto -lssl

ifdef TEST_COVERAGE
	CFLAGS_OPTIMISE = -O0
	CFLAGS_DEBUG = -ggdb
	CFLAGS += -fprofile-arcs -ftest-coverage
	LIBS += -lgov
endif

ifeq ($(OS_FAMILY),Windows)
	LIBS += -lgdi32
endif

# being able to use -pedantic would be nice but then we get errors on the
# structure initialisation in prefix.c
CFLAGS = -ansi -Wall $(CFLAGS_DEBUG) $(CFLAGS_OPTIMISE) \
	$(CFLAGS_DISABLE_WARNINGS) $(INCLUDE) -DOS_FAMILY=$(OS_FAMILY)

OBJECTS = main.o keys.o hash.o base58.o result.o combination.o applog.o \
	utility.o prefix.o

.PHONY : all clean test

all : bitcoin-tool

test : bitcoin-tool
	./tests.sh

clean :
	@-rm bitcoin-tool $(OBJECTS)

bitcoin-tool : $(OBJECTS)
	$(CC) -o $@ $^ -L /usr/lib $(LIBS)

