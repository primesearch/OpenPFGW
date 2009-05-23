#ifndef F_REPUNIT_H
#define F_REPUNIT_H

#include "pffunctionsymbol.h"

class F_Repunit : public PFFunctionSymbol
{
public:
	F_Repunit();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};
#endif
