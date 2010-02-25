#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

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
