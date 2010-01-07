#include "pfiopch.h"
#include <string.h>
#include "pfabcfile.h"

#include "../pfoo/symboltypes.h"
#include "../pfoo/pffunctionsymbol.h"
#include "../pfoo/pfintegersymbol.h"
#include "../pfoo/f_prime.h"
#include "../pfoo/f_fibonacci.h"
#include "../pfoo/f_repunit.h"
#include "../pfoo/f_cyclotomic.h"
#include "../pfoo/f_gcd.h"
#include "../pfoo/f_binomial.h"
#include "../pfoo/f_trivial.h"
#include "../pfoo/f_factor.h"
#include "../pfoo/f_vector.h"
#include "../pfoo/f_issquare.h"
#include "../pfoo/f_smarandache.h"
#include "../pfoo/f_sequence.h"

Integer *ex_evaluate(PFSymbolTable *pContext,const PFString &e,int m);

extern char g_ModularSieveString[256];

int PFABCFile::LetterNumber(char Letter)
{
	if (Letter>='A' && Letter<='Z')
		return (int)(Letter-'A');
	else if (Letter>='a' && Letter<='z')
		return (int)(Letter-'a');
	else
		return -1;
}		

PFABCFile::PFABCFile(const char* FileName)
	: PFSimpleFile(FileName), Line(0), m_nLastLetter(-1), 
	  m_nExprs(1), m_nCurrentMultiPrime(0), m_nCurrentMultiLine(0),
	  m_nCurrentPrimeCount(0), m_bLastNumberPrime(false),
	  m_bLastLineAnd(false), m_pCompleted(NULL)
{
	int i;
	for (i = 0; i < ABCMAXVAR; ++i)
		s_array[i] = 0;
	for (i = 0; i < ABCMAXEXPR; ++i)
		sFormat[i] = 0;
	Line = new char[ABCLINELEN];
	m_SigString = "ABC File";
}

void PFABCFile::LoadFirstLine()
{
	PFPrintf("Recognized ABC Sieve file: \n");

	if (ReadLine(Line, ABCLINELEN))
	{
		fclose(m_fpInputFile);
		throw "Error, Not a valid ABC Sieve file";
	}

	ProcessFirstLine(Line);

	// don't count this "first" line, we have to "reset" to line 0 numbering.
	m_nCurrentLineNum = 1;
}

