#ifndef F_ENDPLUS1_H
#define F_ENDPLUS1_H

#include "pffunctionsymbol.h"

class F_EndPlus1 : public PFFunctionSymbol
{
public:
	F_EndPlus1();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};
#endif
