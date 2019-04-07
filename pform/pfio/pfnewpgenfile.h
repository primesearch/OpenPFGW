// pfnewpgenfile.h  a derived class from PFSimpleFile class for NewPGen output file processing.

#if !defined (__PFNewPGenFile_H__)
#define __PFNewPGenFile_H__

#include "pffile.h"

// Begin #defines from newpgen2.h
#define	MODE_PLUS			0x0001		// n+1
#define	MODE_MINUS			0x0002		// n-1
#define MODE_2PLUS			0x0004		// 2n+1
#define MODE_2MINUS			0x0008		// 2n-1
#define MODE_4PLUS			0x0010		// 4n+1
#define MODE_4MINUS			0x0020		// 4n-1
#define MODE_PRIMORIAL		0x0040
#define MODE_PLUS5			0x0080
#define MODE_KSIEVE			0x0100
#define MODE_AP				0x0200
#define MODE_NOTGENERALISED 0x0400
#define MODE_PLUS7			0x0800
#define MODE_2PLUS3			0x1000
#define MODE_MOREHEAD		0x2000
// Not sure what 0x4000 is
#define MODE_BASE2FIXEDK	0x8000


#define MASK_2		(MODE_2PLUS | MODE_2MINUS)
#define MASK_4		(MODE_4PLUS | MODE_4MINUS)

#define	MODE_TWIN    (MODE_PLUS | MODE_MINUS)

#define MODE_3TUPLE  (MODE_TWIN | MODE_PLUS5)
#define MODE_4TUPLE  (MODE_TWIN | MODE_PLUS5 | MODE_PLUS7)

#define	MODE_CC      (MODE_PLUS | MODE_2PLUS)
#define	MODE_SG      (MODE_MINUS | MODE_2MINUS)
#define MODE_BI      (MODE_PLUS | MODE_MINUS | MODE_2PLUS | MODE_2MINUS)
#define MODE_SG3	 (MODE_PLUS | MODE_2PLUS3)


#define MODE_IJ		 (MODE_PLUS | MODE_MINUS | MODE_2MINUS)
#define MODE_IJ2	 (MODE_PLUS | MODE_MINUS | MODE_2PLUS)

#define MODE_LP		 (MODE_PLUS | MODE_2PLUS | MODE_2MINUS | MODE_4PLUS)
#define MODE_LM		 (MODE_MINUS | MODE_2MINUS | MODE_2PLUS | MODE_4MINUS)

#define MODE_CC23	 (MODE_PLUS | MODE_2PLUS | MODE_4PLUS)
#define MODE_CC13	 (MODE_MINUS | MODE_2MINUS | MODE_4MINUS)
#define MODE_BI3     (MODE_PLUS | MODE_MINUS | MODE_2PLUS | MODE_2MINUS | MODE_4PLUS | MODE_4MINUS)

#define MODE_PLUSKSIEVE (MODE_PLUS | MODE_KSIEVE)
#define MODE_MINUSKSIEVE (MODE_MINUS | MODE_KSIEVE)
// End #defines from newpgen2.h

