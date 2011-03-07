// PrZfile.h  a derived class from PFABCDFile/NewPGen class for files with variables. 
// This is a "delta" compressed format for smaller sieved files.

#if !defined (__PFPrZFile_H__)
#define __PFPrZFile_H__

#include "pfabcfile.h"
#include "pfnewpgenfile.h"

#include "prz.h"

class PFPrZFile : public PFABCFile
{
	public:
		PFPrZFile(const char* FileName);
		~PFPrZFile();

		int SeekToLine(int LineNumber);
		// GetNextLine is used from the parent PFABCFile.  We overload the readline function, and "fixup" the line read there.
//		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0);

	protected:
		void LoadFirstLine();
		void CutOutFirstLine();
		int  ReadLine(char *Line, int sizeofLine);

		int64	m_i64Accum;
		uint64  m_MinNum, m_MaxNum, m_KOffset, prz_nvalsleft;
		bool	m_bReadNextLineFromFile;
		bool	m_bIgnoreOutput;
		char	m_ABCLookingLine[ABCLINELEN];
		PrZ_Section_Header_Base *pFileHead;

	private:
};

class PFPrZ_newpgen_File : public PFNewPGenFile
{
	public:
		PFPrZ_newpgen_File(const char* FileName);
		~PFPrZ_newpgen_File();

		int SeekToLine(int LineNumber);
		// GetNextLine is used from the parent PFABCFile.  We overload the readline function, and "fixup" the line read there.
//		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0);

		// gw_Gapper uses this class, and not the PFGenericFile class, so I need to export a couple of things
		uint64 MinNum() { return m_MinNum; }
		uint64 MaxNum() { return m_MaxNum; }
		uint64 nValsLeft() {return prz_nvalsleft;}

	protected:
		void LoadFirstLine();
		int  ReadLine(char *Line, int sizeofLine);
 
		uint64	m_i64Accum;
		uint64  m_MinNum, m_MaxNum, m_KOffset, prz_nvalsleft;
		uint32	m_nAccum;
		bool	m_bReadNextLineFromFile;
		bool	m_bIgnoreOutput;
		char	m_NPGLookingLine[256];

	private:
};

#endif
