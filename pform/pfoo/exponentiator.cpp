#include "pfoopch.h"
#include "exponentiator.h"

#include "factornode.h"
#include "factorarray.h"
#include "symboltypes.h"
#include "pfstringsymbol.h"

#include "pfintegersymbol.h"
#include "pffactorizationsymbol.h"
#include "pfgw_globals.h"

#undef GWDEBUG
#undef INTDEBUG
#define GWDEBUG(X) {Integer XX;XX=X;printf(#X "=");mpz_out_str(stdout,16,XX.gmp();printf("\n");}
#define INTDEBUG(X) {printf(#X "=");mpz_out_str(stdout,16,(X).gmp();printf("\n");}

//========================================
// Exponentiator constructor and destructor
//========================================
Exponentiator::Exponentiator(const PFString &sName)
    :   PFIterativeSymbol(sName),
    pResidue(NULL), pMultiplier(NULL), pField(NULL),
    exponent(0), bitindex(0), pDestination(NULL),
    itemStack(PFBoolean::b_true),
    m_N(0), m_F(0), m_bTargetMade(PFBoolean::b_false), m_pTree(NULL),
    m_pSource(NULL), m_pJunkyard(NULL), m_X(0), m_pContext(NULL),
    m_PROVED(1)
{
}

Exponentiator::~Exponentiator()
{
}

PFBoolean Exponentiator::OnExecute(PFSymbolTable *pContext)
{
   m_pContext=pContext;
   return PFBoolean::b_true;
}

PFBoolean Exponentiator::OnCompleted(PFSymbolTable *pContext)
{
   // write m_PROVED to the symbol table as proved
   pContext->AddSymbol(new PFIntegerSymbol("_PROVED",new Integer(m_PROVED)));
   return PFBoolean::b_true;
}

PFBoolean Exponentiator::OnCleanup(PFSymbolTable * /*pContext*/)
{
   // delete everything
   if(pResidue)
   {
      delete pResidue;
      pResidue=NULL;
   }
   if(pMultiplier)
   {
      delete pMultiplier;
      pMultiplier=NULL;
   }
   if(pField)
   {
      delete pField;
      pField=NULL;
   }
   if(m_pTree)
   {
      delete m_pTree;
      m_pTree=NULL;
   }
   return PFBoolean::b_true;
}

PFBoolean Exponentiator::OnInitialize()
{
   PFSymbolTable *pContext=m_pContext;

   // create the tree. Note the factortable name is supplied in _FACTORTABLE
   // and the required integer is supplied in _TARGET
   PFBoolean bRetval=PFBoolean::b_false;

   IPFSymbol *pSymbol;
   pSymbol=pContext->LookupSymbol("_N");         // the integer we are trying to prove
   if(pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      m_N=*((PFIntegerSymbol*)pSymbol)->GetValue();      // copy the value
      Integer T=squareroot(m_N);                        // this is always good enough

      // note you may as well perfect square test
      if(T*T!=m_N)
      {
         pSymbol=pContext->LookupSymbol("_TARGET");
         if(pSymbol && pSymbol->GetSymbolType()==INTEGER_SYMBOL_TYPE)
         {
            Integer *pT=((PFIntegerSymbol*)pSymbol)->GetValue();
            if(*pT<T)
            {
               T=*pT;
            }
         }

         // now dig out the factor table
         pSymbol=pContext->LookupSymbol("_FACTORTABLE");
         if(pSymbol && pSymbol->GetSymbolType()==STRING_SYMBOL_TYPE)
         {
            PFString sTable=pSymbol->GetStringValue();
            pSymbol=pContext->LookupSymbol(sTable);
            if(pSymbol && pSymbol->GetSymbolType()==FACTORIZATION_SYMBOL_TYPE)
            {
               m_pSource=(PFFactorizationSymbol*)pSymbol;
               m_pJunkyard=NULL;
               pSymbol=pContext->LookupSymbol("_UNPROVED");      // necessary to repeat
               if(pSymbol && pSymbol->GetSymbolType()==STRING_SYMBOL_TYPE)
               {
                  PFString sJunkyard=pSymbol->GetStringValue();
                  pSymbol=pContext->LookupSymbol(sJunkyard);
                  if(pSymbol && pSymbol->GetSymbolType()==FACTORIZATION_SYMBOL_TYPE)
                  {
                     m_pJunkyard=(PFFactorizationSymbol*)pSymbol;
                  }
               }

               PFList<FactorNode> *pResult;
               m_bTargetMade=AimForTarget(m_pSource,T,pResult,m_F);
               m_pTree=BuildMihailescuTree(pResult);

               delete pResult;
               m_PROVED=1;

               // we are now ready to go
               bRetval=GetTotalExponentiation(pContext,m_X);

               if(bRetval)
               {
                  // if factor limits have been set, m_pTree may be NULL (thanks Jim)
                  if(m_pTree!=NULL)
                  {
                     exponent=m_X/m_pTree->prime();
                     m_dwStepsTotal=m_pTree->powerDepth();
                     if(m_X!=exponent*m_pTree->prime())
                     {
                        testResult=-1;            // big poopy doo.
                        bRetval=PFBoolean::b_false;   // erm, whoops. This should NEVER EVER HAPPEN
                     }
                     else
                     {
                     }
                  }
                  else
                  {
                     exponent=m_X;
                     m_dwStepsTotal=0;
                  }

                  pDestination=m_pTree;      // where we are aiming for
                  bitindex=lg(exponent);     // this is how many bits are in the count
                  m_dwStepsTotal+=(bitindex+1);
                  pField=GetFieldElements(pContext,pResidue,pMultiplier);
                  if(pField==NULL)
                  {
                     bRetval=PFBoolean::b_false;
                  }
                  else
                  {
                        m_dwStepGranularity=4096;     // for starters
                     testResult=0;              // totally unknown
                  }
               }
            }
         }
      }
   }
   return bRetval;
}

