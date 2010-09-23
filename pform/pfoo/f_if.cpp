#include "pfoopch.h"
#include "f_if.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"

F_IF::F_IF()
   : PFFunctionSymbol("If")
{
}

DWORD F_IF::MinimumArguments() const
{
   return 3;
}

DWORD F_IF::MaximumArguments() const
{
   return 3;
}

DWORD F_IF::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_IF::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval="";
   switch(dwIndex)
   {
   case 0:
      sRetval="_C";
      break;
   case 1:
      sRetval="_T";
      break;
   case 2:
      sRetval="_F";
      break;
   default:
      break;
   }
   return sRetval;
}

PFBoolean F_IF::CallFunction(PFSymbolTable *pContext)
{
   IPFSymbol *cSym=pContext->LookupSymbol("_C");
   IPFSymbol *tSym=pContext->LookupSymbol("_T");
   IPFSymbol *fSym=pContext->LookupSymbol("_F");

   Integer *c=0, *t=0, *f=0;

   if(cSym->GetSymbolType()==INTEGER_SYMBOL_TYPE)
      c=((PFIntegerSymbol*)cSym)->GetValue();
   if(tSym->GetSymbolType()==INTEGER_SYMBOL_TYPE)
      t=((PFIntegerSymbol*)tSym)->GetValue();
   if(fSym->GetSymbolType()==INTEGER_SYMBOL_TYPE)
      f=((PFIntegerSymbol*)fSym)->GetValue();

   if (!c || !t || !f)
      return PFBoolean::b_false;

    Integer *r = new Integer;

   if (*c!=0) {
      // Allow a "C" like syntax, where 0 is false, and anything NON-zero is true.
//    if (*c!=1)
//       return PFBoolean::b_false;
      *r=*t;
   } else {
      *r=*f;
   }

   pContext->AddSymbol(new PFIntegerSymbol("_result",r));
   return PFBoolean::b_true;
}
