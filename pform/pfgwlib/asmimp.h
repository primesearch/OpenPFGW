#if !defined (__ASMIMP_H__)
#define __ASMIMP_H__
#include "asmexp.h"
// variables imported by the assembler routines. If the compiler
// doesn't add an underscore, we need to add one ourselves

#ifndef GW_UNDERSCORE

#define	RadixConfigure	_RadixConfigure
#define	DontNormalize	_DontNormalize

#endif


#endif
