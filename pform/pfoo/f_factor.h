#ifndef F_FACTOR_H
#define F_FACTOR_H

#include "pfiterativesymbol.h"
#include "treefactorize.h"

class Integer;
class PFFactorizationSymbol;
class PFSimpleFile;
class Erat_Mod;
class FactorHelperArray;

class TreeFactorize;

class F_Factor : public PFIterativeSymbol
{
	Integer	*pN;
	
	Integer	Q;		// N is factored as PQ
	Integer	R;		// N-1 is factored as FR
	Integer	S;		// N+1 is factored as GS
	
	Integer	P1;			// P1 is the square of pmax
	Integer  *pBiggest;	// pBiggest is the pointer to largest op

	FactorHelperArray *pHelperArray;
	
	TreeFactorize m_pLittleTrees[8];
	uint32 m_nLittleTrees;
	uint32 m_nLittleMaxPR;
	bool m_bCompletedLittlePR;
	bool m_bUseTinyTrick;
	uint32 m_nTinyTrickCnt;
	uint32 m_nTinyTrickCntMax;			// for "normal" factoring, the max is set to 3, and 6 m_64TinyTrickFactors are filled in (thus 3 calls to dual factoring)
	uint64 m_64TinyTrickFactors[12];	// for modular factoring, the max is set to 6, and all 12 m_64TinyTrickFactors are filled in (6 calls to dual)
	TreeFactorize *m_pTreeFactorize;
	bool m_bUseTreeFactorize;

	PFString m_sHelperFile;	// the name helper file
	static PFSimpleFile *pFactorHelperFile;	// the actual helper file object
	
	uint64 pmin;
	uint64 pmax;
	PFBoolean bFactorAtAll;
	PFBoolean bDeep;
	uint64 p;   // Phil - factor to 64 bit primes (48 actually)
	int m_nPercentMultiplier;

	bool m_bModFactor;
	bool m_bDualModFactor;
	int	 m_nModFactor;
	Erat_Mod *m_pEratMod;
	Erat_Mod *m_pEratMod2;
	
	PFFactorizationSymbol *m_pffNminus1;
	PFFactorizationSymbol *m_pffN;
	PFFactorizationSymbol *m_pffNplus1;
	
	double estimatePrimes(double l);
	double estimatePrimes(double l1,double l2);
	double estimateLimit(double x);
	
	void checkBiggest(PFFactorizationSymbol *,Integer *);
	void pmaxadjust(Integer *pChanged);
	
	bool IterateTreeFactorize(TreeFactorize *pTree);

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
