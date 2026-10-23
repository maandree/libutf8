/* See LICENSE file for copyright and license details. */
#include "common.h"
#ifndef TEST


extern inline int libutf8_is_noncharacter(uint_least32_t codepoint);


#else


int
main(void)
{
	unsigned long int i = 0u, j;

	SET_UP_TEST();

	for (; i <   0xFDD0ul; i++) assert(libutf8_is_noncharacter((uint_least32_t)i) == 0);
	for (; i <   0xFDF0ul; i++) assert(libutf8_is_noncharacter((uint_least32_t)i) == 1);
	for (; i <   0xFFFEul; i++) assert(libutf8_is_noncharacter((uint_least32_t)i) == 0);
	for (; i <  0x10000ul; i++) assert(libutf8_is_noncharacter((uint_least32_t)i) == 1);
	for (; i < 0x10FFFFul; i += 0x10000ul) {
		j = i;
		for (; j < i +  0xFFFEul; j++) assert(libutf8_is_noncharacter((uint_least32_t)j) == 0);
		for (; j < i + 0x10000ul; j++) assert(libutf8_is_noncharacter((uint_least32_t)j) == 1);
	}

	return 0;
}


#endif
