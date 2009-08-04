#ifndef ALGEBRA_H
#define ALGEBRA_H

class Multiplier
{
public:
	virtual ~Multiplier();
};

class OutputResidue;

class Residue
{
protected:
    virtual Residue *clone()=0;
    virtual Multiplier *cloneMul()=0;
public:
	virtual ~Residue();

	virtual void multiply(Multiplier *)=0;
	virtual void square(int maxSteps, int stepsLeft)=0;
	virtual void squaremultiply(Multiplier *, int maxSteps, int stepsLeft);

	virtual OutputResidue *collapse()=0;

    Residue *duplicate()
    {
        return clone();
    }
    Multiplier *duplicateAsMultiplier()
    {
        return cloneMul();
    }
};

class OutputResidue
{
protected:

public:
};

// the simple derivations

class FieldZResidue : public Residue
{
public:
};

class IntegerResidue : public FieldZResidue
{
    GWInteger R;
public:
    IntegerResidue(Integer &N);
    IntegerResidue(GWInteger &RR);
    ~IntegerResidue();
protected:
    Residue *clone();
    Multiplier *cloneMul();
public:
	void multiply(Multiplier *);
	void square(int maxSteps, int stepsLeft);
	void squaremultiply(Multiplier *, int maxSteps, int stepsLeft);

	OutputResidue *collapse();
};

class IntegerOutputResidue : public OutputResidue
{
	Integer R;
public:
	IntegerOutputResidue();
	Integer &content();
};


class FieldZMultiplier : public Multiplier
{
public:
    virtual void mulInteger(GWInteger &X)=0;
    virtual void squaremulInteger(GWInteger &X, int maxSteps, int stepsLeft)=0;
};

class IntegerMultiplier : public FieldZMultiplier
{
    GWInteger M;
public:
    IntegerMultiplier(Integer &N);
    IntegerMultiplier(GWInteger &MM);
    ~IntegerMultiplier();

    void mulInteger(GWInteger &X);
    void squaremulInteger(GWInteger &X, int maxSteps, int stepsLeft);
};

class SmallIntegerMultiplier : public FieldZMultiplier
{
    int mm;
public:
    SmallIntegerMultiplier(int mm);
    ~SmallIntegerMultiplier();

    void mulInteger(GWInteger &X);
    void squaremulInteger(GWInteger &X, int maxSteps, int stepsLeft);
};


// Now we do the same for Lucas sequences

class FieldLucasResidue : public Residue
{
};

class FieldLucas;

class IntegerLucasResidue : public FieldLucasResidue
{
    FieldLucas *f;
    GWInteger v;	// v=V/2
    GWInteger u;	// u=U/2
public:
    IntegerLucasResidue(FieldLucas *ff,Integer &VV,Integer &UU);
    IntegerLucasResidue(FieldLucas *ff,GWInteger &VV,GWInteger &UU);
    ~IntegerLucasResidue();
    
    IntegerLucasResidue(const IntegerLucasResidue &);
    IntegerLucasResidue &operator=(const IntegerLucasResidue &);
protected:
    Residue *clone();
    Multiplier *cloneMul();
public:
	void multiply(Multiplier *);
	void square(int maxSteps, int stepsLeft);
	void squaremultiply(Multiplier *, int maxSteps, int stepsLeft);

	OutputResidue *collapse();
};

class IntegerLucasOutputResidue : public OutputResidue
{
	Integer U;
public:
	IntegerLucasOutputResidue();
	Integer &content();
};

class FieldLucasMultiplier : public Multiplier
{
public:
    virtual void mulResidues(GWInteger &v,GWInteger &u)=0;
    virtual void mulad(GWInteger &u,int D)=0;
    virtual void mulad(GWInteger &u,double d)=0;
    virtual void mulad(GWInteger &u,GWInteger &dfft,double dd)=0;
};

// mutlipliers come in a few flavors. b+a, b-a, and a itself may be
// zero, one, small (8 bit), moderate (24 bit), arbitrary.
// the 'cross' functions and mulResidues functions need to route
// through virtual members of FieldLucasMultiplier.
class WideLucasMultiplier : public FieldLucasMultiplier
{
	GWInteger bpa;
	GWInteger bma;
public:
	GWInteger a;

