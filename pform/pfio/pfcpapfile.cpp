//
//
//   PFCPAPFile   
// 
//   This class is based upon the PFSimpleFile, and adds NewPGen logic
//
//

#include "pfiopch.h"
#include <stdio.h>
#include <string.h>
#include "pfcpapfile.h"


PFBoolean bRequestFactorize;


int PFCPAPFile::m_nStartingPoint[13] = { 0, 0, 0, 2, 2, 3, 3, 4, 4, 5, 5, 5, 6 };

// Here is the format for a JF CPAP text input file:
//
// Format of first line       "JF CPAP-5 base^exp\n"
// Format of the data lines:  "BaseInc Gap\n"


PFCPAPFile::PFCPAPFile(const char* FileName)
   : PFSimpleFile(FileName), m_nCPAPFold(0), m_nBase(0), m_nExp(0),
     m_nGap(0), m_fBaseInc(0.), m_bLastNumberPrime(false), m_nCurNumFound(0), 
     m_nCurHi(0), m_nCurLo(0), m_eSearching(e_cur), m_pdIntervalData(NULL),
     m_LastCompositeNum(0.), m_pnFactors(NULL), m_nNumIntervalData(0),
     m_nCurTestIntervalData(0), m_pBaseInteger(0), m_TmpInteger(0),
     m_bOddBase(false), sWhoAmIString("Unknown CPAP type")
{
}

void PFCPAPFile::Printf_WhoAmIsString()
{
        PFOutput::EnableOneLineForceScreenOutput();
		PFPrintfStderr("%s\n", (const char*)(sWhoAmIString));
}

void PFCPAPFile::LoadFirstLine()
{
    sWhoAmIString = "Recognized CPAPSieve file: ";
	char Line[4096];
	if (ReadLine(Line, sizeof(Line)))
	{
		fclose(m_fpInputFile);
		throw "Error, Not a valid CPAPSieve file";
	}
	Line[sizeof(Line)-1] = 0;

	// e_ok now "parse" what this JF CPAP-? file is.  So that the next lines can be "fixed"

	if (strncmp(Line, "JF CPAP-", 8))
	{
		fclose(m_fpInputFile);
		throw "Invalid JF CPAP-? file.  The first line is not of a valid format!\n";
	}

	m_nCPAPFold = atoi(&Line[8]);

	char *cp = strchr(&Line[8], ' ');
	if (!cp)
	{
		fclose(m_fpInputFile);
		throw "Invalid JF CPAP-? file.  The first line is not of a valid format!\n";
	}

	int count = sscanf(cp, "%d^%d\n", &m_nBase, &m_nExp);
	if (count != 2)
	{
		fclose(m_fpInputFile);
		throw "Invalid JF CPAP-? file.  The first line is not of a valid format!\n";
	}
	strtok(cp, "\r\n");
	sWhoAmIString += cp;

	// Ok create the "base" number
	PFSymbolTable *psymTmp=new PFSymbolTable;
	m_pBaseInteger = ex_evaluate(psymTmp,cp);
	delete psymTmp;

	// Ok, this is a valid file, and we are now ready for the first line data.

	// Start of my CPAPSieve sieving project.
	BuildFactorizeBase();

	// don't count this "first" line, we have to "reset" to line 0 numbering.
	m_nCurrentLineNum = 1;
}

PFCPAPFile::~PFCPAPFile()
{
	delete m_pBaseInteger;
	m_pBaseInteger = 0;
	delete[] m_pdIntervalData;
	m_pdIntervalData = 0;
	delete[] m_pnFactors;
	m_pnFactors = 0;

}

int PFCPAPFile::SeekToLine(int LineNumber)
{
	char Buf[128];
	if (LineNumber < m_nCurrentLineNum)
	{
		fseek(m_fpInputFile, 0, SEEK_SET);
		ReadLine(Buf, sizeof(Buf));
		m_nCurrentLineNum = 1;
		m_nCurrentPhysicalLineNum = 1;
		if (m_pIni)
			m_pIni->SetFileProcessing(true);
		m_bEOF = false;
	}
	int ret = e_ok;
	while (m_nCurrentLineNum < LineNumber)
	{
		if (ReadLine(Buf, sizeof(Buf)))
		{
			if (m_pIni)
				m_pIni->SetFileProcessing(false);
			ret = e_eof;
			m_bEOF = true;
			break;
		}
		m_nCurrentLineNum++;
	}
	if (m_pIni)
		m_pIni->SetFileLineNum(m_nCurrentLineNum);
	return ret;
}

