#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#if !defined (_DEBUG)

#ifdef __cplusplus
extern "C" {
#endif

	// there are problems when linking a "static" mfc against MSVCRT.dll.  We link
	// against MSVCRT due to zLib ("official" DLL) and GMP DLL (MinGW build) linking
	// against MSVCRT.  I am not sure why in the debug builds, where we link against
	// MSCRTD.dll, these missing extern vars are not a problem.
	int __argc;
	char **__argv;
	char _mbctype[257*sizeof(unsigned)];

#ifdef __cplusplus
}
#endif

#endif

static int jjjj;	// Shuts up VC's stupid warning about empty translation unit in debug builds.