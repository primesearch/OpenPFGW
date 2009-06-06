#include "primeformpch.h"
#include <signal.h>
#include "../../pform/pfio/pfini.h"

//#define JBC_BUILD

#if defined (JBC_BUILD)
#include "jbc.cxx"
#else
#define JBC()
#endif

#include "pfgw_globals.h"

extern int g_CompositeAthenticationLevel;

// -2 is used for incomplete processing (i.e. user told us to exit early).
// -1 is an error (round off or mod reduction).  It is NOT prime or composite.  We have NO idea what it is.
// 0 is composite.
// 1 is prime (prp actually).
int gwPRP(Integer *N, const char *sNumStr, uint64 *p_n64ValidationResidue, uint32 dwOverride)
{

	// First check to see if N divides iBase
	if (N->gmp()->_mp_size == 1 && Integer(iBase) % *N == 0)
	{
		int TmpIBase = iBase;
		if (--iBase == 1)
			iBase=255;
		PFPrintf ("Error, base %d can't be used to PRP %s, Trying to PRP with base %d\n", TmpIBase, sNumStr, iBase);
		int Ret = gwPRP(N, sNumStr, p_n64ValidationResidue, dwOverride);
		iBase = TmpIBase;
		return Ret;
	}
	extern int iBaseUsed;
	iBaseUsed = iBase;

	Integer X = (*N);
	--X;				// X is the exponent, we are to calculate 3^X mod N

	int iTotal=lg(X);

	// Could be a proth number.
	/*

	// Check for Proth.  We now have special code in GMP to handle them.  This changes
	// the break over points SIGNIFICANTLY for when GMP is faster than Woltman logic.
	bool bIsProth=false;
	uint64 k;
	uint32 n;
	bool bPlus;
	mpz_t *pI = N->gmp();
	mp_limb_t t = mpz_getlimbn(*(N->gmp()), 0);

	if (iTotal > 1200 && iTotal < 6000 && t == 1 || t == ((mp_limb_t)-1))
	{
		uint32 i = 1;
		uint32 len = mpz_size(*pI);
		if (t == 1)
		{
			for (; i < len; ++i)
			{
				if (mpz_getlimbn(*(N->gmp()), i))
					break;
			}
			// Ok, this limb is not zero, so see if the bits fit a uint64
			if (len-i <= 3)
			{
				if (len-2<=i)
				{
					// easiest cases (it always fits)
					k = mpz_getlimbn(*(N->gmp()), len-1);
					if (len-2==i)
					{
						k <<= 32;
						k += mpz_getlimbn(*(N->gmp()), len-2);
					}
					n = 32*i;
					while ( ((*(uint32*)&k) & 1) == 0)
					{
						++n;
						k >>= 1;
					}
					bIsProth = bPlus = true;
					X.Proth_prp(k, n, iBase);
					if (X == 1)
						return 1;
					else if (p_n64ValidationResidue)
						*p_n64ValidationResidue = (X & ((((uint64)1)<<62)-1));
					return 0;
				}
				else
				{
					// uint32 sized limbs are ALL that will work in this code
//					k = 0;
//					uint32 lo = mpz_getlimbn(*(N->gmp()), i);
//					// count bits of lo;
//					uint32 lobits = 32;
//					while ( (lo & 1) == 0)
//					{
//						lo >>= 1;
//						--lobits;
//					}
				}
			}
		}
		else
		{
		}
	}
	*/

	// if iTotal is less than 1000, then use GMP to do exponentaion (we need to work out the exact cutoff's, and 
	// different reduction methods will also "change" this".  However, this code (The current Winbloze build) is 
	// NOT optimized GMP, so the cut off is MUCH less than optimal.
#if defined (_MSC_VER)
	// The PIV break over was at about 2^800.  The PII break over was at about 2^650
	// NOTE these "break" overs are based upon "Proth" base-2 reduction (which makes the Woltman code
	// function faster). If "generic" reduction is used instead, then these break over points are TOO 
	// low, and GMP should be used much higher.  Unfortunately, I am not sure how to "quickly" determine
	// proth-2 mode quickly at this point
	if ( (iTotal < 650 && (CPU_FLAGS&CPU_SSE2) == 0) || (iTotal < 800 && (CPU_FLAGS&CPU_SSE2))   ) 
#else
	if (iTotal < 600)
#endif
	{
		if (!g_bGMPMode)
		{
			g_bGMPMode = true;
			PFPrintf ("Switching to Exponentiating using GMP\n");
		}
		Integer b(iBase);
		// This is the "raw" gmp exponentiator.  It if pretty fast up to about 500 digits or so.
		X = powm(b,X,*N);
		if (X == 1)
			return 1;
		else if (p_n64ValidationResidue)
			*p_n64ValidationResidue = (X & ((((uint64)1)<<62)-1));
		return 0;
	}
	if (g_bGMPMode)
	{
		g_bGMPMode = false;
		PFPrintf ("Switching to Exponentiating using Woltman FFT's\n");
	}

	// create a context
	gwinit (&gwdata);
	gwsetmaxmulbyconst(&gwdata, iBase);	// maximum multiplier
	if (CreateModulus(N)) return -2;

	if (dwOverride == 0)			// Normal PRP test
		return (prp_using_gwnum (N, iBase, sNumStr, p_n64ValidationResidue));

	// Special override to benchmark a specific number of iterations
	{
		GWInteger gwX;
		
		gwX=iBase;					// initialise X to A^1.
		gwsetmulbyconst(&gwdata, iBase);		// and multiplier
		for(;dwOverride;dwOverride--)
		{
			gwsetnormroutine(&gwdata,0,0,dwOverride&1);
			gwsquare(gwX);
		}
	}
	DestroyModulus ();
	return 0;
}


