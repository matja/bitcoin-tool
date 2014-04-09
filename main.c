/* FIXME: this file is getting too long and ugly, I need to cut it up a bit */

#define _POSIX_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#include "hash.h"
#include "keys.h"
#include "utility.h"
#include "base58.h"
#include "applog.h"
#include "result.h"
#include "prefix.h"

#define BITCOINTOOL_OPTION_DEFAULT_BASE58CHECK_CHANGE_CHARS 3
#define BITCOINTOOL_OPTION_DEFAULT_BASE58CHECK_INSERT_CHARS 3
#define BITCOINTOOL_OPTION_DEFAULT_BASE58CHECK_REMOVE_CHARS 3

typedef struct BitcoinTool BitcoinTool;
typedef struct BitcoinToolOptions BitcoinToolOptions;

struct BitcoinToolOptions {
	const char *input;
	const char *input_file;

	enum InputType {
		INPUT_TYPE_NONE,
		INPUT_TYPE_ADDRESS,
		INPUT_TYPE_PUBLIC_KEY_RIPEMD160,
		INPUT_TYPE_PUBLIC_KEY_SHA256,
		INPUT_TYPE_PUBLIC_KEY,
		INPUT_TYPE_PRIVATE_KEY_WIF,
		INPUT_TYPE_PRIVATE_KEY,
		INPUT_TYPE_MINI_PRIVATE_KEY
	} input_type;

	enum InputFormat {
		INPUT_FORMAT_NONE,
		INPUT_FORMAT_RAW,
		INPUT_FORMAT_HEX,
		INPUT_FORMAT_BASE58,
		INPUT_FORMAT_BASE58CHECK
	} input_format;

	enum OutputType {
		OUTPUT_TYPE_NONE,
		OUTPUT_TYPE_ALL,
		OUTPUT_TYPE_ADDRESS,
		OUTPUT_TYPE_PUBLIC_KEY_RIPEMD160,
		OUTPUT_TYPE_PUBLIC_KEY_SHA256,
		OUTPUT_TYPE_PUBLIC_KEY,
		OUTPUT_TYPE_PRIVATE_KEY_WIF,
		OUTPUT_TYPE_PRIVATE_KEY
	} output_type;

	enum OutputFormat {
		OUTPUT_FORMAT_NONE,
		OUTPUT_FORMAT_RAW,
		OUTPUT_FORMAT_HEX,
		OUTPUT_FORMAT_BASE58,
		OUTPUT_FORMAT_BASE58CHECK
	} output_format;

	enum PublicKeyCompression {
		PUBLIC_KEY_COMPRESSION_AUTO,
		PUBLIC_KEY_COMPRESSION_COMPRESSED,
		PUBLIC_KEY_COMPRESSION_UNCOMPRESSED
	} public_key_compression;

	/* attempt to fix invalid base58check encoded inputs? */
	unsigned fix_base58;

	/* maximum number of characters to change */
	unsigned fix_base58_change_chars;

	/* maximum number of characters to insert */
	unsigned fix_base58_insert_chars;

	/* maximum number of characters to remove */
	unsigned fix_base58_remove_chars;

	/* set the network type prefix of addresses, public keys and private keys */
	const struct BitcoinNetworkType *network_type;
};

struct BitcoinTool {
	struct BitcoinToolOptions options;

	char mini_private_key[BITCOIN_MINI_PRIVATE_KEY_SIZE];
	struct BitcoinPrivateKey private_key;
	struct BitcoinPublicKey public_key;
	struct BitcoinSHA256 public_key_sha256;
	struct BitcoinRIPEMD160 public_key_ripemd160;
	struct BitcoinAddress address;

	/* flag the input types as being set if we load or convert into them */
	int mini_private_key_set,
		private_key_set,
		private_key_wif_set,
		public_key_set,
		public_key_sha256_set,
		public_key_ripemd160_set,
		address_set;

	char input[256]; /* input provided by user on command line or from file */
	size_t input_size;

	uint8_t input_raw[256]; /* input format converted to raw input type */
	size_t input_raw_size;

	uint8_t output_raw[256]; /* raw input type converted to raw output type */
	size_t output_raw_size;

	char output_text[256]; /* raw output type converted to output format */
	size_t output_text_size;

	int (*parseOptions)(struct BitcoinTool *self, int argc, char *argv[]);
	void (*help)(struct BitcoinTool *self);
	int (*run)(struct BitcoinTool *self);
	void (*destroy)(struct BitcoinTool *self);
};

static void BitcoinTool_ListInputTypes(FILE *output)
{
	static const char indent[] = "      ";
	fprintf(output, "%smini-private-key : 30 character Casascius mini private key\n", indent);
	fprintf(output, "%sprivate-key      : 32 byte ECDSA private key\n", indent);
	fprintf(output, "%sprivate-key-wif  : 33/34 byte ECDSA WIF private key\n", indent);
	fprintf(output, "%spublic-key       : 33/65 byte ECDSA public key\n", indent);
	fprintf(output, "%spublic-key-sha   : 32 byte SHA256(public key) hash\n", indent);
	fprintf(output, "%spublic-key-rmd   : 20 byte RIPEMD160(SHA256(public key)) hash\n", indent);
	fprintf(output, "%saddress          : 21 byte Bitcoin address (prefix + hash)\n", indent);
}

static void BitcoinTool_ListOutputTypes(FILE *output)
{
	static const char indent[] = "      ";
	fprintf(output, "%sall              : All output types, as type:value pairs, most of which\n", indent);
	fprintf(output, "%s                   are never commonly used, probably for good reason.\n", indent);
	BitcoinTool_ListInputTypes(output);
}

static void BitcoinTool_ListInputFormats(FILE *output)
{
	static const char indent[] = "      ";
	fprintf(output, "%sraw         : Raw binary\n", indent);
	fprintf(output, "%shex         : Hexadecimal encoded\n", indent);
	fprintf(output, "%sbase58      : Base58 encoded\n", indent);
	fprintf(output, "%sbase58check : Base58Check encoded (most common)\n", indent);
}

static void BitcoinTool_ListOutputFormats(FILE *output)
{
	BitcoinTool_ListInputFormats(output);
}

