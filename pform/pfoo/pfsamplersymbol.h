#ifndef 	PFSAMPLERSYMBOL_H
#define	PFSAMPLERSYMBOL_H

class PFSamplerSymbol : public IPFSymbol
{
	uint32_t m_dwLastCRC;
	uint32_t m_dwLargePrime;
	
	uint32_t m_dwSmallPrimes[60];
	uint32_t m_dwAcceptedPrimes[60];
	
	uint32_t m_dwSmallIndex;
	uint32_t m_dwAcceptIndex;
	
	uint32_t m_dwSmallCount;
	
public:
	PFSamplerSymbol();
	~PFSamplerSymbol();
	
	DWORD GetSymbolType() const;
	PFString GetStringValue();
	
	uint32_t ask(const Integer &N);
	uint32_t askagain();
	void accept(uint32_t p);
private:
	void rearrange();
};
#endif
