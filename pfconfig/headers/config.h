/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */
/*	PRIMEFORM/GW base configuration file						*/
#define __OPENPFGW__

/* Define if using alloca.c.  */
/* #undef C_ALLOCA */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
/* #undef CRAY_STACKSEG_END */

/* Define if you have alloca, as a function or macro.  */
/*	#define HAVE_ALLOCA 1	*/

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
/*	#define HAVE_ALLOCA_H 1	*/

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
 STACK_DIRECTION > 0 => grows toward higher addresses
 STACK_DIRECTION < 0 => grows toward lower addresses
 STACK_DIRECTION = 0 => direction of growth unknown
 */
/* #undef STACK_DIRECTION */

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

/* define whether assembler files should be included   */
/* #undef GW_EMULATEASSEMBLER */

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

/* Define if you have the <unistd.h> header file.  */
/*      #define HAVE_UNISTD_H 1 */

/* Define if you have the e library (-le).  */
/* #undef HAVE_LIBE */

/* Define if you have the inks library (-links).  */
/* #undef HAVE_LIBINKS */

/* Define if you have the m library (-lm).  */
/* #undef HAVE_LIBM */
#if !defined (_WIN_COPY_ONLY_)
#include "stdtypes.h"
#endif
