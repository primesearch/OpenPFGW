#ifndef F_DECIMAL_H
#define F_DECIMAL_H

#include "pfiterativesymbol.h"

class Integer;
class PowerBuffer;
class RecursionBuffer;

class F_Decimal : public PFIterativeSymbol
{
	Integer 		*m_pN;

#ifdef USE_GALLOT
	DWORD 		*m_pDigitBuffer;
	DWORD 		*m_pDigitWrite;
	DWORD		m_dwStackPointer;
	DWORD		m_dwRecursionDepth;
	PowerBuffer *m_pPowerBuffer;
	RecursionBuffer *m_pRecursionBuffer;
	DWORD		m_dwDigitBufferLength;
	PFBoolean	m_bSigned;
#else
	char *pDigitBuffer;
#endif
	
	void Deallocate();

public:
	F_Decimal();
	~F_Decimal();
	
	F_Decimal(const F_Decimal&);
	F_Decimal &operator=(const F_Decimal&);
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dIndex) const;
	
	PFBoolean OnExecute(PFSymbolTable *pContext);
	PFBoolean OnInitialize();
	PFBoolean Iterate();
	PFBoolean OnCompleted(PFSymbolTable *pContext);
	PFBoolean OnCleanup(PFSymbolTable *pContext);
};
#endif
