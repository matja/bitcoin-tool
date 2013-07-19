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

#define BITCOIN_PUBLIC_KEY_SIZE 65
struct BitcoinPublicKey
{
	unsigned char data[BITCOIN_PUBLIC_KEY_SIZE];
};

#define BITCOIN_PRIVATE_KEY_SIZE 32
struct BitcoinPrivateKey
{
	unsigned char data[BITCOIN_PRIVATE_KEY_SIZE];
};

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
