//  PFFile class for file handling
#include "pfiopch.h"
#include <stdio.h>
#include <string.h>

// also includes pffile.h but defines PFNewPGenFile class which is needed for the openInputFile() function.
#include "pfnewpgenfile.h"
#include "pfabcdfile.h"
#include "pfabc2file.h"
#include "pfcpapfile.h"
#include "pfscriptfile.h"
#include "pfcheckfile.h"
#include "pfprzfile.h"

extern bool g_bTerseOutput;
extern bool g_ShowTestResult;

#define MAX_INPUT_LINE_LENGTH 5000000

// default protected constructor.  It is used as the "default" constructor needed for the PFStringFile class.
PFSimpleFile::PFSimpleFile()
  : m_nCurrentLineNum(1), m_nCurrentPhysicalLineNum(1),
    m_fpInputFile(0), m_cpFileName(NULL), m_pIni(NULL), m_sCurrentExpression(""), m_bEOF(false)
{
   m_nCurrentLine = new char[MAX_INPUT_LINE_LENGTH];
}

PFSimpleFile::PFSimpleFile(const char* FileName)
  : m_nCurrentLineNum(1), m_nCurrentPhysicalLineNum(1),
    m_fpInputFile(0), m_cpFileName(NULL), m_pIni(NULL), m_sCurrentExpression(""), m_bEOF(false)
{
   m_nCurrentLine = new char[MAX_INPUT_LINE_LENGTH];

   m_cpFileName = new char[strlen(FileName)+1];
   strcpy(m_cpFileName, FileName);
   m_fpInputFile = fopen(m_cpFileName, "rt");
   if (!m_fpInputFile)
      throw ("Can't find input file");
}

PFSimpleFile::~PFSimpleFile()
{
   if (m_fpInputFile)
      fclose(m_fpInputFile);
   // a PFStringFile class does not set this pointer to an object.
   if (m_pIni)
   {
      if (m_pIni->GetFileProcessing())
         m_pIni->SetExprChecksum(&m_sCurrentExpression);
      else
         m_pIni->SetExprChecksum(NULL);   // clear that line.
      // Inform the ini object NOT to use our m_sCurrentExpression any more.
      m_pIni->AssignPointerOfCurrentExpression(NULL);
      m_pIni->ForceFlush();
   }
   delete[] m_cpFileName;
   delete[] m_nCurrentLine;
}