static void BitcoinTool_help(BitcoinTool *self)
{
	FILE *file = stderr;
	fprintf(file,
		"Usage: bitcoin-tool [option]...\n"
		"Convert Bitcoin keys and addresses.\n"
		"\n"
	);
	fprintf(file,
		"  --input-type : Input data type, must be one of :\n"
	);
	BitcoinTool_ListInputTypes(file);

	fprintf(file,
		"  --input-format : Input data format, must be one of :\n"
	);
	BitcoinTool_ListInputFormats(file);

	fprintf(file,
		"  --output-type  : Output data type, must be one of :\n"
	);
	BitcoinTool_ListOutputTypes(file);

	fprintf(file,
		"  --output-format : Output data format, must be one of :\n"
	);
	BitcoinTool_ListOutputFormats(file);

	fprintf(file,
		"  --input         : Specify input data on command line\n"
		"  --input-file    : Specify file name to read for input\n"
	);
	fprintf(file,
		"  --public-key-compression : Can be one of :\n"
		"      auto         : determine compression from base58 private key (default)\n"
		"      compressed   : force compressed public key\n"
		"      uncompressed : force uncompressed public key\n"
		"    (must be specified for raw/hex keys, should be auto for base58)\n"
	);
	fprintf(file,
		"  --network        : Network type of keys, one of :\n"
	);
	Bitcoin_ListNetworks(file);

	fprintf(file,
		"  --fix-base58check : Attempt to fix a Base58Check string by changing\n"
		"                      characters until the checksum matches.\n"
	);
	fprintf(file,
		"  --fix-base58check-change-chars : Maximum number of characters to change\n"
		"                                   (default=%u)\n",
		BITCOINTOOL_OPTION_DEFAULT_BASE58CHECK_CHANGE_CHARS
	);
	fprintf(file,
		"\n"
	);
	fprintf(file,
		"Examples:\n"
	);
	fprintf(file,
		"  Show address for specified WIF private key\n"
		"    --input-type private-key-wif \\\n"
		"    --input-format base58check \\\n"
		"    --input 5J2YUwNA5hmZFW33nbUCp5TmvszYXxVYthqDv7axSisBjFJMqaT \\\n"
		"    --output-type address \\\n"
		"    --output-format base58check \n"
		"\n"
	);
	fprintf(file,
		"  Show everything that a raw private key can be converted to\n"
		"    --input-type private-key \\\n"
		"    --input-format raw \\\n"
		"    --input-file <(openssl rand 32) \\\n"
		"    --output-type all \\\n"
		"    --public-key-compression compressed \\\n"
		"    --network bitcoin\n"
		"\n"
	);
}

