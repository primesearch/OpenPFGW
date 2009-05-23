// abcfile.h  a derived class from PFSimpleFile class for files with variables

#if !defined (__PFABCFile_H__)
#define __PFABCFile_H__

#include "pffile.h"
#include "pfabctaskcompleted.h"

#define ABCLINELEN 60000

#define ABCMAXEXPR 200
#define ABCMAXVAR  26

class PFABCFile : public PFSimpleFile
{
	public:
		PFABCFile(const char* FileName);
		~PFABCFile();
		int SeekToLine(int LineNumber);
		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0);

		void CurrentNumberIsPrime(bool bIsPrime, bool *p_bMessageStringIsValid=0, PFString *p_MessageString=0);


	protected:
		void LoadFirstLine();
		void ProcessFirstLine(char *Line);
		void Printf_WhoAmIsString() {PFOutput::EnableOneLineForceScreenOutput();PFPrintfStderr("%s\n", (const char*)m_SigString);}	// you may want to un-inline this
		PFString SignatureString() {return "ABC_FILE";}
		// Over-ridden in a ABC type file so that // {number_primes,$b,1} and // {number_comps,$b,1} can work
		virtual bool ProcessThisLine();

		void MakeExpr(PFString &sLine);
		void LoadModularFactorString();	// very similar to MakeExpr, but it loads up the string for the Erat_Mod if it was requested.

		int LetterNumber(char Letter);
		char *sFormat[ABCMAXEXPR];  //Make the maximum number of espressions is ABCMAXEXPR.
		char *Line;

		enum { e_And,e_Or,e_Invalid };
		
		int m_eAndOr[ABCMAXEXPR+1];   // Set to e_And or e_Or
		int m_nLastLetter;
		int m_nExprs;
		int m_nCurrentMultiPrime;
		int m_nCurrentMultiLine;
		int m_nCurrentPrimeCount;
		bool m_bLastNumberPrime;
		bool m_bLastLineAnd;

		//char s_array[26][ABCVARLEN]; // Holds variables as strings
		char *s_array[26]; // Holds variables as strings

		char m_szCommentData[256];
		char m_szModFactor[256]; // For use giving the ABC2 file the ability to add modular factors based on the parameters of the file

		PFABCTaskCompleted *m_pCompleted;

	private:
};


#endif
