#ifndef F_BINOMIAL_H
#define F_BINOMIAL_H

#include "pffunctionsymbol.h"

class F_Binomial : public PFFunctionSymbol
{
public:
	F_Binomial();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};
#endif
