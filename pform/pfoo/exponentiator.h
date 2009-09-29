#ifndef EXPONENTIATOR_H
#define EXPONENTIATOR_H

#include "algebra.h"
#include "exponentiation.h"
#include "pfiterativesymbol.h"

class Residue;
class FactorNode;
class FactorArray;
class PFFactorizationSymbol;
class Modulus;
class FiniteField;

class Exponentiator : public PFIterativeSymbol
{
protected:
    // global
    Residue *pResidue;            // current progress of test
    Multiplier *pMultiplier;    // multiplier applied to all tests
    FiniteField *pField;        // field in which we reside

	// current task    
    Integer exponent;           // current exponent applied
    int bitindex;               // which bit we are currently iterating
    FactorNode *pDestination;   // where we are heading

	// stacked tasks
    ExponentStack itemStack;    // stack of in progress items

    PFBoolean CheckForFatalError(const char *caller, int currentIteration, int maxIterations);
protected:
    Integer 	m_N;					// the value under test
    Integer		m_F;					// factored part used by the algorithm
    PFBoolean 	m_bTargetMade;		// whether the factored part is on target
    FactorNode	*m_pTree;			// Mihailescu tree
    
    PFFactorizationSymbol *m_pSource;
    PFFactorizationSymbol *m_pJunkyard;
    
    Integer		m_X;					// total exponentiation length
    PFSymbolTable *m_pContext;	// track context
    Integer		m_PROVED;			// product of all proven factors
public:
    Exponentiator(const PFString &sName);
    ~Exponentiator();
    
    Exponentiator(const Exponentiator &);
    Exponentiator &operator=(const Exponentiator &);

   PFBoolean OnExecute(PFSymbolTable *pContext);
	PFBoolean OnInitialize();

	PFBoolean OnCompleted(PFSymbolTable *pContext);
	PFBoolean OnCleanup(PFSymbolTable *pContext);
   
    PFBoolean Iterate();

    // tests return false if the test is to continue, bool for a
    // premature exit
    virtual PFBoolean testFinal()=0;
    virtual PFBoolean testLeaf()=0;
    virtual PFBoolean testInternal()=0;
    
    virtual PFBoolean 	GetTotalExponentiation(PFSymbolTable *pContext,Integer &X)=0;
    virtual FiniteField *GetFieldElements(PFSymbolTable *pCOntext,Residue *&res,Multiplier *&mul)=0;
    
    
    // first try to get TARGET
    
    PFBoolean AimForTarget(PFFactorizationSymbol *pSymbol,const Integer &T,
    										 PFList<FactorNode> *&pResult,Integer &R);
    static FactorNode *BuildMihailescuTree(PFList<FactorNode> *pSource);

	FactorNode *FindSourceNode(FactorNode *pLeaf);
	
    void AddToJunkyard(FactorNode *pFactor);
    void AddToResults(FactorNode *pFactor);
};
#endif
