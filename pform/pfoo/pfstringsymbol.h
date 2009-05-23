// a string symbol
#ifndef PFSTRINGSYMBOL_H
#define PFSTRINGSYMBOL_H

class PFStringSymbol : public IPFSymbol
{
	PFString m_sValue;
public:
	PFStringSymbol(const PFString &sKey,const PFString &sValue="");
	
	PFString GetStringValue();
	void SetValue(const PFString &sValue);
	DWORD GetSymbolType() const;
};
#endif
