#ifndef GWCONTEXT_H
#define GWCONTEXT_H

#include "gwnum.h"
#include "cpuid.h"
void getCpuInfo (void);
void getCpuDescription (
			char	*buf,			/* A 512 byte buffer */
			int	long_desc);		/* True for a very long description */

extern gwhandle gwdata;				// Global variable to store gwnum state
extern int ERRCHK;
extern double MAXERR;				// Overall roundoff error during the run
extern int g_bCollectStats;			// Stats collection - no longer supported
extern double sumX;
extern double sumXX;
extern double sumN;

int CreateModulus(Integer *NN);
int CreateModulus(double k, unsigned long b, unsigned long n, signed long c);
void DestroyModulus();

#endif
