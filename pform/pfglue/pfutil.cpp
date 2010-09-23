#include "../pfglue/pfgluepch.h"
#include "../pfglue/pfutil.h"
#include "../pfglue/asmimport.h"

DWORD decimalDigits(const Integer &N)
{
   if(N==0) return(0);
// 28738/8651  - big (so reciprocal is too small)
// 42039/12655 - small (so reciprocal is too big)
   DWORD d1=0,d2;
   Integer X(N);
   DWORD l;
   DWORD d=0;  // extra digits

   if(X<0)
   {
      X=-X;
   }

   while(1)
   {
      l=lg(X);    // between 2^l and 2^(l+1)
      if(l<32)    // l will fit in a 32 bit integer
      {
         l=X&0xffffffff;
         d1=0;    // d1 calculates floor(log10(l))
         l/=10;
         while(l>0)
         {
            l/=10;
            d1++;
         }
         break;
      }
      decimalEstimate(l,d1,d2);
      if(d1==d2) break; // integer part is exact
      // calculation was inexact, so downshift and retry
      X/=100000000;     // 1e8
      d+=8;
   }
   return(d1+d+1);
}