// This part of the LoadFirstLine was removed from that function, and placed into it's own function.
// now a PFABCFile derived class can simply "fix" it's first line to look like a ABC line, and 
// call this function, instead of having to re-write this logic each time.  This was done in the
// ABCD file class.  It reads the ABCD line, takes the data that it needs, re-creates the first line
// into the ABC format, and then calls this function to process that "fixed" line.  
void PFABCFile::ProcessFirstLine(char *FirstLine)
{
	int i;
	// Strip off any comment from the first line. Sievers can place a comment on the line, and then 
	// place any state information following that comment.  PFGW should ignore that information.
	char *cpComment = strstr(FirstLine, "//");
	*m_szCommentData = 0;
	*m_szModFactor = 0;


	// These MUST be deleted each time a new first line is parsed!  This was a HUGE memory leak, found
	// when I started to process ABCD files with "imbedded" ABCD lines within the file (i.e. a file
	// that is a concatenation of ABCD files).  These were critical leaks.  They leaked the length of
	// each ABCD expression AND a 60000 byte buffer!  So a couple 1000 such parsings, and the PC crashed
	// due to lack of memory.
	for (i=m_nLastLetter;i>=0;--i)
	{
		delete[] s_array[i];
		s_array[i] = 0;
	}
	for (i=m_nExprs-1;i>=0;--i)
	{
		delete[] sFormat[i];
		sFormat[i] = 0;
	}
	// These need to be reset EACH time ProcessFirstLine is called (which can be more than once during the life of the object). 
	m_nLastLetter = -1;
	m_nExprs = 1;
	m_nCurrentMultiPrime = 0;
	m_nCurrentMultiLine = 0;
	m_nCurrentPrimeCount = 0;
	m_bLastNumberPrime = false;
	m_bLastLineAnd = false;

	if (cpComment)
	{
		strncpy(m_szCommentData, cpComment, sizeof(m_szCommentData));
		m_szCommentData[sizeof(m_szCommentData)-1] = 0;
		*cpComment = 0;

		// Check for modular factoring "signature"
		char *cp = strstr(m_szCommentData, "-f{");
		if (cp)
		{
			char *cp1 = strchr(cp, ' ');
			if (!cp1)
				cp1 = strchr(cp, '\t');
			if (!cp1)
			{
				cp1 = strchr(cp, '\n');
				while(*cp1 == '\n' || *cp1 == '\r')
					cp1--;
			}
			if (cp1 && strchr(cp, '}'))
			{
				strncpy(m_szModFactor, cp, sizeof(m_szModFactor)-1);
				if (sizeof(m_szModFactor) > (size_t)(cp1-cp))
					m_szModFactor[cp1-cp+1] = 0;
				else
					m_szModFactor[sizeof(m_szModFactor)-1] = 0;
			}
		}
		cp = strstr(m_szCommentData, "{number_primes,");
		if (!cp)
			cp = strstr(m_szCommentData, "{number_comps,");
		if (cp)
		{
			try
			{
				delete m_pCompleted;
				m_pCompleted = 0;
				m_pCompleted = new PFABCTaskCompleted(cp);

				char Buf[120];
				sprintf (Buf, " Processing for at most %d %s\n", m_pCompleted->nHowMany(), m_pCompleted->bProcessingForPrimes()?"Primes":"Composites");
				m_SigString += Buf;
			}
			catch(...)
			{
				m_pCompleted = NULL;
			}
		}
	}
	// Eat any trailing whitespace (or newline chars)
	char *cpEnd = &FirstLine[strlen(FirstLine)-1];
	while (cpEnd > FirstLine && (*cpEnd == '\n' || *cpEnd == '\r' || *cpEnd == '\t' || *cpEnd == ' '))
		*cpEnd-- = 0;

	if (strlen(FirstLine)<6) {
		fclose(m_fpInputFile);
		throw "Invalid file.  The first line is not of a valid format!\n";
	}

	int Number;
	char *tempPtr=FirstLine+4;
	char *tempPtr2,*tempPtr3;

	while (1) {
		if ((tempPtr2=strchr(tempPtr,'$'))==NULL)
			break;
		if ((Number=LetterNumber(tempPtr2[1]))>m_nLastLetter)
			m_nLastLetter=Number;
		tempPtr=tempPtr2+2;
	}
	
	for (i=0;i<=m_nLastLetter;i++)
		s_array[i]=new char[ABCLINELEN];

	// ABC2 files were having a ' ' char as the first char of an expression.  The Line+4 is 
	// correct for an ABC first line, but was 1 off for a ABC2 first line.
	while (*tempPtr == ' ' || *tempPtr == '\t')
		tempPtr++;

	tempPtr=FirstLine+4;  // DOS Edit saves | as character code 221.
	while (1) {			// Change these to 124 ('|')
		if ((tempPtr2=strchr(tempPtr,221))==NULL)
			break;
		tempPtr2[0]='|';
	}

	char TooManyInits_ErrBuf[256];
	sprintf (TooManyInits_ErrBuf, "\nCritical Error, TOO many initializers in ABC type file. Max is %d\n", ABCMAXEXPR);
			// Not the most efficient piece of code in the world,
	while (1) {		  // but it will do.
		if ((tempPtr2=strchr(tempPtr,'&'))==NULL)
			break;
		tempPtr2[0]=0;
		while (1) {
			if ((tempPtr3=strchr(tempPtr,'|'))==NULL)
				break;
			tempPtr3[0]=0;
			if (ABCMAXEXPR == m_nExprs)
				throw TooManyInits_ErrBuf;
			sFormat[m_nExprs-1] = new char [strlen(tempPtr)+1];
			strcpy(sFormat[m_nExprs-1],tempPtr);
			m_eAndOr[m_nExprs]=e_Or;
			m_nExprs++;
			tempPtr=tempPtr3+1;
		}
		if (ABCMAXEXPR == m_nExprs)
			throw TooManyInits_ErrBuf;
		sFormat[m_nExprs-1] = new char [strlen(tempPtr)+1];
		strcpy(sFormat[m_nExprs-1],tempPtr);
		m_eAndOr[m_nExprs]=e_And;
		m_nExprs++;
		tempPtr=tempPtr2+1;
	}
	while (1) {
		if ((tempPtr3=strchr(tempPtr,'|'))==NULL)
			break;
		tempPtr3[0]=0;
		if (ABCMAXEXPR == m_nExprs)
			throw TooManyInits_ErrBuf;
		sFormat[m_nExprs-1] = new char [strlen(tempPtr)+1];
		strcpy(sFormat[m_nExprs-1],tempPtr);
		m_eAndOr[m_nExprs]=e_Or;
		m_nExprs++;
		tempPtr=tempPtr3+1;
	}
	if (ABCMAXEXPR == m_nExprs)
		throw TooManyInits_ErrBuf;
	sFormat[m_nExprs-1] = new char [strlen(tempPtr)+1];
	strcpy(sFormat[m_nExprs-1],tempPtr);
	m_eAndOr[m_nExprs]=e_And;
	m_eAndOr[0]=m_eAndOr[1];
}

