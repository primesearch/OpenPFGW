#ifndef EXPONENTIATION_H
#define EXPONENTIATION_H

class Residue;
class FactorNode;
class Multiplier;

class Exponentiation
{
    Integer m_exponent;
    Residue *m_pResidue;
    FactorNode *m_pDestination;
    Multiplier *m_pMultiplier;

public:
    Exponentiation();
    ~Exponentiation();
    
    Exponentiation(const Exponentiation &);
    Exponentiation &operator=(const Exponentiation &);

    void setExponent(const Integer &ex);
    void setResidue(Residue *r);
    void setDestination(FactorNode *dest);
    void setMultiplier(Residue *m);

    const Integer &exponent();
    Residue *residue();
    FactorNode *destination();
    Multiplier *multiplier();
};

typedef PFStack<Exponentiation> ExponentStack;
#endif
