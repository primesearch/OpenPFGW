#include "pfoopch.h"
#include "symboltypes.h"
#include "pffunctionsymbol.h"
#include "pfintegersymbol.h"

#include "f_prime.h"
#include "f_nextprime.h"
#include "f_prevprime.h"
#include "f_fibonacci.h"
#include "f_repunit.h"
#include "f_cyclotomic.h"
#include "f_gcd.h"
#include "f_binomial.h"

#include "f_trivial.h"
#include "f_factor.h"
#include "f_vector.h"
#include "f_issquare.h"
#include "f_smarandache.h"
#include "f_smarandache-wellin.h"
#include "f_copeland-erdos.h"
#include "f_sequence.h"

#include "tests.h"
#include "nminus1exponentiator.h"
#include "nplus1exponentiator.h"

#include "pfsamplersymbol.h"

#include "f_endminus1.h"
#include "f_endplus1.h"

#include "f_if.h"
#include "f_length.h"

extern int g_CompositeAthenticationLevel;
extern bool g_bHaveFatalError;

PFFunctionSymbol::PFFunctionSymbol(const PFString &sName)
   : IPFSymbol(sName)
{
}

PFString PFFunctionSymbol::GetStringValue()
{
   return GetKey();
}

DWORD PFFunctionSymbol::GetSymbolType() const
{
   return FUNCTION_SYMBOL_TYPE;
}

void PFFunctionSymbol::ClearPersistentData()
{
}

// CallSubroutine
// Calls another routine as installed into the symbol table.
// Note the return value is the value of _result, or -1 if the
// host console forces a premature exit (to be implemented later).
// The premature exit code (and the ability to store the subroutine
// stack in a restart file) is to be implemented later
int PFFunctionSymbol::CallSubroutine(const PFString &sRoutineName,PFSymbolTable *pContext)
{
   int iRetval=-1;
   int saveFFTSize;
   PFBoolean bRetval;

   IPFSymbol *pSymbol=pContext->LookupSymbol(sRoutineName);
   if(pSymbol && pSymbol->GetSymbolType()==FUNCTION_SYMBOL_TYPE)
   {
      PFFunctionSymbol *pF=(PFFunctionSymbol*)pSymbol;

      saveFFTSize = g_CompositeAthenticationLevel;
      g_bHaveFatalError = false;
      do
      {
         bRetval=pF->CallFunction(pContext);
         g_CompositeAthenticationLevel++;
      } while (g_bHaveFatalError);

      g_CompositeAthenticationLevel = saveFFTSize;

      if (bRetval)
      {
         iRetval=0;     // well, the code ran....
         // check for a return value
         pSymbol=pContext->LookupSymbol("_result");
         if(pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
         {
            PFIntegerSymbol *pI=(PFIntegerSymbol*)pSymbol;
            Integer *g=pI->GetValue();
            iRetval = ((*g) & INT_MAX);      // All functions return positive integers!
         }
      }

   }
   else
   {
      PFPrintfLog("*** WARNING *** Illegal internal functioncall to %s\n",LPCTSTR(sRoutineName));
   }

   return iRetval;
}

void PFFunctionSymbol::LoadExprFunctions(PFSymbolTable *psymRuntime)
{
   psymRuntime->AddSymbol(new F_NextPrime);
   psymRuntime->AddSymbol(new F_PrevPrime);
   psymRuntime->AddSymbol(new F_Prime);
   psymRuntime->AddSymbol(new F_Fibonacci_U);
   psymRuntime->AddSymbol(new F_Fibonacci_V);
   psymRuntime->AddSymbol(new F_Fibonacci_F);
   psymRuntime->AddSymbol(new F_Fibonacci_L);
   psymRuntime->AddSymbol(new F_Repunit);
   psymRuntime->AddSymbol(new F_Cyclotomic);
   psymRuntime->AddSymbol(new F_GCD);
   psymRuntime->AddSymbol(new F_Binomial);
   psymRuntime->AddSymbol(new F_Smarandache);
   psymRuntime->AddSymbol(new F_Smarandache_r);
   psymRuntime->AddSymbol(new F_SmarandacheWellin);
   psymRuntime->AddSymbol(new F_SmarandacheWellinPrime);
   psymRuntime->AddSymbol(new F_CopelandErdos);
   psymRuntime->AddSymbol(new F_Sequence);
   psymRuntime->AddSymbol(new F_LucasV);
   psymRuntime->AddSymbol(new F_LucasU);
   psymRuntime->AddSymbol(new F_PrimV);
   psymRuntime->AddSymbol(new F_PrimU);
   psymRuntime->AddSymbol(new F_NSW_S);
   psymRuntime->AddSymbol(new F_NSW_W);
   psymRuntime->AddSymbol(new F_IF);
   psymRuntime->AddSymbol(new F_Length);
}

void PFFunctionSymbol::LoadAllFunctions(PFSymbolTable *psymRuntime)
{
   LoadExprFunctions(psymRuntime);

   psymRuntime->AddSymbol(new T_Pocklington);
   psymRuntime->AddSymbol(new T_Morrison);
   psymRuntime->AddSymbol(new T_Combined);

   psymRuntime->AddSymbol(new F_Factor);
   psymRuntime->AddSymbol(new F_Vector);

   psymRuntime->AddSymbol(new F_Trivial);
   psymRuntime->AddSymbol(new F_IsSquare);

   psymRuntime->AddSymbol(new NMinus1Exponentiator);
   psymRuntime->AddSymbol(new NPlus1Exponentiator);

   psymRuntime->AddSymbol(new F_EndMinus1);
   psymRuntime->AddSymbol(new F_EndPlus1);
}
