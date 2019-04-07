#if !defined (__PFCheckFile_H__)
#define __PFCheckFile_H__

#include "pffile.h"

class PFCheckFile : public PFSimpleFile
{
	public:
		PFCheckFile(const char* FileName);
		~PFCheckFile();

		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0, PFSymbolTable *psymRuntime=0);

		void CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool *p_bMessageStringIsValid=0, PFString *p_MessageString=0);

		void Printf_WhoAmIsString() {PFOutput::EnableOneLineForceScreenOutput();PFPrintfStderr("Check File\n");}	// you may want to un-inline this

	private:
		uint64_t m_nResidue;
		bool m_bPrime;
};

#endif
