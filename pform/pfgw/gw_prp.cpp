#include "primeformpch.h"
#include <signal.h>
#include "../../pform/pfio/pfini.h"
#include "pfgw_globals.h"

//#define JBC_BUILD

#if defined (JBC_BUILD)
#include "jbc.cxx"
#else
#define JBC()
#endif

extern int g_CompositeAthenticationLevel;
extern const double maxErrorAllowed;
bool CheckForFatalError(const char *caller, const char *sNumStr, GWInteger *gwX, int currentIteration, int maxIterations);

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
   gwinit2(&gwdata, sizeof(gwhandle), GWNUM_VERSION);
   if (gwdata.GWERROR == GWERROR_VERSION_MISMATCH)
   {
		PFOutput::EnableOneLineForceScreenOutput();
		PFPrintfStderr ("GWNUM version mismatch.  PFGW is not linked with version %s of GWNUM.\n", GWNUM_VERSION);
      g_bExitNow = true;
      return 0;
   }

   if (gwdata.GWERROR == GWERROR_STRUCT_SIZE_MISMATCH)
   {
		PFOutput::EnableOneLineForceScreenOutput();
		PFPrintfStderr ("GWNUM struct size mismatch.  PFGW must be compiled with same switches as GWNUM.\n");
      g_bExitNow = true;
      return 0;
   }

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
#ifdef _MSC_VER
      if (*RestoreName && !_access_s(RestoreName, 0))
#else
      if (*RestoreName && !access(RestoreName, 0))
#endif
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
		for(;i--;)
      {
         int errchk = ErrorCheck(iDone, iTotal);

         gw_clear_maxerr(&gwdata);
			gwsetnormroutine(&gwdata, 0, errchk, bit(X,i));
			gwsquare(gwX);

			iDone++;
			if(g_nIterationCnt && (((iDone%g_nIterationCnt)==0) || bFirst || !i))
			{
				if (*RestoreName)
					SaveState(e_gwPRP, RestoreName, iDone, &gwX, iBase, e_gwnum, N);
				static int lastLineLen;
				bFirst=false;
				// 120 bytes will not overflow, since we "force" the max size within the sprintf()
				char Buf[160];
				if (errchk && gwdata.MAXDIFF > -1e10 && gwdata.MAXDIFF < 1e10)
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

         if (errchk)
	      {
		      double d = gwX.sumdiff();
		      if (d - MaxSeenDiff > 0.02)
			      MaxSeenDiff = d;
      	}

         if (CheckForFatalError("prp_using_gwnum", sNumStr, &gwX, iDone, iTotal))
            return -1;

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

bool CheckForFatalError(const char *caller, const char *sNumStr, GWInteger *gwX, int currentIteration, int maxIterations)
{
   char  buffer1[200], buffer2[200], buffer3[200];
   bool  haveFatalError = false;

   // Code "straight" from PRP.
	if (gw_test_illegal_sumout (&gwdata))
   {
      sprintf(buffer1, "Detected in gw_test_illegal_sumout() in %s", caller);
      sprintf(buffer2, "Iteration: %d/%d ERROR: ILLEGAL SUMOUT", currentIteration, maxIterations);
      buffer3[0] = 0;
      haveFatalError = true;
   }

   if (!haveFatalError && gw_test_mismatched_sums (&gwdata))
   {
		double suminp, sumout;
		suminp = gwX->suminp ();
		sumout = gwX->sumout ();

      sprintf(buffer1, "Detected in gw_test_mismatched_sums() in %s", caller);
      sprintf(buffer2, "Iteration: %d/%d ERROR: SUM(INPUTS) != SUM(OUTPUTS),", currentIteration, maxIterations);
      sprintf(buffer3, "%.16g != %.16g\n  (Diff=%.0f max allowed=%.0f)", suminp, sumout, fabs(suminp-sumout), gwdata.MAXDIFF);
      haveFatalError = true;
   }

   if (gw_get_maxerr(&gwdata) > maxErrorAllowed)
   {
      sprintf(buffer1, "Detected in MAXERR>%.2f (round off check) in %s", maxErrorAllowed, caller);
      sprintf(buffer2, "Iteration: %d/%d ERROR: ROUND OFF %.5g>%.2f", currentIteration, maxIterations, gw_get_maxerr(&gwdata), maxErrorAllowed);
      buffer3[0] = 0;
      haveFatalError = true;
   }

   if (haveFatalError)
	{
		PFOutput::EnableOneLineForceScreenOutput();
		PFPrintfStderr("\n");
		PFOutput::EnableOneLineForceScreenOutput();
		PFOutput::OutputToErrorFileAlso(buffer1, sNumStr, currentIteration, maxIterations);

      PFPrintfStderr("%s\n  ", buffer2);
      if (buffer3[0] != 0)
         PFPrintfStderr("%s\n  ", buffer3);

      if (g_CompositeAthenticationLevel == 1)
			PFPrintfStderr("(Test aborted, try again using the -a2 (or possibly -a0) switch)\n");
		else if (g_CompositeAthenticationLevel == 0)
			PFPrintfStderr("(Test aborted, try again using the -a1 switch)\n");
		else
			PFPrintfStderr("(Test aborted)\n");

		DestroyModulus();
	}

   return haveFatalError;
}

