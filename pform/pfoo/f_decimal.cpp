#include "pfoopch.h"
#include "f_decimal.h"
#include "powerbuffer.h"
#include "recursionbuffer.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "pfstringsymbol.h"

F_Decimal::F_Decimal()
   : PFIterativeSymbol("decimal"),
   m_pN(NULL)
   ,pDigitBuffer(NULL)
{
}

F_Decimal::F_Decimal(const F_Decimal &/*f*/)
   : PFIterativeSymbol("decimal"),
   m_pN(NULL)
   ,pDigitBuffer(NULL)
{
}

F_Decimal &F_Decimal::operator=(const F_Decimal &/*f*/)
{
   Deallocate();
   return *this;
}

F_Decimal::~F_Decimal()
{
   Deallocate();
}

void F_Decimal::Deallocate()
{
   if(pDigitBuffer)
   {
      delete[] pDigitBuffer;
      pDigitBuffer=NULL;
   }
}

DWORD F_Decimal::MinimumArguments() const
{
   return 1;
}

DWORD F_Decimal::MaximumArguments() const
{
   return 2;
}

DWORD F_Decimal::GetArgumentType(DWORD dwIndex) const
{
   DWORD dwRetval;
   switch(dwIndex)
   {
   case 0:
      dwRetval=INTEGER_SYMBOL_TYPE;
      break;
   case 1:
      dwRetval=STRING_SYMBOL_TYPE;
      break;
   default:
      dwRetval=0;
      break;
   }
   return dwRetval;
}

PFString F_Decimal::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval;
   switch(dwIndex)
   {
   case 0:
      sRetval="_N";
      break;
   case 1:
      sRetval="_X";
      break;
   default:
      sRetval="";
      break;
   }
   return sRetval;
}

PFBoolean F_Decimal::OnExecute(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;

   IPFSymbol *pSymbol=pContext->LookupSymbol("_N");
   if(pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      m_pN=((PFIntegerSymbol*)pSymbol)->GetValue();
      bRetval=PFBoolean::b_true;
   }
   return bRetval;
}

PFBoolean F_Decimal::OnInitialize()
{
   PFBoolean bRetval=PFBoolean::b_true;

   m_dwStepsTotal=1;
   m_dwStepGranularity=256;
   testResult=0;

   return bRetval;
}

PFBoolean F_Decimal::Iterate()
{
   PFBoolean bRetval=PFBoolean::b_false;  // not finished yet

   int iSize = (int) mpz_sizeinbase(m_pN->gmp(),10)+2;
   pDigitBuffer=new char[iSize];
   mpz_get_str(pDigitBuffer,10,m_pN->gmp());

   return bRetval;
}

PFBoolean F_Decimal::OnCompleted(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_true;
   PFString sOutput="";

   sOutput=pDigitBuffer;

   pContext->AddSymbol(new PFStringSymbol("_expansion",sOutput));
   testResult=1;

   return bRetval;
}

PFBoolean F_Decimal::OnCleanup(PFSymbolTable * /*pContext*/)
{
   Deallocate();
   return PFBoolean::b_true;
}
