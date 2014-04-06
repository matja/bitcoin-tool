#ifndef BITCOIN_INCLUDE_EC_H
#define BITCOIN_INCLUDE_EC_H

/** @file ec.h
 *  @brief Function prototypes for Bitcoin elliptic curve key functions.
 *
 *  @author Matthew Anger
 */

#include "hash.h" /* BITCOIN_RIPEMD160_SIZE */
#include "result.h" /* BitcoinResult */
#include "utility.h" /* uint_max2 */

/* declare Bitcoin address format */

#define BITCOIN_ADDRESS_VERSION_SIZE 1
#define BITCOIN_ADDRESS_SIZE (BITCOIN_ADDRESS_VERSION_SIZE + BITCOIN_RIPEMD160_SIZE)
#define BITCOIN_TEXT_ADDRESS_MAX_SIZE (BITCOIN_ADDRESS_SIZE)

struct BitcoinAddress
{
	unsigned char data[BITCOIN_ADDRESS_SIZE];
};

/* elliptic curve public key declarations */

#define BITCOIN_PUBLIC_KEY_COMPRESSED_SIZE 33
#define BITCOIN_PUBLIC_KEY_UNCOMPRESSED_SIZE 65
#define BITCOIN_PUBLIC_KEY_MAX_SIZE (BITCOIN_PUBLIC_KEY_UNCOMPRESSED_SIZE)

enum BitcoinPublicKeyCompression {
	BITCOIN_PUBLIC_KEY_EMPTY,
	BITCOIN_PUBLIC_KEY_COMPRESSED,
	BITCOIN_PUBLIC_KEY_UNCOMPRESSED
};

struct BitcoinPublicKey
{
	unsigned char data[BITCOIN_PUBLIC_KEY_UNCOMPRESSED_SIZE];

/*
Compression flag - do we compress the public key representation (store just
the x coordinate), or leave the uncompressed key as-is (store both x and y
coordinates).
*/
	enum BitcoinPublicKeyCompression compression;

/* Network type	(prefix byte on addresses) */
	const struct BitcoinNetworkType *network_type;
};

/* private key defines */
#define BITCOIN_PRIVATE_KEY_SIZE 32

/* WIF keys have a version byte at the start */
#define BITCOIN_PRIVATE_KEY_WIF_VERSION_SIZE 1

/* WIF keys for generating compressed public keys have a compression flag
which comes directly after the private key.  */
#define BITCOIN_PRIVATE_KEY_WIF_COMPRESSION_FLAG_SIZE 1
#define BITCOIN_PRIVATE_KEY_WIF_COMPRESSION_FLAG_COMPRESSED 1

/* WIF sizes (not including base58check checksum, we treat that seperately) */
#define BITCOIN_PRIVATE_KEY_WIF_UNCOMPRESSED_SIZE ( \
	BITCOIN_PRIVATE_KEY_SIZE + \
	BITCOIN_PRIVATE_KEY_WIF_VERSION_SIZE \
)
#define BITCOIN_PRIVATE_KEY_WIF_COMPRESSED_SIZE ( \
	BITCOIN_PRIVATE_KEY_SIZE + \
	BITCOIN_PRIVATE_KEY_WIF_VERSION_SIZE + \
	BITCOIN_PRIVATE_KEY_WIF_COMPRESSION_FLAG_SIZE \
)

/* Mini private key format, as used in Casascius physical bitcoins.
   Described here : https://en.bitcoin.it/wiki/Mini_private_key_format
*/
#define BITCOIN_MINI_PRIVATE_KEY_SIZE 30

struct BitcoinPrivateKey
{
	unsigned char data[BITCOIN_PRIVATE_KEY_SIZE];
/*
A private key is never actually compressed in the sense that the size
changes, this flag just indicates wether to generate a compressed or
uncompressed public key from this private key.
*/
	enum BitcoinPublicKeyCompression public_key_compression;

/* Network type	(prefix byte on addresses) */
	const struct BitcoinNetworkType *network_type;
};

/** @brief Check if a public key is not set.
 *  @param public_key[input] Pointer to public key to read.
 *
 *  @return 0 if not set, 1 otherwise.
 */
int BitcoinPublicKey_Empty(const struct BitcoinPublicKey *public_key);

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

/** @brief Return the size of a private key, in bytes.
 *
 *  @param private_key[input] Pointer to private key to read.
 *
 *  @return size of private key in bytes, or 0 if failure.
 */
size_t BitcoinPrivateKey_GetSize(const struct BitcoinPrivateKey *private_key);


/** @brief Return the size of a private key in Wallet Inport Format, in bytes.
 *
 *  @param private_key[input] Pointer to private key to read.
 *
 *  @return size of private key in bytes, or 0 if failure.
 */
size_t BitcoinPrivateKey_GetWIFSize(const struct BitcoinPrivateKey *private_key);

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
