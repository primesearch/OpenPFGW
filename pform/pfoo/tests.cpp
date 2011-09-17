/*=======================================================================*
 * PRIMEFORM/GW - a program to perform a variety of primality tests      *
 * Copyright (C) 1999-2000 Chris Nash, primeform.net                     *
 *                                                     *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the primeform.net Open Source License as        *
 * published by primeform.net; either version 1 of the License, or (at   *
 * your option) any later version.                                       *
 *                                                                       *
 * This program is distributed in the hope that it will be useful, but   *
 * WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * primeform.net Open Source License for more details.                   *
 *                                                                       *
 * You should have received a copy of the primeform.net Open Source      *
 * License along with this library; if not, contact the licensor at the  *
 * addresses below.                                                      *
 *                                                                       *
 * Write to:                                                             *
 * primeform.net, 5000 Bryan Station Rd #5, Lexington KY 40516-9505, USA *
 *                                                      *
 * e-mail:                                                *
 * For licensing questions: license@primeform.net                  *
 * For Open Source contributions: maintainer@primeform.net               *
 * For all other information: info@primeform.net                         *
 *=======================================================================*/
#include "pfoopch.h"
#include "tests.h"

#include "symboltypes.h"
#include "testreturns.h"

#include "pfintegersymbol.h"
#include "pffactorizationsymbol.h"
#include "pfstringsymbol.h"
#include "f_trivial.h"
#include "factornode.h"
#include "pfsamplersymbol.h"

extern PFString g_sTestMode;           // This will hold things like "PRP: ", "N+1: ", "F: ", "GF(b,3): ", ...
extern int g_nIterationCnt;

// The modified probable prime test. Attempts to produce a simple SPRP
// result. Note that this test does not correctly identify numbers as
// constructed by Francois Arnault. For such numbers you should use the
// Pocklington test (even with inconclusive enabled). Note also that the
// N-1 test is tuned for Woltman performance. A test base will *never*
// be chosen that is larger than 8 bits. This produces a 33% speedup.
// Note it is possible to do a manual Woltman multiplication for numbers
// up to about 30 bits, but this is beyond the scope of the current
// implementation (ie I haven't written that yet).
#if 0
T_PRP::T_PRP()
   : PFFunctionSymbol("isprp")
{
}

DWORD T_PRP::MinimumArguments() const
{
   return 1;
}

DWORD T_PRP::MaximumArguments() const
{
   return 1;
}

DWORD T_PRP::GetArgumentType(DWORD /*dw*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString T_PRP::GetArgumentName(DWORD /*dw*/) const
{
   return "_N";
}