static int BitcoinTool_parseOptions(BitcoinTool *self
	,int argc
	,char *argv[]
)
{
	unsigned i = 0;
	int errors = 0;
	BitcoinToolOptions *o = &self->options;

	/* detect key compression where possible */
	o->public_key_compression = PUBLIC_KEY_COMPRESSION_AUTO;

	/* fail-safe network type - don't assume Bitcoin for raw keys */
	o->network_type = NULL;

	for (i=1; i<argc; i++) {
		const char *a = argv[i];
		const char *v = NULL;

		if (!strcmp(a, "--input-type")) {
			if (++i >= argc) {
				applog(APPLOG_ERROR, __func__, "Missing value for %s, must be one of:", a);
				BitcoinTool_ListInputTypes(stderr);
				errors++;
				break;
			}
			v = argv[i];
			if (!strcmp(v, "address")) {
				o->input_type = INPUT_TYPE_ADDRESS;
			} else if (!strcmp(v, "public-key-rmd")) {
				o->input_type = INPUT_TYPE_PUBLIC_KEY_RIPEMD160;
			} else if (!strcmp(v, "public-key-sha")) {
				o->input_type = INPUT_TYPE_PUBLIC_KEY_SHA256;
			} else if (!strcmp(v, "public-key")) {
				o->input_type = INPUT_TYPE_PUBLIC_KEY;
			} else if (!strcmp(v, "private-key-wif")) {
				o->input_type = INPUT_TYPE_PRIVATE_KEY_WIF;
			} else if (!strcmp(v, "private-key")) {
				o->input_type = INPUT_TYPE_PRIVATE_KEY;
			} else if (!strcmp(v, "mini-private-key")) {
				o->input_type = INPUT_TYPE_MINI_PRIVATE_KEY;
			} else {
				applog(APPLOG_ERROR, __func__,
					"Unknown value \"%s\" for --input-type, must be one of:", v
				);
				BitcoinTool_ListInputTypes(stderr);
				errors++;
				break;
			}
		} else if (!strcmp(a, "--output-type")) {
			if (++i >= argc) {
				applog(APPLOG_ERROR, __func__, "Missing value for %s, must be one of:", a);
				BitcoinTool_ListOutputTypes(stderr);
				errors++;
				break;
			}
			v = argv[i];
			if (!strcmp(v, "address")) {
				o->output_type = OUTPUT_TYPE_ADDRESS;
			} else if (!strcmp(v, "public-key-rmd")) {
				o->output_type = OUTPUT_TYPE_PUBLIC_KEY_RIPEMD160;
			} else if (!strcmp(v, "public-key-sha")) {
				o->output_type = OUTPUT_TYPE_PUBLIC_KEY_SHA256;
			} else if (!strcmp(v, "public-key")) {
				o->output_type = OUTPUT_TYPE_PUBLIC_KEY;
			} else if (!strcmp(v, "private-key-wif")) {
				o->output_type = OUTPUT_TYPE_PRIVATE_KEY_WIF;
			} else if (!strcmp(v, "private-key")) {
				o->output_type = OUTPUT_TYPE_PRIVATE_KEY;
			} else if (!strcmp(v, "all")) {
				o->output_type = OUTPUT_TYPE_ALL;
			} else {
				applog(APPLOG_ERROR, __func__,
					"Unknown value \"%s\" for --output-type", v
				);
				BitcoinTool_ListOutputTypes(stderr);
				errors++;
				break;
			}
		} else if (!strcmp(a, "--input-format")) {
			if (++i >= argc) {
				applog(APPLOG_ERROR, __func__, "Missing value for %s, must be one of:", a);
				BitcoinTool_ListInputFormats(stderr);
				errors++;
				break;
			}
			v = argv[i];
			if (!strcmp(v, "raw")) {
				o->input_format = INPUT_FORMAT_RAW;
			} else if (!strcmp(v, "hex")) {
				o->input_format = INPUT_FORMAT_HEX;
			} else if (!strcmp(v, "base58")) {
				o->input_format = INPUT_FORMAT_BASE58;
			} else if (!strcmp(v, "base58check")) {
				o->input_format = INPUT_FORMAT_BASE58CHECK;
			} else {
				applog(APPLOG_ERROR, __func__,
					"Unknown value \"%s\" for --input-format, must be one of:", v
				);
				BitcoinTool_ListInputFormats(stderr);
				errors++;
				break;
			}
		} else if (!strcmp(a, "--output-format")) {
			if (++i >= argc) {
				applog(APPLOG_ERROR, __func__, "Missing value for %s, must be one of:", a);
				BitcoinTool_ListOutputFormats(stderr);
				errors++;
				break;
			}
			v = argv[i];
			if (!strcmp(v, "raw")) {
				o->output_format = OUTPUT_FORMAT_RAW;
			} else if (!strcmp(v, "hex")) {
				o->output_format = OUTPUT_FORMAT_HEX;
			} else if (!strcmp(v, "base58")) {
				o->output_format = OUTPUT_FORMAT_BASE58;
			} else if (!strcmp(v, "base58check")) {
				o->output_format = OUTPUT_FORMAT_BASE58CHECK;
			} else {
				applog(APPLOG_ERROR, __func__,
					"Unknown value \"%s\" for --output-format, must be one of:", v
				);
				BitcoinTool_ListOutputFormats(stderr);
				errors++;
				break;
			}
		} else if (!strcmp(a, "--public-key-compression")) {
			if (++i >= argc) {
				applog(APPLOG_ERROR, __func__, "Missing value for %s.", a);
				return 0;
			}
			v = argv[i];
			if (!strcmp(v, "auto")) {
				o->public_key_compression = PUBLIC_KEY_COMPRESSION_AUTO;
			} else if (!strcmp(v, "compressed")) {
				o->public_key_compression = PUBLIC_KEY_COMPRESSION_COMPRESSED;
			} else if (!strcmp(v, "uncompressed")) {
				o->public_key_compression = PUBLIC_KEY_COMPRESSION_UNCOMPRESSED;
			} else {
				applog(APPLOG_ERROR, __func__,
					"unknown value \"%s\" for --public-key-compression", v
				);
				return 0;
			}
		} else if (!strcmp(a, "--input-file")) {
			if (++i >= argc) {
				applog(APPLOG_ERROR, __func__, "Missing value for %s", a);
				return 0;
			}
			o->input_file = argv[i];
		} else if (!strcmp(a, "--input")) {
			if (++i >= argc) {
				applog(APPLOG_ERROR, __func__, "Missing value for %s", a);
				return 0;
			}
			o->input = argv[i];
		} else if (
			!strcmp(a, "--network")
			|| !strcmp(a, "--private-key-prefix") /* deprecated */
			|| !strcmp(a, "--public-key-prefix") /* deprecated */
		) {
			if (++i >= argc) {
				applog(APPLOG_ERROR, __func__,
					"Missing value for %s, must be one of:", a
				);
				Bitcoin_ListNetworks(stderr);
				return 0;
			}
			if (o->network_type) {
				applog(APPLOG_ERROR, __func__,
					"--network specified multiple times, please use only once"
				);
				return 0;
			}
			v = argv[i];
			o->network_type = Bitcoin_GetNetworkTypeByName(v);
			if (o->network_type == NULL) {
				applog(APPLOG_ERROR, __func__,
					"Unknown network type \"%s\", must be one of:", v
				);
				Bitcoin_ListNetworks(stderr);
				return 0;
			}
		} else if (!strcmp(a, "--fix-base58check")) {
			o->fix_base58 = 1;
			o->fix_base58_change_chars = BITCOINTOOL_OPTION_DEFAULT_BASE58CHECK_CHANGE_CHARS;
			o->fix_base58_insert_chars = BITCOINTOOL_OPTION_DEFAULT_BASE58CHECK_INSERT_CHARS;
			o->fix_base58_remove_chars = BITCOINTOOL_OPTION_DEFAULT_BASE58CHECK_REMOVE_CHARS;
		} else if (!strcmp(a, "--fix-base58check-change-chars")) {
			unsigned parsed_value = 0;
			if (++i >= argc) {
				applog(APPLOG_ERROR, __func__, "missing value for %s", a);
				return 0;
			}
			v = argv[i];
			if (sscanf(v, "%u", &parsed_value)) {
				o->fix_base58_change_chars = parsed_value;
			} else {
				applog(APPLOG_ERROR, __func__,
					"value for %s should be an unsigned integer", a
				);
				return 0;
			}
		} else if (!strcmp(a, "--help")) {
			BitcoinTool_help(self);
			return 0;
		} else {
			applog(APPLOG_ERROR, __func__, "unknown option \"%s\"", a);
			return 0;
		}
	}

	if (!o->input && !o->input_file) {
		applog(APPLOG_ERROR, __func__, "Either --input <text> or --input-file <filename> must be specified.");
		errors++;
	}

	if (!o->input_type) {
		applog(APPLOG_ERROR, __func__, "--input-type must be specified.");
		errors++;
	}

	if (!o->input_format) {
		applog(APPLOG_ERROR, __func__, "--input-format must be specified.");
		errors++;
	}

	if (!o->output_type) {
		applog(APPLOG_ERROR, __func__, "--output-type must be specified.");
		errors++;
	}

	if (
		INPUT_FORMAT_BASE58CHECK == o->input_format
		&& (
			PUBLIC_KEY_COMPRESSION_COMPRESSED == o->public_key_compression
			|| PUBLIC_KEY_COMPRESSION_UNCOMPRESSED == o->public_key_compression
		)
	) {
		applog(APPLOG_WARNING, __func__,
			"using --input-format base58check with --public-key-compression"
			" other than auto to override the WIF compression type is very"
			" unusual, please be sure what you are doing!");
	}

	if (errors) {
		applog(APPLOG_ERROR, __func__, "Use --help for more information.");
		return 0;
	}

	return argc > 1;
}

void Bitcoin_MakeAddressFromRIPEMD160(
	struct BitcoinAddress *address,
	const struct BitcoinRIPEMD160 *hash,
	const struct BitcoinNetworkType *network_type
)
{
	memcpy(address->data+1, hash->data, BITCOIN_RIPEMD160_SIZE);
	address->data[0] = BitcoinNetworkType_GetPublicKeyPrefix(network_type);;
}

void Bitcoin_MakeRIPEMD160FromAddress(
	struct BitcoinRIPEMD160 *hash,
	const struct BitcoinAddress *address
)
{
	memcpy(&hash->data, address->data+BITCOIN_ADDRESS_VERSION_SIZE, BITCOIN_RIPEMD160_SIZE);
}

void Bitcoin_MakeRIPEMD160FromSHA256(
	struct BitcoinRIPEMD160 *output_hash,
	const struct BitcoinSHA256 *input_hash
)
{
	Bitcoin_RIPEMD160(output_hash, &input_hash->data, BITCOIN_SHA256_SIZE);
}

void Bitcoin_MakeSHA256FromPublicKey(
	struct BitcoinSHA256 *output_hash,
	const struct BitcoinPublicKey *public_key
)
{
	Bitcoin_SHA256(output_hash, &public_key->data, BitcoinPublicKey_GetSize(public_key));
}