int PFSimpleFile::SecondStageConstruction(PFIni* pIniFile)
{
   m_pIni = pIniFile;

   // Load the first line. NOTE that NewPGen and CPAPSieve files will read a line (and possibly more),
   // yet the m_nCurrentLineNum will still be set to 1.
   LoadFirstLine();
   if (!g_bTerseOutput)
      Printf_WhoAmIsString();

   // Ok the "first" line has been read (possibly).  Now if the .ini file says what we were working
   // on this file, and that we had not "finished" this file, then we will skip the correct number
   // of lines, and then reading will proceed from that point.
   
   if (m_pIni)
   {
      // Was the file "still" processing?
      if (m_pIni->GetFileProcessing())
      {
         PFString *s = m_pIni->GetFileName();
         if (!s->CompareNoCase(m_cpFileName))
         {
            PFPrintfLog("Resuming input file %s at line %d\n\n", m_cpFileName, m_pIni->GetFileLineNum());
            // we have to back up 3 lines instead of just 2, to handle the ABCD having another ABCD line
            // in the file. If the breakout happened on that second ABCD "header" line, then we need to back
            // up one more line.  Also in the for() loop below, we go ahead 4 files, instead of the 3 which
            // we were doing before.
            int n = m_pIni->GetFileLineNum() - 4;
            if (n < 1)
               n = 1;
            if (m_pIni->IsExprChecksumNull())
               n = m_pIni->GetFileLineNum() - 1;
            SeekToLine(n);
            PFString sThisLine, sExpectingThisLine;
            for (int i = 0; i < 6; i++, n++)
            {
               GetNextLine(sThisLine);
               if (m_pIni->CompareExprChecksum(&sThisLine, &sExpectingThisLine))
                  break;
            }
            // Check to see if 'l' is what we are expecting it to be
            if (m_pIni->CompareExprChecksum(&sThisLine, &sExpectingThisLine))
               // correct line, so simple seek to it again.  I know this could be "optimized", but this
               // is just the "first shot" at making line recognition work.
               SeekToLine(n);
            else
            {
               // Not the correct line. For now, simply warn the user, and start at the beginning of the file.
               // Todo:
               //    Try one line back and 1 line forward.
               PFPrintfLog("\n***WARNING! file %s line %d does not match what is expected.\n", m_cpFileName, m_pIni->GetFileLineNum());
               PFPrintfLog("Expecting:      %s\n", LPCTSTR(sExpectingThisLine));
               PFPrintfLog("File contained: %s\n", LPCTSTR(sThisLine));
               PFPrintfLog("Starting over at the beginning of the file\n\n");
               SeekToLine(1);
            }
         }
         // We HAVE to clean up the string from the GetFileName() function.
         delete s;
      }
      else
      {
         PFString *s = m_pIni->GetFileName();
         if (!s->CompareNoCase(m_cpFileName) && !g_bTerseOutput)
            PFPrintfLog("\n***WARNING! file %s may have already been fully processed.\n\n", m_cpFileName);
         delete s;
      }
      // Tell the .ini handler, where the PFString is which will be kept updated.
      m_pIni->AssignPointerOfCurrentExpression(&m_sCurrentExpression);
      PFString s(m_cpFileName);
      m_pIni->SetFileName(&s);
      m_pIni->SetFileLineNum(m_nCurrentLineNum);
      m_pIni->SetFileProcessing(true);
      m_bEOF = false;
   }
   return e_ok;
}


int PFSimpleFile::ReadLine(char *Line, int sizeofLine)
{
   Line[0] = 0;
   fgets(Line, sizeofLine, m_fpInputFile);
   
   char *cp = Line;

   // Bug fix request from Joe McLean.  If there was a leading space on an ABC file (or other formats probably), then
   // the ABC parser built the wrong file.  This simply work around simply left trims the line.
   cp = Line;
   while (*cp == ' ' || *cp == '\t')
      cp++;

   if (cp != Line)
      memmove(Line, cp, strlen(cp)+1);

   if (*Line != 'A' && strstr(Line, " | "))
   {
      cp = Line;

      while (*cp != 0) {
         if (*cp == '\n' || *cp == '\r')
            *cp = 0;
         else
            cp++;
      }
      
     // Change "xx | yyyyyy" into "(yyyyyy) % x"
      cp = strstr(Line, " | ");

      char divisor[500];
      *cp = 0;
      *(cp+2) = '(';

      sprintf(divisor, ") %% %s\n", Line);
      strcpy(Line, cp + 2);
      strcat(Line, divisor);
   }

   // NOTE that a file which does NOT have a \n at the end of it will read the last line, but IMMEDIATELY return
   // feof() of true.  In that case, we do have a valid input line (even though feof() is true.  We need to
   // handle this line, and then upon the next read, we will honor the feof().  To do this, we check to see if
   // anything was written to Line[0], and if so, we assume that even though the file is at the end, there is
   // valid data to work on, so we do NOT return that we are out of data.
   return Line[0] == 0 && feof(m_fpInputFile);
}

