#if !defined (__PFCCFile_H__)
#define __PFCCFile_H__

#include "pffile.h"

#define CCLINELEN 100010

#define CCMAXLENGTH 30
#define CCMAXVAR    26

class PFCCFile : public PFSimpleFile
{
public:
	PFCCFile(const char* FileName);
	~PFCCFile();
	int SeekToLine(int LineNumber);
	int GetNextLine(PFString& sLine, Integer* i = 0, bool* b = 0, PFSymbolTable* psymRuntime = 0);

	void CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool* p_bMessageStringIsValid = 0, PFString* p_MessageString = 0);

protected:
	void LoadFirstLine();
	void ProcessFirstLine(char* Line);
	void Printf_WhoAmIsString() { PFOutput::EnableOneLineForceScreenOutput(); PFPrintfStderr("%s\n", (const char*)m_SigString); }	// you may want to un-inline this
	PFString SignatureString() { return "CC_FILE"; }

	void MakeExpr(PFString& sLine);
	void LoadModularFactorString();	// very similar to MakeExpr, but it loads up the string for the Erat_Mod if it was requested.

	int LetterNumber(char Letter);
	char* sFormat[CCMAXLENGTH];  //Make the maximum number of espressions is ABCMAXEXPR.
	char* m_Line;

	enum { e_And, e_Or, e_Invalid };

	int m_eAndOr[CCMAXLENGTH + 1];   // Set to e_And or e_Or
	int m_nLastLetter;
	int m_nExprs;
	int m_nCurrentMultiPrime;
	int m_nCurrentMultiLine;
	int m_nCurrentPrimeCount;
	bool m_bLastNumberPrime;
	bool m_bLastLineAnd;

	char* s_array[26]; // Holds variables as strings

	char m_szCommentData[256];
	char m_szModFactor[256]; // For use giving the ABC2 file the ability to add modular factors based on the parameters of the file

private:
	void RemoveExpressions(char* pMSS, bool bCheckUsingConditionSyntax);
	bool  m_bInitializedTable;
	PFSymbolTable m_tbl;
};


#endif
