#include "pfoopch.h"
#include "f_smarandache.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"

#include "f_trivial.h"
#include "pffactorizationsymbol.h"
#include "factornode.h"

F_Smarandache::F_Smarandache()
   : PFFunctionSymbol("Sm")
{
}

DWORD F_Smarandache::MinimumArguments() const
{
   return 1;
}

DWORD F_Smarandache::MaximumArguments() const
{
   return 2;
}

DWORD F_Smarandache::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_Smarandache::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval="";
   switch(dwIndex)
   {
   case 0:
      sRetval="_C";
      break;
   case 1:
      sRetval="_S";
      break;
   default:
      break;
   }
   return sRetval;
}

static int WhatSize_sm(int nMin, int nMax)
{
   int CharSize = 190;  // this will handle any Sm(1) to Sm(99)
   int tmpCnt = nMax;

   // only handle numbers up to sm(1 million)
   if (nMax > 10000000)
      return -1;

   // check validity
   if (nMin < 1 || nMin > nMax)
      return -1;

   if (tmpCnt > 1000000)
   {
      CharSize += 7*(tmpCnt-1000000);
      tmpCnt = 1000000;
   }
   if (tmpCnt > 100000)
   {
      CharSize += 6*(tmpCnt-100000);
      tmpCnt = 100000;
   }
   if (tmpCnt > 10000)
   {
      CharSize += 5*(tmpCnt-10000);
      tmpCnt = 10000;
   }
   if (tmpCnt > 1000)
   {
      CharSize += 4*(tmpCnt-1000);
      tmpCnt = 1000;
   }
   if (tmpCnt > 100)
   {
      CharSize += 3*(tmpCnt-100);
      tmpCnt = 100;
   }
   return CharSize;
}


PFBoolean F_Smarandache::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;

   Integer *r=NULL;

   // what to do next depends if we are running in one or two parameter mode
   IPFSymbol *pStart=pContext->LookupSymbol("_S");
   IPFSymbol *pCnt=pContext->LookupSymbol("_C");

   Integer *C = ((PFIntegerSymbol*)pCnt)->GetValue();
   int nMax = ((*C) & INT_MAX);
   int nMin = 1;
   if(pStart!=NULL)
   {
      Integer *S = ((PFIntegerSymbol*)pStart)->GetValue();
      nMin = nMax;
      nMax = ((*S) & INT_MAX);
   }

   // calculate how big of a char[] array we need.
   int CharSize = WhatSize_sm(nMin, nMax);
   if (CharSize == -1)
      return bRetval;

   char *tmpBuf = new char[CharSize+10];
   char *cp=tmpBuf;

   int i=nMin;
   while(i <= nMax)
      cp += snprintf(cp, CharSize + 10, "%d", i++);

   r=new Integer;
   r->atoI(tmpBuf);
   delete[] tmpBuf;
   pContext->AddSymbol(new PFIntegerSymbol("_result",r));
   bRetval=PFBoolean::b_true;

   return bRetval;
}

F_Smarandache_r::F_Smarandache_r()
   : PFFunctionSymbol("Smr")
{
}

DWORD F_Smarandache_r::MinimumArguments() const
{
   return 1;
}

DWORD F_Smarandache_r::MaximumArguments() const
{
   return 2;
}

DWORD F_Smarandache_r::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_Smarandache_r::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval="";
   switch(dwIndex)
   {
   case 0:
      sRetval="_C";
      break;
   case 1:
      sRetval="_S";
      break;
   default:
      break;
   }
   return sRetval;
}



PFBoolean F_Smarandache_r::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;

   Integer *r=NULL;

   // what to do next depends if we are running in one or two parameter mode
   IPFSymbol *pStart=pContext->LookupSymbol("_S");
   IPFSymbol *pCnt=pContext->LookupSymbol("_C");

   Integer *C = ((PFIntegerSymbol*)pCnt)->GetValue();
   int nMax = ((*C) & INT_MAX);
   int nMin = 1;
   if(pStart!=NULL)
   {
      Integer *S = ((PFIntegerSymbol*)pStart)->GetValue();
      nMin = ((*S) & INT_MAX);
   }

   // calculate how big of a char[] array we need.
   int CharSize = WhatSize_sm(nMin, nMax);
   if (CharSize == -1)
      return bRetval;

   char *tmpBuf = new char[CharSize+10];
   char *cp=tmpBuf;

   int i = nMax;
   while(i >= nMin)
      cp += snprintf(cp, CharSize + 10, "%d", i--);

   r=new Integer;
   r->atoI(tmpBuf);
   delete[] tmpBuf;
   pContext->AddSymbol(new PFIntegerSymbol("_result",r));
   bRetval=PFBoolean::b_true;

   return bRetval;
}
