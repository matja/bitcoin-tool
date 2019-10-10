
#include "prefix.h"

#include <string.h>

static const struct BitcoinNetworkType network_types[] = {
	/* Bitcoin */
	{
		.name                    = "bitcoin",
		.hrp                     = "bc", /* https://github.com/bitcoin/bitcoin/blob/v0.18.1/src/chainparams.cpp#L138 */
		.public_key_prefix       = 0,    /* https://github.com/bitcoin/bitcoin/blob/v0.18.1/src/chainparams.cpp#L132 */
		.script_prefix           = 5,    /* https://github.com/bitcoin/bitcoin/blob/v0.18.1/src/chainparams.cpp#L133 */
		.private_key_prefix      = 128   /* https://github.com/bitcoin/bitcoin/blob/v0.18.1/src/chainparams.cpp#L134 */
	},
	{
		.name                    = "bitcoin-testnet",
		.hrp                     = "tb", /* https://github.com/bitcoin/bitcoin/blob/v0.18.1/src/chainparams.cpp#L244 */
		.public_key_prefix       = 111,  /* https://github.com/bitcoin/bitcoin/blob/v0.18.1/src/chainparams.cpp#L238 */
		.script_prefix           = 196,  /* https://github.com/bitcoin/bitcoin/blob/v0.18.1/src/chainparams.cpp#L239 */
		.private_key_prefix      = 239   /* https://github.com/bitcoin/bitcoin/blob/v0.18.1/src/chainparams.cpp#L240 */
	},
	/* Litecoin */
	{
		.name                    = "litecoin",
		.hrp                     = "ltc", /* https://github.com/litecoin-project/litecoin/blob/v0.17.1/src/chainparams.cpp#L145 */
		.public_key_prefix       = 48,    /* https://github.com/litecoin-project/litecoin/blob/v0.17.1/src/chainparams.cpp#L138 */
		.script_prefix           = 5,     /* https://github.com/litecoin-project/litecoin/blob/v0.17.1/src/chainparams.cpp#L139 */
		.private_key_prefix      = 48+128 /* https://github.com/litecoin-project/litecoin/blob/v0.17.1/src/chainparams.cpp#L141 */
	},
	{
		.name                    = "litecoin-testnet",
		.hrp                     = "tltc", /* https://github.com/litecoin-project/litecoin/blob/v0.17.1/src/chainparams.cpp#L252 */
		.public_key_prefix       = 111,    /* https://github.com/litecoin-project/litecoin/blob/v0.17.1/src/chainparams.cpp#L345 */
		.script_prefix           = 196,    /* https://github.com/litecoin-project/litecoin/blob/v0.17.1/src/chainparams.cpp#L346 */
		.private_key_prefix      = 111+128 /* https://github.com/litecoin-project/litecoin/blob/v0.17.1/src/chainparams.cpp#L348 */
	},
	/* Feathercoin */
	{
		.name                    = "feathercoin",
		.hrp                     = "fc",  /* https://github.com/FeatherCoin/Feathercoin/blob/0.18/src/chainparams.cpp#L138 */
		.public_key_prefix       = 14,    /* https://github.com/FeatherCoin/Feathercoin/blob/0.18/src/chainparams.cpp#L132 */
		.script_prefix           = 5,     /* https://github.com/FeatherCoin/Feathercoin/blob/0.18/src/chainparams.cpp#L133 */
		.private_key_prefix      = 14+128 /* https://github.com/FeatherCoin/Feathercoin/blob/0.18/src/chainparams.cpp#L134 */
	},
	{
		.name                    = "feathercoin-testnet",
		.hrp                     = "tfc",  /* TODO: Add reference. */
		.public_key_prefix       = 111,    /* TODO: Add reference. */
		.script_prefix           = 196,    /* TODO: Add reference. */
		.private_key_prefix      = 111+128 /* TODO: Add reference. */
	},
	/* Dogecoin */
	{
		.name                    = "dogecoin",
		.hrp                     = "dc",   /* TODO: Add reference. */
		.public_key_prefix       = 30,     /* https://github.com/dogecoin/dogecoin/blob/v1.14.1/src/chainparams.cpp#L167 */
		.script_prefix           = 22,     /* https://github.com/dogecoin/dogecoin/blob/v1.14.1/src/chainparams.cpp#L168 */
		.private_key_prefix      = 30+128  /* https://github.com/dogecoin/dogecoin/blob/v1.14.1/src/chainparams.cpp#L169 */
	},
	{
		.name                    = "dogecoin-testnet",
		.hrp                     = "tdc",  /* TODO: Add reference. */
		.public_key_prefix       = 113,    /* https://github.com/dogecoin/dogecoin/blob/v1.14.1/src/chainparams.cpp#L320 */
		.script_prefix           = 196,    /* https://github.com/dogecoin/dogecoin/blob/v1.14.1/src/chainparams.cpp#L321 */
		.private_key_prefix      = 113+128 /* https://github.com/dogecoin/dogecoin/blob/v1.14.1/src/chainparams.cpp#L322 */
	},
	/* Quarkcoin */
	{
		.name                    = "quarkcoin",
		.hrp                     = "qc",  /* TODO: Add reference. */
		.public_key_prefix       = 58,    /* https://github.com/MaxGuevara/quark/blob/v0.10.7.4/src/chainparams.cpp#L202 */
		.script_prefix           = 9,     /* https://github.com/MaxGuevara/quark/blob/v0.10.7.4/src/chainparams.cpp#L203 */
		.private_key_prefix      = 58+128 /* https://github.com/MaxGuevara/quark/blob/v0.10.7.4/src/chainparams.cpp#L204 */
	},
	{
		.name                    = "quarkcoin-testnet",
		.hrp                     = "tqc",  /* TODO: Add reference. */
		.public_key_prefix       = 119,    /* https://github.com/MaxGuevara/quark/blob/v0.10.7.4/src/chainparams.cpp#L280 */
		.script_prefix           = 199,    /* https://github.com/MaxGuevara/quark/blob/v0.10.7.4/src/chainparams.cpp#L281 */
		.private_key_prefix      = 119+128 /* https://github.com/MaxGuevara/quark/blob/v0.10.7.4/src/chainparams.cpp#L282 */
	},
	/* Darkcoin */
	{
		.name                    = "darkcoin",
		.hrp                     = "xc",  /* TODO: Add reference. */
		.public_key_prefix       = 76,    /* FIXME: (dead link, abandoned chain?) https://github.com/darkcoinproject/darkcoin/blob/master/src/base58.h#L275 */
		.script_prefix           = 9,     /* TODO: Add reference. */
		.private_key_prefix      = 76+128 /* FIXME: (dead link, abandoned chain?) https://github.com/darkcoinproject/darkcoin/blob/master/src/base58.h#L403 */
	},
	{
		.name                    = "darkcoin-testnet",
		.hrp                     = "txc",  /* TODO: Add reference. */
		.public_key_prefix       = 111,    /* TODO: Add reference. */
		.script_prefix           = 196,    /* TODO: Add reference. */
		.private_key_prefix      = 111+128 /* TODO: Add reference. */
	},
	/* Jumbucks */
	{
		.name                    = "jumbucks",
		.hrp                     = "jb",  /* TODO: Add reference. */
		.public_key_prefix       = 43,    /* https://github.com/jyap808/jumbucks/blob/v1.9/src/base58.h#L276 */
		.script_prefix           = 105,   /* https://github.com/jyap808/jumbucks/blob/v1.9/src/base58.h#L277 */
		.private_key_prefix      = 43+128 /* https://github.com/jyap808/jumbucks/blob/v1.9/src/base58.h#L425 */
	},
	{
		.name                    = "jumbucks-testnet",
		.hrp                     = "tjb",  /* TODO: Add reference. */
		.public_key_prefix       = 107,    /* https://github.com/jyap808/jumbucks/blob/v1.9/src/base58.h#L278 */
		.script_prefix           = 176,    /* https://github.com/jyap808/jumbucks/blob/v1.9/src/base58.h#L279 */
		.private_key_prefix      = 107+128 /* https://github.com/jyap808/jumbucks/blob/v1.9/src/base58.h#L428 */
	},
	/* Peercoin */
	{
		.name                    = "peercoin",
		.hrp                     = "pc",  /* https://github.com/peercoin/peercoin/blob/v0.8.4ppc/src/chainparams.cpp#L135 */
		.public_key_prefix       = 55,    /* https://github.com/peercoin/peercoin/blob/v0.8.4ppc/src/chainparams.cpp#L128 */
		.script_prefix           = 117,   /* https://github.com/peercoin/peercoin/blob/v0.8.4ppc/src/chainparams.cpp#L129 */
		.private_key_prefix      = 55+128 /* https://github.com/peercoin/peercoin/blob/v0.8.4ppc/src/chainparams.cpp#L130 */
	},
	{
		.name                    = "peercoin-testnet",
		.hrp                     = "tpc",  /* https://github.com/peercoin/peercoin/blob/v0.8.4ppc/src/chainparams.cpp#L230 */
		.public_key_prefix       = 111,    /* https://github.com/peercoin/peercoin/blob/v0.8.4ppc/src/chainparams.cpp#L223 */
		.script_prefix           = 196,    /* https://github.com/peercoin/peercoin/blob/v0.8.4ppc/src/chainparams.cpp#L224 */
		.private_key_prefix      = 111+128 /* https://github.com/peercoin/peercoin/blob/v0.8.4ppc/src/chainparams.cpp#L225 */
	},
	/* Namecoin */
	{
		.name                    = "namecoin",
		.hrp                     = "nc",  /* https://github.com/namecoin/namecoin-core/blob/nc0.18.1/src/chainparams.cpp#L165 */
		.public_key_prefix       = 52,    /* https://github.com/namecoin/namecoin-core/blob/nc0.18.1/src/chainparams.cpp#L158 */
		.script_prefix           = 13,    /* https://github.com/namecoin/namecoin-core/blob/nc0.18.1/src/chainparams.cpp#L159 */
		.private_key_prefix      = 52+128 /* https://github.com/namecoin/namecoin-core/blob/nc0.18.1/src/chainparams.cpp#L160 */
	},
	{
		.name                    = "namecoin-testnet",
		.hrp                     = "tn",   /* https://github.com/namecoin/namecoin-core/blob/nc0.18.1/src/chainparams.cpp#L322 */
		.public_key_prefix       = 111,    /* https://github.com/namecoin/namecoin-core/blob/nc0.18.1/src/chainparams.cpp#L439 */
		.script_prefix           = 196,    /* https://github.com/namecoin/namecoin-core/blob/nc0.18.1/src/chainparams.cpp#L440 */
		.private_key_prefix      = 111+128 /* https://github.com/namecoin/namecoin-core/blob/nc0.18.1/src/chainparams.cpp#L441 */
	},
	/* Komodo */
	{
		.name                    = "komodo",
		.hrp                     = "zs",  /* https://github.com/jl777/komodo/blob/master/src/chainparams.cpp#L202 */
		.public_key_prefix       = 60,    /* https://github.com/jl777/komodo/blob/master/src/chainparams.cpp#L190 */
		.script_prefix           = 85,    /* https://github.com/jl777/komodo/blob/master/src/chainparams.cpp#L191 */
		.private_key_prefix      = 60+128 /* https://github.com/jl777/komodo/blob/master/src/chainparams.cpp#L192 */
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

const struct BitcoinNetworkType *Bitcoin_GetNetworkTypeByHrp(const char *hrp)
{
	const struct BitcoinNetworkType *pn = network_types;

	while (pn != network_types + (sizeof(network_types)/sizeof(network_types[0]))) {
		if (strcmp(hrp, pn->hrp) == 0) {
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
