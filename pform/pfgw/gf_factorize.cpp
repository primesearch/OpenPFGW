#include "primeformpch.h"
#include <signal.h>
#include <vector>
#include <primesieve.hpp>
#include "../../pform/pfio/pfini.h"

#include "pfgw_globals.h"
#include "../pflib/bmap.cxx"

#ifndef _MSC_VER
#include <unistd.h>
#endif

static bool GF_bExtendedGFNLogic;
static uint32_t nCur_a=1;

static uint32_t GF_n_MinTry, GF_n_MaxTry, GF_n_Subs;
static bool GF_b_SaveIntermed, GF_b_QuickMode, GF_b_DoSingleCheck;

bool GF_b_DoGFFactors;  // tells WinPFGW that we are USING GFFactors.

static uint32_t *pMap_GF, *_pMap_GF;
static uint32_t *pMap_xGF[401], *_pMap_xGF[401];
static bool bGFNMapInit=false;
extern int g_CompositeAthenticationLevel;
bool CheckForFatalError(const char *caller, GWInteger *gwX, int currentIteration, int maxIterations, int fftSize);
static int GFNDivisibilityTest(const char *outputFormat, Integer *N, const char *sNumStr, Integer X,
                               int base, int x, int y, int iTotal, Integer *outX);

// Not super efficient, but it will not be called enough to make much difference
static uint32_t gcd(uint32_t x, uint32_t y)
{
   uint32_t t;
   for (;;)
   {
      if (y == 0)
         return x;
      t = y;
      y = x%y;
      x = t;
   }
}

static bool IsTrivial_GF(uint32_t a)
{
   // Filter out the perfect even powers.  NOTE the perfect odd powers are also "trivial" IF the
   if (a < 10000)
   {
      if (IsBitSet2(pMap_GF, a))
         return true;
   }
   return false; // If over 10000, then who cares.  Perfect squares are VERY sparse anyway.
}


static bool IsTrivial_xGF(uint32_t a, uint32_t b)
{
   if (b >= a)
   {
      if (b == a)
         return true;
      uint32_t t = b;
      b = a;
      a = t;
   }
   if (a > 400)   // We have to use the gcd here, since our bitmaps are only 400x400
   {
      if (gcd(a,b) != 1) // if GCD > 1 return that this is trivial
         return true;
      if (a < 10000 && b < 10000 && IsTrivial_GF(a) && IsTrivial_GF(b))  // if a and b are perfect square, then return this is trivial
         return true;
      return false;
   }
   if (IsBitSet2(pMap_xGF[a], b))
      return true;
   return false;
}

static void _InitGFTrivials()
{
   Init_pMap2(1, 10000, &_pMap_GF, &pMap_GF);
   uint32_t nvalsleft, i, j;
   Set_All_bits_false2(1, 10000, nvalsleft, pMap_GF);
   // Filter out the perfect even powers.  NOTE the perfect odd powers are also "trivial" IF the
   for (i = 2; i <= 100; ++i)
      SetBit2(pMap_GF, i*i);

   // For xGF, we want gcd(a,b) == 1.  There are "others" that are trivial, but this will eliminate "most" of them.
   for (i = 2; i <= 400; ++i)
   {
      Init_pMap2(1, i, &(_pMap_xGF[i]), &(pMap_xGF[i]));
      Set_All_bits_false2(1, i, nvalsleft, pMap_xGF[i]);
      for (j = 2; j < i; ++j)
      {
         // Eliminate all j,i pairs with gcd > 1   i.e.   15,42 is a factor for all 5,14's
         // We also need to elimate all double perfect squares.  i.e. 9,4 is a factor for all 3,2 and 25,16 for all 5,4
         if (gcd(j,i) > 1 || (IsTrivial_GF(i) && IsTrivial_GF(j)))
            SetBit2(pMap_xGF[i], j);
      }
   }
}
// This can get called often, so make it VERY fast (i.e. all it does is check bGFNMapInit bool most of the time)
inline void InitGFTrivials()
{
   if (bGFNMapInit) return;
   bGFNMapInit=true;
   _InitGFTrivials();
}

static void FreeGFTrivials()
{
   if (!bGFNMapInit) return;
   bGFNMapInit=false;
   for (uint32_t i = 400; i >= 2; --i)
      Free_pMap2(&(_pMap_xGF[i]));
   Free_pMap2(&_pMap_GF);
}

