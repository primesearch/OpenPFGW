#include "pfoopch.h"
#include "f_factor.h"
#include "factornode.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "pffactorizationsymbol.h"
#include "primeserver.h"
#include "expr.h"

extern char g_cpTestString[70];  // Located in pfiterativesymbol.cpp
extern int g_nIterationCnt;      // located in pfgw_main.cpp

bool g_bHideNoFactor=false;
bool g_bReLoadFactorFile=false;

//#define GWDEBUG(X) {Integer XX;XX=X;printf(#X "=");mpz_out_str(stdout,16,XX.gmp();printf("\n");}
#undef INTDEBUG
#define INTDEBUG(X) {printf(#X "=");mpz_out_str(stdout,16,(X).gmp();printf("\n");}

// Find the exact power of P that divides X. Based on Jim Fougeron's special case for proth code
// Also divides this power out of X. Depends on the integer library having an efficient
// quotient and remainder function.

extern bool volatile g_bExitNow;
#ifndef _MSC_VER
Integer *ex_evaluate(PFSymbolTable *pContext,const PFString &e,int m);
#endif

uint32 ExactPower(Integer &X,const Integer &P_)
{
   Integer P = P_;
   uint32 iRetval=0;
   static Integer *pPowers[32];     // plenty big enough

   Integer XX,YY,ZZ;
   Integer *apItems[3];

   uint32 iCurrent=0;
   uint32 iQuotient=1;
   uint32 iRemainder=2;

   apItems[iCurrent]=&XX;
   apItems[iQuotient]=&YY;
   apItems[iRemainder]=&ZZ;

   XX=X;

   pPowers[0]=&P;                // copy the pointer
   uint32 iMaxIndex=0;              // how many we have created
   uint32 iCurrentPower=1;
   Integer *PP=pPowers[0];          // the current power of P

   bool bScanning=true;

   // suppose PP does not divide X, and X=Q.PP+R. Then the exact power of P that divides X is the
   // exact power of P, call it PQ, that divides R, and the desired quotient is Q.(PP/PQ)+(R/PQ)
   while(bScanning)
   {
      apItems[iCurrent]->divmod(*PP,*apItems[iQuotient],*apItems[iRemainder]);

      if(*apItems[iRemainder]==0)
      {
         // exchange current and quotient
         DWORD t=iCurrent; iCurrent=iQuotient; iQuotient=t;
         iRetval+=iCurrentPower;

         iMaxIndex++;
         iCurrentPower<<=1;

         pPowers[iMaxIndex]=new Integer(*PP);
         (*pPowers[iMaxIndex])*=(*PP);
         PP=pPowers[iMaxIndex];
      }
      else
      {
         // exchange current and remainder
         DWORD t=iCurrent; iCurrent=iRemainder; iRemainder=t;
         bScanning=false;
      }
   }

   // Note that once getting here, all powers up to BUT NOT INCLUDING
   // PP have been identified as divisors. Note the correctly-divided out
   // portion is sitting in iRemainder (it was current last time)
   if(iRetval!=0)
   {
      // only bother to do the rest if anything divided
      X=*apItems[iRemainder];

      while(iMaxIndex>0)
      {
         delete pPowers[iMaxIndex];    // this doesn't divide so you may as well trash it
         iMaxIndex--;
         iCurrentPower>>=1;
         PP=pPowers[iMaxIndex];

         apItems[iCurrent]->divmod(*PP,*apItems[iQuotient],*apItems[iRemainder]);

         if(*apItems[iRemainder]==0)
         {
            // exchange current and quotient
            DWORD t=iCurrent; iCurrent=iQuotient; iQuotient=t;
            iRetval+=iCurrentPower;
            X/=(*PP);
         }
         else
         {
            // exchange current and remainder
            DWORD t=iCurrent; iCurrent=iRemainder; iRemainder=t;
            bScanning=false;
         }
      }
   }

   return iRetval;
}

