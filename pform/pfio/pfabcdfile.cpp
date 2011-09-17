#include "pfiopch.h"
#include <string.h>
#include "pfabcdfile.h"
#include "stdtypes.h"

extern bool g_bVerbose;
extern bool g_bTerseOutput;

PFABCDFile::PFABCDFile(const char* FileName)
   : PFABCFile(FileName)
{
   for (uint32 i = 0; i < 26; i++)
      m_i64Accum[i] = 0;
   m_nAccum = 0;
   m_bReadNextLineFromFile = false;
   m_bIgnoreOutput = false;
   m_SigString = "ABCD File";
}

char s_Line[ABCLINELEN];

void PFABCDFile::CutOutFirstLine()
{
   // Ok, cut our our [line2] stuff, rebuild the line, and let ABCFile::ProccessFirstLine have at it.
   char Line2[ABCLINELEN];
   char *cp = strchr(s_Line, '[');
   if (!cp)
      throw "Error, Not a valid ABCD Sieve file, Can't find a [ char in the first line";
   strcpy(Line2, cp);
   char *cp1 = strchr(Line2, ']');
   if (!cp1)
      throw "Error, Not a valid ABCD Sieve file, Can't find a ] char in the first line";
   *cp1++ = 0;

   // Now put the end of the line back on the original first line (over write the [...] stuff)
   strcpy(cp, cp1);

   // eat the ABCD down to a ABC
   memmove (&s_Line[3], &s_Line[4], strlen(&s_Line[4]));

   // We already have the first line, so don't hit the file again until we pass over this data.
   m_bReadNextLineFromFile = false;

   cp = &Line2[1];
   while (*cp == ' ' || *cp == '\t')
      ++cp;

   strcpy(m_ABCLookingLine, cp);

   m_i64Accum[0] = _atoi64(cp);

   if (m_i64Accum[0] == 0 && *cp != '0')
      throw("Error, Not a valid ABCD Sieve file, argument 1 in [] format not valid");
   cp = strchr(cp, ' ');

   m_nAccum = 1;
   while (cp)
   {
      ++cp;
      m_i64Accum[m_nAccum] = _atoi64(cp);
      if (!m_i64Accum[m_nAccum]  && *cp != '0')
      {
         char Msg[120];
         sprintf (Msg, "Error, Not a valid ABCD Sieve file, argument %d in [] format not valid", m_nAccum+1);
         throw(Msg);
      }
      ++m_nAccum;
      cp = strchr(cp, ' ');
   }

   // Ready to roll.
}

void PFABCDFile::LoadFirstLine()
{
   if (!g_bTerseOutput)
      PFPrintfLog("Recognized ABCD Sieve file: \n");

   if (PFSimpleFile::ReadLine(s_Line, sizeof(s_Line)))
   {
      fclose(m_fpInputFile);
      throw "Error, Not a valid ABCD Sieve file";
   }
   s_Line[sizeof(s_Line)-1] = 0;

   CutOutFirstLine();

   // do count this "first" line, we have to "reset" to line 0 numbering.
   m_nCurrentLineNum = 0;
   m_nCurrentPhysicalLineNum = 0;

   // Now use PFABCFile to process the line.
   PFABCFile::ProcessFirstLine(s_Line);

   // don't count this "first" line, we have to "reset" to line 0 numbering.
   m_nCurrentLineNum = 1;
}

PFABCDFile::~PFABCDFile()
{
   // Nothing to do.
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
   char TmpLine[ABCLINELEN];
   if (PFSimpleFile::ReadLine(TmpLine, sizeof(TmpLine)))
   {
      fclose(m_fpInputFile);
      return true;
   }

   char *cp = TmpLine;
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
#if !defined (_MSC_VER)
   if (!strncmp(cp, "ABCD ", 5))
#else
   // NON PORTIBLE WARNING.  This will ONLY work on 32 bit little-endian systems (Intel)
   if ('DCBA' == *(unsigned long*)cp)
#endif
   {
      // Don't inc line count here.
//    ++m_nCurrentPhysicalLineNum;
//    ++m_nCurrentLineNum;
      if (g_bVerbose)
      {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("A new ABCD signature seen on line %d, switching to use this new format\n", m_nCurrentLineNum);
      }

      // Now process this as though it was a "first" line.  This will reinitialize the PFABC stuff to this new value
      try
      {
         strcpy(s_Line, cp);
         CutOutFirstLine();
         PFABCFile::ProcessFirstLine(s_Line);
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

   int64 Val = _atoi64(cp);
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
         sprintf (_Line, ULL_FORMAT"\n", m_i64Accum[0]);
   }
   else
   {
      cp = strchr(cp, ' ');
      uint32 nArgs = 1;

      while (cp)
      {
         ++cp;
         Val = _atoi64(cp);
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
            cp = TmpLine;
            while (*cp == ' ' || *cp == '\t')
               ++cp;

            m_i64Accum[0] -= _atoi64(cp);
            nArgs = 1;
            cp = strchr(cp, ' ');
            while (cp)
            {
               ++cp;
               Val = _atoi64(cp);
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
         cp = TmpLine;
         while (*cp == ' ' || *cp == '\t')
            ++cp;
         m_i64Accum[0] -= _atoi64(cp);
         nArgs = 1;
         cp = strchr(cp, ' ');
         while (cp && nArgs < m_nAccum)
         {
            ++cp;
            Val = _atoi64(cp);
            m_i64Accum[nArgs] -= Val;
            ++nArgs;
            cp = strchr(cp, ' ');
         }
         goto ReadLineAgain;
      }
      if (!m_bIgnoreOutput)
      {
         cp = m_ABCLookingLine;
         cp += sprintf (cp, ULL_FORMAT, m_i64Accum[0]);
         for (nArgs = 1; nArgs < m_nAccum; ++nArgs)
            cp += sprintf (cp, " "ULL_FORMAT, m_i64Accum[nArgs]);

         // Not sure the \n is needed, but it does not hurt
         sprintf (cp, "\n");
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
      PFSimpleFile::ReadLine(s_Line, ABCLINELEN);
      CutOutFirstLine();
      m_nCurrentPhysicalLineNum = 0;
      m_nCurrentLineNum = 0;
      PFABCFile::ProcessFirstLine(s_Line);
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
      if (ReadLine(Line,sizeof(Line))) {
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
