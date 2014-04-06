#include "base58.h"
#include "hash.h"
#include "utility.h"
#include "applog.h"
#include "combination.h"

#include <string.h>
#include <stdlib.h>
#include <alloca.h>
#include <openssl/bn.h>

/* base58 is 0-9,A-Z,a-z (62 chars), but with the 0,I,O, and l chars removed,
leaving 58 chars */
static const char base58_digits[] =
	"123456789"
	"ABCDEFGHJKLMNPQRSTUVWXYZ"
	"abcdefghijkmnopqrstuvwxyz";

BitcoinResult Bitcoin_EncodeBase58(
	char *output, size_t output_buffer_size, size_t *encoded_output_size,
	const void *source, size_t source_size
)
{
	static const unsigned base = 58;
	const unsigned char *source_bytes = (const unsigned char *)source;
	BIGNUM x, base_bn, div_bn, rem_bn;
	BN_CTX *bn_ctx = BN_CTX_new();
	char *d = output;
	size_t output_count = 0;
	int output_overflow = 0;

	/* initialise bignums */
	BN_init(&x);
	BN_init(&base_bn);
	BN_init(&div_bn);
	BN_init(&rem_bn);
	BN_bin2bn(source, source_size, &x);
	BN_set_word(&base_bn, base);

	/* while source is not zero, keep dividing source by base, using the
		remainder to map to a digit. (standard base conversion)
	*/
	while (!BN_is_zero(&x) && !output_overflow) {
		BN_div(&div_bn, &rem_bn, &x, &base_bn, bn_ctx);
		BN_copy(&x, &div_bn);
		if (output_count < output_buffer_size) {
			*d++ = base58_digits[BN_get_word(&rem_bn)];
			output_count++;
		} else {
			output_overflow = 1;
		}
	}

	/* for the number of leading zeros in the source, add that many
		digit[0] characters to the end of the encoded string
	*/
	while (
		(source_bytes != source_bytes+source_size)
		&& *source_bytes == 0
	) {
		*d++ = base58_digits[*source_bytes++];
	}

	/* reverse everything, so that it is most significant to least significant */
	Bitcoin_ReverseBytes(output, d - output);

	*encoded_output_size = d - output;

	BN_CTX_free(bn_ctx);
	BN_free(&x);
	BN_free(&base_bn);
	BN_free(&div_bn);
	BN_free(&rem_bn);

	return output_overflow ? BITCOIN_ERROR_OUTPUT_BUFFER_TOO_SMALL : BITCOIN_SUCCESS;
}

BitcoinResult Bitcoin_EncodeBase58Check(
	char *output, size_t output_size, size_t *encoded_output_size,
	const void *source, size_t source_size
)
{
	struct BitcoinSHA256 checksum;
	size_t buffer_size = source_size + BITCOIN_BASE58CHECK_CHECKSUM_SIZE;
	unsigned char *buffer = malloc(buffer_size);
	BitcoinResult result = 0;

	/* calc checksum bytes */
	Bitcoin_DoubleSHA256(&checksum, source, source_size);

	memcpy(buffer, source, source_size);
	memcpy(buffer + source_size, &checksum, BITCOIN_BASE58CHECK_CHECKSUM_SIZE);

	result = Bitcoin_EncodeBase58(output, output_size, encoded_output_size, buffer, buffer_size);
	free(buffer);

	return result;
}

