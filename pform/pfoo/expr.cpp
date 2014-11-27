//==============================================================
// PrimeForm/GW expression evaluator
//==============================================================

#include "pfoopch.h"
#include "expr.h"
#include "symboltypes.h"
#include "pfintegersymbol.h"
#include "pffunctionsymbol.h"
#include "pfstringsymbol.h"
#include "primeserver.h"

//extern volatile unsigned long g_base, g_k, g_n, g_nMin, g_nMax, g_kMin, g_kMax, g_kStep;
//extern bool g_recur;
//extern CString g_recur1, g_recur2, g_recur3, g_recur4, g_pockBase;
//extern volatile long g_a, g_b, g_c;

PFBoolean ex_parseArguments(PFString &w,PFStringArray& tfArguments);

//************************************************************************************************
// NOTES on memory usage.
//  In many functions, there is a .pop() and possibly .peek() functions called.
//  The .pop() returns a pointer which is valid, but MUST be deleted.  We try to use *q for
//  the variable for all of these.  For the .peek() return, you get a valid object pointer,
//  but it is still part of the symbol table/list/whatever.  It can be used and modified
//  (it is not const), however it should NOT be deleted.  We try to use *p in the code
//  below for all peek() return pointers (there is also a *p2 used were 2 peeks were needed).
//
// As other "memory" findings are figured out, they will be documented here also.
//************************************************************************************************

//==============================================================
// recurrence buffer
//==============================================================

#if 0
static unsigned long rBase=0xffffffff;    // the first k value the recurrence is defined for
static unsigned long nBase=0xffffffff;    // the first k value the recurrence is defined for
static unsigned long rLookup=0;        // the index into the next array of the rBase k value
#define RECURRENCE_HISTORY 4
static Integer recurrenceBuffer[RECURRENCE_HISTORY];     // the previous 4 values of the recurrence

// destroy the value of the recurrence

void ex_destroyRecurrence()
{
   rBase=0xffffffff;
   for(int i=0;i<RECURRENCE_HISTORY;i++) recurrenceBuffer[i]=0;   // zap the buffers
}
#endif

//==============================================================
// Library
//==============================================================

#if 0
LibraryItem libraryList[]=
{
   {"FIBONACCI",  fib_primitive_calculator,  fib_primitive_symbol},
   {"FIBFULL",     fib_full_calculator,       fib_full_symbol},
      {"FIBPRIM",     fib_prim_calculator,       fib_prim_symbol},
   {NULL,NULL,NULL}
};

Integer fib(unsigned long n)
{
   double maxError;
// calculate (1+sqrt(5))/2 ^n, call it (a+bsqrt(5)/2)
   if(n==0) return(Integer(0));

   Integer a(2);
   Integer b(0);        // this is to the zeroth power
   Integer c;

   unsigned long f=0x80000000;
   while((f&n)==0) f>>=1;
// f is now the first one bit
   for(;f;f>>=1)
   {
      if(f&n)     // one bit
      {
         // a'=(a+5b)/2 b'=(a+b)/2
         c=b;  // c=b
         c<<=1;   // 2b
         b+=a;
         b>>=1;   // (a+b)/2
         a=b+c;
      }
      if(f!=1) // square except last iteration
      {
         // a'=(aa+5bb)/2, b'=ab
         c=b;
         b.FFTmul(a,0,maxError);    // new b value
         c.FFTsquare(0,maxError);   // bb
         c*=5;
         a.FFTsquare(0,maxError);
         a+=c;
         a>>=1;
      }
   }
   return(b);
}

unsigned long nextprime(unsigned long p)
{
   if(p==2) return(3);
   while(1)
   {
      p+=2;
      unsigned long pl=(unsigned long)floor(sqrt(double(p)));
      unsigned long q;
      for(q=3;q<=pl;q+=2)
         if((p%q)==0) break;
      if(q>pl) break;
   }
   return(p);
}

Integer fib_primitive_calculator(unsigned long b,unsigned long k,unsigned long n)
{
// return the bit of the fib that is unique to us
   Integer x=fib(k);
   unsigned long pl=k/2;
   for(unsigned long p=2;p<=pl;p=nextprime(p))
   {
      if(k%p) continue;
      Integer y=fib(k/p);
      while(y>1)
      {
         Integer z=gcd(x,y);
         x/=z;
         y=z;
      }
   }
   return(x);
}

PFString fib_primitive_symbol(unsigned long b,unsigned long k,unsigned long n)
{
   PFString rv;
   PFString v;
   v.Set(k);

   rv="U(";
   rv+=v;
   rv+=")";

   unsigned long pl=(unsigned long)floor(sqrt(double(k)));
   unsigned long p;
   for(p=2;p<=pl;p=nextprime(p))
      if((k%p)==0) break;

   if(p>pl) rv+="*";
   return(rv);
}

Integer fib_full_calculator(unsigned long b,unsigned long k,unsigned long n)
{
   Integer x=fib(k);
   return(x);
}

PFString fib_full_symbol(unsigned long b,unsigned long k,unsigned long n)
{
   PFString rv="F(";
   PFString v;
   v.Set(k);
   rv+=v;
   rv+=")";

   return(rv);
}

Integer fib_prim_calculator(unsigned long b,unsigned long k,unsigned long n)
{
   double maxError;

   // the method uses the Mobius function
   Integer x(1);
   Integer y(1);

   // first calculate the distinct prime factors of k
   unsigned long pList[32];   // assume less than 32
   unsigned long pc=0;

   unsigned long p;
   unsigned long kw=k;
   for(p=2;;p=nextprime(p))
   {
      if(p*p>kw) break;
      if((kw%p)==0)
      {
         pList[pc++]=p;
         do
         {
            kw/=p;
         }
         while((kw%p)==0);
      }
   }

   if(kw>1)
   {
      pList[pc++]=kw;
   }

   unsigned long q;
   unsigned long px;

   // now for 2^pc combinations, do these calculations
   for(p=0;p<(1UL<<pc);p++)
   {
      // make kw equal the number that k divided by all the primes with bits set
      kw=k;
      px=0;
      for(q=0;q<pc;q++)
      {
         if(p&(1<<q))
         {
            px++; // px is the bitcount
            kw/=pList[q];
         }
      }

      Integer z=fib(kw);

      if(px&1)
      {
         y.FFTmul(z,0,maxError);
      }
      else
      {
         x.FFTmul(z,0,maxError);
      }
   }

   // finally return x/y
   x/=y;
   return(x);
}

