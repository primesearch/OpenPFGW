#ifndef PFFACTORIZATIONSYMBOL_H
#define PFFACTORIZATIONSYMBOL_H

class FactorNode;

class PFFactorizationSymbol : public IPFSymbol
{
	PFList<FactorNode> m_lFactors;
public:
	PFFactorizationSymbol(const PFString &sKey);
	
	PFString GetStringValue();
	DWORD GetSymbolType() const;
	
	void AddFactor(FactorNode* pNode);
	PFList<FactorNode> *AccessList();
};
#endif
