/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pfiopch.h
 *
 * Description:
 * Precompiled headers
 *======================================================================
 */

typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef char TCHAR;

typedef void* LPVOID;

#ifndef PFLIBPCH_H

typedef BYTE* LPBYTE;
typedef const BYTE* LPCBYTE;

typedef TCHAR *LPTSTR;
typedef const TCHAR *LPCTSTR;

#include "config.h"
#include "pfoutput.h"
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

#include "pflib.h"
#include "pfmath.h"
#include "pfgwlib.h"
#include "pfoo.h"

#include "pfini.h"

#ifndef INT_MAX
#define INT_MAX INT32_MAX
#define UINT_MAX UINT32_MAX
#define LLONG_MAX INT64_MAX
#define ULLONG_MAX UINT64_MAX
#endif

