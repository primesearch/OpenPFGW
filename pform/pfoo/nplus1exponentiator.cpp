#include "pfoopch.h"
#include "nplus1exponentiator.h"
#include "factornode.h"
#include "factorarray.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "testreturns.h"

#undef GWDEBUG
#undef INTDEBUG
#define GWDEBUG(X) {Integer XX;XX=X;printf(#X "=");mpz_out_str(stdout,16,XX.gmp();printf("\n");}
#define INTDEBUG(X) {printf(#X "=");mpz_out_str(stdout,16,(X).gmp();printf("\n");}

NPlus1Exponentiator::NPlus1Exponentiator()
    : Exponentiator("nplus1")

{
}

NPlus1Exponentiator::~NPlus1Exponentiator()
{
}

PFBoolean NPlus1Exponentiator::testInternal()
{
    return PFBoolean::b_false;
}

PFBoolean NPlus1Exponentiator::testLeaf()
{
	// for leaf nodes, perform the following test
    // check U.
    // U=0 - can't conclude a thing
    // gcd(U,N) not one, composite
    // gcd(U,N) one, looking good.

    PFBoolean bRetval=PFBoolean::b_false;             // whether we need a premature exit

#ifdef WAS
    IntegerLucasOutputResidue *pILOR=(IntegerLucasOutputResidue*)pResidue->collapse();
    Integer &U=pILOR->content();

	if(U==0)
    {
		AddToJunkyard(pDestination);
    }
    else
	{
	Integer G=gcd(U,m_N);
	Integer G(1);
        if(G!=1)
		{
			PFIntegerSymbol sDummy("dummy",NULL);
			sDummy.SetValue(&G);
			PFString sG=sDummy.GetStringValue();
			sDummy.SetValue(NULL);

			PFPrintfClearCurLine();
			PFPrintfLog("Factored: %s\n",LPCTSTR(sG));

            testResult=PT_FACTOR;   // known factor
            bRetval=PFBoolean::b_true;
        }
		else
		{
			AddToResults(pDestination);
		}
    }

	delete pILOR;
#endif
AddToResults(pDestination);

    ShowPrompt();
    return bRetval;
}

PFBoolean NPlus1Exponentiator::testFinal()
{
    PFBoolean bRetval=PFBoolean::b_false;
	
    IntegerLucasOutputResidue *pILOR=(IntegerLucasOutputResidue*)pResidue->collapse();
    Integer &U=pILOR->content();

    if(U==0)
    {
		// Lucas PRP
	}
    else
    {
		testResult=PT_COMPOSITE;   // can't say what the factor is
        bRetval=PFBoolean::b_true;   // composite
	}

	delete pILOR;
    ShowPrompt();
    return bRetval;
}



DWORD NPlus1Exponentiator::MinimumArguments() const
{
	return 2;
}

DWORD NPlus1Exponentiator::MaximumArguments() const
{
	return 2;
}

DWORD NPlus1Exponentiator::GetArgumentType(DWORD dwIndex) const
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

PFString NPlus1Exponentiator::GetArgumentName(DWORD dwIndex) const
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

PFBoolean NPlus1Exponentiator::GetTotalExponentiation(PFSymbolTable * /*pContext*/,Integer &X)
{
	X=m_N+1;
	return PFBoolean::b_true;
}

FiniteField *NPlus1Exponentiator::GetFieldElements(PFSymbolTable *pContext,Residue *&res,Multiplier *&mul)
{
	FieldLucas *pF=NULL;
	
	// lookup the discriminant _D and the base _B+sqrt(D)
	PFIntegerSymbol *psD=(PFIntegerSymbol*)pContext->LookupSymbol("_D");
	PFIntegerSymbol *psB=(PFIntegerSymbol*)pContext->LookupSymbol("_B");
	
	PFString sD=psD->GetStringValue();
	PFString sB=psB->GetStringValue();

	PFOutput::EnableOneLineForceScreenOutput();

	// I think these need to be in the pfgw.out file!!
//	PFPrintfStderr("Running N+1 test using discriminant %s, base %s+sqrt(%s)\n", 
//		LPCTSTR(sD),LPCTSTR(sB),LPCTSTR(sD));
	PFPrintfLog("Running N+1 test using discriminant %s, base %s+sqrt(%s)\n", 
		LPCTSTR(sD),LPCTSTR(sB),LPCTSTR(sD));
	
	// is D small 2D, 2(D-1) in small range
	Integer *pD=psD->GetValue();
	Integer *pB=psB->GetValue();
	
	int iD=(*pD)&0xFFFFFFFF;
	
	if((*pD)==iD)
	{
		if(iD>=-GWMULBYCONST_MAX/2 && iD<=GWMULBYCONST_MAX/2)
		{
			pF=new FieldLucasSmall(iD,&m_N);
		}
		else if((double)iD>=-GWSMALLMUL_MAX/2.0 && (double)iD<GWSMALLMUL_MAX/2.0)
		{
			pF=new FieldLucasMedium((double)iD,&m_N);
		}
	}
	
	if(pF==NULL)
	{
		pF=new FieldLucasLarge(*pD,&m_N);
	}
	
	// create field residue multiplier
	// residue, multiplier
	Integer one(1);
	mul=pF->createCompatibleMultiplier(*pB,one);
	res=pF->createCompatibleResidue(*pB,one);   // the word DURR springs to mind

	testResult=PT_INCONCLUSIVE;

	return pF;
}
