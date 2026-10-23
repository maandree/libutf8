/* See LICENSE file for copyright and license details. */
#include "common.h"
#ifndef TEST


extern inline int libutf8_is_invalid(uint_least32_t codepoint);


#else


int
main(void)
{
	unsigned long int i = 0u;
	uint_least32_t cp;

	SET_UP_TEST();

	for (; i <= 0x10FFFFul; i++) {
		cp = (uint_least32_t)i;
		if (libutf8_is_noncharacter(cp))
			assert(libutf8_is_invalid(cp) == 1);
		else if (libutf8_is_surrogate(cp))
			assert(libutf8_is_invalid(cp) == 1);
		else
			assert(libutf8_is_invalid(cp) == 0);
	}
	for (; i < 0x120000ul; i++)
		assert(libutf8_is_invalid((uint_least32_t)i) == 1);

	return 0;
}


#endif
