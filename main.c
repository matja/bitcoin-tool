#define _POSIX_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "hash.h"
#include "ec.h"
#include "utility.h"
#include "base58.h"

typedef struct BitcoinTool BitcoinTool;
typedef struct BitcoinToolOptions BitcoinToolOptions;

struct BitcoinToolOptions {
	const char *input;
	const char *input_file;

	enum InputType {
		INPUT_TYPE_NONE,
		INPUT_TYPE_ADDRESS,
		INPUT_TYPE_PUBLIC_KEY,
		INPUT_TYPE_PRIVATE_KEY
	} input_type;

	enum InputFormat {
		INPUT_FORMAT_NONE,
		INPUT_FORMAT_RAW,
		INPUT_FORMAT_HEX,
		INPUT_FORMAT_BASE58CHECK
	} input_format;

	enum OutputType {
		OUTPUT_TYPE_NONE,
		OUTPUT_TYPE_ADDRESS,
		OUTPUT_TYPE_PUBLIC_KEY,
		OUTPUT_TYPE_PRIVATE_KEY,
		OUTPUT_TYPE_ALL
	} output_type;

	enum OutputFormat {
		OUTPUT_FORMAT_NONE,
		OUTPUT_FORMAT_RAW,
		OUTPUT_FORMAT_HEX,
		OUTPUT_FORMAT_BASE58CHECK
	} output_format;
};

struct BitcoinTool {
	struct BitcoinToolOptions options;
	struct BitcoinPrivateKey private_key;
	struct BitcoinPublicKey public_key;
	struct BitcoinAddress address;

	char output_text[256];
	size_t output_text_size;

	int (*parseOptions)(struct BitcoinTool *self, int argc, char *argv[]);
	void (*syntax)(struct BitcoinTool *self);
	int (*run)(struct BitcoinTool *self);
	void (*destroy)(struct BitcoinTool *self);
};

static void BitcoinTool_syntax(BitcoinTool *self)
{
	FILE *file = stderr;
	fprintf(file,
		"Usage: bitcoin-tool [option]...\n"
		"Convert Bitcoin keys and addresses.\n"
		"\n"
	);
	fprintf(file,
		"  -input-type    Input data type, can be one of :\n"
		"                   private-key     : ECDSA private key\n"
		"                   public-key      : ECDSA public key\n"
		"                   public-key-sha  : SHA256(public key)\n"
		"                   public-key-rmd  : RIPEMD160(SHA256(public key))\n"
		"                   address         : Bitcoin address (version + hash)\n"
	);
	fprintf(file,
		"  -input-format  Input data format, can be one of :\n"
		"                   raw             : raw binary data\n"
		"                   hex             : hexadecimal encoded\n"
		"                   base58check     : Base58Check encoded\n"
	);
	fprintf(file,
		"  -output-type   Output data type, can be one of :\n"
		"                   private-key     : ECDSA private key\n"
		"                   public-key      : ECDSA public key\n"
		"                   public-key-sha  : SHA256(public key)\n"
		"                   public-key-rmd  : RIPEMD160(SHA256(public key))\n"
		"                   address         : Bitcoin address (version + hash)\n"
		"                   all             : all output types as type:value\n"
	);
	fprintf(file,
		"  -output-format Output data format, can be one of :\n"
		"                   raw             : raw binary data\n"
		"                   hex             : hexadecimal encoded\n"
		"                   base58check     : Base58Check encoded\n"
	);
	fprintf(file,
		"  -input         Specify input data\n"
		"  -input-file    Specify input file name\n"
		"\n"
	);
	fprintf(file,
		"Examples:\n"
	);
	fprintf(file,
		"  Show address for specified base58 private key\n"
		"    --input-type private-key \\\n"
		"    --input-format base58check \\\n"
		"    --input 5J2YUwNA5hmZFW33nbUCp5TmvszYXxVYthqDv7axSisBjFJMqaT \\\n"
		"    --output-type address \\\n"
		"    --output-format base58check \n"
		"\n"
	);
	fprintf(file,
		"  Show address for random private key (generate random address)\n"
		"    --input-type private-key \\\n"
		"    --input-format raw \\\n"
		"    --input-file <(openssl rand 64) \\\n"
		"    --output-type address \\\n"
		"    --output-format base58check \n"
		"\n"
	);
	fprintf(file,
		"  Show hex public key for SHA256-hashed string used as private key\n"
		"    --input-type private-key \\\n"
		"    --input-format raw \\\n"
		"    --input-file <(echo -n sausage|openssl dgst -sha256 -binary) \\\n"
		"    --output-type public-key \\\n"
		"    --output-format hex \n"
		"\n"
	);
}

