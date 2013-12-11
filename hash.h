#ifndef BITCOIN_INCLUDE_HASH_H
#define BITCOIN_INCLUDE_HASH_H

/** @file hash.h
 *  @brief Wrapper functions for cryptographic hashes, to make a consistant
 *         interface between different implementations.
 *
 *  @author Matthew Anger
 */

#include <openssl/sha.h> /* SHA256_DIGEST_LENGTH */
#include <openssl/ripemd.h> /* RIPEMD160_DIGEST_LENGTH */

/* Wrap various data types in structs for type-safety */

#define BITCOIN_SHA256_SIZE (SHA256_DIGEST_LENGTH)
struct BitcoinSHA256
{
	unsigned char data[BITCOIN_SHA256_SIZE];
};

#define BITCOIN_RIPEMD160_SIZE (RIPEMD160_DIGEST_LENGTH)
struct BitcoinRIPEMD160
{
	unsigned char data[BITCOIN_RIPEMD160_SIZE];
};

/** @brief Calculate SHA256 hash and write to output buffer.
 *
 *  @param[out] output Pointer to hash output buffer.
 *  @param[in] input Pointer to data to hash.
 *  @param[in] size Number of bytes of data at 'input' to hash.
 */
void Bitcoin_SHA256(struct BitcoinSHA256 *output,
	const void *input, size_t size
);

/** @brief Calculate double SHA256 hash (SHA256 hash of SHA256 hash) and
 *         write to output buffer.
 *
 *  @param[out] output Pointer to hash output buffer.
 *  @param[in] input Pointer to data to hash.
 *  @param[in] size Number of bytes of data at 'input' to hash.
 */
void Bitcoin_DoubleSHA256(struct BitcoinSHA256 *output,
	const void *input, size_t size
);

/** @brief Calculate RIPEMD160 hash and write to output buffer.
 *
 *  @param[out] output Pointer to hash output buffer.
 *  @param[in] input Pointer to data to hash.
 *  @param[in] size Number of bytes of data at 'input' to hash.
 */
void Bitcoin_RIPEMD160(struct BitcoinRIPEMD160 *output,
	const void *input, size_t size
);

#endif

