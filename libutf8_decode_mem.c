/* See LICENSE file for copyright and license details. */
#include "common.h"
#ifndef TEST


size_t
libutf8_decode_mem(const char *text, size_t text_len, uint_least32_t *codepoint_out, unsigned flags)
{
	if (!text_len)
		return 0u;

	DECODE(if (n > text_len) return n;);
}


#else
# ifndef FUZZ


# define NONE (uint_least32_t)~(uint_least32_t)0


static int
long_encode(char *out_buffer, uint_least32_t codepoint, size_t n)
{
	if (codepoint >> 31) {
		if (n != 6u)
			return 0;
	} else {
		if (libutf8_encode_measure(codepoint) > n || n > 6u)
			return 0;
	}
	ENCODE(n);
	return 1;
}


static void
check(const char *buf, uint_least32_t codepoint, size_t decode_avail, size_t encode_len, unsigned flags, int overlong)
{
	uint_least32_t expect_codepoint, replacement, res;
	size_t decode_len;
	int extraultimate;

	extraultimate = (codepoint > (uint_least32_t)LIBUTF8_ULTIMATE_CODEPOINT && !(codepoint >> 31));

	if (decode_avail >= encode_len) {
		expect_codepoint = codepoint;
		replacement = (uint_least32_t)LIBUTF8_REPLACEMENT_CHARACTER_CODEPOINT;
	} else {
		expect_codepoint = NONE;
		replacement = expect_codepoint;
	}

	res = NONE;
	decode_len = libutf8_decode_mem(buf, decode_avail, &res, flags | LIBUTF8_RETURN_INVALID);
	if (overlong &&
	    !(flags & LIBUTF8_ALLOW_LONG_ENCODING) &&
	    !(encode_len == 2u && !codepoint && (flags & LIBUTF8_ALLOW_2BYTE_NULL)) &&
	    !decode_len)
		goto expect_zero;
	if (extraultimate && !decode_len)
		goto expect_zero;
	if (codepoint >> 31)
		goto expect_zero;
	assert(decode_len == encode_len);
	assert(res == expect_codepoint);

	assert(libutf8_decode_mem(buf, decode_avail, NULL, flags | LIBUTF8_RETURN_INVALID) == encode_len);

	if (extraultimate && (flags & LIBUTF8_ALLOW_31_BITS))
		goto normal;

	if (libutf8_is_invalid(codepoint)) {
		decode_len = libutf8_decode_mem(buf, decode_avail, &res, 0u);
		if ((decode_len && decode_avail < encode_len) || decode_avail < 2u)
			goto normal;

		res = NONE;
		assert(decode_len == 0u);
		assert(res == NONE);

		res = NONE;
		assert(libutf8_decode_mem(buf, decode_avail, &res, flags | LIBUTF8_REPLACE_INVALID) == encode_len);
		assert(res == replacement);

		res = NONE;
		decode_len = libutf8_decode_mem(buf, decode_avail, &res, flags | LIBUTF8_REPLACE_INVALID
		                                                               | LIBUTF8_RETURN_INVALID);
		assert(!decode_len || decode_len == encode_len);
		if (decode_len)
			assert(res == replacement);
		else
			assert(res == NONE);
	} else {
	normal:
		res = NONE;
		assert(libutf8_decode_mem(buf, decode_avail, &res, flags) == encode_len);
		assert(res == expect_codepoint);

		res = NONE;
		assert(libutf8_decode_mem(buf, decode_avail, &res, flags | LIBUTF8_REPLACE_INVALID) == encode_len);
		assert(res == expect_codepoint);

		res = NONE;
		assert(libutf8_decode_mem(buf, decode_avail, &res, flags | LIBUTF8_REPLACE_INVALID
		                                                         | LIBUTF8_RETURN_INVALID) == encode_len);
		assert(res == expect_codepoint);
	}

	return;

expect_zero:
	assert(!libutf8_decode_mem(buf, decode_avail, &res, flags));
	assert(!libutf8_decode_mem(buf, decode_avail, &res, flags | LIBUTF8_REPLACE_INVALID));
	assert(!libutf8_decode_mem(buf, decode_avail, &res, flags | LIBUTF8_REPLACE_INVALID
	                                                          | LIBUTF8_RETURN_INVALID));
}


static void
check_codepoint(uint_least32_t codepoint)
{
	static const unsigned extra_flags[] = {
		0u,
		LIBUTF8_ALLOW_31_BITS,
		LIBUTF8_ALLOW_2BYTE_NULL,
		LIBUTF8_ALLOW_LONG_ENCODING,
		LIBUTF8_ALLOW_31_BITS | LIBUTF8_ALLOW_2BYTE_NULL,
		LIBUTF8_ALLOW_31_BITS | LIBUTF8_ALLOW_LONG_ENCODING,
		LIBUTF8_ALLOW_2BYTE_NULL | LIBUTF8_ALLOW_LONG_ENCODING,
		LIBUTF8_ALLOW_31_BITS | LIBUTF8_ALLOW_2BYTE_NULL | LIBUTF8_ALLOW_LONG_ENCODING
	};

	static size_t corruption = 0u;

	size_t flags_i, decode_avail, encode_len;
	char buf[128];
	int overlong;
	unsigned flags;
	size_t q, r;

	for (flags_i = 0u; flags_i < sizeof(extra_flags) / sizeof(*extra_flags); flags_i++) {
		flags = extra_flags[flags_i];

		overlong = 0;
		for (encode_len = 1u; encode_len <= 6u; encode_len++) {
			if (!long_encode(buf, codepoint, encode_len))
				continue;

			for (decode_avail = 1u; decode_avail < encode_len + 4u; decode_avail++)
				check(buf, codepoint, decode_avail, encode_len, flags, overlong);

			overlong = 1;
		}
	}

	if (codepoint >> 7) {
		encode_len = libutf8_encode_measure(codepoint);
		encode_len = encode_len ? encode_len : 6u;
		long_encode(buf, codepoint, encode_len);
		q = corruption / 3u;
		r = corruption % 3u;
		q %= encode_len - 1u;
		buf[q + 1u] = (char)((unsigned char)buf[q + 1u] ^ ((r ^ 3u) << 6));
		corruption += 1u;
		assert(libutf8_decode_mem(buf, sizeof(buf), &(uint_least32_t){0u}, ~0u) == 0u);
	}
}


