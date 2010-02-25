#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "pfoopch.h"
#include "recursionbuffer.h"

RecursionBuffer::RecursionBuffer()
   : N(0), depth(0)
{
}

void RecursionBuffer::set(const Integer &X,unsigned long d)
{
   N=X;
   depth=d;
}