class FactorHelperArray
{
   public:
      FactorHelperArray() {m_Array=0;m_nArray=0;m_nMaxArray=0;}
      ~FactorHelperArray() {delete[] m_Array;m_Array=0;m_nArray=0;m_nMaxArray=0;}
      void AddFactor(const Integer *pI);
      uint32 Count() { return m_nArray;}
      const Integer *GetItem(uint32 i) {if (i < 0 || i > m_nArray) return 0; return &m_Array[i]; }
   private:
      uint32 m_nArray, m_nMaxArray;
      Integer *m_Array;

};


void FactorHelperArray::AddFactor(const Integer *pI)
{
   if (m_nArray == m_nMaxArray)
   {
      m_nMaxArray += 100;
      Integer *p = new Integer[m_nMaxArray];
      for (uint32 x = 0; x < m_nArray; ++x)
         p[x] = m_Array[x];
      delete[] m_Array;
      m_Array = p;
   }
   m_Array[m_nArray++] = *pI;
}

PFSimpleFile *F_Factor::pFactorHelperFile=NULL;

/* Variables P, F, and G, which tracked the factored part of N, N-1, N+1, have been
   deprecated. There's no need to track them */

F_Factor::F_Factor()
   :  PFIterativeSymbol("@factor"),
      pN(NULL), Q(0), R(0), S(0), P1(0), pBiggest(NULL), pHelperArray(NULL),
      m_sHelperFile(""), pmin(0), pmax(0),
      bFactorAtAll(PFBoolean::b_false),  bDeep(PFBoolean::b_false),
      p(0), m_nPercentMultiplier(100),
      m_bModFactor(false), m_bDualModFactor(false), m_nModFactor(1),
      m_pEratMod(NULL), m_pEratMod2(NULL),
      m_pffNminus1(NULL), m_pffN(NULL), m_pffNplus1(NULL)
{
}

F_Factor::~F_Factor()
{
   delete m_pEratMod;
   delete m_pEratMod2;
   delete pHelperArray;
}

DWORD F_Factor::MinimumArguments() const
{
   return 1;
}

DWORD F_Factor::MaximumArguments() const
{
   return 1;
}

DWORD F_Factor::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_Factor::GetArgumentName(DWORD /*dwIndex*/) const
{
   return "_N";
}

