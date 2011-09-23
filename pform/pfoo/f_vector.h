#ifndef F_VECTOR_H
#define F_VECTOR_H

#include "pfiterativesymbol.h"
#include "integer.h"

class Integer;
class PFFactorizationSymbol;

class F_Vector : public PFIterativeSymbol
{
	Integer	*pN;

	uint32 pmin;
	uint32 pmax;
	uint32 p;
	
	double estimatePrimes(double l);
	double estimatePrimes(double l1,double l2);
	double estimateLimit(double x);
	
public:
	F_Vector();
	
	F_Vector(const F_Vector &);
	F_Vector &operator=(const F_Vector &);
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;

	PFBoolean OnExecute(PFSymbolTable *pContext);
	PFBoolean OnInitialize();
	PFBoolean Iterate();
	PFBoolean OnCompleted(PFSymbolTable *pContext);
	PFBoolean OnCleanup(PFSymbolTable *pContext);
	
	void OnPrompt();
};
#endif