PFBoolean T_PRP::CallFunction(PFSymbolTable *pContext)
{
   // 0  - composite
   // 1  - prp
   // 2  - prime
   // 3  - something unusual
   // -1 - is no longer running

   // to proceed, the following subfunctions need to be installed
   // trivialtest (eliminates negative, zero, one, even) -1 0 1 2
   // factor (factors N-1 N N+1)
   // nminus1 (testing engine)
   PFBoolean bProcessed=PFBoolean::b_false;
   int iReturnCode=0;
   PFBoolean bRetval=PFBoolean::b_true;

   PFSymbol *pSymbolN=pContext->LookupSymbol("_N");
   Integer *pN;
   PFFactorizationSymbol *pMinus=NULL;

   if(pSymbolN && pSymbolN->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      pN=((PFIntegerSymbol*)pSymbolN)->GetValue();

      PFSymbolTable *pSubContext=new PFSymbolTable(pContext);

      if(bRetval && !bProcessed)
      {
         int iRetval=CallSubroutine("@trivial",pSubContext);
         if(iRetval<0)
         {
            PFPrintfLog("negative number, negating\n");
            iRetval=-iRetval;
         }

         // still running, but check the trivial return code
         switch(iRetval)
         {
         case TT_ZERO:
            PFPrintfLog("zero\n");
            iReturnCode=PT_FACTOR;
            bProcessed=PFBoolean::b_true;
            break;
         case TT_ONE:
            PFPrintfLog("one\n");
            iReturnCode=PT_FACTOR;
            bProcessed=PFBoolean::b_true;
            break;
         case TT_TWO:
            PFPrintfLog("two\n");
            iReturnCode=PT_PRIME;
            bProcessed=PFBoolean::b_true;
            break;
         case TT_FACTOR:
            PFPrintfLog("trivially factored\n");
            iReturnCode=PT_FACTOR;
            bProcessed=PFBoolean::b_true;
            break;
         default:
            break;
         }
      }

      if(bRetval && !bProcessed)
      {
         // set up a couple of symbols in the symbol table
         pMinus=new PFFactorizationSymbol("_NMINUS1FACTOR");
         pSubContext->AddSymbol(pMinus);
         pSubContext->AddSymbol(new PFStringSymbol("_SM","_NMINUS1FACTOR"));

         g_sTestMode = "F: ";
         g_nIterationCnt *= 1000;
         int iRetval=CallSubroutine("@factor",pSubContext);
         g_nIterationCnt /= 1000;
         g_sTestMode = "";
         if(iRetval!=-1)
         {
            if(iRetval>0)
            {
               // the routine found a factor. However not finding a factor might be positive too
               PFPrintfLog("factored, small factor %d\n",iRetval);
               iReturnCode=0;
               bProcessed=PFBoolean::b_true;
            }
            else
            {
               // did we complete trial-factoring?
               IPFSymbol *pLimit=pSubContext->LookupSymbol("_factoredto");
               if(pUs && pLimit && pLimit->GetSymbolType()==INTEGER_SYMBOL_TYPE)
               {
                  Integer T=*((PFIntegerSymbol*)pLimit)=>GetValue();
                  T*=T;
                  if(*pN<T)
                  {
                     iReturnCode=2;    // prime by completion of trial factoring
                     PFPrintfLog("prime by trial factoring\n");
                     bProcessed=PFBoolean::b_true;
                  }
               }
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;
         }
      }

      if(!bProcessed && bRetval)
      {
         int iRetval=CallSubroutine("@issquare",pSubContext);
         if(iRetval!=-1)
         {
            if(iRetval==1)
            {
               iReturnCode=0;
               PFPrintfLog("square\n");
               bProcessed=PFBoolean::b_true;
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;      // stop processing
         }
      }

      if(!bProcessed && bRetval)
      {
         // setup for N-1 testing. Note we only need to extract the power of 2 divisor from N-1.
         // N-1 was factored into the symbol table _SM, which we can access directly with pMinus
         PFList<FactorNode> *pFactorList=pMinus->AccessList();
         PFForwardIterator pffi;
         pFactorList->StartIterator(pffi);
         PFListNode *pNode;
         pffi.Iterate(pNode);
         FactorNode *pFactor=(FactorNode*)pNode->GetData();    // the power of 2 divisor.

         // add it back to the factor table
         PFFactorizationSymbol *p2=new PFFactorizationSymbol("_POWER2");
         p2->AddFactor(new FactorNode(*pObject));
         pSymbol->AddSymbol(p2);

         pSymbol->AddSymbol(new PFStringSymbol("_FACTORTABLE","_POWER2");

         g_sTestMode = "N-1: ";
         int iRetval=CallSubroutine("nminus1",pSubContext);
         g_sTestMode = "";
         bProcessed=PFBoolean::b_true;

         if(iRetval==PT_INCONCLUSIVE)
         {
            g_sTestMode = "BLS: ";
            iRetval=CallSubroutine("@endminus1",pSubContext);
            g_sTestMode = "";
         }

         if(iRetval!=-1)
         {
            switch(iRetval)
            {
            case PT_INCONCLUSIVE:
               iReturnCode=1;
               PFPrintfLog("probable prime\n");
               break;
            case PT_PRIME:
               iReturnCode=2;
               PFPrintfLog("prime\n");
               break;
            case PT_COMPOSITE:
               iReturnCode=0;
               PFPrintfLog("composite\n");
               break;
            case PT_FACTOR:
               {
                  iReturnCode=0;
                  // the known factor is written to the symbol _FACTOR
                  IPFSymbol *pFactorSymbol=pSubContext->LookupSymbol("_FACTOR");
                  if(pFactorSymbol && pFactorSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
                  {
                     PFString sValue=pFactorSymbol->GetStringValue();
                     PFPrintfLog("factor: %s\n",sValue);
                  }
                  else
                  {
                     PFPrintfLog("composite\n");
                  }
               }
               break;
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;
         }
      }
      delete pSubContext;
      bProcessed=PFBoolean::b_true;
   }
   else
   {
      bProcessed=PFBoolean::b_false;
   }

   if(bProcessed)
   {
      pContext->AddSymbol(new PFIntegerSymbol("_result",new Integer(iReturnCode)));
   }
   else
   {
      pContext->EraseSymbol("_result");
   }

   return bRetval;
}
#endif

// The Pocklington N-1 test. Note the intention of this routine is to add support at some
// later time for inconclusive tests or known prime factors from a file. Currently the pocklington
// test will always execute regardless of whether its conclusion is conclusive or not. The test
// will attempt to make optimal choices of base
T_Pocklington::T_Pocklington()
   : PFFunctionSymbol("pocklington")
{
}

DWORD T_Pocklington::MinimumArguments() const
{
   return 1;
}

DWORD T_Pocklington::MaximumArguments() const
{
   return 1;
}

DWORD T_Pocklington::GetArgumentType(DWORD /*dw*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString T_Pocklington::GetArgumentName(DWORD /*dw*/) const
{
   return "_N";
}

PFBoolean T_Pocklington::CallFunction(PFSymbolTable *pContext)
{
   // 0  - composite
   // 1  - prp
   // 2  - prime
   // 3  - something unusual
   // -1 - is no longer running

   // to proceed, the following subfunctions need to be installed
   // trivialtest (eliminates negative, zero, one, even) -1 0 1 2
   // factor (factors N-1 N N+1)
   // nminus1 (testing engine)
   PFBoolean bProcessed=PFBoolean::b_false;
   int iReturnCode=0;
   PFBoolean bRetval=PFBoolean::b_true;

   IPFSymbol *pSymbolN=pContext->LookupSymbol("_N");
   Integer *pN;
   PFFactorizationSymbol *pMinus=NULL;
   PFFactorizationSymbol *pUs=NULL;
   PFFactorizationSymbol *pFactorization;

   if(pSymbolN && pSymbolN->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      pN=((PFIntegerSymbol*)pSymbolN)->GetValue();

      PFSymbolTable *pSubContext=new PFSymbolTable(pContext);

      pSubContext->AddSymbol(new PFSamplerSymbol());

      if(bRetval && !bProcessed)
      {
         int iRetval=CallSubroutine("@trivial",pSubContext);
         if(iRetval!=-1)
         {
            // still running, but check the trivial return code
            switch(iRetval)
            {
            case TT_ZERO:
               PFPrintfLog("zero\n");
               iReturnCode=PT_FACTOR;
               bProcessed=PFBoolean::b_true;
               break;
            case TT_ONE:
               PFPrintfLog("one\n");
               iReturnCode=PT_FACTOR;
               bProcessed=PFBoolean::b_true;
               break;
            case TT_TWO:
               PFPrintfLog("two\n");
               iReturnCode=PT_PRIME;
               bProcessed=PFBoolean::b_true;
               break;
            case TT_FACTOR:
               {
               iReturnCode=PT_FACTOR;
               bProcessed=PFBoolean::b_true;
               // Was it factored prime? This is true if there is only one factor
               pFactorization=(PFFactorizationSymbol *)pSubContext->LookupSymbol ("_TRIVIALFACTOR");
               PFString sTrivial=pFactorization->GetStringValue();
               if (pFactorization->AccessList()->GetSize () == 1 && !strchr(LPCTSTR(sTrivial), '^'))
               {
                  PFPrintfLog("small number, factored prime!\n");
                  iReturnCode = PT_PRIME;
               }
               else
                  PFPrintfLog("small number, factored\n");
               break;
               }
            case TT_NEGATIVE:
               PFPrintfLog("negative\n");
               (*pN)*=-1;
               break;
            default:
               break;
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;
         }
      }

      if(bRetval && !bProcessed)
      {
         // set up a couple of symbols in the symbol table
         pMinus=new PFFactorizationSymbol("_NMINUS1FACTOR");
         pSubContext->AddSymbol(pMinus);
         pSubContext->AddSymbol(new PFStringSymbol("_SM","_NMINUS1FACTOR"));

         // to save trial factoring time, we should only attempt to factor N if we were
         // actually asked to.
         IPFSymbol *pFactorize=pSubContext->LookupSymbol("_FACTORIZE");
         if(pFactorize)
         {
            pUs=new PFFactorizationSymbol("_NFACTOR");
            pSubContext->AddSymbol(pUs);
            pSubContext->AddSymbol(new PFStringSymbol("_SN","_NFACTOR"));
         }

         g_sTestMode = "F: ";
         g_nIterationCnt *= 1000;
         int iRetval=CallSubroutine("@factor",pSubContext);
         g_nIterationCnt /= 1000;
         g_sTestMode = "";

         if(iRetval!=-1)
         {
            if(iRetval>0)
            {
               // the routine found a factor. However not finding a factor might be positive too
               PFString sFactor=pUs->GetStringValue();
               // did we complete trial-factoring?
               if (pUs->AccessList()->GetSize() == 1)
               {
                  // See if the factor WAS our number.  If so, it IS prime.  This can happen when the user selects
                  // -f along with the -t (or -tc -tm -tp)
                  Integer tmp;
                  tmp.atoI(sFactor);
                  if (*pN == tmp)
                  {
                     iReturnCode=2;    // prime by completion of trial factoring
                     PFPrintfLog("prime by trial factoring\n");
                     bProcessed=PFBoolean::b_true;
                     iReturnCode = PT_PRIME;
                  }
               }
               if (bProcessed==PFBoolean::b_false)
               {
                  PFPrintfClearCurLine();
                  PFPrintfLog("factors: %s\n",LPCTSTR(sFactor));
                  iReturnCode=PT_FACTOR;
                  bProcessed=PFBoolean::b_true;
               }
            }
            else
            {
               // did we complete trial-factoring?
               IPFSymbol *pLimit=pSubContext->LookupSymbol("_factoredto");
               if(pUs && pLimit && pLimit->GetSymbolType()==INTEGER_SYMBOL_TYPE)
               {
                  Integer T=*((PFIntegerSymbol*)pLimit)->GetValue();
                  T*=T;
                  if(*pN<T)
                  {
                     iReturnCode=2;    // prime by completion of trial factoring
                     PFPrintfLog("prime by trial factoring\n");
                     bProcessed=PFBoolean::b_true;
                     iReturnCode = PT_PRIME;
                  }
               }
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;
         }
      }

      if(!bProcessed && bRetval)
      {
         int iRetval=CallSubroutine("@issquare",pSubContext);
         if(iRetval!=-1)
         {
            if(iRetval==1)
            {
               iReturnCode=PT_COMPOSITE;
               PFPrintfLog("square\n");
               bProcessed=PFBoolean::b_true;
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;      // stop processing
         }
      }

      if(!bProcessed && bRetval)
      {
         // select a target for the test. Since (AF+1)(BF+1) yields
         // AB.F + A+B = QF+R = (N-1)/F

         // A+B = kF+R
         // AB  = Q-k

         // when is kF+R-1 > Q-k?
         // when k > (Q-R+1)/(F+1) [about N/F^3].

         // Aim for cuberoot N. We need to find cuberoot N.
         // Note that N>2^lg. So we round up.
         DWORD dwExponent=(numbits(*pN)+3)/3;
         Integer *pT=new Integer(1);
         (*pT)<<=dwExponent;
         pSubContext->AddSymbol(new PFIntegerSymbol("_TARGET",pT));

         PFFactorizationSymbol *pFactor2=new PFFactorizationSymbol("_NMINUS1FACTOR2");
         pSubContext->AddSymbol(pFactor2);

         PFString sActive  ="_NMINUS1FACTOR";
         PFString sActive2 ="_NMINUS1FACTOR2";

         int iRetval=PT_INCONCLUSIVE;

         PFBoolean bLooping=PFBoolean::b_true;  // while we need or are still begging
         Integer iProven(1);                 // what's been proved so far.

         while(bLooping)
         {
            pSubContext->AddSymbol(new PFStringSymbol("_FACTORTABLE",sActive));
            pSubContext->AddSymbol(new PFStringSymbol("_UNPROVED",sActive2));

            // get sActive2
            PFFactorizationSymbol *pJunk=(PFFactorizationSymbol*)pSubContext->LookupSymbol(sActive2);
            PFList<FactorNode> *pList=pJunk->AccessList();
            pList->RemoveAll();

            g_sTestMode = "N-1: ";
            iRetval=CallSubroutine("nminus1",pSubContext);
            g_sTestMode = "";

            switch(iRetval)
            {
            case -1:
               bLooping=PFBoolean::b_false;     // breakpoint
               break;
            case PT_INCONCLUSIVE:
               // collate what's been proven
               {
                  PFIntegerSymbol *pI=(PFIntegerSymbol*)pSubContext->LookupSymbol("_PROVED");
                  iProven*=*(pI->GetValue());      // suck them in
                  if(*(pI->GetValue())>=*pT)
                  {
                     bLooping=PFBoolean::b_false;
                  }
                  else if(pList->GetSize()==0)
                  {
                     bLooping=PFBoolean::b_false;
                  }
                  else
                  {
                     // *pT, the target for conclusive proof needs to be modified
                     // so that next time, we don't get too many factors
                     Integer IP=*(pI->GetValue());
                     // divide it out of pT, again upround
                     (*pT)+=(IP-1);
                     (*pT)/=IP;
                  }
               }
               break;                        // may still need to run
            case PT_PRIME:
               bLooping=PFBoolean::b_false;     // done
               break;
            case PT_COMPOSITE:
               bLooping=PFBoolean::b_false;     // done
               break;
            case PT_FACTOR:
               bLooping=PFBoolean::b_false;     // done
               break;
            }

            // swap lists
            PFString t=sActive;
            sActive=sActive2;
            sActive2=t;
         }

         // if at this stage we're still inconclusive, call BLS
         if(iRetval==PT_INCONCLUSIVE)
         {
            Integer *F=new Integer(iProven);
            PFIntegerSymbol *pF=new PFIntegerSymbol("_F",F);
            PFIntegerSymbol *pG=new PFIntegerSymbol("_G",new Integer(1));  // disable advanced BLS
            pSubContext->AddSymbol(pF);
            pSubContext->AddSymbol(pG);

            PFPrintfLog("Calling Brillhart-Lehmer-Selfridge with factored part %.2f%%\n",double(100.0*numbits(*F))/double(numbits(*pN)));

            // the end call performs the Brillhart-Lehmer-Selfridge processing of the
            // Lenstra's Theorem result.
            g_sTestMode = "BLS: ";
            iRetval=CallSubroutine("@endminus1",pSubContext);
            g_sTestMode = "";
         }

         if(iRetval!=-1)
         {
            iReturnCode=iRetval;

            switch(iRetval)
            {
            case PT_INCONCLUSIVE:
               break;
            case PT_PRIME:
               break;
            case PT_COMPOSITE:
               break;
            case PT_FACTOR:
               {
                  // the known factor is written to the symbol _FACTOR
                  IPFSymbol *pFactorSymbol=pSubContext->LookupSymbol("_FACTOR");
                  if(pFactorSymbol && pFactorSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
                  {
                     PFString sValue=pFactorSymbol->GetStringValue();
                     PFPrintfLog("factor: %s\n",LPCTSTR(sValue));
                  }
                  else
                  {
                     iReturnCode=PT_COMPOSITE;
                     PFPrintfLog("composite\n");
                  }
               }
               break;
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;
         }
      }

      delete pSubContext;
      bProcessed=PFBoolean::b_true;
   }
   else
   {
      bProcessed=PFBoolean::b_false;
   }

   if(bProcessed)
   {
      pContext->AddSymbol(new PFIntegerSymbol("_result",new Integer(iReturnCode)));
   }
   else
   {
      pContext->EraseSymbol("_result");
   }

   return bRetval;
}

// The Morrison N+1 test. Note the intention of this routine is to add support at some
// later time for inconclusive tests or known prime factors from a file. Currently the morrison
// test will always execute regardless of whether its conclusion is conclusive or not. The test
// will attempt to make optimal choices of base
T_Morrison::T_Morrison()
   : PFFunctionSymbol("morrison")
{
}

DWORD T_Morrison::MinimumArguments() const
{
   return 1;
}

DWORD T_Morrison::MaximumArguments() const
{
   return 1;
}

DWORD T_Morrison::GetArgumentType(DWORD /*dw*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString T_Morrison::GetArgumentName(DWORD /*dw*/) const
{
   return "_N";
}

PFBoolean T_Morrison::CallFunction(PFSymbolTable *pContext)
{
   // 0  - composite
   // 1  - prp
   // 2  - prime
   // 3  - something unusual
   // -1 - is no longer running

   // to proceed, the following subfunctions need to be installed
   // trivialtest (eliminates negative, zero, one, even) -1 0 1 2
   // factor (factors N-1 N N+1)
   // nplus1 (testing engine)
   PFBoolean bProcessed=PFBoolean::b_false;
   int iReturnCode=0;
   PFBoolean bRetval=PFBoolean::b_true;
   PFFactorizationSymbol *pFactorization;

   IPFSymbol *pSymbolN=pContext->LookupSymbol("_N");
   Integer *pN;
   PFFactorizationSymbol *pPlus=NULL;
   PFFactorizationSymbol *pUs=NULL;

   if(pSymbolN && pSymbolN->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      pN=((PFIntegerSymbol*)pSymbolN)->GetValue();

      PFSymbolTable *pSubContext = new PFSymbolTable(pContext);
      PFSamplerSymbol *pSampler = new PFSamplerSymbol();

      pSubContext->AddSymbol(pSampler);

      if(bRetval && !bProcessed)
      {
         int iRetval=CallSubroutine("@trivial",pSubContext);
         if(iRetval!=-1)
         {
            // still running, but check the trivial return code
            switch(iRetval)
            {
            case TT_ZERO:
               PFPrintfLog("zero\n");
               iReturnCode=PT_FACTOR;
               bProcessed=PFBoolean::b_true;
               break;
            case TT_ONE:
               PFPrintfLog("one\n");
               iReturnCode=PT_FACTOR;
               bProcessed=PFBoolean::b_true;
               break;
            case TT_TWO:
               PFPrintfLog("two\n");
               iReturnCode=PT_PRIME;
               bProcessed=PFBoolean::b_true;
               break;
            case TT_FACTOR:
               {
               iReturnCode=PT_FACTOR;
               bProcessed=PFBoolean::b_true;
               // Was it factored prime? This is true if there is only one factor
               pFactorization=(PFFactorizationSymbol *)pSubContext->LookupSymbol ("_TRIVIALFACTOR");
               PFString sTrivial=pFactorization->GetStringValue();
               if (pFactorization->AccessList()->GetSize () == 1 && !strchr(LPCTSTR(sTrivial), '^'))
               {
                  PFPrintfLog("small number, factored prime!\n");
                  iReturnCode = PT_PRIME;
               }
               else
                  PFPrintfLog("small number, factored\n");
               break;
               }
            case TT_NEGATIVE:
               PFPrintfLog("negative\n");
               (*pN)*=-1;
               break;
            default:
               break;
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;
         }
      }

      if(bRetval && !bProcessed)
      {
         // set up a couple of symbols in the symbol table
         pPlus=new PFFactorizationSymbol("_NPLUS1FACTOR");
         pSubContext->AddSymbol(pPlus);
         pSubContext->AddSymbol(new PFStringSymbol("_SP","_NPLUS1FACTOR"));

         // to save trial factoring time, we should only attempt to factor N if we were
         // actually asked to.
         IPFSymbol *pFactorize=pSubContext->LookupSymbol("_FACTORIZE");
         if(pFactorize)
         {
            pUs=new PFFactorizationSymbol("_NFACTOR");
            pSubContext->AddSymbol(pUs);
            pSubContext->AddSymbol(new PFStringSymbol("_SN","_NFACTOR"));
         }

         g_sTestMode = "F: ";
         g_nIterationCnt *= 1000;
         int iRetval=CallSubroutine("@factor",pSubContext);
         g_nIterationCnt /= 1000;
         g_sTestMode = "";

         if(iRetval!=-1)
         {
            if(iRetval>0)
            {
               // the routine found a factor. However not finding a factor might be positive too
               PFString sFactor=pUs->GetStringValue();
               // did we complete trial-factoring?
               if (pUs->AccessList()->GetSize() == 1)
               {
                  // See if the factor WAS our number.  If so, it IS prime.  This can happen when the user selects
                  // -f along with the -t (or -tc -tm -tp)
                  Integer tmp;
                  tmp.atoI(sFactor);
                  if (*pN == tmp)
                  {
                     iReturnCode=2;    // prime by completion of trial factoring
                     PFPrintfLog("prime by trial factoring\n");
                     bProcessed=PFBoolean::b_true;
                     iReturnCode = PT_PRIME;
                  }
               }
               if (bProcessed==PFBoolean::b_false)
               {
                  PFPrintfClearCurLine();
                  PFPrintfLog("factors: %s\n",LPCTSTR(sFactor));
                  iReturnCode=PT_FACTOR;
                  bProcessed=PFBoolean::b_true;
               }
            }
            else
            {
               // did we complete trial-factoring?
               IPFSymbol *pLimit=pSubContext->LookupSymbol("_factoredto");
               if(pUs && pLimit && pLimit->GetSymbolType()==INTEGER_SYMBOL_TYPE)
               {
                  Integer T=*((PFIntegerSymbol*)pLimit)->GetValue();
                  T*=T;
                  if(*pN<T)
                  {
                     iReturnCode=2;    // prime by completion of trial factoring
                     PFPrintfLog("prime by trial factoring\n");
                     bProcessed=PFBoolean::b_true;
                     iReturnCode = PT_PRIME;
                  }
               }
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;
         }
      }

      if(!bProcessed && bRetval)
      {
         int iRetval=CallSubroutine("@issquare",pSubContext);
         if(iRetval!=-1)
         {
            if(iRetval==1)
            {
               iReturnCode=PT_COMPOSITE;
               PFPrintfLog("square\n");
               bProcessed=PFBoolean::b_true;
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;      // stop processing
         }
      }

      if(!bProcessed && bRetval)
      {
         // select a target for the test. Since (AG+1)(BG-1) yields
         // AB.G + B-A = QG+R = (N+1)/G

         // B-A = kG+R
         // AB  = Q-k

         // Note that A>0, B>0
         // and AB is at least abs(kG+R)+1

         // Aim for cuberoot N. We need to find cuberoot N.
         // Note that N>2^lg. So we round up.
         DWORD dwExponent=(numbits(*pN)+3)/3;
         Integer *pT=new Integer(1);
         (*pT)<<=dwExponent;
         pSubContext->AddSymbol(new PFIntegerSymbol("_TARGET",pT));

         PFFactorizationSymbol *pFactor2=new PFFactorizationSymbol("_NPLUS1FACTOR2");
         pSubContext->AddSymbol(pFactor2);

         PFString sActive  ="_NPLUS1FACTOR";
         PFString sActive2 ="_NPLUS1FACTOR2";

         int iRetval=PT_INCONCLUSIVE;

         PFBoolean bLooping=PFBoolean::b_true;  // while we need or are still begging
         Integer iProven(1);                 // what's been proved so far.

         // an unusual curio. For the N+1 test the discriminant _D and a non-residue
         // _B+sqrt(D) get set
         uint32 d=pSampler->ask(*pN);
         do
         {
            d=pSampler->askagain();
         }
         while(kro((int)d,*pN)!=-1);
         pSubContext->AddSymbol(new PFIntegerSymbol("_D",new Integer((int)d)));

         // now to find a suitable base
         pSubContext->AddSymbol(new PFIntegerSymbol("_B",new Integer(0)));

         while(bLooping)
         {
            pSubContext->AddSymbol(new PFStringSymbol("_FACTORTABLE",sActive));
            pSubContext->AddSymbol(new PFStringSymbol("_UNPROVED",sActive2));

            // get sActive2
            PFFactorizationSymbol *pJunk=(PFFactorizationSymbol*)pSubContext->LookupSymbol(sActive2);
            PFList<FactorNode> *pList=pJunk->AccessList();
            pList->RemoveAll();

            // get B and increment it as needed
            IPFSymbol *psB=pSubContext->LookupSymbol("_B");
            Integer *pB=((PFIntegerSymbol*)psB)->GetValue();

            Integer X=(*pB)*(*pB);
            X-=d;

            do
            {
               X+=(*pB);
               (*pB)++;
               X+=(*pB);
            }
            while(kro(X,*pN)!=-1);

            g_sTestMode = "N+1: ";
            iRetval=CallSubroutine("nplus1",pSubContext);
            g_sTestMode = "";

            switch(iRetval)
            {
            case -1:
               bLooping=PFBoolean::b_false;     // breakpoint
               break;
            case PT_INCONCLUSIVE:
               // collate what's been proven
               {
                  PFIntegerSymbol *pI=(PFIntegerSymbol*)pSubContext->LookupSymbol("_PROVED");
                  iProven*=*(pI->GetValue());      // suck them in
                  if(*(pI->GetValue())>=*pT)
                  {
                     bLooping=PFBoolean::b_false;
                  }
                  else if(pList->GetSize()==0)
                  {
                     bLooping=PFBoolean::b_false;
                  }
                  else
                  {
                     // *pT, the target for conclusive proof needs to be modified
                     // so that next time, we don't get too many factors
                     Integer IP=*(pI->GetValue());
                     // divide it out of pT, again upround
                     (*pT)+=(IP-1);
                     (*pT)/=IP;
                  }
               }
               break;                        // may still need to run
            case PT_PRIME:
               bLooping=PFBoolean::b_false;     // done
               break;
            case PT_COMPOSITE:
               bLooping=PFBoolean::b_false;     // done
               break;
            case PT_FACTOR:
               bLooping=PFBoolean::b_false;     // done
               break;
            }

            // swap lists
            PFString t=sActive;
            sActive=sActive2;
            sActive2=t;
         }

         // if at this stage we're still inconclusive, call BLS
         if(iRetval==PT_INCONCLUSIVE)
         {
            Integer *G=new Integer(iProven);
            PFIntegerSymbol *pF=new PFIntegerSymbol("_F",new Integer(1));
            PFIntegerSymbol *pG=new PFIntegerSymbol("_G",G);
            pSubContext->AddSymbol(pF);
            pSubContext->AddSymbol(pG);

            PFPrintfLog("Calling Brillhart-Lehmer-Selfridge with factored part %.2f%%\n",double(100.0*numbits(*G))/double(numbits(*pN)));

            // the end call performs the Brillhart-Lehmer-Selfridge processing of the
            // Lenstra's Theorem result.
            g_sTestMode = "BLS: ";
            iRetval=CallSubroutine("@endplus1",pSubContext);
            g_sTestMode = "";
         }

         if(iRetval!=-1)
         {
            iReturnCode=iRetval;

            switch(iRetval)
            {
            case PT_INCONCLUSIVE:
               break;
            case PT_PRIME:
               break;
            case PT_COMPOSITE:
               break;
            case PT_FACTOR:
               {
                  // the known factor is written to the symbol _FACTOR
                  IPFSymbol *pFactorSymbol=pSubContext->LookupSymbol("_FACTOR");
                  if(pFactorSymbol && pFactorSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
                  {
                     PFString sValue=pFactorSymbol->GetStringValue();
                     PFPrintfLog("factor: %s\n",LPCTSTR(sValue));
                  }
                  else
                  {
                     iReturnCode=PT_COMPOSITE;
                     PFPrintfLog("composite\n");
                  }
               }
               break;
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;
         }
      }

      delete pSubContext;
      bProcessed=PFBoolean::b_true;
   }
   else
   {
      bProcessed=PFBoolean::b_false;
   }

   if(bProcessed)
   {
      pContext->AddSymbol(new PFIntegerSymbol("_result",new Integer(iReturnCode)));
   }
   else
   {
      pContext->EraseSymbol("_result");
   }

   return bRetval;
}

// The combined N-1 and N+1 test. This is a combination of both the above testing modes. Factoring
// is done on N-1, N+1 (and N if requested). Two Mihailescu trees are built for each test. If a
// single test alone is satisfactory, great stuff. If not, both tests are performed. One test is
// dominant. The key to the routine working is the non-dominant end test result being available to
// the other routine, which helps in the square search
T_Combined::T_Combined()
   : PFFunctionSymbol("combined")
{
}

DWORD T_Combined::MinimumArguments() const
{
   return 1;
}

DWORD T_Combined::MaximumArguments() const
{
   return 1;
}

DWORD T_Combined::GetArgumentType(DWORD /*dw*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString T_Combined::GetArgumentName(DWORD /*dw*/) const
{
   return "_N";
}

PFBoolean T_Combined::CallFunction(PFSymbolTable *pContext)
{
   // 0  - composite
   // 1  - prp
   // 2  - prime
   // 3  - something unusual
   // -1 - is no longer running

   // to proceed, the following subfunctions need to be installed
   // trivialtest (eliminates negative, zero, one, even) -1 0 1 2
   // factor (factors N-1 N N+1)
   // nplus1 (testing engine)
   PFBoolean bProcessed=PFBoolean::b_false;
   int iReturnCode=0;
   PFBoolean bRetval=PFBoolean::b_true;

   IPFSymbol *pSymbolN=pContext->LookupSymbol("_N");
   Integer *pN;
   PFFactorizationSymbol *pPlus=NULL;
   PFFactorizationSymbol *pMinus=NULL;
   PFFactorizationSymbol *pUs=NULL;
   PFFactorizationSymbol *pFactorization;

   // the trivial test is identical for all test cases
   if(pSymbolN && pSymbolN->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      pN=((PFIntegerSymbol*)pSymbolN)->GetValue();

      PFSymbolTable *pSubContext = new PFSymbolTable(pContext);
      PFSamplerSymbol *pSampler = new PFSamplerSymbol();

      pSubContext->AddSymbol(pSampler);

      if(bRetval && !bProcessed)
      {
         int iRetval=CallSubroutine("@trivial",pSubContext);
         if(iRetval!=-1)
         {
            // still running, but check the trivial return code
            switch(iRetval)
            {
            case TT_ZERO:
               PFPrintfLog("zero\n");
               iReturnCode=PT_FACTOR;
               bProcessed=PFBoolean::b_true;
               break;
            case TT_ONE:
               PFPrintfLog("one\n");
               iReturnCode=PT_FACTOR;
               bProcessed=PFBoolean::b_true;
               break;
            case TT_TWO:
               PFPrintfLog("two\n");
               iReturnCode=PT_PRIME;
               bProcessed=PFBoolean::b_true;
               break;
            case TT_FACTOR:
               {
               iReturnCode=PT_FACTOR;
               bProcessed=PFBoolean::b_true;
               // Was it factored prime? This is true if there is only one factor
               pFactorization=(PFFactorizationSymbol *)pSubContext->LookupSymbol ("_TRIVIALFACTOR");
               PFString sTrivial=pFactorization->GetStringValue();
               if (pFactorization->AccessList()->GetSize () == 1 && !strchr(LPCTSTR(sTrivial), '^'))
               {
                  PFPrintfLog("small number, factored prime!\n");
                  iReturnCode = PT_PRIME;
               }
               else
                  PFPrintfLog("small number, factored\n");
               break;
               }
            case TT_NEGATIVE:
               PFPrintfLog("negative\n");
               (*pN)*=-1;
               break;
            default:
               break;
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;
         }
      }

      if(bRetval && !bProcessed)
      {
         // set up a couple of symbols in the symbol table
         // factor both N-1 and N+1 simultaneously
         pPlus=new PFFactorizationSymbol("_NPLUS1FACTOR");
         pSubContext->AddSymbol(pPlus);
         pSubContext->AddSymbol(new PFStringSymbol("_SP","_NPLUS1FACTOR"));
         pMinus=new PFFactorizationSymbol("_NMINUS1FACTOR");
         pSubContext->AddSymbol(pMinus);
         pSubContext->AddSymbol(new PFStringSymbol("_SM","_NMINUS1FACTOR"));

         // to save trial factoring time, we should only attempt to factor N if we were
         // actually asked to.
         IPFSymbol *pFactorize=pSubContext->LookupSymbol("_FACTORIZE");
         if(pFactorize)
         {
            pUs=new PFFactorizationSymbol("_NFACTOR");
            pSubContext->AddSymbol(pUs);
            pSubContext->AddSymbol(new PFStringSymbol("_SN","_NFACTOR"));
         }

         g_sTestMode = "F: ";
         g_nIterationCnt *= 1000;
         int iRetval=CallSubroutine("@factor",pSubContext);
         g_nIterationCnt /= 1000;
         g_sTestMode = "";

         if(iRetval!=-1)
         {
            if(iRetval>0)
            {
               // the routine found a factor. However not finding a factor might be positive too
               PFString sFactor=pUs->GetStringValue();
               // did we complete trial-factoring?
               if (pUs->AccessList()->GetSize() == 1)
               {
                  // See if the factor WAS our number.  If so, it IS prime.  This can happen when the user selects
                  // -f along with the -t (or -tc -tm -tp)
                  Integer tmp;
                  tmp.atoI(sFactor);
                  if (*pN == tmp)
                  {
                     iReturnCode=2;    // prime by completion of trial factoring
                     PFPrintfLog("prime by trial factoring\n");
                     bProcessed=PFBoolean::b_true;
                     iReturnCode = PT_PRIME;
                  }
               }
               if (bProcessed==PFBoolean::b_false)
               {
                  PFPrintfClearCurLine();
                  PFPrintfLog("factors: %s\n",LPCTSTR(sFactor));
                  iReturnCode=PT_FACTOR;
                  bProcessed=PFBoolean::b_true;
               }
            }
            else
            {
               // did we complete trial-factoring?
               IPFSymbol *pLimit=pSubContext->LookupSymbol("_factoredto");
               if(pUs && pLimit && pLimit->GetSymbolType()==INTEGER_SYMBOL_TYPE)
               {
                  Integer T=*((PFIntegerSymbol*)pLimit)->GetValue();
                  T*=T;
                  if(*pN<T)
                  {
                     iReturnCode=2;    // prime by completion of trial factoring
                     PFPrintfLog("prime by trial factoring\n");
                     bProcessed=PFBoolean::b_true;
                     iReturnCode = PT_PRIME;
                  }
               }
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;
         }
      }

      // the squarefree test is identical as well
      if(!bProcessed && bRetval)
      {
         int iRetval=CallSubroutine("@issquare",pSubContext);
         if(iRetval!=-1)
         {
            if(iRetval==1)
            {
               iReturnCode=PT_COMPOSITE;
               PFPrintfLog("square\n");
               bProcessed=PFBoolean::b_true;
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;      // stop processing
         }
      }

      if(!bProcessed && bRetval)
      {
         // we have a tricky bit of evaluation to do here. Note we require F1^3.F2 to be larger than N.
         // F1 and F2 may be F or G, we don't know which. Though the largest proven part will be F1, that's
         // for certain.

         // this means for all practical purposes we should not target cuberoot(N) for both tests, we just
         // should do this for the larger test, and for the smaller just "take up the slack". However we
         // really ought not be doing -tc at all unless all these steps are going to be used.

         // I'll leave it as a future tweak for an automatic test mode that will decide N-1 or N+1 and make
         // the right choices. For the moment, target sqrt(N). Note this routine can always work if F1+F2>0.5

         // Now make this N^2-1, we want all the factors if we can :)
         Integer *pT=new Integer(*pN);
         (*pT)*=(*pT);
         (*pT)--;
         pSubContext->AddSymbol(new PFIntegerSymbol("_TARGET",pT));

         int iRetval=PT_INCONCLUSIVE;
         PFBoolean bLooping=PFBoolean::b_true;  // while we need or are still begging
         Integer fProven(1);                 // what's been proved so far for N-1
         Integer gProven(1);                 // what's been proved so far for N+1

         // do the N-1 tests first. They are faster.
         PFFactorizationSymbol *pFactor2=new PFFactorizationSymbol("_NMINUS1FACTOR2");

         pSubContext->AddSymbol(pFactor2);
         PFString sActive  ="_NMINUS1FACTOR";
         PFString sActive2 ="_NMINUS1FACTOR2";

         while(bLooping)
         {
            pSubContext->AddSymbol(new PFStringSymbol("_FACTORTABLE",sActive));
            pSubContext->AddSymbol(new PFStringSymbol("_UNPROVED",sActive2));

            // get sActive2
            PFFactorizationSymbol *pJunk=(PFFactorizationSymbol*)pSubContext->LookupSymbol(sActive2);
            PFList<FactorNode> *pList=pJunk->AccessList();
            pList->RemoveAll();

            g_sTestMode = "N-1: ";
            iRetval=CallSubroutine("nminus1",pSubContext);
            g_sTestMode = "";

            switch(iRetval)
            {
            case -1:
               bLooping=PFBoolean::b_false;     // breakpoint
               break;
            case PT_INCONCLUSIVE:
               // collate what's been proven
               {
                  PFIntegerSymbol *pI=(PFIntegerSymbol*)pSubContext->LookupSymbol("_PROVED");
                  fProven*=*(pI->GetValue());      // suck them in
                  if(*(pI->GetValue())>=*pT)
                  {
                     bLooping=PFBoolean::b_false;
                  }
                  else if(pList->GetSize()==0)
                  {
                     bLooping=PFBoolean::b_false;
                  }
                  else
                  {
                     // *pT, the target for conclusive proof needs to be modified
                     // so that next time, we don't get too many factors
                     Integer IP=*(pI->GetValue());
                     // divide it out of pT, again upround
                     (*pT)+=(IP-1);
                     (*pT)/=IP;
                  }
               }
               break;                        // may still need to run
            case PT_PRIME:
               bLooping=PFBoolean::b_false;     // done
               break;
            case PT_COMPOSITE:
               bLooping=PFBoolean::b_false;     // done
               break;
            case PT_FACTOR:
               bLooping=PFBoolean::b_false;     // done
               break;
            }

            // swap lists
            PFString t=sActive;
            sActive=sActive2;
            sActive2=t;
         }

         // if we are still inconclusive, keep it going with the N+1 test
         if(iRetval==PT_INCONCLUSIVE)
         {
            // execute the N+1 test until we've squeezed what we can out of it
            pFactor2=new PFFactorizationSymbol("_NPLUS1FACTOR2");
            pSubContext->AddSymbol(pFactor2);
            sActive  ="_NPLUS1FACTOR";
            sActive2 ="_NPLUS1FACTOR2";

            // an unusual curio. For the N+1 test the discriminant _D and a non-residue
            // _B+sqrt(D) get set
            uint32 d=pSampler->ask(*pN);
            do
            {
               d=pSampler->askagain();
            }
            while(kro((int)d,*pN)!=-1);
            pSubContext->AddSymbol(new PFIntegerSymbol("_D",new Integer((int)d)));

            // now to find a suitable base
            pSubContext->AddSymbol(new PFIntegerSymbol("_B",new Integer(0)));
            bLooping=PFBoolean::b_true;
            while(bLooping)
            {
               pSubContext->AddSymbol(new PFStringSymbol("_FACTORTABLE",sActive));
               pSubContext->AddSymbol(new PFStringSymbol("_UNPROVED",sActive2));

               // get sActive2
               PFFactorizationSymbol *pJunk=(PFFactorizationSymbol*)pSubContext->LookupSymbol(sActive2);
               PFList<FactorNode> *pList=pJunk->AccessList();
               pList->RemoveAll();

               // get B and increment it as needed
               IPFSymbol *psB=pSubContext->LookupSymbol("_B");
               Integer *pB=((PFIntegerSymbol*)psB)->GetValue();

               Integer X=(*pB)*(*pB);
               X-=d;

               do
               {
                  X+=(*pB);
                  (*pB)++;
                  X+=(*pB);
               }
               while(kro(X,*pN)!=-1);

               g_sTestMode = "N+1: ";
               iRetval=CallSubroutine("nplus1",pSubContext);
               g_sTestMode = "";

               switch(iRetval)
               {
               case -1:
                  bLooping=PFBoolean::b_false;     // breakpoint
                  break;
               case PT_INCONCLUSIVE:
                  // collate what's been proven
                  {
                     PFIntegerSymbol *pI=(PFIntegerSymbol*)pSubContext->LookupSymbol("_PROVED");
                     gProven*=*(pI->GetValue());      // suck them in
                     if(*(pI->GetValue())>=*pT)
                     {
                        bLooping=PFBoolean::b_false;
                     }
                     else if(pList->GetSize()==0)
                     {
                        bLooping=PFBoolean::b_false;
                     }
                     else
                     {
                        // *pT, the target for conclusive proof needs to be modified
                        // so that next time, we don't get too many factors
                        Integer IP=*(pI->GetValue());
                        // divide it out of pT, again upround
                        (*pT)+=(IP-1);
                        (*pT)/=IP;
                     }
                  }
                  break;                        // may still need to run
               case PT_PRIME:
                  bLooping=PFBoolean::b_false;     // done
                  break;
               case PT_COMPOSITE:
                  bLooping=PFBoolean::b_false;     // done
                  break;
               case PT_FACTOR:
                  bLooping=PFBoolean::b_false;     // done
                  break;
               }

               // swap lists
               PFString t=sActive;
               sActive=sActive2;
               sActive2=t;
            }
         }

         // if at this stage we're still inconclusive, call BLS. However this time we have some
         // extra tricks to pull. Choose which test is needed

         if(iRetval==PT_INCONCLUSIVE)
         {
            Integer *F=new Integer(fProven);
            Integer *G=new Integer(gProven);
            PFIntegerSymbol *pF=new PFIntegerSymbol("_F",F);
            PFIntegerSymbol *pG=new PFIntegerSymbol("_G",G);
            pSubContext->AddSymbol(pF);
            pSubContext->AddSymbol(pG);

            // the total value of the test is big^3.small
            Integer tv(fProven);
            tv*=gProven;

            PFString sPolisher;

            if(fProven>gProven)
            {
               tv*=fProven;
               tv*=fProven;
               sPolisher="@endminus1";
               PFPrintfLog("Calling N-1 BLS with factored part %.2f%% and helper %.2f%% (%.2f%% proof)\n",
                  double(100.0*numbits(*F))/double(numbits(*pN)),double(100.0*numbits(*G))/double(numbits(*pN)),
                  double(100.0*numbits(tv))/double(numbits(*pN)));
            }
            else
            {
               tv*=gProven;
               tv*=gProven;
               sPolisher="@endplus1";
               PFPrintfLog("Calling N+1 BLS with factored part %.2f%% and helper %.2f%% (%.2f%% proof)\n",
                  double(100.0*numbits(*G))/double(numbits(*pN)),double(100.0*numbits(*F))/double(numbits(*pN)),
                  double(100.0*numbits(tv))/double(numbits(*pN)));
            }


            // the end call performs the Brillhart-Lehmer-Selfridge processing of the
            // Lenstra's Theorem result.
            g_sTestMode = "BLS: ";
            iRetval=CallSubroutine(sPolisher,pSubContext);
            g_sTestMode = "";
         }

         if(iRetval!=-1)
         {
            iReturnCode=iRetval;

            switch(iRetval)
            {
            case PT_INCONCLUSIVE:
               break;
            case PT_PRIME:
               break;
            case PT_COMPOSITE:
               break;
            case PT_FACTOR:
               {
                  // the known factor is written to the symbol _FACTOR
                  IPFSymbol *pFactorSymbol=pSubContext->LookupSymbol("_FACTOR");
                  if(pFactorSymbol && pFactorSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
                  {
                     PFString sValue=pFactorSymbol->GetStringValue();
                     PFPrintfLog("factor: %s\n",LPCTSTR(sValue));
                  }
                  else
                  {
                     iReturnCode=PT_COMPOSITE;
                     PFPrintfLog("composite\n");
                  }
               }
               break;
            }
         }
         else
         {
            bRetval=PFBoolean::b_false;
         }
      }

      delete pSubContext;
      bProcessed=PFBoolean::b_true;
   }
   else
   {
      bProcessed=PFBoolean::b_false;
   }

   if(bProcessed)
   {
      pContext->AddSymbol(new PFIntegerSymbol("_result",new Integer(iReturnCode)));
   }
   else
   {
      pContext->EraseSymbol("_result");
   }

   return bRetval;
}
