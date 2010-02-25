#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

//=======================================================
// Reciprocals and other such stuff
//=======================================================
#include "pfgluepch.h"
#include "recip.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef USE_GALLOT

void Reciprocal::div(Integer &x)
{
   Integer t;
   divmodcore(x,t);
}

void Reciprocal::mod(Integer &x)
{
   Integer t(x);
   divmodcore(t,x);
}

void Reciprocal::divmod(Integer &x,Integer &y)
{
   divmodcore(x,y);
}

Reciprocal::~Reciprocal()
{
}

void BufferReciprocal::set(const Integer &Y)
{
// buffer reciprocal does the setup as follows
   int y=lg(Y)+1;
// int x=y+y;

   q1=y-1;
   q2=y+1;

   N=Y;
   R=1;        // the reciprocal seed

   calculateReciprocal();
   calculateBuffers();
}

BufferReciprocal::BufferReciprocal(const Integer &Y)
{
   bmul=bdiv=NULL;
   set(Y);
}

BufferReciprocal::~BufferReciprocal()
{
   eraseBuffers();
}

void BufferReciprocal::eraseBuffers()
{
   if(bmul) delete[] bmul;
   if(bdiv) delete[] bdiv;
}

void BufferReciprocal::calculateBuffers()
{
// multiplication by the 'R' buffer takes a q2 bit integer as the other input (x=q1+q2).
// and R is itself around q2 bits in length
   eraseBuffers();
   R.bufferFFT(q2+2,bdiv,bsdiv);
// multiplication by 'N'
   unsigned int bits=(q1>q2)?q1:q2;
   N.bufferFFT(bits+2,bmul,bsmul);
}

void BufferReciprocal::divmodcore(Integer &Q,Integer &M)
{
   double maxError;

   if(Q==0)
   {
      M=0;     // inane stupidity error
   }
   else
   {

   Integer T(Q);     // need this later
// first do a simple check if we have enough bits to do all this
   unsigned long x=lg(Q)+1;
   if(x>q1+q2)
   {
      q2=x-q1; // adjust q2
      calculateReciprocal();
      calculateBuffers();
   }
// let rip, note the rounding
// x=(q1>0)?bit(Q,q1-1):0; Q>>=q1; Q+=x;
   Q>>=q1;
   Q.FFTmulB(R,0,maxError,bdiv,bsdiv);
// x=(q2>0)?bit(Q,q2-1):0; Q>>=q2; Q+=x;  // Q is our guess at the quotient
   Q>>=q2;
   M=Q;
   M.FFTmulB(N,0,maxError,bmul,bsmul);
   T-=M;                         // T is the remainder

   while(T<0)  {T+=N; --Q;}
   while(T>=N) {T-=N; ++Q;}

   M=T;

   }
}

void BufferReciprocal::calculateReciprocal()
{
   double maxError;

   int q=q1+q2;
// calculate R=2^q/N, note here q1 always equals floor(lg(N))
// so lg(R)=q-lg(N) is "a little larger" than q2
// lg(R)=q-lg(N)=q-q1-epsilon, 0<epsilon<1
// lg(R)=q2-epsilon,

   unsigned int qr=lg(R)+1;
   if(qr>q2) R>>=(qr-q2);
   else if(qr<q2) R<<=(q2-qr);   // our first guess is seeded now.

   Integer P(1);
   P<<=q;                  // 2^q
// the computation, at each step assume R=2^q/N+epsilon
// Note that R is a number of "about" q2+1 bits, N has q1+1 bits
// get a buffered FFT of N
   Complex *fb; unsigned int fs;
   unsigned int bits=(q1>q2)?q1:q2;
   N.bufferFFT(bits+2,fb,fs);
   Integer T;

   while(1)
   {
      T=R;                       // 2^q/N+epsilon
      T.FFTmulB(N,0,maxError,fb,fs);      // 2^q+N.e
      T-=P;                      // N.e
      T>>=q1;
      T.FFTmul(R,0,maxError);
      T>>=q2;                       // as if we were using it for real
      if(T==0) break;
      R-=T;
   }
// Now here R is pretty darn close
   T=R;                       // 2^q/N+epsilon
   T.FFTmulB(N,0,maxError,fb,fs);      // 2^q+N.e
   T-=P;                      // N.e
// check error, we are wanting it within N/2
   Integer N2=N>>1;
   while(T>N2) {T-=N;--R;}
   N2*=-1;
   while(T<N2) {T+=N;++R;}
   if(fb) delete[] fb;
}

#endif