PFString fib_prim_symbol(unsigned long b,unsigned long k,unsigned long n)
{
   PFString rv="F*(";
   PFString v;
   v.Set(k);
   rv+=v;
   rv+=")";
   return(rv);
}
#endif

//==============================================================
// lists of operators
//==============================================================

DyadicList::DyadicList() : PFStack<ExprOperator>(PFBoolean::b_true)
{
    Push(new D_ADD);
    Push(new D_SUB);
    Push(new D_MUL);
    Push(new D_MULP);
    Push(new D_DIV);
   Push(new D_OR2);
   Push(new D_AND2);
    Push(new D_MOD);
//    Push(new D_GCD);
    Push(new D_POW);
   Push(new D_EQUAL);
   Push(new D_NOTEQUAL);
   Push(new D_GE);         // =>
   Push(new D_LE);         // =<
   Push(new D_GE2);     // >= c syntax
   Push(new D_LE2);     // <= c syntax
   Push(new D_OR);
   Push(new D_AND);
   Push(new D_GREATER); // > MUST be below the >= or > will ALWAYS be found!
   Push(new D_LESS);    // < MUST be below the <= or < will ALWAYS be found!
//    Push(new D_OPENPAREN);
    Push(new D_CLOSEPAREN);
}

DyadicList::~DyadicList()
{
}

MonadicPrefixList::MonadicPrefixList() : PFStack<ExprOperator>(PFBoolean::b_true)
{
    Push(new MP_MINUS);
    Push(new MP_OPENPAREN);
    Push(new MP_NOT);
#if 0
   Push(new MP_RECURRENCE);
    Push(new MP_PRIME);
#endif
}

MonadicPrefixList::~MonadicPrefixList()
{
}

MonadicSuffixList::MonadicSuffixList() : PFStack<ExprOperator>(PFBoolean::b_true)
{
    Push(new MS_FACT);
    Push(new MS_PRIM);
}

MonadicSuffixList::~MonadicSuffixList()
{
}


//==============================================================
// operator elements
//==============================================================

ExprOperator::ExprOperator(const PFString &s,int f,int p)
   : symbol(s), flags(f), precedence(p), m_gData(1)
{
}

ExprOperator::~ExprOperator()
{
}

const ExprOperator *ExprOperator::Identify() const
{
   return this;
}

ExprOperator *ExprOperator::Mutant() const
{
   return NULL;
}

CloneOperator::CloneOperator(ExprOperator *pSource)
   : ExprOperator("",0,0), m_pSource(pSource)
{
}

PFBoolean CloneOperator::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m) // perform the op
{
   m_pSource->m_gData=m_gData;
   return m_pSource->evaluate(s,o,m);
}

const ExprOperator *CloneOperator::Identify() const
{
   return m_pSource;
}

PFBoolean D_ADD::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
    if(m) (*p)%=m,(*q)%=m;
    (*p)+=(*q);
    if(m) (*p)%=m;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_SUB::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
    if(m) (*p)%=m,(*q)%=m;
    (*p)-=(*q);
    if(m) (*p)%=m;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_MUL::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
    if(m) (*p)%=m,(*q)%=m;
    (*p)*=(*q);
    if(m) (*p)%=m;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_MULP::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
    if(m) (*p)%=m,(*q)%=m;
    (*p)*=(*q);
    if(m) (*p)%=m;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_DIV::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
    if(m) (*p)%=m,(*q)%=m;
    (*p)/=(*q);
    if(m) (*p)%=m;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_MOD::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
    if(m) (*p)%=m,(*q)%=m;
    (*p)%=(*q);
    if(m) (*p)%=m;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_POW::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
   Integer *q=s.Pop();
   if(q==NULL) return(PFBoolean::b_false);
   Integer *p=s.Peek();
   if(p==NULL) {delete q;return(PFBoolean::b_false);}
   if(m) (*p)%=m;

   // Raising to the q th power is different, note that exponentiation
   // to large q just isn't possible
   if (numbits(*q)>30)       // max is 2^31-1
   {
       delete q;
       return(PFBoolean::b_false);
   }
   int qq = ((*q) & INT_MAX);
   delete q;

   if(qq==0)
   {
      (*p)=1;
   }
   else
   {
     if(m)
     {
        Integer mm;
        mm=m;
        (*p)=powm(*p,qq,mm);
     }
     else (*p)=pow(*p,qq);
   }
   o.Remove();
   return(PFBoolean::b_true);
}

PFBoolean D_EQUAL::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
    if(m) (*p)%=m,(*q)%=m;
   if ((*p)==(*q))
      *p=1;
   else
      *p=0;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_NOTEQUAL::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
    if(m) (*p)%=m,(*q)%=m;
   if ((*p)!=(*q))
      *p=1;
   else
      *p=0;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_GREATER::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
    if(m) (*p)%=m,(*q)%=m;
   if ((*p)>(*q))
      *p=1;
   else
      *p=0;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_LESS::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
    if(m) (*p)%=m,(*q)%=m;
   if ((*p)<(*q))
      *p=1;
   else
      *p=0;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_GE::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
    if(m) (*p)%=m,(*q)%=m;
   if ((*p)>=(*q))
      *p=1;
   else
      *p=0;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_GE2::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
    if(m) (*p)%=m,(*q)%=m;
   if ((*p)>=(*q))
      *p=1;
   else
      *p=0;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_LE::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
    if(m) (*p)%=m,(*q)%=m;
   if ((*p)<=(*q))
      *p=1;
   else
      *p=0;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_LE2::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
    if(m) (*p)%=m,(*q)%=m;
   if ((*p)<=(*q))
      *p=1;
   else
      *p=0;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_OR::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int /*m*/)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
   if ((*p)==0 && (*q)==0)
      *p=0;
   else
      *p=1;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}
PFBoolean D_OR2::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int /*m*/)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
   if ((*p)==0 && (*q)==0)
      *p=0;
   else
      *p=1;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}

PFBoolean D_AND::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int /*m*/)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
   if ((*p)==0 || (*q)==0)
      *p=0;
   else
      *p=1;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}
