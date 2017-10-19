/*
Elliptic curve secp256k1 related functions.

reference : https://en.bitcoin.it/wiki/Secp256k1
*/

#include <string.h>

#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/err.h>

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

typedef struct {
	/* either NID_X9_62_prime_field or NID_X9_62_characteristic_two_field */
	int field_type;
	int seed_len;
	int param_len;
	unsigned int cofactor; /* promoted to BN_ULONG */
} EC_CURVE_DATA;

static const struct {
	EC_CURVE_DATA h;
	unsigned char data[0+32*6];
} EC_SECG_PRIME_256K1 = {
	{ NID_X9_62_prime_field,0,32,1 },
	{                                                  /* no seed */
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, /* p */
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,
	0xFC,0x2F,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* a */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* b */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x07,
	0x79,0xBE,0x66,0x7E,0xF9,0xDC,0xBB,0xAC,0x55,0xA0, /* x */
	0x62,0x95,0xCE,0x87,0x0B,0x07,0x02,0x9B,0xFC,0xDB,
	0x2D,0xCE,0x28,0xD9,0x59,0xF2,0x81,0x5B,0x16,0xF8,
	0x17,0x98,
	0x48,0x3a,0xda,0x77,0x26,0xa3,0xc4,0x65,0x5d,0xa4, /* y */
	0xfb,0xfc,0x0e,0x11,0x08,0xa8,0xfd,0x17,0xb4,0x48,
	0xa6,0x85,0x54,0x19,0x9c,0x47,0xd0,0x8f,0xfb,0x10,
	0xd4,0xb8,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, /* order */
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0xBA,0xAE,0xDC,0xE6,
	0xAF,0x48,0xA0,0x3B,0xBF,0xD2,0x5E,0x8C,0xD0,0x36,
	0x41,0x41 }
};

static EC_GROUP *ec_group_new_from_data(const EC_CURVE_DATA *data)
{
	EC_GROUP *group = NULL;
	EC_POINT *P = NULL;
	BN_CTX *ctx = NULL;
	BIGNUM *p = NULL, *a = NULL, *b = NULL, *x = NULL, *y = NULL, *order = NULL;
	int ok = 0;
	int seed_len = 0;
	int param_len = 0;
	const unsigned char *params = NULL;

	ctx = BN_CTX_new();
	if (ctx == NULL) {
		ECerr(EC_F_EC_GROUP_NEW_FROM_DATA, ERR_R_MALLOC_FAILURE);
		goto err;
	}

	seed_len = data->seed_len;
	param_len = data->param_len;
	params = (const unsigned char *)(data+1); /* skip header */
	params += seed_len; /* skip seed   */

	if (
		   !(p = BN_bin2bn(params+0*param_len, param_len, NULL))
		|| !(a = BN_bin2bn(params+1*param_len, param_len, NULL))
		|| !(b = BN_bin2bn(params+2*param_len, param_len, NULL))
	) {
		ECerr(EC_F_EC_GROUP_NEW_FROM_DATA, ERR_R_BN_LIB);
		goto err;
	}

	if ((group = EC_GROUP_new_curve_GFp(p, a, b, ctx)) == NULL) {
		ECerr(EC_F_EC_GROUP_NEW_FROM_DATA, ERR_R_EC_LIB);
		goto err;
	}

	if ((P = EC_POINT_new(group)) == NULL) {
		ECerr(EC_F_EC_GROUP_NEW_FROM_DATA, ERR_R_EC_LIB);
	goto err;
	}

	if (
		   !(x = BN_bin2bn(params+3*param_len, param_len, NULL))
		|| !(y = BN_bin2bn(params+4*param_len, param_len, NULL))
	) {
		ECerr(EC_F_EC_GROUP_NEW_FROM_DATA, ERR_R_BN_LIB);
		goto err;
	}

	if (!EC_POINT_set_affine_coordinates_GFp(group, P, x, y, ctx)) {
		ECerr(EC_F_EC_GROUP_NEW_FROM_DATA, ERR_R_EC_LIB);
		goto err;
	}

	if (
		!(order = BN_bin2bn(params+5*param_len, param_len, NULL))
		|| !BN_set_word(x, (BN_ULONG)data->cofactor)
	) {
		ECerr(EC_F_EC_GROUP_NEW_FROM_DATA, ERR_R_BN_LIB);
		goto err;
	}

	if (!EC_GROUP_set_generator(group, P, order, x)) {
		ECerr(EC_F_EC_GROUP_NEW_FROM_DATA, ERR_R_EC_LIB);
		goto err;
	}

	if (seed_len) {
		if (!EC_GROUP_set_seed(group, params-seed_len, seed_len)) {
			ECerr(EC_F_EC_GROUP_NEW_FROM_DATA, ERR_R_EC_LIB);
			goto err;
		}
	}

	ok = 1;
err:
	if (!ok) {
		EC_GROUP_free(group);
		group = NULL;
	}

	if (P) { EC_POINT_free(P); }
	if (ctx) { BN_CTX_free(ctx); }
	if (p) { BN_free(p); }
	if (a) { BN_free(a); }
	if (b) { BN_free(b); }
	if (order) { BN_free(order);}
	if (x) { BN_free(x); }
	if (y) { BN_free(y); }
	return group;
}

