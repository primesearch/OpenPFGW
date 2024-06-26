#include "pfgwlibpch.h"

#include "gwcontext.h"
#include "gwinteger.h"
#include "pfgw_globals.h"
#include "pfini.h"

#ifdef _MSC_VER
#include "mpir.h"
#else
#include "gmp.h"
#endif
#include "pfio.h"

#ifdef _MSC_VER
#include <io.h>
#endif

gwhandle gwdata;                       // Global variables - quite common in PFGW
bool     g_bErrorCheckAllTests = false;
bool     g_bErrorCheckThisTest = false;
double   g_dMaxError = 0.0;
const double g_dMaxErrorAllowed = 0.45;
int      g_bCollectStats = 0;          // Stats collection - no longer supported
double   sumX = 0.0;
double   sumXX = 0.0;
double   sumN = 0.0;

extern int g_CompositeAthenticationLevel;
extern bool g_bVerbose;
extern bool g_bWinPFGW_Verbose;

int  StripTrailingWhiteSpace(char *string)
{
  long  x;

  x = (long) strlen(string);

  while (x > 0 && (string[x-1] == ' '))
  {
    string[x-1] = 0x00;
    x--;
  }

  return x;
}

bool EvenlyDivides(uint64_t k, uint32_t b, uint32_t n, int32_t c, int32_t d)
{
   Integer tempInteger(b);

   tempInteger = pow(tempInteger, n);
   tempInteger *= k;
   tempInteger += c;

   if (tempInteger % d == 0)
      return true;
   else
      return false;
}

int CreateModulus(Integer *NN, char *expression, bool kbncdEligible, int increaseFFTSize)
{
   mpz_ptr gmp = NN->gmp();

   char  testString[100];
   int   error_code;
   double   k;
   uint32_t b, n;
   int32_t c, d;
 
   StripTrailingWhiteSpace(expression);

   if (increaseFFTSize == 0)
      gwset_larger_fftlen_count(&gwdata, g_CompositeAthenticationLevel);
   else
      gwset_larger_fftlen_count(&gwdata, increaseFFTSize);

   snprintf(testString, sizeof(testString), "%send1", expression);

   if (sscanf(testString, "%lf*%u^%u%dend%d", &k, &b, &n, &c, &error_code) == 5)
   {
      // We can potentially use the faster modular reduction, but we need to
      // ensure that k, b, n, and c were correctly scanned as it is possible
      // that
      snprintf(testString, sizeof(testString), "%.0lf*%u^%u%+d", k, b, n, c);
      if (!strcmp(testString, expression) && k < 1e53)
         return CreateSpecialModulus(gmp, k, b, n, c);
   }

   if (sscanf(testString, "%u^%u%dend%d", &b, &n, &c, &error_code) == 4)
   {
      snprintf(testString, sizeof(testString), "%u^%u%+d", b, n, c);
      if (!strcmp(testString, expression))
         return CreateSpecialModulus(gmp, 1.0, b, n, c);
   }

   // If this flag is set, then we will mod by (k*b^n+c)/d after the last
   // square to determine if the number is PRP.
   if (kbncdEligible)
   {
      if (sscanf(testString, "(%lf*%u^%u%d)/%dend%d", &k, &b, &n, &c, &d, &error_code) == 6)
      {
         snprintf(testString, sizeof(testString), "(%lf*%u^%u%+d)/%d", k, b, n, c, d);
         if (!strcmp(testString, expression) && k < 1e53 && EvenlyDivides((uint64_t) k, b, n, c, d))
            return CreateSpecialModulus(gmp, k, b, n, c);
      }

      if (sscanf(testString, "(%u^%u%d)/%dend%d", &b, &n, &c, &d, &error_code) == 5)
      {
         snprintf(testString, sizeof(testString), "(%u^%u%+d)/%d", b, n, c, d);
         if (!strcmp(testString, expression) && EvenlyDivides(1, b, n, c, d))
            return CreateSpecialModulus(gmp, 1.0, b, n, c);
      }

      // Phi(p,b) = (b^p-1)/(b-1)
      if (sscanf(testString, "Phi(%u,%u)/%dend%d", &n, &b, &d, &error_code) == 4)
      {
         snprintf(testString, sizeof(testString), "Phi(%u,%u)/%d", n, b, d);
         if (!strcmp(testString, expression) && EvenlyDivides(1, b, n, -1, d))
            return CreateSpecialModulus(gmp, 1.0, b, n, -1);
      }

      if (sscanf(testString, "Phi(%u,%u)end%d", &n, &b, &error_code) == 3)
      {
         snprintf(testString, sizeof(testString), "Phi(%u,%u)", n, b);
         if (!strcmp(testString, expression))
            return CreateSpecialModulus(gmp, 1.0, b, n, -1);
      }
   }

   error_code = CreateGenericModulus(gmp);

   return error_code;
}

