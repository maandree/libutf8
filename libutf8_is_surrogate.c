/* See LICENSE file for copyright and license details. */
#include "common.h"
#ifndef TEST


extern inline int libutf8_is_surrogate(uint_least32_t codepoint);


#else


int
main(void)
{
	unsigned long int i = 0u;

	SET_UP_TEST();

	for (; i <   0xD800ul; i++) assert(libutf8_is_surrogate((uint_least32_t)i) == 0);
	for (; i <   0xE000ul; i++) assert(libutf8_is_surrogate((uint_least32_t)i) == 1);
	for (; i < 0x120000ul; i++) assert(libutf8_is_surrogate((uint_least32_t)i) == 0);

	return 0;
}


#endif
