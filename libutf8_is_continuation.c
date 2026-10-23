/* See LICENSE file for copyright and license details. */
#include "common.h"
#ifndef TEST


extern inline int libutf8_is_continuation(char c);


#else


int
main(void)
{
	unsigned i = 0u;

	SET_UP_TEST();

	for (; i < 0x80u; i++) assert(libutf8_is_continuation((char)i) == 0);
	for (; i < 0xC0u; i++) assert(libutf8_is_continuation((char)i) == 1);
	for (; i < 0xFFu; i++) assert(libutf8_is_continuation((char)i) == 0);

	return 0;
}


#endif
