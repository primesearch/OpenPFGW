#ifndef F_FIBONACCI_H
#define F_FIBONACCI_H

#include "pffunctionsymbol.h"

class F_Fibonacci : public PFFunctionSymbol
{
protected:
	F_Fibonacci(const PFString &sName);
public:
	DWORD MinimumArguments() const;
	DWORD MaximumArguments() const;
	DWORD GetArgumentType(DWORD dwIndex) const;
	PFString GetArgumentName(DWORD dwIndex) const;
};

class F_Fibonacci_U : public F_Fibonacci
{
public:
	F_Fibonacci_U();
	PFBoolean CallFunction(PFSymbolTable *pContext);
};

class F_Fibonacci_V : public F_Fibonacci
{
public:
	F_Fibonacci_V();
	PFBoolean CallFunction(PFSymbolTable *pContext);
};

class F_Fibonacci_F : public F_Fibonacci
{
public:
	F_Fibonacci_F();
	PFBoolean CallFunction(PFSymbolTable *pContext);
};

class F_Fibonacci_L : public F_Fibonacci
{
public:
	F_Fibonacci_L();
	PFBoolean CallFunction(PFSymbolTable *pContext);
};

#endif
