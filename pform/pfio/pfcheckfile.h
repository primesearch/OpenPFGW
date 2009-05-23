#if !defined (__PFCheckFile_H__)
#define __PFCheckFile_H__

#include "pffile.h"

class PFCheckFile : public PFSimpleFile
{
	public:
		PFCheckFile(const char* FileName);
		~PFCheckFile();

		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0);

		void CurrentNumberIsPrime(bool bIsPrime, bool *p_bMessageStringIsValid=0, PFString *p_MessageString=0);

		void Printf_WhoAmIsString() {PFOutput::EnableOneLineForceScreenOutput();PFPrintfStderr("Check File\n");}	// you may want to un-inline this
		PFString SignatureString() {return "CHECK_FILE";}

	private:
		uint64 m_nResidue;
		bool m_bPrime;
};

#endif
