#ifndef BITCOIN_INCLUDE_RESULT_H
#define BITCOIN_INCLUDE_RESULT_H

/** @file result.h
 *  @brief Definitions of different return values from Bitcoin functions.
 *
 *  @author Matthew Anger
 */

/** All possible return values for bitcoin functions */
typedef enum BitcoinResult {
	BITCOIN_SUCCESS,
	BITCOIN_ERROR, /* general error */
	BITCOIN_ERROR_NOT_IMPLEMENTED,
	BITCOIN_ERROR_PRIVATE_KEY_INVALID_FORMAT,
	BITCOIN_ERROR_PUBLIC_KEY_INVALID_FORMAT,
	BITCOIN_ERROR_OUTPUT_BUFFER_TOO_SMALL,
	BITCOIN_ERROR_CHECKSUM_FAILURE,
	BITCOIN_ERROR_INVALID_FORMAT,
	BITCOIN_ERROR_IMPOSSIBLE_CONVERSION,
	BITCOIN_ERROR_FILE
} BitcoinResult;

/** @brief Return the text message corresponding to a BitcoinResult.
 *
 *  @param[in] result BitcoinResult returned from a previous function.
 *
 *  @return Pointer to text message.
 */
const char *Bitcoin_ResultString(BitcoinResult result);

#endif

