/*
Elliptic curve secp256k1 related functions.

reference : https://en.bitcoin.it/wiki/Secp256k1
*/

#include <string.h>

#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>

#include "keys.h"
#include "base58.h"
#include "applog.h"
#include "hash.h"

int BitcoinPublicKey_Empty(const struct BitcoinPublicKey *public_key)
{
	return public_key->compression == BITCOIN_PUBLIC_KEY_EMPTY;
}

size_t BitcoinPublicKey_GetSize(const struct BitcoinPublicKey *public_key)
{
	switch (public_key->compression) {
		case BITCOIN_PUBLIC_KEY_COMPRESSED :
			return BITCOIN_PUBLIC_KEY_COMPRESSED_SIZE;
			break;
		case BITCOIN_PUBLIC_KEY_UNCOMPRESSED :
			return BITCOIN_PUBLIC_KEY_UNCOMPRESSED_SIZE;
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

size_t BitcoinPrivateKey_GetSize(const struct BitcoinPrivateKey *private_key)
{
	return BITCOIN_PRIVATE_KEY_SIZE;
}

size_t BitcoinPrivateKey_GetWIFSize(const struct BitcoinPrivateKey *private_key)
{
	switch (private_key->public_key_compression) {
		case BITCOIN_PUBLIC_KEY_COMPRESSED :
			return BITCOIN_PRIVATE_KEY_WIF_COMPRESSED_SIZE;
			break;
		case BITCOIN_PUBLIC_KEY_UNCOMPRESSED :
			return BITCOIN_PRIVATE_KEY_WIF_UNCOMPRESSED_SIZE;
			break;
		default :
			applog(APPLOG_ERROR, __func__,
				"invalid private_key->public_key_compression value (%u)",
				(unsigned)private_key->public_key_compression
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
	EC_KEY *key = NULL;
	EC_POINT *ec_public = NULL;
	unsigned char *public_key_ptr = public_key->data;
	BIGNUM private_key_bn;
	const EC_GROUP *group = NULL;
	int size, size2;
	unsigned compression = private_key->public_key_compression;
	size_t expected_public_key_size = 0;
	enum BitcoinPublicKeyCompression public_key_compression;

	switch (compression) {
		case BITCOIN_PUBLIC_KEY_COMPRESSED :
		case BITCOIN_PUBLIC_KEY_UNCOMPRESSED :
			break;
		default :
			applog(APPLOG_ERROR, __func__,
				"public key compression is not specified, please set using"
				" --public-key-compression compressed/uncompressed"
			);
			EC_KEY_free(key);
			return BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT;
			break;
	}

	key = EC_KEY_new_by_curve_name(NID_secp256k1);
	group = EC_KEY_get0_group(key);
	BN_init(&private_key_bn);
	BN_bin2bn(private_key->data, BITCOIN_PRIVATE_KEY_SIZE, &private_key_bn);
	ec_public = EC_POINT_new(group);

	ctx = BN_CTX_new();
	if (!ctx) {
		fprintf(stderr, "%s: BN_CTX_new failed\n", __func__);
		EC_KEY_free(key);
		return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
	}

	if (!EC_POINT_mul(group, ec_public, &private_key_bn, NULL, NULL, ctx)) {
		fprintf(stderr, "%s: EC_POINT_mul failed\n", __func__);
		EC_KEY_free(key);
		return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
	}

	EC_KEY_set_private_key(key, &private_key_bn);
	EC_KEY_set_public_key(key, ec_public);

	if (compression == BITCOIN_PUBLIC_KEY_COMPRESSED) {
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
		EC_KEY_free(key);
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
		EC_KEY_free(key);
		return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
	}

	/* public key appears to be valid by now, set the compression type */
	public_key->compression = public_key_compression;
	public_key->network_type = private_key->network_type;

	/* free resources */
	EC_POINT_clear_free(ec_public);
	BN_free(&private_key_bn);
	BN_CTX_free(ctx);
	EC_KEY_free(key);

	return BITCOIN_SUCCESS;
}
