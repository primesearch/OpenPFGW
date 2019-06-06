#include "pfoopch.h"
#include <vector>
#include <primesieve.hpp>
#include "f_prime.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"

F_Prime::F_Prime()
   : PFFunctionSymbol("p")
{
}

F_Prime::~F_Prime()
{
}

DWORD F_Prime::MinimumArguments() const
{
   return 1;
}

DWORD F_Prime::MaximumArguments() const
{
   return 1;
}

DWORD F_Prime::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_Prime::GetArgumentName(DWORD /*dwIndex*/) const
{
   return "_N";
}

PFBoolean F_Prime::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval = PFBoolean::b_false;
   IPFSymbol *pSymbol = pContext->LookupSymbol("_N");

   if (!pSymbol) return bRetval;

   if (pSymbol->GetSymbolType() != INTEGER_SYMBOL_TYPE)  return bRetval;

   Integer *q = ((PFIntegerSymbol*)pSymbol)->GetValue();

   if (!q) return bRetval;

   uint64_t idx = ((*q) & (uint64_t)ULLONG_MAX); // nothing unusual there

   if ((*q) != idx)
      return bRetval;

   bRetval = PFBoolean::b_true;
   Integer *r = new Integer;

   *r = primesieve::nth_prime(idx);

   pContext->AddSymbol(new PFIntegerSymbol("_result", r));

   return bRetval;
}
