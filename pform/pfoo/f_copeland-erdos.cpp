#include "pfoopch.h"
#include "f_copeland-erdos.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "primeserver.h"

#include "f_trivial.h"
#include "pffactorizationsymbol.h"
#include "factornode.h"

F_CopelandErdos::F_CopelandErdos()
   : PFFunctionSymbol("CE")
{
}

DWORD F_CopelandErdos::MinimumArguments() const
{
   return 1;
}

DWORD F_CopelandErdos::MaximumArguments() const
{
   return 1;
}

DWORD F_CopelandErdos::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_CopelandErdos::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval="";
   switch(dwIndex)
   {
   case 0:
      sRetval="_C";
      break;
   default:
      break;
   }
   return sRetval;
}

PFBoolean F_CopelandErdos::CallFunction(PFSymbolTable *pContext)
{
   IPFSymbol *pCnt=pContext->LookupSymbol("_C");

   Integer *C = ((PFIntegerSymbol*)pCnt)->GetValue();
   uint32 n = ((*C) & INT_MAX);
   uint32 length = 0;
   char   buf[10], *ptr;

   Integer mm;
   mm=0;
   uint32 q = 0;
   primeserver->SkipTo(1);

   while ((q = (uint32) primeserver->NextPrime()) != 0)
   {
      sprintf(buf, "%d", q);
      ptr = buf;

      while (length < n && *ptr)
      {
         mm *= 10;
         mm += *ptr - '0';
         length++;
         ptr++;
      }

      if (length == n)
         break;
   }
  
   Integer *r = new Integer(mm);
   pContext->AddSymbol(new PFIntegerSymbol("_result", r));

   return PFBoolean::b_true;
}
