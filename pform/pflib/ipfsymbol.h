// the interface class for an IPFSymbol
#ifndef IPFSYMBOL_H
#define IPFSYMBOL_H

#include "pfstring.h"

class IPFSymbol
{
	PFString m_sKey;
public:
	IPFSymbol(const PFString &sKey);
	IPFSymbol(const IPFSymbol &s);
	virtual ~IPFSymbol();
	
	IPFSymbol& operator=(const IPFSymbol &s);
	
	const PFString &GetKey() const;
	void SetKey(const PFString &s);
	
	virtual PFString GetStringValue()=0;
	virtual DWORD GetSymbolType() const=0;
};
#endif