bool IsValidGF_FactorForm(const char *sNumber, Integer *k, uint32_t *n)
{
   // See if sNumber is of the form k*2^n+1.   (Now also handles k.2^n+1 since * and . are both multiply chars)
   //
   //NOTE that  (3425*12321)*2^123+1 will FAIL!  You have to use "simple" expressions for this parser.
   //
   const char *cp = strchr(sNumber, '*');
   if (!cp)
      // Handle either form of k*2^n+1 of k.2^n+1
      cp = strchr(sNumber, '.');
   if (!cp)
      return false;
   int nKLen = (int) (cp-sNumber);
   char *cpKNum = new char [nKLen+1];
   memcpy(cpKNum, sNumber, nKLen);
   cpKNum[nKLen] = 0;
   for (int i = 0; i < nKLen; i++)
   {
      if (cpKNum[i] < '0' || cpKNum[i] > '9')
      {
         delete[] cpKNum;
         return false;
      }
   }
   if (k)
      k->atoI(cpKNum);
   delete[] cpKNum;
   cp++;
   if (*cp++ != '2')
      return false;
   if (*cp++ != '^')
      return false;
   if (n)
      *n = atoi(cp);
   while (*cp >= '0' && *cp <= '9')
      cp++;

   return !strcmp(cp, "+1");
}

struct GF_SubFactor
{
   Integer subN;
   int      prime;
}*GF_Subs;

static int gwGF_LoadSubs(Integer *N, const char *sNumStr, Integer *k, uint32_t n)
{
   int bRetval = 0;

   static Integer XX, X, Nm1;
   static uint32_t last_N;
   if (last_N != n)
   {
      mpz_set_ui(X.gmp(),1);
      mpz_mul_2exp(X.gmp(), X.gmp(), n-1);
      last_N = n;
   }
   Nm1 = (*N)-1;

   char *k_text = k->Itoa();
   CTimer Timer;
   Timer.Start();
   for (uint32_t j = 0; j < GF_n_Subs; j++)
   {
      char GF_IntermedFName[120];
      sprintf (GF_IntermedFName, "%s_%d_%d.gfi", k_text, n, GF_Subs[j].prime);
#ifdef _MSC_VER
      if (!_access_s(GF_IntermedFName, 0))
#else
      if (!access(GF_IntermedFName, 0))
#endif
      {
         // The file was saved from a prior run, so load it and continue
         FILE *in = fopen(GF_IntermedFName, "rb");
         // I sure wish the filelength() function was ANSI!
         fseek(in, 0, SEEK_END);
         unsigned fLen = ftell(in);
         fseek(in, 0, SEEK_SET);
         char *Buf = new char[fLen+1];
         Buf[fLen] = 0;
         fread(Buf, 1, fLen, in);
         fclose(in);
         GF_Subs[j].subN.atoI(Buf);
         delete[] Buf;
         continue;
      }

      bRetval = GFNDivisibilityTest("GF_sprime_%d: %.50s %d/%d       \r", N, sNumStr, X, GF_Subs[j].prime, 0, 0, n-1, &GF_Subs[j].subN);

      if (GF_b_SaveIntermed)
      {
         FILE *out = fopen(GF_IntermedFName, "wt");
         char *pTmp = GF_Subs[j].subN.Itoa();
         fwrite(pTmp, 1, strlen(pTmp), out);
         fclose(out);
         delete[] pTmp;
      }
   }
   delete[] k_text;
   return bRetval;
}

static int GFNDivisibilityTest(const char *outputFormat, Integer *N, const char *sNumStr, Integer X,
                               int base, int x, int y, int iTotal, Integer *outX)
{
   int   fftSize, haveError;
   uint32_t lastLineLength = 0;
   int   ii, iDone, errchk;
   char  temp[120];
   CTimer Timer;
   GWInteger *gwX;

   fftSize = g_CompositeAthenticationLevel - 1;
   do
   {
      fftSize++;
      haveError = false;
      Timer.Start();

      gwinit2(&gwdata, sizeof(gwhandle), (char *) GWNUM_VERSION);
      gwsetmaxmulbyconst(&gwdata, base);  // maximum multiplier

      if (CreateModulus(N, g_cpTestString, true, fftSize)) return 0;

      gwX = new GWInteger;

      iDone = 0;
      *gwX = base;                        // initialise X
      gwsetmulbyconst(&gwdata, base);     // and multiplier

      for (ii=iTotal; ii--; )
      {
         iDone++;
         errchk = ErrorCheck(iDone, iTotal);

         gw_clear_maxerr(&gwdata);
         gwsetnormroutine(&gwdata, 0, errchk, bit(X, ii));

         if (ii < 30)
            gwsquare2_carefully(*gwX);
         else
            gwsquare2(*gwX);

         if (g_nIterationCnt && (((iDone%g_nIterationCnt)==0) && Timer.GetSecs() > 2/*|| bFirst || !i*/))
         {
            static int lastLineLen;

            Timer.Start();

            if (x || y)
               sprintf(temp, outputFormat, x, y, sNumStr, iDone, iTotal);
            else
               sprintf(temp, outputFormat, base, sNumStr, iDone, iTotal);

            if (lastLineLength > strlen(temp))
               // When mixing stdio, stderr and redirection with a \r stderr output,
               // then the line must "erase" itself, IF it ever shrinks.
               PFPrintfClearCurLine(lastLineLength);

            lastLineLen = (int) strlen(temp);
            PFPrintfStderr("%s", temp);
            PFfflush(stderr);
         }

         if (CheckForFatalError("GFNDivisibilityTest", gwX, iDone, iTotal, fftSize))
            haveError = true;

         if (haveError || g_bExitNow)
            return 0;
      }

      *outX = *gwX;
      DestroyModulus();
      delete gwX;
   } while (haveError && !g_bExitNow && fftSize < 5);

   return haveError;
}

