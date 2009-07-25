#ifndef GWCONTEXT_H
#define GWCONTEXT_H

#ifdef _MSC_VER
#include "gwnum.h"
#include "cpuid.h"
#else
#include "../../packages/gwnum/gwnum.h"
#include "../../packages/gwnum/cpuid.h"
#endif

void getCpuInfo (void);
void getCpuDescription (
			char	*buf,			/* A 512 byte buffer */
			int	long_desc);		/* True for a very long description */

extern gwhandle gwdata;				   // Global variable to store gwnum state
extern double g_dMaxError;				// Overall roundoff error during the run
extern const double g_dMaxErrorAllowed;
extern bool g_bErrorCheckAllTests;
extern bool g_bErrorCheckThisTest;
extern int g_bCollectStats;			// Stats collection - no longer supported
extern double sumX;
extern double sumXX;
extern double sumN;

int CreateModulus(Integer *NN, bool kbncdEligible = false);
int CreateModulus(double k, unsigned long b, unsigned long n, signed long c);
void DestroyModulus();

#endif
