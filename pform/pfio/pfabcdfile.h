// abcdfile.h  a derived class from PFABCFile class for files with variables. 
// This is a "delta" compressed format for smaller sieved files.

#if !defined (__PFABCDFile_H__)
#define __PFABCDFile_H__

#include "pfabcfile.h"

class PFABCDFile : public PFABCFile
{
	public:
		PFABCDFile(const char* FileName);
		~PFABCDFile();

		int SeekToLine(int LineNumber);
		// GetNextLine is used from the parent PFABCFile.  We overload the readline function, and "fixup" the line read there.
//		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0);

	protected:
		void LoadFirstLine();
		void CutOutFirstLine();
		int  ReadLine(char *Line, int sizeofLine);

		int64_t 	m_i64Accum[26];
		uint32_t	m_nAccum;
		bool  	m_bReadNextLineFromFile;
		bool  	m_bIgnoreOutput;
		char    *m_ABCLookingLine;
      
      char    *m_Line1;
      char    *m_TempLine;

	private:
};


#endif
