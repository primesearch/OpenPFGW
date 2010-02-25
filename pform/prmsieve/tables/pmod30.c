#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#if !defined(_PMOD30_H)
#include "pmod30.h"
#endif

const int pmod30[8] = { 1, 7, 11, 13, 17, 19, 23, 29 };
const int pmod30inv[30] = 
{
    -1,  0, -1, -1, -1, -1, -1,  1, -1, -1, 
    -1,  2, -1,  3, -1, -1, -1,  4, -1,  5, 
    -1, -1, -1,  6, -1, -1, -1, -1, -1,  7  
};
const int gcd30[30] =
{
    30,  1,  2,  3,  2,  5,  6,  1,  2,  3,
    10,  1,  6,  1,  2, 15,  2,  1,  6,  1,
    10,  3,  2,  1,  6,  5,  2,  3,  2,  1
};