BitcoinResult Bitcoin_ConvertInputToOutput(struct BitcoinTool *self)
{
	/* Convert from the input type to the output type.
	   Depending on the options selected, this may need multiple conversions
	   for example :

	    private key -> public key -> sha256 -> ripemd160 -> address -> base58

	   The conversion may be impossible, for example asking to output the
	   private key, using the public key as input.  We can detect this and
	   return an error.
	*/
	BitcoinResult result;

	switch (self->options.input_type) {
		case INPUT_TYPE_MINI_PRIVATE_KEY :
			switch (self->options.output_type) {
				case OUTPUT_TYPE_ALL :
				case OUTPUT_TYPE_ADDRESS :
				case OUTPUT_TYPE_PUBLIC_KEY_RIPEMD160 :
				case OUTPUT_TYPE_PUBLIC_KEY_SHA256 :
				case OUTPUT_TYPE_PUBLIC_KEY :
				case OUTPUT_TYPE_PRIVATE_KEY_WIF :
					self->mini_private_key_set = 1;
					break;
				default :
					break;
			}
		case INPUT_TYPE_PRIVATE_KEY :
			switch (self->options.output_type) {
				case OUTPUT_TYPE_ALL :
				case OUTPUT_TYPE_ADDRESS :
				case OUTPUT_TYPE_PUBLIC_KEY_RIPEMD160 :
				case OUTPUT_TYPE_PUBLIC_KEY_SHA256 :
				case OUTPUT_TYPE_PUBLIC_KEY :
				case OUTPUT_TYPE_PRIVATE_KEY_WIF :
					self->private_key_wif_set = 1;
					break;
				case OUTPUT_TYPE_PRIVATE_KEY :
					return BITCOIN_SUCCESS;
					break;
				default :
					break;
			}
		case INPUT_TYPE_PRIVATE_KEY_WIF :
			switch (self->options.output_type) {
				case OUTPUT_TYPE_ALL :
				case OUTPUT_TYPE_ADDRESS :
				case OUTPUT_TYPE_PUBLIC_KEY_RIPEMD160 :
				case OUTPUT_TYPE_PUBLIC_KEY_SHA256 :
				case OUTPUT_TYPE_PUBLIC_KEY :
				case OUTPUT_TYPE_PRIVATE_KEY :
					self->private_key_set = 1;
					break;
				case OUTPUT_TYPE_PRIVATE_KEY_WIF :
					return BITCOIN_SUCCESS;
					break;
				default :
					break;
			}
			switch (self->options.output_type) {
				case OUTPUT_TYPE_ALL :
				case OUTPUT_TYPE_ADDRESS :
				case OUTPUT_TYPE_PUBLIC_KEY_RIPEMD160 :
				case OUTPUT_TYPE_PUBLIC_KEY_SHA256 :
				case OUTPUT_TYPE_PUBLIC_KEY : {
					if (self->private_key.network_type == NULL) {
						applog(APPLOG_ERROR, __func__,
							"Network type is not specified, please set using"
							" --network option"
						);
						return BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT;
					}

					result = Bitcoin_MakePublicKeyFromPrivateKey(
						&self->public_key, &self->private_key
					);
					if (result != BITCOIN_SUCCESS) {
						return result;
					}
					self->public_key_set = 1;
					break;
				}
				case OUTPUT_TYPE_PRIVATE_KEY_WIF :
				case OUTPUT_TYPE_PRIVATE_KEY :
					return BITCOIN_SUCCESS;
					break;
				default :
					break;
			}
		case INPUT_TYPE_PUBLIC_KEY :
			switch (self->options.output_type) {
				case OUTPUT_TYPE_ALL :
				case OUTPUT_TYPE_ADDRESS :
				case OUTPUT_TYPE_PUBLIC_KEY_RIPEMD160 :
				case OUTPUT_TYPE_PUBLIC_KEY_SHA256 :
					Bitcoin_MakeSHA256FromPublicKey(&self->public_key_sha256, &self->public_key);
					self->public_key_sha256_set = 1;
					break;
				case OUTPUT_TYPE_PUBLIC_KEY :
					return BITCOIN_SUCCESS;
					break;
				case OUTPUT_TYPE_PRIVATE_KEY_WIF :
				case OUTPUT_TYPE_PRIVATE_KEY :
					applog(APPLOG_ERROR, __func__, "impossible conversion");
					return BITCOIN_ERROR_IMPOSSIBLE_CONVERSION;
					break;
				default :
					break;
			}
		case INPUT_TYPE_PUBLIC_KEY_SHA256 :
			switch (self->options.output_type) {
				case OUTPUT_TYPE_ALL :
				case OUTPUT_TYPE_ADDRESS :
				case OUTPUT_TYPE_PUBLIC_KEY_RIPEMD160 :
					Bitcoin_MakeRIPEMD160FromSHA256(&self->public_key_ripemd160, &self->public_key_sha256);
					self->public_key_ripemd160_set = 1;
					break;
				case OUTPUT_TYPE_PUBLIC_KEY_SHA256 :
					return BITCOIN_SUCCESS;
					break;
				case OUTPUT_TYPE_PUBLIC_KEY :
				case OUTPUT_TYPE_PRIVATE_KEY_WIF :
				case OUTPUT_TYPE_PRIVATE_KEY :
					applog(APPLOG_ERROR, __func__, "impossible conversion");
					return BITCOIN_ERROR_IMPOSSIBLE_CONVERSION;
					break;
				default :
					break;
			}
		case INPUT_TYPE_PUBLIC_KEY_RIPEMD160 :
			switch (self->options.output_type) {
				case OUTPUT_TYPE_ALL :
				case OUTPUT_TYPE_ADDRESS : {
					/* check if user has asked to override public key prefix */
					if (self->options.network_type) {
						self->public_key.network_type = self->options.network_type;
					}

					/* refuse to generate an address with no prefix set */
					if (!self->public_key.network_type) {
						applog(APPLOG_ERROR, __func__,
							"Raw public key has no network prefix and it is unsafe"
							" to assume one.  Please explicitally specify prefix using"
							" --network option."
						);
						return BITCOIN_ERROR_IMPOSSIBLE_CONVERSION;
					}

					Bitcoin_MakeAddressFromRIPEMD160(&self->address,
						&self->public_key_ripemd160,
						self->public_key.network_type
					);

					self->address_set = 1;
					break;
				}
				case OUTPUT_TYPE_PUBLIC_KEY_RIPEMD160 :
					return BITCOIN_SUCCESS;
					break;
				case OUTPUT_TYPE_PUBLIC_KEY_SHA256 :
				case OUTPUT_TYPE_PUBLIC_KEY :
				case OUTPUT_TYPE_PRIVATE_KEY_WIF :
				case OUTPUT_TYPE_PRIVATE_KEY :
					applog(APPLOG_ERROR, __func__, "impossible conversion");
					return BITCOIN_ERROR_IMPOSSIBLE_CONVERSION;
					break;
				default :
					break;
			}
		case INPUT_TYPE_ADDRESS :
			switch (self->options.output_type) {
				case OUTPUT_TYPE_ALL :
				case OUTPUT_TYPE_ADDRESS :
					return BITCOIN_SUCCESS;
					break;
				case OUTPUT_TYPE_PUBLIC_KEY_RIPEMD160 :
					Bitcoin_MakeRIPEMD160FromAddress(&self->public_key_ripemd160, &self->address);
					self->public_key_ripemd160_set = 1;
					return BITCOIN_SUCCESS;
					break;
				case OUTPUT_TYPE_PUBLIC_KEY_SHA256 :
				case OUTPUT_TYPE_PUBLIC_KEY :
				case OUTPUT_TYPE_PRIVATE_KEY_WIF :
				case OUTPUT_TYPE_PRIVATE_KEY :
					applog(APPLOG_ERROR, __func__, "impossible conversion");
					return BITCOIN_ERROR_IMPOSSIBLE_CONVERSION;
					break;
				default :
					break;
			}
		default :
			break;
	}

	return BITCOIN_SUCCESS;
}

