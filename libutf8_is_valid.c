/* See LICENSE file for copyright and license details. */
#include "common.h"
#ifndef TEST


extern inline int libutf8_is_valid(uint_least32_t codepoint);


#else


int
main(void)
{
	unsigned long int i;
	uint_least32_t cp;

	SET_UP_TEST();

	for (i = 0u; i < 0x120000ul; i++) {
		cp = (uint_least32_t)i;
		assert(libutf8_is_valid(cp) == !libutf8_is_invalid(cp));
	}

	return 0;
}


#endif
