#if !defined _MATH16_H
#define _MATH16_H

#include "../types/uint32.h"

uint32 sqmod16(uint32 x, uint32 p);
uint32 mulmod16(uint32 x, uint32 y, uint32 p);
uint32 expmod16(uint32 x, uint32 n, uint32 p);

#endif
