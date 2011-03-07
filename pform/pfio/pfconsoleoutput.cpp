// PFConsoleOutput.cpp
//
//   Contains global functions PFPrintfStderr(const char *Fmt, ...) and PFPrintfLog(const char *Fmt, ...)
//
//   Also will contain a "class" which will help go between all GUI/console apps.

#include "pfiopch.h"
#include <stdio.h>
#include <string.h>

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
   char   buffer[2000], *cpp, cp;
   int x = vsprintf(buffer, Fmt, va);
   bool bShowStr=true;

   if (g_eConsoleMode != eVerbose)
   {
      // I know this code should be in smarteditfield class, but it is here for now.
      cpp = &buffer[strlen(buffer)-1];
      if (*cpp == '\n')
      {
         if (g_eConsoleMode == eQuiet)
         {
            *cpp = '\r';
            bShowStr = false;
         }
         else if (strstr(buffer, "composite") || strstr(buffer, "factor"))
         {
            *cpp = '\r';
            bShowStr = false;
         }
         else if ((g_eConsoleMode==eGFFactors && GF_b_DoGFFactors) && strstr(buffer, "-PRP!"))
         {
            *cpp = '\r';
            bShowStr = false;
         }
         if (g_ShowTestResult)
         {
            if (!strstr(buffer, "composite") || !strstr(buffer, "factor") || !strstr(buffer, "-PRP!") || !strstr(buffer, "prime"))
               bShowStr = true;
         }
      }
   }

   // Always update output every 2 seconds
   if (printTime < time(NULL))
   {
      cpp = &buffer[strlen(buffer)-1];

      if (*cpp == '\r' || *cpp == '\n')
      {
         cp = *cpp;
         *cpp = 0;
         printf(buffer);
         printf("                                    %c", cp);
      }
      else
         printf(buffer);

      fflush(stdout);
      printTime = time(NULL) + 2;
   }
   else if (bShowStr)
   {
      cpp = &buffer[strlen(buffer)-1];

      if (*cpp == '\r' || *cpp == '\n')
      {
         cp = *cpp;
         *cpp = 0;
         printf(buffer);
         printf("                                    %c", cp);
      }
      else
         printf(buffer);

      fflush(stdout);
   }

   return x;
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

