#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "pfiopch.h"
#include "pffilesymbol.h"

PFInputFileSymbol::PFInputFileSymbol(const PFString &sKey,PFSimpleFile *pFile)
   : IPFSymbol(sKey), m_pFile(pFile)
{
}

PFInputFileSymbol::~PFInputFileSymbol()
{
   if (m_pFile)
      delete m_pFile;
}

PFString PFInputFileSymbol::GetStringValue() {
   return GetKey();
}

DWORD PFInputFileSymbol::GetSymbolType() const {
   return INPUT_FILE_SYMBOL_TYPE;
}

PFSimpleFile *PFInputFileSymbol::GetFile() {
   return m_pFile;
}

PFOutputFileSymbol::PFOutputFileSymbol(const PFString &sKey,FILE *pFile)
   : IPFSymbol(sKey), m_pFile(pFile)
{
}

PFOutputFileSymbol::~PFOutputFileSymbol()
{
   if (m_pFile)
      fclose(m_pFile);
}

PFString PFOutputFileSymbol::GetStringValue() {
   return GetKey();
}

DWORD PFOutputFileSymbol::GetSymbolType() const {
   return OUTPUT_FILE_SYMBOL_TYPE;
}

FILE *PFOutputFileSymbol::GetFile() {
   return m_pFile;
}