PFBoolean D_AND2::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int /*m*/)
{
    Integer *q=s.Pop();
    if(q==NULL) return(PFBoolean::b_false);
    Integer *p=s.Peek();
    if(p==NULL) {delete q;return(PFBoolean::b_false);}
   if ((*p)==0 || (*q)==0)
      *p=0;
   else
      *p=1;
    o.Remove();
    delete q;
    return(PFBoolean::b_true);
}


//PFBoolean D_OPENPAREN::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
//{
// return(PFBoolean::b_false);   // should never be called
//}

PFBoolean D_CLOSEPAREN::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
    // close parentheses are never stacked as they clear immediately all higher precedence ops
   // so basically just unwind the stack until an open parenthesis. Clear monadic operators,
   // and force re-evaluation
   PFBoolean valid=PFBoolean::b_true;

   ExprOperator *p=NULL;
   while(valid && ((p=o.Peek())!=NULL))   // dyadics destack themselves
   {
      if(p->precedence==-10) break; // Open paren
      valid=p->evaluate(s,o,m);
   }
   // the parenthesis should just have left a single integer on the stack
   if(valid)
   {
      o.Remove();    // remove the parenthesis from the op stack
      valid=ex_clearOnInteger(s,o,m);        // clear monadic prefixes.
   }
   // finally, if the open parenthesis was dyadic, then we need to replace with a MUL
   if(valid)
      if(p->flags==DYADIC)
      {
         Integer *i=s.Pop();
         valid=ex_stackPrecedence(s,o,m,new D_MUL);
         s.Push(i);
      }
    return(valid);
}

PFBoolean MP_NOT::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> & /*o*/,int /*m*/)
{

    Integer *p=s.Peek();
    if(p==NULL) return(PFBoolean::b_false);
   if ((*p)==0)
      *p=1;
   else
      *p=0;
    return(PFBoolean::b_true);
}

PFBoolean MP_MINUS::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &/*o*/,int /*m*/)
{
    Integer *p=s.Peek();
    if(p==NULL) return(PFBoolean::b_false);
    (*p)*=-1;
//    o.remove();
    return(PFBoolean::b_true);
}

PFBoolean MP_OPENPAREN::evaluate(PFStack<Integer> &/*s*/,PFStack<ExprOperator> &/*o*/,int /*m*/)
{
   return(PFBoolean::b_false);   // should never be called
}

MS_FACT::MS_FACT()
   : ExprOperator("!",MONADIC_SUFFIX,5), m_pMutant(NULL)
{
   m_pMutant=new D_FACT;
}

MS_FACT::~MS_FACT()
{
   delete m_pMutant;
}

ExprOperator *MS_FACT::Mutant() const
{
   return m_pMutant;
}

void FactorialHelper(Integer *p, long nMultStep, int m)
{
   // here are the 4 "accumulators" used so that each of them grows to only 1/4 the size of the "final" result.
   // We should see improvements similar to the difference betwen Karatsuba compared to "classical" multiplication
   Integer mm2[4];
   mm2[0]=1; mm2[1]=1; mm2[2]=1;mm2[3]=1;

   long nStop = ((*p) & INT_MAX);
   if (nMultStep > 1)
   {
      // find the first integer > 1 which will be in the "set" of factors  valid for this multifactorial.
      long q = nMultStep+nStop%nMultStep;
      if (q-nMultStep > 1)
         q -= nMultStep;
      int which=0;

      for (; q <= nStop; q+=nMultStep)
      {
         uint64 ui64Tmp=q;
         for (; ui64Tmp*(q+nMultStep) < INT_MAX && q < nStop;)
         {
            q += nMultStep;
            ui64Tmp *= q;
         }
         mm2[which%4] *= (int32)ui64Tmp;
         if (m)
            mm2[which%4] %= m;
         which++;
      }
   }
   else
   {
      int32 nTmp=1;

      // Get the singlton runs of 11 out of the way.  After this, all runs will reduce themselves by 1 at
      // the break points.  There is a single run of 7, three runs of 6, 36 runs of 4, lots of runs of 3 (up to
      // 1290), and a bunch of runs of 2 (up to sqrt(2^31)).  From that point on, you can not pack more than
      // one number into a 31 bit long.

      // 12! fits in a long (31 bits)
      int32 q = 2;
      for (; q <= nStop && q < 13; q++)
         nTmp *= q;

      mm2[3] = nTmp;

      if (nStop > 12)
      {
         // Working from small numbers to large (starting at 13), there is 1 case where
         // we can multiply 7 numbers and fit the result into 31 bits, 3 cases where
         // we can fit 6 numbers multiplied into 31 bits, then 7 cases where 5 fit, 36
         // cases where 4 fit ...
         static int nFactTable[] = { 0, 0, 22525, 358, 36, 7, 3, 1};

         // Ok, now we can loop for a while.
         for (int nNumsToPack = 7; nNumsToPack > 1 && q <= nStop; --nNumsToPack)
         {
            int nLoops = nFactTable[nNumsToPack];
            while (nLoops && q <= nStop)
            {
               nTmp=1;
               for (int i = 0; i < nNumsToPack && q <= nStop; i++, q++)
                  nTmp *= q;
               mm2[nLoops%4] *= nTmp;
               if(m!=0)
                  mm2[nLoops%4] %= m;
               --nLoops;
            }
         }
         while (nStop >= q)
         {
            mm2[q%4] *= q;
            if (m)
               mm2[q%4] %= m;
            q++;
         }
      }
   }
   // set p to the first accumulator.
    *p = mm2[0];
   // now multiply the other 3 accumulators to p
        for (int i=1; i<4; i++)
   {
      *p *= mm2[i];
      if (m)
         *p %= m;
   }
}

PFBoolean MS_FACT::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &/*o*/,int m)
{
   Integer *p=s.Peek();
   if(p==NULL) return(PFBoolean::b_false);

   // if p is too large, forget it.
   if((*p)<0) return PFBoolean::b_false;

   Integer gLimit=m_gData;
   gLimit*=1000000;

   if((*p)>gLimit) return(PFBoolean::b_false);

   FactorialHelper(p, m_gData&INT_MAX, m);
   return(PFBoolean::b_true);
}