int PFSimpleFile::GetNextLine(PFString &sLine, Integer *, bool *b, PFSymbolTable *)
{
   if (b)
      *b = false;       // this simple file class does not "remember" or "fill in" the Integer value, ever
   sLine = "";
   m_sCurrentExpression = "";
   if (m_bEOF)
      return e_eof;

ReadRestOfThisLine:;
   if (feof(m_fpInputFile))
   {
      if (sLine=="")
      {
         if (m_pIni)
            m_pIni->SetFileProcessing(false);
         m_bEOF = true;
         m_sCurrentExpression = "";
         return e_eof;
      }
      m_sCurrentExpression = sLine;
      return e_ok;
   }

   ReadLine(m_nCurrentLine, MAX_INPUT_LINE_LENGTH);
   m_nCurrentPhysicalLineNum++;

   int LineLen = (int) strlen(m_nCurrentLine);
   int bGotEOL=false;
   while (LineLen && (m_nCurrentLine[LineLen-1] == '\n' || m_nCurrentLine[LineLen-1] == '\r') )
   {
      bGotEOL = true;
      m_nCurrentLine[--LineLen]  = 0;
   }

   if (!bGotEOL)
   {
      // Eat any comment.
      char *cp = strstr(m_nCurrentLine, "//");
      if (cp)
         *cp = 0;
      LineLen = (int) strlen(m_nCurrentLine);
      // Eat trailing white space.
      while (LineLen && (m_nCurrentLine[LineLen-1] == ' ' || m_nCurrentLine[LineLen-1] == '\t'))
         m_nCurrentLine[--LineLen] = 0;
      sLine += m_nCurrentLine;
      goto ReadRestOfThisLine;
   }
   if (!m_nCurrentLine[0])
      goto ReadRestOfThisLine;

   // Eat any comment.
   char *cp = strstr(m_nCurrentLine, "//");
   if (cp)
      *cp = 0;
   LineLen = (int) strlen(m_nCurrentLine);

   // Check for continuation char
   if (m_nCurrentLine[LineLen-1] == '\\')
   {
      m_nCurrentLine[LineLen-1] = 0;
      sLine += m_nCurrentLine;
      goto ReadRestOfThisLine;
   }

   // right trim the line
   while (LineLen && (m_nCurrentLine[LineLen-1] == ' ' || m_nCurrentLine[LineLen-1] == '\t') )
      m_nCurrentLine[--LineLen]  = 0;
   // Did our right trim eat all of the line, if so, then read the next line.
   if (!LineLen)
      goto ReadRestOfThisLine;

   // got the line.
   sLine += m_nCurrentLine;
   m_sCurrentExpression = sLine;

   m_nCurrentLineNum++;
   if (m_pIni)
      m_pIni->SetFileLineNum(m_nCurrentLineNum);
   return e_ok;
}

int PFSimpleFile::SeekToLine(int LineNumber)
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

bool PFSimpleFile::ProcessThisLine()
{
   // by default a PFFile will process a line.  This "can" be over-ridden to allow a derived class
   // to stop processing "certain" lines in a file (after some "condition" has been met, for instance).
   return true;
}


int PFSimpleFile::Rewind()
{
   int Ret = SeekToLine(1);
   return Ret;
}

int PFSimpleFile::GetCurrentLineNumbers(int &nVirtualLineNumber, int &nPhysicalLineNumber)
{
   nPhysicalLineNumber = -1;
   nVirtualLineNumber = -1;
   if (feof(m_fpInputFile))
      return e_eof;
   nVirtualLineNumber = m_nCurrentLineNum;
   nPhysicalLineNumber = m_nCurrentPhysicalLineNum;
   return e_ok;
}

// in the "base" class, these values don't make since, but the virtual function needs to be here.
int PFSimpleFile::GetKNB(uint64 & /*k*/, uint64 & /*n*/, unsigned & /*b*/)
{
   return e_unknown;
}

void PFSimpleFile::CurrentNumberIsPRPOrPrime(bool /*bIsPRP*/, bool /*bIsPrime*/, bool *p_bMessageStringIsValid, PFString * /*p_MessageString*/)
{
   if (p_bMessageStringIsValid)
      *p_bMessageStringIsValid = false;
}




//
//
//   PFStringFile
//
//   This class is based upon the PFSimpleFile, and adds ability to handle command line entered expression, and can
//   possibly be "morphed" into a "interpreter" similar to the *nix "bc" little language interpreter
//
//

