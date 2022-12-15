#ifndef GWCONTEXT_H
#define GWCONTEXT_H

#ifdef _MSC_VER
#include "gwnum.h"
#include "cpuid.h"
#else
#include "../../packages/gwnum/gwnum.h"
#include "../../packages/gwnum/cpuid.h"
#endif

#include "../pfmath/integer.h"

void getCpuInfo (void);
void getCpuDescription (
			char	*buf,			/* A 512 byte buffer */
			int	bufferSize);		/* True for a very long description */

extern gwhandle gwdata;				   // Global variable to store gwnum state
extern double g_dMaxError;				// Overall roundoff error during the run
extern const double g_dMaxErrorAllowed;
extern bool g_bErrorCheckAllTests;
extern bool g_bErrorCheckThisTest;
extern int g_bCollectStats;			// Stats collection - no longer supported
extern double sumX;
extern double sumXX;
extern double sumN;

int CreateModulus(Integer *NN, char *expression, bool kbncdEligible = false, int increaseFFTSize = 0);
int CreateSpecialModulus(mpz_ptr gmp, double k, unsigned long b, unsigned long n, signed long c);
int CreateGenericModulus(mpz_ptr gmp);
void DestroyModulus();

#endif
