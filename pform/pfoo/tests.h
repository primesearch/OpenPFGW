// derived testing classes are derived from PFFunctionSymbol.
// To call a test, just call its function.

// note the function call syntax here is not precise. It can be
// done with ex_evaluate, but that currently only handles integer
// return types.

// modes
// standard overrides
// FACTOROPTION 	negative	internal multiplier
//						0 - none (minimal)
//						positive	pmax
#include "pffunctionsymbol.h"

#if 0
class T_PRP : public PFFunctionSymbol
{
public:
	T_PRP();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dw) const;
	PFString GetArgumentName(DWORD dw) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
	
// PRP test
// syntax isPRP(_N)
// returns: 0 or 1
};
#endif

class T_Pocklington : public PFFunctionSymbol
{
public:
	T_Pocklington();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dw) const;
	PFString GetArgumentName(DWORD dw) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};

class T_Morrison : public PFFunctionSymbol
{
public:
	T_Morrison();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dw) const;
	PFString GetArgumentName(DWORD dw) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};

class T_Combined : public PFFunctionSymbol
{
public:
	T_Combined();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dw) const;
	PFString GetArgumentName(DWORD dw) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
};

#if 0
class T_Decimal : public PFFunctionSymbol
{
// decimal expansion
// synax decimal(_N,OUTFILE)
// returns	0 - failure
//				1 - success
};



class M_Standard : public PFFunctionSymbol
{
// standard mode
// syntax standardmode(NMIN,NMAX,NSTEP,KMIN,KMAX,KSTEP,EXPRESSION,MODE)
// passes N as the parameter to MODE
// maintains current values in N,K
};

class M_File : public PFFunctionSymbol
{
// file mode
// syntax filemode(INFILE,EXPRESSION,MODE)
};

class M_Verifier : public PFFunctionSymbol
{
// verifier mode
// syntax verifiermode(INFILE,EXPRESSION,MODE)
};

class M_Interactive : public PFFunctionSymbol
{
// interactive mode
// syntax interactivemode(COMMANDLINE)
// stores all expression results in the local symbol table
// - ie DOES NOT CALL ex_evaluate
};

// console updates.
// symbols are given a symbol trigger. Controls can be attached to symbols.
// worry about that later
#endif