int PFCPAPFile::GetNextLine(PFString &sLine, Integer *i, bool *b)
{
	int n;
	char TmpBuf[128], *_TmpBuf;  // no worry about buffer overflow.  NewPGen expressions will be short.
	sLine = "";
	if (b)
		*b = false;		// assume failure;
	if (m_bEOF)
	{
		m_sCurrentExpression = "";
		return e_eof;
	}

	bool bStoreThisExpression=false;
	bool bUseInteger = false;

	if (b)
	{
		if (i && m_pBaseInteger)
		{
			*b = true;
			bUseInteger = true;
		}
	}

TryNextLine:;

	_TmpBuf = TmpBuf;
	switch(m_eSearching)
	{
		case e_cur:
		{
			// Get the "next Line
			char Line[128];
			char *cp = 0;
			while(!cp)
			{
				m_nCurrentLineNum++;
				bStoreThisExpression = true;
				if (m_pIni)
					m_pIni->SetFileLineNum(m_nCurrentLineNum);
				if (ReadLine(Line, sizeof(Line)))
				{
					if (m_pIni)
						m_pIni->SetFileProcessing(false);
					m_bEOF = true;
					m_sCurrentExpression = "";
					return e_eof;
				}
				cp = strchr(Line, ' ');
			}
			// I switched to float, because I was not sure of a "standard" way to do long long's.  Note,
			// that there are issues not only on how to "input" these numbers from the file, and also
			// on the sprintf output formats.  I do not think that things such as %I64d will work anywhere
			// other than in VC.  Using double avoids ALL of these issues.
			m_fBaseInc = atof(Line);
			m_nGap = atoi(&cp[1]);
			//printf ("%s was scanned, and %f %f were found\n", Line, m_k, m_n);
			// If this is the same number as the last composite tested, then skip this line altogether, and 
			// try the next line.
			if (m_LastCompositeNum == m_fBaseInc + m_nGap*(m_nCurLo-1))
				goto TryNextLine;
			m_nCurNumFound = 0;
			m_nCurHi = m_nCurLo = m_nStartingPoint[m_nCPAPFold];
			//sprintf(TmpBuf, "%d^%d+%.0f", m_nBase, m_nExp, m_fBaseInc + m_nGap*(m_nCurLo-1));
			_TmpBuf += sprintf(TmpBuf, "%d^%d+%.0f", m_nBase, m_nExp, m_fBaseInc);
			for (n = 0; n < m_nCurLo-1; n++)
				_TmpBuf += sprintf(_TmpBuf, "+%d", m_nGap);
			sLine = TmpBuf;
			if (bUseInteger)
			{
				// Chris, what is the "quick" way to add these numbers which can be up to 53 bits long?
				sprintf (TmpBuf, "%.0f", m_fBaseInc + m_nGap*(m_nCurLo-1));
				m_TmpInteger.atoI(TmpBuf);
				*i = *m_pBaseInteger + m_TmpInteger;
			}
			if (bStoreThisExpression)
				m_sCurrentExpression = sLine;
			return e_ok;
		}

		case e_lo:
			//sprintf(TmpBuf, "%d^%d+%.0f", m_nBase, m_nExp, m_fBaseInc + m_nGap*(m_nCurLo-1));
			_TmpBuf += sprintf(TmpBuf, "%d^%d+%.0f", m_nBase, m_nExp, m_fBaseInc);
			for (n = 0; n < m_nCurLo-1; n++)
				_TmpBuf += sprintf(_TmpBuf, "+%d", m_nGap);
			sLine = TmpBuf;
			if (bUseInteger)
			{
				sprintf (TmpBuf, "%.0f", m_fBaseInc + m_nGap*(m_nCurLo-1));
				m_TmpInteger.atoI(TmpBuf);
				*i = *m_pBaseInteger + m_TmpInteger;
			}
			return e_ok;

		case e_hi:
			//sprintf(TmpBuf, "%d^%d+%.0f", m_nBase, m_nExp, m_fBaseInc + m_nGap*(m_nCurHi-1));
			_TmpBuf += sprintf(TmpBuf, "%d^%d+%.0f", m_nBase, m_nExp, m_fBaseInc);
			for (n = 0; n < m_nCurHi-1; n++)
				_TmpBuf += sprintf(_TmpBuf, "+%d", m_nGap);
			sLine = TmpBuf;
			if (bUseInteger)
			{
				sprintf (TmpBuf, "%.0f", m_fBaseInc + m_nGap*(m_nCurHi-1));
				m_TmpInteger.atoI(TmpBuf);
				*i = *m_pBaseInteger + m_TmpInteger;
			}
			return e_ok;

		case e_verifyCPAP:
			// Ok, find the next "non-factored" candidate (if one exists).  If not, switch back into "cur" mode, and
			// read the next line.

			if (m_pnFactors[m_nCurTestIntervalData] == -1)  // one of the AP-primes
				++m_nCurTestIntervalData;  // skip it.
			//sprintf(TmpBuf, "%d^%d+%.0f+%d+%.0f", m_nBase, m_nExp, m_fBaseInc, (m_FoundPrimeGaps[0]-1)*m_nGap, m_pdIntervalData[m_nCurTestIntervalData]);
			_TmpBuf += sprintf(TmpBuf, "%d^%d+%.0f", m_nBase, m_nExp, m_fBaseInc);
			for (n = 0; n < m_FoundPrimeGaps[0]-1; n++)
				_TmpBuf += sprintf(_TmpBuf, "+%d", m_nGap);
			sprintf(_TmpBuf, "+%.0f", m_pdIntervalData[m_nCurTestIntervalData]);

			if ( (m_nCurTestIntervalData+1) != m_nNumIntervalData && m_pnFactors[m_nCurTestIntervalData] != 1)
			{
				// Factor found
				//2^3320+1909228381+540+474 has factors: 7
				PFPrintf ("%s has factors: %d\n", TmpBuf, m_pnFactors[m_nCurTestIntervalData++]);
				goto TryNextLine;
			}
			sLine = TmpBuf;
			if (bUseInteger)
			{
				sprintf (TmpBuf, "%.0f", m_fBaseInc + (m_FoundPrimeGaps[0]-1)*m_nGap + m_pdIntervalData[m_nCurTestIntervalData]);
				m_TmpInteger.atoI(TmpBuf);
				*i = *m_pBaseInteger + m_TmpInteger;
			}
			return e_ok;
	}
	if (m_pIni)
		m_pIni->SetFileProcessing(false);
	m_bEOF = true;
	m_sCurrentExpression = "";
	return e_unknown;
}