PFABCFile::~PFABCFile()
{
	delete m_pCompleted;
	delete[] Line;
	int i;
	for (i = ABCMAXEXPR-1; i >= 0; --i)
		delete[] sFormat[i];
	for (i = ABCMAXVAR-1; i >= 0; --i)
		delete[] s_array[i];
}

int PFABCFile::GetNextLine(PFString &sLine, Integer * /*pInt*/, bool *pbIntValid, PFSymbolTable *)
{
	char *tempPtr;
	int i;

SkipThisLine:;

	// we do not return an Integer in the pInt pointer (yet ;)
	if (pbIntValid)
		*pbIntValid = false;
	if (m_bEOF)
	{
		m_sCurrentExpression = "";
		return e_eof;
	}

	bool bStoreThisExpression=false;
	if (m_nCurrentMultiPrime==0) {
		bStoreThisExpression=true;
		m_nCurrentPrimeCount=0;
		if (ReadLine(Line, ABCLINELEN)) {
			if (m_pIni)
				m_pIni->SetFileProcessing(false);
			m_bEOF = true;
			m_sCurrentExpression = "";
			return e_eof;
		}
		// This code will now correctly skip blank lines in the file
		tempPtr=&Line[-1];
		for (i=0;i<=m_nLastLetter;i++)
		{
			if (!tempPtr)
			{
				sLine = "";
				return e_ok;
			}
			tempPtr++;
			if (sscanf(tempPtr,"%s",s_array[i])==EOF)
			{
				sLine = "";
				return e_ok;
			}
			tempPtr=strchr(tempPtr,' ');
		}
		if (bStoreThisExpression)
		{
			m_nCurrentLineNum++;
			if (m_pIni)
				m_pIni->SetFileLineNum(m_nCurrentLineNum);
		}
	}

	if (!m_nCurrentMultiPrime && !ProcessThisLine())
		goto SkipThisLine;

	MakeExpr(sLine);
	if (bStoreThisExpression)
		m_sCurrentExpression = sLine;

	LoadModularFactorString();

	return e_ok;
}

void PFABCFile::MakeExpr(PFString &sLine)
{
	char Buff[ABCLINELEN<<1];
	char Format[ABCLINELEN];
	char *tempPtr,*tempPtr2,*bufPtr;

	strcpy(Format,sFormat[m_nCurrentMultiPrime]);
	tempPtr=Format;
	bufPtr=Buff;
	while (1) {
		if ((tempPtr2=strchr(tempPtr,'$'))==NULL)
			break;
		tempPtr2[0]=0;
		bufPtr+=sprintf(bufPtr,"%s%s",tempPtr,s_array[LetterNumber(tempPtr2[1])]);
		tempPtr=tempPtr2+2;
	}
	strcpy(bufPtr,tempPtr);

	strtok(Buff, "\r\n");  // remove possible trailing crap from line

	char *cp = Buff;
	while (*cp == ' ')
		cp++;
	sLine=cp;
}

int PFABCFile::SeekToLine(int LineNumber)
{
	if (LineNumber < m_nCurrentLineNum)
	{
		fseek(m_fpInputFile, 0, SEEK_SET);
		m_nCurrentPhysicalLineNum = 1;
		ReadLine(Line, ABCLINELEN);
		m_nCurrentLineNum = 1;
		if (m_pIni)
			m_pIni->SetFileProcessing(true);
		m_bEOF = false;
	}
	while (m_nCurrentLineNum < LineNumber)
	{
		if (ReadLine(Line,ABCLINELEN)) {
			if (m_pIni)
				m_pIni->SetFileProcessing(false);
			m_bEOF = true;
			return e_eof;
		}
		m_nCurrentLineNum++;
	}
	if (m_pIni)
		m_pIni->SetFileLineNum(m_nCurrentLineNum);
	return e_ok;
}

void PFABCFile::CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool *p_bMessageStringIsValid, PFString *p_MessageString)
{
	if (m_pCompleted)
		m_pCompleted->AddPrimeOrComposite(s_array, bIsPRP || bIsPrime);

	m_bLastNumberPrime = bIsPrime || bIsPRP;
	if (p_bMessageStringIsValid)
		*p_bMessageStringIsValid = false;
	if (bIsPrime || bIsPRP) {
		m_nCurrentMultiPrime++;
		m_nCurrentPrimeCount++;
	} else {
		if (m_eAndOr[m_nCurrentMultiPrime]==e_Or) {
			if (m_eAndOr[++m_nCurrentMultiPrime]==e_And && m_nCurrentPrimeCount==0) {
				m_nCurrentMultiPrime=0;
				m_nCurrentMultiLine=0;
				return;
			}
		} else {
			m_nCurrentMultiPrime=0;
			m_nCurrentMultiLine=0;
			return;
		}
	}
	if (m_nCurrentMultiPrime<m_nExprs) 
		return;
	m_nCurrentMultiPrime=0;
	if (m_bLastLineAnd) {
		m_nCurrentMultiLine++;
		return;
	}
	if (m_nExprs==1 && m_nCurrentMultiLine<2)
		return;
	if (m_nCurrentPrimeCount==m_nExprs) {
		*p_MessageString = "  - Complete Set -";
		*p_bMessageStringIsValid = true;
	} else if (m_nCurrentPrimeCount>1) {
		*p_MessageString = "  - Partial Set -";
		*p_bMessageStringIsValid = true;
	}
}