static int gwGF_LoadSubs_gmp(Integer *N, uint32_t n)
{
   int bRetval = 0;

   static Integer X, Nm1;
   static uint32_t last_N;
   if (last_N != n)
   {
      mpz_set_ui(X.gmp(),1);
      mpz_mul_2exp(X.gmp(), X.gmp(), n-1);
      last_N = n;
   }
   Nm1 = (*N)-1;

   for (uint32_t j = 0; j < GF_n_Subs; j++)
   {
      Integer BASE(GF_Subs[j].prime);
      mpz_powm( GF_Subs[j].subN.gmp(), BASE.gmp(), X.gmp(), N->gmp());
   }
   return bRetval;
}

// This function is only called from ProcessGF_Factor
static int gwGF_Factor(Integer *N, uint32_t n, uint32_t gfn_base, uint32_t *gfn_exp, const char *sNumStr)
{
   int bRetval=0;

   static Integer XX, X, Nm1, XXa;
   static uint32_t last_N;
   uint32_t gfn_exp_max = --n;
   // if kro(b,N) == -1 then b^2^(n-1) can NOT be == +1 so the N-1 residue can NOT happen before n-1, so simply
   // ignore the starting check from b^2^(n-20) and simply go straight to b^2^(n-1)
   CTimer Timer;
   Timer.Start();
   if(!GF_bExtendedGFNLogic || nCur_a == 1)
   {
      // "Traditional" GFN search
      if (kro(gfn_base,*N)!=-1)
      {
         if(n > 30)
            n -= 30;    // X/=2^20
         else if (n > 10)
            n -= 10;    // X/=2^10
         else if (n > 4)
            n -= 3;        // X/=2^3
         else if (n > 2)
            --n;        // X/=2^1
      }
      if (last_N != n)
      {
         mpz_set_ui(X.gmp(),1);
         mpz_mul_2exp(X.gmp(), X.gmp(), n);
         last_N = n;
      }
      Nm1 = (*N)-1;

      bRetval = GFNDivisibilityTest("GF_%d: %.50s %d/%d           \r", N, sNumStr, X, gfn_base, 0, 0, n, &XX);

      if (XX == 1)
      {
         bRetval=1;
         // This is NOT right, but it is as close as we can get.  The base was MORE than 20 less than
         // the exponent.  Even if the "answer" is not correct, at least inform the user of the found factor.
         PFPrintfLog("\nA GF Factor was found, but the base of %d may not be correct.\n", n-1);
         *gfn_exp = n-1;
      }
      else
      {
         while (XX != Nm1 && n < gfn_exp_max)
         {
            XX *= XX;
            XX %= *N;
            n++;
         }
         if (XX==Nm1)
         {
            bRetval=1;
            *gfn_exp = n;
         }
      }

      return bRetval;
   }

   // Extended GFN search
   if(n > 30)
      n -= 30;    // X/=2^20
   else if (n > 10)
      n -= 10;    // X/=2^10
   else if (n > 4)
      n -= 3;        // X/=2^3
   else if (n > 2)
      --n;        // X/=2^1
   uint32_t nStart = n;
   if (last_N != n)
   {
      mpz_set_ui(X.gmp(),1);
      mpz_mul_2exp(X.gmp(), X.gmp(), n);
      last_N = n;
   }
   Nm1 = (*N)-1;

   bRetval = GFNDivisibilityTest("xGF_'%d'_%d: %.50s %d/%d       \r", N, sNumStr, X, gfn_base, gfn_base, nCur_a, n, &XX);
   bRetval = GFNDivisibilityTest("xGF_%d_'%d': %.50s %d/%d       \r", N, sNumStr, X, nCur_a, gfn_base, nCur_a, n, &XXa);

   while((XX+XXa) % *(N) != 0 && n < gfn_exp_max)
   {
      XX *= XX;
      XX %= *N;
      XXa *= XXa;
      XXa %= *N;
      n++;
   }
   if((XX+XXa) % *(N) == 0)
   {
      bRetval=1;
      *gfn_exp = n;
   }
   else
   {
      bRetval=1;
      *gfn_exp = nStart-1;
      PFPrintfLog("\nA GF Factor was found, but the base of %d may not be correct.\n", *gfn_exp);
   }


   return bRetval;
}

