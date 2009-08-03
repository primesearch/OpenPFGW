#include "pfgwlibpch.h"

#include "gwcontext.h"
#include "gwinteger.h"
#include "pfgw_globals.h"
#include "pfini.h"

#include "gmp.h"
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

bool EvenlyDivides(uint64 k, uint32 b, uint32 n, int32 c, int32 d)
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

int CreateModulus(Integer *NN, bool kbncdEligible)
{
   mpz_ptr gmp = NN->gmp();

   char  testString[100];
   int   error_code;
   double   k;
   unsigned long b, n;
   long c, d;

   sprintf(testString, "%send1", g_cpTestString);

   if (sscanf(testString, "%lf*%u^%u%dend%d", &k, &b, &n, &c, &error_code) == 5)
   {
      // We can potentially use the faster modular reduction, but we need to
      // ensure that k, b, n, and c were correctly scanned as it is possible
      // that
      sprintf(testString, "%.0lf*%u^%u%d", k, b, n, c);
      if (!strcmp(testString, g_cpTestString) && k < 1e53)
         return CreateModulus(k, b, n, c);
   }

   if (sscanf(testString, "%u^%u%dend%d", &b, &n, &c, &error_code) == 4)
   {
      sprintf(testString, "%u^%u%d", b, n, c);
      if (!strcmp(testString, g_cpTestString))
         return CreateModulus(1.0, b, n, c);
   }

   // If this flag is set, then we will mod by (k*b^n+c)/d after the last
   // square to determine if the number is PRP.
   if (kbncdEligible)
   {
      if (sscanf(testString, "(%lf*%u^%u%d)/%dend%d", &k, &b, &n, &c, &d, &error_code) == 6)
      {
         sprintf(testString, "%(%lf*%u^%u%d)/%d", k, b, n, c, d);
         if (!strcmp(testString, g_cpTestString) && k < 1e53 && EvenlyDivides((uint64) k, b, n, c, d))
            return CreateModulus(k, b, n, c);
      }

      if (sscanf(testString, "(%u^%u%d)/%dend%d", &b, &n, &c, &d, &error_code) == 5)
      {
         sprintf(testString, "(%u^%u%d)/%d", b, n, c, d);
         if (!strcmp(testString, g_cpTestString) && EvenlyDivides(1, b, n, c, d))
            return CreateModulus(1.0, b, n, c);
      }

      // Phi(p,b) = (b^p-1)/(b-1)
      if (sscanf(testString, "Phi(%u,%u)/%dend%d", &n, &b, &d, &error_code) == 4)
      {
         sprintf(testString, "Phi(%u,%u)/%d", n, b, d);
         if (!strcmp(testString, g_cpTestString) && EvenlyDivides(1, b, n, -1, d))
            return CreateModulus(1.0, b, n, -1);
      }

      if (sscanf(testString, "Phi(%u,%u)end%d", &n, &b, &error_code) == 3)
      {
         sprintf(testString, "Phi(%u,%u)", n, b);
         if (!strcmp(testString, g_cpTestString))
            return CreateModulus(1.0, b, n, -1);
      }
   }

   gwset_larger_fftlen_count(&gwdata, g_CompositeAthenticationLevel);

   if (sizeof (mp_limb_t) == sizeof (uint32_t))
      error_code = gwsetup_general_mod (&gwdata, (uint32_t *) gmp->_mp_d, gmp->_mp_size);
   else
      error_code = gwsetup_general_mod_64 (&gwdata, (uint64_t *) gmp->_mp_d, gmp->_mp_size);

   // debugging output
   if (error_code)
      PFPrintf("Error %d initializing FFT code: ", error_code);
   else if (g_bVerbose || g_bWinPFGW_Verbose)
   {
      char   buf[200];
      gwfft_description (&gwdata, buf);
      PFPrintf("Generic modular reduction using %s on %s\n", buf, gwmodulo_as_string(&gwdata));
   }

   if (gwnear_fft_limit (&gwdata, 2.0))
      g_bErrorCheckThisTest = true;      // Getting close to max bits (within 2%)

   // Tell GWNUM to use square carefully for the first few iterations
   // Passing -1 will tell GWNUM to determine how many based upon the
   // size of the modulus
   gwset_square_carefully_count(&gwdata, -1);
   return error_code;
}

