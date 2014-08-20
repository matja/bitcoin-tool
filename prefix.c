
#include "prefix.h"

#include <string.h>

static const struct BitcoinNetworkType network_types[] = {
/*
Bitcoin:
https://en.bitcoin.it/wiki/List_of_address_prefixes
Peercoin, Primecoin and Zetacoin use the same constants.
*/
	{
		.name                    = "bitcoin",
		.public_key_prefix       = 0,
		.script_prefix           = 5,
		.private_key_prefix      = 128
	},
	{
		.name                    = "bitcoin-testnet",
		.public_key_prefix       = 111,
		.script_prefix           = 196,
		.private_key_prefix      = 239
	},
/*
Litecoin:
public keys / script:
https://github.com/litecoin-project/litecoin/blob/88e2a2e8988b89f905145bdc9af8c34028d0af90/src/base58.h#L275
private keys:
https://github.com/litecoin-project/litecoin/blob/88e2a2e8988b89f905145bdc9af8c34028d0af90/src/base58.h#L403
*/
	{
		.name                    = "litecoin",
		.public_key_prefix       = 48,
		.script_prefix           = 5,
		.private_key_prefix      = 48+128
	},
	{
		.name                    = "litecoin-testnet",
		.public_key_prefix       = 111,
		.script_prefix           = 196,
		.private_key_prefix      = 111+128
	},
/*
Feathercoin:
public keys / script:
https://github.com/FeatherCoin/Feathercoin-0.8.5/blob/master-0.8/src/base58.h#L275
private keys:
https://github.com/FeatherCoin/Feathercoin-0.8.5/blob/master-0.8/src/base58.h#L403
*/
	{
		.name                    = "feathercoin",
		.public_key_prefix       = 14,
		.script_prefix           = 5,
		.private_key_prefix      = 14+128
	},
	{
		.name                    = "feathercoin-testnet",
		.public_key_prefix       = 111,
		.script_prefix           = 196,
		.private_key_prefix      = 111+128
	},
/*
Dogecoin:
public keys / script:
https://github.com/dogecoin/dogecoin/blob/v1.5.0/src/base58.h#L276
private keys:
https://github.com/dogecoin/dogecoin/blob/v1.5.0/src/base58.h#L404
*/
	{
		.name                    = "dogecoin",
		.public_key_prefix       = 30,
		.script_prefix           = 22,
		.private_key_prefix      = 30+128
	},
	{
		.name                    = "dogecoin-testnet",
		.public_key_prefix       = 113,
		.script_prefix           = 196,
		.private_key_prefix      = 113+128
	},
/*
Quarkcoin:
public keys / script:
https://github.com/MaxGuevara/quark/blob/v0.8.3.20/src/base58.h#L275
private keys:
https://github.com/MaxGuevara/quark/blob/v0.8.3.20/src/base58.h#L403
*/
	{
		.name                    = "quarkcoin",
		.public_key_prefix       = 58,
		.script_prefix           = 9,
		.private_key_prefix      = 58+128
	},
	{
		.name                    = "quarkcoin-testnet",
		.public_key_prefix       = 119,
		.script_prefix           = 199,
		.private_key_prefix      = 119+128
	},
/*
Darkcoin:
public keys / script:
https://github.com/darkcoinproject/darkcoin/blob/master/src/base58.h#L275
private keys:
https://github.com/darkcoinproject/darkcoin/blob/master/src/base58.h#L403
*/
	{
		.name                    = "darkcoin",
		.public_key_prefix       = 76,
		.script_prefix           = 9,
		.private_key_prefix      = 76+128
	},
	{
		.name                    = "darkcoin-testnet",
		.public_key_prefix       = 111,
		.script_prefix           = 196,
		.private_key_prefix      = 111+128
	}
};

BitcoinKeyPrefix BitcoinNetworkType_GetPublicKeyPrefix(const struct BitcoinNetworkType *n)
{
	return n->public_key_prefix;
}

BitcoinKeyPrefix BitcoinNetworkType_GetPrivateKeyPrefix(const struct BitcoinNetworkType *n)
{
	return n->private_key_prefix;
}

const struct BitcoinNetworkType *Bitcoin_GetNetworkTypeByName(const char *name)
{
	const struct BitcoinNetworkType *pn = network_types;

	while (pn != network_types + (sizeof(network_types)/sizeof(network_types[0]))) {
		if (strcmp(name, pn->name) == 0) {
			return pn;
		}
		pn++;
	}

	return NULL;
}

const struct BitcoinNetworkType *Bitcoin_GetNetworkTypeByPrivateKeyPrefix(const BitcoinKeyPrefix prefix)
{
	const struct BitcoinNetworkType *pn = network_types;

	while (pn != network_types + (sizeof(network_types)/sizeof(network_types[0]))) {
		if (prefix == pn->private_key_prefix) {
			return pn;
		}
		pn++;
	}

	return NULL;
}

void Bitcoin_ListNetworks(FILE *output)
{
	const struct BitcoinNetworkType *pn = network_types;
	static const char indent[] = "      ";

	while (pn != network_types + (sizeof(network_types)/sizeof(network_types[0]))) {
		fprintf(output, "%s%s\n", indent, pn->name);
		pn++;
	}
}
