#include "pfoopch.h"
#include "f_fibonacci.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "primeserver.h"

#include "h_helpers.h"

F_Fibonacci::F_Fibonacci(const PFString &sName)
	: PFFunctionSymbol(sName)
{
}

F_Fibonacci_U::F_Fibonacci_U() : F_Fibonacci("U")
{
}

F_Fibonacci_V::F_Fibonacci_V() : F_Fibonacci("V")
{
}

F_Fibonacci_F::F_Fibonacci_F() : F_Fibonacci("F")
{
}

F_Fibonacci_L::F_Fibonacci_L() : F_Fibonacci("L")
{
}

DWORD F_Fibonacci::MinimumArguments() const
{
	return 1;
}

DWORD F_Fibonacci::MaximumArguments() const
{
	return 1;
}

DWORD F_Fibonacci::GetArgumentType(DWORD /*dwIndex*/) const
{
	return INTEGER_SYMBOL_TYPE;
}

PFString F_Fibonacci::GetArgumentName(DWORD /*dwIndex*/) const
{
	return "_N";
}

PFBoolean helpFibonacci2(PFSymbolTable * /*pRetval*/,int iIndex,Integer &U,Integer &V)
{
	int iBitmask=0x40000000;
	
	while(!(iIndex&iBitmask) && (iBitmask))
	{
		iBitmask>>=1;
	}

	if(iBitmask==0)
	{
		V=2;
		U=0;
	}
	else
	{
		V=1;
		U=1;		// (V,U)=phi^1

		Integer UU;
				
		for(iBitmask>>=1;iBitmask;iBitmask>>=1)
		{
			// square
			// V'=(VV+5UU)/2
			// U'=UV
			UU=U;
			UU*=V;
			
			V*=V;
			U*=U;
			U*=5;
			V+=U;
			V>>=1;
			U=UU;	
		
			// multiply
			if(iIndex&iBitmask)
			{
				// V'=(V+5U)/2
				// U'=(U+V)/2
				UU=U;
				UU+=V;
				UU>>=1;
				
				U*=5;
				V+=U;

				V>>=1;
				U=UU;
			}
		}
	}
	return PFBoolean::b_true;
}

PFBoolean helpFibonacci(PFSymbolTable *pRetval,int iIndex,Integer &U)
{
	Integer V;
	return helpFibonacci2(pRetval,iIndex,U,V);
}

PFBoolean F_Fibonacci_F::CallFunction(PFSymbolTable *pContext)
{
	Integer *rU=new Integer;
	Integer *rV=new Integer;
	
	PFBoolean bRetval=PFBoolean::b_false;
	// _N in the symbol table is the index
	IPFSymbol *pSymbol=pContext->LookupSymbol("_N");
	if(pSymbol)
	{
		if(pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
		{
			Integer *q=((PFIntegerSymbol*)pSymbol)->GetValue();
			if(q)
			{
		   		int idx=(*q)&(0x7fffffff); // nothing unusual there
		    	if((*q)==idx)
				{
					bRetval=helpFibonacci2(pContext,idx,*rU,*rV);
					if(bRetval)
					{
			    		pContext->AddSymbol(new PFIntegerSymbol("_result",rU));
			    	}
			    	else
			    	{
			    		delete rU;
			    	}
			    	delete rV;
			    }
			}
    	}
	}
	return bRetval;
}

PFBoolean F_Fibonacci_L::CallFunction(PFSymbolTable *pContext)
{
	Integer *rU=new Integer;
	Integer *rV=new Integer;
	
	PFBoolean bRetval=PFBoolean::b_false;
	// _N in the symbol table is the index
	IPFSymbol *pSymbol=pContext->LookupSymbol("_N");
	if(pSymbol)
	{
		if(pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
		{
			Integer *q=((PFIntegerSymbol*)pSymbol)->GetValue();
			if(q)
			{
		   		int idx=(*q)&(0x7fffffff); // nothing unusual there
		    	if((*q)==idx)
				{
					bRetval=helpFibonacci2(pContext,idx,*rU,*rV);
					if(bRetval)
					{
			    		pContext->AddSymbol(new PFIntegerSymbol("_result",rV));
			    	}
			    	else
			    	{
			    		delete rV;
			    	}
			    	delete rU;
			    }
			}
    	}
	}
	return bRetval;
}

PFBoolean F_Fibonacci_U::CallFunction(PFSymbolTable *pContext)
{
	Integer *r=new Integer;
	PFBoolean bRetval=H_Primitive::Evaluate(pContext,helpFibonacci,*r);
	
	if(r)
	{
		if(!bRetval)
		{
			delete r;
		}
		else
		{
    		pContext->AddSymbol(new PFIntegerSymbol("_result",r));
    	}
	}
	return bRetval;
}

PFBoolean F_Fibonacci_V::CallFunction(PFSymbolTable *pContext)
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
				// you need to cheat and pass 2N to the evaluator
				PFSymbolTable *pSubContext=new PFSymbolTable(pContext);
				Integer *p2N=new Integer(*q);
				(*p2N)<<=1;
				pSubContext->AddSymbol(new PFIntegerSymbol("_N",p2N));
				
				Integer *r=new Integer;
				bRetval=H_Primitive::Evaluate(pSubContext,helpFibonacci,*r);
				
				if(r)
				{
					if(!bRetval)
					{
						delete r;
					}
					else
					{
			    		pContext->AddSymbol(new PFIntegerSymbol("_result",r));
			    	}
				}
				delete pSubContext;
        	}
		}
	}
	return bRetval;
}
