#ifndef F_SMARANDACHE_WELLIN_H
#define F_SMARANDACHE_WELLIN_H

#include "pffunctionsymbol.h"

class F_SmarandacheWellinBase : public PFFunctionSymbol
{
public:
	F_SmarandacheWellinBase(const PFString &sName);
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
};

class F_SmarandacheWellin : public F_SmarandacheWellinBase
{
public:
	F_SmarandacheWellin();

	PFBoolean CallFunction(PFSymbolTable *pContext);
};

class F_SmarandacheWellinPrime : public F_SmarandacheWellinBase
{
public:
	F_SmarandacheWellinPrime();
	
	PFBoolean CallFunction(PFSymbolTable *pContext);
};

#endif 
