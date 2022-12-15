#include <stdio.h>
#include <string.h>
#include "pfiopch.h"
#include "pfabcdfile.h"

extern bool g_bVerbose;
extern bool g_bTerseOutput;

PFABCDFile::PFABCDFile(const char* FileName)
   : PFABCFile(FileName), m_Line1(0), m_TempLine(0)
{
   for (uint32_t i = 0; i < 26; i++)
      m_i64Accum[i] = 0;
   m_nAccum = 0;
   m_bReadNextLineFromFile = false;
   m_bIgnoreOutput = false;
   m_SigString = "ABCD File";
   m_Line1 = new char[ABCLINELEN];
   m_TempLine = new char[ABCLINELEN];
   m_ABCLookingLine = new char[ABCLINELEN];
}

void PFABCDFile::CutOutFirstLine()
{
   char *temp = new char[ABCLINELEN];

   // Ok, cut our our [line2] stuff, rebuild the line, and let ABCFile::ProccessFirstLine have at it.
   char *cp = strchr(m_Line1, '[');
   if (!cp)
      throw "Error, Not a valid ABCD Sieve file, Can't find a [ char in the first line";
   strcpy(temp, cp);
   char *cp1 = strchr(temp, ']');
   if (!cp1)
      throw "Error, Not a valid ABCD Sieve file, Can't find a ] char in the first line";
   *cp1++ = 0;

   // Now put the end of the line back on the original first line (over write the [...] stuff)
   strcpy(cp, cp1);

   // eat the ABCD down to a ABC
   memmove(&m_Line1[3], &m_Line1[4], strlen(&m_Line1[4]));

   // We already have the first line, so don't hit the file again until we pass over this data.
   m_bReadNextLineFromFile = false;

   cp = &temp[1];
   while (*cp == ' ' || *cp == '\t')
      ++cp;

   strcpy(m_ABCLookingLine, cp);

   sscanf(cp, "%" SCNu64"", &m_i64Accum[0]);

   if (m_i64Accum[0] == 0 && *cp != '0')
      throw("Error, Not a valid ABCD Sieve file, argument 1 in [] format not valid");
   cp = strchr(cp, ' ');

   m_nAccum = 1;
   while (cp)
   {
      ++cp;
      sscanf(cp, "%" SCNu64"", &m_i64Accum[m_nAccum]);
      if (!m_i64Accum[m_nAccum]  && *cp != '0')
      {
         char Msg[120];
         snprintf (Msg, sizeof(Msg), "Error, Not a valid ABCD Sieve file, argument %d in [] format not valid", m_nAccum + 1);
         throw(Msg);
      }
      ++m_nAccum;
      cp = strchr(cp, ' ');
   }

   // Ready to roll.
   delete [] temp;
}

void PFABCDFile::LoadFirstLine()
{
   if (!g_bTerseOutput)
      PFPrintfLog("Recognized ABCD Sieve file: \n");

   if (PFSimpleFile::ReadLine(m_Line1, ABCLINELEN))
   {
      fclose(m_fpInputFile);
      throw "Error, Not a valid ABCD Sieve file";
   }

   CutOutFirstLine();

   // do count this "first" line, we have to "reset" to line 0 numbering.
   m_nCurrentLineNum = 0;
   m_nCurrentPhysicalLineNum = 0;

   // Now use PFABCFile to process the line.
   ProcessFirstLine(m_Line1);

   // don't count this "first" line, we have to "reset" to line 0 numbering.
   m_nCurrentLineNum = 1;
}

PFABCDFile::~PFABCDFile()
{
   delete [] m_Line1;
   delete [] m_TempLine;
}

