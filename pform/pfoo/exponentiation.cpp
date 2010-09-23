#include "pfoopch.h"
#include "exponentiation.h"

#include "algebra.h"

Exponentiation::Exponentiation() : m_exponent(0), m_pResidue(NULL), m_pDestination(NULL), m_pMultiplier(NULL)
{
}

Exponentiation::~Exponentiation()
{
    if(m_pResidue) delete m_pResidue;   // if it's still around
    if(m_pMultiplier) delete m_pMultiplier;
}

void Exponentiation::setExponent(const Integer &x)
{
    m_exponent=x;
}

void Exponentiation::setResidue(Residue *r)
{
    if(m_pResidue) delete m_pResidue;
   m_pResidue=r->duplicate();
}

void Exponentiation::setMultiplier(Residue *r)
{
    if(m_pMultiplier) delete m_pMultiplier;
   m_pMultiplier=r->duplicateAsMultiplier();
}

void Exponentiation::setDestination(FactorNode *dest)
{
    m_pDestination=dest;
}

const Integer& Exponentiation::exponent()
{
    return(m_exponent);
}

Residue *Exponentiation::residue()
{
    Residue *r=m_pResidue;
    m_pResidue=NULL;        // no longer my ownership
    return(r);
}

FactorNode *Exponentiation::destination()
{
    return(m_pDestination);
}

Multiplier *Exponentiation::multiplier()
{
    Multiplier *m=m_pMultiplier;
    m_pMultiplier=NULL;
    return(m);
}
