#ifndef NMINUS1EXPONENTIATOR_H
#define NMINUS1EXPONENTIATOR_H

#include "exponentiator.h"

class NMinus1Exponentiator : public Exponentiator
{
//    unsigned long m_base;
//    CertificateFile *pCertificate;
public:
    NMinus1Exponentiator();
    ~NMinus1Exponentiator();

    // tests return false if the test is to continue, bool for a
    // premature exit
    PFBoolean testFinal();
    PFBoolean testLeaf();
    PFBoolean testInternal();
 
 	PFBoolean 	GetTotalExponentiation(PFSymbolTable *pContext,Integer &X);
 	FiniteField *GetFieldElements(PFSymbolTable *pContext,Residue *&res,Multiplier *&mul);

	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
};

#endif

