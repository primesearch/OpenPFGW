//  PFFile class for file handling
#include "pfiopch.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// also includes pffile.h but defines PFNewPGenFile class which is needed for the openInputFile() function.
#include "pfnewpgenfile.h"
#include "pfabcdfile.h"
#include "pfabc2file.h"
#include "pfcpapfile.h"
#include "pfscriptfile.h"
#include "pfdecimalfile.h"
#include "pfcheckfile.h"
#include "pfprzfile.h"

extern bool g_bTerseOutput;
extern bool g_ShowTestResult;

#define MAX_INPUT_LINE_LENGTH 5000100

PFDecimalFile::PFDecimalFile(const char* FileName)
   : PFSimpleFile(FileName)
{
   m_nDecimalLine = new char[MAX_INPUT_LINE_LENGTH];
   m_nNextLine = new char[MAX_INPUT_LINE_LENGTH];
}

PFDecimalFile::~PFDecimalFile()
{
   delete [] m_nDecimalLine;
   delete [] m_nNextLine;
}

void PFDecimalFile::LoadFirstLine()
{
   char *pos;

   if (!g_bTerseOutput)
      PFPrintfLog("Recognized DECIMAL Sieve file: \n");

   if (ReadLine(m_nNextLine, MAX_INPUT_LINE_LENGTH))
   {
      fclose(m_fpInputFile);
      throw "Not a valid DECIMAL sieve file.  EOF reached";
   }

   pos = strchr(m_nNextLine, ' ');
   if (!pos)
   {
      fclose(m_fpInputFile);
      throw "Not a valid DECIMAL sieve file.  Bad format";
   }

   while (*pos == ' ')
      pos++;

   if (!isdigit(*pos))
   {
      fclose(m_fpInputFile);
      throw "Not a valid DECIMAL sieve file.  No value";
   }

   strcpy(m_nDecimalLine, pos);

   // don't count this "first" line, we have to "reset" to line 0 numbering.
   m_nCurrentLineNum = 1;
}

int PFDecimalFile::GetNextLine(PFString &sLine, Integer *, bool *b, PFSymbolTable *)
{
   char *pos;
   int  lineLength = 0;
   int  length = 0;

   if (b)
      *b = false;       // this simple file class does not "remember" or "fill in" the Integer value, ever

   sLine = "";
   m_sCurrentExpression = "";

   if (m_bEOF)
      return e_eof;

   while (lineLength == 0)
   {
      if (feof(m_fpInputFile))
      {
         if (m_pIni)
            m_pIni->SetFileProcessing(false);

         m_bEOF = true;
         m_sCurrentExpression = "";
         return e_eof;
      }

      ReadLine(m_nNextLine, MAX_INPUT_LINE_LENGTH);

      if (!isdigit(*m_nNextLine))
         return 0;

      m_nCurrentPhysicalLineNum++;

      lineLength = (int) strlen(m_nNextLine);

      while (lineLength && (m_nNextLine[lineLength-1] == '\n' || m_nNextLine[lineLength-1] == '\r') )
         m_nNextLine[--lineLength]  = 0;

      // Eat any comment.
      char *cp = strstr(m_nNextLine, "//");
      if (cp)
         *cp = 0;

      lineLength = (int) strlen(m_nNextLine);

      // right trim the line
      while (lineLength && (m_nNextLine[lineLength-1] == ' ' || m_nNextLine[lineLength-1] == '\t') )
         m_nNextLine[--lineLength]  = 0;
   }

   // theLine specifies either the length of the substring of the decimal
   // from the first line or specifies a divisor for one of them.
   pos = strstr(m_nNextLine, " % ");

   if ((pos && sscanf(m_nNextLine, "(%d)", &length) != 1) ||
       (!pos && sscanf(m_nNextLine, "%d", &length) != 1))
   {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Invalid format on line %d", m_nCurrentLineNum);
      return e_bad_file;
   }

   if (strlen(m_nDecimalLine) < length)
   {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Length is beyond length of decimal on line %d", m_nCurrentLineNum);
      return e_bad_file;
   }

   memcpy(m_nCurrentLine, m_nDecimalLine, length);
   m_nCurrentLine[length] = 0;

   if (pos)
      strcat(m_nCurrentLine, pos);

   sLine = m_nCurrentLine;

   m_sCurrentExpression = sLine;

   m_nCurrentLineNum++;

   if (m_pIni)
      m_pIni->SetFileLineNum(m_nCurrentLineNum);

   return e_ok;
}

int PFDecimalFile::SeekToLine(int LineNumber)
{
   if (LineNumber < m_nCurrentLineNum)
   {
      m_nCurrentLineNum = 1;
      m_nCurrentPhysicalLineNum = 1;
      fseek(m_fpInputFile, 0, SEEK_SET);
      if (m_pIni)
         m_pIni->SetFileProcessing(true);
      m_bEOF = false;
   }
   PFIni *p = m_pIni;
   m_pIni = 0;
   int ret = e_ok;
   PFString sTmpLine;
   while (m_nCurrentLineNum < LineNumber)
   {
      if (GetNextLine(sTmpLine) == e_eof)
      {
         ret = e_eof;
         m_bEOF = true;
         break;
      }
   }
   if (p)
   {
      m_pIni = p;
      m_pIni->SetFileLineNum(m_nCurrentLineNum);
   }
   return ret;
}
