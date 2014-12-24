#ifndef BITCOIN_INCLUDE_UTILITY_H
#define BITCOIN_INCLUDE_UTILITY_H

/** @file utility.h
 *  @brief Function prototypes for utility functions.
 *
 *  @author Matthew Anger
 */

#include <stdlib.h> /* size_t type */
#include <stdint.h> /* uint* types */
#include "result.h"

/** @brief Convert ASCII hex digit to its value.
 *
 *  @param[out] output Pointer to where output will be written.
 *  @param[in] c ASCII value of hex digit
 *  @return 1 if char was valid, 0 if char was invalid.
 */
int Bitcoin_DecodeHexChar(uint_fast8_t *output, char c);

/** @brief Convert pointer to multiple of two hex digits
 *          (hex representation of complete bytes) to its byte values.
 *
 *  @param[out] output Buffer to write decoded values to.
 *  @param[in] output_size Size of buffer to write decoded values to.
 *                     No more than output_size bytes will be written.
 *  @param[in] source Pointer to multiple of two hex digits.
 *  @param[in] source_size Count of multiple of two hex digits.

 *  @return BITCOIN_SUCCESS if chars were valid.
 */
BitcoinResult Bitcoin_DecodeHex(
	void *output, size_t output_size,
	size_t *decoded_output_size,
	const char *source, size_t source_size
);

/** @brief Convert multiple bytes of data to ASCII hex digits.
 *
 *  @param[out] output Buffer to write encoded values to.
 *  @param[in] output_size Size of buffer to write encoded values to.
 *                     No more than output_size bytes will be written.
 *  @param[out] encoded_output_size Number of characters written to output
                                    buffer.  This will be twice the number of
                                    input bytes unless a buffer overrun
                                    occurs.
 *  @param[in] source Pointer to raw data.
 *  @param[in] source_size Number of bytes of raw data.
 *  @param[in] lower_case Non-zero if output is to be lower-case, otherwise
                          output will be upper-case.

 *  @return BITCOIN_SUCCESS if all the input data was encoded.
 */
BitcoinResult Bitcoin_EncodeHex(
	char *output, size_t output_size,
	size_t *encoded_output_size,
	const void *source, size_t source_size,
	int lower_case
);

/** @brief Output the hex representation of a pointer to byte values,
 *         to standard output.
 *
 *  @param[in] source Pointer to byte values.
 *  @param[in] source_size Number of bytes to output.
 */
void Bitcoin_OutputHex(const void *source, size_t source_size);

/** @brief Reverse the bytes in a buffer.
 *         eg: {1,2,3,0} will become {0,3,2,1}
 *
 *  @param[out] buffer Pointer to byte values.
 *  @param[in] size Number of bytes to reverse.
 */
void Bitcoin_ReverseBytes(void *buffer, size_t size);

#endif

