#ifndef PFEXPR_H
#define PFEXPR_H

#define DYADIC          0   // eg, + - * ^ / %
#define MONADIC_PREFIX  1   // eg. -
#define MONADIC_SUFFIX  2   // eg. ! #

class ExprOperator
{
	friend class D_CLOSEPAREN;	// want to be able to access protected members in other objects
	friend class CloneOperator;
protected:
   PFString symbol;
   int flags;              // type of symbol
   int precedence;         // operator precedence
   Integer m_gData;			// added integer data
public:
   ExprOperator(const PFString &,int f,int p);
   virtual ~ExprOperator();
   virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0)=0; // perform the op
	virtual const ExprOperator *Identify() const;
	virtual ExprOperator *Mutant() const;
	
   friend Integer *ex_parseInteger(PFString &w);
   friend PFBoolean ex_parseArguments(PFString &w,PFStringArray& tfArguments);
   friend ExprOperator *ex_seekOperator(PFStack<ExprOperator> &s,PFString &w);
   friend PFBoolean ex_clearOnInteger(PFStack<Integer> &i,PFStack<ExprOperator> &o,int m);
   friend PFBoolean ex_stackPrecedence(PFStack<Integer> &i,PFStack<ExprOperator> &o,int m,ExprOperator *op);
   friend Integer *ex_evaluate(PFSymbolTable *pContext,const PFString &e,int m);
   friend PFBoolean ex_clearMonadicSuffixes(PFStack<Integer> &i,PFStack<ExprOperator> &o,int m);
};

class CloneOperator : public ExprOperator
{
	ExprOperator *m_pSource;
public:
	CloneOperator(ExprOperator *pSource);
	CloneOperator(const CloneOperator &);
	
	CloneOperator& operator=(const CloneOperator&);

	virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
	virtual const ExprOperator *Identify() const;
};	

class DyadicList : public PFStack<ExprOperator>
{
public:
    DyadicList();
    ~DyadicList();
};

class MonadicPrefixList : public PFStack<ExprOperator>
{
public:
    MonadicPrefixList();
    ~MonadicPrefixList();
};

class MonadicSuffixList : public PFStack<ExprOperator>
{
public:
    MonadicSuffixList();
    ~MonadicSuffixList();
};

//==============================================================
// Dyadic operators
//==============================================================

