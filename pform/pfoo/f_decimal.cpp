#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

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
#ifdef USE_GALLOT
   , m_pDigitBuffer(NULL), m_pDigitWrite(NULL), m_dwStackPointer(0),
   m_dwRecursionDepth(0), m_pPowerBuffer(NULL), m_pRecursionBuffer(NULL), m_dwDigitBufferLength(0),
   m_bSigned(PFBoolean::b_false)
#else
   ,pDigitBuffer(NULL)
#endif
{
}

F_Decimal::F_Decimal(const F_Decimal &/*f*/)
   : PFIterativeSymbol("decimal"),
   m_pN(NULL)
#ifdef USE_GALLOT
   , m_pDigitBuffer(NULL), m_pDigitWrite(NULL), m_dwStackPointer(0),
   m_dwRecursionDepth(0), m_pPowerBuffer(NULL), m_pRecursionBuffer(NULL), m_dwDigitBufferLength(0),
   m_bSigned(PFBoolean::b_false)
#else
   ,pDigitBuffer(NULL)
#endif
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
#ifdef USE_GALLOT
   if(m_pPowerBuffer)
   {
      delete[] m_pPowerBuffer;
      m_pPowerBuffer=NULL;
   }
   if(m_pRecursionBuffer)
   {
      delete[] m_pRecursionBuffer;
      m_pRecursionBuffer=NULL;
   }
   if(m_pDigitBuffer)
   {
      delete[] m_pDigitBuffer;
      m_pDigitBuffer=NULL;
   }
#else
   if(pDigitBuffer)
   {
      delete[] pDigitBuffer;
      pDigitBuffer=NULL;
   }
#endif
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
#ifdef USE_GALLOT
   double maxError;

   DWORD d=decimalDigits(*m_pN);
   DWORD i;
   if(d==0)
   {
      d=1;
   }
   m_dwDigitBufferLength=(d+8)/9;
   m_pDigitBuffer=new DWORD[m_dwDigitBufferLength];
   m_pDigitWrite=m_pDigitBuffer;
   m_dwStackPointer=0;

   DWORD q=9;
   m_dwRecursionDepth=0;
   while(q<=d)
   {
      q<<=1;
      m_dwRecursionDepth++;
   }

   m_pPowerBuffer=new PowerBuffer[m_dwRecursionDepth+1];
   m_pRecursionBuffer=new RecursionBuffer[m_dwRecursionDepth+1];

   {
      Integer P;
      P.Ipow(5,9);      // 5^9
      for(i=0;i<m_dwRecursionDepth;i++)
      {
         m_pPowerBuffer[i].set(P);
         P.FFTsquare(0,maxError);
      }
   }

   Integer P(*m_pN);
   if(P<0)
   {
      P=-P;
      m_bSigned=PFBoolean::b_true;
   }
   else
   {
      m_bSigned=PFBoolean::b_false;
   }

   m_dwStepsTotal=0;

   for(i=m_dwRecursionDepth;i--;)
   {
   // if P has more digits than 9<<i, divide it into a recursion buffer
      DWORD dx=(9<<i);
      if(d>dx)
      {
         d-=dx;               // strip these digits off
// watch it here. We're trying to mangle it so it has the same number of bits.
// But this thing DOESN'T.
         Integer Q=P>>dx;
         P-=(Q<<dx);          // remainder of power two division
         Integer R;
         m_pPowerBuffer[i].divmod(Q,R);   // quotient and remainder after division by pow of 5
         P+=(R<<dx);          // make it power 10

         // P is the remainder, Q the quotient
         // the remainder is what gets buffered
         m_pRecursionBuffer[m_dwStackPointer++].set(P,i);
         P=Q;
         // compute the number of calls i=0 1 i=1 1 i=2 3
         if(i==0) m_dwStepsTotal++;       // just a flush step
         else m_dwStepsTotal+=((1<<i)-1); // *grins* a little more interesting
      }
   }
   m_dwStackPointer--;
   *m_pDigitWrite++=P&0xffffffff;
#else
   m_dwStepsTotal=1;
   m_dwStepGranularity=256;
   testResult=0;
#endif

   return bRetval;
}

PFBoolean F_Decimal::Iterate()
{
   PFBoolean bRetval=PFBoolean::b_false;  // not finished yet
#ifdef USE_GALLOT
   DWORD i;

   if((i=m_pRecursionBuffer[m_dwStackPointer].depth)==0)
   {
      *m_pDigitWrite++=m_pRecursionBuffer[m_dwStackPointer--].N&0xffffffff;
   }
   else
   {
      i--;        // move to the previous powerBuffer
      // high bits need to be stashed in the next rb
      m_pRecursionBuffer[m_dwStackPointer].depth=
         m_pRecursionBuffer[m_dwStackPointer+1].depth=i;

      Integer P=m_pRecursionBuffer[m_dwStackPointer].N;

      int dx=(9<<i);

      Integer Q=P>>dx;
      P-=(Q<<dx);       // power 2 remainder
      Integer R;
      m_pPowerBuffer[i].divmod(Q,R);
      P+=(R<<dx);
      // P is the remainder, Q the quotient
      // the remainder is what gets buffered
      if(i==0)
      {
         m_dwStackPointer--;     // write Q then P
         *m_pDigitWrite++=Q&0xffffffff;         // may as well squirt it
         *m_pDigitWrite++=P&0xffffffff;         // may as well squirt it
      }
      else
      {
         m_pRecursionBuffer[m_dwStackPointer].set(P,i);
         m_dwStackPointer++;
         m_pRecursionBuffer[m_dwStackPointer].set(Q,i);
      }
   }
#else
   int iSize=mpz_sizeinbase(m_pN->gmp(),10)+2;
   pDigitBuffer=new char[iSize];
   mpz_get_str(pDigitBuffer,10,m_pN->gmp());
#endif

   return bRetval;
}

PFBoolean F_Decimal::OnCompleted(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_true;
   PFString sOutput="";
#ifdef USE_GALLOT
   if(m_bSigned)
   {
      sOutput+='-';
   }
   static LPCTSTR pNineZeros="000000000";

   for(DWORD i=0;i<m_dwDigitBufferLength;i++)
   {
      PFString sDigit;
      sDigit.Set(m_pDigitBuffer[i]);
      if(i!=0)
      {
         // add some leading zeroes
         DWORD dwLength=sDigit.GetLength();
         sOutput+=(pNineZeros+dwLength);
      }
      sOutput+=sDigit;
   }
#else
   sOutput=pDigitBuffer;
#endif

   pContext->AddSymbol(new PFStringSymbol("_expansion",sOutput));
   testResult=1;

   return bRetval;
}

PFBoolean F_Decimal::OnCleanup(PFSymbolTable * /*pContext*/)
{
   Deallocate();
   return PFBoolean::b_true;
}
