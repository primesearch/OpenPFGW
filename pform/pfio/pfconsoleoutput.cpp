// PFConsoleOutput.cpp
//
//   Contains global functions PFPrintfStderr(const char *Fmt, ...) and PFPrintfLog(const char *Fmt, ...)
//
//   Also will contain a "class" which will help go between all GUI/console apps.

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "pfiopch.h"

extern eOutputMode g_eConsoleMode;
extern bool GF_b_DoGFFactors;
extern bool g_ShowTestResult;

PFConsoleOutput::PFConsoleOutput() : PFOutput()
{
}

PFConsoleOutput::~PFConsoleOutput()
{
}

int PFConsoleOutput::PFPrintfStderr(const char *Fmt, va_list &va)
{
   int x = vfprintf(stderr, Fmt, va);
   fflush(stderr);

   return x;
}

int PFConsoleOutput::PFPrintf(const char *Fmt, va_list &va)
{
   static time_t printTime = 0;
   char   *cpp, cp;
   int    ret;
   bool   bShowStr=true;

   // If the buffer isn't large enough, re-allocate the buffer and return -1
   ret = vsnprintf(m_pBuffer, m_iBufferSize, Fmt, va);
   if (ret == -1 || ret > m_iBufferSize)
   {
      delete[] m_pBuffer;
      if (ret == -1)
         m_iBufferSize *= 2;
      else
         m_iBufferSize = ret + 100;
      m_pBuffer = new char[m_iBufferSize];
      return -1;
   }

   if (g_eConsoleMode != eVerbose)
   {
      // I know this code should be in smarteditfield class, but it is here for now.
      cpp = &m_pBuffer[strlen(m_pBuffer)-1];
      if (*cpp == '\n')
      {
         if (g_eConsoleMode == eQuiet)
         {
            *cpp = '\r';
            bShowStr = false;
         }
         else if (strstr(m_pBuffer, "composite") || strstr(m_pBuffer, "factor"))
         {
            *cpp = '\r';
            bShowStr = false;
         }
         else if ((g_eConsoleMode==eGFFactors && GF_b_DoGFFactors) && strstr(m_pBuffer, "-PRP!"))
         {
            *cpp = '\r';
            bShowStr = false;
         }
         if (g_ShowTestResult)
         {
            if (!strstr(m_pBuffer, "composite") || !strstr(m_pBuffer, "factor") || !strstr(m_pBuffer, "-PRP!") || !strstr(m_pBuffer, "prime"))
               bShowStr = true;
         }
      }
   }

   // Always update output every 2 seconds
   if (printTime < time(NULL))
   {
      cpp = &m_pBuffer[strlen(m_pBuffer)-1];

      if (*cpp == '\r' || *cpp == '\n')
      {
         cp = *cpp;
         *cpp = 0;
         printf("%s", m_pBuffer);
         printf("                                    %c", cp);
      }
      else
         printf("%s", m_pBuffer);

      fflush(stdout);
      printTime = time(NULL) + 2;
   }
   else if (bShowStr)
   {
      cpp = &m_pBuffer[strlen(m_pBuffer)-1];

      if (*cpp == '\r' || *cpp == '\n')
      {
         cp = *cpp;
         *cpp = 0;
         printf("%s", m_pBuffer);
         printf("                                    %c", cp);
      }
      else
         printf("%s", m_pBuffer);

      fflush(stdout);
   }

   return ret;
}

void PFConsoleOutput::PFPrintfClearCurLine(int line_len)
{
   if (line_len < 1)
      line_len = 1;
   else if (line_len > 79)
      line_len = 79;
   fprintf(stderr, "\r%*.*s\r", line_len, line_len, " ");
}

int PFConsoleOutput::PFfflush(FILE *f)
{
   return fflush(f);
}