PFStringFile::PFStringFile() : sData(""), bUsed(false)
{
}

PFStringFile::~PFStringFile()
{
}

int PFStringFile::GetNextLine(PFString &sLine, Integer *, bool *b, PFSymbolTable *)
{
   if (b)
      *b = false;       // this string file class does not "remember" or "fill in" the Integer value, ever
   if (bUsed)
      return e_eof;
   sLine = sData;
   bUsed=true;
   return e_ok;
}

int PFStringFile::Rewind()
{
   return e_ok;
}

int PFStringFile::SeekToLine(int LineNumber)
{
   if (LineNumber == 0)
      return e_ok;
   return e_eof;
}

int PFStringFile::WriteToString(const char *String)
{
   sData = String;
   return e_ok;
}

int PFStringFile::AppendString(const char *String)
{
   sData += String;
   return e_ok;
}

int PFStringFile::ClearString(const char * /*String*/)
{
   sData = "";
   return e_ok;
}

// Global function to "allocate" an unknown file.  This function will allocate a PFSimpleFile or a PFNewPGenFile
// depending upon if the file is a NewPGen log file, or simply a file containing expressions.
PFSimpleFile *openInputFile(const char *FileName, PFIni* pIniFile, const char **ErrorMessage)
{
   static char s_ErrorMessage[256];
   *s_ErrorMessage = 0;
   if (ErrorMessage)
      *ErrorMessage = s_ErrorMessage;

   char Line[65000];
   FILE *fp;
   fp = fopen(FileName, "rt");
   if (!fp)
   {
      sprintf (s_ErrorMessage, "Error opening file %s", FileName);
      return NULL;
   }
   // Check for a NewPGen or JFCPAP signature
   *Line = 0;
   fgets(Line, sizeof(Line), fp);
   Line[sizeof(Line)-1] = 0;
   fclose(fp);

   char c;
   uint64 u64tmp; // Unused right now, but may be useful in the future (tells the depth of the sieving)
   int len, base, bits;
   int count = sscanf(Line, ""ULL_FORMAT":%c:%d:%d:%d", &u64tmp, &c, &len, &base, &bits);

   // set null so that we can delete in the catch
   PFSimpleFile *pf=0;
   try
   {
      pf = 0;
      if (count == 5)
         pf = new PFNewPGenFile(FileName);
      else
      {
         if (!strncmp(Line, "JF CPAP-", 8))
            pf = new PFCPAPFile(FileName);
         else if (!strncmp(Line, "ABC ", 4))
            pf = new PFABCFile(FileName);
         else if (!strncmp(Line, "ABCD ", 5))
            pf = new PFABCDFile(FileName);
         else if (!strncmp(Line, "ABC2 ", 5))
            pf = new PFABC2File(FileName);
         else if (!strncmp(Line, "PrZ", 3)) // In win32 this IS the string, but in Unix, it might not be, so only look at the first 3 chars
         {
            fp = fopen(FileName, "rb");
            PrZ_File_Header Head;
            fread(&Head, 1, sizeof(Head), fp);
            fclose(fp);
            if (Head.PrZ_IsNewPGen)
               pf = new PFPrZ_newpgen_File(FileName);
            else
               pf = new PFPrZFile(FileName);
         }
         else if (!strncmp(Line, "SCRIPT", 6))
            pf = new PFScriptFile(FileName);
         else if ((strstr(Line, " is composite: ")) || (strstr(Line, " is ") && strstr(Line, "-PRP! ")))
         {
            pf = new PFCheckFile(FileName);
            g_ShowTestResult = true;
         }
         else
         {
            pf = new PFSimpleFile(FileName);
            g_ShowTestResult = true;
         }
      }
      pf->SecondStageConstruction(pIniFile);
   }
   catch(char *s)
   {
      // NOTE pf CAN be constructed, and still throw, so we MUST delete this item.
      delete pf;
      sprintf (s_ErrorMessage, "Error %s opening file %s", s, FileName);
      return NULL;
   }
   return pf;
}