// This function is only called from ProcessGF_Factor
static int gwGF_Factor_gmp(Integer *N, uint32_t n, uint32_t gfn_base, uint32_t *gfn_exp)
{
   int bRetval=0;

   static Integer XX, XXa, X, Nm1;
   static uint32_t last_N;

   uint32_t gfn_exp_max = --n;
   if(!GF_bExtendedGFNLogic || nCur_a == 1)
   {
      // "Traditional" GFN search
      // if kro(b,N) == -1 then b^2^(n-1) can NOT be == +1 so the N-1 residue can NOT happen before n-1, so simply
      // ignore the starting check from b^2^(n-20) and simply go straight to b^2^(n-1)
      if (kro(gfn_base,*N)!=-1)
      {
         if(gfn_exp_max > 30)
            n -= 30;    // X/=2^20
         else if (gfn_exp_max > 10)
            n -= 10;    // X/=2^10
         else if (gfn_exp_max > 4)
            n -= 3;        // X/=2^3
         else if (gfn_exp_max > 2)
            --n;        // X/=2^1
      }
      if (last_N != n)
      {
         mpz_set_ui(X.gmp(),1);
         mpz_mul_2exp(X.gmp(), X.gmp(), n);
         last_N = n;
      }
      Nm1 = (*N)-1;

      Integer BASE(gfn_base);

      mpz_powm( XX.gmp(), BASE.gmp(), X.gmp(), N->gmp());

      if (XX==1)
      {
         bRetval=1;
         // This is NOT right, but it is as close as we can get.  The base was MORE than 20 less than
         // the exponent.  Even if the "answer" is not correct, at least inform the user of the found factor.
         PFPrintfLog("\nA GF Factor was found, but the base of %d may not be correct.\n", n-1);
         *gfn_exp = n-1;
      }
      else
      {
         while(XX != Nm1 && n < gfn_exp_max)
         {
            XX *= XX;
            XX %= *N;
            n++;
         }
         if(XX==Nm1)
         {
            bRetval=1;
            *gfn_exp = n;
         }
      }
      return bRetval;
   }

   // Extended GFN search
   if(gfn_exp_max > 30)
      n -= 30;    // X/=2^20
   else if (gfn_exp_max > 10)
      n -= 10;    // X/=2^10
   else if (gfn_exp_max > 4)
      n -= 3;        // X/=2^3
   else if (gfn_exp_max > 2)
      --n;        // X/=2^1
   uint32_t nStart = n;
   if (last_N != n)
   {
      mpz_set_ui(X.gmp(),1);
      mpz_mul_2exp(X.gmp(), X.gmp(), n);
      last_N = n;
   }
   Nm1 = (*N)-1;

   Integer BASE(gfn_base);
   Integer BASEa(nCur_a);
   mpz_powm( XX.gmp(), BASE.gmp(), X.gmp(), N->gmp());
   mpz_powm( XXa.gmp(), BASEa.gmp(), X.gmp(), N->gmp());

   while((XX+XXa) % *(N) != 0 && n < gfn_exp_max)
   {
      XX *= XX;
      XX %= *N;
      XXa *= XXa;
      XXa %= *N;
      n++;
   }
   if((XX+XXa) % *(N) == 0)
   {
      bRetval=1;
      *gfn_exp = n;
   }
   else
   {
      bRetval=1;
      *gfn_exp = nStart-1;
      PFPrintfLog("\nA GF Factor was found, but the base of %d may not be correct.\n", *gfn_exp);
   }
   return bRetval;
}

static Integer *pIntegerVals;

void CleanupGFs()
{
   delete[] GF_Subs;
   GF_Subs = 0;
   delete[] pIntegerVals;
   pIntegerVals = 0;
   FreeGFTrivials();
}

uint32_t s_Factors[1000];
uint32_t s_nFactors;

