#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "pfoopch.h"
#include "f_sequence.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "h_helpers.h"

//#define INTDEBUG(X) {printf(#X "=");mpz_out_str(stdout,16,*(X).gmp());printf("\n");}
#undef INTDEBUG
#define INTDEBUG(X)

F_Sequence::F_Sequence()
   : PFFunctionSymbol("Linear")
{
}

DWORD F_Sequence::MinimumArguments() const
{
   return 5;
}

DWORD F_Sequence::MaximumArguments() const
{
   return 5;
}

DWORD F_Sequence::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

static LPCTSTR symNames[]={"_F0","_F1","_F2","_F3","_N"};
#define SYMBOL_N 4

PFString F_Sequence::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval="";

   if(dwIndex<5)
   {
      sRetval=symNames[dwIndex];
   }
   return sRetval;
}

PFBoolean F_Sequence::LinearSolve(  const Integer &A,const Integer &B,const Integer &P,
                           const Integer &C,const Integer &D,const Integer &Q,
                           Integer &X,Integer &Y)
{
   // Find, if one exists, a unique integer solution to
   // AX+BY=P
   // CX+DY=Q

   // Simple matrix algebra, the inverse is
   //       D  -B
   // 1/DELTA  -C A

   // compute DELTA
   Integer DELTA(A*D-B*C);
   if(DELTA==0) return PFBoolean::b_false;

   // get solutions except for determinant delta
   X= D*P-B*Q;
   Y= A*Q-P*C;

   if((X%DELTA!=0)||(Y%DELTA!=0)) return PFBoolean::b_false;

   X/=DELTA;
   Y/=DELTA;

   return PFBoolean::b_true;
}

