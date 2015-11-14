#ifndef F_COPELAND_ERDOS_H
#define F_COPELAND_ERDOS_H

#include "pffunctionsymbol.h"

class F_CopelandErdos : public PFFunctionSymbol
{
public:
	F_CopelandErdos();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};

#endif 
