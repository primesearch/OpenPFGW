#include "primeformpch.h"
#include <signal.h>
#include "../../pform/pfio/pfini.h"
#include "pfgw_globals.h"

extern int g_CompositeAthenticationLevel;
extern const double g_dMaxErrorAllowed;
extern bool g_FFTSizeOnly;
bool CheckForFatalError(const char *caller, GWInteger *gwX, int currentIteration, int maxIterations, int fftSize);

// -2 is used for incomplete processing (i.e. user told us to exit early).
// -1 is an error (round off or mod reduction).  It is NOT prime or composite.  We have NO idea what it is.
// 0 is composite.
// 1 is prime (prp actually).
int gwPRP(Integer *N, const char *sNumStr, uint64 *p_n64ValidationResidue)
{
   // First check to see if N divides iBase
   if (N->gmp()->_mp_size == 1 && Integer(iBase) % *N == 0)
   {
      int TmpIBase = iBase;
      if (--iBase == 1)
         iBase=255;
      PFPrintfLog ("Error, base %d can't be used to PRP %s, Trying to PRP with base %d\n", TmpIBase, sNumStr, iBase);
      int Ret = gwPRP(N, sNumStr, p_n64ValidationResidue);
      iBase = TmpIBase;
      return Ret;
   }
   extern int iBaseUsed;
   iBaseUsed = iBase;

   Integer X = (*N);
   --X;            // X is the exponent, we are to calculate 3^X mod N

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
         PFPrintf("Switching to Exponentiating using GMP\n");
      }
      if (g_FFTSizeOnly)
         return 0;

      Integer b(iBase);
      // This is the "raw" gmp exponentiator.  It if pretty fast up to about 500 digits or so.
      X = powm(b,X,*N);
      if (X == 1)
         return 1;
      else
         if (p_n64ValidationResidue)
         {
            *p_n64ValidationResidue = X & 0x7fffffff;
            *p_n64ValidationResidue |= ( (uint64)((X>>31)&0x7fffffff) << 31);
            *p_n64ValidationResidue |= ( (uint64)((X>>62)&0x00000003) << 62);
         }
      return 0;
   }
   if (g_bGMPMode)
   {
      g_bGMPMode = false;
      PFPrintf("Switching to Exponentiating using Woltman FFT's\n");
   }

   int fftSize = g_CompositeAthenticationLevel - 1;
   int testResult = -1;

   do
   {
      fftSize++;

      gwinit2(&gwdata, sizeof(gwhandle), (char *) GWNUM_VERSION);
      gwsetmaxmulbyconst(&gwdata, iBase); // maximum multiplier

      if (CreateModulus(N, g_cpTestString, true, fftSize)) return -2;

      if (!g_FFTSizeOnly)
	      testResult = prp_using_gwnum(N, iBase, sNumStr, p_n64ValidationResidue, fftSize);

      DestroyModulus();
   } while (testResult == -1 && fftSize < 5 && !g_FFTSizeOnly);

   return testResult;
}

void  bench_gwPRP(Integer *N, uint32 iterations)
{
   Integer testN;
   Integer X = (*N);
   --X;            // X is the exponent, we are to calculate 3^X mod N

   // create a context
   gwinit2(&gwdata, sizeof(gwhandle), (char *) GWNUM_VERSION);

   testN = *N;
   gwsetmaxmulbyconst(&gwdata, iBase);   // maximum multiplier
   if (CreateModulus(N, g_cpTestString, true)) return;

   GWInteger gwX;

   gwX=iBase;               // initialise X to A^1.
   gwsetmulbyconst(&gwdata, iBase);      // and multiplier

   for(; iterations; iterations--)
   {
      gwsetnormroutine(&gwdata, 0, 0, iterations&1);
      gwsquare(gwX);
   }

   DestroyModulus ();
}