PFBoolean D_FACT::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int m)
{
   Integer *q=s.Pop();
   if(q==NULL) return(PFBoolean::b_false);
   if((*q)<=0) {delete q; return(PFBoolean::b_false);}

   Integer *p=s.Peek();
   if(p==NULL) {delete q; return(PFBoolean::b_false);}
   
   // if p is too large, forget it.
   if((*p)<0) {delete q; return(PFBoolean::b_false);}
   
   Integer gLimit=*q;
   gLimit*=1000000;

   if((*p)>gLimit) {delete q; return(PFBoolean::b_false);}

   if (m)
      *p %= m;
   Integer Val = *p - *q;
   while (Val > 1)
   {
      *p *= Val;
      Val -= *q;
   }

   delete q;
   o.Remove();
   return(PFBoolean::b_true);
}

MS_PRIM::MS_PRIM()
   : ExprOperator("#",MONADIC_SUFFIX,5), m_pMutant(NULL)
{
   m_pMutant=new D_PRIM;
}

MS_PRIM::~MS_PRIM()
{
   delete m_pMutant;
}

ExprOperator *MS_PRIM::Mutant() const
{
   return m_pMutant;
}


PFBoolean PrimorialHelper(Integer *p, uint32 pp)
{
   uint64 q;
   Integer mm;
   mm=1;

   primeserver->SkipTo(1);
   for (q = primeserver->NextPrime(); q <= pp; q = primeserver->NextPrime())
     mm *= q;

   *p=mm;

   return(PFBoolean::b_true);
}

PFBoolean PrimorialHelper2(Integer *p, uint32 pp, uint32 m)
{
   uint64 q;
   Integer mm;
   mm=1;
   
   primeserver->SkipTo(m);
   for (q = primeserver->NextPrime(); q <= pp; q = primeserver->NextPrime())
     mm *= q;

   *p=mm;

   return(PFBoolean::b_true);
}

PFBoolean D_PRIM::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> &o,int /*m*/)
{
   Integer *q=s.Pop();
   if(q==NULL) return(PFBoolean::b_false);
   if((*q)<=0) {delete q; return(PFBoolean::b_false);}

   // the 'data' item allows for extended primorials  1093#103 is 1093#/101# or (103.107.109....1091.1093)
    Integer *p=s.Peek();
    if(p==NULL) {delete q; return(PFBoolean::b_false);}

    PFBoolean b = PrimorialHelper2(p, (*p)&INT_MAX, (*q)&INT_MAX);
    delete q;
    o.Remove();
    return(b);
}

PFBoolean MS_PRIM::evaluate(PFStack<Integer> &s,PFStack<ExprOperator> & /*o*/,int m)
{
    Integer *p=s.Peek();
    if(p==NULL) return(PFBoolean::b_false);

    // if p is too large, forget it.
    if((*p)>20000000) return(PFBoolean::b_false);

    uint32 pp= ((*p) & INT_MAX);

   if (m == 0)
      return PrimorialHelper(p,pp);

   // I don't think we can EVER get here.


   Integer mm;
   mm=1;

   uint64 q;
   for (q = primeserver->NextPrime(); q <= pp; q = primeserver->NextPrime())
   {
      mm*=(q%m);
      mm%=m;
   }
   *p=mm;

   return(PFBoolean::b_true);
}

//==============================================================
// basic expression evaluator is a number/operator stack with
// operator precedence and support for parentheses.
//==============================================================

Integer *ex_parseInteger(PFString &w)
{
   Integer *p=new Integer;
   (*p)=0;                 // initialize
   PFBoolean seen=PFBoolean::b_false;
   char c;
   DWORD dwIndex;
   uint32 Accum = 0;
   DWORD ADigits=0;

   for(dwIndex=0;dwIndex<w.GetLength();dwIndex++)
   {
      c=w[dwIndex];
      if((c<'0')||(c>'9'))
         break;
      seen=PFBoolean::b_true;
      Accum *= 10;
      Accum += (c-'0');
      ADigits++;
      if(ADigits==9)
      {
         (*p)*=1000000000;
         (*p)+=Accum;
         Accum = 0;
         ADigits=0;
      }
   }
   if (seen)
   {
      switch(ADigits)
      {
         case 8:
            (*p)*=100000000;
            break;
         case 7:
            (*p)*=10000000;
            break;
         case 6:
            (*p)*=1000000;
            break;
         case 5:
            (*p)*=100000;
            break;
         case 4:
            (*p)*=10000;
            break;
         case 3:
            (*p)*=1000;
            break;
         case 2:
            (*p)*=100;
            break;
         case 1:
            (*p)*=10;
            break;
         default:
            break;
      }
      (*p) += Accum;
      w=w.Mid(dwIndex);
   }
   else
   {
      delete p;
      p=NULL;
   }
   return(p);
}

ExprOperator *ex_seekOperator(PFStack<ExprOperator> &s,PFString &w)
{
   PFForwardIterator pffi;
   s.StartIterator(pffi);

    ExprOperator *p;
    PFListNode *pn;

    ExprOperator *pRetval=NULL;

    while(pffi.Iterate(pn))
    {
      p=(ExprOperator*)pn->GetData();
      if(p->symbol.GetLength()>w.GetLength())
         continue;
      DWORD dwLength=p->symbol.GetLength();
      PFString sl=w.Left(dwLength);
      if(sl==p->symbol)
      {
         w=w.Mid(dwLength);
         pRetval=p;
         break;
      }
    }

    return(pRetval);
}

PFBoolean ex_clearOnInteger(PFStack<Integer> &i,PFStack<ExprOperator> &o,int m)
{
// an integer has been stacked, so clear all monadic prefixes
    ExprOperator *p;
    PFBoolean rv=PFBoolean::b_true;

    while((p=o.Peek())!=NULL)
    {
        if(p->flags!=MONADIC_PREFIX) break;
// do not remove parenthesis
      if(p->precedence==-10) break; // Openparen
        o.Remove();     // remove it from the stack
        rv=p->evaluate(i,o,m);
        if(!rv) break;
    }
    return(rv);
}

PFBoolean ex_stackPrecedence(PFStack<Integer> &i,PFStack<ExprOperator> &o,int m)
{
   return ex_stackPrecedence(i, o, m, 0);
}

PFBoolean ex_stackPrecedence(PFStack<Integer> &i,PFStack<ExprOperator> &o,int m,ExprOperator *op)
{
    ExprOperator *p;
    PFBoolean rv=PFBoolean::b_true;
    while((p=o.Peek())!=NULL)
    {
        if(p->flags!=DYADIC) break;
// a dyadic operator has been found. If it is higher or equal
// than the suggested, clear it
        if((op==NULL) || (p->precedence>=op->precedence))
        {
            rv=p->evaluate(i,o,m);
            if(!rv) break;
        }
      else break;
    }
   if(rv) if(op) o.Push(op);
   return(rv);
}