EC_KEY *EC_KEY_new_by_curve_name_NID_secp256k1(void)
{
	static EC_GROUP *group = NULL;
	EC_KEY *ret = NULL;

	if (group == NULL) {
#ifdef HAVE_NID_secp256k1
		group = EC_GROUP_new_by_curve_name(NID_secp256k1);
#else
		group = ec_group_new_from_data(&EC_SECG_PRIME_256K1.h);
#endif
		if (group == NULL) {
			return NULL;
		}
	}

	ret = EC_KEY_new();
	if (ret == NULL) {
		return NULL;
	}

	EC_KEY_set_group(ret, group);

	return ret;
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
	BIGNUM *private_key_bn;
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

	key = EC_KEY_new_by_curve_name_NID_secp256k1();
	if (!key) {
		applog(APPLOG_ERROR, __func__,
			"EC_KEY_new_by_curve_name failed: %s",
			ERR_error_string(ERR_get_error(), NULL)
		);
		return BITCOIN_ERROR_LIBRARY_FAILURE;
	}

	group = EC_KEY_get0_group(key);
	if (!group) {
		applog(APPLOG_ERROR, __func__,
			"EC_KEY_get0_group failed: %s",
			ERR_error_string(ERR_get_error(), NULL)
		);
		EC_KEY_free(key);
		return BITCOIN_ERROR_LIBRARY_FAILURE;
	}

	private_key_bn = BN_new();
	BN_bin2bn(private_key->data, BITCOIN_PRIVATE_KEY_SIZE, private_key_bn);
	ec_public = EC_POINT_new(group);

	ctx = BN_CTX_new();
	if (!ctx) {
		applog(APPLOG_ERROR, __func__,
			"BN_CTX_new failed: %s",
			ERR_error_string(ERR_get_error(), NULL)
		);
		EC_KEY_free(key);
		return BITCOIN_ERROR_LIBRARY_FAILURE;
	}

	if (!EC_POINT_mul(group, ec_public, private_key_bn, NULL, NULL, ctx)) {
		applog(APPLOG_ERROR, __func__,
			"EC_POINT_mul failed: %s",
			ERR_error_string(ERR_get_error(), NULL)
		);
		EC_KEY_free(key);
		return BITCOIN_ERROR_LIBRARY_FAILURE;
	}

	EC_KEY_set_private_key(key, private_key_bn);
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
		BN_free(private_key_bn);
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
		BN_free(private_key_bn);
		EC_KEY_free(key);
		return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
	}

	/* public key appears to be valid by now, set the compression type */
	public_key->compression = public_key_compression;
	public_key->network_type = private_key->network_type;

	/* free resources */
	EC_POINT_clear_free(ec_public);
	BN_free(private_key_bn);
	BN_CTX_free(ctx);
	EC_KEY_free(key);

	return BITCOIN_SUCCESS;
}
