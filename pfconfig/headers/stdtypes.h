#ifndef STDTYPES_H
#define STDTYPES_H

#include <stdio.h>

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#if defined (_MSC_VER)
#define LL_FORMAT "%I64d"
#define ULL_FORMAT "%I64u"
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef signed long long int64;
typedef unsigned long long uint64;
//#define LL_FORMAT "%lld"
//#define ULL_FORMAT "%llu"
// Paul J stated that %qu was more "generic" than %llu, so it is used here.
#define LL_FORMAT "%qd"
#define ULL_FORMAT "%qu"


// Provide compatiblity with VC's _atoi64() function for non-VC compilers.
#if defined (__cplusplus)
inline int64 _atoi64(const char *s)
{
   int64 i;
   sscanf(s, "%lld", &i);
   return i;
}
#endif // __cplusplus
#endif // ! defined (_MSC_VER)

// VC does not have this.
#if defined (__cplusplus)
inline uint64 _atou64(const char *s)
{
   uint64 u;
   sscanf(s, ULL_FORMAT, &u);
   return u;
}
#endif // defined __cplusplus

#if defined (_MSC_VER)
// Remove these VC warning level 4 warnings
//  C4514 unreferenced inline function has been removed
//  C4511 copy constructor could not be generated      (that is what we want, non-copyable objects)
//  C4512 assignment operator could not be generated   (ditto)
//  C4127 conditional expression is constant           (triggered by code like:   while(1) {}
//  C4505 unreferenced local function has been removed (triggered by static functions in bmap.cxx)
//  C4723 potential divide by 0                        (deliberately triggered)
#pragma warning (disable : 4514 4511 4512 4127 4505 4723)
#endif

#endif