void PFCPAPFile::CurrentNumberIsPrime(bool bIsPrime, bool *p_bMessageStringIsValid, PFString *p_MessageString)
{
	m_bLastNumberPrime = bIsPrime;
	if (p_bMessageStringIsValid)
		*p_bMessageStringIsValid = false;
	char TmpBuf[256];

	// searching the "first" number and it is not prime.  Simply read the next value from the file.
	if (!bIsPrime && e_cur == m_eSearching)
	{
		m_LastCompositeNum = m_fBaseInc + m_nGap*(m_nCurLo-1);
		m_nCurNumFound = 0;
		return;
	}

	m_bLastNumberPrime = bIsPrime;

	m_LastCompositeNum = -1;

	switch(m_eSearching)
	{
		case e_cur:
			SwitchToLo();
			break;

		case e_lo:
			if (!m_bLastNumberPrime)
				SwitchToHi();
			else
			{
				m_FoundPrimeGaps[m_nCurNumFound] = m_nCurLo;
				if (++m_nCurNumFound > 2 && p_bMessageStringIsValid)
				{
					sprintf(TmpBuf, " - AP-%d (Gap %d) -\n", m_nCurNumFound, m_nGap);
					*p_MessageString = TmpBuf;
					*p_bMessageStringIsValid = true;
				}
				if (--m_nCurLo == 0)
					SwitchToHi();
			}
			break;

		case e_hi:
			if (!m_bLastNumberPrime)
			{
				if (m_nCurNumFound > 2)
					SwitchToVerify();
				else
					SwitchToNextLine();
			}
			else
			{
				m_FoundPrimeGaps[m_nCurNumFound] = m_nCurHi;
				if (++m_nCurNumFound > 2 && p_bMessageStringIsValid)
				{
					sprintf(TmpBuf, " - AP-%d (Gap %d) -\n", m_nCurNumFound, m_nGap);
					*p_MessageString = TmpBuf;
					*p_bMessageStringIsValid = true;
				}
				if (++m_nCurHi > m_nCPAPFold)
				{
					if (m_nCurNumFound > 2)
						SwitchToVerify();
					else
						SwitchToNextLine();
				}
			}
			break;

		case e_verifyCPAP:
			if (m_bLastNumberPrime)
			{
				if (p_bMessageStringIsValid)
				{ 
					char *_TmpBuf=TmpBuf;
					_TmpBuf += sprintf(TmpBuf, " - Last AP is not CPAP-%d -  (Gap was %d)\n", m_nCurNumFound, m_nGap);
					// check to see if we still have a CPAP-(n-1) of (n-2) .. n > 3
					if (m_nCurNumFound > 3)
					{
						int which = 2;
						while ( (which*m_nGap)/2 < m_nCurTestIntervalData)
							which++;
						if (which > 2)
							sprintf(_TmpBuf, " !!But it is:\n - !!CPAP-%d!! -  (Gap %d, base=%d^%d+%.0f)\n", which, m_nGap, m_nBase, m_nExp, m_fBaseInc+(m_FoundPrimeGaps[0]-1)*m_nGap);

					}
					*p_MessageString = TmpBuf;
					*p_bMessageStringIsValid = true;
				}
				SwitchToNextLine();
			}
			else
			{
				if (m_nNumIntervalData == ++m_nCurTestIntervalData)
				{
					if (p_bMessageStringIsValid)
					{
						sprintf(TmpBuf, " - !!CPAP-%d!! -  (Gap %d, base=%d^%d+%.0f)\n", m_nCurNumFound, m_nGap, m_nBase, m_nExp, m_fBaseInc+(m_FoundPrimeGaps[0]-1)*m_nGap);
						*p_MessageString = TmpBuf;
						*p_bMessageStringIsValid = true;
					}
					SwitchToNextLine();
				}
			}
			break;
	}
}


