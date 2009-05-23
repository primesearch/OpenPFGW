#ifndef 	PFSAMPLERSYMBOL_H
#define	PFSAMPLERSYMBOL_H

class PFSamplerSymbol : public IPFSymbol
{
	uint32 m_dwLastCRC;
	uint32 m_dwLargePrime;
	
	uint32 m_dwSmallPrimes[60];
	uint32 m_dwAcceptedPrimes[60];
	
	uint32 m_dwSmallIndex;
	uint32 m_dwAcceptIndex;
	
	uint32 m_dwSmallCount;
	
public:
	PFSamplerSymbol();
	~PFSamplerSymbol();
	
	DWORD GetSymbolType() const;
	PFString GetStringValue();
	
	uint32 ask(const Integer &N);
	uint32 askagain();
	void accept(uint32 p);
private:
	void rearrange();
};
#endif
