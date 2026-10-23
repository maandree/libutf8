/* See LICENSE file for copyright and license details. */
#ifndef LIBUTF8_H
#define LIBUTF8_H

#include <stddef.h>
#include <stdint.h>

#if defined(__GNUC__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Winline"
#endif


/**
 * The greatest Unicode codepoint
 * 
 * @seealso  libutf8_is_invalid
 */
#define LIBUTF8_ULTIMATE_CODEPOINT UINT32_C(0x10FFFF)


/**
 * The Unicode replacement character as a UTF-32 codepoint
 * 
 * @seealso  LIBUTF8_REPLACEMENT_CHARACTER_UTF8
 */
#define LIBUTF8_REPLACEMENT_CHARACTER_CODEPOINT 0xFFFDu


/**
 * The Unicode replacement character encoded as UTF-8
 * 
 * @seealso  LIBUTF8_REPLACEMENT_CHARACTER_CODEPOINT
 */
#define LIBUTF8_REPLACEMENT_CHARACTER_UTF8 "\xEF\xBF\xBD"


/**
 * Replace a decoded invalid codepoint with
 * `LIBUTF8_REPLACEMENT_CHARACTER_CODEPOINT`
 * 
 * @seealso  LIBUTF8_RETURN_INVALID
 */
#define LIBUTF8_REPLACE_INVALID 0x0001u


/**
 * Return a decoded invalid codepoint unchanged
 * 
 * @seealso  LIBUTF8_REPLACE_INVALID
 */
#define LIBUTF8_RETURN_INVALID 0x0002u


/**
 * Accept UTF-8 sequences that decode past
 * `LIBUTF8_ULTIMATE_CODEPOINT` to a maximum of 31 bits
 */
#define LIBUTF8_ALLOW_31_BITS 0x0004u


/**
 * Accept the two-byte overlong encoding of U+0000
 * 
 * @seealso  LIBUTF8_ALLOW_LONG_ENCODING
 */
#define LIBUTF8_ALLOW_2BYTE_NULL 0x0008u


/**
 * Accept all overlong UTF-8 encodings
 * 
 * This flag implies LIBUTF8_ALLOW_2BYTE_NULL
 * 
 * @seealso  LIBUTF8_ALLOW_2BYTE_NULL
 */
#define LIBUTF8_ALLOW_LONG_ENCODING 0x0010u


/**
 * Test whether a codepoint is a non-character Unicode codepoint
 * 
 * @param   codepoint  The codepoint to test
 * @return             1 if `codepoint` is a non-character Unicode codepoint, 0 otherwise
 * 
 * A non-character Unicode codepoint is codepoint
 * reserved as invalid and without special meaning
 * 
 * The result is unspecified if `codepoint` is
 * greater than LIBUTF8_ULTIMATE_CODEPOINT
 * 
 * @seealso  libutf8_is_invalid
 * 
 * This function is MT-Safe, AS-Safe, and AC-Safe
 */
#if defined(__GNUC__)
__attribute__((const, warn_unused_result))
#endif
inline int
libutf8_is_noncharacter(uint_least32_t codepoint)
{
	return (0xFDD0u <= codepoint && codepoint <= 0xFDEFu) || (codepoint & 0xFFFEu) == 0xFFFEu;
}


/**
 * Test whether a codepoint is a UTF-16 surrogate
 * 
 * @param   codepoint  The codepoint to test
 * @return             1 if `codepoint` is a UTF-16 surrogate, 0 otherwise
 * 
 * @seealso  libutf8_is_invalid
 * 
 * This function is MT-Safe, AS-Safe, and AC-Safe
 */
#if defined(__GNUC__)
__attribute__((const, warn_unused_result))
#endif
inline int
libutf8_is_surrogate(uint_least32_t codepoint)
{
	return 0xD800u <= codepoint && codepoint <= 0xDFFFu;
}


/**
 * Test whether a codepoint is invalid Unicode
 * 
 * @param   codepoint  The codepoint to test
 * @return             1 if `codepoint` is outside the Unicode range,
 *                     a surrogate, or a non-character; 0 otherwise
 * 
 * @seealso  libutf8_is_valid
 * @seealso  LIBUTF8_ULTIMATE_CODEPOINT
 * @seealso  libutf8_is_noncharacter
 * @seealso  libutf8_is_surrogate
 * 
 * This function is MT-Safe, AS-Safe, and AC-Safe
 */