BitcoinResult Bitcoin_DecodeBase58(
	uint8_t *output, size_t output_buffer_size, size_t *decoded_output_size,
	const void *input, size_t input_size
)
{
	/*
	map ASCII chars 0x00 to 0x7f to base58 digit values 0 to 57,
	or -1 if the character is invalid
	*/
	static int digits[] = {
		 -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
		,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
		,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
		,-1, 0, 1, 2, 3, 4, 5, 6, 7, 8,-1,-1,-1,-1,-1,-1
		,-1, 9,10,11,12,13,14,15,16,-1,17,18,19,20,21,-1
		,22,23,24,25,26,27,28,29,30,31,32,-1,-1,-1,-1,-1
		,-1,33,34,35,36,37,38,39,40,41,42,43,-1,44,45,46
		,47,48,49,50,51,52,53,54,55,56,57,-1,-1,-1,-1,-1
	};
	const char *input_bytes_start = (const char *)input;
	const char *input_bytes_end = input_bytes_start + input_size;
	const char *pzc = input_bytes_start;
	const char *pc = input_bytes_end - 1;
	unsigned leading_zeros = 0;
	unsigned bn_bytes_req = 0;
	unsigned bn_bytes_wrote = 0;

	BIGNUM base, m1, m2, result, sub;
	BN_CTX *bn_ctx;

	bn_ctx = BN_CTX_new();
	BN_init(&base);
	BN_init(&m1);
	BN_init(&m2);
	BN_init(&result);
	BN_init(&sub);

	BN_zero(&result);
	BN_one(&m2);
	BN_set_word(&base, 58);

	/* count leading zero bytes (encoded as '1') */
	while (pzc != input_bytes_end && *pzc == '1') {
		leading_zeros++;
		pzc++;
	}

	pc = input_bytes_end - 1;

	while (pc >= pzc) {
		int v = digits[(unsigned)*pc];
		if (v == -1) {
			printf("%s: invalid character (%u)\n", __func__, (unsigned)*pc);
			return 0;
		}
		BN_set_word(&m1, v);
		BN_mul(&sub, &m1, &m2, bn_ctx);
		BN_add(&result, &result, &sub);
		pc--;
		BN_mul(&m2, &m2, &base, bn_ctx);
	}

	bn_bytes_req = BN_num_bytes(&result);

	if (bn_bytes_req > output_buffer_size) {
		printf("%s: bn_bytes_req too large (%u)\n", __func__, bn_bytes_req);
		/* output buffer too small, failure */
		return BITCOIN_ERROR_OUTPUT_BUFFER_TOO_SMALL;
	}

	bn_bytes_wrote = BN_bn2bin(&result, output+leading_zeros);
	BN_CTX_free(bn_ctx);
	BN_free(&base);
	BN_free(&m1);
	BN_free(&m2);
	BN_free(&result);
	BN_free(&sub);

	*decoded_output_size = bn_bytes_wrote+leading_zeros;

	return BITCOIN_SUCCESS;
}

BitcoinResult Bitcoin_DecodeBase58Check(
	uint8_t *output, size_t output_buffer_size, size_t *decoded_output_size,
	const void *input, size_t input_size
)
{
	struct BitcoinSHA256 hash;
	size_t temp_decoded_output_size;

	BitcoinResult result = Bitcoin_DecodeBase58(
		output, output_buffer_size, &temp_decoded_output_size,
		input, input_size
	);

	if (result != BITCOIN_SUCCESS) {
		return result;
	}

	Bitcoin_DoubleSHA256(&hash, output,
		temp_decoded_output_size - BITCOIN_BASE58CHECK_CHECKSUM_SIZE
	);

	if (memcmp(hash.data
		,output + temp_decoded_output_size - BITCOIN_BASE58CHECK_CHECKSUM_SIZE
		,BITCOIN_BASE58CHECK_CHECKSUM_SIZE)
	) {
		/* checksums didn't match, failure */
		return BITCOIN_ERROR_CHECKSUM_FAILURE;
	}


	*decoded_output_size = temp_decoded_output_size -
		BITCOIN_BASE58CHECK_CHECKSUM_SIZE;

	/* checksum matched, success */
	return BITCOIN_SUCCESS;
}