// state is given by
// bitindex loop variable
// exponentiation
// current multiplier
// current residue
// destination node
// on arrival at a destination node one either launches a leaf node test or
// creates a recursion

PFBoolean Exponentiator::Iterate()
{
   PFBoolean bExit=PFBoolean::b_false;
   char buffer[200];

// lg(1)=0 lg(2,3)=1, lg(4-7)=2....
   if(bitindex==0)
   {   // the current calculation is finished, we have arrived at our destination

      // callouts depend if we are at a leaf or not
      // so does the next object to process
      if(pDestination==NULL)
      {
            // arrival at ultimate destination
            bExit=testFinal();
            delete pResidue;
            delete pMultiplier;
       pResidue=NULL;
       pMultiplier=NULL;

            // if there is an item on the stack, then switch to
            // it. If not, then we are done
            if(!bExit)
            {

                if(itemStack.GetSize())
                {
                    Exponentiation *pEx=itemStack.Pop();

                    pMultiplier=pEx->multiplier();
                    pResidue=pEx->residue();
                    exponent=pEx->exponent();
                    pDestination=pEx->destination();
                    bitindex=lg(exponent);

                    delete pEx;
                }
                else bExit=PFBoolean::b_true;
            }
      }
      else if(pDestination->isLeaf())
      {
       bExit=testLeaf();
       delete pMultiplier;
       pMultiplier=NULL;

       if(!bExit)
            {
                // residue continues, but the multiplier changes

                pMultiplier=pResidue->duplicateAsMultiplier();

                exponent=pDestination->prime();
                pDestination=NULL;
                bitindex=lg(exponent);
            }
            else
         {
            delete pResidue;
            pResidue=NULL;
         }
      }
      else
      {
       bExit=testInternal();
       delete pMultiplier;
       pMultiplier=NULL;

       if(!bExit)
            {
                // stack a recursion down the right, and continue
                // down the left
                Exponentiation *pEx=new Exponentiation;
                pEx->setResidue(pResidue);
                pEx->setMultiplier(pResidue);
                pEx->setExponent(pDestination->child1->prime());
                pEx->setDestination(pDestination->child2);
                itemStack.Push(pEx);


                pMultiplier=pResidue->duplicateAsMultiplier();

                exponent=pDestination->child2->prime();
                pDestination=pDestination->child1;
                bitindex=lg(exponent);
            }
            else
         {
            delete pResidue;
            pResidue=NULL;
         }
      }
   }
   else
   {
      bitindex--;
      if (bit(exponent,bitindex))
         pResidue->squaremultiply(pMultiplier, m_dwStepsTotal, bitindex);
      else
         pResidue->square(m_dwStepsTotal, bitindex);

      if (gw_test_illegal_sumout (&gwdata))
      {
         sprintf(buffer, "Detected SUMOUT error in exponentiator.cpp");
         PFOutput::EnableOneLineForceScreenOutput();
         PFOutput::OutputToErrorFileAlso(buffer, g_cpTestString, lg(exponent)-bitindex, lg(exponent));
         PFPrintfStderr("ERROR, ILLEGAL SUMOUT near Iteration %d/%d\n", lg(exponent)-bitindex, lg(exponent));
         bExit = PFBoolean::b_true;
      }
      if (!bExit && gw_get_maxerr(&gwdata) > g_dMaxErrorAllowed)
      {
         sprintf(buffer, "Detected MAXERR>%.2f (round off check) in exponentiator.cpp", g_dMaxErrorAllowed);
         PFOutput::EnableOneLineForceScreenOutput();
         PFOutput::OutputToErrorFileAlso(buffer, g_cpTestString, lg(exponent)-bitindex, lg(exponent));
         PFPrintfStderr("ERROR, roundoff was (%.10g)  near Iteration %d/%d\n", gw_get_maxerr(&gwdata), lg(exponent)-bitindex, lg(exponent));
         bExit = PFBoolean::b_true;
      }
   }

    // if a premature exit was called, clear the stack
    if(bExit)
    {
        while(itemStack.GetSize())
        {
            Exponentiation *pEx=itemStack.Pop();
            delete pEx;
        }
    }

   return(bExit);      // no premature exit
}

