#include "pfoopch.h"
#include "symboltypes.h"
#include "pfstringsymbol.h"

PFStringSymbol::PFStringSymbol(const PFString &sKey,const PFString &sValue)
   : IPFSymbol(sKey), m_sValue(sValue)
{
}

PFString PFStringSymbol::GetStringValue()
{
   return m_sValue;
}

void PFStringSymbol::SetValue(const PFString &sValue) {
   m_sValue=sValue;
}

DWORD PFStringSymbol::GetSymbolType() const
{
   return STRING_SYMBOL_TYPE;
}