bool OnlySmallFactors(uint32_t n)
{
   if (!GF_bExtendedGFNLogic && IsTrivial_GF(n))
      return false;
   s_nFactors = 0;
   for (uint32_t i = 0; i < GF_n_Subs && n > 1; i++)
   {
      while(n%GF_Subs[i].prime == 0)
      {
         n/=GF_Subs[i].prime;
         s_Factors[s_nFactors++] = i;
      }
   }
   return n==1;
}

static void DumpPatterns()
{
   uint32_t TotTests=0;
   for (uint32_t a = GF_n_MinTry; a <= GF_n_MaxTry; a++)
   {
      if (OnlySmallFactors(a))
      {
         if (a == 2)
            TotTests += !!PFPrintfLog ("Test for F??\n");
         else if (!IsTrivial_GF(a))
            TotTests += !!PFPrintfLog ("Test for GF(??,%d)\n", a);

         if (GF_bExtendedGFNLogic)
         {
            for (uint32_t b = GF_n_MinTry; b < a; b++)
               if (OnlySmallFactors(b) && !IsTrivial_xGF(a, b))
                  TotTests += !!PFPrintfLog ("Test for xGF(??,%d,%d)\n", a, b);
         }
      }
   }
   PFPrintfLog ("There are %d total F/GF/xGF tests being performed\n", TotTests);
}

//default is -g[o]{2,5}{2,12}
void Parse_GF_FactorCommandLine(const char *sCmdLine, bool *bOnlyGFNs)
{
   bool DumpSearchPatterns=false;
   GF_bExtendedGFNLogic = false;
   GF_b_SaveIntermed = false;
   GF_b_QuickMode = false;
   GF_b_DoSingleCheck = false;
   *bOnlyGFNs = false;

   GF_b_DoGFFactors = true;   // Tell WinPFGW we are doing [x]GF work.

   // Check for an appended _dump_search_patterns string.
   if (strstr(sCmdLine, "_dump_search_patterns"))
      DumpSearchPatterns=true;

   InitGFTrivials();

   // find out if this is a -gx[o][...] "extended" GF divisiblity check
   // The extended GF's are of the form a^2^n+b^2^n  Note if b==1, then we
   // have the "normal"
   if (sCmdLine && (*sCmdLine == 'x' || *sCmdLine == 'X'))
   {
      sCmdLine++;
      GF_bExtendedGFNLogic = true;
   }

   while (sCmdLine && *sCmdLine)
   {
      // find if it is a -go...
      if ((*sCmdLine == 'o' || *sCmdLine == 'O'))
      {
         sCmdLine++;
         *bOnlyGFNs = true;
      }
      // find if it is a -g[o]q...
      else if ((*sCmdLine == 'q' || *sCmdLine == 'Q'))
      {
         sCmdLine++;
         GF_b_QuickMode = true;
      }
      // find if it is a -g[o]s...
      else if ((*sCmdLine == 's' || *sCmdLine == 'S'))
      {
         sCmdLine++;
         GF_b_SaveIntermed = true;
      }
      else
         break; // at the end of the "real" data
   }

   delete[] GF_Subs;
   GF_Subs = 0;

   delete[] pIntegerVals;
   pIntegerVals = 0;

   if (!sCmdLine || !*sCmdLine)
   {
UseDefault:;
      //default is -g{2,5}{2,12} or for -gx it is -gx{2,11}{2,12}
      GF_n_MinTry = 2;
      GF_n_MaxTry = 12;
      GF_n_Subs = 3;
      GF_Subs = new GF_SubFactor[/*GF_n_Subs*/5];
      GF_SubFactor *p = GF_Subs;
      p[0].prime = 2;
      p[1].prime = 3;
      p[2].prime = 5;
      if (GF_bExtendedGFNLogic)
      {
         GF_n_Subs = 5;
         p[3].prime = 7;
         p[4].prime = 11;
      }
      pIntegerVals = new Integer[13];
      for (uint32_t i = 0; i < 13; ++i)
         pIntegerVals[i] = -1;

      // Ok now dump out what we are searching for:
      if (DumpSearchPatterns)
         DumpPatterns();
      return;
   }


   // Allow -g[o][s]#  to check just a single number.
   if (*sCmdLine != '{')
   {
      GF_n_MinTry = GF_n_MaxTry = 0xFFFFFFFF;
      sscanf(sCmdLine, "%d,%d", &GF_n_MinTry, &GF_n_MaxTry);
      if (GF_n_MaxTry != 0xFFFFFFFF)
      {
         if (!GF_bExtendedGFNLogic)
         {
            PFOutput::EnableOneLineForceScreenOutput();
            PFPrintfStderr("Error, xGF divisibility command line switch was not correct format (x was missing), using default syntax\n");
            goto UseDefault;
         }
         if (GF_n_MinTry > GF_n_MaxTry)
         {
            uint32_t t = GF_n_MaxTry;
            GF_n_MaxTry = GF_n_MinTry;
            GF_n_MinTry = t;
         }
         GF_Subs = new GF_SubFactor[2];
         GF_Subs[0].prime = GF_n_MinTry;
         GF_Subs[1].prime = GF_n_MaxTry;
         GF_n_Subs = 2;
         pIntegerVals = new Integer[GF_n_MaxTry+1];
         for (uint32_t i = 0; i < GF_n_MaxTry+1; ++i)
         {
            if (i != GF_n_MinTry)
               pIntegerVals[i] = -1;
         }
         // Ok now dump out what we are searching for:
         if (DumpSearchPatterns)
            DumpPatterns();
         GF_b_DoSingleCheck=true;
         return;
      }
      GF_n_MinTry = GF_n_MaxTry = 0xFFFFFFFF;
      sscanf(sCmdLine, "%d", &GF_n_MinTry);
      if (GF_n_MinTry == 0xFFFFFFFF)
      {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("Error, GF divisibility command line switch was not correct format, using default syntax\n");
         goto UseDefault;
      }
      GF_n_MaxTry = GF_n_MinTry;
      GF_Subs = new GF_SubFactor[1];
      GF_Subs[0].prime = GF_n_MaxTry;
      GF_n_Subs = 1;
      pIntegerVals = new Integer[GF_n_MaxTry+1];
      for (uint32_t i = 0; i < GF_n_MaxTry+1; ++i)
         pIntegerVals[i] = -1;
      // Ok now dump out what we are searching for:
      if (DumpSearchPatterns)
         DumpPatterns();
      GF_b_DoSingleCheck=true;
      return;
   }

   unsigned minPrime, maxPrime;
   if (sscanf(sCmdLine, "{%d,%d}{%d,%d}", &minPrime, &maxPrime, &GF_n_MinTry, &GF_n_MaxTry) != 4)
   {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Error, GF divisibility command line switch was not correct format, using default syntax\n");
      goto UseDefault;
   }
   
   uint32_t Cnt = 0;

   minPrime--;

   std::vector<uint64_t> vPrimes;
   std::vector<uint64_t>::iterator it;

   vPrimes.clear();

   primesieve::generate_primes(minPrime, maxPrime, &vPrimes);

   GF_n_Subs = (uint32_t) vPrimes.size();
   GF_Subs = new GF_SubFactor[GF_n_Subs];

   it = vPrimes.begin();
   while (it != vPrimes.end())
   {
      GF_Subs[Cnt++].prime = (uint32_t) *it;
      it++;
   }

   pIntegerVals = new Integer[GF_n_MaxTry+1];
   for (uint32_t i = 0; i < GF_n_MaxTry+1; ++i)
      pIntegerVals[i] = -1;

   // Ok now dump out what we are searching for:
   if (DumpSearchPatterns)
      DumpPatterns();
}


