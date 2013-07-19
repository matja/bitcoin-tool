#include <string.h>

#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>

#include "ec.h"
#include "base58.h"

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

	BN_init(&private_key_bn);
	BN_bin2bn(private_key->data, BITCOIN_PRIVATE_KEY_SIZE, &private_key_bn);
	ec_public = EC_POINT_new(group);
	ctx = BN_CTX_new();
	EC_POINT_mul(group, ec_public, &private_key_bn, NULL, NULL, ctx);
	EC_KEY_set_private_key(key, &private_key_bn);
	EC_KEY_set_public_key(key, ec_public);

	EC_KEY_set_conv_form(key, POINT_CONVERSION_UNCOMPRESSED);
	size = i2o_ECPublicKey(key, NULL);
	if (size != BITCOIN_PUBLIC_KEY_SIZE) {
		BN_free(&private_key_bn);
		return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
	}

	size2 = i2o_ECPublicKey(key, &public_key_ptr);
	if (size2 != BITCOIN_PUBLIC_KEY_SIZE) {
		BN_free(&private_key_bn);
		return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
	}

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
	Bitcoin_SHA256(&public_key_hash1, public_key->data, BITCOIN_PUBLIC_KEY_SIZE);
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
			return BITCOIN_SUCCESS;
		} else if (size == 38) {
			/* compressed private key -
				1 byte version
				32 bytes private key
				1 byte compression type
				4 bytes checksum
			*/
			memcpy(output_private_key->data, buffer+1, BITCOIN_PRIVATE_KEY_SIZE);
			return BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT;
		}
	}

	return BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT;
}


BitcoinResult Bitcoin_LoadPublicKeyFromBase58(
	struct BitcoinPublicKey *output_public_key,
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
		return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
	}

	if (buffer[0] == '\x80') {
		if (size == 37) {
			/* uncompressed private key -
				1 byte version
				32 bytes private key
				4 bytes checksum
			*/
			memcpy(output_public_key->data, buffer+1, BITCOIN_PUBLIC_KEY_SIZE);
			return BITCOIN_SUCCESS;
		} else if (size == 38) {
			/* compressed private key -
				1 byte version
				32 bytes private key
				1 byte compression type
				4 bytes checksum
			*/
			memcpy(output_public_key->data, buffer+1, BITCOIN_PUBLIC_KEY_SIZE);
			return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
		}
	}

	return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
}