PFBoolean F_Factor::OnExecute(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pSymbol=pContext->LookupSymbol("_N");

   if(pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      pN=((PFIntegerSymbol*)pSymbol)->GetValue();
      bRetval=PFBoolean::b_true;
   }

   m_sHelperFile="";
   pSymbol=pContext->LookupSymbol("_HELPER");
   if(pSymbol && pSymbol->GetSymbolType()==STRING_SYMBOL_TYPE)
   {
      m_sHelperFile=pSymbol->GetStringValue();
   }

   m_bModFactor = false;
   m_bDualModFactor = false;
   pSymbol=pContext->LookupSymbol("_USEMODFACTOR");
   if(pSymbol && pSymbol->GetSymbolType()==STRING_SYMBOL_TYPE)
   {
      pSymbol=pContext->LookupSymbol("_MODFACTOR");
      if(pSymbol && pSymbol->GetSymbolType()==STRING_SYMBOL_TYPE)
      {
         m_bModFactor = true;

         // Now see if this is a "dual" mod factor
         char Buf[1024];
         strcpy(Buf, (LPCSTR)pSymbol->GetStringValue());
         char *cp = strstr(Buf, "+-1}");
         if (cp)
         {
            m_bDualModFactor = true;
            memmove(&cp[1], &cp[2], strlen(&cp[1]));
            m_pEratMod2 = new Erat_Mod(Buf);
            if (!m_pEratMod2->isValid())
            {
               delete m_pEratMod2;
               m_pEratMod2 = 0;
               m_bDualModFactor = false;
            }
            *cp = '-';
         }
         m_pEratMod = new Erat_Mod(Buf);
         if (!m_pEratMod->isValid())
         {
            delete m_pEratMod;
            m_pEratMod = 0;
            m_bModFactor = false;
         }
         else
            m_nModFactor = m_pEratMod->GetModVal();
      }
   }

   // lookup the symbol table names
   m_pffNminus1=NULL;
   pSymbol=pContext->LookupSymbol("_SM");
   if(pSymbol && pSymbol->GetSymbolType()==STRING_SYMBOL_TYPE)
   {
      PFString sTableName=pSymbol->GetStringValue();
      m_pffNminus1=(PFFactorizationSymbol*)pContext->LookupSymbol(sTableName);
   }

   m_pffN=NULL;
   pSymbol=pContext->LookupSymbol("_SN");
   if(pSymbol && pSymbol->GetSymbolType()==STRING_SYMBOL_TYPE)
   {
      PFString sTableName=pSymbol->GetStringValue();
      m_pffN=(PFFactorizationSymbol*)pContext->LookupSymbol(sTableName);
   }

   m_pffNplus1=NULL;
   pSymbol=pContext->LookupSymbol("_SP");
   if(pSymbol && pSymbol->GetSymbolType()==STRING_SYMBOL_TYPE)
   {
      PFString sTableName=pSymbol->GetStringValue();
      m_pffNplus1=(PFFactorizationSymbol*)pContext->LookupSymbol(sTableName);
   }

   if(m_pffNminus1)
   {
      R=*pN-1;
      pBiggest=&R;
   }

   if(m_pffN)
   {
      Q=*pN;
      pBiggest=&Q;
   }

   if(m_pffNplus1)
   {
      S=*pN+1;
      pBiggest=&S;
   }

   // retrieve pmin, pmax and deep
   pmin=0;
   pSymbol=pContext->LookupSymbol("_PMIN");
   if(pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      Integer *ipmin=((PFIntegerSymbol*)pSymbol)->GetValue();
      // Phil's hack to get a 64-bit value
      uint64 n=1;
      ipmin->m_mod(n<<62, &pmin);
      if (pmin > n<<48)
      {
         PFPrintfStderr("WARNING, trial factoring past 2^48 is NOT tested, and may not work correctly\n");
      }
#if !defined (NDEBUG) && (0)
      char i64Buf[40];
      PFPrintfStderr("Phil - pmin="ULL_FORMAT" (1)\n", pmin);
#endif
   }

   pmax=0;
   pSymbol=pContext->LookupSymbol("_PMAX");
   if(pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      Integer *ipmax=((PFIntegerSymbol*)pSymbol)->GetValue();
      // Phil's hack to get a 64-bit value
      uint64 n=1;
      ipmax->m_mod(n<<62, &pmax);
      if (pmax > n<<48)
      {
         PFPrintfStderr("WARNING, trial factoring past 2^48 is NOT tested, and may not work correctly\n");
      }
#if !defined (NDEBUG) && (0)
      char i64Buf[40];
      PFPrintfStderr("Phil - pmax="ULL_FORMAT" (1)\n", pmax);
#endif
   }

   bDeep=PFBoolean::b_false;
   pSymbol=pContext->LookupSymbol("_DEEPFACTOR");
   if(pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      Integer *ipdeep=((PFIntegerSymbol*)pSymbol)->GetValue();
      bDeep=(*ipdeep&1)?PFBoolean::b_true:PFBoolean::b_false;
   }

   bFactorAtAll=PFBoolean::b_false;
   pSymbol=pContext->LookupSymbol("_FACTORIZE");
   if(pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      bFactorAtAll=PFBoolean::b_true;
      // the "multiplier" value.   This value is an integer in a "percent" value.  i.e. 100 is
      // the "normal" and 200 would be to search twice as high.
      if (!pmax)
      {
         Integer FactPercent = *(((PFIntegerSymbol*)pSymbol)->GetValue());
         if (FactPercent > 0)
            m_nPercentMultiplier=FactPercent&0x0000FFFF;
      }
   }

   return bRetval;
}

double F_Factor::estimatePrimes(double l)
{
   double d=0.0;
   if(l>3.0)
   {
      d=(l/(log(l)-1.0));
   }
   return d;
}

double F_Factor::estimatePrimes(double l1,double l2)
{
   return(estimatePrimes(l2)-estimatePrimes(l1));
}

// we will call it "the first 4n primes" for an n-bit number
double F_Factor::estimateLimit(double x)
{
   return(x*log(x));
}

void F_Factor::CleanupHelperFileObject()
{
   delete pFactorHelperFile;
   pFactorHelperFile = 0;
}