// -2 is used for incomplete processing (i.e. user told us to exit early).
// -1 is an error (round off or mod reduction).  It is NOT prime or composite.  We have NO idea what it is.
// 0 is composite.
// 1 is prime (prp actually).
int prp_using_gwnum(Integer *N, uint32 iBase, const char *sNumStr, uint64 *p_n64ValidationResidue)
{
	int	retval;
	Integer X = (*N);
	--X;				// X is the exponent, we are to calculate iBase^X mod N
	int iTotal=lg(X);

	// Data for the save/restore file.
	char RestoreName[13];   // file name will fit an 8.3
	*RestoreName = 0;
	if (iTotal > 50000)
		CreateRestoreName(N, RestoreName);

	// Handle "auto" round off checking.
	extern bool g_bForceNoRoundOffChecking, g_bForceRoundOffChecking;
	if (g_bForceNoRoundOffChecking)
		ERRCHK=0;
	else
	{
		if (!g_bForceRoundOffChecking)	// turn off error checking from last run if we are not "forced" on.
			ERRCHK=0;
		if (gwnear_fft_limit (&gwdata, 2.0))
			ERRCHK=1;		// Getting close to max bits (within 2%)
	}

	int ThisLineLen_Final=0;
	// everything with a GWInteger has a scope brace, so that
	// GWIntegers are destroyed before the context they live in
	{
		// prepare the gw buffers we need
		GWInteger gwX;

		// I think we're ready to go, let's do it.
		gwX=iBase;					// initialise X to A^1.
		gwsetmulbyconst(&gwdata, iBase);		// and multiplier
		
		// keep a simple iteration counter just for rudimentary progress output
		int iDone=0;

		bool bFirst=true;

		// reduce screen output for tiny numbers.
		if (iTotal < 2*g_nIterationCnt)
		{
			static time_t last;
			bFirst = false;	// don't print the "PRP:  ...." line on bit#1
			if (time(0)-last > 4)
			{
				// Every 3 seconds, we DO print the "PRP: ..." line
				bFirst = true;
				last = time(0);
			}
		}

		// i MUST be handled outside of the loop below, since the resume code will have to modify it.
		int i=iTotal;
		// Check for "existance" of a file which matches the hash pattern of this number.
		if (*RestoreName && !_access(RestoreName, 0))
		{
			uint32 DoneBits;
			if (RestoreState(e_gwPRP, RestoreName, &DoneBits, &gwX, iBase, e_gwnum))
			{
				// The number not only passes the hash, but EVERY check was successful.  We are working with the right number.
				iDone = DoneBits;
				i -= iDone;
				PFPrintf ("Resuming at bit %d\n", iDone);
			}
		}

		double MaxSeenDiff=0.;
		for(;i--;) {
			// These blocks of commented out items
//			if (1 && (iDone >= 112240 || iDone == 25000 || iDone==50000))
//			{
//				char FN[40];
//				sprintf(FN, "c:/pfgw_bit_%02d_pre_sqr", iDone+1);
//				FILE *out = fopen(FN, "wt");
//				for (int i = 0; i < FFTLEN; ++i)
//				{
//					fprintf(out, "%04d - %.2f\n", i, (gwX.G())[0]->data[i]);
//				}
//				fclose(out);
//			}

			gw_clear_maxerr(&gwdata);
			gwsetnormroutine(&gwdata,0,ERRCHK,bit(X,i));
			gwsquare(gwX);

//			if (1 && (iDone >= 112240 || iDone == 25000 || iDone==50000))
//			{
//				char FN[40];
//				sprintf(FN, "c:/pfgw_bit_%02d_post_sqr", iDone+1);
//				FILE *out = fopen(FN, "wt");
//				for (int i = 0; i < FFTLEN; ++i)
//				{
//					fprintf(out, "%04d - %.2f\n", i, (gwX.G())[0]->data[i]);
//				}
//				fclose(out);
//			}

			iDone++;
			if(g_nIterationCnt && (((iDone%g_nIterationCnt)==0) || bFirst || !i))
			{
				if (*RestoreName)
					SaveState(e_gwPRP, RestoreName, iDone, &gwX, iBase, e_gwnum, N);
				static int lastLineLen;
				bFirst=false;
				// 120 bytes will not overflow, since we "force" the max size within the sprintf()
				char Buf[160];
				if (ERRCHK && gwdata.MAXDIFF != 1e80)
					sprintf(Buf, "PRP: %.36s %d/%d mro=%.5g sum=%.2f/%.2f\r",
                       sNumStr,iDone,iTotal, gw_get_maxerr(&gwdata), gwdata.MAXDIFF, MaxSeenDiff);
				else
					sprintf(Buf, "PRP: %.36s %d/%d\r", sNumStr,iDone,iTotal);
				int thisLineLen = strlen(Buf);
				if (iDone == 1 && iTotal > 50000)
					g_pIni->ForceFlush();
				if (lastLineLen > thisLineLen)
					// When mixing stdio, stderr and redirection with a \r stderr output,
					// then the line must "erase" itself, IF it ever shrinks.
					PFPrintfClearCurLine(lastLineLen);
				lastLineLen = thisLineLen;
				PFPrintfStderr("%s", Buf);
				PFfflush(stderr);
				ThisLineLen_Final = thisLineLen;
			}
			// Code "straight" from PRP.
			if (gw_test_illegal_sumout (&gwdata)) {
				PFOutput::EnableOneLineForceScreenOutput();
				PFPrintfStderr("\n");
				PFOutput::EnableOneLineForceScreenOutput();
				PFOutput::OutputToErrorFileAlso("Detected in gw_test_illegal_sumout() in gwprp.cpp (PRP Testing)", sNumStr, iTotal-i, iTotal);
				if (g_CompositeAthenticationLevel == 1)
					PFPrintfStderr("Iteration: %d/%d ERROR: ILLEGAL SUMOUT\n   (Test aborted, try again using the -a2 (or possibly -a0) switch)\n", iTotal-i, iTotal);
				else if (g_CompositeAthenticationLevel == 0)
					PFPrintfStderr("Iteration: %d/%d ERROR: ILLEGAL SUMOUT\n   (Test aborted, try again using the -a1 switch)\n", iTotal-i, iTotal);
				else
					PFPrintfStderr("Iteration: %d/%d ERROR: ILLEGAL SUMOUT\n   (Test aborted)\n", iTotal-i, iTotal);
				DestroyModulus();
				return -1;
			}
			if (gw_test_mismatched_sums (&gwdata)) {
//				static unsigned long last_bit = 0;
//				static double last_suminp = 0.0;
//				static double last_sumout = 0.0;
				double suminp, sumout;
				suminp = gwX.suminp ();
				sumout = gwX.sumout ();
//				if (bit == last_bit &&
//					suminp == last_suminp &&
//					sumout == last_sumout) {
//					writeResults (ERROK);
//					saving = 1;
//				} else {
//					char	msg[80];
				PFOutput::EnableOneLineForceScreenOutput();
				PFPrintfStderr("\n");
				PFOutput::EnableOneLineForceScreenOutput();
				PFOutput::OutputToErrorFileAlso("Detected in gw_test_mismatched_sums() in gwprp.cpp (PRP Testing)", sNumStr, iDone, iTotal);
				if (g_CompositeAthenticationLevel == 1)
					PFPrintfStderr("Iteration: %d/%d ERROR: SUM(INPUTS) != SUM(OUTPUTS),\n   %.16g != %.16g\n  (Diff=%.0f max allowed=%.0f)\n  (Test aborted, try again using the -a2 (or possibly -a0) switch)\n",  iTotal-i, iTotal, suminp, sumout, fabs(suminp-sumout), gwdata.MAXDIFF);
				else if (g_CompositeAthenticationLevel == 0)
					PFPrintfStderr("Iteration: %d/%d ERROR: SUM(INPUTS) != SUM(OUTPUTS),\n   %.16g != %.16g\n  (Diff=%.0f max allowed=%.0f)\n  (Test aborted, try again using the -a1 switch)\n",  iTotal-i, iTotal, suminp, sumout, fabs(suminp-sumout), gwdata.MAXDIFF);
				else
					PFPrintfStderr("Iteration: %d/%d ERROR: SUM(INPUTS) != SUM(OUTPUTS),\n   %.16g != %.16g\n  (Diff=%.0f max allowed=%.0f)\n  (Test aborted)\n",  iTotal-i, iTotal, suminp, sumout, fabs(suminp-sumout), gwdata.MAXDIFF);
//				last_bit = bit;
//				last_suminp = suminp;
//				last_sumout = sumout;
//				goto error;
				DestroyModulus();
				return -1;
//				}
			}
//#define ROUND_OFF_CHECK_TEST
			if (ERRCHK)
			{
				double d = gwX.sumdiff();
				if ( (d-MaxSeenDiff) > 0.02)
				{
					MaxSeenDiff = d;
#if defined (ROUND_OFF_CHECK_TEST)
					// NOTE this is "special" testing code.  This is NOT for general release!!!
					extern char g_szInputFileName[1024];
					char FN[1060];
					sprintf(FN, "%s_a%d_level.roundoff", g_szInputFileName, g_CompositeAthenticationLevel);
					FILE *out = fopen(FN, "a");
					if (out)
					{
						fprintf(out, "PRP: %.36s %d/%d mro=%.5g SM=%.2f/%.2f\n", sNumStr,iDone,iTotal, gw_get_maxerr(&gwdata), gwdata.MAXDIFF, MaxSeenDiff);
						fclose(out);
					}
					else 
						exit(printf ("Error opeing file %s\n", FN));
#endif
				}
			}
			if (gw_get_maxerr(&gwdata) > 0.40)
			{
				// Test for "excessive" error conditions
				PFOutput::EnableOneLineForceScreenOutput();
				PFPrintfStderr("\n");
				PFOutput::EnableOneLineForceScreenOutput();
				PFOutput::OutputToErrorFileAlso("Detected in MAXERR>0.4 (round off check) in gwprp.cpp (PRP Testing)", sNumStr, iDone, iTotal);
				if (g_CompositeAthenticationLevel == 1)
					PFPrintfStderr("Iteration: %d/%d ERROR: ROUND OFF %.5g>0.40\n   (Test aborted, try again using the -a2 (or possibly -a0) switch)\n",  iTotal-i, iTotal, gw_get_maxerr(&gwdata));
				else if (g_CompositeAthenticationLevel == 0)
					PFPrintfStderr("Iteration: %d/%d ERROR: ROUND OFF %.5g>0.40\n   (Test aborted, try again using the -a1 switch)\n",  iTotal-i, iTotal, gw_get_maxerr(&gwdata));
				else
					PFPrintfStderr("Iteration: %d/%d ERROR: ROUND OFF %.5g>0.40\n   (Test aborted)\n",  iTotal-i, iTotal, gw_get_maxerr(&gwdata));
				DestroyModulus();
				return -1;
			}

			if (g_bExitNow)
			{
				if (*RestoreName)
					SaveState(e_gwPRP, RestoreName, iDone, &gwX, iBase, e_gwnum, N, true);
				// zap the gw  (Good place to "save" the context to be loaded on a restart.
				DestroyModulus();
				return -2; // we really do not know at this time.  It is NOT a true error, but is undetermined, due to not comple processing
			}
		}

		X = gwX;
		if (p_n64ValidationResidue) {
			*p_n64ValidationResidue = X & 0x7fffffff;
			*p_n64ValidationResidue |= ( (uint64)((X>>31)&0x7fffffff) << 31);
		}

		if (X==1)
			retval=1;
		else
			retval=0;
	}
	// zap the gw
	DestroyModulus();

	// Nuke any temp file, since we have totally processed the number.
	remove(RestoreName);

	if (ThisLineLen_Final)
		PFPrintfClearCurLine(ThisLineLen_Final);

	return retval;
}