PFBoolean ex_clearMonadicSuffixes(PFStack<Integer> &s, PFStack<ExprOperator> &o, int m)
{
// monadic suffixes may be complex, such as "multifactorial" '!!!!'
// so there may be a need to clear the stack
// basically the monadic suffixes are reversed and filtered
    PFStack<ExprOperator> r; // reversed stack
    ExprOperator* p;

    while((p=o.Peek())!=NULL)
    {
        if(p->flags!=MONADIC_SUFFIX) break;
        o.Remove();     // taken off stack
        ExprOperator* p2=r.Peek();     // look at the last reverse stacked operator
        if(p2 && (p->Identify()==p2->Identify()))
        {
         ++(p2->m_gData);
        }
        else
        {
         r.Push(new CloneOperator(p));
        }
    }
// r reverses the monadic suffixes with incidence count in 'data'
// now clear monadic suffixes

   PFBoolean valid=PFBoolean::b_true;
    ExprOperator* q;

   while((q=r.Pop())!=NULL)
   {
      if(valid)
      {
         valid=q->evaluate(s,o,m);
      }
      delete q;
   }
   return(valid);
}

PFBoolean ex_matchparen(const PFString &s)
{
    {
        int p=0;
        for(DWORD i=0;i<s.GetLength();i++)
        {
            if(s[i]=='(') p++;
            if(s[i]==')') if(p--) {} else {return PFBoolean::b_false;}
        }
        if(p) return(PFBoolean::b_false);
    }
   return(PFBoolean::b_true);
}

PFBoolean ex_cleanup(const PFString &s,PFString &w)
{
   if(!ex_matchparen(s)) return(PFBoolean::b_false);
   PFString t;
    w="";
    for(DWORD i=0;i<s.GetLength();i++)
         if(s[i]!=' ') w+=s[i];
// strip spurious parenthesis. Note this isn't actually that easy ()/() might fool you
   while(!w.IsEmpty())
   {
      if(w.Left(1)!="(") break;
      if(w.Right(1)!=")") break;
// check to see if the left parenthesis matches the right one
      t=w.Mid(1,w.GetLength()-2);
      if(!ex_matchparen(t)) break;
      w=t;
   }
   return(PFBoolean::b_true);
}

// now do the substitutions
#if 0
void ex_substitute(PFString &w)
{
   PFString t;
   DWORD i;
   t=w;
   w="";
   for(i=0;i<t.GetLength();i++)
   {
      char c=t[i];
      if(c=='%') {i++; c=t[i]; w+=c; continue;}
      if((c>='A')&&(c<='Z')) c+=('a'-'A');
      if((c>='a')&&(c<='z'))
      {
// parametric substitution
         PFString x;
         switch(c)
         {
         case 'k':
            x.Format("%u",g_k);
            break;
         case 'n':
            x.Format("%u",g_n);
            break;
         case 'b':
            x.Format("%u",g_base);
            break;
            case 'x':
                x.Format("%d",g_a);
                break;
            case 'y':
                x.Format("%d",g_b);
                break;
            case 'z':
                x.Format("%d",g_c);
                break;
         default:
            x=c;
            break;
         }
         w+=x;
      }
      else if(c=='\"')
      {  // an open quote denotes, "filename to follow"
         PFString fn="";
         while(1)
         {
            c=t[++i];
            if((c==0)||(c=='\"'))
            {
               if(c==0) --i;  // back up
               break;
            }
            fn+=c;
         }
         // now the file name is received, so begin churing lines
         PFTextFile pftf(fn);
         PFTextFileIterator pftfi(pftf);
         PFTextLine *pLine;

         // walk until we find a non-comment line
         while((pLine=pftfi.GetNextLine())!=NULL)
         {
            if(!pLine->IsComment())
            {
               w+=pLine->GetText();
               break;
            }
         }
      }
      else
         w+=c;
   }
}
#endif

PFString ex_display(PFSymbolTable *pContext,const PFString &w)
{
   PFBoolean bSuccessful=PFBoolean::b_true;

#if 0
// if the parameter string begins with the word LIBRARY, then do a lib call
   if(w.Left(7)=="LIBRARY")
   {
      PFString s=w.Mid(7);
      while(s.Left(1)==" ") s=s.Mid(1);
      LibraryItem *p=libraryList;
      while(p->name)
      {
         if(s.CompareNoCase(p->name)==0)
         {
            return ((p->symbolism)(g_base,g_k,g_n));
         }
         p++;
      }
      return("**library not found**");
   }
#endif
   // we disable the library

   PFString r;
#if 0
// if we are in recurrence mode, this is a wee bit different
   if(g_recur && (w==g_pockBase))
      r="f(k) %n=n";
   else
// this routine calculates the screen display format with necessary substitutions
// and also with abbreviation
#endif
      ex_cleanup(w,r);     // excleanup does substitutions as well

   // we used to run ex_substitute. But this is NOT the correct way to proceed now.
   // we need to do a tokenizing scan

   //ex_substitute(r);

   DWORD dwLength=r.GetLength();
   PFString sOutput;
   PFString sSymbol;

   DWORD dwState=TS_NORMAL;

   for(DWORD dw=0;bSuccessful && (dw<dwLength);dw++)
   {
      TCHAR c=r[dw];

      switch(dwState)
      {
      case TS_NORMAL:
         if(c=='\"')
         {
            sOutput+=c;
            dwState=TS_STRING;
         }
         else if(IsFirstSymbolCharacter(c))
         {
            sSymbol=c;
            dwState=TS_SYMBOL;
         }
         else
         {
            sOutput+=c;
         }
         break;

      case TS_STRING:
         sOutput+=c;
         if(c=='\"')
         {
            dwState=TS_NORMAL;
         }
         break;

      case TS_SYMBOL:
         if(IsSymbolCharacter(c))
         {
            sSymbol+=c;
         }
         else if(c=='\"')
         {
            bSuccessful=PFBoolean::b_false;
         }
         else
         {
            // the symbol has terminated so find it, its string representation, etc
            IPFSymbol *pSymbol=pContext->LookupSymbol(sSymbol);

            if(pSymbol==NULL)
            {
               bSuccessful=PFBoolean::b_false;
            }
            else
            {
               sOutput+=pSymbol->GetStringValue();
            }
            dwState=TS_NORMAL;
            sOutput+=c;
         }
      }
   }

   // do final cleanup
   if(bSuccessful)
   {
      switch(dwState)
      {
      case TS_NORMAL:
         break;
      case TS_STRING:
         bSuccessful=PFBoolean::b_false;
         break;
      case TS_SYMBOL:
         {
            IPFSymbol *pSymbol=pContext->LookupSymbol(sSymbol);
            if(pSymbol==NULL)
            {
               bSuccessful=PFBoolean::b_false;
            }
            else
            {
               sOutput+=pSymbol->GetStringValue();
            }
         }
      }
   }

   if(!bSuccessful)
   {
      sOutput="";
   }

// if(r.GetLength()>32)
// {
//    r=r.Left(12)+"........"+r.Right(12);
// }

   return(sOutput);
}

