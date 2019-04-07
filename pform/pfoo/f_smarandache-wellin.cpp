#include "pfoopch.h"
#include <vector>
#include <primesieve.hpp>
#include "f_smarandache-wellin.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"

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
   uint32_t p = ((*C) & INT_MAX);

   Integer mm;
   mm=0;
   uint32_t q = 0;
   
   std::vector<uint64_t> vPrimes;
   std::vector<uint64_t>::iterator it;
   
   vPrimes.clear();

   primesieve::generate_primes(1, p, &vPrimes);
   
   it = vPrimes.begin();
   while (it != vPrimes.end())
   {
      q = (uint32_t) *it;

      if (q < 10) mm *= 10;
      else if (q < 100) mm *= 100;
      else if (q < 1000) mm *= 1000;
      else if (q < 10000) mm *= 10000;
      else if (q < 100000) mm *= 100000;
      else if (q < 1000000) mm *= 1000000;
      else if (q < 10000000) mm *= 10000000;
      else if (q < 100000000) mm *= 100000000;
      else if (q < 1000000000) mm *= 1000000000;
      else return PFBoolean::b_false;

      mm += q;
      it++;
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
   uint32_t index = ((*C) & INT_MAX);

   Integer mm;
   mm=0;
   uint32_t q = 0;
   
   std::vector<uint64_t> vPrimes;
   std::vector<uint64_t>::iterator it;
   
   vPrimes.clear();

   primesieve::generate_n_primes(index, 1, &vPrimes);

   it = vPrimes.begin();
   while (it != vPrimes.end())
   {
      q = (uint32_t) *it;

      if (q < 10) mm *= 10;
      else if (q < 100) mm *= 100;
      else if (q < 1000) mm *= 1000;
      else if (q < 10000) mm *= 10000;
      else if (q < 100000) mm *= 100000;
      else if (q < 1000000) mm *= 1000000;
      else if (q < 10000000) mm *= 10000000;
      else if (q < 100000000) mm *= 100000000;
      else if (q < 1000000000) mm *= 1000000000;
      else return PFBoolean::b_false;
      
      it++;
      mm += q;
   }
  
   Integer *r = new Integer(mm);
   pContext->AddSymbol(new PFIntegerSymbol("_result", r));

   return PFBoolean::b_true;
}
