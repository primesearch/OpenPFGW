#ifndef F_ENDMINUS1_H
#define F_ENDMINUS1_H

#include "pffunctionsymbol.h"

class F_EndMinus1 : public PFFunctionSymbol
{
public:
	F_EndMinus1();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};
#endif