//====================================================================================================
// The expression evaluator. It will ultimately support $b, $n, $k to use the Proth values
//====================================================================================================

// if in recurrence mode, and an evaluate call is made, and we are evaluating g_pockBase
// if so then make sure the recurrence is evaluated correctly
#if 0
Integer *ex_prepRecurrence()
{
   Integer *retval=NULL;
   if((rBase==0xffffffff)||(g_k<rBase)||(g_n!=nBase)) // bless it, it isn't warmed up yet
   {
      rLookup=0;
      rBase=1;
      nBase=g_n;
      retval=ex_evaluate(g_recur1,0,PFBoolean::b_false); if(retval) {recurrenceBuffer[0]=*retval; delete retval;}
      retval=ex_evaluate(g_recur2,0,PFBoolean::b_false); if(retval) {recurrenceBuffer[1]=*retval; delete retval;}
      retval=ex_evaluate(g_recur3,0,PFBoolean::b_false); if(retval) {recurrenceBuffer[2]=*retval; delete retval;}
      retval=ex_evaluate(g_recur4,0,PFBoolean::b_false); if(retval) {recurrenceBuffer[3]=*retval; delete retval;}
   }
// now warm up the recurrence if needed
   if(g_k<rBase+RECURRENCE_HISTORY)
   {
      retval=new Integer(recurrenceBuffer[(g_k-rBase+rLookup)%RECURRENCE_HISTORY]);
      return(retval);
   }
// otherwise run the recurrence until you calculate g_k
   unsigned long k=g_k;
   g_k=rBase+RECURRENCE_HISTORY;
   retval=NULL;
   do
   {
      if(retval) delete retval;  // delete the previously-calculated value
      retval=ex_evaluate(g_pockBase,0,PFBoolean::b_false);     // do not re-enter this routine
      if(retval) recurrenceBuffer[rLookup]=*retval;   // store it
      rBase++;             // increment the base
      rLookup++;              // move the lookup base
      rLookup%=RECURRENCE_HISTORY;
      g_k++;                  // increment to the next value
   }
   while(g_k<=k);    // the last one written to the buffer is g_k
   g_k=k;            // reset g_k back
   return(retval);      // the last value computed
}
#endif

Integer *ex_evaluate(PFSymbolTable *pContext,const PFString &s)
{
   return ex_evaluate(pContext, s, 0);
}

