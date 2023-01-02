#include <stdio.h>
#include <string.h>
#include "pfiopch.h"
#include "pfccfile.h"

#include "../pfoo/symboltypes.h"
#include "../pfoo/pffunctionsymbol.h"
#include "../pfoo/pfintegersymbol.h"
#include "../pfoo/f_prime.h"
#include "../pfoo/f_nextprime.h"
#include "../pfoo/f_prevprime.h"
#include "../pfoo/f_fibonacci.h"
#include "../pfoo/f_repunit.h"
#include "../pfoo/f_cyclotomic.h"
#include "../pfoo/f_gcd.h"
#include "../pfoo/f_binomial.h"
#include "../pfoo/f_trivial.h"
#include "../pfoo/f_factor.h"
#include "../pfoo/f_vector.h"
#include "../pfoo/f_issquare.h"
#include "../pfoo/f_smarandache.h"
#include "../pfoo/f_sequence.h"

Integer* ex_evaluate(PFSymbolTable* pContext, const PFString& e);
Integer* ex_evaluate(PFSymbolTable* pContext, const PFString& e, int m);

extern char g_ModularSieveString[256];
extern bool g_bTerseOutput;

int PFCCFile::LetterNumber(char Letter)
{
   if (Letter >= 'A' && Letter <= 'Z')
      return (int)(Letter - 'A');
   else if (Letter >= 'a' && Letter <= 'z')
      return (int)(Letter - 'a');
   else
      return -1;
}

PFCCFile::PFCCFile(const char* FileName)
   : PFSimpleFile(FileName), m_Line(0), m_nLastLetter(-1),
   m_nExprs(1), m_nCurrentMultiPrime(0), m_nCurrentMultiLine(0),
   m_nCurrentPrimeCount(0), m_bLastNumberPrime(false),
   m_bLastLineAnd(false)
{
   int i;
   for (i = 0; i < CCMAXVAR; ++i)
      s_array[i] = 0;
   for (i = 0; i < CCMAXLENGTH; ++i)
      sFormat[i] = 0;
   m_Line = new char[CCLINELEN];
   m_SigString = "CC File";
   m_bInitializedTable = false;
}

void PFCCFile::LoadFirstLine()
{
   if (!g_bTerseOutput)
      PFPrintfLog("Recognized CC Sieve file: \n");

   if (ReadLine(m_Line, CCLINELEN))
   {
      fclose(m_fpInputFile);
      throw "Error, Not a valid CC Sieve file";
   }

   ProcessFirstLine(m_Line);

   // don't count this "first" line, we have to "reset" to line 0 numbering.
   m_nCurrentLineNum = 1;
}