// NOTE that pMSS MUST be contained within the g_ModularSieveString array.
static void RemoveExpressions(char *pMSS, bool bCheckUsingConditionSyntax)
{
	if (*pMSS != '{')
		return;
	pMSS++;
	bool bAgain = false;
	if (bCheckUsingConditionSyntax)
	{
		if (*pMSS == 'y' || *pMSS == 'n')
		{
			bAgain = true;
			pMSS += 2;
		}
		else if (*pMSS == 'f' || *pMSS == 'p')
			pMSS += 2;
		else
			throw "Unknown condition within modular factoring expression in the ABC file\n";
	}
	for(;;)
	{
		char *cp = strchr(pMSS, '}');
		if (!cp)
			return;
		char *cp1 = strchr(pMSS, ',');
		if (cp1 && cp1 < cp)
			cp = cp1;	// the comma was preceeding the }
		char *cpTest = pMSS;
		bool bExpr = false;
		while (!bExpr && cpTest < cp)
		{
			if (*cpTest < '0' || *cpTest > '9')
				bExpr = true;
			cpTest++;
		}
		if (bExpr)
		{
			// expression found, now remove it.
			char *head = new char [pMSS-g_ModularSieveString+1];
			strncpy(head, g_ModularSieveString, pMSS-g_ModularSieveString);
			head[pMSS-g_ModularSieveString] = 0;
			char *tail = new char [strlen(cp)+1];
			strcpy(tail, cp);
			char *expr = new char [cp-pMSS+1];
			strncpy(expr, pMSS, cp-pMSS);
			expr[cp-pMSS] = 0;
			// tbl is a "known" leak, but it is a one-time only leak.
			static PFSymbolTable tbl;
			static bool bFirst = true;
			if (bFirst)
			{
				tbl.AddSymbol(new F_Prime);
				tbl.AddSymbol(new F_Fibonacci_U);
				tbl.AddSymbol(new F_Fibonacci_V);
				tbl.AddSymbol(new F_Fibonacci_F);
				tbl.AddSymbol(new F_Fibonacci_L);
				tbl.AddSymbol(new F_Repunit);
				tbl.AddSymbol(new F_Cyclotomic);
				tbl.AddSymbol(new F_GCD);
				tbl.AddSymbol(new F_Binomial);
				tbl.AddSymbol(new F_Sequence);
				tbl.AddSymbol(new F_LucasV);
				tbl.AddSymbol(new F_LucasU);
				tbl.AddSymbol(new F_PrimV);
				tbl.AddSymbol(new F_PrimU);
				tbl.AddSymbol(new F_NSW_S);
				tbl.AddSymbol(new F_NSW_W);
				bFirst = false;
			}

			Integer *N=ex_evaluate(&tbl,expr);
			if (N)
			{
				int n = (*N)&0x7FFFFFFF;
				sprintf (g_ModularSieveString, "%s%d%s", head, n, tail);
				delete N;
			}
			else
				*g_ModularSieveString = 0;
			delete[] expr;
			delete[] tail;
			delete[] head;
		}
		if (!bAgain)
			return;
		pMSS = cp+1;
		bAgain = false;
	}
}

void PFABCFile::LoadModularFactorString()
{
	if (m_szModFactor[0])
	{
		char *cpIn = &m_szModFactor[2];	// skip the -f
		char *cpOut = g_ModularSieveString;
		while (*cpIn && *cpIn != ' ')
		{
			if (*cpIn == '$')
			{
				cpOut += sprintf (cpOut, "%s", s_array[LetterNumber(cpIn[1])]);
				cpIn += 2;
			}
			else
				*cpOut++ = *cpIn++;
		}
		*cpOut = 0;

		// Now remove any expressions if there are any from the factor string
		char *cp = g_ModularSieveString;
		RemoveExpressions(cp, false);
		cp = strchr(&cp[1], '{');
		while (cp)
		{
			RemoveExpressions(cp, true);
			cp = strchr(&cp[1], '{');
		}
	}
}

bool PFABCFile::ProcessThisLine()
{
	// Returns true for now.
	if (!m_pCompleted)
		return true; // if there is no completed object, then simply return true to process the line.
	return m_pCompleted->ProcessThisValue(s_array);
}
