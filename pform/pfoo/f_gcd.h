#ifndef F_GCD_H
#define F_GCD_H

#include "pffunctionsymbol.h"

class F_GCD : public PFFunctionSymbol
{
public:
	F_GCD();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};
#endif
