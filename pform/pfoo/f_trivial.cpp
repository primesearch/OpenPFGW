#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "pfoopch.h"
#include "f_trivial.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "pffactorizationsymbol.h"

#include "factornode.h"
#include "primeserver.h"

F_Trivial::F_Trivial()
   : PFFunctionSymbol("@trivial")
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
      // default to 31 bit trivial factor depth. This depth is needed for mobius and primative factoring, but for
      // "normal" testing, trivial factoring this deep is actually way too much, and slows things down a lot. For
      // "normal" testing, 2^6 is sufficient (GWIntegers may have problems below this size).
      int nDepth=31;
      pSymbol=pContext->LookupSymbol("_trivial_depth");
      if (pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
      {
         PFIntegerSymbol *pI=(PFIntegerSymbol*)pSymbol;
         Integer *g=pI->GetValue();
         nDepth=(*g)&0x1F;
         if (nDepth < 6)
            nDepth = 6;
      }
      if((*pN)<0)
      {
         bNeg=PFBoolean::b_true;
         (*pN)*=-1;
         iResult=TT_NEGATIVE;
      }

      // now find out if the number is small. Remember lg() is greatest power of 2 no
      // greater than N
      if((*pN)<3)
      {
         pFactorization->AddFactor(new FactorNode((*pN),1));
         // Zero, one, two
         iResult=((*pN)&0x7fffffff);
      }
      else if(lg(*pN)<nDepth)    // Why not ? We can handle it!
                                  //  because it is slow as hell!  Possibly it will be faster with a
                           //  better prime generator, but for now it is SLOW.
                           //
                           // If we do NOT dip into this code for numbers less than 2^31, then the
                           // V() and Phi() functions start failing!!!  We need to check into this!!!
      {
         int dwFactor=(*pN)&0x7FFFFFFF;

         primeserver->restart();
         uint32 p;

          for(primeserver->next(p); dwFactor!=1 && p<65536; primeserver->next(p))
         {
            if(p*p>uint32(dwFactor))
            {
               pFactorization->AddFactor(new FactorNode(Integer(int(dwFactor)),1));
               dwFactor=1;
            }
            else
            {
               int iPower=0;
               while((dwFactor%p)==0)
               {
                  iPower++;
                  dwFactor/=p;
               }
               if(iPower>0)
               {
                  pFactorization->AddFactor(new FactorNode(Integer(int(p)),iPower));
               }
            }
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
