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
Integer *ex_evaluate(PFSymbolTable *pContext,const PFString &e);
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
   else if (f<1e-8) sprintf(buf, "%ld ps",(long)(1e12*f));
   else if (f<1e-5) sprintf(buf, "%ld ns",(long)(1e9*f));
   else if (f<1e-2) sprintf(buf, "%ld us",(long)(1e6*f));
   else if (f<10)   sprintf(buf, "%ld ms",(long)(1e3*f));
   else if (f<600)  sprintf(buf, "%ld s ",(long)(f));
   else if (f<7200) sprintf(buf, "%ld mi",(long)(f/60));
   else if (f<172800) sprintf(buf, "%ld hr",(long)(f/3600));
   else sprintf(buf,"%ld dy",(long)(f/86400));
   sprintf(fmt, "%%%ds", x);
   PFPrintfLog(fmt, buf);
}

double benchFactor(const char *expr, uint32_t fmax, int rep, PFSymbolTable *pContext, double *len)
{
   PFSymbolTable *pTestContext=new PFSymbolTable(pContext);

   Integer *pResult=ex_evaluate(pTestContext,expr);
   PFIntegerSymbol *pN=new PFIntegerSymbol("_N",pResult);
   pTestContext->AddSymbol(pN);

   *len=numbits(*pResult);

   PFFactorizationSymbol *pffN;
   pTestContext->AddSymbol(pffN=new PFFactorizationSymbol("_NFACTOR"));
   pTestContext->AddSymbol(new PFIntegerSymbol("_DEEPFACTOR",new Integer(1)));
   pTestContext->AddSymbol(new PFStringSymbol("_SN","_NFACTOR"));

   if (fmax) pTestContext->AddSymbol(new PFIntegerSymbol("_PMAX",new Integer(fmax)));

   uint32_t oldICount = g_nIterationCnt;
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

bool getFFTInfo(const char *expr, PFSymbolTable *pContext, char *fftInfo)
{
   PFSymbolTable *pTestContext=new PFSymbolTable(pContext);

   Integer *pResult=ex_evaluate(pTestContext,expr);

   gwinit2(&gwdata, sizeof(gwhandle), (char *) GWNUM_VERSION);

   if (CreateModulus(pResult, g_cpTestString, true)) return false;

   gwfft_description(&gwdata, fftInfo);

   DestroyModulus();

   return true;
}

double benchPRP(const char *expr, uint32_t rep, uint32_t iterations, PFSymbolTable *pContext, double *len)
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
   *len=numbits(*pResult);

   for (uint32_t dwP=0; dwP<rep; dwP++)
   {
      if (g_bExitNow) break;

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
   return (totaltime/(iterations*rep));
#else
   return (totaltime/(clocks_per_sec*iterations*rep));
#endif
}

void benchmarkFactor(PFSymbolTable *pContext)
{
   uint32_t i,k;
   double sum, total;
   double len;

   // First benchmark: Trial factoring
   double dfp1,dfp2;
   DWORD dwFactorRepeat=40;
   long FactorTestValues[]={500, 800, 1000, 1600, 1800, 2500, 3100, 0};

   sum=total=0;
   
   PFPrintfLog("\nBenchmarking trial factoring!\n");

   PFPrintfLog("Timing Trial Factoring   [time/factorbit]\n");
   PFPrintfLog("-----------------------------------------\n");
   PFPrintfLog("  k      p(k)#+1             p(k)#/2\n");
   PFPrintfLog("         10000 primes tried  2 primes tried\n");
   PFfflush(stdout);

   for (i=0;FactorTestValues[i];i++)
   {
      if (g_bExitNow) break;

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
}

void benchmarkGeneric(PFSymbolTable *pContext, char *expression, int32_t minF, int32_t maxF, bool allFFT)
{
   char   last_fftlen[200], next_fftlen[200];
   double tpb, tpe, bits;
   int32_t  f;
   int32_t  dwPRPRepeat;
   int32_t  dwPRPIterations;
   
   PFPrintfLog("\nBenchmarking generic modular reduction for %s!\n", expression);

   PFPrintfLog("[appx max f]   [num bits]   [iter time]  [PRP time]  [FFT used]\n");
   PFPrintfLog("--------------------------------------------------------------------------------\n");

   dwPRPRepeat = 10;
   dwPRPIterations = 50;
   f = minF;

   if (f > 10000000) f = f - (f % 10000000);
   else if (f > 1000000) f = f - (f % 1000000);
   else if (f > 100000) f = f - (f % 100000);
   else if (f > 10000) f = f - (f % 10000);
   else if (f > 1000) f = f - (f % 1000);
   else if (f > 100) f = f - (f % 100);

   pContext->AddSymbol(new PFIntegerSymbol("f", new Integer(f)));

   getFFTInfo(expression, pContext, last_fftlen);

   while (f <= maxF)
   {
      if (g_bExitNow) break;

      pContext->AddSymbol(new PFIntegerSymbol("f", new Integer(f)));

      if (!getFFTInfo(expression, pContext, next_fftlen)) break;

      if (!allFFT || strcmp(last_fftlen, next_fftlen) || f == maxF)
      {
         PFPrintfLog("%10ld   ", f);

         pContext->AddSymbol(new PFIntegerSymbol("f", new Integer(f)));

         tpb = benchPRP(expression, dwPRPRepeat, dwPRPIterations, pContext, &bits);

         tpe = tpb * bits;

         PFPrintfLog("%10ld   ", (int64_t) bits);

         dispTiming(tpb, 11);
         dispTiming(tpe, 12);

         PFPrintfLog("     %s\n", last_fftlen);
         PFfflush(stdout);

         strcpy(last_fftlen, next_fftlen);
      }
      
      if (f == maxF) break;

      if (allFFT)
      {
         if (f < 1000) f += 10;
         else if (f < 10000) f += 100;
         else if (f < 1000000) f += 1000;
         else if (f < 10000000) f += 10000;
         else f += 100000;
      }
      else
      {
         if (f < 1000) f += 300;
         else if (f < 10000) f += 3000;
         else if (f < 100000) f += 30000;
         else if (f < 1000000) f += 300000;
         else if (f < 10000000) f += 3000000;
         else if (f < 100000000) f += 30000000;
         else f += 300000000;
      }

      if (f > 1000) { dwPRPRepeat = 20; dwPRPIterations = 200; }
      if (f > 1000000) { dwPRPRepeat = 10; dwPRPIterations = 100; }

      if (f > maxF) f = maxF;
   }
   
   PFPrintfLog("\n");
   PFfflush(stdout);
}

void benchmarkSpecial(PFSymbolTable *pContext, char *expression, int32_t minN, int32_t maxN, double k, uint32_t b, int32_t c, bool allFFT)
{
   char   last_fftlen[200], next_fftlen[200];
   double tpb, tpe, bits;
   int32_t  n;
   int32_t  dwPRPRepeat;
   int32_t  dwPRPIterations;

   PFPrintfLog("\nBenchmarking special modular reduction for %s!\n", expression);

   PFPrintfLog("[appx max n]   [num bits]   [iter time]  [PRP time]  [FFT used]\n");
   PFPrintfLog("--------------------------------------------------------------------------------\n");

   dwPRPRepeat = 10;
   dwPRPIterations = 50;
   n = minN;

   if (n > 10000000) n = n - (n % 10000000);
   else if (n > 1000000) n = n - (n % 1000000);
   else if (n > 100000) n = n - (n % 100000);
   else if (n > 10000) n = n - (n % 10000);
   else if (n > 1000) n = n - (n % 1000);
   else if (n > 100) n = n - (n % 100);

   gwmap_to_fftlen(k, b, 90, c);

   gwfft_description (&gwdata, last_fftlen);

   while (n <= maxN)
   {
      if (g_bExitNow) break;

      if (gwmap_to_fftlen(k, b, n, c) == 0) break;

      gwfft_description (&gwdata, next_fftlen);

      if (!allFFT || strcmp(last_fftlen, next_fftlen) || n == maxN)
      {
         PFPrintfLog("%10ld   ", n);

         pContext->AddSymbol(new PFIntegerSymbol("n", new Integer(n)));

         tpb = benchPRP(expression, dwPRPRepeat, dwPRPIterations, pContext, &bits);

         tpe = tpb * bits;
         
         PFPrintfLog("%10ld   ", (int64_t) bits);

         dispTiming(tpb, 11);
         dispTiming(tpe, 12);

         PFPrintfLog("     %s\n", last_fftlen);
         PFfflush(stdout);

         strcpy(last_fftlen, next_fftlen);
      }

      if (n == maxN) break;

      if (allFFT)
      {
         if (n < 1000) n += 10;
         else if (n < 10000) n += 100;
         else if (n < 1000000) n += 1000;
         else if (n < 10000000) n += 10000;
         else n += 100000;
      }
      else
      {
         if (n < 1000) n += 300;
         else if (n < 10000) n += 3000;
         else if (n < 100000) n += 30000;
         else if (n < 1000000) n += 300000;
         else if (n < 10000000) n += 3000000;
         else if (n < 100000000) n += 30000000;
         else n += 300000000;
      }

      if (n > 1000) { dwPRPRepeat = 20; dwPRPIterations = 200; }
      if (n > 1000000) { dwPRPRepeat = 10; dwPRPIterations = 100; }

      if (n > maxN) n = maxN;
   }
   
   PFPrintfLog("\n");
   PFfflush(stdout);
}

void benchmark(PFSymbolTable *pContext, char *parameter)
{
   char genericExpression[200], specialExpression[200];
   char *cPtr;
   int error_code;
   double  k;
   uint32_t b;
   int32_t c, d;
   int32_t minN = 100, maxN = 10000000;
   int32_t minF = 100, maxF = 1000000;
   bool allFFT = false, haveGenericExp = false, haveSpecialExp = false;
   bool doGeneric = false, doSpecial  = false, doFactor = false;
 
   cPtr = strtok(parameter, ",");
   if (!cPtr) { doSpecial = doGeneric = true; }

   while (cPtr)
   {
      if (!strcmp(cPtr, "fft")) allFFT = true;
      else if (!strcmp(cPtr, "fact")) doFactor = true;
      else if (!strcmp(cPtr, "spec")) doSpecial = true;
      else if (!strcmp(cPtr, "gen")) doGeneric = true;
      else if (!memcmp(cPtr, "minf=", 5)) { doGeneric = true; minF = atol(cPtr+5); }
      else if (!memcmp(cPtr, "maxf=", 5)) { doGeneric = true; maxF = atol(cPtr+5); }
      else if (!memcmp(cPtr, "minn=", 5)) { doSpecial = true; minN = atol(cPtr+5); }
      else if (!memcmp(cPtr, "maxn=", 5)) { doSpecial = true; maxN = atol(cPtr+5); }
      else if (!memcmp(cPtr, "gexp=", 5))
      {
         strcpy(genericExpression, cPtr+5);
         doGeneric = haveGenericExp = true;
      }
      else if (!memcmp(cPtr, "sexp=", 5))
      {
         sprintf(specialExpression, "%send1", cPtr+5);
         doSpecial = false;

         if (sscanf(specialExpression, "(%lf*%u^n%d)/%dend%d", &k, &b, &c, &d, &error_code) == 5)       { doSpecial = haveSpecialExp = true; }
         if (!doSpecial && sscanf(specialExpression, "%u^n%dend%d", &b, &c, &error_code) == 3)          { doSpecial = haveSpecialExp = true; k = 1; d = 1; }
         if (!doSpecial && sscanf(specialExpression, "%lf*%u^n%dend%d", &k, &b, &c, &error_code) == 4)  { doSpecial = haveSpecialExp = true; d = 1; }
         if (!doSpecial && sscanf(specialExpression, "(%u^n%d)/%dend%d", &b, &c, &d, &error_code) == 4) { doSpecial = haveSpecialExp = true; k = 1; }
         if (!doSpecial && sscanf(specialExpression, "Phi(n,%u)/%dend%d", &b, &d, &error_code) == 3)    { doSpecial = haveSpecialExp = true; k = 1; c = -1; }
         if (!doSpecial && sscanf(specialExpression, "Phi(n,%u)end%d",  &b, &error_code) == 2)          { doSpecial = haveSpecialExp = true; k = 1; c = -1; d = 1; }

         if (!haveSpecialExp)
         {
            PFPrintf("Invalid expression ignored: %s\n", cPtr+5);
            return;
         }

         strcpy(specialExpression, cPtr+5);
      }
      else
      {
         PFPrintf("Invalid parameter ignored: %s\n", cPtr);
         return;
      }

      cPtr = strtok(NULL, ",");
   }

   if (allFFT && !doGeneric && !doSpecial) doGeneric = doSpecial = true;

   if (minF < 100) minF = 100;
   if (maxF < minF) maxF = minF * 10;
   if (minN < 100) minN = 100;
   if (maxN < minN) maxN = minN * 10;
   
   if (!haveGenericExp)
      strcpy(genericExpression, "f!-1");

   if (!haveSpecialExp)
   {
      strcpy(specialExpression, "3*2^n+1");
      k = 3.0;
      b = 2;
      c = 1;
   }

   PFPrintfLog("Benchmarking!  This may take several minutes.\n");

   if (!g_bExitNow && doFactor)
      benchmarkFactor(pContext);

   if (!g_bExitNow && doGeneric)
      benchmarkGeneric(pContext, genericExpression, minF, maxF, allFFT);

   if (!g_bExitNow && doSpecial)
      benchmarkSpecial(pContext, specialExpression, minN, maxN, k, b, c, allFFT);
}
