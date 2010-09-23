#include "pfoopch.h"
#include "f_repunit.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "primeserver.h"

F_Repunit::F_Repunit()
   : PFFunctionSymbol("R")
{
}

DWORD F_Repunit::MinimumArguments() const
{
   return 1;
}

DWORD F_Repunit::MaximumArguments() const
{
   return 2;
}

DWORD F_Repunit::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_Repunit::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval="";
   switch(dwIndex)
   {
   case 0:
      sRetval="_N";
      break;
   case 1:
      sRetval="_PAT"; // I know that longer things "break" the pattern, but why limit to single chars??
      break;
   default:
      break;
   }
   return sRetval;
}

PFBoolean F_Repunit::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pSymbol=pContext->LookupSymbol("_N");
   if(pSymbol)
   {
      IPFSymbol *pPattern = pContext->LookupSymbol("_PAT");
      if (pPattern)
      {
         // we will make a pattern of _PAT_PAT_PAT .... _N times
         if(pPattern->GetSymbolType()==INTEGER_SYMBOL_TYPE && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
         {
            Integer *N=((PFIntegerSymbol*)pSymbol)->GetValue();
            if(N)
            {
               int idx=(*N)&(0x7fffffff); // nothing unusual there
               if(((*N)==idx)&&(idx<10000000))
               {
                  Integer *P=((PFIntegerSymbol*)pPattern)->GetValue();
                  if(P)
                  {
                     char *szPattern = P->Itoa();
                     int nPatLen = (int) strlen(szPattern);
                     char *szTmp = new char [nPatLen*idx+1];
                     char *cp = szTmp;
                     while(idx--)
                     {
                        strcpy(cp, szPattern);
                        cp += nPatLen;
                     }
                     Integer *r=new Integer;
                     r->atoI(szTmp);
                     delete[] szPattern;
                     delete[] szTmp;
                     pContext->AddSymbol(new PFIntegerSymbol("_result",r));
                     bRetval=PFBoolean::b_true;
                  }
               }
            }
         }
      }
      else
      {
         // Default to r(#,1) which is the old redigit r(#) type.
         if(pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
         {
            Integer *q=((PFIntegerSymbol*)pSymbol)->GetValue();
            if(q)
            {
               // PFGW runs up to about 40,000,000 bits
               // so we'll stop at 10 megadigit repunits

                  int idx=(*q)&(0x7fffffff); // nothing unusual there
               if(((*q)==idx)&&(idx<10000000))
               {
                  Integer *r=new Integer(10);

                  // R has to be (10^iIndex-1)/9
                  // we could subcall Phi, but that isn't right
                  // if iIndex isn't a prime.

                  //(*r)=10;
                  (*r)=pow((*r),idx);
                  --(*r);
                  (*r)/=9;
                  pContext->AddSymbol(new PFIntegerSymbol("_result",r));
                  bRetval=PFBoolean::b_true;
               }
            }
         }
      }
   }
   return bRetval;
}
