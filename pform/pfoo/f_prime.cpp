#include "pfoopch.h"
#include "f_prime.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "primeserver.h"

F_Prime::F_Prime()
   : PFFunctionSymbol("p")
{
   m_pPrimeServer = 0;
}

F_Prime::~F_Prime()
{
   if (m_pPrimeServer) delete m_pPrimeServer;
}

DWORD F_Prime::MinimumArguments() const
{
   return 1;
}

DWORD F_Prime::MaximumArguments() const
{
   return 1;
}

DWORD F_Prime::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_Prime::GetArgumentName(DWORD /*dwIndex*/) const
{
   return "_N";
}

PFBoolean F_Prime::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pSymbol=pContext->LookupSymbol("_N");
   
   if (!m_pPrimeServer)
      m_pPrimeServer = new PrimeServer(1e15);

   if (pSymbol) return bRetval;

   if (pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)  return bRetval;

   Integer *q=((PFIntegerSymbol*)pSymbol)->GetValue();

   if (!q) return bRetval;

   uint64 idx = ((*q) & ULLONG_MAX); // nothing unusual there

   if ((*q)!=idx)
      return bRetval;

   bRetval=PFBoolean::b_true;
   Integer *r=new Integer;

   *r = m_pPrimeServer->ByIndex(idx);

   pContext->AddSymbol(new PFIntegerSymbol("_result",r));

   return bRetval;
}
