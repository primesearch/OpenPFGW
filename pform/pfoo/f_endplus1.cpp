#include "pfoopch.h"
#include "f_endplus1.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "primeserver.h"
#include "testreturns.h"

extern int g_ExtraSQFree;
extern bool volatile g_bExitNow;

F_EndPlus1::F_EndPlus1()
   : PFFunctionSymbol("@endplus1")
{
}

DWORD F_EndPlus1::MinimumArguments() const
{
   return 2;
}

DWORD F_EndPlus1::MaximumArguments() const
{
   return 2;
}

DWORD F_EndPlus1::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_EndPlus1::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval="";

   switch(dwIndex)
   {
   case 0:
      sRetval="_N";
      break;
   case 1:
      sRetval="_G";
      break;
   default:
      break;
   }

   return sRetval;
}

PFBoolean F_EndPlus1::CallFunction(PFSymbolTable *pContext)
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
            pSymbol=pContext->LookupSymbol("_G");
            if(pSymbol)
            {
               if(pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
               {
                  Integer *g=((PFIntegerSymbol*)pSymbol)->GetValue();
                  if(g)
                  {
                     int iRetval=PT_INCONCLUSIVE;
                     if(*g<=1)
                     {
                        bRetval=PFBoolean::b_true;
                     }
                     else
                     {
                        Integer N(*n);
                        N++;
                        if(N%(*g)==0)
                        {
                           bRetval=PFBoolean::b_true;

                           N/=(*g);
                           Integer R=N%(*g);
                           Integer Q=N/(*g);

                           // A little different this time
                           // AB.G + (B-A) = QG+R
                           // So (B-A) = kG+R   // maintain this in R
                           // AB = Q-k       // maintain this in Q

                           // note minimum is abs(kG+R)+1
                           // so positive cutoff is where
                           // kG+R+1 > Q-k k>(Q-1-R)/(G+1)

                           // negative cutoff is where
                           // -kG-R+1 > Q-k k<-(Q-1+R)/(G-1)

                           // there are possible solutions with A>0, B>0 in two possible
                           // domains depending on whether B-A is +ve or negative.

                           // if k<0, B-A is negative (kG+R) and so put B=1, A=1-(kG+R)
                           // and so AB is at least 1-(kG+R).
                           // 1-(kG+R)<=Q-k
                           // 1-R-Q<=(G-1)k

                           // the combined test extension uses F
                           PFIntegerSymbol *pSF=(PFIntegerSymbol*)pContext->LookupSymbol("_F");
                           Integer F(*(pSF->GetValue()));
                           if((((*g)&1)==0)&&((F&1)==0))
                           {
                              F>>=1;         // G is now the stepsize/modulus for K
                           }

                           Integer KMAX=(Q-1-R);
                           Integer MKMIN=0;

                           if(KMAX>=0)
                           {
                              MKMIN=KMAX;
                              KMAX/=((*g)+1);
                              MKMIN/=((*g)-1);
                           }
                           else
                           {
                              KMAX=-1;
                              MKMIN=0;
                           }

                           // the adjustments require K=Q mod F
                           Integer GF=(*g);
                           if(F>1)
                           {
                              GF*=F;
                              // move KMAX DOWN and -MKMIN UP if needed to
                              // points where they equal Q mod F.
                              if(KMAX+MKMIN>=0)
                              {
                                 Integer QMODF=Q%F;

                                 Integer NKMAX=KMAX;
                                 NKMAX-=(KMAX%F);
                                 NKMAX+=QMODF;
                                 if(NKMAX>KMAX)
                                 {
                                    NKMAX-=F;
                                 }
                                 KMAX=NKMAX;

                                 Integer NMKMIN=MKMIN;
                                 NMKMIN-=(MKMIN%F);   // this is already 'less negative'
                                 NMKMIN+=F;
                                 NMKMIN-=QMODF;
                                 if(NMKMIN>MKMIN)
                                 {
                                    NMKMIN-=F;
                                 }
                                 MKMIN=NMKMIN;
                              }
                           }

                           Integer KTEST(KMAX+MKMIN);
                           KTEST/=F;

                           if (KTEST>g_ExtraSQFree)
                           {
                              if ( KTEST < INT_MAX && numbits(*n) > 2000)
                                 PFPrintfLog("Proof incomplete rerun with -x%d\n", KTEST&INT_MAX);
                              iRetval=PT_INCONCLUSIVE;
                           }
                           else
                           {
                              int kCount=0,kMax = (KTEST & INT_MAX); //MB: For status
                              iRetval=PT_PRIME;

                              PFSymbolTable *pSubContext=new PFSymbolTable(pContext);

                              // move Q and R to k=-MKMIN
                              Integer K=-MKMIN;
                              Q+=MKMIN;
                              R-=MKMIN*(*g);

                              Integer RR=R*R;

                              PFBoolean bScanning=PFBoolean::b_true;

                              while(bScanning && bRetval && K<=KMAX && Q>0)
                              {
                                 Integer *pq=new Integer(RR+Q*4);
                                 if((*pq)<0)
                                 {
                                    delete pq;  //MB:  Added to cure memory leak
                                 }
                                 else
                                 {
                                    //pSubContext->AddSymbol(new PFIntegerSymbol("_N",pq));
                                    //int iRetHere=CallSubroutine("@issquare",pSubContext);

                                    // This is MUCH faster, but it locks us in to GMP.
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
                                       iRetval=PT_COMPOSITE;
                                       bScanning=PFBoolean::b_false;
                                       break;
                                    }
                                 }

                                 // move to next Q and R
                                 Q-=F;
                                 K+=F;

                                 // we're adding G to R
                                 RR+=R*GF;
                                 R+=GF;
                                 RR+=R*GF;
                                 if (g_bExitNow)
                                 {
                                    bRetval=PFBoolean::b_false;
                                    iRetval=PT_INCONCLUSIVE;
                                    break;
                                 }

                                 //MB:  Status reporting:
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

