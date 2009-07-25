// PFWin32GUIOutput.cpp
//
//   Contains global functions PFPrintfStderr(const char *Fmt, ...) and PFPrintf(const char *Fmt, ...)
//
//   Also will contain a "class" which will help go between all GUI/console apps.



#include "pfiopch.h"

// Skip this whole file, unless building with VC.

#if defined (_MSC_VER)

#include <stdio.h>
#include <string.h>

#include "pfoutput.h"
#include "windows.h"
#include "..\winpfgw\winbloz_msg.h"

char g_cpTrayMsg[200] = "\0";

PFWin32GUIOutput::PFWin32GUIOutput(int hWnd) : PFOutput(), m_hWnd(hWnd)
{
}

PFWin32GUIOutput::~PFWin32GUIOutput()
{
}

extern bool g_bWinPFGW_Verbose;
static DWORD dwLast;

int PFWin32GUIOutput::PFPrintfStderr(const char *Fmt, const va_list &va)
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
   char *Buffer = new char [BufLen];
   ret = _vsnprintf(Buffer, BufLen, Fmt, va);
   while (ret == -1)
   {
      delete[] Buffer;
      BufLen *= 2;
      Buffer = new char[BufLen];
      ret = _vsnprintf(Buffer, BufLen, Fmt, va);
   }

   if (m_bErrorPrint)
   {
      m_bErrorPrint = false;  // clear so the next print does not print this.
      FILE *out = fopen("pfgw_err.log", "a");
      if (out)
      {
         time_t t = time(NULL);
         fprintf(out, "-----------------------------------------------------------------------\n");
         fprintf(out, "Error occuring in PFGW at %s", ctime(&t));
         fprintf(out, "Expr = %s\n", (const char*)m_ErrExpr);
         fprintf(out, "Failed at bit %d of %d\n", m_BitsDone, m_BitsTotal);
         if (m_ErrMsg)
            fprintf(out, "Msg = %s\n", (const char*)m_ErrMsg);
         vfprintf(out, Fmt, va);
         fprintf(out, "\n");
         fclose(out);
      }
   }

   if (strlen(Buffer) < 150 && memcmp(Buffer, "PFGW", 4))
   {
      sprintf(g_cpTrayMsg, "WinPFGW: %s", Buffer);
      ret = strlen(g_cpTrayMsg) - 1;
      while (ret && (g_cpTrayMsg[ret] == '\n' || g_cpTrayMsg[ret] == '\r'))
      {
         g_cpTrayMsg[ret] = 0;
         ret--;
      }
   }
   else
      strcpy(g_cpTrayMsg, "WinPFGW (Running)");

   if (!PostMessage((HWND)m_hWnd, WinPFGW_MSG, M_STDERR, (LPARAM)Buffer))
      delete[] Buffer;
   return ret;
}

int PFWin32GUIOutput::PFPrintf(const char *Fmt, const va_list &va)
{
   DWORD Cur = GetTickCount();
   DWORD Diff = Cur-dwLast;

   // Note that WinPFGW must delete[] this item.
   int BufLen = 2048;
   int ret;

   char *Buffer = new char [BufLen];
   ret = _vsnprintf(Buffer, BufLen, Fmt, va);
   while (ret == -1)
   {
      delete[] Buffer;
      BufLen *= 2;
      Buffer = new char[BufLen];
      ret = _vsnprintf(Buffer, BufLen, Fmt, va);
   }

   if (Diff < 1000 && !g_bWinPFGW_Verbose)
   {
      if (strstr(Buffer, "composite") || strstr(Buffer, "factor"))
      {
         delete[] Buffer; // we have to delete it here.
         return ret;
      }
   }
   dwLast = Cur;

   if (!PostMessage((HWND)m_hWnd, WinPFGW_MSG, M_PRINTF, (LPARAM)Buffer))
      delete[] Buffer;
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