int CreateSpecialModulus(mpz_ptr gmp, double k, unsigned long b, unsigned long n, signed long c)
{
   int   error_code;

   gwset_irrational_general_mod(&gwdata, false);
   gwset_num_threads(&gwdata, g_Threads);
   error_code = gwsetup(&gwdata, k, b, n, c);

   // debugging output
   if (error_code)
      return CreateGenericModulus(gmp);

   if (g_bVerbose || g_bWinPFGW_Verbose)
   {
      char   buf[200];
      gwfft_description (&gwdata, buf);
      PFPrintfLog("Special modular reduction using %s on %s\n", buf, gwmodulo_as_string(&gwdata));
   }

   if (gwnear_fft_limit (&gwdata, 2.0))
      g_bErrorCheckThisTest = true;      // Getting close to max bits (within 2%)

   // Tell GWNUM to use square carefully for the first few iterations
   // Passing -1 will tell GWNUM to determine how many based upon the
   // size of the modulus
   gwset_carefully_count(&gwdata, -1);

   return 0;
}

int CreateGenericModulus(mpz_ptr gmp)
{
   int error_code;

   gwset_irrational_general_mod(&gwdata, false);
   gwset_num_threads(&gwdata, g_Threads);

   if (sizeof (mp_limb_t) == sizeof (uint32_t))
      error_code = gwsetup_general_mod (&gwdata, (uint32_t *) gmp->_mp_d, gmp->_mp_size);
   else
      error_code = gwsetup_general_mod_64 (&gwdata, (uint64_t *) gmp->_mp_d, gmp->_mp_size);

   // debugging output
   if (error_code)
   {
      PFPrintfLog("Error %d initializing FFT code: ", error_code);
      return error_code;
   }

   if (g_bVerbose || g_bWinPFGW_Verbose)
   {
      char   buf[200];
      gwfft_description (&gwdata, buf);
      PFPrintfLog("Generic modular reduction using %s on %s\n", buf, gwmodulo_as_string(&gwdata));
   }

   if (gwnear_fft_limit (&gwdata, 2.0))
      g_bErrorCheckThisTest = true;      // Getting close to max bits (within 2%)

   // Tell GWNUM to use square carefully for the first few iterations
   // Passing -1 will tell GWNUM to determine how many based upon the
   // size of the modulus
   gwset_carefully_count(&gwdata, -1);

   return 0;
}

void DestroyModulus()
{
   g_dMaxError = gw_get_maxerr (&gwdata);
   gwdone (&gwdata);
}

// Code stolen from prime95's commonc.c to determine CPU type

// These wrappers are used to wrap George's ini code with our ini class
#define INI_FILE 0
void IniGetString (int, const char *szKey, char *szRetVal, int nRetValSize, const char *szDefault)
{
   PFString s, sKey=szKey, sDefault=szDefault, sSection;
   g_pIni->GetCurrentSection(sSection);
   g_pIni->SetCurrentSection("Woltman_FFTs");
   g_pIni->GetIniString(&s,&sKey,&sDefault,true);   // Switch to FALSE for "real" production run
   g_pIni->SetCurrentSection(sSection);
   strncpy(szRetVal, s, nRetValSize-1);
   szRetVal[nRetValSize-1] = 0;
}

int IniGetInt (int, const char *szKey, int nDefault, bool bSetIfNotThere=true);    // Switch to FALSE for "real" production run
int IniGetInt (int, const char *szKey, int nDefault, bool bSetIfNotThere)
{
   int n;
   PFString s, sKey=szKey, sSection;
   g_pIni->GetCurrentSection(sSection);
   g_pIni->SetCurrentSection("Woltman_FFTs");
   g_pIni->GetIniInt(&n,&sKey,nDefault,bSetIfNotThere);
   g_pIni->SetCurrentSection(sSection);
   return n;
}

