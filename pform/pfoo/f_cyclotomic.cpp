#include "pfoopch.h"
#include "f_cyclotomic.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"

#include "f_trivial.h"
#include "pffactorizationsymbol.h"
#include "factornode.h"

#include "h_helpers.h"

F_Cyclotomic::F_Cyclotomic()
   : PFFunctionSymbol("Phi")
{
}

DWORD F_Cyclotomic::MinimumArguments() const
{
   return 1;
}

DWORD F_Cyclotomic::MaximumArguments() const
{
   return 2;
}

DWORD F_Cyclotomic::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_Cyclotomic::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval="";
   switch(dwIndex)
   {
   case 0:
      sRetval="_N";
      break;
   case 1:
      sRetval="_B";
      break;
   default:
      break;
   }
   return sRetval;
}

// evaluators called by the cyclotomics
PFBoolean helpPhi(PFSymbolTable * /*pTable*/,int iIndex,Integer &X)
{
   X=iIndex;
   return PFBoolean::b_true;
}

PFBoolean helpCyclotomic(PFSymbolTable *pTable,int iIndex,Integer &X)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pBase=pTable->LookupSymbol("_B");
   if(pBase)
   {
      if(pBase->GetSymbolType()==INTEGER_SYMBOL_TYPE)
      {
         Integer *q=((PFIntegerSymbol*)pBase)->GetValue();
         if(q)
         {
            X=pow(*q,iIndex);
            --X;
            bRetval=PFBoolean::b_true;
         }
      }
   }
   return bRetval;
}

PFBoolean F_Cyclotomic::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;

   Integer *r=NULL;

   // what to do next depends if we are running in one or two parameter mode
   IPFSymbol *pBase=pContext->LookupSymbol("_B");

   if(pBase==NULL)
   {
      r=new Integer;
      bRetval=H_Mobius::Evaluate(pContext,helpPhi,*r);
   }
   else
   {
      if(pBase->GetSymbolType()==INTEGER_SYMBOL_TYPE)
      {
         Integer *B = ((PFIntegerSymbol*)pBase)->GetValue();
         if (B && (*B) == 1)
         {
            // if N the power of a prime, result is this prime, otherwise result is 1
            PFSymbolTable *pSubContext=new PFSymbolTable(pContext);
            int iTrivial=PFFunctionSymbol::CallSubroutine("@trivial",pSubContext);

            IPFSymbol *pFactor=pSubContext->LookupSymbol("_TRIVIALFACTOR");

            // Use the Mobius inversion formula.
            r=new Integer;
            *r = 1;
            switch(iTrivial)
            {
               case TT_ZERO:
               case TT_ONE:
                  break;
               default:
               {
                  PFFactorizationSymbol *pp=(PFFactorizationSymbol *)pFactor;
                  // multiply through by (p-1)p^(a-1)
                  PFList<FactorNode> *pList=pp->AccessList();
                  DWORD dwFactors=pList->GetSize();
                  if (dwFactors == 1)
                  {
                     PFForwardIterator pffi;
                     pList->StartIterator(pffi);
                     PFListNode *n;
                     pffi.Iterate(n);
                     FactorNode *fn=(FactorNode*)n->GetData();
                     *r=fn->prime();
                  }
               }
            }
            delete pSubContext;
            bRetval = PFBoolean::b_true;
         }
         else if (B && (*B) != -1)
         {
            r=new Integer;
            bRetval=H_Primitive::Evaluate(pContext,helpCyclotomic,*r);
         }
      }
   }

   if(r)
   {
      if(!bRetval)
      {
         delete r;
      }
      else
      {
         pContext->AddSymbol(new PFIntegerSymbol("_result",r));
      }
   }
   return bRetval;
}