PFBoolean F_Sequence::CallFunction(PFSymbolTable *pContext)
{
   // read in the symbols _F0 to _F3 and _N
   Integer *N,*F[4];
   PFBoolean bRetval=PFBoolean::b_false;

   N=NULL;
   IPFSymbol *psym=pContext->LookupSymbol(symNames[SYMBOL_N]);
   if(psym && psym->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      N=((PFIntegerSymbol*)psym)->GetValue();
   }

   if(N)
   {
      if(*N<0)
      {
         return PFBoolean::b_false;             // no retro
      }

      int nn=(*N)&0x7FFFFFFF;
      if((*N)!=nn)
      {
         return PFBoolean::b_false;             // get real!
      }

      bRetval=PFBoolean::b_true;
      for(int i=0;bRetval&&(i<4);i++)
      {
         psym=pContext->LookupSymbol(symNames[i]);
         if(psym && psym->GetSymbolType()==INTEGER_SYMBOL_TYPE)
         {
            F[i]=((PFIntegerSymbol*)psym)->GetValue();
         }
         else
         {
            bRetval=PFBoolean::b_false;
         }
      }

      if(*N<4)
      {
         Integer *r = new Integer(*F[(*N&0x3)]);      // just return an input
         pContext->AddSymbol(new PFIntegerSymbol("_result",r));
         return PFBoolean::b_true;
      }

      INTDEBUG(*F[0]);
      INTDEBUG(*F[1]);
      INTDEBUG(*F[2]);
      INTDEBUG(*F[3]);
      INTDEBUG(*N);

      Integer B,C;
      Integer D;

      if(bRetval)
      {
         // All parameters read in. Step 1 is to find the defining quadratic
         // X^2+BX+C, in other words solve for B and C in

         // B.F1+C.F0 = -F2
         // B.F2+C.F1 = -F3
         bRetval=LinearSolve(*F[1],*F[0],((*F[2])*-1),*F[2],*F[1],((*F[3])*-1),B,C);
         INTDEBUG(B);
         INTDEBUG(C);
      }

      if(bRetval)
      {
         // what we're really looking at is a Lucas sequence with p=-B and q=C.
         // So tell our primality tester this very important fact (In other words,
         // 'quit bothering me with this nonstandard syntax!' Note lg(N) is the
         // power of 2 less than N
         int iLargest=(lg(B)>lg(C))?(lg(B)):(lg(C));
         iLargest++;
         // we want ceil(iLargest * log2(10)). For a rough estimate, take log2(10) as
         // 1/3
         int iDigits=((iLargest+2)/3);    // almost always less needed than this.

         // Allow a couple extra for sign, terminator, and in case I messed up
         // The library does export Itoa, but I don't trust it, since it'll use
         // malloc and not new.  (the lib now uses new, but I have kept this code)
         TCHAR *b=new char[iDigits+4];

         PFString sMessage="Linear() -> Lucas P=";
         mpz_get_str(b,10,(-B).gmp());
         sMessage+=b;
         sMessage+=",Q=";
         mpz_get_str(b,10,C.gmp());
         sMessage+=b;
         sMessage+="\n";
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr(sMessage);
         delete[] b;

         // Integer solutions B and C do indeed exist. Compute the discriminant
         D=B*B-C*4;
         INTDEBUG(D);
         if(D==0)
         {
            // special case, solution is (P+Qn).(-B/2)^n.
            // note B must be even
            // P=_F0, and P+Q = _F1/(-B/2).
            // Let BB=-B/2, Note P is an integer, but Q isn't necessarily,
            // so these aren't totally dull, just divisible by BB^(n-1).
            // Hmmm, maybe they are totally dull

            Integer BB=B/-2;
            Integer P= *F[0];
            P*=BB;
            Integer QBB= *F[1] - P;

            // get BB^(n-1)
            Integer BBNM1=pow(BB,nn-1);

             Integer *r = new Integer((P+QBB*nn)*BBNM1);
            pContext->AddSymbol(new PFIntegerSymbol("_result",r));
         }
         else
         {
            // general case, solution is P a^n + Q b^n
            // where a=(-B+sqrt(D))/2 and b=(-B-sqrt(D))/2


            if(B&1)
            {
               // odd discriminant case, use half integer code
               // so we consider integers of the form (X + Y sqrt(D))/2
               // for X and Y integers of the same parity if D is odd
               B*=-1;
               // Now a=(B+sqrt(D))/2
               // Let a^n = (X_0 + Y_0*sqrt(D))/2
               // giving the primitive solutions
               // X {2, B .....}
               // Y {0, 1 .....}
               // and a general solution PX+QY, BUT BEWARE, P and Q may be
               // HALFINTEGERS, so work with P2 and Q2

               // note when primitive parts are required we have
               // X(n)=Y(2n)/Y(n) so PP X(n) = PP Y(2n)
               Integer P2(*F[0]);
               Integer Q2(*F[1]*2-P2*B);

               INTDEBUG(P2);
               INTDEBUG(Q2);

               // Recurrences
               // X_2N = (X_N*X_N+ D*Y_N*Y_N)/2
               // Y_2N = X_N*Y_N

               // X_N+1 = (B*X_N + D*Y_N)/2
               // Y_N+1 = (X_N + B*Y_N)/2
               Integer X(B);
               Integer Y(1);
               Integer XX;

               int iMask=1;
               while((iMask<<1)<=nn) iMask<<=1;    // top bit

               for(;iMask>>=1;)
               {
                  // squaring rule
                  XX=(X*X+D*Y*Y)/2;
                  Y*=X;
                  if(nn&iMask)
                  {
                     // addition rule
                     X=(B*XX+D*Y)/2;
                     Y*=B;
                     Y+=XX;
                     Y/=2;
                  }
                  else
                  {
                     X=XX;
                  }
               }

               Integer *r = new Integer((P2*X+Q2*Y)/2);
               pContext->AddSymbol(new PFIntegerSymbol("_result",r));
            }
            else
            {
               // even discriminant case, in theory the easiest
               B/=-2;
               D/=4;
               // now a=(B+sqrt(D)) and b=(B-sqrt(D))
               // Let a^n = X_0 + Y_0 * sqrt(D)
               // giving the primitive solutions
               // X {1, B, .....}
               // Y {0, 1, .....}
               // and a general solution PX+QY.
               // Note it doesn't matter one iota if D is a square.

               // X(n) = Y(2n)/2Y(n) so PP X(n) = PP Y(2n)

               Integer P(*F[0]);
               Integer Q(*F[1]-P*B);      // PX_N+QY_N needed

               // Recurrences
               // X_2N = X_N*X_N+D*Y_N*Y_N
               // Y_2N = 2*X_N*Y_N

               // X_N+1 = B*X_N + D*Y_N
               // Y_N+1 =   X_N + B*Y_N
               Integer X(B);
               Integer Y(1);     // X_1,Y_1
               Integer XX;

               int iMask=1;
               while((iMask<<1)<=nn) iMask<<=1;    // top bit

               for(;iMask>>=1;)
               {
                  // squaring rule
                  XX=X*X+D*Y*Y;
                  Y*=X;
                  Y*=2;
                  if(nn&iMask)
                  {
                     // addition rule
                     X=B*XX+D*Y;
                     Y*=B;
                     Y+=XX;
                  }
                  else
                  {
                     X=XX;
                  }
               }

               Integer *r = new Integer(P*X+Q*Y);
               pContext->AddSymbol(new PFIntegerSymbol("_result",r));
            }
         }
      }
   }
   return bRetval;
}

