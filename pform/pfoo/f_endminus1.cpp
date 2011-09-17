#include "pfoopch.h"
#include "f_endminus1.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "primeserver.h"
#include "testreturns.h"

#undef INTDEBUG
#define INTDEBUG(X) {printf(#X "=");mpz_out_str(stdout,16,*(X).gmp());printf("\n");}
//#define INTDEBUG(X)

extern int g_ExtraSQFree;
extern bool volatile g_bExitNow;

F_EndMinus1::F_EndMinus1()
   : PFFunctionSymbol("@endminus1")
{
}

DWORD F_EndMinus1::MinimumArguments() const
{
   return 2;
}

DWORD F_EndMinus1::MaximumArguments() const
{
   return 2;
}

DWORD F_EndMinus1::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_EndMinus1::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval="";

   switch(dwIndex)
   {
   case 0:
      sRetval="_N";
      break;
   case 1:
      sRetval="_F";
      break;
   default:
      break;
   }

   return sRetval;
}

PFBoolean F_EndMinus1::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pSymbol=pContext->LookupSymbol("_N");
   if(pSymbol)
   {
      if(pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
      {
         Integer *n=((PFIntegerSymbol*)pSymbol)->GetValue();
         if(n)
         {
            pSymbol=pContext->LookupSymbol("_F");
            if(pSymbol)
            {
               if(pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
               {
                  Integer *f=((PFIntegerSymbol*)pSymbol)->GetValue();
                  if(f)
                  {
                     int iRetval=PT_INCONCLUSIVE;
                     if(*f<=1)
                     {
                        bRetval=PFBoolean::b_true;
                     }
                     else
                     {
                        Integer N(*n);
                        --N;
                        if(N%(*f)==0)
                        {
                           bRetval=PFBoolean::b_true;

                           N/=(*f);
                           Integer R=N%(*f);
                           Integer Q=N/(*f);

                           // OK so now AB.F + (A+B) = QF+R
                           // So (A+B) = kF+R   // maintain this in R
                           // AB = Q-k       // maintain this in Q

                           // First establish k cutoff.
                           // When does kF+R-1>Q-k?
                           // when k>(Q-R+1)/(F+1)

                           // the extension for squarefree testing requires the symbol G.
                           // Note it is always there
                           PFIntegerSymbol *pSG=(PFIntegerSymbol*)pContext->LookupSymbol("_G");
                           Integer G(*(pSG->GetValue()));
                           // usually both (*f) and G are even, so remove the common factor
                           if((((*f)&1)==0)&&((G&1)==0))
                           {
                              G>>=1;         // G is now the stepsize/modulus for K
                           }

                           Integer KBEGIN(0);
                           Integer GF=(*f);
                           if(G>1)
                           {
                              KBEGIN=Q%G;
                              GF*=G;
                              R+=(KBEGIN*(*f));
                              Q-=KBEGIN;
                           }

                           Integer KMAX=(Q-R+1)/((*f)+1);

                           // the number of k values to be tested is?
                           Integer KTEST(KMAX);
                           if(KTEST>=0)
                           {
                              KTEST+=G;
                              KTEST-=KBEGIN;
                              KTEST/=G;
                           }

                           if(KMAX<0)
                           {
                              iRetval=PT_PRIME;
                           }
                           else if(KTEST>g_ExtraSQFree)
                           {
                              if (KTEST < INT_MAX && numbits(*n) > 2000)
                                 PFPrintfLog("Proof incomplete rerun with -x%d\n", KTEST&INT_MAX);
                              iRetval=PT_INCONCLUSIVE;
                           }
                           else
                           {
                              int kCount=0,kMax = (KMAX & INT_MAX); //MB: For status

                              iRetval=PT_PRIME;

                              PFSymbolTable *pSubContext=new PFSymbolTable(pContext);
                              Integer RR=R*R;

                              PFBoolean bScanning=PFBoolean::b_true;

                              while(bScanning && bRetval && (R-1<=Q) && (Q>0))
                              {
                                 Integer *pq=new Integer(RR-Q*4);
                                 if((*pq)<0)
                                 {
                                    delete pq;  //MB:  Added to cure memory leak
                                    break;
                                 }

                                 //pSubContext->AddSymbol(new PFIntegerSymbol("_N",pq));
                                 //int iRetHere=CallSubroutine("@issquare",pSubContext);

                                 // This is MUCH faster, but it locks us in to GMP.
                                 // This whole routine should really be sieving.
                                 int iRetHere=mpz_perfect_square_p(pq->gmp());

                                 delete pq;  //MB:  Added to cure memory leak

                                 switch(iRetHere)
                                 {
                                 case -1:
                                    bRetval=PFBoolean::b_false;
                                    iRetval=PT_INCONCLUSIVE;
                                    break;
                                 case 0:
                                    break;   // success on this test
                                 case 1:
                                    // this is asinine. Here we have A+B=R, AB=Q, A-B=sqrt(pq)
                                    iRetval=PT_COMPOSITE;
                                    bScanning=PFBoolean::b_false;
                                    break;
                                 }

                                 // move to next Q and R
                                 Q-=G;

                                 // we're adding GF to R, since (x+F)^2-x^2=(2x+F).F...
                                 RR+=R*GF;
                                 R+=GF;
                                 RR+=R*GF;
                                 if (g_bExitNow)
                                 {
                                    bRetval=PFBoolean::b_false;
                                    iRetval=PT_INCONCLUSIVE;
                                    break;
                                 }

                                 //MB: Report status to user every 8192 loops.
                                 kCount++;
                                 if ((kCount&0x1FFF)==1)
                                    PFPrintfLog("\r%d/%d\r",kCount,kMax);
                              }
                              delete pSubContext;
                           }
                        }
                        PFPrintfClearCurLine(70);  //MB: Get rid of status
                        pContext->AddSymbol(new PFIntegerSymbol("_result",new Integer(iRetval)));
                     }
                  }
               }
            }
         }
      }
   }
   return bRetval;
}

