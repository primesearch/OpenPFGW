#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "pfoopch.h"
#include "factornode.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "pffactorizationsymbol.h"

PFFactorizationSymbol::PFFactorizationSymbol(const PFString &sName)
   : IPFSymbol(sName), m_lFactors(PFBoolean::b_true)
{
}

PFString PFFactorizationSymbol::GetStringValue()
{
   PFString sOut;

   PFIntegerSymbol symDummy("dummy",NULL);

   PFForwardIterator pffi;
   m_lFactors.StartIterator(pffi);
   PFListNode *pNode;

   while(pffi.Iterate(pNode))
   {
      FactorNode *pFactor=(FactorNode*)pNode->GetData();
      if(!sOut.IsEmpty())
      {
         sOut+='*';
      }

      DWORD dwPower=pFactor->powval();
      Integer sInteger=pFactor->prime();

      symDummy.SetValue(&pFactor->prime());
      PFString sValue=symDummy.GetStringValue();

      sOut+=sValue;
      if(dwPower>1)
      {
         sValue.Set(dwPower);
         sOut+='^';
         sOut+=sValue;
      }
   }

   symDummy.SetValue(NULL);
   if(sOut.IsEmpty())
   {
      sOut="1";
   }
   return sOut;
}

void PFFactorizationSymbol::AddFactor(FactorNode *pFactor)
{
   m_lFactors.AddTail(pFactor);
}

DWORD PFFactorizationSymbol::GetSymbolType() const
{
   return FACTORIZATION_SYMBOL_TYPE;
}

PFList<FactorNode> *PFFactorizationSymbol::AccessList()
{
   return &m_lFactors;
}


