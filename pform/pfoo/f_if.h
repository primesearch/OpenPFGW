#ifndef F_IF_H
#define F_IF_H

#include "pffunctionsymbol.h"

class F_IF : public PFFunctionSymbol
{
public:
	F_IF();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};
#endif