PFBoolean F_Factor::OnInitialize()
{
   // get an estimate of the limit. We will use the metric that we expect 32 bit numbers
   // to be fully factored. ie 2^5 bits are factored up to 2^13 primes.
   if (pmax==0)
   {
      double dPrimes=7.0*numbits(*pN)+1.0;
      double pp=estimateLimit(dPrimes);

      if (pp > (double) primeserver->GetUpperLimit())
         pmax = primeserver->GetUpperLimit();
      else
         pmax = (uint64) pp;

      if (pmax < 65536) pmax=65536;

      if ((m_pffN==NULL)&&(m_pffNminus1==NULL)&&(m_pffNplus1==NULL))
      {
         pmax=65536;
      }

#if !defined (NDEBUG) && (0)
      char i64Buf[40];
      PFPrintfStderr("Phil - pmax="ULL_FORMAT" (2)\n", pmax);
#endif

      if (m_bModFactor)
      {
         pmax *= m_pEratMod->GetModVal();
         double d = (double)(int64)pmax;
         if (m_bDualModFactor)
            // Note if dual modular factoring, then divide by 2 (well almost divide by 2)
            d *= 0.6;

         if (d > (double) primeserver->GetUpperLimit())
         {
            delete primeserver;
            primeserver = new PrimeServer(d);
         }

         pmax = primeserver->GetUpperLimit();
      }
#if !defined (NDEBUG) && (0)
      PFPrintfStderr("Phil - pmax="ULL_FORMAT" (3)\n", pmax);
#endif

      // did the user "request" a multiplier to the defalt factorization.
      if (m_nPercentMultiplier != 100)
      {
         double d = m_nPercentMultiplier;
         d /= 100;
         d *= (double) pmax;

         if (d > (double) primeserver->GetUpperLimit())
         {
            delete primeserver;
            primeserver = new PrimeServer(d);
         }

         pmax = primeserver->GetUpperLimit();
#if !defined (NDEBUG) && (0)
         PFPrintfStderr("Phil - pmax="ULL_FORMAT" (4)\n", pmax);
#endif
      }
   }

   P1=pmax;
   P1*=P1;
   pmaxadjust(pBiggest);

   if (pmin>pmax)
   {
      pmin=pmax;
   }

   double StepsVal = estimatePrimes((double)(int64)pmin,(double)(int64)pmax) / 2;
   m_dwStepsTotal =  (DWORD)(StepsVal/m_nModFactor + 1);
   m_dwStepGranularity=2048;
   m_bStopOverride=PFBoolean::b_true;

   Timer.Start();

   if (!m_bModFactor)
   {
      primeserver->SkipTo(pmin);
      p = primeserver->NextPrime();
   }
   else
   {
      m_pEratMod->init();
      if (pmin && pmin-1 > m_pEratMod->GetModVal())
         m_pEratMod->skipto(pmin);
      p = m_pEratMod->next();
      if (m_bDualModFactor)
      {
         m_pEratMod2->init();
         if (pmin && pmin-1 > m_pEratMod2->GetModVal())
            m_pEratMod2->skipto(pmin);
      }
   }

#ifndef NDEBUG
   if(p==1) {PFPrintfStderr("ERROR!! primegen returned 1 as the first prime\n"); exit(0); }
#endif


   // before doing anything else, why not read in the factor helper
BailOut:;
   if(m_sHelperFile.IsEmpty())
   {
   }
   else
   {

      extern bool g_bReLoadFactorFile;
      if (g_bReLoadFactorFile)
      {
         delete pFactorHelperFile;
         pFactorHelperFile=0;
      }
      if (!pFactorHelperFile)
      {
         delete pHelperArray;
         pHelperArray = new FactorHelperArray;

         char *cpHelper = new char[strlen(m_sHelperFile)+1];
         strcpy(cpHelper, m_sHelperFile);
         char *cpFName = strtok(cpHelper, "\xFF");
         bool bResultValid;
         Integer Result;
         PFString sNumber;
         PFSymbolTable *pTable=new PFSymbolTable(m_pContext);

         DoTheNextFile:;

         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr("Reading factors from helper file %s\n",cpFName);
         const char *cpError;
         pFactorHelperFile = openInputFile(cpFName, NULL, &cpError);
         if (!pFactorHelperFile)
         {
            PFOutput::EnableOneLineForceScreenOutput();
            PFPrintfStderr("%s\n", cpError);
            cpFName = strtok(NULL, "\xFF");
            if (cpFName)
               goto DoTheNextFile;
            if (pHelperArray->Count())
               goto GotSome_So_Continue;
            m_sHelperFile = "";
            delete[] cpHelper;
            delete pTable;
            goto BailOut;
         }
         pFactorHelperFile->Rewind();
         while((pFactorHelperFile->GetNextLine(sNumber,&Result,&bResultValid)==PFSimpleFile::e_ok)&&!g_bExitNow)
         {
            if(sNumber.IsEmpty()) continue;
            if (!bResultValid)
            {
               Integer *pResult=ex_evaluate(pTable,LPCTSTR(sNumber));
               if(pResult==NULL) continue;
               pHelperArray->AddFactor(pResult);
               delete pResult;
            }
            else
               pHelperArray->AddFactor(&Result);
         }

         cpFName = strtok(NULL, "\xFF");
         if (cpFName)
         {
            delete pFactorHelperFile;
            goto DoTheNextFile;
         }

         GotSome_So_Continue:;

         delete[] cpHelper;
         delete pTable;
      }

      PFBoolean bRunHelper=PFBoolean::b_true;
      bool bResultValid;
      bResultValid=false;

      uint32 Cnt = 0;
      while(bRunHelper && Cnt < pHelperArray->Count() && !g_bExitNow)
      {
         // hmm, there is no screen output in this loop, should there be???

         const Integer *pResult = pHelperArray->GetItem(Cnt++);

         Integer I=(*pN)%(*pResult);
         const Integer &pp=*pResult;

         if(m_pffN && (I==0))
         {
            uint32 pc=ExactPower(Q,pp);
            if(pc!=0)
            {
               m_pffN->AddFactor(new FactorNode(pp,pc));
               pmaxadjust(&Q);
               if(!bDeep)
               {
                  bRunHelper=PFBoolean::b_false;      // stop deep factoring
               }
            }
         }

         bool bUsed=false;
         if(m_pffNminus1 && (I==1))             // divides N-1
         {
            bUsed = true;
            uint32 pc=ExactPower(R,pp);
            if(pc!=0)
            {
               m_pffNminus1->AddFactor(new FactorNode(pp,pc));
               pmaxadjust(&R);
            }
         }

         if(m_pffNplus1 && (I==(*pResult)-1))
         {
            bUsed = true;
            uint32 pc=ExactPower(S,pp);
            if(pc!=0)
            {
               m_pffNplus1->AddFactor(new FactorNode(pp,pc));
               pmaxadjust(&S);
            }
         }
         extern bool g_bTestingMode;
         extern bool g_bHideNoFactor;
         if (g_bTestingMode && !bUsed && !g_bHideNoFactor)
         {
            char *cp = pResult->Itoa();
            PFPrintfLog("Prime_Testing_Warning, unused factor from helper file: %s\n",cp);
            delete[] cp;
         }

      }
      // if the helper was quit early, then don't factor at all
      if(!bRunHelper)
      {
         pmax=1;
      }
   }

   if(bFactorAtAll && ((m_pffN)||(m_pffNminus1)||(m_pffNplus1)) )
      PFPrintf("trial factoring to " ULL_FORMAT "\n",pmax);

   return PFBoolean::b_true;
}

