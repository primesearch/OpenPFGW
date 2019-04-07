#ifndef F_FACTOR_H
#define F_FACTOR_H

#include <primesieve.hpp>
#include "pfiterativesymbol.h"

class Integer;
class PFFactorizationSymbol;
class PFSimpleFile;
class Erat_Mod;
class FactorHelperArray;

class F_Factor : public PFIterativeSymbol
{
	Integer	*pN;
	
	Integer	Q;		// N is factored as PQ
	Integer	R;		// N-1 is factored as FR
	Integer	S;		// N+1 is factored as GS
	
	Integer	P1;			// P1 is the square of pmax
	Integer  *pBiggest;	// pBiggest is the pointer to largest op

	FactorHelperArray *pHelperArray;
	
	PFString m_sHelperFile;	// the name helper file
	static PFSimpleFile *pFactorHelperFile;	// the actual helper file object
	
	uint64_t pmin;
	uint64_t pmax;
   uint64_t maxpTested;
	PFBoolean bFactorAtAll;
	PFBoolean bDeep;
   primesieve::iterator psIterator;
	int m_nPercentMultiplier;

	bool m_bModFactor;
	bool m_bDualModFactor;
	int	 m_nModFactor;
	Erat_Mod *m_pEratMod;
	Erat_Mod *m_pEratMod2;
	
	PFFactorizationSymbol *m_pffNminus1;
	PFFactorizationSymbol *m_pffN;
	PFFactorizationSymbol *m_pffNplus1;
	
	double estimateLimit(double x);
	
   bool checkForFactor(uint64_t p, uint64_t rem);
	void checkBiggest(PFFactorizationSymbol *,Integer *);
	void pmaxadjust(Integer *pChanged);
	
public:
	F_Factor();
	~F_Factor();
	
	F_Factor(const F_Factor &);
	F_Factor &operator=(const F_Factor &);
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;

	PFBoolean OnExecute(PFSymbolTable *pContext);
	PFBoolean OnInitialize();
	PFBoolean Iterate();
	PFBoolean OnCompleted(PFSymbolTable *pContext);
	PFBoolean OnCleanup(PFSymbolTable *pContext);
	
	CTimer Timer;
	void OnPrompt();
	static void CleanupHelperFileObject(); // call on program exit.
};
#endif
