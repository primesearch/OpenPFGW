#ifndef F_SEQUENCE_H
#define F_SEQUENCE_H

#include "pffunctionsymbol.h"

class F_Sequence : public PFFunctionSymbol
{
public:
	F_Sequence();
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
	PFBoolean CallFunction(PFSymbolTable *pContext);
private:
	static PFBoolean LinearSolve(	const Integer &A,const Integer &B,const Integer &P,
									const Integer &C,const Integer &D,const Integer &Q,
									Integer &X,Integer &Y);
	
};

class F_LucasType : public PFFunctionSymbol
{
public:
	F_LucasType(const PFString &sName);
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
};

class F_LucasV : public F_LucasType
{
public:
	F_LucasV();
	PFBoolean CallFunction(PFSymbolTable *pContext);
};	

class F_LucasU : public F_LucasType
{
public:
	F_LucasU();
	PFBoolean CallFunction(PFSymbolTable *pContext);
};	

class F_PrimV : public F_LucasType
{
public:
	F_PrimV();
	PFBoolean CallFunction(PFSymbolTable *pContext);
};	

class F_PrimU : public F_LucasType
{
public:
	F_PrimU();
	PFBoolean CallFunction(PFSymbolTable *pContext);
};	
	
class F_NSWType : public PFFunctionSymbol
{
public:
	F_NSWType(const PFString &sName);
	
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
};

class F_NSW_S : public F_NSWType
{
public:
	F_NSW_S();
	PFBoolean CallFunction(PFSymbolTable *pContext);
};	

class F_NSW_W : public F_NSWType
{
public:
	F_NSW_W();
	PFBoolean CallFunction(PFSymbolTable *pContext);
};	

#endif
