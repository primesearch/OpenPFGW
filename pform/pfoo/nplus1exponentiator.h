#ifndef NPLUS1EXPONENTIATOR_H
#define NPLUS1EXPONENTIATOR_H

#include "exponentiator.h"

class NPlus1Exponentiator : public Exponentiator
{
public:
    NPlus1Exponentiator();
    ~NPlus1Exponentiator();

    // tests return false if the test is to continue, bool for a
    // premature exit
    PFBoolean testFinal();
    PFBoolean testLeaf();
    PFBoolean testInternal();
    
  	PFBoolean 	GetTotalExponentiation(PFSymbolTable *pContext,Integer &X);
 	FiniteField *GetFieldElements(PFSymbolTable *pCOntext,Residue *&res,Multiplier *&mul);

	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
};

#endif