void getCpuInfo (void)
{
/* Get the CPU info using CPUID instruction */

   guessCpuType ();
   guessCpuSpeed ();
}

/* Format a long or very long textual cpu description */

void getCpuDescription (
   char   *buf,         /* A 512 byte buffer */
   int   bufferSize)      /* True for a very long description */
{

/* Format a pretty CPU description */

   snprintf (buf, bufferSize, "%s\nCPU speed: %.2f MHz", CPU_BRAND, CPU_SPEED);
   if (CPU_CORES > 1 && CPU_HYPERTHREADS > 1)
      snprintf (buf + strlen (buf), bufferSize, ", %d hyperthreaded cores", CPU_CORES);
   else if (CPU_CORES > 1)
      snprintf (buf + strlen (buf), bufferSize, ", %d cores", CPU_CORES);
   else if (CPU_HYPERTHREADS > 1)
      snprintf (buf + strlen (buf), bufferSize, ", with hyperthreading");
   strcat (buf, "\n");
   if (CPU_FLAGS) {
      strcat (buf, "CPU features: ");
      if (CPU_FLAGS & CPU_RDTSC) strcat (buf, "RDTSC, ");
      if (CPU_FLAGS & CPU_CMOV) strcat (buf, "CMOV, ");
      if (CPU_FLAGS & CPU_PREFETCH) strcat (buf, "Prefetch, ");
      if (CPU_FLAGS & CPU_3DNOW) strcat (buf, "3DNow!, ");
      if (CPU_FLAGS & CPU_MMX) strcat (buf, "MMX, ");
      if (CPU_FLAGS & CPU_SSE) strcat (buf, "SSE, ");
      if (CPU_FLAGS & CPU_SSE2) strcat (buf, "SSE2, ");
      if (CPU_FLAGS & CPU_SSE41) strcat (buf, "SSE4.1, ");
      if (CPU_FLAGS & CPU_SSE42) strcat (buf, "SSE4.2, ");
      if (CPU_FLAGS & CPU_TLB_PRIMING) strcat (buf, "TLB, ");
      strcpy (buf + strlen (buf) - 2, "\n");
   }
   strcat (buf, "L1 cache size: ");
   if (CPU_L1_CACHE_SIZE < 0) strcat (buf, "unknown\n");
   else snprintf (buf + strlen (buf), bufferSize, "%d KB\n", CPU_L1_CACHE_SIZE);
   strcat (buf, "L2 cache size: ");
   if (CPU_L2_CACHE_SIZE < 0) strcat (buf, "unknown\n");
   else {
      if (CPU_L2_CACHE_SIZE & 0x3FF)
         snprintf (buf + strlen (buf), bufferSize, "%d KB\n", CPU_L2_CACHE_SIZE);
      else
         snprintf (buf + strlen (buf), bufferSize, "%d MB\n", CPU_L2_CACHE_SIZE >> 10);
   }
   if (CPU_L3_CACHE_SIZE > 0) {
      if (CPU_L3_CACHE_SIZE & 0x3FF)
         snprintf (buf + strlen (buf) - 1, bufferSize, ", L3 cache size: %d KB\n", CPU_L3_CACHE_SIZE);
      else
         snprintf (buf + strlen (buf) - 1, bufferSize, ", L3 cache size: %d MB\n", CPU_L3_CACHE_SIZE >> 10);
   }

   strcat (buf, "L1 cache line size: ");
   if (CPU_L1_CACHE_LINE_SIZE < 0) strcat (buf, "unknown\n");
   else snprintf (buf+strlen(buf), bufferSize, "%d bytes\n", CPU_L1_CACHE_LINE_SIZE);
   strcat (buf, "L2 cache line size: ");
   if (CPU_L2_CACHE_LINE_SIZE < 0) strcat (buf, "unknown\n");
   else snprintf (buf+strlen(buf), bufferSize, "%d bytes\n", CPU_L2_CACHE_LINE_SIZE);
   if (CPU_L1_DATA_TLBS > 0)
      snprintf (buf + strlen (buf), bufferSize, "L1 TLBS: %d\n", CPU_L1_DATA_TLBS);
   if (CPU_L2_DATA_TLBS > 0)
      snprintf (buf + strlen (buf), bufferSize, "%sTLBS: %d\n",
          CPU_L1_DATA_TLBS > 0 ? "L2 " : "",
          CPU_L2_DATA_TLBS);
}