static int BitcoinTool_parseOptions(BitcoinTool *self
	,int argc
	,char *argv[]
)
{
	unsigned i = 0;
	BitcoinToolOptions *o = &self->options;

	for (i=1; i<argc; i++) {
		const char *a = argv[i];
		const char *v = NULL;

		if (!strcmp(a, "--input-type")) {
			if (++i >= argc) {
				fprintf(stderr, "expected value for --input-type\n");
				return 0;
			}
			v = argv[i];
			if (!strcmp(v, "address")) { o->input_type = INPUT_TYPE_ADDRESS; }
			else if (!strcmp(v, "public-key")) { o->input_type = INPUT_TYPE_PUBLIC_KEY; }
			else if (!strcmp(v, "private-key")) { o->input_type = INPUT_TYPE_PRIVATE_KEY; }
		} else if (!strcmp(a, "--input-format")) {
			if (++i >= argc) {
				fprintf(stderr, "expected value for --input-format\n");
				return 0;
			}
			v = argv[i];
			if (!strcmp(v, "raw")) { o->input_format = INPUT_FORMAT_RAW; }
			else if (!strcmp(v, "hex")) { o->input_format = INPUT_FORMAT_HEX; }
			else if (!strcmp(v, "base58check")) { o->input_format = INPUT_FORMAT_BASE58CHECK; }
		} else if (!strcmp(a, "--output-type")) {
			if (++i >= argc) {
				fprintf(stderr, "expected value for --output-type\n");
				return 0;
			}
			v = argv[i];
			if (!strcmp(v, "address")) { o->output_type = OUTPUT_TYPE_ADDRESS; }
			else if (!strcmp(v, "public-key")) { o->output_type = OUTPUT_TYPE_PUBLIC_KEY; }
			else if (!strcmp(v, "private-key")) { o->output_type = OUTPUT_TYPE_PRIVATE_KEY; }
			else if (!strcmp(v, "all")) { o->output_type = OUTPUT_TYPE_ALL; }
		} else if (!strcmp(a, "--output-format")) {
			if (++i >= argc) {
				fprintf(stderr, "expected value for --output-format\n");
				return 0;
			}
			v = argv[i];
			if (!strcmp(v, "raw")) { o->output_format = OUTPUT_FORMAT_RAW; }
			else if (!strcmp(v, "hex")) { o->output_format = OUTPUT_FORMAT_HEX; }
			else if (!strcmp(v, "base58check")) { o->output_format = OUTPUT_FORMAT_BASE58CHECK; }
		} else if (!strcmp(a, "--input-file")) {
			if (++i >= argc) {
				fprintf(stderr, "expected value for --input-file\n");
				return 0;
			}
			o->input_file = argv[i];
		} else if (!strcmp(a, "--input")) {
			if (++i >= argc) {
				fprintf(stderr, "expected value for --input\n");
				return 0;
			}
			o->input = argv[i];
		}
	}

	return argc > 1;
}

static void BitcoinTool_outputAddressRaw(BitcoinTool *self)
{
	int bytes_wrote = fwrite(&self->address.data, BITCOIN_ADDRESS_SIZE, 1, stdout);
	if (bytes_wrote <= 0) {
		fprintf(stderr, "failed to write output\n");
	}
}

static void BitcoinTool_outputAddressHex(BitcoinTool *self)
{
	Bitcoin_OutputHex(&self->address.data, BITCOIN_ADDRESS_SIZE);
}

static void BitcoinTool_outputAddressBase58(BitcoinTool *self)
{
	if (!Bitcoin_EncodeBase58Check(self->output_text, &self->output_text_size, &self->address, BITCOIN_ADDRESS_SIZE)) {
		int bytes_wrote = fwrite(self->output_text, self->output_text_size, 1, stdout);
		if (bytes_wrote <= 0) {
			fprintf(stderr, "failed to write output\n");
		}
	} else {
		fprintf(stderr, "self->output_text buffer too small\n");
	}
}

static void BitcoinTool_outputPublicKeyRaw(const struct BitcoinPublicKey *public_key)
{
	int bytes_wrote = fwrite(&public_key->data, BITCOIN_PUBLIC_KEY_SIZE, 1, stdout);
	if (bytes_wrote <= 0) {
		fprintf(stderr, "failed to write output\n");
	}
}

static void BitcoinTool_outputPublicKeyHex(const struct BitcoinPublicKey *public_key)
{
	Bitcoin_OutputHex(&public_key->data, BITCOIN_PUBLIC_KEY_SIZE);
}

