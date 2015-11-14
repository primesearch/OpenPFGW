#include "pfoopch.h"
#include "f_smarandache-wellin.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "primeserver.h"

#include "f_trivial.h"
#include "pffactorizationsymbol.h"
#include "factornode.h"

F_SmarandacheWellinBase::F_SmarandacheWellinBase(const PFString &sName)
   : PFFunctionSymbol(sName)
{
}

DWORD F_SmarandacheWellinBase::MinimumArguments() const
{
   return 1;
}

DWORD F_SmarandacheWellinBase::MaximumArguments() const
{
   return 1;
}

DWORD F_SmarandacheWellinBase::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_SmarandacheWellinBase::GetArgumentName(DWORD dwIndex) const
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

F_SmarandacheWellin::F_SmarandacheWellin()
   : F_SmarandacheWellinBase("SmW")
{
}

PFBoolean F_SmarandacheWellin::CallFunction(PFSymbolTable *pContext)
{
   IPFSymbol *pCnt=pContext->LookupSymbol("_C");

   Integer *C = ((PFIntegerSymbol*)pCnt)->GetValue();
   uint32 p = ((*C) & INT_MAX);

   Integer mm;
   mm=0;
   uint32 q = 0;
   primeserver->SkipTo(1);

   while ((q = (uint32) primeserver->NextPrime()) <= p)
   {
      if (q < 10) mm *= 10;
      else if (q < 100) mm *= 100;
      else if (q < 1000) mm *= 1000;
      else if (q < 10000) mm *= 10000;
      else if (q < 100000) mm *= 100000;
      else if (q < 1000000) mm *= 1000000;
      else if (q < 10000000) mm *= 10000000;
      else if (q < 100000000) mm *= 100000000;
      else return PFBoolean::b_false;

      mm += q;
   }
  
   Integer *r = new Integer(mm);
   pContext->AddSymbol(new PFIntegerSymbol("_result", r));

   return PFBoolean::b_true;
}


F_SmarandacheWellinPrime::F_SmarandacheWellinPrime()
   : F_SmarandacheWellinBase("SmWp")
{
}

PFBoolean F_SmarandacheWellinPrime::CallFunction(PFSymbolTable *pContext)
{
   IPFSymbol *pCnt=pContext->LookupSymbol("_C");

   Integer *C = ((PFIntegerSymbol*)pCnt)->GetValue();
   uint32 index = ((*C) & INT_MAX);

   Integer mm;
   mm=0;
   uint32 i, q = 0;

   for (i=1; i<=index; i++)
   {
      q = (uint32) primeserver->ByIndex(i);

      if (q < 10) mm *= 10;
      else if (q < 100) mm *= 100;
      else if (q < 1000) mm *= 1000;
      else if (q < 10000) mm *= 10000;
      else if (q < 100000) mm *= 100000;
      else if (q < 1000000) mm *= 1000000;
      else if (q < 10000000) mm *= 10000000;
      else if (q < 100000000) mm *= 100000000;
      else return PFBoolean::b_false;

      mm += q;
   }
  
   Integer *r = new Integer(mm);
   pContext->AddSymbol(new PFIntegerSymbol("_result", r));

   return PFBoolean::b_true;
}
