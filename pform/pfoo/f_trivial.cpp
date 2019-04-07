#include "pfoopch.h"
#include <vector>
#include <primesieve.hpp>
#include "f_trivial.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "pffactorizationsymbol.h"

#include "factornode.h"

F_Trivial::F_Trivial()
   : PFFunctionSymbol("@trivial")
{
}

F_Trivial::~F_Trivial()
{
}

DWORD F_Trivial::MinimumArguments() const
{
   return 1;
}

DWORD F_Trivial::MaximumArguments() const
{
   return 1;
}

DWORD F_Trivial::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_Trivial::GetArgumentName(DWORD /*dwIndex*/) const
{
   return "_N";
}

PFBoolean F_Trivial::CallFunction(PFSymbolTable *pContext)
{
   int iResult=TT_COMPLETED;

   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pSymbol=pContext->LookupSymbol("_N");
   if(pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      bRetval=PFBoolean::b_true;
      PFBoolean bNeg=PFBoolean::b_false;
      PFFactorizationSymbol *pFactorization=new PFFactorizationSymbol("_TRIVIALFACTOR");

      Integer *pN=((PFIntegerSymbol*)pSymbol)->GetValue();

      if ((*pN)<0)
      {
         bNeg=PFBoolean::b_true;
         (*pN)*=-1;
         iResult=TT_NEGATIVE;
      }

      // now find out if the number is small. Remember numbits() is greatest power of 2 no greater than N
      if ((*pN)<3)
      {
         pFactorization->AddFactor(new FactorNode((*pN),1));
         // Zero, one, two
         iResult = ((*pN) & INT_MAX);
      }
      else if (numbits(*pN) < 40)
      {
         // If we do NOT dip into this code for numbers less than 2^31, then the
         // V() and Phi() functions start failing!!!  We need to check into this!!!
         uint64_t p, sqrtN;
         uint64_t rawN = ((*pN) & ULLONG_MAX);

         sqrtN = (uint64_t) sqrt((double) rawN);
         std::vector<uint64_t> vPrimes;
         std::vector<uint64_t>::iterator it;

         vPrimes.clear();

         // This should be large enough for trivial factors
         primesieve::generate_primes(1, 1000000, &vPrimes);

         it = vPrimes.begin();
         while (it != vPrimes.end())
         {
            p = *it;

            if (p > sqrtN)
            {
               pFactorization->AddFactor(new FactorNode(Integer(rawN), 1));
               rawN = 1;
            }
            else
            {
               int iPower = 0;
               while ((rawN%p) == 0)
               {
                  iPower++;
                  rawN /= (int)p;
               }
               if (iPower>0)
               {
                  sqrtN = (uint64_t)sqrt((double)rawN);
                  pFactorization->AddFactor(new FactorNode(Integer(p), iPower));
               }
            }

            if (rawN == 1)
               break;

            it++;
         }
         

         iResult=TT_FACTOR;
      }  // endif we have a small number
      else
      {
         iResult=TT_COMPLETED;
      }

      pContext->AddSymbol(new PFIntegerSymbol("_TRIVIALNEG",new Integer(bNeg?1:0)));
      pContext->AddSymbol(pFactorization);
      pContext->AddSymbol(new PFIntegerSymbol("_result",new Integer(iResult)));
   }
   return bRetval;
}
