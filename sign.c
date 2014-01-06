
BitcoinResult Bitcoin_VerifySignature(
	const void *message,
	size_t message_size,
	const void *signature,
	size_t signature_size,
	EC_KEY *public_key
)
{


	return BITCOIN_SUCCESS;
}

BitcoinResult Bitcoin_CreateSignature(
	const void *message,
	size_t message_size,
	EC_KEY *private_key
)
{
	ECDSA_SIG *sig = NULL;
	struct BitcoinSHA256 digest;
	unsigned char *encoded_sig_ptr = NULL;
	unsigned char *encoded_sig = NULL;
	const size_t digest_size = BITCOIN_SHA256_SIZE;
	int max_encoded_sig_size = 0, encoded_sig_size = 0;

	/* hash the message using SHA256 into the digest */
	Bitcoin_SHA256(&digest, message, message_size);

	/* sign the digest using the provided public key */
	sig = ECDSA_do_sign((const unsigned char *)&digest.data,
		digest_size, private_key);
	if (!sig) {
		applog(APPLOG_ERROR, __func__, "ECDSA_do_sign failed");
		return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
	}

	/* get the maximum size of the DER representation of the signature */
	max_encoded_sig_size = ECDSA_size(private_key);
	if (max_encoded_sig_size == 0 || max_encoded_sig_size >= 1024) {
		applog(APPLOG_ERROR, __func__,
			"ECDSA_size returned unusual value (%d)",
			max_encoded_sig_size
		);
		return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
	}

	/* allocate a buffer large enough for the DER signature */
	encoded_sig = malloc(max_encoded_sig_size);
	encoded_sig_ptr = encoded_sig;

	/* get the DER encoded signature */
	encoded_sig_size = i2d_ECDSA_SIG(sig, &encoded_sig_ptr);

	Bitcoin_OutputHexPretty(encoded_sig, encoded_sig_size);

	applog(APPLOG_NOTICE, __func__, "signing seemed to work");

	ECDSA_SIG_free(sig);
	free(encoded_sig);

	return BITCOIN_SUCCESS;
}