BitcoinResult Bitcoin_ParseInput(struct BitcoinTool *self)
{
	self->output_text_size = sizeof(self->output_text);

	/* get input data from file or from command line option */
	if (self->options.input_file) {
		FILE *file = NULL;
		int bytes_read = 0;

		file = fopen(self->options.input_file, "rb");
		if (!file) {
			applog(APPLOG_ERROR, __func__, "Failed to open file [%s] (%s)",
				self->options.input_file,
				strerror(errno)
			);
			return BITCOIN_ERROR_FILE;
		}

		/* allow space for NUL char, so we can use it as a string later */
		bytes_read = fread(self->input, 1, sizeof(self->input) - 1, file);
		if (bytes_read <= 0) {
			applog(APPLOG_ERROR, __func__, "Failed to read file [%s] (%s)",
				self->options.input_file,
				strerror(errno)
			);
			fclose(file);
			return BITCOIN_ERROR_FILE;
		}

		fclose(file);

		self->input_size = bytes_read;
	} else if (self->options.input) {
		self->input_size = strlen(self->options.input);
		if (self->input_size >= sizeof(self->input)) {
			applog(APPLOG_ERROR, __func__,
				"--input value too large for internal buffer or any expected type"
			);
			return BITCOIN_ERROR;
		}
		strncpy(self->input, self->options.input, self->input_size);
	}

	/* check if we have any input we can work with */
	if (self->input_size == 0) {
		applog(APPLOG_ERROR, __func__,
			"No input data specified, use --input or --input-file to specify input data."
		);
		return BITCOIN_ERROR;
	}

	/* convert input format to raw data */
	switch (self->options.input_format) {
		case INPUT_FORMAT_RAW : {
			/* no translation required, just copy */
			memcpy(self->input_raw, self->input, self->input_size);
			self->input_raw_size = self->input_size;
			break;
		}
		case INPUT_FORMAT_HEX : {
			BitcoinResult result = Bitcoin_DecodeHex(
				self->input_raw, sizeof(self->input_raw), &self->input_raw_size,
				self->input, self->input_size
			);
			if (result != BITCOIN_SUCCESS) {
				applog(APPLOG_ERROR, __func__,
					"Failed to decode hex input (%s).",
					Bitcoin_ResultString(result)
				);
				return BITCOIN_ERROR_INVALID_FORMAT;
			}
			break;
		}
		case INPUT_FORMAT_BASE58 : {
			BitcoinResult result = Bitcoin_DecodeBase58(
				self->input_raw, sizeof(self->input_raw), &self->input_raw_size,
				self->input, self->input_size
			);
			if (result != BITCOIN_SUCCESS) {
				applog(APPLOG_ERROR, __func__,
					"Failed to decode Base58 input (%s).",
					Bitcoin_ResultString(result)
				);
				return BITCOIN_ERROR_INVALID_FORMAT;
			}
			break;
		}
		case INPUT_FORMAT_BASE58CHECK : {
			BitcoinResult result = Bitcoin_DecodeBase58Check(
				self->input_raw, sizeof(self->input_raw), &self->input_raw_size,
				self->input, self->input_size
			);
			if (result != BITCOIN_SUCCESS) {
				applog(APPLOG_ERROR, __func__,
					"Failed to decode Base58Check input (%s).",
					Bitcoin_ResultString(result)
				);

				if (self->options.fix_base58) {
					size_t output_base58_buffer_size = self->input_size + 1;
					char *output_base58 = calloc(1, output_base58_buffer_size);
					size_t output_base58_size = 0;
					int result;

					result = Bitcoin_FixBase58Check(
						output_base58, output_base58_buffer_size, &output_base58_size,
						self->input_raw, sizeof(self->input_raw), &self->input_raw_size,
						self->input, self->input_size,
						self->options.fix_base58_change_chars,
						self->options.fix_base58_insert_chars,
						self->options.fix_base58_remove_chars
					);

					free(output_base58);

					return result;
				} else {
					applog(APPLOG_ERROR, __func__,
						"You can use the --fix-base58check option to change"
						" the input string until the checksum is valid, but"
						" this may return a false positive match."
					);
				}

				return BITCOIN_ERROR_INVALID_FORMAT;
			}
			break;
		}
		default :
			applog(APPLOG_ERROR, __func__, "unspecified input type");
			return BITCOIN_ERROR_INVALID_FORMAT;
			break;
	}

	return BITCOIN_SUCCESS;
}

