
ifeq ($(OS),Windows_NT)
	DETECT_OS := WindowsNT
else
	DETECT_OS := $(shell sh -c 'uname -s 2>/dev/null || echo unknown')
endif

ifeq ($(DETECT_OS),WindowsNT)
	# TODO: Handle environments other than Cygwin
	# Need to have explicit include path on Cygwin
	INCLUDE = -I /usr/include
	# Cygwin doesn't seem to always install a 'cc', set a default.
	CC = x86_64-w64-mingw32-gcc
	CFLAGS += -D OS_WINDOWS_NT
else
	# Everything else is considered UNIX-like
	CFLAGS += -D OS_UNIX
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

ifeq ($(OS_FAMILY_WINDOWS),1)
	LIBS += -lgdi32
endif

# being able to use -pedantic would be nice but then we get errors on the
# structure initialisation in prefix.c
CFLAGS += -ansi -Wall $(CFLAGS_DEBUG) $(CFLAGS_OPTIMISE) \
	$(CFLAGS_DISABLE_WARNINGS) $(INCLUDE)

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

