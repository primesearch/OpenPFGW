#include "pfoopch.h"
#include "f_prevprime.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "primeserver.h"

F_PrevPrime::F_PrevPrime()
   : PFFunctionSymbol("PrevPrime")
{
   m_pPrimeServer = 0;
}

F_PrevPrime::~F_PrevPrime()
{
   if (m_pPrimeServer)
   {
      delete m_pPrimeServer;
      m_pPrimeServer = 0;
   }
}

DWORD F_PrevPrime::MinimumArguments() const
{
   return 1;
}

DWORD F_PrevPrime::MaximumArguments() const
{
   return 1;
}

DWORD F_PrevPrime::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_PrevPrime::GetArgumentName(DWORD /*dwIndex*/) const
{
   return "_N";
}

PFBoolean F_PrevPrime::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pSymbol=pContext->LookupSymbol("_N");
 
   if (!m_pPrimeServer)
      m_pPrimeServer = new PrimeServer(1e12 + 10000);

   if (!pSymbol) return bRetval;

   if (pSymbol->GetSymbolType()!=INTEGER_SYMBOL_TYPE)  return bRetval;

   Integer *q=((PFIntegerSymbol*)pSymbol)->GetValue();

   if (!q) return bRetval;

   uint64 last=(*q)&(ULLONG_MAX);

   Integer *r=new Integer;
   bRetval=PFBoolean::b_true;

   // This should be faster than using the Integer::prevprime function
   if (numbits(*q) < 60 && last < (uint64) 1e12)
   {
      *r = m_pPrimeServer->PrevPrime(last);
   }
   else
   {
      *r = q->prevprime();
   }

   pContext->AddSymbol(new PFIntegerSymbol("_result",r));

   return bRetval;
}
