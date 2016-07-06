// pfdecimalfile.h  a derived class from PFSimpleFile class for files with variables

#if !defined (__PFDecimalFile_H__)
#define __PFDecimalFile_H__

#include "pffile.h"

class PFDecimalFile : public PFSimpleFile
{
	public:
		PFDecimalFile(const char* FileName);
		~PFDecimalFile();
		int SeekToLine(int LineNumber);
		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0, PFSymbolTable *psymRuntime=0);
      
	protected:
		void LoadFirstLine();
		void ProcessFirstLine(char *Line);
      void Printf_WhoAmIsString() {}
      PFString SignatureString() {return "DECIMAL_FILE";}

	private:
      char *m_nDecimalLine;
      char *m_nNextLine;

};


#endif
