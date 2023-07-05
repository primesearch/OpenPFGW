#if !defined (PFGW_GLOBALS_H__)
#define PFGW_GLOBALS_H__

extern int iBase;
extern int g_Threads;
extern int g_nIterationCnt;
extern bool volatile g_bExitNow;
extern bool volatile g_bExited;
extern unsigned long clocks_per_sec;			// Machine dependent
extern char g_cpTestString[70];
extern bool g_bGMPMode;
extern PFString g_sTestMode;				// This will hold things like "PRP: ", "N+1: ", "F: ", "GF(b,3): ", ...

// Under Windows, it's <io.h>, here's what it is for autconf
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#elif _MSC_VER
#include "windows.h"
#include <io.h>
#endif

#ifndef _MSC_VER
#define Sleep(x) usleep((x)*1000)
#endif

#undef GWDEBUG
#undef INTDEBUG
#define GWDEBUG(X) {Integer XX;XX=X;printf(#X "=");mpz_out_str(stdout,16,XX.gmp());printf(" ");}
#define INTDEBUG(X) {printf(#X "=");mpz_out_str(stdout,16,(X).gmp());printf("\n");}

// in save_restore_prp.cpp
enum ePRPType {e_gwPRP, e_GFN_DTWPRP, e_Phi_DTWPRP, e_GFN_Factorize};
enum eContextType {e_FFTW=2, e_gwnum};

void CreateRestoreName(Integer *N, char RestoreName[13]);
bool RestoreState(ePRPType e_gwPRP, char *RestoreName, uint32_t *iDone, GWInteger *gwX, uint32_t iBase, eContextType eCType);
bool SaveState(ePRPType e_gwPRP, char *RestoreName, uint32_t iDone, GWInteger *gwX, uint32_t iBase, eContextType eCType, Integer *N, bool bForce=false);


// in gw_prp.cpp
int gwPRP(Integer *N, const char *sNumStr, uint64_t *p_n64ValidationResidue);
void bench_gwPRP(Integer *N, uint32_t iterations);
int prp_using_gwnum(Integer *N, uint32_t iiBase, const char *sNumStr, uint64_t *p_n64ValidationResidue, int fftSize);

// in gw_gapper.cpp
void gw_gapper(const char *FName, int MinGap, uint64_t restart=0);

// in gf_factorize.cpp
bool IsValidGF_FactorForm(const char *sNumber, Integer *k=NULL, uint32_t *n=NULL);
void Parse_GF_FactorCommandLine(const char *sCmdLine, bool *bOnlyGFNs);
// The return from ProcessGF_Factors is only stating that the number was proper form or not.  NOT whether
// the number was a factor or not.  By returning the result of the IsValidGF_FactorForm, then we can avoid
// having to double call that function in -go mode
bool ProcessGF_Factors(Integer *N, const char *sNumStr);

// in bench.cpp
void vectorout(PFString &sFilename,PFSymbolTable *psymRuntime);
void bench(PFSymbolTable *pContext);
void benchmark(PFSymbolTable *pContext, char *parameter);

extern "C" int pfgw_main(int argc,char *argv[]);
void pfgw_main_init();
void pfgw_main_cleanup();

inline int ErrorCheck(int current, int max)
{
   // Automatically error check all iterations
   if (g_bErrorCheckAllTests || g_bErrorCheckThisTest)
      return 1;

   // Error check first 50 and last 50 iterations
   if (current < 50 || current > max - 50)
      return 1;

   if (current & 0x7f)
      return 0;

   // Error check every 128th iteration
   return 1;
}

/* used to calculate frequency of Gerbicz Error Checks in gw_prp.c */
/* from rosettacode.org */
inline int64_t lsqrt(int64_t x) {
   int64_t q = 1, r = 0;
   while (q <= x) {
      q <<= 2;
   }
   while (q > 1) {
      int64_t t;
      q >>= 2;
      t = x - r - q;
      r >>= 1;
      if (t >= 0) {
         x = t;
         r += q;
      }
   }
   return r;
}

#endif // PFGW_GLOBALS_H__