int PFABCDFile::ReadLine(char *_Line, int sizeofLine)
{
ReadLineAgain_0:;
   if (!m_bReadNextLineFromFile)
   {
      // the first line of data is actually embedding in the first line (not in the second line like most other sieve formats).
      // So we should NOT hit the file until after we have processed this stored information.
      m_bReadNextLineFromFile = true;  // Next time, we read from the file
      strncpy(_Line, m_ABCLookingLine, sizeofLine);
      _Line[sizeofLine-1] = 0;
      return feof(m_fpInputFile);
   }
   // Load new line, compute delta, make a "fake" line, and return it.

ReadLineAgain:;

   if (PFSimpleFile::ReadLine(m_TempLine, ABCLINELEN))
   {
      fclose(m_fpInputFile);
      return true;
   }

   char *cp = m_TempLine;
   while (*cp == ' ' || *cp == '\t')
      ++cp;

   // Handle blank lines here.
   //if (!strcmp(cp, "\n"))
   if (*cp == '\n')
   {
      ++m_nCurrentPhysicalLineNum;
      ++m_nCurrentLineNum;
      goto ReadLineAgain;
   }

   // Handle ABCD lines here
   if (!strncmp(cp, "ABCD ", 5))
   {
      if (g_bVerbose)
      {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("A new ABCD signature seen on line %d, switching to use this new format\n", m_nCurrentLineNum);
      }

      // Now process this as though it was a "first" line.  This will reinitialize the PFABC stuff to this new value
      try
      {
         strcpy(m_Line1, cp);
         CutOutFirstLine();
         ProcessFirstLine(m_Line1);
      }
      catch(char *s)
      {
         if (g_bVerbose)
         {
            PFOutput::EnableOneLineForceScreenOutput();
            PFPrintfStderr("Warning, [%s] parsing line %d will try to continue\n", s, m_nCurrentLineNum);
         }
      }
      goto ReadLineAgain_0;
   }

   int64_t Val;
   sscanf(cp, "%" SCNu64"", &Val);
   
   if (!Val && *cp != '0')
   {
      if (g_bVerbose)
      {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("WARNING, Invalid line in ABCD file (error in argument 1 line %d)\n", m_nCurrentLineNum);
      }
      ++m_nCurrentPhysicalLineNum;
      ++m_nCurrentLineNum;
      goto ReadLineAgain;
   }
   m_i64Accum[0] += Val;

   // Speedup for the 1 argument case.  We don't do any strchr or other logic.  We just take the first
   // value and run with it.
   if (1 == m_nAccum)
   {
      if (!m_bIgnoreOutput)
         snprintf (_Line, sizeofLine, "%" PRIu64"\n", m_i64Accum[0]);
   }
   else
   {
      cp = strchr(cp, ' ');
      uint32_t nArgs = 1;

      while (cp)
      {
         ++cp;
         sscanf(cp, "%" SCNu64"", &Val);

         if (!Val  && *cp != '0')
         {
            if (g_bVerbose)
            {
               PFOutput::EnableOneLineForceScreenOutput();
               PFPrintfStderr("WARNING, Invalid line in ABCD file (error in argument %d line %d\n)", nArgs+1, m_nCurrentLineNum);
            }
            ++m_nCurrentPhysicalLineNum;
            ++m_nCurrentLineNum;
            // undo what was done
            cp = m_TempLine;
            while (*cp == ' ' || *cp == '\t')
               ++cp;

            sscanf(cp, "%" SCNu64"", &m_i64Accum[0]);
            nArgs = 1;
            cp = strchr(cp, ' ');
            while (cp)
            {
               ++cp;
               sscanf(cp, "%" SCNu64"", &Val);
               if (!Val  && *cp != '0')
                  goto ReadLineAgain;
               m_i64Accum[nArgs] -= Val;
               ++nArgs;
               cp = strchr(cp, ' ');
            }
         }
         m_i64Accum[nArgs] += Val;
         ++nArgs;
         cp = strchr(cp, ' ');
      }
      if (nArgs != m_nAccum)
      {
         if (g_bVerbose)
         {
            PFOutput::EnableOneLineForceScreenOutput();
            PFPrintfStderr("WARNING, Wrong number of arguments in ABCD file (ignoring line %d)\n", m_nCurrentLineNum);
         }
         ++m_nCurrentPhysicalLineNum;
         ++m_nCurrentLineNum;

         // undo what was done
         cp = m_TempLine;
         while (*cp == ' ' || *cp == '\t')
            ++cp;
         sscanf(cp, "%" SCNu64"", &m_i64Accum[0]);
         nArgs = 1;
         cp = strchr(cp, ' ');
         while (cp && nArgs < m_nAccum)
         {
            ++cp;
            sscanf(cp, "%" SCNu64"", &Val);
            m_i64Accum[nArgs] -= Val;
            ++nArgs;
            cp = strchr(cp, ' ');
         }
         goto ReadLineAgain;
      }
      if (!m_bIgnoreOutput)
      {
         cp = m_ABCLookingLine;
         cp += snprintf (cp, ABCLINELEN, "%" PRIu64"", m_i64Accum[0]);
         for (nArgs = 1; nArgs < m_nAccum; ++nArgs)
            cp += snprintf (cp, ABCLINELEN, " %" PRIu64"", m_i64Accum[nArgs]);

         // Not sure the \n is needed, but it does not hurt
         snprintf (cp, ABCLINELEN, "\n");
         strncpy(_Line, m_ABCLookingLine, sizeofLine);
         _Line[sizeofLine-1] = 0;
      }
   }
   ++m_nCurrentPhysicalLineNum;
   return feof(m_fpInputFile);
}

int PFABCDFile::SeekToLine(int LineNumber)
{
   // turn "off" the output of the line into ABC format.  All we need to do is to read the
   // lines, and update all accumulators.  Turning off output speeds up the processing
   // significantly.
   m_bIgnoreOutput = true;

   fseek(m_fpInputFile, 0, SEEK_SET);
   if (m_pIni)
      m_pIni->SetFileProcessing(true);
   m_bEOF = false;
   try
   {
      PFSimpleFile::ReadLine(m_Line1, ABCLINELEN);
      CutOutFirstLine();
      m_nCurrentPhysicalLineNum = 0;
      m_nCurrentLineNum = 0;
      ProcessFirstLine(m_Line1);
   }
   catch(char *s)
   {
      if (g_bVerbose)
      {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("Warning, [%s] parsing line will try to continue\n", s);
      }
      m_bEOF = true;
      m_bIgnoreOutput = false;
      return e_eof;
   }
   while (m_nCurrentLineNum < LineNumber)
   {
      if (ReadLine(m_Line1, ABCLINELEN)) {
         if (m_pIni)
            m_pIni->SetFileProcessing(false);
         m_bEOF = true;
         return e_eof;
      }
      m_nCurrentLineNum++;
   }
   if (m_pIni)
      m_pIni->SetFileLineNum(m_nCurrentLineNum);

   m_bIgnoreOutput = false;
   return e_ok;
}