void PFCPAPFile::SwitchToLo()
{
	// Ok, the first number was prime, now look at the "lower" numbers.
	m_FoundPrimeGaps[0] = m_nCurLo;
	m_nCurNumFound = 1;
	m_eSearching = e_lo;
	m_nCurLo--;
	if (m_nCurLo == 0)
		SwitchToHi();
}

void PFCPAPFile::SwitchToHi()
{
	// Ok, all the "lower" numbers (which were prp) have been checked.  Now continue with the "high" numbers
	m_eSearching = e_hi;
	if (m_nCurNumFound + (m_nCPAPFold - m_nCurHi) < 3)  // AP-3 is not possible, so get next line of file.
		SwitchToNextLine();
	else
	{
		m_nCurHi++;
		if (m_nCurHi > m_nCPAPFold)
		{
			if (m_nCurNumFound > 2)
				SwitchToVerify();
			else
				SwitchToNextLine();
		}
	}
}

void PFCPAPFile::SwitchToVerify()
{
	// Ok, we are "done" with the hi and lo searching for this line.  Lets see if we have
	// enough primes for an AP, and if so, lets check for CPAP's

	//  NOTE that during the "check" any prp's found will halt the CPAP search, so if we have 
	// an AP-4 or an AP-5, then we still may have a CPAP-3 (or CPAP-4) even though the "search"
	// stops because of a found PRP.  This issue will require a  "hand" investigation.

	m_eSearching = e_verifyCPAP;
	qsort(m_FoundPrimeGaps, m_nCurNumFound, sizeof(m_FoundPrimeGaps[0]), SimpleIntArraySortCompare);

	// Ok, now allocate and fill up m_pNIntervalData
	m_nNumIntervalData = (m_nGap*(m_nCurNumFound-1))/2;
	m_nCurTestIntervalData = 0;
	m_pdIntervalData = new double[m_nNumIntervalData];
	m_pnFactors = new int[m_nNumIntervalData];

	double addVal = 0;
	int i;
	for (i = 0; i < m_nNumIntervalData; i++)
	{
		m_pnFactors[i] = 1;
		if (int(addVal) % m_nGap == 0)
			m_pnFactors[i] = -1;		// skip the "found" PRP's within the AP.
		m_pdIntervalData[i] = addVal;
		addVal += 2;
	}

	// Ok, now sieve out the factors of primes [3..9973]
	int64 MaxNum = int64(m_fBaseInc) + (m_FoundPrimeGaps[0]-1)*m_nGap + int64(m_pdIntervalData[m_nNumIntervalData-1]);
	int64 MinNum = int64(m_fBaseInc) + (m_FoundPrimeGaps[0]-1)*m_nGap;

	primeserver.restart();
	primeserver.skip((uint32)3);
	uint32 p;
	primeserver.next(p);

	// Hacked & slashed code from CPAPSieve project (much has been changed to fit into this project)

	for (i = 0; i < eNumPrimesToFactorWith; i++)
	{
		// b's for factor primes are contained within the pre-calc'd array
		// m_FactorizeBase, now remove any k = +/-b mod p.
		int64 num = m_FactorizeBase[i];
		int64 inc = p<<1;
		int first = -1;
		if (num < MinNum)
		{
			// MUCH improvement over simply doing num += inc when MinNum is not 0.
			// if MinNum is high, then this will help get the ball rolling for small prime factors.
			do
			{
				if (first == -1)
					num += ((MinNum-num)/inc-1)*inc;
				num += inc;
				first = 0;
			}
			while (num < MinNum);
			first = -1;
		}
		while (num <= MaxNum)
		{
			if (m_pnFactors[(num-MinNum)/2] == 1)
			{
				//printf ("CAP factor %d^%d+%.0f+%d+%.0f is %d\n", m_nBase, m_nExp, m_fBaseInc, (m_FoundPrimeGaps[0]-1)*m_nGap, m_pdIntervalData[(num-MinNum)/2], p);
				m_pnFactors[ (num-MinNum)/2] = p;
			}
			num += inc;
		}
		primeserver.next(p);
	}

}

