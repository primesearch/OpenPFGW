#ifndef F_NEXTPRIME_H
#define F_NEXTPRIME_H

#include "pffunctionsymbol.h"
#include "primeserver.h"

class F_NextPrime : public PFFunctionSymbol
{
public:
	F_NextPrime();
	~F_NextPrime();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);

private:
   PrimeServer    *m_pPrimeServer;
};
#endif
