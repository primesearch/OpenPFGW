#include <stdio.h>
#include <string.h>
#include "pfiopch.h"
#include "pfprzfile.h"

extern bool g_bVerbose;
extern bool g_bTerseOutput;

extern bool DeCompress_bits_From_PrZ_setup(uint32_t BitLevel, uint32_t _EscapeLevel, uint64_t _OffsetK, uint32_t _Base, uint32_t _nvalsleft, char *cpp, bool _bSkipEvens, bool _bNewPGenFile);
extern bool DeCompress_bits_From_PrZ(FILE *fpIn, char *cpp, bool bIgnoreOutput);


PFPrZFile::PFPrZFile(const char* FileName)
   : PFABCFile(FileName), m_Line1(0)
{
   m_i64Accum = 0;
   m_bReadNextLineFromFile = false;
   m_bIgnoreOutput = false;
   fclose(m_fpInputFile);
   m_fpInputFile = fopen(FileName, "rb");
   m_SigString = "PrZ_ABCD File";
   m_Line1 = new char[ABCLINELEN];
   m_ABCLookingLine = new char[ABCLINELEN];
}

void PFPrZFile::CutOutFirstLine()
{
   // Ok, cut our our [line2] stuff, rebuild the line, and let ABCFile::ProccessFirstLine have at it.
   char *temp = new char[ABCLINELEN];

   char *cp = strchr(m_Line1, '[');
   if (!cp)
      throw "Error, Not a valid PrZ_ABCD Sieve file, Can't find a [ char in the first line";
   strcpy(temp, cp);
   char *cp1 = strchr(temp, ']');
   if (!cp1)
      throw "Error, Not a valid PrZ_ABCD Sieve file, Can't find a ] char in the first line";
   *cp1++ = 0;

   // Now put the end of the line back on the original first line (over write the [...] stuff)
   strcpy(cp, cp1);

   // eat the ABCD down to a ABC
   memmove (&m_Line1[3], &m_Line1[4], strlen(&m_Line1[4]));

   // We already have the first line, so don't hit the file again until we pass over this data.
   m_bReadNextLineFromFile = false;

   cp = &temp[1];
   while (*cp == ' ' || *cp == '\t')
      ++cp;

   strcpy(m_ABCLookingLine, cp);

   sscanf(cp, "%" SCNu64"", &m_i64Accum);

   if (m_i64Accum == 0 && *cp != '0')
      throw("Error, Not a valid PrZ_ABCD Sieve file, argument 1 in [] format not valid");

   cp = strchr(cp, ' ');

   // Ready to roll.
   delete [] temp;
}

void PFPrZFile::LoadFirstLine()
{
   PrZ_File_Header PrZHead;
   fread(&PrZHead, 1, sizeof(PrZHead), m_fpInputFile);

   PrZ_Section_Header_Base *ipFileHead;
   if (PrZHead.PrZ_IsFermFactABCD)
   {
      if (!g_bTerseOutput)
         PFPrintfLog("Recognized ABCZ (Fermfact) Sieve file: \n");
      ipFileHead = new PrZ_FermFact_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
   }
   else if (PrZHead.PrZ_IsAPSieveABCD)
   {
      if (!g_bTerseOutput)
         PFPrintfLog("Recognized ABCZ (APSieve) Sieve file: \n");
      ipFileHead = new PrZ_APSieve_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
   }
   else if (PrZHead.PrZ_IsNewPGen)
   {
      if (!g_bTerseOutput)
         PFPrintfLog("Recognized ABCZ (NewPGen) Sieve file: \n");
      throw "Error, wrong constructor was called!\n";
   }
   else
   {
      if (!g_bTerseOutput)
         PFPrintfLog("Recognized ABCZ (Generic ABCD) Sieve file: \n");
      ipFileHead = new PrZ_Generic_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
   }

   uint64_t xx;
   if (!ipFileHead->GetValues(m_MinNum, m_MaxNum, prz_nvalsleft, xx))
   {
      m_MaxNum = LLONG_MAX;
      m_MinNum = ipFileHead->KOffset();
   }

   snprintf(m_Line1, ABCLINELEN, "%s", ipFileHead->PrZ_GetFirstLine());
   CutOutFirstLine();

   // do count this "first" line, we have to "reset" to line 0 numbering.
   m_nCurrentLineNum = 0;
   m_nCurrentPhysicalLineNum = 0;

   // Now use PFABCFile to process the line.
   PFABCFile::ProcessFirstLine(m_Line1);

   // don't count this "first" line, we have to "reset" to line 0 numbering.
   m_nCurrentLineNum = 1;

   char *cpp = new char[ABCLINELEN];
   DeCompress_bits_From_PrZ_setup(PrZHead.PrZ_Bits+3, PrZHead.Prz_ESCs, ipFileHead->KOffset(), ipFileHead->getPrZ_Base(), (uint32_t)prz_nvalsleft, cpp, PrZHead.PrZ_SkipEvens, PrZHead.PrZ_IsNewPGen);
   delete ipFileHead;
   delete [] cpp;
}