class D_ADD : public ExprOperator
{
public:
    D_ADD() : ExprOperator("+",DYADIC,2) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_SUB : public ExprOperator
{
public:
    D_SUB() : ExprOperator("-",DYADIC,2) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_MUL : public ExprOperator
{
public:
    D_MUL() : ExprOperator("*",DYADIC,3) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_MULP : public ExprOperator
{
public:
    D_MULP() : ExprOperator(".",DYADIC,3) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_DIV : public ExprOperator
{
public:
    D_DIV() : ExprOperator("/",DYADIC,3) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_MOD : public ExprOperator
{
public:
    D_MOD() : ExprOperator("%",DYADIC,3) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

//class D_GCD : public ExprOperator
//{
//public:
//    D_GCD() : ExprOperator("|",DYADIC,2) {}
//    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
//};

class D_POW : public ExprOperator
{
public:
    D_POW() : ExprOperator("^",DYADIC,5) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_FACT : public ExprOperator
{
public:
    D_FACT() : ExprOperator("!",DYADIC,5) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

// This is NOT for a multi-primorial (like the Multi-Factorial). This is for a different "purpose"
// 1093#103 would give you 1093#/101# (or in other words:  103.107.109.113....1091.1093)
// without doing the division and the 2 primorial builds
class D_PRIM : public ExprOperator
{
public:
    D_PRIM() : ExprOperator("#",DYADIC,5) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_EQUAL : public ExprOperator
{
public:
    D_EQUAL() : ExprOperator("==",DYADIC,0) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_NOTEQUAL : public ExprOperator
{
public:
    D_NOTEQUAL() : ExprOperator("!=",DYADIC,0) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_GREATER : public ExprOperator
{
public:
    D_GREATER() : ExprOperator(">",DYADIC,1) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_LESS : public ExprOperator
{
public:
    D_LESS() : ExprOperator("<",DYADIC,1) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_GE : public ExprOperator
{
public:
    D_GE() : ExprOperator("=>",DYADIC,1) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_GE2 : public ExprOperator	// "proper" C++ syntax
{
public:
    D_GE2() : ExprOperator(">=",DYADIC,1) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_LE : public ExprOperator
{
public:
    D_LE() : ExprOperator("=<",DYADIC,1) {} // who the hell put this in there!!
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_LE2 : public ExprOperator
{
public:
    D_LE2() : ExprOperator("<=",DYADIC,1) {} // who the hell put this in there!!
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_OR : public ExprOperator
{
public:
    D_OR() : ExprOperator("OR",DYADIC,-2) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_OR2 : public ExprOperator
{
public:
    D_OR2() : ExprOperator("||",DYADIC,-2) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_AND : public ExprOperator
{
public:
    D_AND() : ExprOperator("AND",DYADIC,-1) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class D_AND2 : public ExprOperator
{
public:
    D_AND2() : ExprOperator("&&",DYADIC,-1) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

//class D_OPENPAREN : public ExprOperator
//{
//public:
//    D_OPENPAREN() : ExprOperator("(",DYADIC,0) {}
//    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
//};

class D_CLOSEPAREN : public ExprOperator
{
public:
    D_CLOSEPAREN() : ExprOperator(")",DYADIC,-20) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

//==============================================================
// Monadic prefix operators
//==============================================================

class MP_NOT : public ExprOperator
{
public:
    MP_NOT() : ExprOperator("!",MONADIC_PREFIX,1) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class MP_MINUS : public ExprOperator
{
public:
    MP_MINUS() : ExprOperator("-",MONADIC_PREFIX,4) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class MP_OPENPAREN : public ExprOperator
{
public:
    MP_OPENPAREN() : ExprOperator("(",MONADIC_PREFIX,-10) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

#if 0
class MP_RECURRENCE : public ExprOperator
{
public:
    MP_RECURRENCE() : ExprOperator("f",MONADIC_PREFIX,4) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};

class MP_PRIME : public ExprOperator
{
public:
    MP_PRIME() : ExprOperator("p",MONADIC_PREFIX,4) {}
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
};
#endif

//==============================================================
// Monadic suffix operators
//==============================================================

class MS_FACT : public ExprOperator
{
	ExprOperator *m_pMutant;
public:
    MS_FACT();
    ~MS_FACT();
    
    MS_FACT(const MS_FACT&);
    MS_FACT& operator=(const MS_FACT&);
    
    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
    virtual ExprOperator *Mutant() const;
};

class MS_PRIM : public ExprOperator
{
	ExprOperator *m_pMutant;
public:
    MS_PRIM();
    ~MS_PRIM();

    MS_PRIM(const MS_PRIM&);
    MS_PRIM& operator=(const MS_PRIM&);

    virtual PFBoolean evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m=0); // perform the op
    virtual ExprOperator *Mutant() const;
};

//==============================================================
// The workhorse
//==============================================================

PFBoolean ex_matchparen(const PFString &);
PFBoolean ex_cleanup(const PFString &,PFString &);
PFString ex_display(const PFString &);
void ex_destroyRecurrence();


PFBoolean ex_getchain(const PFString &c,unsigned long &a,long &b);
PFString ex_setchain(unsigned long a,long b);

//==============================================================
// lib call items.
//==============================================================

typedef struct libraryitem
{
	LPCTSTR name;
	Integer (*calculator)(unsigned long b,unsigned long k,unsigned long n);
	PFString (*symbolism)(unsigned long b,unsigned long k,unsigned long n);
}
LibraryItem;

Integer fib(unsigned long n);
Integer fib_primitive_calculator(unsigned long b,unsigned long k,unsigned long n);
PFString fib_primitive_symbol(unsigned long b,unsigned long k,unsigned long n);

Integer fib_prim_calculator(unsigned long b,unsigned long k,unsigned long n);
PFString fib_prim_symbol(unsigned long b,unsigned long k,unsigned long n);

Integer fib_full_calculator(unsigned long b,unsigned long k,unsigned long n);
PFString fib_full_symbol(unsigned long b,unsigned long k,unsigned long n);

// tokenizing scan equates
#define TS_NORMAL	0
#define	TS_STRING	1
#define	TS_SYMBOL	2
#define	TS_DYADIC	3

// symbol definition functions
PFBoolean IsFirstSymbolCharacter(TCHAR c);
PFBoolean IsSymbolCharacter(TCHAR c);
PFBoolean IsIntegerCharacter(TCHAR c);

#endif