static void ProcessGF_Factor(Integer *N, const char *sNumStr, uint32_t n, uint32_t gfn_base)
{
   clock_t start=clock();
   uint32_t gfn_exp;
   int bRetVal;

   if (numbits(*N) < 800)
      bRetVal = gwGF_Factor_gmp(N, n, gfn_base, &gfn_exp);
   else
      bRetVal = gwGF_Factor(N, n, gfn_base, &gfn_exp, sNumStr);

   if (bRetVal == -1)
      return;

   if (g_bExitNow)
   {
      g_bExited = true;
      return;
   }
   double t=double(clock()-start)/clocks_per_sec;

   if (bRetVal)
   {
      // use factor as "Factor" so that the non-verbose mode of WinPFGW does not throw these
      // strings away.

      if (nCur_a == 1)
      {
         if (gfn_base == 2)
            PFPrintfLog("%s is a Factor of F%d!!!! (%f seconds)\n",LPCTSTR(sNumStr), gfn_exp, t);
         else
            PFPrintfLog("%s is a Factor of GF(%d,%d)!!!! (%f seconds)\n",LPCTSTR(sNumStr), gfn_exp, gfn_base, t);
      }
      else
         PFPrintfLog("%s is a Factor of xGF(%d,%d,%d)!!!! (%f seconds)\n",LPCTSTR(sNumStr), gfn_exp, gfn_base, nCur_a, t);
      PFfflush(stdout);

      // All fermat factors are PRIMES, so in actuality, this number has been PROVEN!
      // The above statement is NOT valid.  There are infinite examples of were 2 GF factors can be multiplied together
      // can be converted into a k*2^n+1 form, and this will still be a factor, but not a prime factor.  However, the
      // logic of placing these into pfgw-prime.log should be OK.  The only caveat is when someone runs a -go  (gfn
      // only test without prp'ing the numbers).  Then it is easy to drop these composite factors out.
      FILE *f=fopen("pfgw-prime.log","at");
      if(f)
      {
         fseek(f,0L,SEEK_END);
         if (nCur_a == 1)
         {
            if (gfn_base == 2)
               fprintf(f,"%s is a Factor of F%d!!!!\n",LPCTSTR(sNumStr), gfn_exp);
            else
               fprintf(f,"%s is a Factor of GF(%d,%d)!!!!\n",LPCTSTR(sNumStr), gfn_exp, gfn_base);
         }
         else
            fprintf(f,"%s is a Factor of xGF(%d,%d,%d)!!!!\n",LPCTSTR(sNumStr), gfn_exp, gfn_base, nCur_a);
         fclose(f);
      }
   }
}