// This part of the LoadFirstLine was removed from that function, and placed into it's own function.
// now a PFCCFile derived class can simply "fix" it's first line to look like a CC line, and
// call this function, instead of having to re-write this logic each time.  This was done in the
// ABCD file class.  It reads the ABCD line, takes the data that it needs, re-creates the first line
// into the ABC format, and then calls this function to process that "fixed" line.
void PFCCFile::ProcessFirstLine(char* FirstLine)
{
   int idx;

   // Strip off any comment from the first line. Sievers can place a comment on the line, and then
   // place any state information following that comment.  PFGW should ignore that information.
   char* cpComment = strstr(FirstLine, "//");
   *m_szCommentData = 0;
   *m_szModFactor = 0;

   // These MUST be deleted each time a new first line is parsed!  This was a HUGE memory leak, found
   // when I started to process ABCD files with "imbedded" ABCD lines within the file (i.e. a file
   // that is a concatenation of ABCD files).  These were critical leaks.  They leaked the length of
   // each ABCD expression AND a 60000 byte buffer!  So a couple 1000 such parsings, and the PC crashed
   // due to lack of memory.
   for (idx = m_nLastLetter; idx >= 0; --idx)
   {
      delete[] s_array[idx];
      s_array[idx] = 0;
   }
   for (idx = m_nExprs - 1; idx >= 0; --idx)
   {
      delete[] sFormat[idx];
      sFormat[idx] = 0;
   }
   // These need to be reset EACH time ProcessFirstLine is called (which can be more than once during the life of the object).
   m_nLastLetter = -1;
   m_nExprs = 1;
   m_nCurrentMultiPrime = 0;
   m_nCurrentMultiLine = 0;
   m_nCurrentPrimeCount = 0;
   m_bLastNumberPrime = false;
   m_bLastLineAnd = false;

   if (cpComment)
   {
      strncpy(m_szCommentData, cpComment, sizeof(m_szCommentData));
      m_szCommentData[sizeof(m_szCommentData) - 1] = 0;
      *cpComment = 0;

      // Check for modular factoring "signature"
      char* cp = strstr(m_szCommentData, "-f{");
      if (cp)
      {
         char* cp1 = strchr(cp, ' ');
         if (!cp1)
            cp1 = strchr(cp, '\t');
         if (!cp1)
         {
            cp1 = strchr(cp, '\n');
            while (*cp1 == '\n' || *cp1 == '\r')
               cp1--;
         }
         if (cp1 && strchr(cp, '}'))
         {
            strncpy(m_szModFactor, cp, sizeof(m_szModFactor) - 1);
            if (sizeof(m_szModFactor) > (size_t)(cp1 - cp))
               m_szModFactor[cp1 - cp + 1] = 0;
            else
               m_szModFactor[sizeof(m_szModFactor) - 1] = 0;
         }
      }
   }
   // Eat any trailing whitespace (or newline chars)
   char* cpEnd = &FirstLine[strlen(FirstLine) - 1];
   while (cpEnd > FirstLine && (*cpEnd == '\n' || *cpEnd == '\r' || *cpEnd == '\t' || *cpEnd == ' '))
      *cpEnd-- = 0;

   if (strlen(FirstLine) < 6) {
      fclose(m_fpInputFile);
      throw "Invalid file.  The first line is not of a valid format!\n";
   }

   int Number;
   char *expPtr, *tempPtr2;
   int chainKind = 0;
   int length = 0;
   char expression[CCLINELEN];

   expPtr = FirstLine + 3;
   while (1) {
      if ((tempPtr2 = strchr(expPtr, '$')) == NULL)
         break;
      if ((Number = LetterNumber(tempPtr2[1])) > m_nLastLetter)
         m_nLastLetter = Number;
      expPtr = tempPtr2 + 2;
   }

   for (idx = 0; idx <= m_nLastLetter; idx++)
      s_array[idx] = new char[CCLINELEN];

   char errorMessage[256];
   if (sscanf(FirstLine, "CC %u,%u,%s", &chainKind, &length, expression) != 3) {
      snprintf(errorMessage, sizeof(errorMessage), "\nCritical Error, first line in CC file has invalid format\n");
      throw errorMessage;
   }

   if (chainKind < 1 || chainKind > 2) {
      snprintf(errorMessage, sizeof(errorMessage), "\nCritical Error, first line in CC file must have chain kind of 1 or 2\n");
      throw errorMessage;
   }

   if (length <  2) {
      snprintf(errorMessage, sizeof(errorMessage), "\nCritical Error, first line in CC file must have length of at least 2\n");
      throw errorMessage;
   }

   if (length > CCMAXLENGTH) {
      snprintf(errorMessage, sizeof(errorMessage), "\nCritical Error, first line in CC file must have length less than %u\n", CCMAXLENGTH);
      throw errorMessage;
   }

   // Remove leading spaces
   expPtr = expression;
   while (expPtr[0] == ' ')
      expPtr++;

   if (strchr(expPtr, '&') > 0 || strchr(expPtr, '|') > 0) {
      snprintf(errorMessage, sizeof(errorMessage), "\nCritical Error, first line in CC file does not support & or |\n");
      throw errorMessage;

   }

   while (1) {
      if ((tempPtr2 = strchr(expPtr, '$')) == NULL)
         break;
      if ((Number = LetterNumber(tempPtr2[1])) > m_nLastLetter)
         m_nLastLetter = Number;
      expPtr = tempPtr2 + 2;
   }


   sFormat[0] = new char[strlen(expression) + 1];
   strcpy(sFormat[0], expression);
   m_eAndOr[0] = e_And;

   for (idx = 1; idx <= length; idx++) {
      int mult = 1 << idx;
      int add = (chainKind == 1 ? (mult - 1) : -(mult - 1));

      sFormat[idx] = new char[strlen(expression) + 30];
      snprintf(sFormat[idx], strlen(expression) + 30, "%u*(%s)%+d", mult, expression, add);
      m_eAndOr[idx] = e_And;
   }

   char cpTmpOutput[256];
   snprintf(cpTmpOutput, sizeof(cpTmpOutput), "CC File with first term of %s with a length of %u", sFormat[0], length);
   m_SigString = cpTmpOutput;
   m_nExprs = length;
}

PFCCFile::~PFCCFile()
{
   delete[] m_Line;
   int i;
   for (i = CCMAXLENGTH - 1; i >= 0; --i)
      delete[] sFormat[i];
   for (i = CCMAXVAR - 1; i >= 0; --i)
      delete[] s_array[i];
   if (m_bInitializedTable)
      m_tbl.RemoveAll();
}