int CreateModulus(double k, unsigned long b, unsigned long n, signed long c)
{
   int   error_code;

   gwset_larger_fftlen_count(&gwdata, g_CompositeAthenticationLevel);
   error_code = gwsetup (&gwdata, k, b, n, c);

   // debugging output
   if (error_code)
      PFPrintf("Error %d initializing FFT code: ", error_code);
   else if (g_bVerbose || g_bWinPFGW_Verbose)
   {
      char   buf[200];
      gwfft_description (&gwdata, buf);
      PFPrintf("Special modular reduction using %s on %s\n", buf, gwmodulo_as_string(&gwdata));
   }

   if (gwnear_fft_limit (&gwdata, 2.0))
      g_bErrorCheckThisTest = true;      // Getting close to max bits (within 2%)

   // Tell GWNUM to use square carefully for the first few iterations
   // Passing -1 will tell GWNUM to determine how many based upon the
   // size of the modulus
   gwset_square_carefully_count(&gwdata, -1);
   return error_code;
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
   PFString s, sKey=szKey, sSection;
   g_pIni->GetCurrentSection(sSection);
   g_pIni->SetCurrentSection("Woltman_FFTs");
   int n;
   g_pIni->GetIniInt(&n,&sKey,nDefault,bSetIfNotThere);
   g_pIni->SetCurrentSection(sSection);
   return n;
}

static bool bOverRidden = false;

void getCpuInfo (void)
{
   int   temp;

/* Get the CPU info using CPUID instruction */

   guessCpuType ();
   guessCpuSpeed ();

/* Now let the user override the cpu type and speed from the local.ini file */

   if (g_pIni)
   {
      if (IniGetInt (INI_FILE, "CpuOverride", 0)) {
         temp = IniGetInt (INI_FILE, "CpuSpeed", 99);
         if (temp != 99) CPU_SPEED = temp;
         bOverRidden = true;
      }
      // REMOVE this for a production run
      else
      {
         // write defaults
         IniGetInt (INI_FILE, "CpuSpeed", (int)(CPU_SPEED + 0.5), true);
      }
   }

/* Make sure the cpu speed is reasonable */

   if (CPU_SPEED > 50000) CPU_SPEED = 50000;
   if (CPU_SPEED < 25) CPU_SPEED = 25;

/* Let the user override the cpu flags from the local.ini file */

   if (g_pIni)
   {
      temp = IniGetInt (INI_FILE, "CpuSupportsRDTSC", 99);
      if (temp == 0) {CPU_FLAGS &= ~CPU_RDTSC; bOverRidden = true;}
      if (temp == 1) {CPU_FLAGS |= CPU_RDTSC; bOverRidden = true;}
      temp = IniGetInt (INI_FILE, "CpuSupportsCMOV", 99);
      if (temp == 0) {CPU_FLAGS &= ~CPU_CMOV; bOverRidden = true;}
      if (temp == 1) {CPU_FLAGS |= CPU_CMOV; bOverRidden = true;}
      temp = IniGetInt (INI_FILE, "CpuSupportsPrefetch", 99);
      if (temp == 0) {CPU_FLAGS &= ~CPU_PREFETCH; bOverRidden = true;}
      if (temp == 1) {CPU_FLAGS |= CPU_PREFETCH; bOverRidden = true;}
      temp = IniGetInt (INI_FILE, "CpuSupportsSSE", 99);
      if (temp == 0) {CPU_FLAGS &= ~CPU_SSE; bOverRidden = true;}
      if (temp == 1) {CPU_FLAGS |= CPU_SSE; bOverRidden = true;}
      temp = IniGetInt (INI_FILE, "CpuSupportsSSE2", 99);
      if (temp == 0) {CPU_FLAGS &= ~CPU_SSE2; bOverRidden = true;}
      if (temp == 1) {CPU_FLAGS |= CPU_SSE2; bOverRidden = true;}
      temp = IniGetInt (INI_FILE, "CpuSupportsMMX", 99);
      if (temp == 0) {CPU_FLAGS &= ~CPU_MMX; bOverRidden = true;}
      if (temp == 1) {CPU_FLAGS |= CPU_MMX; bOverRidden = true;}
      temp = IniGetInt (INI_FILE, "CpuSupportsSSE4", 99);
      if (temp == 0) {CPU_FLAGS &= ~CPU_SSE41; bOverRidden = true;}
      if (temp == 1) {CPU_FLAGS |= CPU_SSE41; bOverRidden = true;}
      temp = IniGetInt (INI_FILE, "CpuSupports3DNow", 99);
      if (temp == 0) {CPU_FLAGS &= ~CPU_3DNOW; bOverRidden = true;}
      if (temp == 1) {CPU_FLAGS |= CPU_3DNOW; bOverRidden = true;}

/* Let the user override the L2 cache size in local.ini file */

      temp = IniGetInt (INI_FILE, "CpuL2CacheSize", CPU_L2_CACHE_SIZE);
      if (temp != CPU_L2_CACHE_SIZE) {CPU_L2_CACHE_SIZE = temp; bOverRidden = true;}
      temp = IniGetInt (INI_FILE, "CpuL2CacheLineSize", CPU_L2_CACHE_LINE_SIZE);
      if (temp != CPU_L2_CACHE_LINE_SIZE) {CPU_L2_CACHE_LINE_SIZE = temp; bOverRidden = true;}
      temp = IniGetInt (INI_FILE, "CpuL2SetAssociative", CPU_L2_SET_ASSOCIATIVE);
      if (temp != CPU_L2_SET_ASSOCIATIVE) {CPU_L2_SET_ASSOCIATIVE = temp; bOverRidden = true;}

/* Let the user override the L3 cache size in local.ini file */

      temp = IniGetInt (INI_FILE, "CpuL3CacheSize", CPU_L3_CACHE_SIZE);
      if (temp != CPU_L3_CACHE_SIZE) {CPU_L3_CACHE_SIZE = temp; bOverRidden = true;}
      temp = IniGetInt (INI_FILE, "CpuL3CacheLineSize", CPU_L3_CACHE_LINE_SIZE);
      if (temp != CPU_L3_CACHE_LINE_SIZE) {CPU_L3_CACHE_LINE_SIZE = temp; bOverRidden = true;}
      temp = IniGetInt (INI_FILE, "CpuL3SetAssociative", CPU_L3_SET_ASSOCIATIVE);
      if (temp != CPU_L3_SET_ASSOCIATIVE) {CPU_L3_SET_ASSOCIATIVE = temp; bOverRidden = true;}

/* Let the user override the CPUID brand string.  It should never be necessary. */
/* However, one Athlon owner's brand string became corrupted with illegal characters. */

      IniGetString (INI_FILE, "CpuBrand", CPU_BRAND, sizeof(CPU_BRAND), CPU_BRAND);

/* Let user override the number of hyperthreads */

      temp = IniGetInt (INI_FILE, "CpuNumHyperthreads", CPU_HYPERTHREADS);
      if (temp == 0) temp = 1;
      if (temp != (int) CPU_HYPERTHREADS) {CPU_HYPERTHREADS = temp; bOverRidden = true;}
   }
}