Integer *ex_evaluate(PFSymbolTable *pContext,const PFString &s,int m)
{
#if 0
   if(s.Left(7)=="LIBRARY")
   {
      PFString t=s.Mid(7);
      while(t.Left(1)==" ") t=t.Mid(1);

      LibraryItem *p=libraryList;
      while(p->name)
      {
         if(t.CompareNoCase(p->name)==0)
         {
            return new Integer((p->calculator)(g_base,g_k,g_n));
         }
         p++;
      }
      return(NULL);
   }
#endif

#if 0
   if(testRecur) prm_init();  // initialize prime generator, but not if the calculation was re-entered
// before doing anything else, check if we may as well reset testRecur
   if(s!=g_pockBase) testRecur=PFBoolean::b_false;       // disable the testRecur
   if(!g_recur) testRecur=PFBoolean::b_false;
   if(testRecur) return(ex_prepRecurrence());   // go through the recurrence relation
// before doing anything too complex, check parentheses
#endif

    Integer *retval=NULL;

   PFString w;
   if(!ex_cleanup(s,w)) return(NULL);

// ex_substitute(w);

   // no substitution is done now. However we still have a state machine

    DyadicList dl;
    MonadicPrefixList mpl;
    MonadicSuffixList msl;

    PFStack<Integer> iStack(PFBoolean::b_true);    // owns the objects
    PFStack<ExprOperator> oStack;


// state machine, is in one of two states
    DWORD dwState=TS_NORMAL;
    PFBoolean valid=PFBoolean::b_true;

   PFString sFilename;
   PFString sSymbol;

   while(valid && (!w.IsEmpty()))
    {
      TCHAR c=(LPCTSTR(w))[0];

        switch(dwState)
        {
        case TS_NORMAL:
         if(IsIntegerCharacter(c))
         {
            Integer *i=ex_parseInteger(w);
            iStack.Push(i);
            valid=ex_clearOnInteger(iStack,oStack,m);
            dwState=TS_DYADIC;
         }
            else
            {
               ExprOperator *o=ex_seekOperator(mpl,w);
                if(o!=NULL)
                {
               if (o->precedence == 4)
               {
                  // unary minus
                  PFString sub_op("-");
                  o=ex_seekOperator(dl, sub_op);
                  // Should find D_SUB
                  Integer *i = new Integer;
                  *i = 0;
                  // Now push 0 onto the Integer stack
                  iStack.Push(i);
                  // Then push the D_SUB.   now a -c becomes 0-c
               }
                  oStack.Push(o);
                }
                else
                {
                  // this is where strings or symbols can start
                  if(c=='\"')
                  {
                     dwState=TS_STRING;
                     w=w.Mid(1);             // strip off that opening quote
                     DWORD dwClose=w.Find('\"');   // look for the close quote
                     if(dwClose==NOT_FOUND)
                     {
                        valid=PFBoolean::b_false;
                     }
                     else
                     {
                        sFilename=w.Left(dwClose);
                        w=w.Mid(dwClose+1);

                        // sFilename now evaluates to an integer. So load it
                        // actually, what is loaded could be an expression!
                     valid=PFBoolean::b_false;  // assume false (can't find file, invalid expr, ...)
                     try
                     {
                        // note the constructor for PFSimpleFile throws exceptions, so we have to wrap in a try/catch
                        PFSimpleFile pftf(sFilename);
                     PFString sSubExpression;

                        if(PFSimpleFile::e_ok==pftf.GetNextLine(sSubExpression))
                        {
                           Integer *piSub=ex_evaluate(pContext,sSubExpression,m);
                           if(piSub)
                           {
                              iStack.Push(piSub);
                           valid=ex_clearOnInteger(iStack,oStack,m);
                           dwState=TS_DYADIC;
                        }
                       }   // endif file contained a valid line
                     }
                     catch(...) { }
                     }  // endif close quote found
               }  // endif opening quote was found
               else if(IsFirstSymbolCharacter(c))
               {
                  // this is a symbol. Now it could be an INTEGER, or it could be a FUNCTION
                  // or it could be no symbol at all
                  DWORD dw;
                  for(dw=1;dw<w.GetLength();dw++)
                  {
                     if(!IsSymbolCharacter(w[dw]))
                     {
                        break;
                     }
                  }
                  // dw is the number of characters in the symbol
                  sSymbol=w.Left(dw);
                  w=w.Mid(dw);

                  IPFSymbol *pSymbol=pContext->LookupSymbol(sSymbol);
                  if(pSymbol==NULL)
                  {
                     valid=PFBoolean::b_false;
                  }
                  else
                  {
                     // switch case on the possible symbol types
                     switch(pSymbol->GetSymbolType())
                     {
                     case INTEGER_SYMBOL_TYPE:
                        {
                           Integer *ii=((PFIntegerSymbol*)pSymbol)->DuplicateValue();
                           if(ii)
                           {
                              iStack.Push(ii);
                              valid=ex_clearOnInteger(iStack,oStack,m);
                              dwState=TS_DYADIC;
                           }
                           else
                           {
                              valid=PFBoolean::b_false;
                           }
                          }
                        break;
                     case STRING_SYMBOL_TYPE:
                        {
                           PFString sString=pSymbol->GetStringValue();
                           Integer *ii=ex_evaluate(pContext,sString,m);
                           if(ii)
                           {
                              iStack.Push(ii);
                              valid=ex_clearOnInteger(iStack,oStack,m);
                              dwState=TS_DYADIC;
                           }
                           else
                           {
                              valid=PFBoolean::b_false;
                           }
                          }
                        break;
                     case FUNCTION_SYMBOL_TYPE:
                        {
                           // this is the tough one, or the interesting one, depending on
                           // your point of view.
                           PFFunctionSymbol *pFunction=(PFFunctionSymbol*)pSymbol;
                           DWORD dwMinArguments=pFunction->MinimumArguments();
                           DWORD dwMaxArguments=pFunction->MaximumArguments();
                           DWORD dwActualArguments=0;

                           PFStringArray tfArguments;

                           if(((LPCTSTR)(w))[0]=='(')
                           {
                              // we actually have arguments, we parse them!
                              valid=ex_parseArguments(w,tfArguments);
                              dwActualArguments=tfArguments.GetSize();
                              // note each of these arguments is an expression in its own right.
                              // however we might have some type checking to do in a moment
                           }
                           if(valid)
                           {
                              if((dwActualArguments>=dwMinArguments)&&(dwActualArguments<=dwMaxArguments))
                              {
                                 // time to pass the arguments to pFunction. Note the arguments come in two types.
                                 // STRING arguments are not evaluated (the target may evaluate them if it chooses).
                                 // INTEGER arguments are evaluated here
                                 PFSymbolTable *pSubContext=new PFSymbolTable(pContext);

                                 Integer *pArgc=new Integer(int(dwActualArguments));
                                 pSubContext->AddSymbol(new PFIntegerSymbol("argc",pArgc));

                                 for(DWORD dww=0;valid && (dww<dwActualArguments);dww++)
                                 {
                                    PFString sText=tfArguments[dww];
                                    DWORD dwArgType=pFunction->GetArgumentType(dww);
                                    PFString sName=pFunction->GetArgumentName(dww);

                                    switch(dwArgType)
                                    {
                                    case INTEGER_SYMBOL_TYPE:
                                       {
                                          Integer *i=ex_evaluate(pContext,sText,m);
                                          if(i)
                                          {
                                             pSubContext->AddSymbol(new PFIntegerSymbol(sName,i));
                                          }
                                          else
                                          {
                                             valid=PFBoolean::b_false;
                                          }
                                       }
                                       break;
                                    case STRING_SYMBOL_TYPE:
                                       pSubContext->AddSymbol(new PFStringSymbol(sName,sText));
                                       break;
                                    default:
                                       valid=PFBoolean::b_false;
                                       break;
                                    }
                                 }  // all parameters are passed

                                 if(valid)
                                 {
                                    valid=pFunction->CallFunction(pSubContext);
                                    if(valid)
                                    {
                                       // stack the result. Note the result is currently restricted to be an integer
                                       IPFSymbol *pResult=pSubContext->LookupSymbol("_result");
                                       if(pResult==NULL)
                                       {
                                          valid=PFBoolean::b_false;
                                       }
                                       else if(pResult->GetSymbolType()!=INTEGER_SYMBOL_TYPE)
                                       {
                                          valid=PFBoolean::b_false;
                                       }
                                       else
                                       {
                                          PFIntegerSymbol *pi=(PFIntegerSymbol*)pResult;
                                          Integer *i=pi->DuplicateValue();

                                          if(i==NULL)
                                          {
                                             valid=PFBoolean::b_false;
                                          }
                                          else
                                          {
                                             iStack.Push(i);
                                             valid=ex_clearOnInteger(iStack,oStack,m);
                                             dwState=TS_DYADIC;
                                          }
                                       }
                                    }
                                 }

                                 delete pSubContext;
                              }
                              else
                              {
                                 valid=PFBoolean::b_false;
                              }
                           }
                        }
                        break;

                     default:
                        valid=PFBoolean::b_false;
                     }
                  }
               }
               else
               {
                        valid=PFBoolean::b_false;
                     }
                }
         }
         break;

        case TS_STRING:
        case TS_SYMBOL:
            break;

        case TS_DYADIC:
// expect monadic suffix, dyadic, eol
            {
                PFString w2=w;
                ExprOperator *o=ex_seekOperator(msl,w);
            if (o && *w2 == '!' && *w == '=')
            {
               o = NULL;   // this was a != and not a factorial !
               w = w2;
            }
                if(o!=NULL) oStack.Push(o);               // allow a chaining of monadic suffixes
                else
                {
                    // clear buffered monadic suffixes
                    w=w2;
                    o=ex_seekOperator(dl,w);
                    if(o!=NULL)
                    {

                        // note the peephole here, if it is a close paren don't change state
                  // but instead call the close parenthesis evaluator
                        valid=ex_clearMonadicSuffixes(iStack,oStack,m);
                        if(valid)
                        {
                           if(o->precedence!=-20)
                           {
                              valid=ex_stackPrecedence(iStack,oStack,m,o);
                              dwState=TS_NORMAL;
                           }
                           else
                           {
                        // This was a close paren ( which has precedence of -2
                              valid=o->evaluate(iStack,oStack,m);
                           }
                        }
                    }
                    else
                    {
                        w=w2;
                        // what we have wasn't another monadic suffix
                        // or a dyadic operator. So we have one chance at mutation
                        ExprOperator *pMutant=NULL;
                        ExprOperator *pLast=oStack.Peek();

                        if(pLast)
                        {
                           pMutant=pLast->Mutant();
                        }

                        if(pMutant)
                        {
                           oStack.Pop();
                           valid=ex_clearMonadicSuffixes(iStack,oStack,m);
                           if(valid)
                           {
                              valid=ex_stackPrecedence(iStack,oStack,m,pMutant);
                           }
                           dwState=TS_NORMAL;
                        }
                        else
                        {
                           valid=PFBoolean::b_false;
                        }
                    }
                }
            }
            break;
        }
    }

    // note after clkearing, we are clear!

    if(valid && (dwState==TS_DYADIC))    // we ended in the right syntax
    {
        if(ex_clearMonadicSuffixes(iStack,oStack,m))
         if(ex_stackPrecedence(iStack,oStack,m))  // no operator to stack just unwind
            if((iStack.GetSize()==1)&&(oStack.GetSize()==0))
               retval=iStack.Pop();
    }
// if(testRecur) ex_addToRecurrence(retval);
    return(retval);
}