int PFCCFile::GetNextLine(PFString& sLine, Integer* /*pInt*/, bool* pbIntValid, PFSymbolTable*)
{
   char* tempPtr;
   int i;

SkipThisLine:;

   // we do not return an Integer in the pInt pointer (yet ;)
   if (pbIntValid)
      *pbIntValid = false;
   if (m_bEOF)
   {
      m_sCurrentExpression = "";
      return e_eof;
   }

   bool bStoreThisExpression = false;
   if (m_nCurrentMultiPrime == 0) {
      bStoreThisExpression = true;
      m_nCurrentPrimeCount = 0;
      if (ReadLine(m_Line, CCLINELEN)) {
         if (m_pIni)
            m_pIni->SetFileProcessing(false);
         m_bEOF = true;
         m_sCurrentExpression = "";
         return e_eof;
      }
      // This code will now correctly skip blank lines in the file
      tempPtr = &m_Line[-1];
      for (i = 0; i <= m_nLastLetter; i++)
      {
         if (!tempPtr)
         {
            sLine = "";
            return e_ok;
         }
         tempPtr++;
         if (sscanf(tempPtr, "%s", s_array[i]) == EOF)
         {
            sLine = "";
            return e_ok;
         }
         tempPtr = strchr(tempPtr, ' ');
      }
      if (bStoreThisExpression)
      {
         m_nCurrentLineNum++;
         if (m_pIni)
            m_pIni->SetFileLineNum(m_nCurrentLineNum);
      }
   }

   if (!m_nCurrentMultiPrime && !ProcessThisLine())
      goto SkipThisLine;

   MakeExpr(sLine);
   if (bStoreThisExpression)
      m_sCurrentExpression = sLine;

   LoadModularFactorString();

   return e_ok;
}

void PFCCFile::MakeExpr(PFString& sLine)
{
   char* Buff = new char[CCLINELEN << 1];
   char* Format = new char[CCLINELEN];
   char* tempPtr, * tempPtr2, * bufPtr;

   strcpy(Format, sFormat[m_nCurrentMultiPrime]);
   tempPtr = Format;
   bufPtr = Buff;
   while (1) {
      if ((tempPtr2 = strchr(tempPtr, '$')) == NULL)
         break;
      tempPtr2[0] = 0;
      bufPtr += snprintf(bufPtr, CCLINELEN << 1, "%s%s", tempPtr, s_array[LetterNumber(tempPtr2[1])]);
      tempPtr = tempPtr2 + 2;
   }
   strcpy(bufPtr, tempPtr);

   strtok(Buff, "\r\n");  // remove possible trailing crap from line

   char* cp = Buff;
   while (*cp == ' ')
      cp++;
   sLine = cp;

   delete[] Buff;
   delete[] Format;
}

int PFCCFile::SeekToLine(int LineNumber)
{
   if (LineNumber < m_nCurrentLineNum)
   {
      fseek(m_fpInputFile, 0, SEEK_SET);
      m_nCurrentPhysicalLineNum = 1;
      ReadLine(m_Line, CCLINELEN);
      m_nCurrentLineNum = 1;
      if (m_pIni)
         m_pIni->SetFileProcessing(true);
      m_bEOF = false;
   }
   while (m_nCurrentLineNum < LineNumber)
   {
      if (ReadLine(m_Line, CCLINELEN)) {
         if (m_pIni)
            m_pIni->SetFileProcessing(false);
         m_bEOF = true;
         return e_eof;
      }
      m_nCurrentLineNum++;
   }
   if (m_pIni)
      m_pIni->SetFileLineNum(m_nCurrentLineNum);
   return e_ok;
}

void PFCCFile::CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool* p_bMessageStringIsValid, PFString* p_MessageString)
{
   m_bLastNumberPrime = bIsPrime || bIsPRP;
   if (p_bMessageStringIsValid)
      *p_bMessageStringIsValid = false;
   if (bIsPrime || bIsPRP) {
      m_nCurrentMultiPrime++;
      m_nCurrentPrimeCount++;
   }
   else {
      if (m_eAndOr[m_nCurrentMultiPrime] == e_Or) {
         if (m_eAndOr[++m_nCurrentMultiPrime] == e_And && m_nCurrentPrimeCount == 0) {
            m_nCurrentMultiPrime = 0;
            m_nCurrentMultiLine = 0;
            return;
         }
      }
      else {
         m_nCurrentMultiPrime = 0;
         m_nCurrentMultiLine = 0;
         return;
      }
   }
   if (m_nCurrentMultiPrime < m_nExprs)
      return;
   m_nCurrentMultiPrime = 0;
   if (m_bLastLineAnd) {
      m_nCurrentMultiLine++;
      return;
   }
   if (m_nExprs == 1 && m_nCurrentMultiLine < 2)
      return;
   if (m_nCurrentPrimeCount == m_nExprs) {
      *p_MessageString = "  - Complete Set -";
      *p_bMessageStringIsValid = true;
   }
   else if (m_nCurrentPrimeCount > 1) {
      *p_MessageString = "  - Partial Set -";
      *p_bMessageStringIsValid = true;
   }
}

