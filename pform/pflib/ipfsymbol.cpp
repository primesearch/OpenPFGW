// The interface class for an IPFSymbol
#include "pflibpch.h"
#include "ipfsymbol.h"

IPFSymbol::IPFSymbol(const PFString &sKey)
	: m_sKey(sKey)
{
	m_sKey.ToUpper();
}

IPFSymbol::IPFSymbol(const IPFSymbol &s)
	: m_sKey(s.m_sKey)
{
}

IPFSymbol& IPFSymbol::operator=(const IPFSymbol &s)
{
	m_sKey=s.m_sKey;
	return *this;
}

IPFSymbol::~IPFSymbol()
{
}

const PFString &IPFSymbol::GetKey() const
{
	return m_sKey;
}

void IPFSymbol::SetKey(const PFString &s)
{
	m_sKey=s;
	m_sKey.ToUpper();
}