static void BitcoinTool_outputPublicKeyBase58(const struct BitcoinPublicKey *public_key)
{
	char output_text[256];
	size_t output_text_size = sizeof(output_text);
	if (!Bitcoin_EncodeBase58Check(output_text, &output_text_size, &public_key->data, BITCOIN_PUBLIC_KEY_SIZE)) {
		int bytes_wrote = fwrite(output_text, output_text_size, 1, stdout);
		if (bytes_wrote <= 0) {
			fprintf(stderr, "failed to write output\n");
		}
	} else {
		fprintf(stderr, "self->output_text buffer too small\n");
	}
}

static void BitcoinTool_outputPrivateKeyRaw(const struct BitcoinPrivateKey *private_key)
{
	int bytes_wrote = fwrite(&private_key->data, BITCOIN_PRIVATE_KEY_SIZE, 1, stdout);
	if (bytes_wrote <= 0) {
		fprintf(stderr, "failed to write output\n");
	}
}

static void BitcoinTool_outputPrivateKeyHex(const struct BitcoinPrivateKey *private_key)
{
	Bitcoin_OutputHex(&private_key->data, BITCOIN_PRIVATE_KEY_SIZE);
}

static void BitcoinTool_outputPrivateKeyBase58(const struct BitcoinPrivateKey *private_key)
{
	char output_text[256];
	char private_key_buffer[256];
	size_t private_key_buffer_size = BITCOIN_PRIVATE_KEY_SIZE + 1;
	size_t output_text_size = sizeof(output_text);

	private_key_buffer[0] = '\x80';
	memcpy(private_key_buffer+1, &private_key->data, BITCOIN_PRIVATE_KEY_SIZE);

	if (!Bitcoin_EncodeBase58Check(output_text, &output_text_size, private_key_buffer, private_key_buffer_size)) {
		int bytes_wrote = fwrite(output_text, output_text_size, 1, stdout);
		if (bytes_wrote <= 0) {
			fprintf(stderr, "failed to write output\n");
		}
	} else {
		fprintf(stderr, "self->output_text buffer too small\n");
	}
}

