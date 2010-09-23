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
