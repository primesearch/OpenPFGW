#ifndef F_ISSQUARE_H
#define F_ISSQUARE_H

#include "pffunctionsymbol.h"

class F_IsSquare : public PFFunctionSymbol
{
public:
	F_IsSquare();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};
#endif