BitcoinResult Bitcoin_CheckInputSize(struct BitcoinTool *self)
{
	/* convenience pointers with less verbose names */
	const size_t input_raw_size = self->input_raw_size;
	const uint8_t *input_raw = self->input_raw;

	/* check the size of the input matches what we expect for its type */
	switch (self->options.input_type) {
		case INPUT_TYPE_MINI_PRIVATE_KEY : {
			size_t expected_size = BITCOIN_MINI_PRIVATE_KEY_SIZE;
			char test_buffer[BITCOIN_MINI_PRIVATE_KEY_SIZE + 1];
			struct BitcoinSHA256 hash;

			if (input_raw_size != expected_size) {
				const char *extra_message = "";
				applog(APPLOG_ERROR, __func__,
					"Invalid size input for mini private key:"
					" expected %u bytes but got %u bytes instead.",
					(unsigned)expected_size,
					(unsigned)input_raw_size,
					extra_message
				);
				return BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT;
			}

			/* prepare a test buffer, to check that the key is valid */
			memcpy(test_buffer, input_raw, input_raw_size);
			test_buffer[input_raw_size] = '?';
			Bitcoin_SHA256(&hash, test_buffer, BITCOIN_MINI_PRIVATE_KEY_SIZE + 1);
			if (hash.data[0] != 0) {
				applog(APPLOG_ERROR, __func__,
					"Mini private key invalid: SHA256(key + '?')[0] results in"
					" 0x%02x when the expected value is 0x00.  Check the key"
					" for typing errors and try again.",
					(unsigned)hash.data[0]
				);
				return BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT;
			}

			/* 1/256 chance the key is valid, hash the string into the real
			   private key. */
			Bitcoin_SHA256(&hash, input_raw, BITCOIN_MINI_PRIVATE_KEY_SIZE);
			memcpy(self->private_key.data, hash.data, BITCOIN_SHA256_SIZE);

			/* since the comression type is always uncompressed, we can set
			   that too, and we can produce a valid WIF key */
			self->private_key.public_key_compression = BITCOIN_PUBLIC_KEY_UNCOMPRESSED;

			if (!self->options.network_type) {
				/* This is normal : mini keys don't store a prefix, so Bitcoin
				   is implied */
				self->private_key.network_type = Bitcoin_GetNetworkTypeByName("bitcoin");
			} else {
				/* user is asking to override the implicit Bitcoin prefix -
				   this is very unusual so warn about it. */
				self->private_key.network_type = self->options.network_type;
				applog(APPLOG_WARNING, __func__,
					"Overriding mini private key prefix is unusual, since"
					" only Bitcoin is implied in the mini key format."
					" Please check your inputs."
				);
			}

			/* we have a valid pirvate key */
			self->private_key_set = 1;

			/* we have a valid WIF private key */
			self->private_key_wif_set = 1;

			break;
		}
		case INPUT_TYPE_PRIVATE_KEY : {
			size_t expected_size = BITCOIN_PRIVATE_KEY_SIZE;
			if (input_raw_size != expected_size) {
				const char *extra_message = "";
				if (
					(input_raw_size == BITCOIN_PRIVATE_KEY_WIF_UNCOMPRESSED_SIZE) ||
					(input_raw_size == BITCOIN_PRIVATE_KEY_WIF_COMPRESSED_SIZE)
				) {
					extra_message = " (did you mean \"--input-type private-key-wif\"?)";
				}
				applog(APPLOG_ERROR, __func__,
					"Invalid size input for private key:"
					" expected %u bytes but got %u bytes%s.",
					(unsigned)expected_size,
					(unsigned)input_raw_size,
					extra_message
				);
				return BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT;
			}
			assert(sizeof(self->private_key.data) >= input_raw_size);
			memcpy(self->private_key.data, input_raw, input_raw_size);

			if (!self->options.network_type) {
				applog(APPLOG_ERROR, __func__,
					"Raw private key has no network prefix and it is unsafe"
					" to assume one.  Please explicitally specify prefix using"
					" --network option."
				);
				return BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT;
			}
			self->private_key.network_type = self->options.network_type;
			self->private_key_set = 1;
			break;
		}
		case INPUT_TYPE_PRIVATE_KEY_WIF : {
			if (input_raw_size != BITCOIN_PRIVATE_KEY_WIF_UNCOMPRESSED_SIZE &&
				input_raw_size != BITCOIN_PRIVATE_KEY_WIF_COMPRESSED_SIZE
			) {
				applog(APPLOG_ERROR, __func__,
					"Invalid size input for WIF private key:"
					" expected %u (uncompressed) or"
					" %u (compressed) bytes but got %u bytes instead.",
					(unsigned)BITCOIN_PRIVATE_KEY_WIF_UNCOMPRESSED_SIZE,
					(unsigned)BITCOIN_PRIVATE_KEY_WIF_COMPRESSED_SIZE,
					(unsigned)input_raw_size
				);
				return BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT;
			}
			switch (input_raw_size) {
				case BITCOIN_PRIVATE_KEY_WIF_UNCOMPRESSED_SIZE :
					self->private_key.public_key_compression = BITCOIN_PUBLIC_KEY_UNCOMPRESSED;
					break;
				case BITCOIN_PRIVATE_KEY_WIF_COMPRESSED_SIZE :
					self->private_key.public_key_compression = BITCOIN_PUBLIC_KEY_COMPRESSED;
					break;
			}
			assert(sizeof(self->private_key.data) == BITCOIN_PRIVATE_KEY_SIZE);
			memcpy(self->private_key.data,
				input_raw+BITCOIN_PRIVATE_KEY_WIF_VERSION_SIZE,
				BITCOIN_PRIVATE_KEY_SIZE
			);
			self->private_key.network_type = Bitcoin_GetNetworkTypeByPrivateKeyPrefix(input_raw[0]);
			if (self->private_key.network_type == NULL) {
				applog(APPLOG_ERROR, __func__,
					"Unknown prefix byte in WIF private key [%u]",
					(unsigned)input_raw[0]
				);
				return BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT;
			}
			self->private_key_wif_set = 1;
			break;
		}
		case INPUT_TYPE_PUBLIC_KEY : {
			if (input_raw_size != BITCOIN_PUBLIC_KEY_UNCOMPRESSED_SIZE &&
				input_raw_size != BITCOIN_PUBLIC_KEY_COMPRESSED_SIZE
			) {
				applog(APPLOG_ERROR, __func__,
					"Invalid size input for public key:"
					" expected %u (uncompressed) or"
					" %u (compressed) bytes but got %u bytes instead.",
					(unsigned)BITCOIN_PUBLIC_KEY_UNCOMPRESSED_SIZE,
					(unsigned)BITCOIN_PUBLIC_KEY_COMPRESSED_SIZE,
					(unsigned)input_raw_size
				);
				return BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT;
			}
			switch (input_raw_size) {
				case BITCOIN_PUBLIC_KEY_UNCOMPRESSED_SIZE :
					self->public_key.compression = BITCOIN_PUBLIC_KEY_UNCOMPRESSED;
					break;
				case BITCOIN_PUBLIC_KEY_COMPRESSED_SIZE :
					self->public_key.compression = BITCOIN_PUBLIC_KEY_COMPRESSED;
					break;
			}
			assert(sizeof(self->public_key.data) >= input_raw_size);
			memcpy(self->public_key.data, input_raw, input_raw_size);
			self->public_key_set = 1;
			break;
		}
		case INPUT_TYPE_PUBLIC_KEY_SHA256 : {
			size_t expected_size = BITCOIN_SHA256_SIZE;
			if (input_raw_size != expected_size) {
				applog(APPLOG_ERROR, __func__,
					"Invalid size input for SHA256(public_key):"
					" expected %u bytes but got %u bytes instead.",
					(unsigned)expected_size,
					(unsigned)input_raw_size
				);
				return BITCOIN_ERROR_INVALID_FORMAT;
			}
			self->public_key_sha256_set = 1;
			break;
		}
		case INPUT_TYPE_PUBLIC_KEY_RIPEMD160 : {
			size_t expected_size = BITCOIN_RIPEMD160_SIZE;
			if (input_raw_size != expected_size) {
				applog(APPLOG_ERROR, __func__,
					"Invalid size input for RIPEMD160(SHA256(public_key)):"
					" expected %u bytes but got %u bytes instead.",
					(unsigned)expected_size,
					(unsigned)input_raw_size
				);
				return BITCOIN_ERROR_INVALID_FORMAT;
			}
			assert(sizeof(self->public_key_ripemd160.data) >= BITCOIN_RIPEMD160_SIZE);
			assert(input_raw_size >= BITCOIN_RIPEMD160_SIZE);
			memcpy(self->public_key_ripemd160.data, input_raw, BITCOIN_RIPEMD160_SIZE);
			self->public_key_ripemd160_set = 1;
			break;
		}
		case INPUT_TYPE_ADDRESS : {
			size_t expected_size = BITCOIN_ADDRESS_SIZE;
			if (input_raw_size != expected_size) {
				applog(APPLOG_ERROR, __func__,
					"Invalid size input for address:"
					" expected %u bytes but got %u bytes instead.",
					(unsigned)expected_size,
					(unsigned)input_raw_size
				);
				return BITCOIN_ERROR_INVALID_FORMAT;
			}
			memcpy(self->address.data, input_raw, BITCOIN_ADDRESS_SIZE);
			self->address_set = 1;
			break;
		}
		default :
			applog(APPLOG_ERROR, __func__, "Unknown input.");
			return BITCOIN_ERROR_INVALID_FORMAT;
			break;
	}

	return BITCOIN_SUCCESS;
}

