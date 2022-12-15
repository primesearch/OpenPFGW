// PFWin32GUIOutput.cpp
//
//   Contains global functions PFPrintfStderr(const char *Fmt, ...) and PFPrintfLog(const char *Fmt, ...)
//
//   Also will contain a "class" which will help go between all GUI/console apps.

// Skip this whole file, unless building with VC.

#if defined (_MSC_VER)

#include <stdio.h>
#include <string.h>

#include "pfiopch.h"
#include "pfoutput.h"
#include "windows.h"
#include "..\winpfgw\winbloz_msg.h"

char g_cpTrayMsg[200] = "\0";

PFWin32GUIOutput::PFWin32GUIOutput(uint64_t hWnd) : PFOutput(), m_hWnd(hWnd)
{
}

PFWin32GUIOutput::~PFWin32GUIOutput()
{
}

extern bool g_bWinPFGW_Verbose;
static DWORD dwLast;

int PFWin32GUIOutput::PFPrintfStderr(const char *Fmt, va_list &va)
{
   DWORD Cur = GetTickCount();
   DWORD Diff = Cur-dwLast;

   if (Diff < 1000 && !m_bForcePrint)
      return 0;
   m_bForcePrint = false;
   dwLast = Cur;

   // Note that WinPFGW must delete[] this item.
   int BufLen = 2048;
   int ret;
   char *pBuffer = new char [BufLen];

   ret = _vsnprintf(pBuffer, BufLen, Fmt, va);
   while (ret == -1 || ret > BufLen)
   {
      delete [] pBuffer;
      if (ret == -1)
         BufLen *= 2;
      else
         BufLen = ret + 100;
      pBuffer = new char[BufLen];
      ret = _vsnprintf(pBuffer, BufLen, Fmt, va);
   }

   if (strlen(pBuffer) < 150 && memcmp(pBuffer, "PFGW", 4))
   {
      snprintf(g_cpTrayMsg, sizeof(g_cpTrayMsg), "WinPFGW: %s", pBuffer);
      ret = (int) strlen(g_cpTrayMsg) - 1;
      while (ret && (g_cpTrayMsg[ret] == '\n' || g_cpTrayMsg[ret] == '\r'))
      {
         g_cpTrayMsg[ret] = 0;
         ret--;
      }
   }
   else
      strcpy(g_cpTrayMsg, "WinPFGW (Running)");

   if (!PostMessage((HWND)m_hWnd, WinPFGW_MSG, M_STDERR, (LPARAM)pBuffer))
      delete [] pBuffer;
   return ret;
}

int PFWin32GUIOutput::PFPrintf(const char *Fmt, va_list &va)
{
   int BufLen = 2048;
   int ret;
   char *pBuffer = new char [BufLen];

   ret = _vsnprintf(pBuffer, BufLen, Fmt, va);
   while (ret == -1 || ret > BufLen)
   {
      delete[] pBuffer;
      if (ret == -1)
         BufLen *= 2;
      else
         BufLen = ret + 100;
      pBuffer = new char[BufLen];
      ret = _vsnprintf(pBuffer, BufLen, Fmt, va);
   }

   if (!PostMessage((HWND)m_hWnd, WinPFGW_MSG, M_PRINTF, (LPARAM)pBuffer))
      delete [] pBuffer;
   return ret;
}


void PFWin32GUIOutput::PFPrintfClearCurLine(int /*line_len*/)
{
   // no-op
}

int PFWin32GUIOutput::PFfflush(FILE * /*f*/)
{
   // no-op
   return 0;
}

#endif   // #if defined (_MSC_VER)
