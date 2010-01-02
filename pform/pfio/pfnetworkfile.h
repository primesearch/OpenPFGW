#ifdef _MSC_VER // All network code is MSVC specific (currently anyway)

// Network "files" allow one machine to act as a server distributing numbers to a number of other machines for testing.

#if !defined (__PFNetworkFile_H__)
#define __PFNetworkFile_H__

#include "pffile.h"
#include "tcpip_client.h"

#define MAXEXPRS 200
#define NETLINELEN 4096

class PFNetworkFile : public PFSimpleFile
{
	public:
		PFNetworkFile(const char* FileName);
		~PFNetworkFile();

		int SeekToLine(int LineNumber);
		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0, PFSymbolTable *psymRuntime=0);

		void CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool *p_bMessageStringIsValid=0, PFString *p_MessageString=0);

   protected:
		tcpip_client *cli;
		char expr[MAXEXPRS][NETLINELEN];
		int curExpr,maxExpr;
		int ExprsToGet;
		void LoadFirstLine();
		void Printf_WhoAmIsString() {PFOutput::EnableOneLineForceScreenOutput();PFPrintfStderr("Network File\n");}
		PFString SignatureString() {return "NETWORK_FILE";}
	private:
//		bool m_bIsBeingResumed;
};

#endif
#endif
