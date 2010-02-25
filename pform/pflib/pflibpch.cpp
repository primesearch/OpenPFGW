/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pflibpch.cpp
 *
 * Description:
 * Fake target for precompiled header
 *======================================================================
 */
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "pflibpch.h"
