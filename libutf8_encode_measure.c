/* See LICENSE file for copyright and license details. */
#include "common.h"
#ifndef TEST


extern inline size_t libutf8_encode_measure(uint_least32_t codepoint);


#else


#define LIMIT(BITS) ((uint_least32_t)(1ul << (BITS)))


static void
check(uint_least32_t cp, size_t n)
{
	char buf[128];
	uint_least32_t rcp;

	memset(buf, 255, sizeof(buf));

	assert(libutf8_encode_measure(cp) == n);
	assert(libutf8_encode(buf, cp) == n);

	if (!n)
		return;

	buf[n] = '\0';
	rcp = 0u;
	rcp = ~rcp;

	assert(libutf8_decode(buf, &rcp, LIBUTF8_RETURN_INVALID | LIBUTF8_ALLOW_31_BITS) == n);
	assert(rcp == cp);
}


static void
check_range(uint_least32_t start, uint_least32_t end, size_t n)
{
	uint_least32_t cp = 0u;
	unsigned i;

	check(start++, n);
	check(start++, n);
	check(start++, n);
	check(start++, n);

	check(--end, n);
	check(--end, n);
	check(--end, n);
	check(--end, n);

	for (i = 0u; i < 1000u; i++) {
		cp ^= (uint_least32_t)((uint_least32_t)rand() << 0);
		cp ^= (uint_least32_t)((uint_least32_t)rand() << 8);
		cp ^= (uint_least32_t)((uint_least32_t)rand() << 16);
		cp ^= (uint_least32_t)((uint_least32_t)rand() << 24);
		cp %= (uint_least32_t)(end - start);
		cp += start;
		check(cp, n);
	}
}

int
main(void)
{
	uint_least32_t i = 0u;

	SET_UP_TEST();

	for (; i < LIMIT(7); i++) check(i, 1u);
	for (; i < LIMIT(6 * 1 + 5); i++) check(i, 2u);
	for (; i < LIMIT(6 * 2 + 4); i++) check(i, 3u);
	for (; i < LIMIT(6 * 3 + 3); i++) check(i, 4u);
	check_range(i, LIMIT(6 * 4 + 2), 5u);
	check_range(LIMIT(6 * 4 + 2), LIMIT(31), 6u);
	check_range(LIMIT(31), (uint_least32_t)~(uint_least32_t)0, 0u);

	return 0;
}


#endif