// Now the Lucas sequences defined in a consistent manner
PFBoolean lucasEval(PFSymbolTable *pTable,int n,Integer &V,Integer &U)
{
   PFBoolean bRetval=PFBoolean::b_false;

   IPFSymbol *psP=pTable->LookupSymbol("_P");
   IPFSymbol *psQ=pTable->LookupSymbol("_Q");

   if(   psP && (psP->GetSymbolType()==INTEGER_SYMBOL_TYPE) &&
      psQ && (psQ->GetSymbolType()==INTEGER_SYMBOL_TYPE))
   {
      if(n>=0)
      {
         bRetval=PFBoolean::b_true;
         Integer *pP=((PFIntegerSymbol*)psP)->GetValue();
         Integer *pQ=((PFIntegerSymbol*)psQ)->GetValue();

         if(n==0)
         {
            V=2;
            U=0;
         }
         else if(n==1)
         {
            V=*pP;
            U=1;
         }
         else
         {
            // exponentiate (P+sqrt(D))/2
            Integer B=*pP;
            Integer D=B*B-4*(*pQ);

            // initialize to \alpha^1 = (V+U.sqrt(D))/2
            V=*pP;
            U=1;
            Integer VV;

            int iMask=1;
            while((iMask<<1)<=n) iMask<<=1;     // top bit

            for(;iMask>>=1;)
            {
               // squaring rule
               VV=(V*V+D*U*U)/2;
               U*=V;
               if(n&iMask)
               {
                  // addition rule
                  V=(B*VV+D*U)/2;
                  U*=B;
                  U+=VV;
                  U/=2;
               }
               else
               {
                  V=VV;
               }
            }
         }
      }
   }
   return bRetval;
}

PFBoolean lucasHelper(PFSymbolTable *pTable,int n,Integer &U)
{
   Integer V;
   return lucasEval(pTable,n,V,U);
}

F_LucasType::F_LucasType(const PFString &s)
   : PFFunctionSymbol(s)
{
}

DWORD F_LucasType::MinimumArguments() const
{
   return 3;
}

DWORD F_LucasType::MaximumArguments() const
{
   return 3;
}

DWORD F_LucasType::GetArgumentType(DWORD /* dwIndex */) const
{
   return INTEGER_SYMBOL_TYPE;
}

static LPCTSTR lucasSyms[]={"_P","_Q","_N"};

PFString F_LucasType::GetArgumentName(DWORD dwIndex) const
{
   PFString sRetval="";
   if(dwIndex<3) sRetval=lucasSyms[dwIndex];
   return sRetval;
}

F_LucasV::F_LucasV() : F_LucasType("lucasV")
{
}

PFBoolean F_LucasV::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pN=pContext->LookupSymbol("_N");
   if(pN && (pN->GetSymbolType()==INTEGER_SYMBOL_TYPE))
   {
      Integer *N=((PFIntegerSymbol*)pN)->GetValue();
      int n=(*N)&0x3FFFFFFF;
      if(n==(*N))
      {
         // the function can be called, note lucasEval will catch
         // missing _P and _Q definitions
         Integer V,U;
         bRetval=lucasEval(pContext,n,V,U);
         if(bRetval)
         {
            Integer *r = new Integer(V);
            pContext->AddSymbol(new PFIntegerSymbol("_result",r));
         }
      }
   }
   return bRetval;
}

F_LucasU::F_LucasU() : F_LucasType("lucasU")
{
}

PFBoolean F_LucasU::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pN=pContext->LookupSymbol("_N");
   if(pN && (pN->GetSymbolType()==INTEGER_SYMBOL_TYPE))
   {
      Integer *N=((PFIntegerSymbol*)pN)->GetValue();
      int n=(*N)&0x3FFFFFFF;
      if(n==(*N))
      {
         // the function can be called, note lucasEval will catch
         // missing _P and _Q definitions
         Integer V,U;
         bRetval=lucasEval(pContext,n,V,U);
         if(bRetval)
         {
            Integer *r = new Integer(U);
            pContext->AddSymbol(new PFIntegerSymbol("_result",r));
         }
      }
   }
   return bRetval;
}

F_PrimV::F_PrimV() : F_LucasType("primV")
{
}

