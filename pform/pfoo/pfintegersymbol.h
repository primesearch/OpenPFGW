// an integer symbol
#ifndef PFINTEGERSYMBOL_H
#define PFINTEGERSYMBOL_H

#include "f_decimal.h"

class PFIntegerSymbol : public IPFSymbol
{
	static F_Decimal 		*m_pFuncDecimal;
	static PFSymbolTable *m_pSymDecimal;
	Integer *m_pInteger;
	bool m_bShallowSave;	// whether we should handle "clean-up" for the m_pInteger value.
public:
	PFIntegerSymbol(const PFString &sKey,Integer *pInteger=NULL, bool bShallowSave=false);
	~PFIntegerSymbol();
	
	PFIntegerSymbol(const PFIntegerSymbol &s);
	
	PFIntegerSymbol& operator=(const PFIntegerSymbol &s);
	
	PFString GetStringValue();
	DWORD GetSymbolType() const;
	
	Integer *DuplicateValue() const;
	Integer *GetValue() const;
	Integer *SetValue(Integer *p);
	
	static void Startup();
	static void Shutdown();
};
#endif