//==============================================================
// the simple expression evaluator works as follows, look for
// expressions aN+-b, where a and b are integers
//==============================================================

PFBoolean ex_getchain(const PFString &c,unsigned long &a,long &b)
{
// parse aN+b
   DWORD npos=c.Find("N");
   if((npos==NOT_FOUND)||(npos>=c.GetLength()-2)) return(PFBoolean::b_false); // if not there, or at the end
   char s=c[npos+1];
   if((s!='+')&&(s!='-')) return(PFBoolean::b_false);             // don't understand the sign

   PFString ap,bp;
   ap=c.Left(npos);
   bp=c.Mid(npos+2);
   if(ap=="") ap="1";
// check ab, bp are decimal
   a=atoi(ap);
   b=atoi(ap);
   if((a==0)||(b==0)) return(PFBoolean::b_false);
   return(PFBoolean::b_true);
}

PFString ex_setchain(unsigned long a,long b)
{
   PFString r,s;
   PFString v;

   if(a!=1)
   {
      v.Set(a);
      r=v+"N";
   }
   else
   {
      r="N";
   }

   if(b>0)
   {
      r+="+";
   }
   else
   {
      b=-b;
   }

   if(b!=0)
   {
      v.Set(b);
      s=r+v;
   }

   return(s);
}

PFBoolean ex_parseArguments(PFString &w,PFStringArray& tfArguments)
{
   // w begins with an open brace. The challenge that awaits us is to chase to the close brace, picking up
   // commas, keeping our parenthetical wrapping straight, and adding PFTextLines to tfArguments
   PFBoolean bValid=PFBoolean::b_true;
   PFBoolean bScanning=PFBoolean::b_true;
   DWORD dwParenthesis=0;
   DWORD dwCloseQuote;
   PFString sArgument;

   while(bValid && bScanning && !w.IsEmpty())
   {
      TCHAR c=(LPCTSTR(w))[0];
      w=w.Mid(1);

      switch(c)
      {
      case '(':
         ++dwParenthesis;
         if(dwParenthesis==1)
         {
            sArgument="";        // this is a new guy
         }
         else
         {
            sArgument+=c;
         }
         break;
      case ')':
         --dwParenthesis;
         if(dwParenthesis==0)
         {
            tfArguments.Add(sArgument);
            bScanning=PFBoolean::b_false;
         }
         else
         {
            sArgument+=c;
         }
         break;
      case ',':
         if(dwParenthesis==1)
         {
            tfArguments.Add(sArgument);
            sArgument="";
         }
         else
         {
            sArgument+=c;
         }
         break;
      case '\"':
         dwCloseQuote=w.Find(c);
         if(dwCloseQuote==NOT_FOUND)
         {
            bValid=PFBoolean::b_false;
         }
         else
         {
            sArgument+=c;
            sArgument+=w.Left(dwCloseQuote+1);
            w=w.Mid(dwCloseQuote+1);
         }
         break;
      default:
         if(dwParenthesis>=1)
         {
            sArgument+=c;
         }
         break;
      }
   }

   return bValid;
}

PFBoolean IsFirstSymbolCharacter(TCHAR c)
{
   PFBoolean bRetval=PFBoolean::b_false;

   if(
      ((c>='A')&&(c<='Z'))
   || ((c>='a')&&(c<='z'))
   || ((c=='_'))
   )
   {
      bRetval=PFBoolean::b_true;
   }
   return bRetval;
}

PFBoolean IsSymbolCharacter(TCHAR c)
{
   PFBoolean bRetval=PFBoolean::b_false;

   if(
      ((c>='A')&&(c<='Z'))
   || ((c>='a')&&(c<='z'))
   || ((c>='0')&&(c<='9'))
   || ((c=='_'))
   )
   {
      bRetval=PFBoolean::b_true;
   }
   return bRetval;
}

PFBoolean IsIntegerCharacter(TCHAR c)
{
   PFBoolean bRetval=PFBoolean::b_false;

   if(
      ((c>='0')&&(c<='9'))
   )
   {
      bRetval=PFBoolean::b_true;
   }
   return bRetval;
}
