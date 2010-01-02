#ifdef _MSC_VER // All network code is MSVC specific (currently anyway)

// Network "files" allow one machine to act as a server distributing numbers to a number of other machines for testing.

#if !defined (__PFNetworkFile2_H__)
#define __PFNetworkFile2_H__

#include "pffile.h"
#include "tcpip_client2.h"

class PFNetworkFile2 : public PFSimpleFile
{
	public:
		PFNetworkFile2(const char* FileName);
		~PFNetworkFile2();

		int SeekToLine(int LineNumber);
		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0, PFSymbolTable *psymRuntime=0);

		void CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool *p_bMessageStringIsValid=0, PFString *p_MessageString=0);

   protected:
		PFSimpleFile *pf;
		tcpip_client2 *cli;
		uint32 ExprsToGet, maxExpr;
		int NoDataTimeoutMin;
		char Host[256];
		unsigned short  Port;
		DWORD m_WaitTil;
		bool m_bSendCompositesAlso;
		bool m_bSendResidues;
		bool m_bVerboseMode;
		bool m_bAbortMode;
		char m_szWhoAmI[40];
		char *m_SendStr;	/* remove this at a later time */
		FILE *m_fpSendStr;

		void SendIfTimedOut();
		void LoadFirstLine();
		void Printf_WhoAmIsString() {PFOutput::EnableOneLineForceScreenOutput();PFPrintfStderr("Network FileV2\n");}
		PFString SignatureString() {return "NETWORK_FILE";}			// Leave same signature as the Network File v1 code (so that resume will NOT be attempted)

		/* Here are new "functions" to do the transmission and preparatory work */
		/* This used to be a HUGE GetNextLine() function */
		bool ConnectToServer();
		bool LogoutOfServer();
		bool SendRequestToServer(char *Str);
		bool SendResultsToServer();
		bool GetResponseFromServer();
		bool PrepServerWithDataRequest();
		bool RequestDataFromServer();
		bool TryAgain_in60seconds();
		bool TryAgain_in5seconds();
};

#endif
#endif
