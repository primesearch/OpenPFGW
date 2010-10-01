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

#ifndef _MSC_VER
Integer *ex_evaluate(PFSymbolTable *pContext,const PFString &e,int m);
#endif

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
            PFPrintfLog("%s - Evaluator failed\n",LPCTSTR(sNumber));
            PFfflush(stdout);
         }
         else
         {
            PFIntegerSymbol *pN=new PFIntegerSymbol("_N",pResult);
            psymRuntime->AddSymbol(pN);

            PFFunctionSymbol::CallSubroutine("@vector",psymRuntime);
            PFPrintfLog("\n");
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
   PFPrintfLog(fmt,buf);
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
      bench_gwPRP(pResult,iterations);

#if defined (NEW_TIMER)
      Timer2.Stop();
#else
      t1=clock();
#endif

      bench_gwPRP(pResult,2*iterations);
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
   PFPrintfLog("Benchmarking PrimeForm/GW!  This may take several minutes.\n");

   uint32 i,k;
   double sum, total;
   double len;

   // First benchmark: Trial factoring
   double dfp1,dfp2;
   DWORD dwFactorRepeat=40;
   long FactorTestValues[]={500, 800, 1000, 1600, 1800, 2500, 3100, 0};


   sum=total=0;

   PFPrintfLog("Timing Trial Factoring   [time/factorbit]\n");
   PFPrintfLog("-----------------------------------------\n");
   PFPrintfLog("  k      p(k)#+1             p(k)#/2\n");
   PFPrintfLog("         10000 primes tried  2 primes tried\n");
   PFfflush(stdout);

   for (i=0;FactorTestValues[i];i++)
   {
      k=FactorTestValues[i];

      pContext->AddSymbol(new PFIntegerSymbol("k",new Integer(k)));

      dfp1=benchFactor("p(k)#+1",104729,dwFactorRepeat,pContext,&len);
      sum+=dfp1;
      total+=len*10000;
      dfp1/=len*10000;     // first 10000 factors tried  p(10000) == 104729

      // Kick in an extra 50x of the 2-factor test, since they are so fast.  Otherwise
      // on non "NEW_TIMER" mode, clock_t does not have enough resolution to handle this
      dfp2=benchFactor("p(k)#/2",2,dwFactorRepeat*50,pContext,&len);
      dfp2 /= 50;    // Note, we did 50x the repeat count, so factor back down to 1 iteration.
      sum+=dfp2;
      total+=len*2;
      dfp2/=len*2;      // Two factors tried (2,3)

      PFPrintfLog("%5ld   ",k);
      dispTiming(dfp1,8);
      PFPrintfLog("            "); dispTiming(dfp2,8);
      PFPrintfLog("\n");
      PFfflush(stdout);
   }

   if (total) sum/=total; else sum=0;
   PFPrintfLog("-----------------------------------\n");
   PFPrintfLog("Estimate for Trial Factoring\n");
   PFPrintfLog("-----------------------------------\n");
   for (k=5;k<=23;k++)
   {
      PFPrintfLog("%8ld: ",1l<<k);
      dispTiming(sum*(1l<<k)*(1l<<k),7);
      PFPrintfLog("\n");
   }

   // Second benchmark: PRP testing
   double dpp1,dpp2;
   DWORD dwPRPRepeat=20;
   DWORD dwPRPIterations=500;

   sum=total=0;
   PFPrintfLog("Timing PRP test    [time/iterationbit]\n");
   PFPrintfLog("--------------------------------------\n");
   PFPrintfLog(" Test      2^k-1       2^k+1\n");

   for (k=5000;k<=50000;k+=5000)
   {
      PFPrintfLog("%5ld   ",k);

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
      PFPrintfLog("   "); dispTiming(dpp2,8);
      PFPrintfLog("\n");
      PFfflush(stdout);
   }

   if (total) sum/=total; else sum=0;
   PFPrintfLog("-----------------------------------\n");
   PFPrintfLog("Estimate for PRP test\n");
   PFPrintfLog("-----------------------------------\n");
   for (k=5;k<=23;k++)
   {
      double prptime=sum*(1l<<k)*(1l<<k)*k;
      PFPrintfLog("%8ld: ",1l<<k);
      dispTiming(prptime,7);
      PFPrintfLog("\n");
   }
}
