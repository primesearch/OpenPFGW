#include "pfoopch.h"
#include "f_vector.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "primeserver.h"

F_Vector::F_Vector()
	: 	PFIterativeSymbol("@vector"), pN(NULL),
		pmin(0), pmax(0), p(0)
{
}

DWORD F_Vector::MinimumArguments() const
{
	return 3;
}

DWORD F_Vector::MaximumArguments() const
{
	return 3;
}

DWORD F_Vector::GetArgumentType(DWORD /*dwIndex*/) const
{
	return INTEGER_SYMBOL_TYPE;
}

PFString F_Vector::GetArgumentName(DWORD dwIndex) const
{
	PFString sRetval="";
	switch(dwIndex)
	{
	case 0:
		sRetval="_N";
		break;
	case 1:
		sRetval="_PMIN";
		break;
	case 2:
		sRetval="_PMAX";
		break;
	default:
		break;
	}
		
	return sRetval;
}

PFBoolean F_Vector::OnExecute(PFSymbolTable *pContext)
{
	PFBoolean bRetval=PFBoolean::b_true;
	IPFSymbol *pSymbol=pContext->LookupSymbol("_N");
	
	if(pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
	{
		pN=((PFIntegerSymbol*)pSymbol)->GetValue();
	}
	else
	{
		bRetval=PFBoolean::b_false;
	}
	
	// retrieve pmin, pmax and deep
	
	if(bRetval)
	{
		pSymbol=pContext->LookupSymbol("_PMIN");
		if(pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
		{
			Integer *ipmin=((PFIntegerSymbol*)pSymbol)->GetValue();
			pmin=(*ipmin&0x7FFFFFFF);
		}	
		else
		{
			bRetval=PFBoolean::b_false;
		}
	}
		
	if(bRetval)
	{	
		pSymbol=pContext->LookupSymbol("_PMAX");
		if(pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
		{
			Integer *ipmax=((PFIntegerSymbol*)pSymbol)->GetValue();
			pmax=(*ipmax&0x7FFFFFFF);
		}	
		else
		{
			bRetval=PFBoolean::b_false;
		}
	}

	return bRetval;
}

double F_Vector::estimatePrimes(double l)
{
	double d=0.0;
	if(l>3.0)
	{
		d=(l/(log(l)-1.0));
	}
	return d;
}

double F_Vector::estimatePrimes(double l1,double l2)
{
	return(estimatePrimes(l2)-estimatePrimes(l1));
}

// we will call it "the first 4n primes" for an n-bit number
double F_Vector::estimateLimit(double x)
{
	return(x*log(x));
}

PFBoolean F_Vector::OnInitialize()
{
	// get an estimate of the limit. We will use the metric that we expect 32 bit numbers
	// to be fully factored. ie 2^5 bits are factored up to 2^13 primes.
	if(pmin>pmax)
	{
		pmin=pmax;
	}
	
	m_dwStepsTotal=((DWORD)estimatePrimes(pmin,pmax))>>1;
	m_dwStepGranularity=2048;
	m_bStopOverride=PFBoolean::b_true;

	primeserver.restart();
	primeserver.skip(pmin);
	primeserver.next(p);

	return PFBoolean::b_true;
}

void F_Vector::OnPrompt()
{
// provided estimate primes is an overestimate, this will work
	m_dwStepsTotal=m_dwStepsDone+(DWORD)estimatePrimes(p,pmax);
}

PFBoolean F_Vector::Iterate()
{
	if(p>pmax)
	{
		return(PFBoolean::b_true);			// end the test
	}
	uint32 p2;
	primeserver.next(p2);
	int i1,i2;

	pN->m_mod2(p,p2,&i1,&i2);
	if(i1<0) i1+=p;
	if(i2<0) i2+=p2;

	PFPrintfLog("%u: %d\n",p,i1);
	if(p2<=pmax)
	{
		PFPrintfLog("%u: %d\n",p2,i2);	
	}
		
	primeserver.next(p);				// ready for the next iteration
	return(PFBoolean::b_false);				// and its not quitting time yet
}

PFBoolean F_Vector::OnCompleted(PFSymbolTable *pContext)
{
	PFfflush(stdout);
	pContext->AddSymbol(new PFIntegerSymbol("_factoredto",new Integer(int(p))));
	return PFBoolean::b_true;
}

PFBoolean F_Vector::OnCleanup(PFSymbolTable * /*pContext*/)
{
	return PFBoolean::b_true;
}
