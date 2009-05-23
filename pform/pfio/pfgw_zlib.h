// myzlib.h

#if !defined (__MY_ZLIB_H__)
#define __MY_ZLIB_H__

#include "zlib.h"

bool bLoad_zLibDLL();
void Free_zLibDLL();
bool PFGW_deflate(uint8 *compr, uint8 *uncompr, uint32 *comprLen, uint32 uncomprLen);
bool PFGW_inflate(uint8 *uncompr, uint8 *compr, uint32 comprLen, uint32 *uncomprLen);


#endif // __MY_ZLIB_H__
