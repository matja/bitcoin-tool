#include "base58.h"
#include "hash.h"
#include "utility.h"

#include <string.h>
#include <stdlib.h>
#include <alloca.h>
#include <openssl/bn.h>

BitcoinResult Bitcoin_EncodeBase58(
	char *output, size_t *output_size,
	const void *source, size_t source_size
)
{
	/* base58 is 0-9,A-Z,a-z (62 chars), but with the 0,I,O and l chars
	removed, leaving 58 chars */
	static const char digits[] =
		"123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
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
		if (output_count < *output_size) {
			*d++ = digits[BN_get_word(&rem_bn)];
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
		*d++ = digits[*source_bytes++];
	}

	/* reverse everything, so that it is most significant to least significant */
	Bitcoin_ReverseBytes(output, d - output);

	*output_size = d - output;

	BN_CTX_free(bn_ctx);
	BN_free(&x);
	BN_free(&base_bn);
	BN_free(&div_bn);
	BN_free(&rem_bn);

	return output_overflow ? BITCOIN_ERROR_OUTPUT_BUFFER_TOO_SMALL : BITCOIN_SUCCESS;
}

BitcoinResult Bitcoin_EncodeBase58Check(
	char *output, size_t *output_size,
	const void *source, size_t source_size
)
{
	struct BitcoinSHA256 checksum;
	size_t inter_size = source_size + BITCOIN_BASE58CHECK_CHECKSUM_SIZE;
	unsigned char *inter = alloca(inter_size);

	/* calc checksum bytes */
	Bitcoin_DoubleSHA256(&checksum, source, source_size);

	memcpy(inter, source, source_size);
	memcpy(inter + source_size, &checksum, BITCOIN_BASE58CHECK_CHECKSUM_SIZE);

	return Bitcoin_EncodeBase58(output, output_size, inter, inter_size);
}

BitcoinResult Bitcoin_DecodeBase58Check(
	void *output, size_t *output_size,
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
	unsigned char buffer[256] = { 0 };
	struct BitcoinSHA256 hash;

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

	if (bn_bytes_req > sizeof(buffer)) {
		printf("%s: bn_bytes_req too large (%u)\n", __func__, bn_bytes_req);
		/* output buffer too small, failure */
		return BITCOIN_ERROR_OUTPUT_BUFFER_TOO_SMALL;
	}

	bn_bytes_wrote = BN_bn2bin(&result, buffer+leading_zeros);
	BN_CTX_free(bn_ctx);
	BN_free(&base);
	BN_free(&m1);
	BN_free(&m2);
	BN_free(&result);
	BN_free(&sub);

	*output_size = bn_bytes_wrote+leading_zeros;

	Bitcoin_DoubleSHA256(&hash, buffer, *output_size - BITCOIN_BASE58CHECK_CHECKSUM_SIZE);

	if (memcmp(hash.data
		,buffer + *output_size - BITCOIN_BASE58CHECK_CHECKSUM_SIZE
		,BITCOIN_BASE58CHECK_CHECKSUM_SIZE)
	) {
		/* checksums didn't match, failure */
		return BITCOIN_ERROR_CHECKSUM_FAILURE;
	}

	memcpy(output, buffer, *output_size);

	/* checksum matched, success */
	return BITCOIN_SUCCESS;
}
