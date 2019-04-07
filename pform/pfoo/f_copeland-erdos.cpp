#include "pfoopch.h"
#include <primesieve.hpp>
#include "f_smarandache-wellin.h"
#include "f_copeland-erdos.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"

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
   uint32_t n = ((*C) & INT_MAX);
   uint64_t length = 0, lastPrime;
   char   buf[10], *ptr;

   Integer mm;
   mm=0;

   std::vector<uint64_t> vPrimes;
   std::vector<uint64_t>::iterator it;

   vPrimes.clear();

   primesieve::generate_n_primes(1000000, &vPrimes);
   
   it = vPrimes.begin();

   while (it != vPrimes.end())
   {
      lastPrime = (uint32_t) *it;

      sprintf(buf, "%" PRIu64"", lastPrime);
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

      it++;

      // If we have reached the end, but our string isn't long enough,
      // so get the next group of primes
      if (it == vPrimes.end())
      {
         vPrimes.clear();

         primesieve::generate_n_primes(1000000, lastPrime + 1, &vPrimes);
   
         it = vPrimes.begin();
      }
   }
  
   Integer *r = new Integer(mm);
   pContext->AddSymbol(new PFIntegerSymbol("_result", r));

   return PFBoolean::b_true;
}
