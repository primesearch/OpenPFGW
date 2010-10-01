#include "primeformpch.h"
#include <signal.h>
#include "../../pform/pfio/pfini.h"

#include "pfgw_globals.h"

//#define JBC_BUILD

#if defined (JBC_BUILD)
#include "jbc.cxx"
#else
#define JBC()
#endif

extern int g_CompositeAthenticationLevel;
#ifndef _MSC_VER
Integer *ex_evaluate(PFSymbolTable *pContext,const PFString &e,int m);
#endif

void PhiCofactorExperiment(PFSymbolTable *psym,const PFString &sPhi,const PFBoolean &bFactors,const PFBoolean &bDeep,const PFBoolean &bOnlyFactors)
{
   char sExpression[100];
   int iPhi=0;
   PFBoolean bUnfactored=PFBoolean::b_true;

   // the factored part

   JBC()

   {
      iPhi=atoi(LPCTSTR(sPhi));
      sprintf(sExpression,"%d",iPhi);
      PFString sTest=sExpression;
      if(sTest!=sPhi)
      {
         iPhi=0;
      }
      sprintf(sExpression,"Phi(%s,2)",LPCTSTR(sPhi));
      sprintf(g_cpTestString,"Phi(%s,2)",LPCTSTR(sPhi));
   }

   Integer *N=NULL;

   if(iPhi>1)
   {
      N=ex_evaluate(psym,sExpression);
      if(bFactors)
      {
         int lastLineLen=0;
         // we have a GWContext that can happily handle up to 320 bit modular reductions in one go,
         // that's nice
         Integer PS(iPhi);       // the step
         if(PS&1)
         {
            PS*=2;
         }
         Integer PF(PS);
         ++PF;             // the first one to try

#if 1
         int Mod = PS&0x7FFFFFFF;
         Erat_Mod Phi_mod(Mod);
         //Phi_mod.AddModCondition(8,1);
         //Phi_mod.AddModCondition(8,7);
         Phi_mod.init();
#endif

         Integer PP;

         // estimating timings now of the factoring loop, see how much we can do in 10 seconds
         PP=PF;

         Integer Q;
         int iLog;
         int iNext;

         int k=1;
         Integer POW(iPhi);
         int iTotal=lg(POW);

         Integer R;

         // don't get caught by the exception to the rule
         R=iPhi;
         while(R>1)
         {
            R=gcd(R,*N);
            if(R>1)
            {
               (*N)/=R;
               // output R as a factor
               PFIntegerSymbol *f=new PFIntegerSymbol("_FACTOR",new Integer(R));
               PFString s=f->GetStringValue();
               delete f;
               PFPrintfLog("%s has factor: %s\n",g_cpTestString,LPCTSTR(s));
               bUnfactored=PFBoolean::b_false;
            }
         }

         // Add a trivial wheel sieve
         const unsigned iWheel=3*5*7*11*13*17*19*23*29U;
         unsigned iSieve=PP%iWheel;
         unsigned iStep=PS%iWheel;

         while (
         ((iSieve%3)==0)||((iSieve%5)==0)||((iSieve%7)==0)||((iSieve%11)==0)||
         ((iSieve%13)==0)||((iSieve%17)==0)||((iSieve%19)==0)||((iSieve%23)==0)||((iSieve%29)==0))
         {
            k++;
            PP+=PS;
            iSieve+=iStep;
            iSieve%=iWheel;
         }

         // how long to factor for? Well, Phi(p(67)^2,2) takes (unscientifically)
         // 500 seconds, while we can factor 10000 K per second without breaking into a sweat.

         // Note that factoring time grows wih log n, while testing time grows with n^2 log n
         // Since n is about 10^5 in this example, choose base 10 logs and estimate F on the
         // slow side

         // F = C log n C=2e-5
         // P = D n^2 log n D=1e-8

         // so try a factor limit of 0.0005*n^2 (5 million for n=10^5), 1/2000.
         // Let's make factor time 20%, so call it 1/10000.
         // This is science at its worst, isn't it.

         Integer KMAX=iPhi;
         KMAX*=iPhi;
         //KMAX/=10000;
         // Different limit for use with erat_mod
         KMAX/=1000;

         IPFSymbol *pSymbol=psym->LookupSymbol("_FACTORIZE");
         if (pSymbol)
         {
            Integer FactPercent = *(((PFIntegerSymbol*)pSymbol)->GetValue());
            if (FactPercent > 0)
            {
               int iFactPercent=FactPercent&0x0000FFFF;
               PFPrintfStderr("Factoring numbers to %d%% of normal.\n", iFactPercent);
               KMAX*=iFactPercent;
               KMAX/=100;
            }
         }

         if(KMAX<10000) KMAX=10000;

         int kmax=KMAX&0x7FFFFFFF;
         int klast=0;

         if(KMAX!=kmax) kmax=2000000000;

         if(bUnfactored||bDeep)
         {
            PFPrintfStderr("%s: Trial factoring to %d*%d+1\n",g_cpTestString,kmax,PS&0x7FFFFFFF);
         }
         while(k<kmax && (bUnfactored || bDeep))
         {
            if((klast==0)||(k-klast>25000))
            {
               klast=k;
               // 120 bytes will not overflow, since we "force" the max size within the sprintf()
               char Buf[120];
               if (g_bErrorCheckAllTests)
                  sprintf(Buf, "%.60s %d/%d mro=%0.10g\r",g_cpTestString,k,kmax, gw_get_maxerr(&gwdata));
               else
                  sprintf(Buf, "%.60s %d/%d\r",g_cpTestString,k,kmax);
               int thisLineLen = (int) strlen(Buf);
               if (lastLineLen > thisLineLen)
               // When mixing stdio, stderr and redirection with a \r stderr output,
               // then the line must "erase" itself, IF it ever shrinks.
               PFPrintfClearCurLine(lastLineLen);
               lastLineLen = thisLineLen;
               PFPrintfStderr("%s", Buf);
               fflush(stderr);
            }

            Q=1;
            iLog=0;
            iNext=lg(PP);

            do
            {
               Q*=PP;
               //iLog+=iNext;
               iLog=lg(Q);

#if 1
               uint64 u64prime = Phi_mod.next();
               PP = Integer(u64prime);
               k = (int)(u64prime/Mod);
#else

               do
               {
                  k++;
                  PP+=PS;
                  iSieve+=iStep;
                  iSieve%=iWheel;
               }
               while(
               ((iSieve%3)==0)||((iSieve%5)==0)||((iSieve%7)==0)||((iSieve%11)==0)||
               ((iSieve%13)==0)||((iSieve%17)==0)||((iSieve%19)==0)||((iSieve%23)==0)||((iSieve%29)==0));
#endif

               iNext=lg(PP)+1;
            }
            while(iLog+iNext<=320);    // amass up to 320 bits in Q.

            // calculate 2^iPhi mod Q
            gwinit2(&gwdata, sizeof(gwhandle), (char *) GWNUM_VERSION);
            gwsetmaxmulbyconst(&gwdata, 2);  // maximum multiplier

            if (CreateModulus(&Q, true)) return;
            {
               GWInteger gwX;
               gwX=2;
               gwsetmulbyconst(&gwdata,2);

               for(int i=iTotal;i--;)
               {
                  int errchk = ErrorCheck(iTotal-i, iTotal);
                  gwsetnormroutine(&gwdata,0,errchk,bit(POW,i));
                  gwsquare(gwX);
               }
               R = gwX;
               --R;
            }
            DestroyModulus();

            // Phi(p,b) = (b^p-1)/(b-1)
            // Q < R only when Q is of the form Phi(p,b) as we used the modulus
            // (b^p-1) in the loop above.
            R %= Q;

            // factors of N are possibly in gcd(Q,R)
            R=gcd(Q,R);

            if (g_bExitNow) return;

            while(R>1)
            {
               R=gcd(R,*N);
               if(R>1)
               {
                  (*N)/=R;
                  // output R as a factor
                  PFIntegerSymbol *f=new PFIntegerSymbol("_FACTOR",new Integer(R));
                  PFString s=f->GetStringValue();
                  delete f;
                  PFPrintfLog("%s has factor: %s\n",g_cpTestString,LPCTSTR(s));
                  bUnfactored=PFBoolean::b_false;
               }
            }
         }
         PFPrintfClearCurLine(lastLineLen);
      }
   }
   else
   {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Could not find cyclotomic polynomial.\n");
   }

   if (!bOnlyFactors && (bUnfactored || bDeep) && N)
   {
      int fftSize = g_CompositeAthenticationLevel - 1;
      int testResult;

      if (!bUnfactored)
         strcat(g_cpTestString," cofactor");

      // Let's begin the experiment by creating a Context
      if (iPhi > 1)
      {
         do
         {
            fftSize++;

            gwinit2(&gwdata, sizeof(gwhandle), (char *) GWNUM_VERSION);
            gwsetmaxmulbyconst(&gwdata, iBase); // maximum multiplier

            if (CreateModulus(1.0, 2, iPhi, -1, fftSize)) return;

            testResult = prp_using_gwnum(N, iBase, sExpression, NULL, fftSize);

            DestroyModulus();
         } while (testResult == -1 && fftSize < 5);
      }
   }

   // so idle
   if (N)
      delete N;
}
