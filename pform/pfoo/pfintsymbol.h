// an int symbol, not to be confused with *integer* symbol
#ifndef PFINTSYMBOL_H
#define PFINTSYMBOL_H

class PFIntSymbol : public IPFSymbol
{
public:
	PFIntSymbol(const PFString &sKey,int i);
	PFIntSymbol(const PFIntSymbol &s);
	~PFIntSymbol();

	PFIntSymbol& operator=(const PFIntSymbol &s);

	PFString GetStringValue();
	DWORD GetSymbolType() const;
	
	int GetValue() const;
	void SetValue(int i);

protected:
	int m_i;
};
#endif
