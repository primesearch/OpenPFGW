// pfoutput.cpp
//
//   Contains global functions PFPrintfStderr(const char *Fmt, ...) and PFPrintfLog(const char *Fmt, ...)
//
//   Also will contain a "class" which will help go between all GUI/console apps.

#include "pfiopch.h"
#include <stdio.h>
#if defined (_MSC_VER)
#include <io.h>  // for dup() and close()
#endif
#include <string.h>

#include "pfoutput.h"

PFOutput *pOutputObj;

bool PFOutput::m_bForcePrint=false;

// Nothing in the base class, just a place holder pure virtual class.
PFOutput::PFOutput()
{
   // put here so I can use bounds checker.  That damn program is great help, but it also has bugs.
   // Moving these from constructor initialzing to here fixed BC's puking/
   m_OutputLogFileName = NULL;
   m_fpOutputLog = NULL;
}

PFOutput::~PFOutput()
{
   CloseLogFile();
}

void PFOutput::InitLogFile(const char *FName, const int terseOutput)
{
   if (!FName)
      return;
   char *cpTmp = new char [strlen(FName)+1];
   strcpy(cpTmp, FName);
   m_bForcePrint = true;
   if (!terseOutput)
      ::PFPrintfStderr("Output logging to file %s\n", cpTmp);
   m_fpOutputLog = fopen(cpTmp, "a+t");
   if (!m_fpOutputLog)
   {
      m_bForcePrint = true;
      ::PFPrintfStderr("Error opening output logging file %s, logging skipped\n", cpTmp);
      delete[] cpTmp;
      cpTmp = 0;
   }
   m_OutputLogFileName = cpTmp;
}

void PFOutput::CloseLogFile()
{
   if (m_fpOutputLog)
      fclose(m_fpOutputLog);
   m_fpOutputLog = 0;
   delete[] m_OutputLogFileName;
   m_OutputLogFileName = 0;
}

int  PFOutput::PFLogPrintf (const char *Fmt, va_list &va)
{
   if (!m_fpOutputLog)
   {
      if (!m_OutputLogFileName)
         return 0;
      // try to reopen the file.  (Note this should only happen for non-VC builds.
      m_fpOutputLog = fopen(m_OutputLogFileName, "a+t");
      if (!m_fpOutputLog)
         return 0;
      fprintf(m_fpOutputLog, "\nWarning, some lines may have been missed due to a a locked %s file\n", m_OutputLogFileName);
   }
   int ret = vfprintf(m_fpOutputLog, Fmt, va);
#if defined (_MSC_VER)
   // use the "close(dup(fileno(fp))) method to make sure the file is flushed fully (only valid for VC)
   fflush(m_fpOutputLog);
   // much faster than fclose() / fopen()
   _close(_dup(_fileno(m_fpOutputLog)));
#else
   // flush the slow way  fclose / fopen
   fclose(m_fpOutputLog);
   m_fpOutputLog = fopen(m_OutputLogFileName, "a+t");
#endif
   return ret;
}

// This writes to stderr, but not to pfgw.log
int PFPrintfStderr(const char *Fmt, ...)
{
   va_list va;
   va_start(va, Fmt);
   int ret = pOutputObj->PFPrintfStderr(Fmt, va);
   va_end(va);
   return ret;
}

// This writes to stdout, but not to pfgw.log
int PFPrintf(const char *Fmt, ...)
{
   va_list va;
   va_start(va, Fmt);
   int ret = pOutputObj->PFPrintf(Fmt, va);
   va_end(va);
   return ret;
}

// This writes to stdout and to pfgw.log
int PFPrintfLog(const char *Fmt, ...)
{
   va_list va;
   va_start(va, Fmt);
   int ret = pOutputObj->PFPrintf(Fmt, va);
   pOutputObj->PFLogPrintf(Fmt, va);
   va_end(va);
   return ret;
}

void PFPrintfClearCurLine(int line_len)
{
   // this is a "no-op" for the GUI version, but does a fprintf(stderr, "\r%79.79s\r", " ") for the console mode
   if (line_len)
      pOutputObj->PFPrintfClearCurLine(line_len);
}

int PFfflush(FILE *f)
{
   // this is a "no-op" for the GUI version, but does a fflush(f) for the console mode
   return pOutputObj->PFfflush(f);
}

void  PFWriteErrorToLog(const char *expr, const char *msg1, const char *msg2, const char *msg3, const char *msg4)
{
   FILE *out = fopen("pfgw_err.log", "a");

   if (out)
   {
      time_t t = time(NULL);
      fprintf(out, "-----------------------------------------------------------------------\n");
      fprintf(out, "Error occuring in PFGW at %s", ctime(&t));
      fprintf(out, "Expr = %s\n", expr);
      fprintf(out, "%s\n", msg1);
      fprintf(out, "%s\n", msg2);
      if (msg3 && *msg3) fprintf(out, "%s\n", msg3);
      if (msg4 && *msg4) fprintf(out, "%s\n", msg4);

      fclose(out);
   }
}
