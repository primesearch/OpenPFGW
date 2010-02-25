// A symbol table for PrimeForm. Note that a symbol table is dynamically
// editable, so we'll make it a list
#ifndef PFSYMBOLTABLE_H
#define PFSYMBOLTABLE_H

#include "pflist.h"
#include "ipfsymbol.h"

class PFSymbolTable : public PFList<IPFSymbol>
{
	PFSymbolTable *m_pParent;
	IPFSymbol* _LookupSymbol(const PFString &sKey);
	PFListNode* _LookupSymbolNode(const PFString &sKey);
public:
	PFSymbolTable(PFSymbolTable *pParent=NULL);
	virtual ~PFSymbolTable();
	
	PFSymbolTable(const PFSymbolTable &st);
	PFSymbolTable &operator=(const PFSymbolTable &st);
	
	IPFSymbol *LookupSymbol(const PFString &sKey);
	IPFSymbol *RemoveSymbol(const PFString &sKey);
	void EraseSymbol(const PFString &sKey);
	
	PFBoolean AddSymbol(IPFSymbol *pSymbol);

#ifdef _DEBUG
	void ListSymbols();
#endif
};
#endif
