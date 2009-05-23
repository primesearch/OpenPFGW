#ifndef F_PRIME_H
#define F_PRIME_H

#include "pffunctionsymbol.h"

class F_Prime : public PFFunctionSymbol
{
public:
	F_Prime();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};
#endif
