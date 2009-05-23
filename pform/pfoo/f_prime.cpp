#include "pfoopch.h"
#include "f_prime.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "primeserver.h"

F_Prime::F_Prime()
	: PFFunctionSymbol("p")
{
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
	if(pSymbol)
	{
		if(pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
		{
			Integer *q=((PFIntegerSymbol*)pSymbol)->GetValue();
			if(q)
			{
		   		int idx=(*q)&(0x7fffffff); // nothing unusual there

				// Bug fix.  With out lousy primegen, we only get solid primes
				// up to (2^15)^2.  That being the case, we only want to compute
				// primes up to this value.  Anything over that is not correctly
				// computed.
				if (idx > 54400001)   // p(54400001) is 1073741237 which is just under (2^15)^2 whic is 1073741824
					return bRetval;

		    	if((*q)==idx)
				{
		    		Integer *r=new Integer;
					if(idx==0)
				   	{
        				*r=1;
        				bRetval=PFBoolean::b_true;
        			}
					else
					{
						// this is a really lousy implementation
						primeserver.restart();
						uint32 p=1;
			        	for(uint32 i=0;i<idx;i++)
            				primeserver.next(p);
       					*r=p;
       					bRetval=PFBoolean::b_true;
        			}
        			
        			if(!bRetval)
        			{
        				delete r;
        			}
        			else
        			{
        				pContext->AddSymbol(new PFIntegerSymbol("_result",r));
        			}
        		}
        	}
		}
	}
	return bRetval;
}
