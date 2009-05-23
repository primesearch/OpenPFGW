#ifndef F_CYCLOTOMIC_H
#define F_CYCLOTOMIC_H

#include "pffunctionsymbol.h"

class F_Cyclotomic : public PFFunctionSymbol
{
public:
	F_Cyclotomic();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};
#endif
