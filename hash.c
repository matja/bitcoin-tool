#include "hash.h"

void Bitcoin_SHA256(struct BitcoinSHA256 *output, const void *input, size_t size)
{
	SHA256_CTX ctx;
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, input, size);
	SHA256_Final(output->data, &ctx);
}

void Bitcoin_DoubleSHA256(struct BitcoinSHA256 *output, const void *input, size_t size)
{
	struct BitcoinSHA256 round1;
	SHA256_CTX ctx;
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, input, size);
	SHA256_Final(round1.data, &ctx);
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, &round1.data, BITCOIN_SHA256_SIZE);
	SHA256_Final(output->data, &ctx);
}

void Bitcoin_RIPEMD160(struct BitcoinRIPEMD160 *output, const void *input, size_t size)
{
	RIPEMD160_CTX ctx;
	RIPEMD160_Init(&ctx);
	RIPEMD160_Update(&ctx, input, size);
	RIPEMD160_Final(output->data, &ctx);
}

