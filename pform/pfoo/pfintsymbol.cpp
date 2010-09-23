#include "pfoopch.h"
#include "symboltypes.h"
#include "pfintsymbol.h"

PFIntSymbol::PFIntSymbol(const PFString &sKey,int i)
   : IPFSymbol(sKey), m_i(i)
{
}

PFIntSymbol::PFIntSymbol(const PFIntSymbol &s)
   : IPFSymbol(s), m_i(0)
{
   m_i=s.GetValue();
}

PFIntSymbol::~PFIntSymbol()
{
}

PFIntSymbol& PFIntSymbol::operator=(const PFIntSymbol &s) {
   // allow x=x
   if (this == &s)
      return *this;

   SetKey(s.GetKey());
   m_i=s.GetValue();
   return *this;
}

PFString PFIntSymbol::GetStringValue() {
   PFString str;
   char buff[12];
   sprintf(buff,"%d",m_i);
   str=buff;
   return str;
}

DWORD PFIntSymbol::GetSymbolType() const {
   return INT_SYMBOL_TYPE;
}

int PFIntSymbol::GetValue() const { return m_i; }

void PFIntSymbol::SetValue(int i) { m_i=i; }