static int BitcoinTool_run(BitcoinTool *self)
{
	const char *input = NULL;
	char input_buffer[256];
	unsigned input_size = 0;

	self->output_text_size = sizeof(self->output_text);

	/* get private key */
	if (self->options.input_file) {
		FILE *file = NULL;
		int bytes_read = 0;

		printf("input file set\n");

		file = fopen(self->options.input_file, "rb");
		if (!file) {
			fprintf(stderr, "%s: failed to read private key file [%s] (%s)\n"
				,__func__
				,self->options.input_file
				,strerror(errno)
			);
			return 0;
		}

		bytes_read = fread(input_buffer, 1, sizeof(input_buffer), file);
		if (bytes_read <= 0) {
			fprintf(stderr, "%s: failed to read private key (%s)\n"
				,__func__
				,strerror(errno)
			);
			fclose(file);
			return 0;
		}

		fclose(file);

		input = input_buffer;
		input_size = bytes_read;
	} else if (self->options.input) {
		input = self->options.input;
		input_size = strlen(input);
	}

	/* convert private key, if specified */
	if (!input) {
		fprintf(stderr, "%s: no input data specified\n", __func__);
		return 0;
	}

	switch (self->options.input_type) {
		case INPUT_TYPE_PRIVATE_KEY :
			switch (self->options.input_format) {
				case INPUT_FORMAT_RAW : {
					memcpy(&self->private_key.data, input, BITCOIN_PRIVATE_KEY_SIZE);
					break;
				}
				case INPUT_FORMAT_HEX : {
					Bitcoin_DecodeHex(&self->private_key, BITCOIN_PRIVATE_KEY_SIZE
						,input
						,input_size
					);
					break;
				}
				case INPUT_FORMAT_BASE58CHECK : {
					int error = Bitcoin_LoadPrivateKeyFromBase58(
						&self->private_key, input, input_size
					);
					if (error) {
						fprintf(stderr
							,"%s: failed to load private key from base58 string [%s]\n"
							,__func__
							,Bitcoin_ResultString(error)
						);
						return 0;
					}
					break;
				}
				default : {
					fprintf(stderr, "%s: no private key format specified\n"
						,__func__
					);
					return 0;
					break;
				}
			}
			break;
		case INPUT_TYPE_PUBLIC_KEY :
			switch (self->options.input_format) {
				case INPUT_FORMAT_RAW :
					memcpy(&self->public_key.data, input, BITCOIN_PUBLIC_KEY_SIZE);
					break;
				case INPUT_FORMAT_HEX : {
					Bitcoin_DecodeHex(&self->public_key, BITCOIN_PUBLIC_KEY_SIZE
						,input
						,input_size
					);
					break;
				}
				case INPUT_FORMAT_BASE58CHECK : {
					int error = Bitcoin_LoadPublicKeyFromBase58(
						&self->public_key, input, input_size
					);
					if (error) {
						fprintf(stderr
							,"%s: failed to load public key from base58 string [%s]\n"
							,__func__
							,Bitcoin_ResultString(error)
						);
						return 0;
					}
					break;
				}
				default : {
					fprintf(stderr, "%s: no public key format specified\n"
						,__func__
					);
					return 0;
					break;
				}
			}
			break;
		case INPUT_TYPE_ADDRESS :
			switch (self->options.input_format) {
				case INPUT_FORMAT_RAW : {
					break;
				}
				case INPUT_FORMAT_HEX : {
					break;
				}
				case INPUT_FORMAT_BASE58CHECK : {
					break;
				}
				default : {
					fprintf(stderr, "%s: no address format specified\n"
						,__func__
					);
					return 0;
					break;
				}
			}
			break;
		default :
			fprintf(stderr, "unsupported input type\n");
			return 0;
			break;
	}

	switch (self->options.output_type) {
		case OUTPUT_TYPE_ALL :
		case OUTPUT_TYPE_PUBLIC_KEY :
		case OUTPUT_TYPE_ADDRESS :
			Bitcoin_MakePublicKeyFromPrivateKey(&self->public_key, &self->private_key);
			break;
		case OUTPUT_TYPE_NONE :
		default :
			break;
	}

	switch (self->options.output_type) {
		case OUTPUT_TYPE_ALL :
		case OUTPUT_TYPE_ADDRESS :
			Bitcoin_MakeAddressFromPublicKey(&self->address, &self->public_key);
			break;
		case OUTPUT_TYPE_NONE :
		default :
			break;
	}

	switch (self->options.output_type) {
		case OUTPUT_TYPE_ALL :
			printf("address-hex:");
			BitcoinTool_outputAddressHex(self);
			putchar('\n');

			printf("address-base58:");
			BitcoinTool_outputAddressBase58(self);
			putchar('\n');

			printf("public-key-hex:");
			BitcoinTool_outputPublicKeyHex(&self->public_key);
			putchar('\n');

			printf("public-key-base58:");
			BitcoinTool_outputPublicKeyBase58(&self->public_key);
			putchar('\n');

			printf("private-key-hex:");
			BitcoinTool_outputPrivateKeyHex(&self->private_key);
			putchar('\n');

			printf("private-key-base58:");
			BitcoinTool_outputPrivateKeyBase58(&self->private_key);

			break;
		case OUTPUT_TYPE_ADDRESS :
			switch (self->options.output_format) {
				case OUTPUT_FORMAT_RAW :
					BitcoinTool_outputAddressRaw(self);
					break;
				case OUTPUT_FORMAT_HEX :
					BitcoinTool_outputAddressHex(self);
					break;
				case OUTPUT_FORMAT_BASE58CHECK :
					BitcoinTool_outputAddressBase58(self);
					break;
				case OUTPUT_FORMAT_NONE :
				default :
					break;
			}
			break;
		case OUTPUT_TYPE_PUBLIC_KEY :
			switch (self->options.output_format) {
				case OUTPUT_FORMAT_RAW :
					BitcoinTool_outputPublicKeyRaw(&self->public_key);
					break;
				case OUTPUT_FORMAT_HEX :
					BitcoinTool_outputPublicKeyHex(&self->public_key);
					break;
				case OUTPUT_FORMAT_BASE58CHECK :
					BitcoinTool_outputPublicKeyBase58(&self->public_key);
					break;
				case OUTPUT_FORMAT_NONE :
				default :
					break;
			}
			break;
		case OUTPUT_TYPE_PRIVATE_KEY :
			switch (self->options.output_format) {
				case OUTPUT_FORMAT_RAW :
					BitcoinTool_outputPrivateKeyRaw(&self->private_key);
					break;
				case OUTPUT_FORMAT_HEX :
					BitcoinTool_outputPrivateKeyHex(&self->private_key);
					break;
				case OUTPUT_FORMAT_BASE58CHECK :
					BitcoinTool_outputPrivateKeyBase58(&self->private_key);
					break;
				case OUTPUT_FORMAT_NONE :
				default :
					break;
			}
			break;
		default :
			break;
	}

	if (isatty(fileno(stdin))) {
		putchar('\n');
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

	self->syntax = BitcoinTool_syntax;
	self->parseOptions = BitcoinTool_parseOptions;
	self->run = BitcoinTool_run;
	self->destroy = BitcoinTool_destroy;

	return self;
}

int main(int argc, char *argv[])
{
	BitcoinTool *bat = BitcoinTool_create();

	if (!bat->parseOptions(bat, argc, argv)) {
		bat->syntax(bat);
		bat->destroy(bat);
		return EXIT_FAILURE;
	}
	bat->run(bat);
	bat->destroy(bat);

	return EXIT_SUCCESS;
}