void F_Factor::OnPrompt()
{
// provided estimate primes is an overestimate, this will work

   double StepsVal = estimatePrimes((double)(int64)p,(double)(int64)pmax) / 2;
   m_dwStepsTotal =  (DWORD)(StepsVal/m_nModFactor + 1);
   m_dwStepsTotal+=m_dwStepsDone;

   // Update the screen if it has been a while
   if (g_nIterationCnt && Timer.GetSecs() > 5)
   {
      char Buf[256];
      if (*g_cpTestString)
         sprintf(Buf,"F: %.50s %lu/%lu\r",g_cpTestString, m_dwStepsDone,m_dwStepsTotal);
      else
         sprintf(Buf,"F: %lu/%lu\r",m_dwStepsDone,m_dwStepsTotal);
      int thisLineLen = (int) strlen(Buf);
      if (lastLineLen > thisLineLen)
         // When mixing stdio, stderr and redirection with a \r stderr output,
         // then the line must "erase" itself, IF it ever shrinks.
         PFPrintfClearCurLine(lastLineLen);
      lastLineLen = thisLineLen;
      PFPrintfStderr("%s", Buf);
      Timer.Start();
   }
}

//#define FDEBUG(x,y)   fprintf(stderr,"%u %u\n",p,p2)
#define  FDEBUG(x,y)

