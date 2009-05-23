/* Bit twiddly things useful in many places */
#include "../types/uint32.h"

/* count bits in a byte */
extern const unsigned long pop[256];

/* the powers of two */
extern const uint32 two[32];
#if !defined(__i386__)
# define BIT32(i) (1u<<((i)&31))
#else
# define BIT32(i) two[(i)&31]
#endif

