// file symbols
#ifndef PFFILESYMBOL_H
#define PFFILESYMBOL_H

#include "pffile.h"

#if !defined (_WIN_COPY_ONLY_)
#include "pflib.h"
#endif

class PFInputFileSymbol : public IPFSymbol
{
public:
	PFInputFileSymbol(const PFString &sKey,PFSimpleFile *pFile);
	~PFInputFileSymbol();
	
	PFString GetStringValue();
	DWORD GetSymbolType() const;
	
	PFSimpleFile *GetFile();

protected:
	PFSimpleFile *m_pFile;
};

class PFOutputFileSymbol : public IPFSymbol
{
public:
	PFOutputFileSymbol(const PFString &sKey,FILE *pFile);
	~PFOutputFileSymbol();
	
	PFString GetStringValue();
	DWORD GetSymbolType() const;
	
	FILE *GetFile();

protected:
	FILE *m_pFile;
};

#endif