PFBoolean F_Factor::Iterate()
{
   if (p>pmax)
   {
      return(PFBoolean::b_true);       // end the test
   }
   uint64 p2;
   if (!m_bModFactor)
      p2 = primeserver->NextPrime();
   else
   {
      if (m_bDualModFactor)
         p2=m_pEratMod2->next();
      else
         p2=m_pEratMod->next();
   }

   // This is the ONLY 2^31 code left.
   uint64  li1, li2;

   FDEBUG(p,p2);
   if (p2 <= INT_MAX)
   {
       int  i1,i2;
      // Note that if the FP stack is not PERFECTLY clear before calling this function, then
      // there it will have problems and not work right (this was found inside of APSieve).
      // I think this bug may ALSO be in pfgw in some instances during factoring.

      // Note that STAT=120 and TAGS=FFFF should be seen in the VC registers debug window.
      pN->m_mod2((int)p,(int)p2,&i1,&i2);
      if (i1<0) i1+=(int)p;
      if (i2<0) i2+=(int)p2;
      li1=i1;
      li2=i2;
   }
   else
   {
      pN->m_mod2(p, p2, &li1, &li2);
   }

   if(m_pffN && (li1==0))
   {
      Integer P(p);

      // here is an "already" written version of the ExactPower function.  I have not yet had time
      // yet to see if it is faster or not. but it does give correct results.
      //int pc=mpz_remove(*(Q.gmp()), *(Q.gmp()),*(P.gmp()));
      int pc=ExactPower(Q,P);
      if(pc!=0)
      {
         m_pffN->AddFactor(new FactorNode(p,pc));
         if(!bDeep)
         {
            return(PFBoolean::b_true);
         }
         pmaxadjust(&Q);
      }
   }
   else
   {
      if(m_pffNminus1 && (li1==1))              // divides N-1
      {
         Integer P(p);
         uint32 pc=ExactPower(R,P);
         if(pc!=0)
         {
            m_pffNminus1->AddFactor(new FactorNode(p,pc));
            pmaxadjust(&R);
         }
      }
      if(m_pffNplus1 && (li1==p-1))
      {
         Integer P(p);
         uint32 pc=ExactPower(S,P);
         if(pc!=0)
         {
            m_pffNplus1->AddFactor(new FactorNode(p,pc));
            pmaxadjust(&S);
         }
      }
   }

   if(m_pffN && (li2==0))
   {
      Integer P(p2);
      uint32 pc=ExactPower(Q,P);
      if(pc!=0)
      {
         m_pffN->AddFactor(new FactorNode(p2,pc));
         if(!bDeep)
         {
            return(PFBoolean::b_true);
         }
         pmaxadjust(&Q);
      }
   }
   else
   {
      if(m_pffNminus1 && (li2==1))              // divides N-1
      {
         Integer P(p2);
         uint32 pc=ExactPower(R,P);
         if(pc!=0)
         {
            m_pffNminus1->AddFactor(new FactorNode(p2,pc));
            pmaxadjust(&R);
         }
      }
      if(m_pffNplus1 && (li2==p2-1))
      {
         Integer P(p2);
         uint32 pc=ExactPower(S,P);
         if(pc!=0)
         {
            m_pffNplus1->AddFactor(new FactorNode(p2,pc));
            pmaxadjust(&S);
         }
      }
   }

   // ready for the next iteration
   if (!m_bModFactor)
      p=primeserver->NextPrime();
   else
      p=m_pEratMod->next();
   return(PFBoolean::b_false);            // and its not quitting time yet
}

