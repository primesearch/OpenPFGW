// H_Primitive is an evaluator helper class

#include "pfoopch.h"
#include "h_helpers.h"

#include "f_trivial.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "pffactorizationsymbol.h"
#include "factornode.h"

PFBoolean H_Primitive::Evaluate(PFSymbolTable *pTable,EVALUATOR fEval,Integer &N)
{
	PFBoolean bRetval=PFBoolean::b_false;
	
	// _N in the symbol table is the index
	IPFSymbol *pSymbol=pTable->LookupSymbol("_N");
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
					// its in range
					// index is now a small, trivially factorable integer
					// so why not run F_Trivial on it
					PFSymbolTable *pSubContext=new PFSymbolTable(pTable);
					pSubContext->AddSymbol(new PFIntegerSymbol("_trivial_depth",new Integer(31)));
					int iTrivial=PFFunctionSymbol::CallSubroutine("@trivial",pSubContext);
					
					IPFSymbol *pFactor=pSubContext->LookupSymbol("_TRIVIALFACTOR");

					// Use the Mobius inversion formula.
					switch(iTrivial)
					{
					case TT_ZERO:
						bRetval=(fEval)(pSubContext,0,N);
						break;
					case TT_ONE:
						bRetval=(fEval)(pSubContext,1,N);
						break;
					default:
						{
							N=1;
							Integer D(1);
												
							PFFactorizationSymbol *pp=(PFFactorizationSymbol *)pFactor;
							// multiply through by (p-1)p^(a-1)
							PFList<FactorNode> *pList=pp->AccessList();
							DWORD dwFactors=pList->GetSize();
							
							bRetval=PFBoolean::b_true;
							
							for(DWORD dw=0;(bRetval)&&(dw<(1U<<dwFactors));dw++)
							{
								// for all Mobius terms
								int iPower=idx;
								int iMobius=0;
													
								PFForwardIterator pffi;
								pList->StartIterator(pffi);
								DWORD dwMask=1;
								PFListNode *n;
													
								while(pffi.Iterate(n))
								{
									FactorNode *fn=(FactorNode*)n->GetData();
									if(dw&dwMask)
									{
										iMobius++;
										// divide iIndex by this prime
										int iDivider=(fn->prime())&0x7FFFFFFF;
										iPower/=iDivider;
									}
									dwMask<<=1;
								}
											
								Integer T;
								bRetval=(fEval)(pSubContext,iPower,T);
								
								if(bRetval)
								{
									if(iMobius&1)
									{
										D*=T;
									}
									else
									{
										N*=T;	// this is not th most efficient method
									}
								}
							}
							
							if(bRetval)
							{
								N/=D;
							}
						}
						break;
					}
					delete pSubContext;
				}	// endif q was small
			}	// endif q exists
		}	// endif symbol is an integer
	}	// endif symbol exists
	
	return bRetval;
}
