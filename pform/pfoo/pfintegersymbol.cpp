#include "pfoopch.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"

F_Decimal*		PFIntegerSymbol::m_pFuncDecimal=NULL;
PFSymbolTable*	PFIntegerSymbol::m_pSymDecimal=NULL;

PFIntegerSymbol::PFIntegerSymbol(const PFString &sKey,Integer *pValue, bool bShallowSave)
	: IPFSymbol(sKey), m_pInteger(pValue), m_bShallowSave(bShallowSave)
{
}

PFIntegerSymbol::PFIntegerSymbol(const PFIntegerSymbol &s)
	: IPFSymbol(s), m_pInteger(NULL), m_bShallowSave(false)
{
	// copy constructor is always a "deep" copy.
	if(s.m_pInteger)
	{
		m_pInteger=new Integer(*s.m_pInteger);
	}
}

PFIntegerSymbol& PFIntegerSymbol::operator=(const PFIntegerSymbol &s)
{
	// allow x=x
	if (this == &s)
		return *this;
	// assign operator is always a "deep" copy.
	m_bShallowSave = false;
	if(m_pInteger)
	{
		delete m_pInteger;
		m_pInteger=NULL;
	}
	PFString sKey=s.GetKey();
	SetKey(sKey);

	if(s.m_pInteger)
	{
		m_pInteger=new Integer(*s.m_pInteger);
	}
	
	return *this;
}

PFIntegerSymbol::~PFIntegerSymbol()
{
	if(m_pInteger  && !m_bShallowSave)
	{
		delete m_pInteger;
	}
}

PFString PFIntegerSymbol::GetStringValue()
{
	PFString sRetval="";
	// we cheat a little bit
	
	PFIntegerSymbol iSymbol("_N",m_pInteger);
	m_pSymDecimal->AddSymbol(&iSymbol);
	if(m_pFuncDecimal->CallFunction(m_pSymDecimal))
	{
		IPFSymbol *pResult=m_pSymDecimal->LookupSymbol("_expansion");
		if(pResult && pResult->GetSymbolType()==STRING_SYMBOL_TYPE)
		{
			sRetval=pResult->GetStringValue();
		}
	}
	m_pSymDecimal->RemoveSymbol("_N");
	iSymbol.m_pInteger=NULL;
	
	return sRetval;	
}

DWORD PFIntegerSymbol::GetSymbolType() const
{
	return INTEGER_SYMBOL_TYPE;
}

Integer *PFIntegerSymbol::DuplicateValue() const
{
	return new Integer(*m_pInteger);
}

Integer *PFIntegerSymbol::GetValue() const
{
	return m_pInteger;
}

Integer *PFIntegerSymbol::SetValue(Integer *p)
{
	Integer *r=m_pInteger;
	m_pInteger=p;
	return r;
}

void PFIntegerSymbol::Startup()
{
	m_pFuncDecimal=new F_Decimal;
	m_pSymDecimal=new PFSymbolTable;
}

void PFIntegerSymbol::Shutdown()
{
	delete m_pSymDecimal;
	delete m_pFuncDecimal;
}

