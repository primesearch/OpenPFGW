#ifndef F_PREVPRIME_H
#define F_PREVPRIME_H

#include "pffunctionsymbol.h"
#include "primeserver.h"

class F_PrevPrime : public PFFunctionSymbol
{
public:
	F_PrevPrime();
	~F_PrevPrime();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);

private:
   PrimeServer    *m_pPrimeServer;
};
#endif
