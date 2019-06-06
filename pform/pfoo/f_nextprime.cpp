#include "pfoopch.h"
#include <vector>
#include <primesieve.hpp>
#include "f_nextprime.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"

F_NextPrime::F_NextPrime()
   : PFFunctionSymbol("NextPrime")
{
}

F_NextPrime::~F_NextPrime()
{
}

DWORD F_NextPrime::MinimumArguments() const
{
   return 1;
}

DWORD F_NextPrime::MaximumArguments() const
{
   return 1;
}

DWORD F_NextPrime::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_NextPrime::GetArgumentName(DWORD /*dwIndex*/) const
{
   return "_N";
}

PFBoolean F_NextPrime::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval = PFBoolean::b_false;
   IPFSymbol *pSymbol = pContext->LookupSymbol("_N");

   if (!pSymbol) return bRetval;

   if (pSymbol->GetSymbolType() != INTEGER_SYMBOL_TYPE)  return bRetval;

   Integer *q = ((PFIntegerSymbol*)pSymbol)->GetValue();

   if (!q) return bRetval;

   uint64_t last = (*q)&((uint64_t)ULLONG_MAX);

   Integer *r = new Integer;
   bRetval = PFBoolean::b_true;

   *r = primesieve::nth_prime(1, last);

   pContext->AddSymbol(new PFIntegerSymbol("_result", r));

   return bRetval;
}