void F_Factor::checkBiggest(PFFactorizationSymbol *pF,Integer *pTarget)
{
   if(pF)
   {
      if((pBiggest!=pTarget)&&(*pTarget>*pBiggest))
      {
         pBiggest=pTarget;
      }
   }
}

void F_Factor::pmaxadjust(Integer *PP)
{
// step 1. Check if pBiggest needs changing
   if(PP==pBiggest)
   {
      // the item formerly the biggest has changed.
      checkBiggest(m_pffN,&Q);
      checkBiggest(m_pffNminus1,&R);
      checkBiggest(m_pffNplus1,&S);
   }

   // we are possibly factoring too far
   if(pBiggest && P1>*pBiggest)
   {
      // yes we are going too far
      Integer PMAX=squareroot(*pBiggest);
      //PMAX.m_mod(uint64(1)<<48, &pmax);
      PMAX.m_mod(uint64(1)<<62, &pmax);
#if !defined (NDEBUG) && (0)
      char i64Buf[40];
      PFPrintfStderr("Phil - pmax="ULL_FORMAT" (5)\n", pmax);
#endif
      P1=pmax;
      P1*=P1;
   }
}

PFBoolean F_Factor::OnCleanup(PFSymbolTable * /*pContext*/)
{
   return PFBoolean::b_true;
}

PFBoolean F_Factor::OnCompleted(PFSymbolTable *pContext)
{
   pContext->AddSymbol(new PFIntegerSymbol("_factoredto",new Integer(p)));
   P1=p;
   P1*=P1;

   // Fixed a BIG memory leak.  the destory for F_Factor is only called on program exit.  I was using the
   // destructor to clean up a EratMod object.  This destruction needs to be done after each factor trial, and
   // not only one time at the end of program  run.
   delete m_pEratMod;
   m_pEratMod = NULL;

   delete m_pEratMod2;
   m_pEratMod2 = NULL;

   m_bModFactor = false;

   // when we get here, all factoring up to and including p has been performed.
   // so if what is left is less than P1*P1, it's also prime.
   try
   {
   if(pmin<=2)
   {
      if((Q>1)&&(Q<P1))
      {
         if(m_pffN)
         {
            m_pffN->AddFactor(new FactorNode(Q,1));
         }
         Q=1;
      }

      if((R>1)&&(R<P1))
      {
         if(m_pffNminus1)
         {
            m_pffNminus1->AddFactor(new FactorNode(R,1));
         }
         R=1;
      }

      if((S>1)&&(S<P1))
      {
         if(m_pffNplus1)
         {
            m_pffNplus1->AddFactor(new FactorNode(S,1));
         }
         S=1;
      }
   }

   // P, F, and G are no longer tracked
   Integer X;
   if(m_pffN)
   {
      X=*pN;
      X/=Q;
      pContext->AddSymbol(new PFIntegerSymbol("_P",new Integer(X)));
      pContext->AddSymbol(new PFIntegerSymbol("_Q",new Integer(Q)));
   }
   if(m_pffNminus1)
   {
      X=*pN;
      --X;
      X/=R;
      pContext->AddSymbol(new PFIntegerSymbol("_F",new Integer(X)));
      pContext->AddSymbol(new PFIntegerSymbol("_R",new Integer(R)));
   }
   if(m_pffNplus1)
   {
      X=*pN;
      ++X;
      X/=S;
      pContext->AddSymbol(new PFIntegerSymbol("_G",new Integer(X)));
      pContext->AddSymbol(new PFIntegerSymbol("_S",new Integer(S)));
   }

   // set up the return value to be the number of unique factors of N
   int iRetval=0;
   if(m_pffN)
   {
      PFList<FactorNode> *pp=m_pffN->AccessList();
      iRetval=pp->GetSize();
   }
   testResult=iRetval;
   }
   catch(...)
   {
      PFPrintfLog ("\rError factoring, probably number if too big for alloca version of GMP\n");
      testResult=0;
      //throw;
   }

   return PFBoolean::b_true;
}