BitcoinResult Bitcoin_WriteOutput(struct BitcoinTool *self);

BitcoinResult Bitcoin_WriteAllOutput(struct BitcoinTool *self)
{
	FILE *file = stdout;

	struct OutputFormatString {
		enum OutputFormat output_format;
		char *name;
	} output_formats[] = {
		{ OUTPUT_FORMAT_HEX,         "hex" },
		{ OUTPUT_FORMAT_BASE58,      "base58" },
		{ OUTPUT_FORMAT_BASE58CHECK, "base58check" }
	}, *output_format = NULL;

	struct OutputTypeString {
		enum OutputType output_type;
		char *name;
		int is_set;
	} output_types[] = {
		{ OUTPUT_TYPE_ADDRESS,              "address" },
		{ OUTPUT_TYPE_PUBLIC_KEY_RIPEMD160, "public-key-ripemd160" },
		{ OUTPUT_TYPE_PUBLIC_KEY_SHA256,    "public-key-sha256" },
		{ OUTPUT_TYPE_PUBLIC_KEY,           "public-key" },
		{ OUTPUT_TYPE_PRIVATE_KEY_WIF,      "private-key-wif" },
		{ OUTPUT_TYPE_PRIVATE_KEY,          "private-key" }
	}, *output_type = NULL;

	for (output_type = output_types;
		output_type != output_types +
		(sizeof(output_types) / sizeof(output_types[0]));
		output_type++
	) {
		for (output_format = output_formats;
			output_format != output_formats +
			(sizeof(output_formats) / sizeof(output_formats[0]));
			output_format++
		) {
			if (
				(output_type->output_type == OUTPUT_TYPE_ADDRESS && self->address_set) ||
				(output_type->output_type == OUTPUT_TYPE_PUBLIC_KEY_RIPEMD160 && self->public_key_ripemd160_set) ||
				(output_type->output_type == OUTPUT_TYPE_PUBLIC_KEY_SHA256 && self->public_key_sha256_set) ||
				(output_type->output_type == OUTPUT_TYPE_PUBLIC_KEY && self->public_key_set) ||
				(output_type->output_type == OUTPUT_TYPE_PRIVATE_KEY_WIF && self->private_key_wif_set) ||
				(output_type->output_type == OUTPUT_TYPE_PRIVATE_KEY && self->private_key_set)
			) {
				self->options.output_type = output_type->output_type;
				self->options.output_format = output_format->output_format;
				fprintf(file, "%s.%s:",
					output_type->name, output_format->name
				);
				Bitcoin_WriteOutput(self);
			}
		}
	}

	return BITCOIN_SUCCESS;
}

