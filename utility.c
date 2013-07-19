#include "utility.h"

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

int Bitcoin_DecodeHexChar(uint_fast8_t *output, char c)
{
	/* convert hex char 0-9,a-f,A-F to 0-15 decimal value and return 1,
	or return 0 if char invalid */
	if (c >= '0' && c <= '9') {
		*output = c - '0';
		return 1;
	} else if (c >= 'A' && c <= 'F') {
		*output = c - 'A' + 10;
		return 1;
	} else if (c >= 'a' && c <= 'f') {
		*output = c - 'a' + 10;
		return 1;
	}
	return 0;
}

int Bitcoin_DecodeHex(void *output, size_t output_size,
	const char *source, size_t source_size
)
{
	unsigned char *output_bytes = (unsigned char *)output;

	while (source_size) {
		uint_fast8_t high, low;

		if (!Bitcoin_DecodeHexChar(&high, *source++)) { return 0; }
		if (!Bitcoin_DecodeHexChar(&low, *source++)) { return 0; }

		*output_bytes++ = (high << 4) | low;
		source_size -= 2;
	}

	return 1;
}

void Bitcoin_OutputHex(const void *source, size_t source_size)
{
	const uint8_t *source_bytes = (const uint8_t *)source;
	while (source_size--) {
		printf("%02x", *source_bytes++);
	}
}

void Bitcoin_OutputHexPretty(const void *source, size_t source_size)
{
	const char *delim = "";
	size_t i;
	for (i=0; i < source_size; i++) {
		unsigned m = i % 16;
		if (!m) {
			if (!i) {
				delim = "";
			} else {
				delim = "\n";
			}
		} else if (m == 8) {
			delim = "-";
		} else {
			delim = " ";
		}

		printf("%s%02x", delim,
			((const unsigned char *)source)[i]
		);
	}
	printf("\n");
}

void Bitcoin_ReverseBytes(void *buffer, size_t size)
{
	char *start = (char *)buffer;
	char *end = start + size;
	char *last = end - 1;

	while (start < last) {
		/* swap chars */
		char t = *start;
		*start = *last;
		*last = t;
		/* next pair */
		start++;
		last--;
	}
}
