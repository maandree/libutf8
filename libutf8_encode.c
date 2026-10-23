/* See LICENSE file for copyright and license details. */
#include "common.h"
#ifndef TEST


size_t
libutf8_encode(char *out_buffer, uint_least32_t codepoint)
{
	size_t n = libutf8_encode_measure(codepoint);
	ENCODE(n);
	return n;
}


#else


int
main(void)
{
	/* Already mostly tested in libutf8_encode_measure */

	uint_least32_t cp;
	char buf[128];
	char ones[128];
	unsigned char bit;
	size_t i, n;

	SET_UP_TEST();

	memset(ones, 255, sizeof(ones));

	for (cp = 0u; cp < (uint_least32_t)0x200000ul; cp++) {
		memset(buf, 255, sizeof(buf));
		n = libutf8_encode(buf, cp);
		assert(!memcmp(&buf[n], ones, sizeof(buf) - n));

		for (i = 1u; i < n; i++)
			assert(((unsigned char)buf[i] & 0xC0u) == 0x80u);

		if (n == 1u) {
			assert(((unsigned char)buf[0u] & 0x80u) == 0x00u);
			continue;
		}

		bit = (unsigned char)0x80u;
		for (i = 0u; i < n; i++, bit >>= 1)
			assert((unsigned char)buf[0u] & bit);
		assert(!((unsigned char)buf[0u] & bit));
	}

	memset(buf, 255, sizeof(buf));
	assert(libutf8_encode(buf, (uint_least32_t)1 << 31) == 0u);
	assert(!memcmp(buf, ones, sizeof(buf)));

	return 0;
}


#endif
