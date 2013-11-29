/*
Elliptic curve secp256k1 related functions.

reference : https://en.bitcoin.it/wiki/Secp256k1
*/

#include <string.h>

#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>

#include "ec.h"
#include "base58.h"

size_t BitcoinPublicKey_GetSize(const struct BitcoinPublicKey *public_key)
{
	switch (public_key->compression) {
		case BITCOIN_PUBLIC_KEY_COMPRESSED :
			return 33;
			break;
		case BITCOIN_PUBLIC_KEY_UNCOMPRESSED :
			return 65;
			break;
		default :
			fprintf(stderr, "%s: invalid public_key->compression value (%u)\n",
				__func__,
				(unsigned)public_key->compression
			);
			break;
	}
	return 0;
}

BitcoinResult Bitcoin_MakePublicKeyFromPrivateKey(
	struct BitcoinPublicKey *public_key,
	const struct BitcoinPrivateKey *private_key
)
{
	BN_CTX *ctx = NULL;
	EC_KEY *key = EC_KEY_new_by_curve_name(NID_secp256k1);
	EC_POINT *ec_public = NULL;
	unsigned char *public_key_ptr = public_key->data;
	BIGNUM private_key_bn;
	const EC_GROUP *group = EC_KEY_get0_group(key);
	int size, size2;
	unsigned ec_compression = private_key->public_key_compression;
	size_t expected_public_key_size = 0;
	enum BitcoinPublicKeyCompression public_key_compression;

	BN_init(&private_key_bn);
	BN_bin2bn(private_key->data, BITCOIN_PRIVATE_KEY_SIZE, &private_key_bn);
	ec_public = EC_POINT_new(group);

	ctx = BN_CTX_new();
	if (!ctx) {
		fprintf(stderr, "%s: BN_CTX_new failed\n", __func__);
		return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
	}

	if (!EC_POINT_mul(group, ec_public, &private_key_bn, NULL, NULL, ctx)) {
		fprintf(stderr, "%s: EC_POINT_mul failed\n", __func__);
		return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
	}

	EC_KEY_set_private_key(key, &private_key_bn);
	EC_KEY_set_public_key(key, ec_public);

	if (ec_compression == BITCOIN_PUBLIC_KEY_COMPRESSED) {
		EC_KEY_set_conv_form(key, POINT_CONVERSION_COMPRESSED);
		expected_public_key_size = BITCOIN_PUBLIC_KEY_COMPRESSED_SIZE;
		public_key_compression = BITCOIN_PUBLIC_KEY_COMPRESSED;
	} else {
		EC_KEY_set_conv_form(key, POINT_CONVERSION_UNCOMPRESSED);
		expected_public_key_size = BITCOIN_PUBLIC_KEY_UNCOMPRESSED_SIZE;
		public_key_compression = BITCOIN_PUBLIC_KEY_UNCOMPRESSED;
	}

	size = i2o_ECPublicKey(key, NULL);

	if (size != expected_public_key_size) {
		fprintf(stderr, "%s: invalid public key size (%u), should be %u\n",
			__func__,
			(unsigned)size,
			(unsigned)expected_public_key_size
		);
		BN_free(&private_key_bn);
		return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
	}	

	size2 = i2o_ECPublicKey(key, &public_key_ptr);
	if (size2 != expected_public_key_size) {
		fprintf(stderr, "%s: invalid public key size (%u), should be %u\n",
			__func__,
			(unsigned)size,
			(unsigned)expected_public_key_size
		);		
		BN_free(&private_key_bn);
		return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
	}

	/* public key appears to be valid by now, set the compression type */
	public_key->compression = public_key_compression;

	/* free resources */
	EC_POINT_clear_free(ec_public);
	BN_free(&private_key_bn);
	BN_CTX_free(ctx);
	EC_KEY_free(key);

	return BITCOIN_SUCCESS;
}

BitcoinResult Bitcoin_MakeAddressFromPublicKey(
	struct BitcoinAddress *address,
	const struct BitcoinPublicKey *public_key
)
{
	struct BitcoinSHA256 public_key_hash1;
	struct BitcoinRIPEMD160 public_key_hash2;

	/* hash public key to make address */
	Bitcoin_SHA256(&public_key_hash1, public_key->data, BitcoinPublicKey_GetSize(public_key));
	Bitcoin_RIPEMD160(&public_key_hash2, public_key_hash1.data, BITCOIN_SHA256_SIZE);

	/* add version byte */
	memcpy(address->data+1, public_key_hash2.data, BITCOIN_RIPEMD160_SIZE);
	address->data[0] = 0;

	return BITCOIN_SUCCESS;
}

BitcoinResult Bitcoin_LoadPrivateKeyFromBase58(
	struct BitcoinPrivateKey *output_private_key,
	const char *input_text, size_t input_size
)
{
	char buffer[256];
	size_t size = sizeof(buffer);

	int error = Bitcoin_DecodeBase58Check(
		buffer
		,&size
		,input_text
		,input_size
	);

	if (error) {
		return error;
	}

	if (size == 0) {
		return BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT;
	}

	if (buffer[0] == '\x80') {
		if (size == 37) {
			/* uncompressed private key -
				1 byte version
				32 bytes private key
				4 bytes checksum
			*/
			memcpy(output_private_key->data, buffer+1, BITCOIN_PRIVATE_KEY_SIZE);
			output_private_key->public_key_compression = BITCOIN_PUBLIC_KEY_UNCOMPRESSED;
			return BITCOIN_SUCCESS;
		} else if (size == 38) {
			/* compressed private key -
				1 byte version
				32 bytes private key
				1 byte compression type
				4 bytes checksum
			*/
			unsigned compression_type = buffer[BITCOIN_PRIVATE_KEY_SIZE+1];

			if (compression_type != 1) {
				/* compression type must be 1 */
				fprintf(stderr,
					"%s: compression type was 0x%02x when it should have been 0x01",
					__func__,
					(unsigned)compression_type
				);
				return BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT;
			}
			memcpy(output_private_key->data, buffer+1, BITCOIN_PRIVATE_KEY_SIZE);
			output_private_key->public_key_compression = BITCOIN_PUBLIC_KEY_COMPRESSED;
			return BITCOIN_SUCCESS;
		}
	}

	return BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT;
}


BitcoinResult Bitcoin_LoadPublicKeyFromBase58(
	struct BitcoinPublicKey *output_public_key,
	const char *input_text, size_t input_size
)
{
/*
not implemented - does anyone use this?
*/
	return BITCOIN_ERROR_NOT_IMPLEMENTED;
}