// NOTE that pMSS MUST be contained within the g_ModularSieveString array.
void PFCCFile::RemoveExpressions(char* pMSS, bool bCheckUsingConditionSyntax)
{
   if (*pMSS != '{')
      return;
   pMSS++;
   bool bAgain = false;
   if (bCheckUsingConditionSyntax)
   {
      if (*pMSS == 'y' || *pMSS == 'n')
      {
         bAgain = true;
         pMSS += 2;
      }
      else if (*pMSS == 'f' || *pMSS == 'p')
         pMSS += 2;
      else
         throw "Unknown condition within modular factoring expression in the CC file\n";
   }
   for (;;)
   {
      char* cp = strchr(pMSS, '}');
      if (!cp)
         return;
      char* cp1 = strchr(pMSS, ',');
      if (cp1 && cp1 < cp)
         cp = cp1;   // the comma was preceeding the }
      char* cpTest = pMSS;
      bool bExpr = false;
      while (!bExpr && cpTest < cp)
      {
         if (*cpTest < '0' || *cpTest > '9')
            bExpr = true;
         cpTest++;
      }
      if (bExpr)
      {
         // expression found, now remove it.
         char* head = new char[pMSS - g_ModularSieveString + 1];
         strncpy(head, g_ModularSieveString, pMSS - g_ModularSieveString);
         head[pMSS - g_ModularSieveString] = 0;
         char* tail = new char[strlen(cp) + 1];
         strcpy(tail, cp);
         char* expr = new char[cp - pMSS + 1];
         strncpy(expr, pMSS, cp - pMSS);
         expr[cp - pMSS] = 0;
         if (!m_bInitializedTable)
         {
            m_tbl.AddSymbol(new F_Prime);
            m_tbl.AddSymbol(new F_NextPrime);
            m_tbl.AddSymbol(new F_PrevPrime);
            m_tbl.AddSymbol(new F_Fibonacci_U);
            m_tbl.AddSymbol(new F_Fibonacci_V);
            m_tbl.AddSymbol(new F_Fibonacci_F);
            m_tbl.AddSymbol(new F_Fibonacci_L);
            m_tbl.AddSymbol(new F_Repunit);
            m_tbl.AddSymbol(new F_Cyclotomic);
            m_tbl.AddSymbol(new F_GCD);
            m_tbl.AddSymbol(new F_Binomial);
            m_tbl.AddSymbol(new F_Sequence);
            m_tbl.AddSymbol(new F_LucasV);
            m_tbl.AddSymbol(new F_LucasU);
            m_tbl.AddSymbol(new F_PrimV);
            m_tbl.AddSymbol(new F_PrimU);
            m_tbl.AddSymbol(new F_NSW_S);
            m_tbl.AddSymbol(new F_NSW_W);
            m_bInitializedTable = true;
         }

         Integer* N = ex_evaluate(&m_tbl, expr);
         if (N)
         {
            int n = ((*N) & INT_MAX);
            snprintf(g_ModularSieveString, sizeof(g_ModularSieveString), "%s%d%s", head, n, tail);
            delete N;
         }
         else
            *g_ModularSieveString = 0;
         delete[] expr;
         delete[] tail;
         delete[] head;
      }
      if (!bAgain)
         return;
      pMSS = cp + 1;
      bAgain = false;
   }
}

void PFCCFile::LoadModularFactorString()
{
   if (m_szModFactor[0])
   {
      char* cpIn = &m_szModFactor[2];  // skip the -f
      char* cpOut = g_ModularSieveString;
      while (*cpIn && *cpIn != ' ')
      {
         if (*cpIn == '$')
         {
            cpOut += snprintf(cpOut, sizeof(g_ModularSieveString), "%s", s_array[LetterNumber(cpIn[1])]);
            cpIn += 2;
         }
         else
            *cpOut++ = *cpIn++;
      }
      *cpOut = 0;

      // Now remove any expressions if there are any from the factor string
      char* cp = g_ModularSieveString;
      RemoveExpressions(cp, false);
      cp = strchr(&cp[1], '{');
      while (cp)
      {
         RemoveExpressions(cp, true);
         cp = strchr(&cp[1], '{');
      }
   }
}
