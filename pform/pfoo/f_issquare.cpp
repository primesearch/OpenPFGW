#include "pfoopch.h"
#include "f_issquare.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "primeserver.h"

F_IsSquare::F_IsSquare()
   : PFFunctionSymbol("@issquare")
{
}

DWORD F_IsSquare::MinimumArguments() const
{
   return 1;
}

DWORD F_IsSquare::MaximumArguments() const
{
   return 1;
}

DWORD F_IsSquare::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_IsSquare::GetArgumentName(DWORD /*dwIndex*/) const
{
   return "_N";
}

PFBoolean F_IsSquare::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pSymbol=pContext->LookupSymbol("_N");
   if(pSymbol)
   {
      if(pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
      {
         Integer *q=((PFIntegerSymbol*)pSymbol)->GetValue();
         if(q)
         {
            // _result is to be set to 0 or 1
            PFBoolean bConclusive=PFBoolean::b_false;
            PFBoolean bIsSquare=PFBoolean::b_true; // assume it might be

            if(*q<0)
            {
               bConclusive=PFBoolean::b_true;
               bIsSquare=PFBoolean::b_false;    // negatives never are
            }
            else
            {
               Integer QQ(*q);                  // use QQ as scratch
               PFBoolean bScanning=PFBoolean::b_true;
               while(bScanning&&(!bConclusive)&&(QQ>=4))
               {
                  int iMask=QQ&3;
                  switch(iMask)
                  {
                  case 0:
                     QQ>>=2;
                     break;
                  case 1:
                     bScanning=PFBoolean::b_false;
                     break;
                  default:
                     bIsSquare=PFBoolean::b_false;
                     bConclusive=PFBoolean::b_true;
                     break;
                  }
               }

               if(QQ<4)
               {
                  int iMask=QQ&2;
                  bIsSquare=(iMask==0)?PFBoolean::b_true:PFBoolean::b_false;
                  bConclusive=PFBoolean::b_true;
               }
               else if(!bConclusive)
               {
                  // we don't know yet, so do some more tests
                  // using the kronecker symbol
                  int iTests=lg(QQ)+1;    // for this many bits
                  primeserver->restart();
                  uint32 p;
                  primeserver->next(p);         // forget mod 2

                  while(!bConclusive && iTests--)
                  {
                     primeserver->next(p);
                     int iKro=kro(QQ,int(p));
                     if(iKro==-1)
                     {
                        bConclusive=PFBoolean::b_true;
                        bIsSquare=PFBoolean::b_false;
                     }
                  }

                  if(!bConclusive)
                  {
                     Integer RR=squareroot(QQ);
                     QQ-=(RR*RR);
                     bIsSquare=(QQ==0)?PFBoolean::b_true:PFBoolean::b_false;
                  }
               }

               Integer *r=new Integer(bIsSquare?1:0);
                  pContext->AddSymbol(new PFIntegerSymbol("_result",r));
                  bRetval=PFBoolean::b_true;
            }
         }
      }
   }
   return bRetval;
}
