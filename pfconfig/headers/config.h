/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */
/*	PRIMEFORM/GW base configuration file						*/
#define __OPENPFGW__

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* define whether or not to check for suboptimal adds   */
/* #undef GW_ADDOPTIMIZE */

/* define whether or not to check for unsafe multiplies   */
/* #undef GW_MULCHECK */

/* define whether or not to check for unsafe adds    */
/* #undef GW_ADDCHECK */

/* define whether to export the interface as inline functions */
#define GW_INLINE inline

/* define whether to export the interface as inline functions */
#define GW_INLINE_ENABLED 1

/* define whether or not to check for FFT vs non-FFT functions */
/* #undef GW_FFTCHECK */

/* define whether the compiler adds an underscore    */
#define GW_UNDERSCORE

/* Define if you have the strtol function.  */
#define HAVE_STRTOL 1

/* Define if you have the strtoul function.  */
#define HAVE_STRTOUL 1

/* Define if you have the <gmp.h> header file.  */
#define HAVE_GMP_H 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

#include "stdtypes.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifndef DEBUG_NEW
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

