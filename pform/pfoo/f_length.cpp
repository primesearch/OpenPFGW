#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "pfoopch.h"
#include "f_length.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "primeserver.h"

// f_length is "kinda" like lg, ln and log or log(n,b).  It is basically ceiling(log(n))

// the default is base10 or ceil(log(n))

F_Length::F_Length()
   : PFFunctionSymbol("LEN")
{
}

DWORD F_Length::MinimumArguments() const
{
   return 1;
}

DWORD F_Length::MaximumArguments() const
{
   return 2;
}

DWORD F_Length::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_Length::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval="";
   switch(dwIndex)
   {
   case 0:
      sRetval="_N";
      break;
   case 1:
      // _BASE can NOT be used!!!! this causes the -b command line switch to start failing!!!!!!
      //sRetval="_BASE"; // I know that longer things "break" the pattern, but why limit to single chars??
      sRetval = "_B";
      break;
   default:
      break;
   }
   return sRetval;
}

PFBoolean F_Length::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pSymbol=pContext->LookupSymbol("_N");

   if(pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      Integer *N=((PFIntegerSymbol*)pSymbol)->GetValue();
      if (N)
      {
         IPFSymbol *pBase = pContext->LookupSymbol("_B");
         int base = 10;
         if (pBase && pBase->GetSymbolType()==INTEGER_SYMBOL_TYPE)
         {
            Integer *B=((PFIntegerSymbol*)pBase)->GetValue();
            if(B)
            {
               base=(*B)&(0x7fffffff);
               if(((*B)!=base))
                  base = 10;
            }
         }
         char *szIntegerInBaseN = N->Itoa(base);
         int len = strlen(szIntegerInBaseN);
         if (*szIntegerInBaseN == '-')
            len--;   // remove the negative sign
         else if (*szIntegerInBaseN == '0' && len == 1)
            len = 0;
         Integer *r=new Integer(len);
         delete[] szIntegerInBaseN;
         pContext->AddSymbol(new PFIntegerSymbol("_result",r));
         bRetval=PFBoolean::b_true;
      }
   }
   return bRetval;
}
