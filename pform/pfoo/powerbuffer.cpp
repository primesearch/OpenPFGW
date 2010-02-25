#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "pfoopch.h"
#include "powerbuffer.h"

#ifdef USE_GALLOT
PowerBuffer::PowerBuffer() : BufferReciprocal(Integer(1))
{
}
#else
PowerBuffer::PowerBuffer()
{
}
#endif

PowerBuffer::~PowerBuffer()
{
}