BitcoinResult Bitcoin_WriteOutput(struct BitcoinTool *self)
{
	BitcoinResult result = BITCOIN_SUCCESS;
	size_t output_raw_size = 0;
	int bytes_wrote = 0;

	if (self->options.output_type == OUTPUT_TYPE_ALL) {
		return Bitcoin_WriteAllOutput(self);
	}

	switch (self->options.output_type) {
		case OUTPUT_TYPE_ADDRESS :
			output_raw_size = BITCOIN_ADDRESS_SIZE;
			assert(sizeof(self->output_raw) >= output_raw_size);
			memcpy(self->output_raw, self->address.data, output_raw_size);
			break;
		case OUTPUT_TYPE_PUBLIC_KEY_RIPEMD160 :
			output_raw_size = BITCOIN_RIPEMD160_SIZE;
			assert(sizeof(self->output_raw) >= output_raw_size);
			memcpy(self->output_raw, self->public_key_ripemd160.data, output_raw_size);
			break;
		case OUTPUT_TYPE_PUBLIC_KEY_SHA256 :
			output_raw_size = BITCOIN_SHA256_SIZE;
			assert(sizeof(self->output_raw) >= output_raw_size);
			memcpy(self->output_raw, self->public_key_sha256.data, output_raw_size);
			break;
		case OUTPUT_TYPE_PUBLIC_KEY :
			output_raw_size = BitcoinPublicKey_GetSize(&self->public_key);
			assert(sizeof(self->output_raw) >= output_raw_size);
			memcpy(self->output_raw, self->public_key.data, output_raw_size);
			break;
		case OUTPUT_TYPE_PRIVATE_KEY_WIF :
			self->output_raw[0] = BitcoinNetworkType_GetPrivateKeyPrefix(self->private_key.network_type);
			memcpy(self->output_raw + BITCOIN_PRIVATE_KEY_WIF_VERSION_SIZE,
				self->private_key.data, BITCOIN_PRIVATE_KEY_SIZE
			);
			switch (self->private_key.public_key_compression) {
				case BITCOIN_PUBLIC_KEY_COMPRESSED :
					/* set compression flag */
					self->output_raw[
						BITCOIN_PRIVATE_KEY_WIF_VERSION_SIZE +
						BITCOIN_PRIVATE_KEY_SIZE
					] = BITCOIN_PRIVATE_KEY_WIF_COMPRESSION_FLAG_COMPRESSED;
					output_raw_size = BITCOIN_PRIVATE_KEY_WIF_VERSION_SIZE +
						BITCOIN_PRIVATE_KEY_SIZE +
						BITCOIN_PRIVATE_KEY_WIF_COMPRESSION_FLAG_SIZE;
					break;
				case BITCOIN_PUBLIC_KEY_UNCOMPRESSED :
					/* no compression flag to set, size determines that the
					corresponding public key should be uncompressed. */
					output_raw_size = BITCOIN_PRIVATE_KEY_WIF_VERSION_SIZE +
						BITCOIN_PRIVATE_KEY_SIZE;
					break;
				default :
					applog(APPLOG_ERROR, __func__,
						"Public key compression flag must be set using"
						" --public-key-compression (compressed | uncompressed)"
						" when importing raw private keys."
					);
					return BITCOIN_ERROR_INVALID_FORMAT;
					break;
			}
			assert(sizeof(self->output_raw) >= output_raw_size);
			break;
		case OUTPUT_TYPE_PRIVATE_KEY :
			output_raw_size = BitcoinPrivateKey_GetSize(&self->private_key);
			assert(sizeof(self->output_raw) >= output_raw_size);
			memcpy(self->output_raw, self->private_key.data, output_raw_size);
			break;
		default :
			applog(APPLOG_ERROR, __func__, "Unknown output type.");
			return BITCOIN_ERROR_INVALID_FORMAT;
			break;
	}

	self->output_raw_size = output_raw_size;

	switch (self->options.output_format) {
		case OUTPUT_FORMAT_RAW : {
			if (output_raw_size > sizeof(self->output_text)) {
				applog(APPLOG_BUG, __func__,
					"output_raw buffer (%u) larger than output_text buffer (%u),"
					"unable to write output",
					(unsigned)output_raw_size,
					(unsigned)sizeof(self->output_text)
				);
				result = BITCOIN_ERROR_INVALID_FORMAT;
			}
			break;
		}
		case OUTPUT_FORMAT_HEX : {
			int lower_case = 1;
			result = Bitcoin_EncodeHex(
				self->output_text, sizeof(self->output_text),
				&self->output_text_size,
				self->output_raw, output_raw_size,
				lower_case
			);
			break;
		}
		case OUTPUT_FORMAT_BASE58 : {
			result = Bitcoin_EncodeBase58(
				self->output_text, sizeof(self->output_text),
				&self->output_text_size,
				self->output_raw, output_raw_size
			);
			break;
		}
		case OUTPUT_FORMAT_BASE58CHECK : {
			result = Bitcoin_EncodeBase58Check(
				self->output_text, sizeof(self->output_text),
				&self->output_text_size,
				self->output_raw, output_raw_size
			);
			break;
		}
		default:
			applog(APPLOG_ERROR, __func__,
				"Unspecified output format."
				" Please use --output-format with one of:"
			);
			BitcoinTool_ListOutputFormats(stderr);
			return BITCOIN_ERROR_INVALID_FORMAT;
			break;
	}

	if (result != BITCOIN_SUCCESS) {
		applog(APPLOG_ERROR, __func__,
			"Failed to encode raw output data (%s)",
			Bitcoin_ResultString(result)
		);
		return result;
	}

	if (self->output_text == 0) {
		applog(APPLOG_BUG, __func__,
			"No text to output - something went wrong"
		);
		return BITCOIN_ERROR_INVALID_FORMAT;
	}

	bytes_wrote = fwrite(self->output_text, 1, self->output_text_size, stdout);
	if (bytes_wrote < 0) {
		applog(APPLOG_ERROR, __func__, "Error writing output (%s)", strerror(errno));
	} else if (bytes_wrote < self->output_text_size) {
		applog(APPLOG_ERROR, __func__, "Error writing output, short write");
	}

	/* output a newline for clarity if we're on a TTY */
	if (isatty(fileno(stdin))) {
		putchar('\n');
	}

	return BITCOIN_SUCCESS;
}

static int BitcoinTool_run(BitcoinTool *self)
{
	if (Bitcoin_ParseInput(self) != BITCOIN_SUCCESS) {
		return 0;
	}

	if (Bitcoin_CheckInputSize(self) != BITCOIN_SUCCESS) {
		return 0;
	}

	/* has user asked to override public key compression? */
	switch (self->options.public_key_compression) {
		/* user wants compressed public key */
		case PUBLIC_KEY_COMPRESSION_COMPRESSED :
			self->private_key.public_key_compression =
				BITCOIN_PUBLIC_KEY_COMPRESSED;
			break;
		/* user wants uncompressed public key */
		case PUBLIC_KEY_COMPRESSION_UNCOMPRESSED :
			self->private_key.public_key_compression =
				BITCOIN_PUBLIC_KEY_UNCOMPRESSED;
			break;
		/* use the compression specified in the private key */
		case PUBLIC_KEY_COMPRESSION_AUTO :
		default :
			break;
	}

	if (Bitcoin_ConvertInputToOutput(self) != BITCOIN_SUCCESS) {
		return 0;
	}

	if (Bitcoin_WriteOutput(self) != BITCOIN_SUCCESS) {
		return 0;
	}

	return 1;
}

static void BitcoinTool_destroy(BitcoinTool *self)
{
	free(self);
}

BitcoinTool *BitcoinTool_create(void)
{
	BitcoinTool *self = (BitcoinTool *)calloc(1, sizeof(*self));

	self->help = BitcoinTool_help;
	self->parseOptions = BitcoinTool_parseOptions;
	self->run = BitcoinTool_run;
	self->destroy = BitcoinTool_destroy;

	self->private_key.network_type =
	self->public_key.network_type = NULL;

	return self;
}

int main(int argc, char *argv[])
{
	BitcoinTool *bat = BitcoinTool_create();
	int result = 0;

	if (!bat->parseOptions(bat, argc, argv)) {
		bat->destroy(bat);
		return EXIT_FAILURE;
	}
	result = bat->run(bat);
	bat->destroy(bat);

	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
