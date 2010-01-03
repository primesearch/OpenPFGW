#include "primeformpch.h"
#include <signal.h>
#include "../../pform/pfio/pfini.h"
#include "pfgw_globals.h"
#include "../../pform/pflib/timer.h"

#define NEW_TIMER

#ifndef GWDEBUG
#define GWDEBUG(X) {Integer XX;XX=X;printf(#X "=");mpz_out_str(stdout,16,XX.gmp();printf(" ");}
#define INTDEBUG(X) {printf(#X "=");mpz_out_str(stdout,16,(X).gmp();printf("\n");}
#endif

Integer *ex_evaluate(PFSymbolTable *pContext,const PFString &e,int m);

void vectorout(PFString &sFilename,PFSymbolTable *psymRuntime)
{
	const char *cpError;
	PFSimpleFile *vector_pFile = openInputFile(sFilename, NULL, &cpError);
	if (!vector_pFile)
	{
		PFPrintfStderr("%s\n", cpError);
		return;
	}
	
	PFString sNumber;
	PFString sMessage;
	while(vector_pFile->GetNextLine(sNumber) == PFSimpleFile::e_ok)
	{

		if(!sNumber.IsEmpty())
		{
			Integer *pResult=NULL;
			pResult=ex_evaluate(psymRuntime,LPCTSTR(sNumber));
			if(pResult==NULL)
			{
				PFPrintf("%s - Evaluator failed\n",LPCTSTR(sNumber));
				PFfflush(stdout);
			}
			else
			{
				PFIntegerSymbol *pN=new PFIntegerSymbol("_N",pResult);
				psymRuntime->AddSymbol(pN);

				PFFunctionSymbol::CallSubroutine("@vector",psymRuntime);			
				PFPrintf("\n");
				PFfflush(stdout);
			}
		}
	}
	delete vector_pFile;
}

void dispTiming(double f, int x)
{
	char buf[30];
	char fmt[5];

	if (f<1e-12) sprintf(buf,"---");
	else if (f<1e-7) sprintf(buf,"%ldps",(long)(1e12*f));
	else if (f<1e-4) sprintf(buf,"%ldns",(long)(1e9*f));
	else if (f<1e-1) sprintf(buf,"%ldus",(long)(1e6*f));
	else if (f<100)  sprintf(buf,"%ldms",(long)(1e3*f));
	else if (f<600) sprintf(buf,"%lds",(long)(f));
	else if (f<7200) sprintf(buf,"%ldmi",(long)(f/60));
	else if (f<172800) sprintf(buf,"%ldhr",(long)(f/3600));
	else sprintf(buf,"%ldd",(long)(f/86400));
	sprintf(fmt,"%%%ds",x);
	PFPrintf(fmt,buf);
}

double benchFactor(const char *expr, uint32 fmax,int rep,PFSymbolTable *pContext, double *len)
{
	PFSymbolTable *pTestContext=new PFSymbolTable(pContext);

	Integer *pResult=ex_evaluate(pTestContext,expr);
	PFIntegerSymbol *pN=new PFIntegerSymbol("_N",pResult);
	pTestContext->AddSymbol(pN);

	*len=lg(*pResult);

	PFFactorizationSymbol *pffN;
	pTestContext->AddSymbol(pffN=new PFFactorizationSymbol("_NFACTOR"));
	pTestContext->AddSymbol(new PFIntegerSymbol("_DEEPFACTOR",new Integer(1)));
	pTestContext->AddSymbol(new PFStringSymbol("_SN","_NFACTOR"));

	if (fmax) pTestContext->AddSymbol(new PFIntegerSymbol("_PMAX",new Integer(fmax)));

	uint32 oldICount = g_nIterationCnt;
	g_nIterationCnt=0;

#if defined (NEW_TIMER)
	CTimer Timer;
	Timer.Start();
#else
	clock_t starttime=clock();
#endif
	for (DWORD dwF=0;dwF<DWORD(rep);dwF++)
	{
		PFFunctionSymbol::CallSubroutine("@factor",pTestContext);
	}
	g_nIterationCnt=oldICount;

	
	delete pTestContext;

#if defined (NEW_TIMER)
	return Timer.GetSecs()/rep;
#else
	return (double(clock()-starttime)/clocks_per_sec/rep);
#endif
}

double benchPRP(const char *expr,int rep, int iterations, PFSymbolTable *pContext, double *len)
{
	PFSymbolTable *pTestContext=new PFSymbolTable(pContext);

	Integer *pResult=ex_evaluate(pTestContext,expr);

#if defined (NEW_TIMER)
	CTimer Timer1, Timer2;
	Timer1.Start();
#else
	clock_t starttime,t1;
#endif
	double totaltime=0;
	*len=lg(*pResult);

	for (DWORD dwP=0;dwP<DWORD(rep);dwP++)
	{
#if defined (NEW_TIMER)
		Timer1.Start();
		Timer2.Start();
#else
		starttime=clock();
#endif
		gwPRP(pResult,"",NULL,iterations);

#if defined (NEW_TIMER)
		Timer2.Stop();
#else
		t1=clock();
#endif

		gwPRP(pResult,"",NULL,2*iterations);
#if defined (NEW_TIMER)
		totaltime+=Timer1.GetSecs()-2*Timer2.GetSecs();
#else
		totaltime+=(double)((clock()+starttime)-2*t1);
#endif
	}

	delete pTestContext;
#if defined (NEW_TIMER)
	return (totaltime/iterations/rep);
#else
	return (totaltime/clocks_per_sec/iterations/rep);
#endif
}