// -2 is used for incomplete processing (i.e. user told us to exit early).
// -1 is an error (round off or mod reduction).  It is NOT prime or composite.  We have NO idea what it is.
// 0 is composite.
// 1 is prime (prp actually).
int prp_using_gwnum(Integer *N, uint32 iBase, const char *sNumStr, uint64 *p_n64ValidationResidue, int fftSize)
{
   int   retval;
   Integer X = (*N);
   --X;            // X is the exponent, we are to calculate iBase^X mod N
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
      gwX=iBase;               // initialise X to A^1.
      gwsetmulbyconst(&gwdata, iBase);      // and multiplier

      // keep a simple iteration counter just for rudimentary progress output
      int iDone=0;

      bool bFirst=true;

      // reduce screen output for tiny numbers.
      if (iTotal < 2*g_nIterationCnt)
      {
         static time_t last;
         bFirst = false;   // don't print the "PRP:  ...." line on bit#1
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
            PFPrintfLog ("Resuming at bit %d\n", iDone);
         }
      }

      double MaxSeenDiff=0.;
      for(;i--;)
      {
         int errchk = ErrorCheck(iDone, iTotal);

         gw_clear_maxerr(&gwdata);
         gwsetnormroutine(&gwdata, 0, errchk, bit(X,i));

         // Use square_carefully for the last 30 iterations as some PRPs have a ROUND OFF
         // error during the last iteration.
         if (i < 30)
            gwsquare_carefully(gwX);
         else
            gwsquare(gwX);

         iDone++;
         if(g_nIterationCnt && (((iDone%g_nIterationCnt)==0) || bFirst || !i))
         {
            if (*RestoreName)
               SaveState(e_gwPRP, RestoreName, iDone, &gwX, iBase, e_gwnum, N);
            static int lastLineLen;
            bFirst=false;
            // 150 bytes will not overflow, since we "force" the max size within the sprintf()
            char Buf[150];
            if (errchk && gwdata.MAXDIFF < 1e10)
               sprintf(Buf, "PRP: %.36s %d/%d mro=%.5g sum=%.2f/%.2f\r",
                       sNumStr,iDone,iTotal, gw_get_maxerr(&gwdata), gwdata.MAXDIFF, MaxSeenDiff);
            else
               sprintf(Buf, "PRP: %.36s %d/%d\r", sNumStr,iDone,iTotal);
            int thisLineLen = (int) strlen(Buf);
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

         if (CheckForFatalError("prp_using_gwnum", &gwX, iDone, iTotal, fftSize))
         {
            remove(RestoreName);
            return -1;
         }

         if (g_bExitNow)
         {
            if (*RestoreName)
               SaveState(e_gwPRP, RestoreName, iDone, &gwX, iBase, e_gwnum, N, true);

            return -2; // we really do not know at this time.  It is NOT a true error, but is undetermined, due to not comple processing
         }
      }

      X = gwX;

      // N < X only when N is of the form (k*b^n+c)/d as we used the modulus
      // k*b^n+c in the loop above.
      X %= *N;

      if (p_n64ValidationResidue)
      {
         *p_n64ValidationResidue = X & 0x7fffffff;
         *p_n64ValidationResidue |= ( (uint64)((X>>31)&0x7fffffff) << 31);
         *p_n64ValidationResidue |= ( (uint64)((X>>62)&0x00000003) << 62);
      }

      if (X==1)
         retval=1;
      else
         retval=0;
   }

   // Nuke any temp file, since we have totally processed the number.
   remove(RestoreName);

   if (ThisLineLen_Final)
      PFPrintfClearCurLine(ThisLineLen_Final);

   return retval;
}

bool CheckForFatalError(const char *caller, GWInteger *gwX, int currentIteration, int maxIterations, int fftSize)
{
   char  buffer1[200], buffer2[200], buffer3[200], buffer4[200];
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

   if (gw_get_maxerr(&gwdata) > g_dMaxErrorAllowed)
   {
      sprintf(buffer1, "Detected in MAXERR>%.2f (round off check) in %s", g_dMaxErrorAllowed, caller);
      sprintf(buffer2, "Iteration: %d/%d ERROR: ROUND OFF %.5g>%.2f", currentIteration, maxIterations, gw_get_maxerr(&gwdata), g_dMaxErrorAllowed);
      buffer3[0] = 0;
      haveFatalError = true;
   }

   if (haveFatalError)
   {
      sprintf(buffer4, "PFGW will automatically rerun the test with -a%d", fftSize+1);

      PFWriteErrorToLog(g_cpTestString, buffer1, buffer2, buffer3, buffer4);

      if (*buffer1) PFPrintf("%s\n", buffer1);
      if (*buffer2) PFPrintf("%s\n", buffer2);
      if (*buffer3) PFPrintf("%s\n", buffer3);
      if (*buffer4) PFPrintf("%s\n", buffer4);
   }

   return haveFatalError;
}
