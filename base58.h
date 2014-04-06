#ifndef BITCOIN_INCLUDE_BASE58_H
#define BITCOIN_INCLUDE_BASE58_H

/** @file base58.h
 *  @brief Function prototypes for Base58/Base58Check functions.
 *
 *  Base58 is designed to be a printable representation of binary data
 *  with the following properties:
 *  - No punctuation symbols used, so double-click selection works more
 *    reliably than the Base64 equivalent, and prevents word-wrap from
 *    splitting the string across multiple lines.
 *  - O/0/I/l characters are not used, to prevent them being confused with
 *    similar characters in certain fonts.
 *
 *  Base58Check builds on top of Base58 by adding a 4-byte checksum to the end
 *  of the Base58 representation, which is the most-significant four bytes of
 *  the SHA256 hash of the data.
 *
 *  @author Matthew Anger
 */

#include <stdlib.h> /* size_t */
#include <stdint.h> /* uint8_t */

#include "result.h" /* BitcoinResult / BITCOIN_SUCCESS / BITCOIN_ERROR_* */

/** Base58Check defines a four byte suffix to be used as the checksum */
#define BITCOIN_BASE58CHECK_CHECKSUM_SIZE 4

/** @brief Convert a sequence of bytes to its Base58 representation.
 *
 *  @param[out] output Pointer to output buffer for writing Base58 string.
 *  @param[out] output_size Pointer to size of output buffer.  No more bytes
 *              than the number pointed to by 'output_size' will be written to
 *              'output'.  The number of bytes actually decoded will be written
 *              to the contents pointed to by 'output_size'.
 *  @param[in] source Pointer to sequence of bytes to read.
 *  @param[in] source_size Number of bytes to read.
 *
 *  @return BitcoinResult indicating error state :
 *          BITCOIN_SUCCESS if success.
 *          BITCOIN_ERROR_OUTPUT_BUFFER_TOO_SMALL if output buffer too small.
 */
BitcoinResult Bitcoin_EncodeBase58(
	char *output, size_t output_size, size_t *encoded_output_size,
	const void *source, size_t source_size
);

/** @brief Convert a sequence of bytes to its Base58Check representation.
 *
 *  @param[out] output Pointer to output buffer for writing Base58 string.
 *  @param[out] output_size Pointer to size of output buffer.  No more bytes
 *              than the number pointed to by 'output_size' will be written to
 *              'output'.  The number of bytes actually decoded will be written
 *              to the contents pointed to by 'output_size'.
 *  @param[in] source Pointer to sequence of bytes to read.
 *  @param[in] source_size Number of bytes to read.
 *
 *  @return BitcoinResult indicating error state :
 *          BITCOIN_SUCCESS if success.
 *          BITCOIN_ERROR_OUTPUT_BUFFER_TOO_SMALL if output buffer too small.
 */
BitcoinResult Bitcoin_EncodeBase58Check(
	char *output, size_t output_size, size_t *encoded_output_size,
	const void *source, size_t source_size
);

/** @brief Convert a Base58 string to its binary representation.
 *         Base58 (non-check) has no checksum to check, so be careful of
 *         passing incorrect input - errors will not be found.
 *         The output buffer will be modified even if the input cannot be
 *         decoded correctly.
 *
 *  @param[out] output Pointer to write decoded output into.
 *  @param[out] output_buffer_size Size in bytes of the output buffer.
 *              This specifies the maximum number of bytes to be output.
 *  @param[out] decoded_output_size Size in bytes of the decoded output.
 *              This can be 0 for empty input, or error condition, up to the
 *              maximum specified in output_buffer_size.
 *  @param[in] source Pointer to Base58 string to read.
 *  @param[in] source_size Number of characters to read from 'source'.
 *
 *  @return BitcoinResult indicating error state :
 *          BITCOIN_SUCCESS if success.
 *          BITCOIN_ERROR_OUTPUT_BUFFER_TOO_SMALL if output buffer too small.
 */
BitcoinResult Bitcoin_DecodeBase58(
	uint8_t *output, size_t output_buffer_size, size_t *decoded_output_size,
	const void *input, size_t input_size
);

/** @brief Convert a Base58Check string to its binary representation.
 *         The checksum is checked during decoding.
 *         The output buffer will be modified even if the checksum failed.
 *
 *  @param[out] output Pointer to write decoded output into.
 *  @param[out] output_buffer_size Size in bytes of the output buffer.
 *              This specifies the maximum number of bytes to be output.
 *  @param[out] decoded_output_size Size in bytes of the decoded output.
 *              This can be 0 for empty input, or error condition, up to the
 *              maximum specified in output_buffer_size.
 *  @param[in] source Pointer to Base58Check string to read.
 *  @param[in] source_size Number of characters to read from 'source'.
 *
 *  @return BitcoinResult indicating error state :
 *          BITCOIN_SUCCESS if success.
 *          BITCOIN_ERROR_CHECKSUM_FAILURE if checksum failed.
 *          BITCOIN_ERROR_OUTPUT_BUFFER_TOO_SMALL if output buffer too small.
 */
BitcoinResult Bitcoin_DecodeBase58Check(
	uint8_t *output, size_t output_buffer_size, size_t *decoded_output_size,
	const void *input, size_t input_size
);

/** @brief Convert a Base58Check string to its binary representation, changing
 *         characters necessary to make the checksum valid.
 *         This is a very much NOT recommended, and last-ditch, effort of
 *         fixing bad input (typos, damaged printout, etc).
 *         The output buffer will be modified even if no recovery was possible.
 *
 *  @param[out] fixed_output Pointer to write fixed Base58Check output into.
 *  @param[in] fixed_output_buffer_size Size in bytes of the fixed Base58Check
               output buffer.  This specifies the maximum number of bytes to be
               output.
 *  @param[out] fixed_output_size Size in bytes of the fixed Base58Check output.
 *              This can be 0 for empty input, or error condition, up to the
 *              maximum specified in output_buffer_size
 *  @param[out] output Pointer to write decoded output into.
 *  @param[out] output_buffer_size Size in bytes of the output buffer.
 *              This specifies the maximum number of bytes to be output.
 *  @param[out] decoded_output_size Size in bytes of the decoded output.
 *              This can be 0 for empty input, or error condition, up to the
 *              maximum specified in output_buffer_size.
 *  @param[in] input Pointer to Base58Check string to read.
 *  @param[in] input_size Number of characters to read from 'source'.
 *  @param[in] change_chars Maximum number of characters to change.  Every
 *                          possible combination of characters up to this
 *                          amount will be tested, this can take a long time
 *                          for large numbers.
 *  @param[in] insert_chars Maximum number of characters to insert.  Every
 *                          possible combination of inserted characters up to
 *                          this amount will be tested.
 *  @param[in] remove_chars Maximum number of characters to remove.  Every
 *                          possible combination of inserted characters up to
 *                          this amount will be tested.
 *
 *  @return BitcoinResult indicating error state :
 *          BITCOIN_SUCCESS if success.
 *          BITCOIN_ERROR_CHECKSUM_FAILURE if checksum failed.
 *          BITCOIN_ERROR_OUTPUT_BUFFER_TOO_SMALL if output buffer too small.
 */
BitcoinResult Bitcoin_FixBase58Check(
	char *fixed_output, size_t fixed_output_buffer_size, size_t *fixed_output_size,
	uint8_t *output, size_t output_buffer_size, size_t *decoded_output_size,
	const char *input, size_t input_size,
	unsigned change_chars,
	unsigned insert_chars,
	unsigned remove_chars
);

#endif