void bench(PFSymbolTable *pContext)
{
	PFPrintf("Benchmarking PrimeForm/GW!  This may take several minutes.\n");

	uint32 i,k;
	double sum, total;
	double len;

	// First benchmark: Trial factoring
	double dfp1,dfp2;
	DWORD dwFactorRepeat=40;
	long FactorTestValues[]={500, 800, 1000, 1600, 1800, 2500, 3100, 0};


	sum=total=0;

	PFPrintf("Timing Trial Factoring   [time/factorbit]\n");
	PFPrintf("-----------------------------------------\n");
	PFPrintf("  k      p(k)#+1             p(k)#/2\n");
	PFPrintf("         10000 primes tried  2 primes tried\n");
	PFfflush(stdout);

	for (i=0;FactorTestValues[i];i++)
	{
		k=FactorTestValues[i];

		pContext->AddSymbol(new PFIntegerSymbol("k",new Integer(k)));

		dfp1=benchFactor("p(k)#+1",104729,dwFactorRepeat,pContext,&len);
		sum+=dfp1;
		total+=len*10000;
		dfp1/=len*10000;		// first 10000 factors tried  p(10000) == 104729

		// Kick in an extra 50x of the 2-factor test, since they are so fast.  Otherwise
		// on non "NEW_TIMER" mode, clock_t does not have enough resolution to handle this
		dfp2=benchFactor("p(k)#/2",2,dwFactorRepeat*50,pContext,&len);
		dfp2 /= 50;		// Note, we did 50x the repeat count, so factor back down to 1 iteration.
		sum+=dfp2;
		total+=len*2;
		dfp2/=len*2;		// Two factors tried (2,3)

		PFPrintf("%5ld   ",k);
		dispTiming(dfp1,8);
		PFPrintf("            "); dispTiming(dfp2,8);
		PFPrintf("\n");
		PFfflush(stdout);
	}

	if (total) sum/=total; else sum=0;
	PFPrintf("-----------------------------------\n");
	PFPrintf("Estimate for Trial Factoring\n");
	PFPrintf("-----------------------------------\n");
	for (k=5;k<=23;k++)
	{
		PFPrintf("%8ld: ",1l<<k);
		dispTiming(sum*(1l<<k)*(1l<<k),7);
		PFPrintf("\n");
	}

	// Second benchmark: PRP testing
	double dpp1,dpp2;
	DWORD dwPRPRepeat=20;
	DWORD dwPRPIterations=500;

	sum=total=0;
	PFPrintf("Timing PRP test    [time/iterationbit]\n");
	PFPrintf("--------------------------------------\n");
	PFPrintf(" Test      2^k-1       2^k+1\n");

	for (k=5000;k<=50000;k+=5000)
	{
		PFPrintf("%5ld   ",k);

		pContext->AddSymbol(new PFIntegerSymbol("k",new Integer(k)));

		dpp1=benchPRP("2^k-1",dwPRPRepeat,dwPRPIterations,pContext,&len);
		sum+=dpp1;
		total+=len*log(len)/log(2.0);
		dpp1/=len*log(len)/log(2.0); 

		dpp2=benchPRP("2^k+1",dwPRPRepeat,dwPRPIterations,pContext,&len);
		sum+=dpp2;
		total+=len*log(len)/log(2.0);
		dpp2/=len*log(len)/log(2.0);

		dispTiming(dpp1,8);
		PFPrintf("   "); dispTiming(dpp2,8);
		PFPrintf("\n");
		PFfflush(stdout);
	}

	if (total) sum/=total; else sum=0;
	PFPrintf("-----------------------------------\n");
	PFPrintf("Estimate for PRP test\n");
	PFPrintf("-----------------------------------\n");
	for (k=5;k<=23;k++)
	{
		double prptime=sum*(1l<<k)*(1l<<k)*k;
		PFPrintf("%8ld: ",1l<<k);
		dispTiming(prptime,7);
		PFPrintf("\n");
	}
}


#ifndef FIXED_SIGNAL_SIZE