BitcoinResult Bitcoin_FixBase58Check(
	char *fixed_output, size_t fixed_output_buffer_size, size_t *fixed_output_size,
	uint8_t *output, size_t output_buffer_size, size_t *decoded_output_size,
	const char *input, size_t input_size,
	unsigned change_chars,
	unsigned remove_chars,
	unsigned insert_chars
)
{
	/* attempt to 'fix' an invalid base58check string by changing characters
	until the checksum is valid */

	uint64_t change_count = 0;
	size_t required_fixed_output_size = fixed_output_buffer_size + insert_chars;
	BitcoinResult result;
	static const unsigned radix = 58;
	unsigned i, j, overflow;
	char *format_output = NULL;
	struct Combination c;
	unsigned n, r, done = 0;
	char *digits;

	applog(APPLOG_NOTICE, __func__,
		"Attempting to fix Base58Check input by changing %s%d character%s ...",
		change_chars == 1 ? "" : "up to ",
		(int)change_chars,
		change_chars == 1 ? "" : "s"
	);

	if (required_fixed_output_size < input_size) {
		applog(APPLOG_ERROR, __func__,
			"output buffer is not large enough (%u) to store changed"
			" input text (%u)",
			(unsigned)input_size,
			(unsigned)required_fixed_output_size
		);
		return BITCOIN_ERROR_OUTPUT_BUFFER_TOO_SMALL;
	}

	format_output = malloc(required_fixed_output_size + 1);
	*fixed_output_size = input_size;

	for (r = 1; r <= change_chars && !done; r++) {
		applog(APPLOG_NOTICE, __func__,
			"Changing %d character%s ...",
			r,
			r == 1 ? "" : "s"
		);
		n = input_size;
		Combination_create(&c, n, r);
		digits = malloc(r);
		do {
			memcpy(fixed_output, input, input_size);
			memset(digits, 0, r);
			overflow = 0;

			while (!overflow && !done) {
				int carry = 0;

				/* convert digits to base58 and update 'fixed' output */
				for (i=0; i < r; i++) {
					fixed_output[c.k[i]] = base58_digits[(int)digits[i]];
					change_count++;
				}

				result = Bitcoin_DecodeBase58Check(output, output_buffer_size,
					decoded_output_size, fixed_output, input_size
				);

				if (result == BITCOIN_SUCCESS) {
					memcpy(format_output, input, input_size);
					format_output[input_size] = '\0';
					applog(APPLOG_WARNING, __func__,
						"from: %s", format_output
					);

					memcpy(format_output, fixed_output, *fixed_output_size);
					format_output[*fixed_output_size] = '\0';
					applog(APPLOG_WARNING, __func__,
						"  to: %s", format_output
					);

					memset(format_output, ' ', *fixed_output_size);
					format_output[*fixed_output_size] = '\0';
					for (j=0; j < r; j++) {
						format_output[c.k[j]] = '^';
					}
					applog(APPLOG_WARNING, __func__,
						"      %s", format_output
					);
					done = 1;
				}

				/* calculate next set of 'r' digits */
				digits[0]++;
				for (i=0; i < r; i++) {
					if (carry) {
						digits[i] += carry;
						carry = 0;
					}
					if (digits[i] == radix) {
						digits[i] = 0;
						carry = 1;
						overflow = i == r-1;
					} else {
						carry = 0;
					}
				}
			}
		} while (!done && Combination_next(&c));
		Combination_destroy(&c);
	}

	if (!done) {
		applog(APPLOG_WARNING, __func__,
			"Failed to find any combination of changing the Base58Check input"
			" that results in a valid checksum. %llu combinations were tried."
			" (%f%% chance of error).",
			(long long unsigned)change_count,
			((double)change_count / ((unsigned long long)1 << 32)) * 100
		);
		return BITCOIN_ERROR_CHECKSUM_FAILURE;
	}

	applog(APPLOG_WARNING, __func__,
		"Base58Check input has been corrected after %llu combinations "
		" (%f%% chance of error).",
		(long long unsigned)change_count,
		((double)change_count / ((unsigned long long)1 << 32)) * 100
	);

	return BITCOIN_SUCCESS;
}
