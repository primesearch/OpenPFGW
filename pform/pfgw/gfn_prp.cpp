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

bool IsValidGFN(const char *sNum, uint32 *GFN_Base, uint32 *GFN_Exp)
{
	int pl;
	if (sscanf(sNum, "%u^%u+%d", GFN_Base, GFN_Exp, &pl) == 3 && pl == 1)
	{
		char sTmp[40];
		sprintf (sTmp, "%u^%u+1", *GFN_Base, *GFN_Exp);
		if (strcmp(sTmp, sNum))		// avoid  b^n+1+2 or something similiar bunk which would "pass" the sscanf
			return false;
		if (*GFN_Base & 1)
			// Note this number can NEVER be a prime. base MUST be even.  We could easily handle these by a print("composite");
			// it is possible to get here, if someone is factoring, and found no factors, or if someone is ignorant to the
			// format of GFN's and the knowledge that NO odd based gfns can exist.
			return false;

		for (unsigned tmpExp = *GFN_Exp; tmpExp<2; tmpExp>>=1 )
		{
			if (tmpExp & 1)
				return false;		// Note this number can NEVER be a prime.
		}
		switch(*GFN_Exp)
		{
			// below this is TOO damn small ;)

			// log_2(12123055) is < 753/32 but log_2(12123056) is > 753/2  (see table in FGWContext.cpp)
			case (1<<5):	return *GFN_Base <= 12123055;
			// log_2(16777216) is < 1489/64 but log_2(10084422) is > 753/2  (see table in FGWContext.cpp)
			case (1<<6):	return *GFN_Base <= 10084421; // 64
			case (1<<7):	return *GFN_Base <= 7989575;  // 128
			case (1<<8):	return *GFN_Base <= 6556667;  // 256
			case (1<<9):	return *GFN_Base <= 5535916;  // 512
			case (1<<10):	return *GFN_Base <= 4400804;  // 1024
			case (1<<11):	return *GFN_Base <= 3808627;  // 2048
			case (1<<12):	return *GFN_Base <= 3192920;  // 4096
			case (1<<13):	return *GFN_Base <= 2606128;  // 8192
			case (1<<14):	return *GFN_Base <= 2219099;  // 16384
			case (1<<15):	return *GFN_Base <= 1765876;  // 32768
			case (1<<16):	return *GFN_Base <= 1506815;  // 65536
			case (1<<17):	return *GFN_Base <= 1255523;  // 131072
			case (1<<18):	return *GFN_Base <= 1068503;  // 262144
			case (1<<19):	return *GFN_Base <= 842220;   // 524288
			case (1<<20):	return *GFN_Base <= 718663;   // 1048576
			case (1<<21):	return *GFN_Base <= 599207;   // 2097152
			case (1<<22):	return *GFN_Base <= 491418;   // 4194304

			// above this is TOO damn big ;)   simply return false.
		}
	}
	return false;
}

// code stolen from gwPRP
bool gwPRP_GFN(Integer *N,const char *sNumStr, uint32 base, uint32 _exp, uint64 *p_n64ValidationResidue)
{
   int fftSize = g_CompositeAthenticationLevel - 1;
   int testResult;

   do
   {
      fftSize++;

      gwinit2(&gwdata, sizeof(gwhandle), GWNUM_VERSION);
      gwsetmaxmulbyconst(&gwdata, iBase);  // maximum multiplier

      if (CreateModulus(1.0, base, _exp, 1, fftSize)) return false;

      testResult = prp_using_gwnum(N, iBase, sNumStr, p_n64ValidationResidue, fftSize);

      DestroyModulus();
   } while (testResult == -1 && fftSize < 5);

   if (testResult == 1)
      return true;

   return false;
}
