#ifndef BITCOIN_INCLUDE_PREFIX_H
#define BITCOIN_INCLUDE_PREFIX_H

#include <stdio.h>

/* prefix byte for addresses, public keys and private keys, to identify network */
typedef unsigned BitcoinKeyPrefix;

struct BitcoinNetworkType
{
	const char *name;
	const char *hrp;
	BitcoinKeyPrefix public_key_prefix,
		script_prefix,
		private_key_prefix;
};

const struct BitcoinNetworkType *Bitcoin_GetNetworkTypeByName(const char *name);
const struct BitcoinNetworkType *Bitcoin_GetNetworkTypeByHrp(const char *hrp);
const struct BitcoinNetworkType *Bitcoin_GetNetworkTypeByPrivateKeyPrefix(const BitcoinKeyPrefix prefix);

BitcoinKeyPrefix BitcoinNetworkType_GetPublicKeyPrefix(const struct BitcoinNetworkType *n);
BitcoinKeyPrefix BitcoinNetworkType_GetPrivateKeyPrefix(const struct BitcoinNetworkType *n);

void Bitcoin_ListNetworks(FILE *output);

#endif
