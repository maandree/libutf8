/* See LICENSE file for copyright and license details. */
#include "libutf8.h"


#if defined(__clang__)
# pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
# pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#endif

#if defined(__GNUC__)
# pragma GCC diagnostic ignored "-Winline"
#endif


#define DECODE(LENGTH_CHECK)\
	do {\
		static const uint_least32_t limits[] = {\
			(UINT32_C(1) <<  0),\
			(UINT32_C(1) <<  7),\
			(UINT32_C(1) << 11),\
			(UINT32_C(1) << 16),\
			(UINT32_C(1) << 21),\
			(UINT32_C(1) << 26),\
			(UINT32_C(1) << 31)\
		};\
		\
		uint_least32_t codepoint, lower_limit;\
		unsigned char c;\
		size_t n, char_len = 0u;\
		\
		c = ((const unsigned char *)text)[char_len++];\
		\
		n = 0u;\
		while (c & 0x80u) {\
			c = (unsigned char)(c << 1);\
			n += 1u;\
		}\
		\
		if (n == 0u) {\
			codepoint = (uint_least32_t)c;\
			goto out;\
		}\
		if (n == 1u || n >= sizeof(limits) / sizeof(*limits))\
			return 0u;\
		\
		LENGTH_CHECK\
		\
		lower_limit = limits[n - 1u];\
		\
		codepoint = (uint_least32_t)((c & 0xFFu) >> n);\
		while (--n) {\
			c = ((const unsigned char *)text)[char_len++];\
			if ((c & 0xC0u) != 0x80u)\
				return 0u;\
			codepoint <<= 6;\
			codepoint |= (uint_least32_t)(c & 0x3Fu);\
		}\
		\
		if (codepoint < lower_limit && !(flags & LIBUTF8_ALLOW_LONG_ENCODING))\
			if (codepoint || char_len > 2u || !(flags & LIBUTF8_ALLOW_2BYTE_NULL))\
				return 0u;\
		\
		if (codepoint > LIBUTF8_ULTIMATE_CODEPOINT) {\
			if (!(flags & LIBUTF8_ALLOW_31_BITS) || codepoint >> 31)\
				return 0u;\
		} else if (flags & LIBUTF8_REPLACE_INVALID) {\
			if (libutf8_is_noncharacter(codepoint) || libutf8_is_surrogate(codepoint))\
				codepoint = LIBUTF8_REPLACEMENT_CHARACTER_CODEPOINT;\
		} else if (!(flags & LIBUTF8_RETURN_INVALID)) {\
			if (libutf8_is_noncharacter(codepoint) || libutf8_is_surrogate(codepoint))\
				return 0u;\
		}\
		\
	out:\
		if (codepoint_out)\
			*codepoint_out = codepoint;\
		return char_len;\
	} while (0)


#define ENCODE(N)\
	do {\
		switch (N) {\
		default:\
			break;\
		case 1u:\
			out_buffer[0u] = (char)codepoint;\
			break;\
		case 6u:\
			out_buffer[5u] = (char)(0x80u | ((unsigned char)(codepoint & 0x3Fu)));\
			codepoint >>= 6;\
			/* fall through */\
		case 5u:\
			out_buffer[4u] = (char)(0x80u | ((unsigned char)(codepoint & 0x3Fu)));\
			codepoint >>= 6;\
			/* fall through */\
		case 4u:\
			out_buffer[3u] = (char)(0x80u | ((unsigned char)(codepoint & 0x3Fu)));\
			codepoint >>= 6;\
			/* fall through */\
		case 3u:\
			out_buffer[2u] = (char)(0x80u | ((unsigned char)(codepoint & 0x3Fu)));\
			codepoint >>= 6;\
			/* fall through */\
		case 2u:\
			out_buffer[1u] = (char)(0x80u | ((unsigned char)(codepoint & 0x3Fu)));\
			codepoint >>= 6;\
			\
			out_buffer[0u] = (char)(((0xFFu << (8u - (N))) & 0xFFu) | codepoint);\
			break;\
		}\
	 } while (0)


#ifdef TEST
# ifdef __linux__
#  include <sys/prctl.h>
# endif
# include <sys/resource.h>
# include <assert.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>
# include <unistd.h>

# define ARM_ALARM()\
	do {\
		unsigned int alarm_time__ = alarm(10u);\
		if (alarm_time__ > 10u)\
			alarm(alarm_time__);\
	} while (0)

# if defined(PR_SET_DUMPABLE)
#  define DISABLE_COREDUMP()\
	do {\
		struct rlimit rl__;\
		rl__.rlim_cur = 0;\
		rl__.rlim_max = 0;\
		(void) setrlimit(RLIMIT_CORE, &rl__);\
		(void) prctl(PR_SET_DUMPABLE, 0);\
	} while (0)
# else
#  define DISABLE_COREDUMP()\
	do {\
		struct rlimit rl__;\
		rl__.rlim_cur = 0;\
		rl__.rlim_max = 0;\
		(void) setrlimit(RLIMIT_CORE, &rl__);\
	} while (0)
# endif

# define SEED_RAND()\
	srand((unsigned)time(NULL))

# define SET_UP_TEST()\
	do {\
		ARM_ALARM();\
		DISABLE_COREDUMP();\
		SEED_RAND();\
	} while (0)

#endif
