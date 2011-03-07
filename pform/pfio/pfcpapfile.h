// PFCPAPfile.h  a derived class from PFSimpleFile class for JF CPAP output file processing.

#if !defined (__PFCPAPFile_H__)
#define __PFCPAPFile_H__

#include "pffile.h"

// This is a special file output from my (Jim Fougeron's) CAPSieve.exe and find_apn.exe utilities,
// which are designed specifically to find CPAP primes.

// Format of first line       "JF CPAP-5 base^exp\n"
// Format of the data lines:  "BaseInc Gap\n"


class PFCPAPFile : public PFSimpleFile
{
	public:
		PFCPAPFile(const char* FileName);
		~PFCPAPFile();
		int SeekToLine(int LineNumber);
		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0, PFSymbolTable *psymRuntime=0);

		// This virtual function does something in this class
		void CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool *p_bMessageStringIsValid=0, PFString *p_MessageString=0);

	protected:
		// This virtual function does something in this class.
		void LoadFirstLine();
		void Printf_WhoAmIsString();

		int m_nCPAPFold;
		int m_nBase;
		int m_nExp;
		int m_nGap;
		double m_fBaseInc;
		bool m_bLastNumberPrime;
		int  m_nCurNumFound;
		static int m_nStartingPoint[13];
		int m_nCurHi, m_nCurLo;
		int m_FoundPrimeGaps[12];

		enum {e_cur, e_lo, e_hi, e_verifyCPAP} m_eSearching;

	private:
		void SwitchToHi();
		void SwitchToVerify();
		void BuildFactorizeBase();
		inline int powNmodP(int p);
		void SwitchToLo();
		void SwitchToNextLine();
		double *m_pdIntervalData;
		double m_LastCompositeNum;
		int *m_pnFactors;
		int m_nNumIntervalData, m_nCurTestIntervalData;
		static int SimpleIntArraySortCompare(const void *x, const void *y);

		Integer *m_pBaseInteger;
		Integer m_TmpInteger;

		enum {eNumPrimesToFactorWith=100000};
		int m_FactorizeBase[eNumPrimesToFactorWith];
		bool m_bOddBase;
		PFString sWhoAmIString;

		// effective C++ requires these overrides.
		// They are declared, but not actually defined here
		PFCPAPFile(const PFCPAPFile&);
		PFCPAPFile& operator=(const PFCPAPFile&);
};


#endif