void PFCPAPFile::SwitchToNextLine()
{
	delete[] m_pdIntervalData;
	m_pdIntervalData = 0;
	delete[] m_pnFactors;
	m_pnFactors = 0;

	m_nNumIntervalData = m_nCurTestIntervalData = 0;
	m_eSearching = e_cur;
	m_nCurNumFound = 0;
	m_nCurHi = m_nCurLo = m_nStartingPoint[m_nCPAPFold];
}

int PFCPAPFile::SimpleIntArraySortCompare(const void *x, const void *y)
{
	if (*((const int*)x) > *((const int*)y))
		return 1;
	if (*((const int*)x) < *((const int*)y))
		return -1;
	return 0;
}

/**********************************************************
 * return base^navlue%p    Not the most optimal function, 
 * but it is only called eNumPrimesToFactorWith times during 
 * initialization, so it is fast enough, and the code is free
 * to use, as opposed to Yves double asm code, which is faster,
 * but has not been "cleared" to be included in "open" software.
 **********************************************************/

int PFCPAPFile::powNmodP(int p)
{
	int64 w = 1;
	int64 x = m_nBase;
	int n = m_nExp;
	for (;;)
	{
		if (n&1)
			w = (w*x)%p;
		n >>= 1;
		if (!n)
			return (int)w;
		x = (x*x)%p;
	}
}


void PFCPAPFile::BuildFactorizeBase()
{
	primeserver.restart();
	primeserver.skip((uint32)3);
	uint32 p;
	primeserver.next(p);
	int Cnt = 0;
	if (m_nBase&1)
		m_bOddBase = true;

	// Hacked & slashed code from CPAPSieve project (much has been changed to fit into this project)
	// Before there was no "pre-calcing" of the b's  In the real sieve, we simply keep finding b's
	// and running them.  Here in this code, I simply want to factor a VERY small amount, to simply
	// remove a large portion of the composites when checking a found AP-n to see if it is consecutive.

#if defined (_DEBUG)
	PFPrintfStderr("\nBuilding Factor Base for consective part of CPAP checking\n");
#endif
	while (Cnt < eNumPrimesToFactorWith)
	{
		int b = powNmodP(p);
		b = ((-1*b)%p) + p;
		if (m_bOddBase)
		{
			// Odd base's make odd values for base^exp
			if (b&1)
				b += p;
		}
		else
		{
			// even base's make even values for base^exp
			if (!(b&1))
				b += p;
		}

		m_FactorizeBase[Cnt++] = b;  // Starting place for the sieve of Eratosthenes for this prime into the "base" expression.

		primeserver.next(p);
	}
#if defined (_DEBUG)
	if (p > 3000000000)
	{
		// actually it can handle up to sqrt(0x7FFFFFFFFFFFFFFF) which is about 3.037 billion.
		// but this code should NEVER be accessed, because there is NO reason to factor this
		// high.  Factoring to a million, or several million is MORE than sufficient.
		PFPrintfStderr("Error, eNumPrimesToFactorWith for the CPAP search is TOO large!\n");
		exit(-1);
	}
	PFPrintfStderr("Max prime used: %d  (the first %d odd primes)\n\n", p, eNumPrimesToFactorWith);
#endif

}


