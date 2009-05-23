/*	PRIMEFORM/GW base configuration file						*/
#define __OPENPFGW__
@TOP@

/* define whether or not to check for suboptimal adds			*/
#undef	GW_ADDOPTIMIZE

/* define whether or not to check for unsafe multiplies			*/
#undef	GW_MULCHECK

/* define whether or not to check for unsafe adds				*/
#undef	GW_ADDCHECK

/*	define whether to export the interface as inline functions	*/
#undef	GW_INLINE

/*	define whether to export the interface as inline functions	*/
#undef	GW_INLINE_ENABLED

/*	define whether or not to check for FFT vs non-FFT functions	*/
#undef	GW_FFTCHECK

/*	define whether the compiler adds an underscore				*/
#undef	GW_UNDERSCORE

/*	define whether assembler files should be included			*/
#undef	GW_EMULATEASSEMBLER

#undef	GW_ADD_EFFCPP

#undef	GW_ADD_PEDANTIC

#undef	GW_NASM_WIN32

#undef	GW_WOLTMAN_OBJ

@BOTTOM@
#include "stdtypes.h"
