
/* define whether to export the interface as inline functions */
#define GW_INLINE inline

/* define whether to export the interface as inline functions */
#define GW_INLINE_ENABLED 1

/* define whether the compiler adds an underscore    */
#define GW_UNDERSCORE

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifndef DEBUG_NEW
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