/* Format a long or very long textual cpu description */

void getCpuDescription (
   char   *buf,         /* A 512 byte buffer */
   int   long_desc)      /* True for a very long description */
{

/* Format a pretty CPU description */

   sprintf (buf, "%s\nCPU speed: %.2f MHz", CPU_BRAND, CPU_SPEED);
   if (CPU_CORES > 1 && CPU_HYPERTHREADS > 1)
      sprintf (buf + strlen (buf),
          ", %d hyperthreaded cores", CPU_CORES);
   else if (CPU_CORES > 1)
      sprintf (buf + strlen (buf), ", %d cores", CPU_CORES);
   else if (CPU_HYPERTHREADS > 1)
      sprintf (buf + strlen (buf), ", with hyperthreading");
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
      if (CPU_FLAGS & CPU_SSE41) strcat (buf, "SSE4, ");
      strcpy (buf + strlen (buf) - 2, "\n");
   }
   strcat (buf, "L1 cache size: ");
   if (CPU_L1_CACHE_SIZE < 0) strcat (buf, "unknown\n");
   else sprintf (buf + strlen (buf), "%d KB\n", CPU_L1_CACHE_SIZE);
   strcat (buf, "L2 cache size: ");
   if (CPU_L2_CACHE_SIZE < 0) strcat (buf, "unknown\n");
   else {
      if (CPU_L2_CACHE_SIZE & 0x3FF)
         sprintf (buf + strlen (buf), "%d KB\n", CPU_L2_CACHE_SIZE);
      else
         sprintf (buf + strlen (buf), "%d MB\n", CPU_L2_CACHE_SIZE >> 10);
   }
   if (CPU_L3_CACHE_SIZE > 0) {
      if (CPU_L3_CACHE_SIZE & 0x3FF)
         sprintf (buf + strlen (buf) - 1, ", L3 cache size: %d KB\n", CPU_L3_CACHE_SIZE);
      else
         sprintf (buf + strlen (buf) - 1, ", L3 cache size: %d MB\n", CPU_L3_CACHE_SIZE >> 10);
   }
   if (! long_desc) return;
   strcat (buf, "L1 cache line size: ");
   if (CPU_L1_CACHE_LINE_SIZE < 0) strcat (buf, "unknown\n");
   else sprintf (buf+strlen(buf), "%d bytes\n", CPU_L1_CACHE_LINE_SIZE);
   strcat (buf, "L2 cache line size: ");
   if (CPU_L2_CACHE_LINE_SIZE < 0) strcat (buf, "unknown\n");
   else sprintf (buf+strlen(buf), "%d bytes\n", CPU_L2_CACHE_LINE_SIZE);
   if (CPU_L1_DATA_TLBS > 0)
      sprintf (buf + strlen (buf), "L1 TLBS: %d\n", CPU_L1_DATA_TLBS);
   if (CPU_L2_DATA_TLBS > 0)
      sprintf (buf + strlen (buf), "%sTLBS: %d\n",
          CPU_L1_DATA_TLBS > 0 ? "L2 " : "",
          CPU_L2_DATA_TLBS);
   if (bOverRidden)
      sprintf (buf + strlen (buf), "Overrides from the .ini file in affect!\n");
}
