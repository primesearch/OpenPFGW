#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "pfiopch.h"
#include <string.h>
#include "pfcheckfile.h"

extern uint64 g_u64ResidueVal;

PFCheckFile::PFCheckFile(const char *FileName) : PFSimpleFile(FileName)
{
}

PFCheckFile::~PFCheckFile() {
}

int PFCheckFile::GetNextLine(PFString &sLine, Integer * /*i*/, bool *b, PFSymbolTable *)
{
   if (b)
      *b = false;       // this simple file class does not "remember" or "fill in" the Integer value, ever
Start:
   sLine = "";
   m_sCurrentExpression = "";
   if (m_bEOF)
      return e_eof;

   char Line[65536];
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

   ReadLine(Line, sizeof(Line)-1);
   m_nCurrentPhysicalLineNum++;

   int LineLen = strlen(Line);
   int bGotEOL=false;
   while (LineLen && (Line[LineLen-1] == '\n' || Line[LineLen-1] == '\r') )
   {
      bGotEOL = true;
      --LineLen;
      Line[LineLen]  = 0;
   }
   if (!bGotEOL)
   {
      // Eat any comment.
      char *cp = strstr(Line, "//");
      if (cp)
         *cp = 0;
      LineLen = strlen(Line);
      // Eat trailing white space.
      while (LineLen && (Line[LineLen-1] == ' ' || Line[LineLen-1] == '\t'))
         Line[LineLen--] = 0;
      // eat leading whitespace
      cp = Line;
      while (*cp == ' ' || *cp == '\t')
         cp++;
      sLine += cp;
      goto ReadRestOfThisLine;
   }
   if (!Line[0])
      goto ReadRestOfThisLine;

   // Eat any comment.
   char *cp = strstr(Line, "//");
   if (cp)
      *cp = 0;
   LineLen = strlen(Line);

   // Check for continuation char
   if (Line[LineLen-1] == '\\')
   {
      Line[LineLen-1] = 0;
      sLine += Line;
      goto ReadRestOfThisLine;
   }

   // right trim the line
   while (LineLen && (Line[LineLen-1] == ' ' || Line[LineLen-1] == '\t') )
      Line[LineLen-1]  = 0;
   // Did our right trim eat all of the line, if so, then read the next line.
   if (!LineLen)
      goto ReadRestOfThisLine;

   // got the line.
   sLine += Line;

   // Change needed due to Phil's changes.
// char *expr=sLine.GetBuffer(sLine.GetLength());
   char *expr=(char *) (const char *) sLine;

   char *ptr;

   if ((ptr=strstr(expr,"composite"))!=NULL) {
      *(ptr-4)=0;
      ptr+=12;
      m_nResidue=0;
      m_bPrime=false;
      for (int i=0;i<16;i++) {
         if (ptr[i]==']')
            break;
         m_nResidue<<=4;
         if (ptr[i]>='0' && ptr[i]<='9')
            m_nResidue|=ptr[i]-'0';
         else if (ptr[i]>='A' && ptr[i]<='F')
            m_nResidue|=ptr[i]-'A'+10;
         else {
            PFOutput::EnableOneLineForceScreenOutput();
            PFPrintfStderr("Bad line at line %d",m_nCurrentLineNum);
            return e_bad_file;
         }
      }
   } else if (strstr(expr,"PRP")!=NULL) {
      if ((ptr=strstr(expr," is "))==NULL) {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("Bad line at line %d",m_nCurrentLineNum);
         return e_bad_file;
      }
      *ptr=0;
      m_nResidue=1;
      m_bPrime=true;
   } else
      goto Start;

   m_nCurrentLineNum++;
   if (m_pIni)
      m_pIni->SetFileLineNum(m_nCurrentLineNum);

   // since sLine had a "NULL" placed in the middle of it, its size is now "broken".
   // The easiest fix is to simply re-asign the string buffer back to itself.
   sLine = expr;
   m_sCurrentExpression = sLine;

   return e_ok;
}

void PFCheckFile::CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool *p_bMessageStringIsValid, PFString *p_MessageString)
{
   if (!p_bMessageStringIsValid || !p_MessageString)
      return;

   if ((bIsPrime || bIsPRP) && !m_bPrime) {
      *p_MessageString = PFString("\n!!!Double Verify ERROR!!! testing ")+m_sCurrentExpression+": PRP result NOT expected.\n";
      *p_bMessageStringIsValid=true;
      return;
   }

   if (!(bIsPrime || bIsPRP) && m_bPrime) {
      *p_MessageString = PFString("\n!!!Double Verify ERROR!!! testing ")+m_sCurrentExpression+": PRP result WAS expected.\n";
      *p_bMessageStringIsValid=true;
      return;
   }

   if (!(bIsPrime || bIsPRP) && m_nResidue!=g_u64ResidueVal && g_u64ResidueVal!=0) {   // note u64Res=0 happens when composites are tossed at one of the  "test" modes.
      char residue1[40], residue2[40];
      sprintf(residue1,"[%08lX%08lX]",(uint32)(m_nResidue>>32),(uint32)(m_nResidue&0xFFFFFFFF));
      sprintf(residue2,"[%08lX%08lX]\n",(uint32)(g_u64ResidueVal>>32),(uint32)(g_u64ResidueVal&0xFFFFFFFF));
      *p_MessageString = PFString("\n!!!Double Verify ERROR!!! testing ")+m_sCurrentExpression+": Residue mismatch.\nExpected residue: ";
      *p_MessageString += residue1;
      *p_MessageString += " residue computed: ";
      *p_MessageString += residue2;
      *p_MessageString += "\n";

      *p_bMessageStringIsValid=true;
      return;
   }

   *p_bMessageStringIsValid = false;
}
