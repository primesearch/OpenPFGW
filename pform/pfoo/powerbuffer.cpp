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
