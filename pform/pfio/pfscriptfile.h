// abcfile.h  a derived class from PFSimpleFile class for files with variables

#if !defined (__PFScriptFile_H__)
#define __PFScriptFile_H__

#include "pffile.h"
#include "pffilesymbol.h"

#if !defined (_WIN_COPY_ONLY_)  // We need pfoo for various things.
#include "pfoo.h"
#endif

class PFScriptFile : public PFSimpleFile {
	public:
		PFScriptFile(const char* FileName);
		~PFScriptFile();

			// SeekToLine is not supported, but must override to stop it seeking about the file!
		int SeekToLine(int LineNumber);

		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0, PFSymbolTable *psymRuntime=0);

		void CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool *p_bMessageStringIsValid=0, PFString *p_MessageString=0);

	protected:
		void LoadFirstLine();
		void Printf_WhoAmIsString() {PFOutput::EnableOneLineForceScreenOutput();PFPrintfStderr("Script File\n");}	// you may want to un-inline this
		PFString SignatureString() {return "SCRIPT_FILE";}

		bool doCommand(char *cmd, char *args);
		
		bool Dim(char *args);
		bool Set(char *args);
		bool DimS(char *args);
		bool SetS(char *args);
		bool Goto(char *args);
		bool Gosub(char *args);
		bool Return(char *args);
		bool Factorize(char *args);
//		bool Label(char *args);  // this function does not exist.  A label actually does nothing
		bool If(char *args);
		bool Powmod(char *args);
		bool Print(char *args);
		bool Write(char *args);
		bool OpenFile(char *args);
		bool OpenOutFile(char *args);
		bool OpenAppFile(char *args);
		bool GetNext(char *args);
		bool CloseFile(char *args);
		bool Shell(char *args);
		bool StrToInt(char *args);

		bool BadCommand();  // called to output error on bad parsed command.
		char *FindVarName(char *varname);  // splits out the varname and normalizes it

		PFSymbolTable *m_pTable;

		char **script;
		int m_nNumLines;
		int m_nInstrPtr;

		int m_GosubLevel;

	private:
};

#endif
