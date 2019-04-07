#include "pfoopch.h"
#include "nminus1exponentiator.h"
#include "factornode.h"
#include "factorarray.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "pfsamplersymbol.h"
#include "testreturns.h"

NMinus1Exponentiator::NMinus1Exponentiator()
    : Exponentiator("nminus1")/*, m_base(base), pCertificate(pCert)*/
{
// pCertificate=new CertificateFile(((FieldZModulus*)pM)->asInteger(),"N-1");
}

PFBoolean NMinus1Exponentiator::GetTotalExponentiation(PFSymbolTable * /*pContext*/,Integer &X)
{
   X=m_N-1;
   return PFBoolean::b_true;
}

FiniteField *NMinus1Exponentiator::GetFieldElements(PFSymbolTable *pContext,Residue *&res,Multiplier *&mul)
{
   FieldZ *pF=NULL;

   PFSamplerSymbol *pSampler=(PFSamplerSymbol*)pContext->LookupSymbol("_SAMPLER");

   uint32_t p=pSampler->ask(m_N);

   // Officially, we don't need to do this kronecker test if the factor set
   // doesn't include 2. If we avoid it, we can often select a smaller proving
   // base, which runs faster in the main routines.
   while(kro(Integer(int(p)),m_N)!=-1)
   {
      p=pSampler->askagain();
   }
   pSampler->accept(p);

   PFPrintfClearCurLine();
   PFOutput::EnableOneLineForceScreenOutput();
   // I think these need to be in the pfgw.out file!!
// PFPrintfStderr("Running N-1 test using base %u\n",p);
// PFfflush(stderr);
   PFPrintfLog("Running N-1 test using base %u\n",p);

   // create field modulus residue multiplier

   pF=new FieldZ(&m_N);

   // residue, multiplier
   Integer P((int)p);
   mul=pF->createCompatibleMultiplier(P);
   res=pF->createCompatibleResidue(P);

   testResult=PT_INCONCLUSIVE;

   return pF;
}

NMinus1Exponentiator::~NMinus1Exponentiator()
{
// delete pCertificate;
}

PFBoolean NMinus1Exponentiator::testInternal()
{
    return PFBoolean::b_false;         // there are no tests for internal nodes
}


PFBoolean NMinus1Exponentiator::testLeaf()
{
   // for leaf nodes, perform the following test
   // residue is zero - whoops, how did we miss that? Game over, man
   // residue is one - can't determine a thing, stick it on the next heap
   // residue is not one, subtract 1, get gcd with N.
   // gcd not one - then it's composite (durr)
   // gcd one - then we're ok

    PFBoolean bRetval=PFBoolean::b_false;             // whether we need a premature exit

   IntegerOutputResidue *pOut=(IntegerOutputResidue*)pResidue->collapse();
   Integer &X=pOut->content();

   if(X==0)
   {
      testResult=PT_FACTOR;
      bRetval=PFBoolean::b_true;
   }
   else if(X==1)
   {
      AddToJunkyard(pDestination);
   }
   else
   {
      // How do we get N in here?

      --X;    // subtract 1

      Integer G=gcd(X,m_N);

      if(G!=1)
      {
         PFIntegerSymbol sDummy("dummy",NULL);
         sDummy.SetValue(&G);
         PFString sG=sDummy.GetStringValue();
         sDummy.SetValue(NULL);

         PFPrintfLog("Factored: %s\n",LPCTSTR(sG));

         testResult=PT_FACTOR;   // known factor
         bRetval=PFBoolean::b_true;
      }
      else
      {
         AddToResults(pDestination);
      }
   }

   ShowPrompt();
   delete pOut;
   return bRetval;
}

PFBoolean NMinus1Exponentiator::testFinal()
{
   PFBoolean bRetval=PFBoolean::b_false;

   IntegerOutputResidue *pOut=(IntegerOutputResidue*)pResidue->collapse();
   Integer &X=pOut->content();

   if(X==0)
   {
      testResult=PT_FACTOR;
      bRetval=PFBoolean::b_true;
   }
   else if(X==1)
   {
   }
   else
   {
      testResult=PT_COMPOSITE;       // can't say what the factor is
      bRetval=PFBoolean::b_true;   // composite
   }

   ShowPrompt();
   delete pOut;
   return bRetval;
}

DWORD NMinus1Exponentiator::MinimumArguments() const
{
   return 2;
}

DWORD NMinus1Exponentiator::MaximumArguments() const
{
   return 2;
}

DWORD NMinus1Exponentiator::GetArgumentType(DWORD dwIndex) const
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

PFString NMinus1Exponentiator::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval;

   switch(dwIndex)
   {
   case 0:
      sRetval="_N";
      break;
   case 1:
      sRetval="_FACTORTABLE";
      break;
   default:
      sRetval="";
      break;
   }

   return sRetval;
}

