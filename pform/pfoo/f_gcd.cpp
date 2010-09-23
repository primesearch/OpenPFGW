#include "pfoopch.h"
#include "f_gcd.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"

F_GCD::F_GCD()
   : PFFunctionSymbol("Gcd")
{
}

DWORD F_GCD::MinimumArguments() const
{
   return 2;
}

DWORD F_GCD::MaximumArguments() const
{
   return 2;
}

DWORD F_GCD::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_GCD::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval="";
   switch(dwIndex)
   {
   case 0:
      sRetval="_P";
      break;
   case 1:
      sRetval="_Q";
      break;
   default:
      break;
   }
   return sRetval;
}

PFBoolean F_GCD::CallFunction(PFSymbolTable *pContext)
{
   IPFSymbol *qSym=pContext->LookupSymbol("_Q");
   IPFSymbol *pSym=pContext->LookupSymbol("_P");

   Integer *p=0, *q=0;

   if(pSym->GetSymbolType()==INTEGER_SYMBOL_TYPE)
      p=((PFIntegerSymbol*)pSym)->GetValue();
   if(qSym->GetSymbolType()==INTEGER_SYMBOL_TYPE)
      q=((PFIntegerSymbol*)qSym)->GetValue();

   if (!q || !p)
      return PFBoolean::b_false;

    Integer *r = new Integer;
   *r=gcd(*p,*q);
   pContext->AddSymbol(new PFIntegerSymbol("_result",r));
   return PFBoolean::b_true;
}
