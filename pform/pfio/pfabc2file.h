#if !defined (__PFABC2File_H__)
#define __PFABC2File_H__

#include "pfabcfile.h"

class PFABC2File : public PFABCFile
{
	public:
		PFABC2File(const char* FileName);
		~PFABC2File();
		
		int SeekToLine(int LineNumber);
		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0, PFSymbolTable *psymRuntime=0);

	protected:
		void LoadFirstLine();

		enum { e_Norm,e_Prime,e_NormDown,e_In };
		int m_eRangeType[26];
		char **m_pSet[26];
		int m_nSetNum[26];
		int m_nFirstPrime;  // The number of the first variable using primes.
		double array[26];
		double min[26];  //The bottom of the range for each variable
		double max[26];  //The top of the range for each variable
		double step[26]; //The step length for each variable

	private:
};

#endif
