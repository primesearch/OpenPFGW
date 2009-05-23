#ifndef F_LENGTH_H
#define F_LENGTH_H

#include "pffunctionsymbol.h"

class F_Length : public PFFunctionSymbol
{
public:
	F_Length();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};
#endif
