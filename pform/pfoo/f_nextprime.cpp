#include "pfoopch.h"
#include "f_nextprime.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "primeserver.h"

F_NextPrime::F_NextPrime()
   : PFFunctionSymbol("NextPrime")
{
   m_pPrimeServer = 0;
}

F_NextPrime::~F_NextPrime()
{
   if (m_pPrimeServer) delete m_pPrimeServer;
}

DWORD F_NextPrime::MinimumArguments() const
{
   return 1;
}

DWORD F_NextPrime::MaximumArguments() const
{
   return 1;
}

DWORD F_NextPrime::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_NextPrime::GetArgumentName(DWORD /*dwIndex*/) const
{
   return "_N";
}

PFBoolean F_NextPrime::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pSymbol=pContext->LookupSymbol("_N");
   
   if (!m_pPrimeServer)
      m_pPrimeServer = new PrimeServer();

   if (pSymbol) return bRetval;

   if (pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)  return bRetval;

   Integer *q=((PFIntegerSymbol*)pSymbol)->GetValue();

   if (!q) return bRetval;

   uint64 last=(*q)&(ULLONG_MAX); // nothing unusual there

   if ((*q)!=last)
      return bRetval;

   Integer *r=new Integer;
   bRetval=PFBoolean::b_true;

   primeserver->SkipTo(last);
   *r = primeserver->NextPrime();

   pContext->AddSymbol(new PFIntegerSymbol("_result",r));

   return bRetval;
}
