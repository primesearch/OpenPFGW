/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pflibpch.h
 *
 * Description:
 * Precompiled headers
 *======================================================================
 */
#ifndef PFLIBPCH_H
#define PFLIBPCH_H

#if !defined (_WIN_COPY_ONLY_)
#include "config.h"
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long DWORD;
typedef unsigned char BYTE;

typedef void* LPVOID;
typedef BYTE* LPBYTE;
typedef const BYTE* LPCBYTE;

typedef char TCHAR;

typedef TCHAR *LPTSTR;
typedef const TCHAR *LPCTSTR;
typedef const char *LPCSTR;

#endif
