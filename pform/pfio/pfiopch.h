/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pfiopch.h
 *
 * Description:
 * Precompiled headers
 *======================================================================
 */
#if !defined (_WIN_COPY_ONLY_)

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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pflib.h"
#include "pfmath.h"
#include "pfgwlib.h"
#include "pfglue.h"
#include "pfoo.h"

#include "pfini.h"

#endif