    WideLucasMultiplier(Integer &BB,Integer &AA);
    WideLucasMultiplier(GWInteger &BB,GWInteger &AA);
    ~WideLucasMultiplier();
    
    void mulResidues(GWInteger &v,GWInteger &u);
    virtual void mulad(GWInteger &u,int D);						// hypersmall
    virtual void mulad(GWInteger &u,double d);					// moderate
    virtual void mulad(GWInteger &u,GWInteger &dfft,double dd);	// hyperlarge
};

class NarrowLucasMultiplier : public FieldLucasMultiplier
{
	double bpa;
	double bma;
public:
	double a;

    NarrowLucasMultiplier(double BB,double AA);
    ~NarrowLucasMultiplier();
    
    void mulResidues(GWInteger &v,GWInteger &u);
    virtual void mulad(GWInteger &u,int D);
    virtual void mulad(GWInteger &u,double d);
    virtual void mulad(GWInteger &u,GWInteger &dfft,double dd);
};

//===============
// a finite field
//===============

class FiniteField
{
public:
	virtual ~FiniteField()=0;
};

class FieldZ : public FiniteField
{
public:
	FieldZ(Integer *N);
	virtual ~FieldZ();

	Multiplier *createCompatibleMultiplier(Integer &M);
	Residue *createCompatibleResidue(Integer &R);
};

// FieldLucas requires we choose a discriminant. The problem is that the discriminant
// must be the same across calls. We use (B+As)/(B-As) as the ring element, since its
// invariant under scalar transforms, we may as well assume A=1, or A=2, whatever is
// convenient, and require BB-D.AA to be a nonresidue.

// The best choice is actually A=2, and B even.
// The symbols _D and _B get set in the calling test.

// multiplication in a Lucas field uses 2(D-1), as does squaremul,
// but squaring uses 2D
class FieldLucas : public FiniteField
{
public:
	GWInteger *ps1;
	GWInteger *ps2;
	GWInteger *ps3;
	GWInteger *ps4;		// scratch variables
public:
	FieldLucas(Integer *N);
    virtual ~FieldLucas();

    Multiplier *createCompatibleMultiplier(Integer &VV,Integer &UU);
    Residue *createCompatibleResidue(Integer &VV,Integer &UU);

	virtual void mulcross(FieldLucasMultiplier *mm,GWInteger &u)=0;
	virtual void mulcross2(FieldLucasMultiplier *mm,GWInteger &u)=0;
	virtual void squarecross(GWInteger &ufft)=0;
	virtual void squaremulcross(GWInteger &u, int maxSteps, int stepsLeft)=0;
private:
	FieldLucas(const FieldLucas &);
	FieldLucas &operator=(const FieldLucas &);
};

// breakpoint is D<128
class FieldLucasSmall : public FieldLucas
{
    int D;	// 2(D-1), 2D
public:
	FieldLucasSmall(int DD,Integer *);
    virtual ~FieldLucasSmall();

	virtual void mulcross(FieldLucasMultiplier *mm,GWInteger &u);
	virtual void mulcross2(FieldLucasMultiplier *mm,GWInteger &u);
	virtual void squarecross(GWInteger &ufft);
	virtual void squaremulcross(GWInteger &u, int maxSteps, int stepsLeft);
};

class FieldLucasMedium : public FieldLucas
{
	double D;
public:
	FieldLucasMedium(double DD,Integer *);
    virtual ~FieldLucasMedium();

	virtual void mulcross(FieldLucasMultiplier *mm,GWInteger &u);
	virtual void mulcross2(FieldLucasMultiplier *mm,GWInteger &u);
	virtual void squarecross(GWInteger &ufft);
	virtual void squaremulcross(GWInteger &u, int maxSteps, int stepsLeft);
};	

class FieldLucasLarge : public FieldLucas
{
	GWInteger dminus1;
	GWInteger twod;
public:
	FieldLucasLarge(Integer &DD,Integer *);
    virtual ~FieldLucasLarge();

	virtual void mulcross(FieldLucasMultiplier *mm,GWInteger &u);
	virtual void mulcross2(FieldLucasMultiplier *mm,GWInteger &u);
	virtual void squarecross(GWInteger &ufft);
	virtual void squaremulcross(GWInteger &u, int maxSteps, int stepsLeft);
};

#endif
