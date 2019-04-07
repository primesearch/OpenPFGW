#ifndef F_TRIVIAL_H
#define F_TRIVIAL_H

#include "pffunctionsymbol.h"

class F_Trivial : public PFFunctionSymbol
{
public:
	F_Trivial();
   ~F_Trivial();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);

private:
};

// The values in the _result for F_Trivial are enumerated below
#define	TT_ZERO			0
#define	TT_ONE			1
#define	TT_TWO			2
#define	TT_FACTOR		3
#define	TT_NEGATIVE		4	// warning only
#define TT_COMPLETED	9999

#endif
