#include "pfoopch.h"
#include <vector>
#include <primesieve.hpp>
#include "f_binomial.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"

F_Binomial::F_Binomial()
   : PFFunctionSymbol("C")
{
}

DWORD F_Binomial::MinimumArguments() const
{
   return 2;
}

DWORD F_Binomial::MaximumArguments() const
{
   return 2;
}

DWORD F_Binomial::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_Binomial::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval="";
   switch(dwIndex)
   {
   case 0:
      sRetval="_N";
      break;
   case 1:
      sRetval="_K";
      break;
   default:
      break;
   }
   return sRetval;
}

PFBoolean F_Binomial::CallFunction(PFSymbolTable *pContext)
{
   IPFSymbol *nSym=pContext->LookupSymbol("_N");
   IPFSymbol *kSym=pContext->LookupSymbol("_K");

   Integer *N=0, *K=0;

   if(nSym->GetSymbolType()==INTEGER_SYMBOL_TYPE)
      N=((PFIntegerSymbol*)nSym)->GetValue();
   if(kSym->GetSymbolType()==INTEGER_SYMBOL_TYPE)
      K=((PFIntegerSymbol*)kSym)->GetValue();

   if (!N || !K)
      return PFBoolean::b_false;
   if (*N <= 0 || *K <=0)
      return PFBoolean::b_false;
   if (*N < *K)
      return PFBoolean::b_false;
   // note the next line will allow C(2000000000,20) which is reasonable size (about 100 digits),
   // but it will also allow C(2000000000,1000000000) which is WAY out of range.  I don't know any
   // good way to validate the size of these things since there is so much variability.
   if (*N > INT_MAX | *K > INT_MAX)
      return PFBoolean::b_false;

   // NOTE this function is not optimal for C($a,$b) where a is big and b is small. In that circumstance,
   // it would proably be best to simply handle it by multiplying together the "high" numbers of $a, and
   // then dividing that number by $b!
   //
   // Chris, your comments here are welcome, here are some observatations (but this is only partly right).
   // The cut off for this type of functionality would probably be $a > 1000000 and $a / $b > 1000
   // I know that C(2000000000,20) took a long time to create, but (2000000000*1999999999*...)/20! was built
   // almost instantly.

   long n = (*N & INT_MAX);
   long k = (*K & INT_MAX);

   Integer *r = new Integer(1);

// printf ("mpz_bin_uiui\n");
// mpz_bin_uiui(*(r->gmp()), n, k);
// pContext->AddSymbol(new PFIntegerSymbol("_result",r));
// return PFBoolean::b_true;

   Integer r1;

   // all primes p no greater than n

   // This lists what the "max" int is which can be raised to this power and still not overflow a 31 bit integer.
   static int powertable[] =
   {
   // ^0   ^1     ^2    ^3   ^4  ^5  ^6  ^7  // ignore ^0 and ^1  they are handled by the if(expo) and the if(expo==1)
        0,   0, 46340, 1290, 215, 73, 35, 21,
   // ^8   ^9  ^10  ^11  ^12  ^13  ^14  ^15
       14,  10,   8,   7,   5,   5,   4,   4,
    //^16  ^17  ^18  ^19  ^20  ^21  ^22  ^23
        3,   3,   3,   3,   2,   2,   2,   2,
    //^24  ^25  ^26  ^27  ^28  ^29  ^30  ^31  // 2^31 is the highest power the a signed 31 bit int can handle.  All powers from here on would cause overflow
        3,   3,   3,   3,   2,   2,   2,   2
   };

   std::vector<int32_t> vPrimes;
   std::vector<int32_t>::iterator it;
   int32_t p;

   vPrimes.clear();

   primesieve::generate_primes(1, n, &vPrimes);
   
   it = vPrimes.begin();
   while (it != vPrimes.end())
   {
      int64_t i1 = n, i2 = n - k, i3 = k;
      int32_t expo=0;

      p = *it;
      while (i1 >= p)
      {
         i1 /= p;
         i2 /= p;
         i3 /= p;
         expo += (int32_t) (i1-i2-i3);
      }

      if (expo)
      {
         if (expo == 1)
            *r *= (int32_t) p;
         else if (expo>31 || p>powertable[expo])
         {
            // p^expo would overflow a 31 bit int.  Use GMP to handle it.  This code is RARELY if ever called.
            r1.Ipow(p,expo);  // p^expo is the exact power of p dividing B(n,k)
            *r *= r1;
#if defined (_DEBUG)
            printf ("binomial slow expo %d^%d\n", p, expo);
#endif
         }
         else
         {
            // This silly loop is actually a very fast way to perform the exponentiation for these numbers.
            int32_t _r1 = p;
            while (--expo)
               _r1 *= p;
            *r *= _r1;
         }
      }

      it++;
   }

   pContext->AddSymbol(new PFIntegerSymbol("_result",r));
   return PFBoolean::b_true;
}