PFBoolean Exponentiator::AimForTarget(PFFactorizationSymbol *pSymbol,const Integer &T,
                  PFList<FactorNode> *&pResult,Integer &R)
{
   // quite an easy algorithm
   PFList<FactorNode> *pSource=pSymbol->AccessList();
   FactorArray fa;

   PFForwardIterator pffi;
   pSource->StartIterator(pffi);
   PFListNode *pNode;

   while(pffi.Iterate(pNode))
   {
      FactorNode *pFactor=(FactorNode*)pNode->GetData();
      fa.add(pFactor,PFBoolean::b_true);   // include the power
   }

   // now take the nodes off
   pResult=new PFList<FactorNode>(PFBoolean::b_false);

   FactorNode *pRoot = 0;
   R=1;

   while((R<T)&&(NULL!=(pRoot=fa.remove(PFBoolean::b_true))))
   {
      R*=pRoot->power();
      pResult->AddTail(pRoot);
   }

   // unused factors go into the junkyard
   if(m_pJunkyard)
   {
      while(NULL!=(pRoot=fa.remove(PFBoolean::b_true)))
      {
         m_pJunkyard->AddFactor(new FactorNode(*pRoot));
      }
   }

   // stop as soon as we reach T
   return((R<T)?PFBoolean::b_false:PFBoolean::b_true);
}

FactorNode *Exponentiator::BuildMihailescuTree(PFList<FactorNode> *pSource)
{
   FactorArray fa;

   PFForwardIterator pffi;
   pSource->StartIterator(pffi);
   PFListNode *pNode;

   while(pffi.Iterate(pNode))
   {
      FactorNode *pFactor=(FactorNode*)pNode->GetData();
      FactorNode *pClone=new FactorNode(*pFactor);
      fa.add(pClone,PFBoolean::b_false);   // include the power
   }

   while(fa.heapsize()>1)
   {
      // pull of two, make a new one
      FactorNode *p1=fa.remove(PFBoolean::b_false);
      FactorNode *p2=fa.remove(PFBoolean::b_false);
      FactorNode *p3=new FactorNode(p1,p2);
      fa.add(p3,PFBoolean::b_false);
   }

   FactorNode *pRetval=fa.remove(PFBoolean::b_false);

   return pRetval;
}

FactorNode *Exponentiator::FindSourceNode(FactorNode *pFactor)
{
   FactorNode *pRetval=NULL;

   PFList<FactorNode> *pOriginal=m_pSource->AccessList();
   PFForwardIterator pffi;
   pOriginal->StartIterator(pffi);
   PFListNode *pNode;

   while(pffi.Iterate(pNode))
   {
      FactorNode *pOldNode=(FactorNode*)pNode->GetData();
      if(pOldNode->prime()==pFactor->prime())
      {
         pRetval=pOldNode;
         break;
      }
   }

   return pRetval;
}

void Exponentiator::AddToJunkyard(FactorNode *pFactor)
{
   // pFactor is junk, it contains a prime value that is in the source
   // but didn't work this time. So you need to add it to the junkyard.

   if(m_pJunkyard)
   {
      FactorNode *pRetval=FindSourceNode(pFactor);
      if(pRetval)
      {
         m_pJunkyard->AddFactor(new FactorNode(*pRetval));
      }
   }
}

void Exponentiator::AddToResults(FactorNode *pFactor)
{
   FactorNode *pRetval=FindSourceNode(pFactor);
   if(pRetval)
   {
      m_PROVED*=pRetval->power();
   }
}