#if defined(__GNUC__)
__attribute__((const, warn_unused_result))
#endif
inline int
libutf8_is_invalid(uint_least32_t codepoint)
{
	return codepoint > LIBUTF8_ULTIMATE_CODEPOINT ||
	       libutf8_is_noncharacter(codepoint) ||
	       libutf8_is_surrogate(codepoint);
}


/**
 * Test whether a codepoint is valid Unicode
 * 
 * @param   codepoint  The codepoint to test
 * @return             1 if `codepoint` is within the Unicode range
 *                     and is neither a surrogate nor a non-character,
 *                     0 otherwise
 * 
 * @seealso  libutf8_is_invalid
 * 
 * This function is MT-Safe, AS-Safe, and AC-Safe
 */
#if defined(__GNUC__)
__attribute__((const, warn_unused_result))
#endif
inline int
libutf8_is_valid(uint_least32_t codepoint)
{
	return !libutf8_is_invalid(codepoint);
}


/**
 * Test whether a byte is a UTF-8 continuation byte
 * 
 * @param   c  The byte to test
 * @return     1 if `c` is a codepoint continuation byte,
 *             0 otherwise
 * 
 * This function is MT-Safe, AS-Safe, and AC-Safe
 */
#if defined(__GNUC__)
__attribute__((const, warn_unused_result))
#endif
inline int
libutf8_is_continuation(char c)
{
	return ((unsigned char)c & 0xC0u) == 0x80u;
}


/**
 * Get the number of bytes required to encode a codepoint
 * 
 * @param   codepoint  The codepoint to measure
 * @return             The number of bytes required to encode `codepoint`;
 *                     0 if `codepoint` does not fit in an unsigned 31-bit integer
 * 
 * The maximum return value is 4 for valid Unicode
 * codepoints and 6 for any value accept by `libutf8_encode`
 * 
 * This function never sets `errno`, however an
 * appropriate `errno`-value is `EDOM` when this
 * function returns 0
 * 
 * @seealso  libutf8_encode
 * 
 * This function is MT-Safe, AS-Safe, and AC-Safe
 */
#if defined(__GNUC__)
__attribute__((const, warn_unused_result))
#endif
inline size_t
libutf8_encode_measure(uint_least32_t codepoint)
{
	if (codepoint >>  7 == 0u) return 1u;
	if (codepoint >> 11 == 0u) return 2u;
	if (codepoint >> 16 == 0u) return 3u;
	if (codepoint >> 21 == 0u) return 4u;
	if (codepoint >> 26 == 0u) return 5u;
	if (codepoint >> 31 == 0u) return 6u;
	return 0u;
}


/**
 * Encode a codepoint as UTF-8
 * 
 * @param   out_buffer  Output buffer; must have room for the number
 *                      of bytes required to encode `codepoint`
 * @param   codepoint   The codepoint to encode
 * @return              The number of bytes written
 * 
 * NB! The output is not NUL-terminated
 * 
 * Besides Unicode codepoints, this function encodes 31-bit
 * values greater than `LIBUTF8_ULTIMATE_CODEPOINT` using
 * historical UTF-8 sequences up to 6 bytes. This lets
 * applications embed application-specific data in a file.
 * 
 * This function never sets `errno`, however an
 * appropriate `errno`-value is `EDOM` when this
 * function returns 0
 * 
 * @seealso  libutf8_encode_measure
 * @seealso  libutf8_is_valid
 * @seealso  libutf8_decode
 * 
 * This function is MT-Safe, AS-Safe, and AC-Safe
 */
#if defined(__clang__)
__attribute__((nonnull(1), warn_unused_result))
#elif defined(__GNUC__)
__attribute__((nonnull(1), access(write_only, 1), warn_unused_result))
#endif
size_t libutf8_encode(char *out_buffer, uint_least32_t codepoint);


