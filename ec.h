#ifndef BITCOIN_INCLUDE_EC_H
#define BITCOIN_INCLUDE_EC_H

/** @file ec.h
 *  @brief Function prototypes for Bitcoin elliptic curve key functions.
 *
 *  @author Matthew Anger
 */

#include "hash.h" /* BITCOIN_RIPEMD160_SIZE */
#include "result.h" /* BitcoinResult */

#define BITCOIN_TEXT_ADDRESS_MAX_SIZE BITCOIN_ADDRESS_SIZE

#define BITCOIN_ADDRESS_VERSION_SIZE 1
#define BITCOIN_ADDRESS_SIZE (BITCOIN_ADDRESS_VERSION_SIZE + BITCOIN_RIPEMD160_SIZE)
struct BitcoinAddress
{
	unsigned char data[BITCOIN_ADDRESS_SIZE];
};

/* elliptic curve public key declarations */

#define BITCOIN_PUBLIC_KEY_COMPRESSED_SIZE 33
#define BITCOIN_PUBLIC_KEY_UNCOMPRESSED_SIZE 65

enum BitcoinPublicKeyCompression {
	BITCOIN_PUBLIC_KEY_COMPRESSED,
	BITCOIN_PUBLIC_KEY_UNCOMPRESSED
};

struct BitcoinPublicKey
{
	unsigned char data[BITCOIN_PUBLIC_KEY_UNCOMPRESSED_SIZE];
	enum BitcoinPublicKeyCompression compression;
};

/* elliptic curve private key declarations */

#define BITCOIN_PRIVATE_KEY_SIZE 32


struct BitcoinPrivateKey
{
	unsigned char data[BITCOIN_PRIVATE_KEY_SIZE];
/*
a private key is never actually compressed, this flag just indicates wether
to generate a compressed or uncompressed public key from this private key.
*/	
	enum BitcoinPublicKeyCompression public_key_compression;
};

/** @brief Return the size in bytes of a public key.
 *         Compressed keys are 33 bytes, which are composed of a byte
 *         indicating the sign of the y coordinate of the point on the curve,
 *         followed by 32 bytes of the x coordinate.
 *          
 *         Uncompressed keys are 65 bytes, which are composed of 0x04 byte
 *         followed by 32 bytes of the x coordinate, then 32 bytes of the
 *         y coordinate of the point on the curve.
 *
 *         All the bytes of the public key (either 33 or 65) are then hashed
 *         to derive the Bitcoin address.
 *
 *  @param public_key[input] Pointer to public key to read.
 *
 *  @return size of public key in bytes, or 0 if failure.
 */
size_t BitcoinPublicKey_GetSize(const struct BitcoinPublicKey *public_key);

/** @brief Convert a private key to a public key.
 *
 *  @param public_key[output] Pointer to public key to write.
 *  @param private_key Pointer to private key to read.

 *  @return BitcoinResult indicating error state.
 */
BitcoinResult Bitcoin_MakePublicKeyFromPrivateKey(
	struct BitcoinPublicKey *public_key,
	const struct BitcoinPrivateKey *private_key
);

/** @brief Convert a public key to a Bitcoin address structure.
 *
 *  @param address[output] Pointer to address to write.
 *  @param public_key Pointer to Bitcoin public read.

 *  @return BitcoinResult indicating error state.
 */
BitcoinResult Bitcoin_MakeAddressFromPublicKey(
	struct BitcoinAddress *address,
	const struct BitcoinPublicKey *public_key
);

/** @brief Convert a base58 representation of a private key to a raw
 *         private key.
 *
 *  @param[output] output_private_key Pointer to private key to write.
 *  @param[input] input_text Base58Check string to read.
 *  @param[input] input_size Number of characters in Base58Check string.

 *  @return BitcoinResult indicating error state.
 */
BitcoinResult Bitcoin_LoadPrivateKeyFromBase58(
	struct BitcoinPrivateKey *output_private_key,
	const char *input_text, size_t input_size
);

/** @brief Convert a base58 representation of a public key to a raw
 *         public key.
 *
 *  @param[output] output_public_key Pointer to public key to write.
 *  @param[input] input_text Base58Check string to read.
 *  @param[input] input_size Number of characters in Base58Check string.

 *  @return BitcoinResult indicating error state.
 */
BitcoinResult Bitcoin_LoadPublicKeyFromBase58(
	struct BitcoinPublicKey *output_public_key,
	const char *input_text, size_t input_size
);

#endif
