#ifndef INCLUDE_SHA256_H
#define INCLUDE_SHA256_H

#include <stdint.h>

typedef struct sha256_context_t {
	uint32_t state[8];
	uint32_t total[2];
	uint8_t buffer[64];
} sha256_context;

typedef struct sha256_hash_t {
	uint8_t data[32];
} sha256_hash;

void sha256_starts(sha256_context *ctx);
void sha256_process(sha256_context *ctx, const uint8_t data[64]);
void sha256_update(sha256_context *ctx, const void *input, size_t length);
void sha256_finish(sha256_context *ctx, sha256_hash *output_hash);

#endif
