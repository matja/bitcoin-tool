#include "result.h"

const char *Bitcoin_ResultString(BitcoinResult result)
{
	const char *m = "";
	switch (result) {
		case BITCOIN_SUCCESS: m = "success"; break;
		case BITCOIN_ERROR_NOT_IMPLEMENTED: m = "not implemented"; break;
		case BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT: m = "invalid private key format"; break;
		case BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT: m = "invalid public key format"; break;
		case BITCOIN_ERROR_OUTPUT_BUFFER_TOO_SMALL: m = "output buffer too small"; break;
		case BITCOIN_ERROR_CHECKSUM_FAILURE: m = "checksum failure"; break;
		case BITCOIN_ERROR_INVALID_FORMAT: m = "invalid format"; break;
		case BITCOIN_ERROR_IMPOSSIBLE_CONVERSION: m = "impossible conversion from input type to output type"; break;
		default : m = "unknown result code"; break;
	}
	return m;
}