bool ProcessGF_Factors(Integer *N, const char *sNumStr)
{
   static Integer k;  // we don't need to call constructor every time.
   static Integer OrigI;
   uint32_t n, b;
   int    bRetval;
   Integer I, J;

   if (!IsValidGF_FactorForm(LPCTSTR(sNumStr), &k, &n))
      return false;

   uint32_t exp_m1=n-1;
   clock_t start=clock();

   if (GF_b_DoSingleCheck)
   {
      // Skip all the code below, and simply head straight to the test for a single factor code
      if (!GF_bExtendedGFNLogic)
      {
         nCur_a = 1;
         b = GF_Subs[0].prime;
      }
      else
      {
         nCur_a = GF_Subs[0].prime;
         b = GF_Subs[1].prime;
      }
      ProcessGF_Factor(N, sNumStr, n, b);
      return true;
   }

   if (numbits(*N) < 800)
      bRetval = gwGF_LoadSubs_gmp(N, n);
   else
      bRetval = gwGF_LoadSubs(N, sNumStr, &k, n);

   if (bRetval == -1)
      return true;

   PFPrintfClearCurLine();
   Integer Nm1 = (*N) - 1;
   int Cnt = 0;

   // Precompute the "valid" bases (only compute them one time, then for the xGF, we simply use a copy constructor.
   CTimer Timer;
   Timer.Start();

   bool bIsFermat=false;

   for (b = GF_n_MinTry; b <= GF_n_MaxTry; b++)
   {
      if (OnlySmallFactors(b))
      {
         if (Cnt++ % 50 == 0 && Timer.GetSecs() > 1.)
         {
            Timer.Start();
            PFPrintfStderr("GF_chk_%d:   \r", b);
         }
         pIntegerVals[b] = GF_Subs[s_Factors[0]].subN;
         for (uint32_t j = 1; j < s_nFactors; j++)
         {
            pIntegerVals[b] *= GF_Subs[s_Factors[j]].subN;
            pIntegerVals[b] %= (*N);
         }
      }
   }


   for (b = GF_n_MinTry; b <= GF_n_MaxTry; b++)
   {
      nCur_a = 1;
      if (OnlySmallFactors(b))
      {
         if (Cnt++ % 50 == 0 && Timer.GetSecs() > 1.)
         {
            Timer.Start();
            PFPrintfStderr("GFx_chk_%d:   \r", b);
         }

         I = pIntegerVals[b];
Try_Next_a:;
         if (GF_bExtendedGFNLogic)
         {
            if (nCur_a == 1)
               OrigI = I;
            else
            {
               if (OnlySmallFactors(nCur_a))
               {
                  J = pIntegerVals[nCur_a];
                  if (J == OrigI)
                  {
                     // Not sure why, but J can == OrigI, and if so, then attempt to find a factor.
                     // frequently this will be J==1 and OrigI==1, but it can be J==OrigI == 1, but
                     // even then there is a factor.

                     // set I to 1, so that it will be checked by ProcessGF_Factor() and the factor found.
                     I = 1;
                  }
                  else
                  {
                     I = J+OrigI;
                     // since we check for N-1 and 1 below, we MUST adjust I by 1.  We don't do this
                     // in the nCur_a==1 case, but in that case we also don't add the 1 (and the logic
                     // stays in the "original" GFN logic).
                     --I;
                     I %= (*N);
                  }
               }
               else
                  I = 0;
            }
         }

         if (I == Nm1)
         {
            double t=double(clock()-start)/clocks_per_sec;
            start=clock();
            bool bValid = false;
            if (nCur_a == 1)
            {
               if (!IsTrivial_GF(b))
               {
                  bValid = true;
                  if (b == 2)
                  {
                     bIsFermat = true;
                     PFPrintfLog("%s is a Factor of F%d!!!! (%f seconds)\n",LPCTSTR(sNumStr), exp_m1, t);
                  }
                  else
                  {
                     if (bIsFermat && b == 8)   // don't output GF-8 for Fermat factors
                        ; // do nothing
                     else
                        PFPrintfLog("%s is a Factor of GF(%d,%d)!!!! (%f seconds)\n",LPCTSTR(sNumStr), exp_m1, b, t);
                  }
               }
            }
            else if (!IsTrivial_xGF(b, nCur_a))
            {
               bValid = true;
               PFPrintfLog("%s is a Factor of xGF(%d,%d,%d)!!!! (%f seconds)\n",LPCTSTR(sNumStr), exp_m1, b, nCur_a, t);
            }
            if (bValid)
               PFfflush(stdout);

            // All fermat factors are PRIMES, so in actuality, this number has been PROVEN!
            if (bValid)
            {
               FILE *f=fopen("pfgw-prime.log","at");
               if(f)
               {
                  fseek(f,0L,SEEK_END);
                  if (nCur_a == 1)
                  {
                     if (b == 2)
                        fprintf(f,"%s is a Factor of F%d!!!!\n",LPCTSTR(sNumStr), exp_m1);
                     else
                     {
                        if (bIsFermat && b == 8)  // don't output GF-8 for Fermat factors
                           ; // do nothing
                        else
                           fprintf(f,"%s is a Factor of GF(%d,%d)!!!!\n",LPCTSTR(sNumStr), exp_m1, b);
                     }
                  }
                  else
                     fprintf(f,"%s is a Factor of xGF(%d,%d,%d)!!!!\n",LPCTSTR(sNumStr), exp_m1, b, nCur_a);
                  fclose(f);
               }
            }
         }
         else if (I == 1)
         {
            bool bValid = false;
            if (nCur_a == 1)
            {
               if (!IsTrivial_GF(b))
                  bValid = true;
               if (b == 8 && bIsFermat)
                  bValid = false;
            }
            else if (!IsTrivial_xGF(b, nCur_a))
               bValid = true;
            if (bValid)
            {
               if (b == 2 && nCur_a == 1)
                  bIsFermat = true;
               if (GF_b_QuickMode)
               {
                  double t=double(clock()-start)/clocks_per_sec;
                  // We were told to NOT re-exponentate, but simply report results.
                  if (b == 2)
                     PFPrintfLog("%s is a Factor of F%d-? !!!! (%f seconds)\n",LPCTSTR(sNumStr), exp_m1, t);
                  else if (nCur_a == 1)
                  {
                     if (bIsFermat && b == 8)   // don't output GF-8 for Fermat factors
                        ; // do nothing
                     else
                        PFPrintfLog("%s is a Factor of GF(%d-? ,%d)!!!! (%f seconds)\n",LPCTSTR(sNumStr), exp_m1, b, t);
                  }
                  else
                     PFPrintfLog("%s is a Factor of xGF(%d-? ,%d,%d)!!!! (%f seconds)\n",LPCTSTR(sNumStr), exp_m1, b, nCur_a, t);
                  PFfflush(stdout);

                  FILE *f=fopen("pfgw-prime.log","at");
                  if(f)
                  {
                     fseek(f,0L,SEEK_END);
                     if (nCur_a == 1)
                     {
                        if (b == 2)
                           fprintf(f,"%s is a Factor of F%d-? !!!!\n",LPCTSTR(sNumStr), exp_m1);
                        else
                        {
                           if (bIsFermat && b == 8)  // don't output GF-8 for Fermat factors
                              ; // do nothing
                           else
                              fprintf(f,"%s is a Factor of GF(%d-? ,%d)!!!!\n",LPCTSTR(sNumStr), exp_m1, b);
                        }
                     }
                     else
                        fprintf(f,"%s is a Factor of xGF(%d-? ,%d,%d)!!!!\n",LPCTSTR(sNumStr), exp_m1, b, nCur_a);
                     fclose(f);
                  }
               }
               else
                  /* perform the FULL factor finding */
                  ProcessGF_Factor(N, sNumStr, n, b);
            }
         }

         if (g_bExitNow)
            break;
         if (GF_bExtendedGFNLogic)
         {
            if (nCur_a == 1)
               nCur_a = GF_n_MinTry;
            else
               ++nCur_a;
            if (nCur_a < b)
               goto Try_Next_a;
         }
      }
   }

   PFPrintfLog("GFN testing completed for %s\n", LPCTSTR(sNumStr));

   return true;
}
