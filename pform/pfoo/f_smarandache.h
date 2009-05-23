#ifndef F_SMARANDACHE_H
#define F_SMARANDACHE_H

#include "pffunctionsymbol.h"

class F_Smarandache : public PFFunctionSymbol
{
public:
	F_Smarandache();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};

// reverse
class F_Smarandache_r : public PFFunctionSymbol
{
public:
	F_Smarandache_r();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};

#endif 