/**
 * Decode the first UTF-8 sequence in a string
 * 
 * @param   text           The input string; must contain a complete candidate sequence
 * @param   codepoint_out  Unless `NULL`, output for the decoded codepoint
 * @param   flags          Bitwise OR of zero or more or the following flags:
 *                         -  LIBUTF8_REPLACE_INVALID:
 *                                Replace invalid codepoints, up to `LIBUTF8_ULTIMATE_CODEPOINT`,
 *                                with the REPLACEMENT CHARACTER.
 *                                NB! This only affects invalid codepoints, not illegal byte sequences.
 *                         -  LIBUTF8_RETURN_INVALID:
 *                                Treat invalid codepoints, up to `LIBUTF8_ULTIMATE_CODEPOINT`, as valid
 *                         -  LIBUTF8_ALLOW_31_BITS
 *                                Treat 31-bit codepoints greater than
 *                                `LIBUTF8_ULTIMATE_CODEPOINT` as valid.
 *                         -  LIBUTF8_ALLOW_2BYTE_NULL
 *                                Accept the overlong 2-byte encoding of U+0000.
 *                         -  LIBUTF8_ALLOW_LONG_ENCODING
 *                                Accept overlong encodings, up to 6 bytes.
 * @return                 The number of decoded bytes, 0 if the input is invalid
 * 
 * It is unspecified which flag dominates:
 * LIBUTF8_REPLACE_INVALID or LIBUTF8_RETURN_INVALID
 * 
 * This function never sets `errno`, however an
 * appropriate `errno`-value is `EILSEQ` when this
 * function returns 0
 * 
 * @seealso  libutf8_decode_mem
 * @seealso  libutf8_is_continuation
 * 
 * This function is MT-Safe, AS-Safe, and AC-Safe
 */
#if defined(__clang__)
__attribute__((nonnull(1), warn_unused_result))
#elif defined(__GNUC__)
__attribute__((nonnull(1), access(write_only, 2), warn_unused_result))
#endif
size_t libutf8_decode(const char *text, uint_least32_t *codepoint_out, unsigned flags);


/**
 * Decode the first UTF-8 sequence in a string
 * 
 * @param   text           The input string; must contain a complete candidate sequence
 * @param   text_len       The number of bytes available in `text`
 * @param   codepoint_out  Unless `NULL`, output for the decoded codepoint
 * @param   flags          Bitwise OR of zero or more or the following flags:
 *                         -  LIBUTF8_REPLACE_INVALID:
 *                                Replace invalid codepoints, up to `LIBUTF8_ULTIMATE_CODEPOINT`,
 *                                with the REPLACEMENT CHARACTER.
 *                                NB! This only affects invalid codepoints, not illegal byte sequences.
 *                         -  LIBUTF8_RETURN_INVALID:
 *                                Treat invalid codepoints, up to `LIBUTF8_ULTIMATE_CODEPOINT`, as valid
 *                         -  LIBUTF8_ALLOW_31_BITS
 *                                Treat 31-bit codepoints greater than
 *                                `LIBUTF8_ULTIMATE_CODEPOINT` as valid.
 *                         -  LIBUTF8_ALLOW_2BYTE_NULL
 *                                Accept the overlong 2-byte encoding of U+0000.
 *                         -  LIBUTF8_ALLOW_LONG_ENCODING
 *                                Accept overlong encodings, up to 6 bytes.
 * @return                 The number of decoded bytes, 0 if the input is invalid;
 *                         or the required sequence length specified by a valid
 *                         leading byte if `text_len` is less than this value
 *                         (input is truncated)
 *
 * Note, that a return value greater than `text_len` does not
 * necessarily (it is unspecified) indicate that all available
 * bytes are invalid, only that the first byte is valid
 * 
 * This function never sets `errno`, however an
 * appropriate `errno`-value is `EILSEQ` when this
 * function returns 0
 * 
 * @seealso  libutf8_decode
 * @seealso  libutf8_is_continuation
 * 
 * This function is MT-Safe, AS-Safe, and AC-Safe
 */
#if defined(__clang__)
__attribute__((warn_unused_result))
#elif defined(__GNUC__)
__attribute__((access(read_only, 1, 2), access(write_only, 3), warn_unused_result))
#endif
size_t libutf8_decode_mem(const char *text, size_t text_len, uint_least32_t *codepoint_out, unsigned flags);


#if defined(__GNUC__)
# pragma GCC diagnostic pop
#endif

#endif