PFPrZFile::~PFPrZFile()
{
   // Nothing to do.
}

int PFPrZFile::ReadLine(char *lineBuffer, int sizeofLine)
{
   if (!m_bReadNextLineFromFile)
   {
      // the first line of data is actually embedding in the first line (not in the second line like most other sieve formats).
      // So we should NOT hit the file until after we have processed this stored information.
      m_bReadNextLineFromFile = true;  // Next time, we read from the file
      strncpy(lineBuffer, m_ABCLookingLine, sizeofLine);
      lineBuffer[sizeofLine-1] = 0;
      return prz_nvalsleft == 0;
   }
   // Load new line, compute delta, make a "fake" line, and return it.

   if (!DeCompress_bits_From_PrZ(m_fpInputFile, lineBuffer, m_bIgnoreOutput))
   {
      fclose(m_fpInputFile);
      return true;
   }
   ++m_nCurrentPhysicalLineNum;
   return false;
}

int PFPrZFile::SeekToLine(int LineNumber)
{
   // turn "off" the output of the line into ABC format.  All we need to do is to read the
   // lines, and update all accumulators.  Turning off output speeds up the processing
   // significantly.
   m_bIgnoreOutput = true;

   m_i64Accum = 0;
   m_bReadNextLineFromFile = false;

   fseek(m_fpInputFile, 0, SEEK_SET);
   if (m_pIni)
      m_pIni->SetFileProcessing(true);
   m_bEOF = false;
   PrZ_Section_Header_Base *ipFileHead=0;
   try
   {
      PrZ_File_Header PrZHead;
      fread(&PrZHead, 1, sizeof(PrZHead), m_fpInputFile);
      if (PrZHead.PrZ_IsFermFactABCD)
         ipFileHead = new PrZ_FermFact_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
      else if (PrZHead.PrZ_IsAPSieveABCD)
         ipFileHead = new PrZ_APSieve_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
      else if (PrZHead.PrZ_IsNewPGen)
         throw "Error, wrong constructor was called!\n";
      else
         ipFileHead = new PrZ_Generic_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
      uint64_t xx;
      if (!ipFileHead->GetValues(m_MinNum, m_MaxNum, prz_nvalsleft, xx))
      {
         m_MaxNum = LLONG_MAX;
         m_MinNum = ipFileHead->KOffset();
      }
      snprintf(m_Line1, ABCLINELEN, "%s", ipFileHead->PrZ_GetFirstLine());
      CutOutFirstLine();
      m_nCurrentLineNum = 0;
      m_nCurrentPhysicalLineNum = 0;
      PFABCFile::ProcessFirstLine(m_Line1);
      
      char *cpp = new char[ABCLINELEN];
      DeCompress_bits_From_PrZ_setup(PrZHead.PrZ_Bits+3, PrZHead.Prz_ESCs, ipFileHead->KOffset(), ipFileHead->getPrZ_Base(), (uint32_t)prz_nvalsleft, cpp, PrZHead.PrZ_SkipEvens, PrZHead.PrZ_IsNewPGen);
      delete [] cpp;
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
      delete ipFileHead;
      return e_eof;
   }
#if defined (_DEBUG)
   m_bIgnoreOutput = false;
#endif
   while (m_nCurrentLineNum < LineNumber)
   {
      if (ReadLine(m_Line, ABCLINELEN)) {
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
   delete ipFileHead;
   return e_ok;
}


PFPrZ_newpgen_File::PFPrZ_newpgen_File(const char* FileName)
   : PFNewPGenFile(FileName), m_Line1(0)
{
   m_i64Accum = 0;
   m_nAccum = 0;
   m_bReadNextLineFromFile = false;
   m_bIgnoreOutput = false;
   fclose(m_fpInputFile);
   m_fpInputFile = fopen(FileName, "rb");
   m_SigString = "PrZ_NewPGen file: ";
   m_Line1 = new char[ABCLINELEN];
}

void PFPrZ_newpgen_File::LoadFirstLine()
{
   PrZ_File_Header PrZHead;
   fread(&PrZHead, 1, sizeof(PrZHead), m_fpInputFile);

   PrZ_Section_Header_Base *pFileHead;
   if (PrZHead.PrZ_IsFermFactABCD)
      throw "Error, wrong constructor was called!\n";
   else if (PrZHead.PrZ_IsAPSieveABCD)
      throw "Error, wrong constructor was called!\n";
   else if (PrZHead.PrZ_IsNewPGen)
      pFileHead = new PrZ_NewPGen_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
   else
      throw "Error, wrong constructor was called!\n";

   uint64_t xx;
   if (!pFileHead->GetValues(m_MinNum, m_MaxNum, prz_nvalsleft, xx))
   {
      m_MaxNum = LLONG_MAX;
      m_MinNum = pFileHead->KOffset();
   }

   snprintf(m_Line1, ABCLINELEN, "%s", pFileHead->PrZ_GetFirstLine());

   // do count this "first" line, we have to "reset" to line 0 numbering.
   m_nCurrentLineNum = 0;
   m_nCurrentPhysicalLineNum = 0;

   // Now use PFABCFile to process the line.
   PFNewPGenFile::ProcessFirstLine(m_Line1, pFileHead->KOffset(), pFileHead->getPrZ_Base());

   // don't count this "first" line, we have to "reset" to line 0 numbering.
   m_nCurrentLineNum = 1;

   DeCompress_bits_From_PrZ_setup(PrZHead.PrZ_Bits+3, PrZHead.Prz_ESCs, pFileHead->KOffset(), pFileHead->getPrZ_Base(), (uint32_t)prz_nvalsleft, m_NPGLookingLine, PrZHead.PrZ_SkipEvens, PrZHead.PrZ_IsNewPGen);
   delete pFileHead;
}

PFPrZ_newpgen_File::~PFPrZ_newpgen_File()
{
   // Nothing to do.
}

int PFPrZ_newpgen_File::ReadLine(char *lineBuffer, int sizeofLine)
{
   if (!m_bReadNextLineFromFile)
   {
      // the first line of data is actually embedding in the first line (not in the second line like most other sieve formats).
      // So we should NOT hit the file until after we have processed this stored information.
      m_bReadNextLineFromFile = true;  // Next time, we read from the file
      strncpy(lineBuffer, m_NPGLookingLine, sizeofLine);
      lineBuffer[sizeofLine-1] = 0;
      return prz_nvalsleft == 0;
   }
   // Load new line, compute delta, make a "fake" line, and return it.

   if (!DeCompress_bits_From_PrZ(m_fpInputFile, lineBuffer, m_bIgnoreOutput))
   {
      fclose(m_fpInputFile);
      return true;
   }
   ++m_nCurrentPhysicalLineNum;
   return false;
}

int PFPrZ_newpgen_File::SeekToLine(int LineNumber)
{
   // turn "off" the output of the line into ABC format.  All we need to do is to read the
   // lines, and update all accumulators.  Turning off output speeds up the processing
   // significantly.
   m_bIgnoreOutput = true;

   fseek(m_fpInputFile, 0, SEEK_SET);
   if (m_pIni)
      m_pIni->SetFileProcessing(true);
   m_bEOF = false;
   PrZ_Section_Header_Base *pFileHead=0;
   try
   {
      PrZ_File_Header PrZHead;
      fread(&PrZHead, 1, sizeof(PrZHead), m_fpInputFile);
      if (PrZHead.PrZ_IsFermFactABCD)
         pFileHead = new PrZ_FermFact_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
      else if (PrZHead.PrZ_IsAPSieveABCD)
         pFileHead = new PrZ_APSieve_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
      else if (PrZHead.PrZ_IsNewPGen)
         throw "Error, wrong constructor was called!\n";
      else
         pFileHead = new PrZ_Generic_Section_Header(m_fpInputFile, PrZHead.PrZ_nvalsleft);
      uint64_t xx;
      if (!pFileHead->GetValues(m_MinNum, m_MaxNum, prz_nvalsleft, xx))
      {
         m_MaxNum = LLONG_MAX;
         m_MinNum = pFileHead->KOffset();
      }
      snprintf(m_Line1, ABCLINELEN, "%s", pFileHead->PrZ_GetFirstLine());
      m_nCurrentLineNum = 0;
      m_nCurrentPhysicalLineNum = 0;
      PFNewPGenFile::ProcessFirstLine(m_Line1, pFileHead->KOffset(), pFileHead->getPrZ_Base());
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
      delete pFileHead;
      return e_eof;
   }
   while (m_nCurrentLineNum < LineNumber)
   {
      char Line[1024];
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
   delete pFileHead;
   return e_ok;
}