PFBoolean F_PrimV::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pN=pContext->LookupSymbol("_N");
   if(pN && (pN->GetSymbolType()==INTEGER_SYMBOL_TYPE))
   {
      Integer *N=((PFIntegerSymbol*)pN)->GetValue();
      int n=(*N)&0x3FFFFFFF;
      if(n==(*N))
      {
         // the function can be called, note lucasEval will catch
         // missing _P and _Q definitions. However for V we use primU(2n)
         Integer *R=new Integer;
         PFSymbolTable *pTempContext=new PFSymbolTable(pContext);
         pTempContext->AddSymbol(new PFIntegerSymbol("_N",new Integer(2*n)));
         bRetval=H_Primitive::Evaluate(pTempContext,lucasHelper,*R);
         if(bRetval)
         {
            pContext->AddSymbol(new PFIntegerSymbol("_result",R));
         }
         else
         {
            delete R;
         }
         delete pTempContext;
      }
   }
   return bRetval;
}

F_PrimU::F_PrimU() : F_LucasType("primU")
{
}

PFBoolean F_PrimU::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pN=pContext->LookupSymbol("_N");
   if(pN && (pN->GetSymbolType()==INTEGER_SYMBOL_TYPE))
   {
      Integer *N=((PFIntegerSymbol*)pN)->GetValue();
      int n=(*N)&0x3FFFFFFF;
      if(n==(*N))
      {
         // the function can be called, note lucasEval will catch
         // missing _P and _Q definitions
         Integer *R=new Integer;
         bRetval=H_Primitive::Evaluate(pContext,lucasHelper,*R);
         if(bRetval)
         {
            pContext->AddSymbol(new PFIntegerSymbol("_result",R));
         }
         else
         {
            delete R;
         }
      }
   }
   return bRetval;
}

F_NSWType::F_NSWType(const PFString &sName)
   : PFFunctionSymbol(sName)
{
}

DWORD F_NSWType::MinimumArguments() const
{
   return 1;
}

DWORD F_NSWType::MaximumArguments() const
{
   return 1;
}

DWORD F_NSWType::GetArgumentType(DWORD /*dwIndex*/) const
{
   return INTEGER_SYMBOL_TYPE;
}

PFString F_NSWType::GetArgumentName(DWORD /*dwIndex*/) const
{
   return "_N";
}

F_NSW_S::F_NSW_S()
   : F_NSWType("S")
{
}

PFBoolean F_NSW_S::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pS=pContext->LookupSymbol("_N");
   if(pS && pS->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      Integer *pN=((PFIntegerSymbol*)pS)->GetValue();
      int n=(*pN)&0x7FFFFFFF;
      if(((*pN)==n)&&(n>=0))
      {
         Integer XX;
         Integer *R=new Integer;
         PFSymbolTable *pSubContext=new PFSymbolTable(pContext);
         // Add symbols for P and Q
         pSubContext->AddSymbol(new PFIntegerSymbol("_P",new Integer(2)));
         pSubContext->AddSymbol(new PFIntegerSymbol("_Q",new Integer(-1)));
         // run the caller, and capture the value of V
         bRetval=lucasEval(pSubContext,n,*R,XX);
         if(bRetval)
         {
            // remove the additional factor of 2
            (*R)/=2;
            pContext->AddSymbol(new PFIntegerSymbol("_result",R));
         }
         else
         {
            delete R;
         }
         delete pSubContext;
      }
   }
   return bRetval;
}

F_NSW_W::F_NSW_W()
   : F_NSWType("W")
{
}

PFBoolean F_NSW_W::CallFunction(PFSymbolTable *pContext)
{
   PFBoolean bRetval=PFBoolean::b_false;
   IPFSymbol *pS=pContext->LookupSymbol("_N");
   if(pS && pS->GetSymbolType()==INTEGER_SYMBOL_TYPE)
   {
      Integer *pN=((PFIntegerSymbol*)pS)->GetValue();
      int n=(*pN)&0x7FFFFFFF;
      if(((*pN)==n)&&(n>=0))
      {
         Integer XX;
         Integer *R=new Integer;
         PFSymbolTable *pSubContext=new PFSymbolTable(pContext);
         // Add symbols for P and Q
         pSubContext->AddSymbol(new PFIntegerSymbol("_P",new Integer(2)));
         pSubContext->AddSymbol(new PFIntegerSymbol("_Q",new Integer(-1)));
         // run the caller, and capture the value of V
         bRetval=lucasEval(pSubContext,n,XX,*R);
         if(bRetval)
         {
            // remove the additional factor of 2
            pContext->AddSymbol(new PFIntegerSymbol("_result",R));
         }
         else
         {
            delete R;
         }
         delete pSubContext;
      }
   }
   return bRetval;
}