void optimize(PFSymbolTable *pContext)
{
	PFPrintf("Benchmarking PrimeForm/GW! This may take several minutes.\n");

	// enable error checking within this routine
	g_bErrorCheckAllTests = true;
	g_bCollectStats = true;
	
	// first find out how many there are
#ifdef NOT_WORKING
	int x;

   FFTEntryNew *_fftMatrix = fftMatrixNew;
	if (CPU_FLAGS & CPU_SSE2)
	{
		_fftMatrix = fftMatrixNewSSE2;
		PFPrintf ("Running with the SSE2 context sizes (Pentium 4 detected)\n");
	}

	for(x=0;_fftMatrix[x].dwElemCnt;x++) {}
	int xcount=x;
	PFPrintf("%d different FFT sizes installed\n\n",xcount);

	char Buffer[512];
	getCpuDescription(Buffer, 1);
	PFPrintf("CPU Information (From Woltman v25 library code)\n%s\n\n", Buffer);

	xcount -= 10;  // Skip testing the hugest FFT's (for now).  Simply comment out this line for a FULL test, but be prepared to WAIT

	for(x=0;x<xcount;x++)
	{
		DWORD C=_fftMatrix[x].dwElemCnt;
		DWORD BB=_fftMatrix[x].dwStartWordsize;

		// plus or minus one
		for(DWORD B=BB-1;B<=BB+1;B++)
		{
			DWORD E=C*B;
		
			int iPow5=int(floor((log(2.0)*E)/log(5.0)/2.0));

			pContext->AddSymbol(new PFIntegerSymbol("k",new Integer(iPow5)));
			CTimer Timer;
			Timer.Start();
			Integer *pResult=ex_evaluate(pContext,"5^k-1");
			double deval = Timer.GetSecs ();

			// try to create a context of the size we asked for.
			int idiv=0;
	
			DWORD NB,XB;
			do
			{
				(*pResult)>>=1;			// always divide
				idiv++;
				NB=lg(*pResult)+1;			// NB is the number of bits in N
				XB=2*(NB)+8;	// XB is the number of bits in an intermediate result
				// create a context
			}
			while(!CreateContext(NB,XB,C,B));
			PFPrintf("Testing (5^%d-1)/2^%d for FFT length %lu*%lu\n",iPow5,idiv,C,B);
			{
				GWInteger gwN;
				Integer NN;
				double dftogw,dgwtof;
			
				Timer.Start();
				flattogw(*pResult,gwN,NULL,NULL);
				dftogw = Timer.GetSecs();

				Timer.Start();
				gwtoflat(gwN,NN);
				dgwtof = Timer.GetSecs();
	
				if(NN==*pResult)
				{
					PFPrintf("Test OK.\nTimings:\n");
					PFPrintf("eval:%0.8f\nflattogw:%0.8f\ngwtoflat:%0.8f\n",deval,dftogw,dgwtof);
				}
				else
				{
					PFPrintf("Conversion test failed!!!!\n");
					PFPrintf("RR=");

					mpz_out_str(stdout,16,pResult->gmp());
					PFPrintf("\nNN=");
					mpz_out_str(stdout,16,NN.gmp());
					PFPrintf("\nDD=");
					NN-=*pResult;
					mpz_out_str(stdout,16,NN.gmp());
					PFPrintf("\n");
								
					g_bCollectStats = false;
					return;
				}
			}
		
			// the other tests continue in a scope brace, since DestroyContext
			// will deallocate GWIntegers
			{
				// create a reciprocal
				Timer.Start();
				IGWReciprocal *pReciprocal=IGWReciprocal::create(pResult,IGWContext::FFT_ElementSize());
				double drec = Timer.GetSecs();;
				
				PFPrintf("Reciprocal: %0.8f\n",drec);
				
				// run the tests. Note this is 100 iterations to get the
				// residue filled with something essentially random, then
				// 100 residues we actually collect stats on

				double d1;	
				DWORD dwIterations=100;
				
				GWInteger gwX;
				gwX=255;								// initialise X to A^1.
				gwsetmulbyconst(&gwdata,255);		// and multiplier
				
				DWORD dw=dwIterations;
				for(;dw;dw--)
				{
					gwsetnormroutine(&gwdata,0,ERRCHK,1);
					gwsquare(gwX);
				}
				// clear statistical variables
				sumN=0.0;
				sumX=0.0;
				sumXX=0.0;

				// Use the Woltman high res timer form CPU.c.  This should allow us
				// to use the Pentium timer on all x86's above 486 (and PFGW won't run on 486 anyway).
				Timer.Start();

				dw=dwIterations;
				for(;dw;dw--)
				{
					gwsetnormroutine(&gwdata,0,ERRCHK,1);
					gwsquare(gwX);
				}
				d1 = Timer.GetSecs();
				PFPrintf("Iteration: %0.8fs\n",d1/dwIterations);

				double ex=sumX/sumN;
				double exx=sumXX/sumN;
				double s2=exx-ex*ex;
				double s=sqrt(s2);
				PFPrintf("Roundoff: mean:%f s.d.:%f confidence:%f\n\n", ex,s,(0.5-ex)/s);
			
			}
		
			//delete pResult;
			DestroyModulus();		
		}
	}
#endif
	g_bCollectStats = false;
}
#endif