class PFNewPGenFile : public PFSimpleFile
{
	public:
		PFNewPGenFile(const char* FileName);
		~PFNewPGenFile();
		int SeekToLine(int LineNumber);
		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0, PFSymbolTable *psymRuntime=0);

		// These virtual functions do something in this class
		int GetKNB(uint64_t &k, uint64_t &n, unsigned &b);
      void CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool *p_bMessageStringIsValid=0, PFString *p_MessageString=0);

	protected:
		// This virtual function does something in this class.
		void LoadFirstLine();
		void ProcessFirstLine(char *FirstLine, uint64_t k, uint64_t n);
		void Printf_WhoAmIsString() {PFOutput::EnableOneLineForceScreenOutput();PFPrintfStderr("%s\n", (const char*)(m_SigString));}
      void Primorial(Integer *p, uint32_t pp);

		// Possible "normal" formats.  NOTE that b^n+2k-1 changes the order of the k, b and n vars
		//k*b^n+1
		//k*b^n-1
		//k*b^(n+1)+1
		//k*b^(n+1)-1
		//k*b^(n-1)+1
		//k*b^(n-1)-1
		//k*b^(n+2)+1
		//k*b^(n+2)-1
		//k*b^(n+3)+1
		//k*b^(n+3)-1
		//k*b^(n+4)+1
		//k*b^(n+4)-1
		//b^n+2k-1
		//2*k*b^n+1     BiTwin correct for non base-2 search		(Not used)
		//2*k*b^n-1     BiTwin correct for non base-2 search		(Not used)
		//(k*b^n)/2-1   BiTwin correct (long) for non base-2 search	(Not used)
		//(k*b^n)/2-1   BiTwin correct (long) for non base-2 search	(Not used)
		// Extra stuff for the non-generalised chains
		//2k*b^n+1
		//2k*b^n-1
		//4k*b^n+1
		//4k*b^n-1
		//8k*b^n+1
		//8k*b^n-1
		//16k*b^n+1
		//16k*b^n-1
		// Extra stuff for the 3-tuple/4-tuple
		//k*b^n+5
		//k*b^n+7
		// This is for the SG of the form k.b^n+1, 2k.b^n+3 
		//2k*b^n+3
		// This is for fixed k dual Sierpinski search
		//2^n+k
		//2^n-k
		static char NormFormats[30][22];

		// Possible "Primorial" formats.  NOTE that n#+2k-1 changes the order of the k and n vars
		//k*n#+1
		//k*n#-1
		//2*k*n#+1
		//2*k*n#-1
		//(k*n#)/2+1
		//(k*n#)/2-1
		//4*k*n#+1
		//4*k*n#-1
		//8*k*n#+1
		//8*k*n#-1
		//16*k*n#+1
		//16*k*n#-1
		//n#+2k-1
		// first 13 are in the same order as the non-primorial.  The next set adds the k-tuple formats
		//(k*n#)/5+1
		//(k*n#)/5-1
		//(k*n#)/5+5
		//(k*n#)/35+1
		//(k*n#)/35-1
		//(k*n#)/35+5
		//(k*n#)/35+7
		// Extra stuff for the primoproth searches
		//k#*2^n+1
		//k#*2^n-1
		static char PrimorialFormats[22][21];

		uint64_t m_k, m_n;	// current k, n, values.  (Note that SG, CC and others may have these values different than the number being generated).
		uint64_t m_base_n;	// read from the first line.  We check against this when building the Integer, to make sure that the current n value is "still" equal to this.
		bool m_bGoOnToNextMuli;
		bool m_bPrimorial;
		bool m_bLuckyPrime;		// has the "bitmap" of a BiTwin, but is not 2 twins.
		bool m_bBiTwinPrime;	// the "bitmap" of a BiTwin confuses CC searches
		bool m_bConsecutive;	// swiches order of k, n, b
		bool m_bMultiPrimeSearch;
		int  m_nCurrentMultiPrime;
		int  m_nNumMultiPrime;
		char *m_pFormat[10];			// can handle up to an 10 prime multi-prime search (BiTwin len5 is 10, CC len10 is 10, but we only allow CC len 5 ...)

		char m_npg_c;
		int  m_npg_len, m_npg_b, m_npg_bitmap;

		Integer *m_pBaseInteger;
		Integer m_TmpInteger;
		Integer m_Integer_one, m_Integer_neg_one, m_Integer_five, m_Integer_seven;
		bool    m_bBaseIntegerValid;

	private:
		// effective C++ requires these overrides.
		// They are declared, but not actually defined here
		PFNewPGenFile(const PFNewPGenFile&);
		PFNewPGenFile& operator=(const PFNewPGenFile&);
};

class PFNewPGenDeepFile : public PFNewPGenFile
{
	public:
		PFNewPGenDeepFile(const char* FileName);
		~PFNewPGenDeepFile();

		void CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool *p_bMessageStringIsValid=0, PFString *p_MessageString=0);
		int GetNextLine(PFString &sLine, Integer *i=0, bool *b=0, PFSymbolTable *psymRuntime=0);

	protected:
		void LoadFirstLine();

	private:
		bool m_bSpecialDeepFactoringNeeded;
		bool m_bSimpleDeepFactoringNeeded;
		bool m_bPrimesFound[10];
		void ResetFoundPrimes() {memset(&m_bPrimesFound, 1, sizeof(m_bPrimesFound)); }
		enum eSearchType { eunk, e4tuple, ebitwin, eluckyp, eluckym, etwinsg, etwincc, etwincc_c1, etwincc_c2, ebitwin_c };
		eSearchType m_eSearchType;

};

#endif