int
main(void)
{
	static const char *rejected[] = {
		"\x80",
		"\x80\x80",
		"\x80\x80\x80",
		"\x80\x80\x80\x80",
		"\x80\x80\x80\x80\x80",
		"\x80\x80\x80\x80\x80\x80",
		"\x80\x80\x80\x80\x80\x80\x80",
		"\x80\x80\x80\x80\x80\x80\x80\x80",
		"\xFE",
		"\xFE\x80",
		"\xFE\x80\x80",
		"\xFE\x80\x80\x80",
		"\xFE\x80\x80\x80\x80",
		"\xFE\x80\x80\x80\x80\x80",
		"\xFE\x80\x80\x80\x80\x80\x80",
		"\xFE\x80\x80\x80\x80\x80\x80\x80",
		"\xFF",
		"\xFF\x80",
		"\xFF\x80\x80",
		"\xFF\x80\x80\x80",
		"\xFF\x80\x80\x80\x80",
		"\xFF\x80\x80\x80\x80\x80",
		"\xFF\x80\x80\x80\x80\x80\x80",
		"\xFF\x80\x80\x80\x80\x80\x80\x80",
		NULL
	};

	uint_least32_t cp, res;
	char buf[128];
	size_t i;

	SET_UP_TEST();

	assert(libutf8_decode_mem(buf, 0u, &cp, LIBUTF8_RETURN_INVALID | LIBUTF8_ALLOW_31_BITS) == 0u);

	for (cp = 0u; cp <= (uint_least32_t)LIBUTF8_ULTIMATE_CODEPOINT;) {
		check_codepoint(cp);
		if (!(++cp & 0x7FFFu))
			ARM_ALARM();
	}

	check_codepoint((uint_least32_t)0x110000ul);
	check_codepoint((uint_least32_t)(1ul << 22));
	check_codepoint((uint_least32_t)(1ul << 23));
	check_codepoint((uint_least32_t)(1ul << 24));
	check_codepoint((uint_least32_t)(1ul << 25));
	check_codepoint((uint_least32_t)(1ul << 26));
	check_codepoint((uint_least32_t)(1ul << 27));
	check_codepoint((uint_least32_t)(1ul << 28));
	check_codepoint((uint_least32_t)(1ul << 29));
	check_codepoint((uint_least32_t)(1ul << 30));

	check_codepoint((uint_least32_t)0x11FFFFul);
	check_codepoint((uint_least32_t)((1ul << 22) - 1ul));
	check_codepoint((uint_least32_t)((1ul << 23) - 1ul));
	check_codepoint((uint_least32_t)((1ul << 24) - 1ul));
	check_codepoint((uint_least32_t)((1ul << 25) - 1ul));
	check_codepoint((uint_least32_t)((1ul << 26) - 1ul));
	check_codepoint((uint_least32_t)((1ul << 27) - 1ul));
	check_codepoint((uint_least32_t)((1ul << 28) - 1ul));
	check_codepoint((uint_least32_t)((1ul << 29) - 1ul));
	check_codepoint((uint_least32_t)((1ul << 30) - 1ul));
	check_codepoint((uint_least32_t)((1ul << 31) - 1ul));

	check_codepoint((uint_least32_t)(1ul << 31));
	check_codepoint((uint_least32_t)0xFFFFFFFFul);

	for (i = 0u; rejected[i]; i++) {
		res = NONE;
		assert(libutf8_decode_mem(rejected[i], strlen(rejected[i]), &res, ~0u) == 0u);
		assert(res == NONE);
		assert(libutf8_decode_mem(rejected[i], strlen(rejected[i]), NULL, ~0u) == 0u);
	}

	return 0;
}


# else


int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	char encoded[6];
	uint_least32_t cp;
	size_t n;

	n = libutf8_decode_mem((const char *)data, size, &cp,
	                       LIBUTF8_RETURN_INVALID | LIBUTF8_ALLOW_31_BITS | LIBUTF8_ALLOW_LONG_ENCODING);

	if (n && n <= size) {
		if (libutf8_decode_mem((const char *)data, size, &cp, LIBUTF8_RETURN_INVALID | LIBUTF8_ALLOW_31_BITS) == n) {
			assert(libutf8_encode(encoded, cp) == n);
			assert(!memcmp(encoded, data, n));
		}
	}

	return 0;
}


# endif
#endif
