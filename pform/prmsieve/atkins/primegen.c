#include "primegen.h"
#include "primegen_impl.h"
#include "../types/int64.h"
#include "../tables/twiddles.h"
#include <stdio.h>

#if 0
#define profile(var, val)
#else
#define profile(var, val) var+=val;
#endif
#if 1
#define printprofile(VARS)
#else
#define printprofile(VARS) printf VARS
#endif


static void clear(register bufLine*buf)
{
  register int i;
  register int j;

  for (j = 0;j < 16;++j) {
    for (i = 0;i < B32;++i)
      (*buf)[i] = (uint32) (~0);
    ++buf;
  }
}
static void doit4prepare(int64 start, freesteps_s*ret)
{
  register long x=45; /* 15 is max in doit4, *2 +15 */
  register int count=ret->xsteps;
  register long i;
  int xsteps45;

  int64 origstart = (start += ret->xsteps45 + (ret->idelta));
  long origx = (x += (ret->xdelta));

  if(start < -1000000000)
  {
    start += 1000000000;
    while (start < 0) { ++count; start += x; x += 30; }
    start -= 1000000000;
  }
  i = start;
  
  while (i < B) { ++count; i += x; x += 30; }
  start=i;

  xsteps45=count*45;
  ret->xsteps = count;
  ret->xdelta += (x-origx);
  ret->idelta += (start-origstart)+(ret->xsteps45-xsteps45);
  ret->xsteps45 = xsteps45;
}
static void doit4(register uint32 *a,register long x,register long y,int64 start,
		  freesteps_s const *freesteps)
{
  register long i;
  int blockwidth=1; /* should be 0 */
  long blockarea=0;
  long blockareasmalldelta=0;
  long blockareabigdelta=0;
  long deltadelta=30;
  /* simple profiling */
  int counttrivialsteps=0, /* trivial loops, what the normal thing does */
      countprepsteps=0,    /* our setup loop, quite long */
      countsmallsteps=0,   /* our small fairly simple steps */
      countbigsteps=0,     /* our longer but fairly simple steps... */
      countavoidedsteps=0, /* ... have replaced these from the naive version */
      countbacktracks=0,   /* these cost - we went too far */
      counttoggles=0,      /* can't improve on this - the work */
      countxsteps=0;       /* our original optimisation reduced this */
   
  /* printf("doit4(&, %ld, %ld, %lld, {%i})\n", x, y, start, (freesteps->xsteps)); */
  x += x; x += 15;
  y += 15;

  /* need simulate quadratic steps of i 
   * 1 step -> i += x               or 30*0 (=1*0/2)
   * 2 steps-> i += x + x+30        or 30*1 (=2*1/2)
   * 3 steps-> i += x + x+30 + x+60 or 30*3 (=3*2/2)
   */
  start += x*freesteps->xsteps + freesteps->idelta;
  /* need to simulate xfreesteps steps of x */
  x += freesteps->xdelta;

  i = start;
  /* now we need to step forward typically 0 or 1 more times. */
  while (i < B) { profile(countxsteps,1); i += x; x += 30; }

  /* Phase 1
   * initially expect 0s at low L
   */
  while((blockwidth<=0) && ((x -= 30) > 15))
  {
    i -= x;

    /* phase 1 ends as i drops below 0 */
    if(i<0)
    {
      /* no point in counting up twice - remember start point
       * and work out how much i and y moved afterwards
       */
      blockarea=i; blockareasmalldelta=y; 
      do 
      {
	++blockwidth; 
	i += y; 
	y += 30; 
	blockareabigdelta+=deltadelta;
	deltadelta+=60; 
      } while(i<0);
      blockarea = i-blockarea+blockareabigdelta;
      blockareasmalldelta = y-blockareasmalldelta;
      profile(countprepsteps,blockwidth);
    }

    /* gratuitious scope for register colouring hints */
    {
      register long i0=i;
      register long y0=y;
      while (i0 < B) 
      {
        profile(counttoggles,1);
        a[i0>>5] ^= BIT32(i0);
        i0 += y0; y0 += 30;
      }
    }
  }

  if(x<=15) goto outahere4;
  /* printf("PHASE 2 - %i loops expected\n", blockwidth);*/

  /* phase 2 only works best while the block is > 2 or >=2 or 1 maybe? */
  while((blockwidth>=2) && ((x -= 30) > 15))
  {
    i -= x;

    /* phase 2 - OK we are jumping forward */
    profile(countbigsteps,1);
    profile(countavoidedsteps,blockwidth);

    /* if we will go down and do extras, why go down?
     * hence we compare to y not 0 
     */
    /*if((i+=blockarea) < 0)*/
    if((i+=blockarea) < y)  
    {
/*       printf("\nCan jump %i steps from %li starting with %li", */
/* 	     blockwidth, i, y); */
      blockarea+=blockareabigdelta;
      y+=blockareasmalldelta;
      
      /* may need some extra steps */
      while (i < 0) 
      {
	profile(countsmallsteps,1);
	i += y; 
	y += 30; 
	blockarea+=blockareasmalldelta;
      }
    }
    else
    {
      y+=blockareasmalldelta;

#if 0
      printf("\nCannot %i steps to i=%li y=%li too big (area=%li+=%li/%li)\n",
	     blockwidth, i, y, blockarea, blockareabigdelta, blockareasmalldelta);
#endif

      /* we know we've gone too far */
      while(i>=0)
      {
        y-=30;  /* y=last column height, final. */
	i-=y;   /* i=start+(w-1)columns, final. */
	blockarea-=y; /* block no longer has last column, temp. */
	deltadelta-=60;
	blockareabigdelta-=deltadelta; /* are one thinner and lower */
	blockareasmalldelta-=30;
	--blockwidth;

#if 0
	printf("try %i steps to i=%li y=%li (area=%li+=%li/%li)\n",
	       blockwidth, i, y, blockarea, blockareabigdelta, blockareasmalldelta);
#endif
      
	profile(countbacktracks,1);
      }
      /* now we know we can safely apply it, safely apply it. */
      blockarea+=blockareabigdelta;

      /* we know there will only be one of the following loops
       * to get i back above zero again
       */
      profile(countsmallsteps,1);
      i += y; 
      y += 30; 
      blockarea+=blockareasmalldelta;
    }

    /* gratuitious scope for register colouring hints */
    {
      register long i0=i;
      register long y0=y;
      while (i0 < B) 
      {
        profile(counttoggles,1);
        a[i0>>5] ^= BIT32(i0);
        i0 += y0; y0 += 30;
      }
    }
  }

  if(x<=15) goto outahere4; 

  /* phase 3 only works best while there are few or no steps forward */
  while((x -= 30) > 15)
  {
    i -= x;

    /* phase 3 - OK we are stepping forward */
    while (i < 0) 
    {
      profile(counttrivialsteps,1);
      i += y; 
      y += 30; 
    }
    /* gratuitious scope for register colouring hints */
    {
      register long i0=i;
      register long y0=y;
      while (i0 < B) 
      {
        profile(counttoggles,1);
        a[i0>>5] ^= BIT32(i0);
        i0 += y0; y0 += 30;
      }
    }
  }  


outahere4:
  printprofile(("triv=%u prep=%u small=%u big=%u avoided=%u backtracks=%u toggles=%u x=%u\n",
	counttrivialsteps,
	countprepsteps,
	countsmallsteps,
	countbigsteps,
	countavoidedsteps,
	countbacktracks,
	counttoggles,
	countxsteps));


  return;
}

static void doit6prepare(int64 start, freesteps_s*ret)
{
  register long x=14; /* 9 is max in doit6, +5 */
  register int count=ret->xsteps;
  register long i;
  int xsteps45;

  int64 origstart = (start += ret->xsteps45 + (ret->idelta));
  long origx = (x += (ret->xdelta));
  if(start < -1000000000)
  {
    start += 1000000000;
    while (start < 0) { ++count; start += x; x += 10; }
    start -= 1000000000;
  }
  i = start;
  while (i < B) { ++count; i += x; x += 10; }
  start=i;
  xsteps45=count*14;
  ret->xsteps = count;
  ret->xdelta += (x-origx);
  ret->idelta += (start-origstart)+(ret->xsteps45-xsteps45);
  ret->xsteps45 = xsteps45;
}
static void doit6(register uint32 *a,register long x,register long y,int64 start,
		  freesteps_s const *freesteps)
{
  long i0;
  long y0;
  register long i;
  int count1=0, count2=0, count3=0;   
  x += 5;
  y += 15;

  start += x*freesteps->xsteps + freesteps->idelta;
  x += freesteps->xdelta;
   
  i = start;
  while (i < B) { ++count1; i += x; x += 10; }

  for (;;) {
    x -= 10;
    if (x <= 5)
    {
      return;
    }
    i -= x;

    while (i < 0) { ++count2; i += y; y += 30; }
    
    i0 = i; y0 = y;
    while (i < B) {
      ++count3;
      a[i>>5] ^= BIT32(i);
      i += y; y += 30;
    }
    i = i0; y = y0;
  }
}

static void doit12prepare(int64 start, freesteps_s*ret)
{
  register long x=15; /* 10 is max in doit12, +5 */
  register int count=ret->xsteps;
  register long i;
  int xsteps45;
  int64 origstart = (start += ret->xsteps45 + (ret->idelta));
  long origx = (x += (ret->xdelta));
  if(start < -1000000000)
  {
    start += 1000000000;
    while (start < 0) { ++count; start += x; x += 10; }
    start -= 1000000000;
  }
  i = start;
  while (i < 0) { ++count; i += x; x += 10; }
  start=i;
  xsteps45=count*15;
  ret->xsteps = count;
  ret->xdelta += (x-origx);
  ret->idelta += (start-origstart)+(ret->xsteps45-xsteps45);
  ret->xsteps45 = xsteps45;
}
static void doit12(register uint32 *a,register long x,register long y,int64 start,
		   freesteps_s const *freesteps)
{
  long i0;
  long y0;
  register long i;
  int count1=0, count2=0, count3=0;   
  x += 5;

  start += x*freesteps->xsteps + freesteps->idelta;
  x += freesteps->xdelta;
   
  i = start;
  while (i < 0) { ++count1; i += x; x += 10; }

  y += 15;
  x += 10;

  for (;;) {
    while (i >= B) {
      if (x <= y)
      {
        return;
      }
      ++count2;
      i -= y;
      y += 30;
    }
    i0 = i;
    y0 = y;
    while ((i >= 0) && (y < x)) {
      ++count3;
      a[i>>5] ^= BIT32(i);
      i -= y; y += 30;
    }
    i = i0;
    y = y0;
    i += x - 10;
    x += 10;
  }
}

static const int deltainverse[60] = {
 -1,PRIMEGEN_WORDS * 0,-1,-1,-1,-1
,-1,PRIMEGEN_WORDS * 1,-1,-1,-1,PRIMEGEN_WORDS * 2
,-1,PRIMEGEN_WORDS * 3,-1,-1,-1,PRIMEGEN_WORDS * 4
,-1,PRIMEGEN_WORDS * 5,-1,-1,-1,PRIMEGEN_WORDS * 6
,-1,-1                ,-1,-1,-1,PRIMEGEN_WORDS * 7
,-1,PRIMEGEN_WORDS * 8,-1,-1,-1,-1
,-1,PRIMEGEN_WORDS * 9,-1,-1,-1,PRIMEGEN_WORDS * 10
,-1,PRIMEGEN_WORDS *11,-1,-1,-1,PRIMEGEN_WORDS * 12
,-1,PRIMEGEN_WORDS *13,-1,-1,-1,PRIMEGEN_WORDS * 14
,-1,-1                ,-1,-1,-1,PRIMEGEN_WORDS * 15
} ;

static void squarefree1big(bufLine*buf,uint64 base,uint32 q,uint64 qq)
{
  uint64 i;
  uint32 pos;
  int n;
  uint64 bound = base + 60 * B;

  while (qq < bound) {
    if (bound < 2000000000)
      i = qq - (((uint32) base) % ((uint32) qq));
    else
      i = qq - (base % qq);
    if (!(i & 1)) i += qq;

    if (i < B * 60) { 
      pos = i;
      n = deltainverse[pos % 60];
      if (n >= 0) {
        pos /= 60;
        (*buf)[n + (pos >> 5)] |= BIT32(pos);
      }
    }

    qq += q; q += 1800;
  }
}

static void squarefree1(register bufLine*buf,uint64 L,uint32 q)
{
  uint32 qq;
  register uint32 qqhigh;
  uint32 i;
  register uint32 ilow;
  register uint32 ihigh;
  register int n;
  uint64 base;

  base = 60 * L;
  qq = q * q;
  q = 60 * q + 900;

  while (qq < B * 60) {
    if (base < 2000000000)
      i = qq - (((uint32) base) % qq);
    else
      i = qq - (base % qq);
    if (!(i & 1)) i += qq;

    if (i < B * 60) { 
      qqhigh = qq / 60;
      ilow = i % 60; ihigh = i / 60;
  
      qqhigh += qqhigh;
      while (ihigh < B) {
        n = deltainverse[ilow];
        if (n >= 0)
          (*buf)[n + (ihigh >> 5)] |= BIT32(ihigh);

        ilow += 2; ihigh += qqhigh;
        if (ilow >= 60) { ilow -= 60; ihigh += 1; }
      }
    }

    qq += q; q += 1800;
  }

  squarefree1big(buf,base,q,(uint64)qq);
}

static void squarefree49big(bufLine*buf,uint64 base,uint32 q,uint64 qq)
{
  uint64 i;
  uint32 pos;
  int n;
  uint64 bound = base + 60 * B;

  while (qq < bound) {
    if (bound < 2000000000)
      i = qq - (((uint32) base) % ((uint32) qq));
    else
      i = qq - (base % qq);
    if (!(i & 1)) i += qq;

    if (i < B * 60) { 
      pos = i;
      n = deltainverse[pos % 60];
      if (n >= 0) {
        pos /= 60;
        (*buf)[n + (pos >> 5)] |= BIT32(pos);
      }
    }

    qq += q; q += 1800;
  }
}

static void squarefree49(register bufLine*buf,uint64 L,uint32 q)
{
  uint32 qq;
  register uint32 qqhigh;
  uint32 i;
  register uint32 ilow;
  register uint32 ihigh;
  register int n;
  uint64 base = 60 * L;

  qq = q * q;
  q = 60 * q + 900;

  while (qq < B * 60) {
    if (base < 2000000000)
      i = qq - (((uint32) base) % qq);
    else
      i = qq - (base % qq);
    if (!(i & 1)) i += qq;

    if (i < B * 60) { 
      qqhigh = qq / 60;
      ilow = i % 60; ihigh = i / 60;
  
      qqhigh += qqhigh;
      qqhigh += 1;
      while (ihigh < B) {
        n = deltainverse[ilow];
        if (n >= 0)
          (*buf)[n + (ihigh >> 5)] |= BIT32(ihigh);

        ilow += 38; ihigh += qqhigh;
        if (ilow >= 60) { ilow -= 60; ihigh += 1; }
      }
    }

    qq += q; q += 1800;
  }

  squarefree49big(buf,base,q,(uint64)qq);
}

/* squares of primes >= 7, < 240 */
uint32 qqtab[49] = {
  49,121,169,289,361,529,841,961,1369,1681
 ,1849,2209,2809,3481,3721,4489,5041,5329,6241,6889
 ,7921,9409,10201,10609,11449,11881,12769,16129,17161,18769
 ,19321,22201,22801,24649,26569,27889,29929,32041,32761,36481
 ,37249,38809,39601,44521,49729,51529,52441,54289,57121
} ;

/* (qq * 11 + 1) / 60 or (qq * 59 + 1) / 60 */
uint32 qq60tab[49] = {
  9,119,31,53,355,97,827,945,251,1653
 ,339,405,515,3423,3659,823,4957,977,6137
 ,1263,7789,1725,10031,1945,2099,11683,2341,2957
 ,16875,3441,18999,21831,22421,4519,4871,5113,5487
 ,31507,32215,35873,6829,7115,38941,43779,9117,9447,51567,9953,56169
} ;

static void squarefreetiny(register uint32 *a,uint32 *Lmodqq,int d)
{
  int j;
  register uint32 k;
  register uint32 qq;

  for (j = 0;j < 49;++j) {
    qq = qqtab[j];
    k = qq - 1 - ((Lmodqq[j] + qq60tab[j] * d - 1) % qq);
    while (k < B) {
      a[k>>5] |= BIT32(k);
      k += qq;
    }
  }
}

typedef struct { char index; char f; char g; char k; } todo;

static const todo for4[] = {
  {0,2,15,4} , {0,3,5,1} , {0,3,25,11} , {0,5,9,3}
, {0,5,21,9} , {0,7,15,7} , {0,8,15,8} , {0,10,9,8}
, {0,10,21,14} , {0,12,5,10} , {0,12,25,20} , {0,13,15,15}
, {0,15,1,15} , {0,15,11,17} , {0,15,19,21} , {0,15,29,29}
, {3,1,3,0} , {3,1,27,12} , {3,4,3,1} , {3,4,27,13}
, {3,6,7,3} , {3,6,13,5} , {3,6,17,7} , {3,6,23,11}
, {3,9,7,6} , {3,9,13,8} , {3,9,17,10} , {3,9,23,14}
, {3,11,3,8} , {3,11,27,20} , {3,14,3,13} , {3,14,27,25}
, {4,2,1,0} , {4,2,11,2} , {4,2,19,6} , {4,2,29,14}
, {4,7,1,3} , {4,7,11,5} , {4,7,19,9} , {4,7,29,17}
, {4,8,1,4} , {4,8,11,6} , {4,8,19,10} , {4,8,29,18}
, {4,13,1,11} , {4,13,11,13} , {4,13,19,17} , {4,13,29,25}
, {7,1,5,0} , {7,1,25,10} , {7,4,5,1} , {7,4,25,11}
, {7,5,7,2} , {7,5,13,4} , {7,5,17,6} , {7,5,23,10}
, {7,10,7,7} , {7,10,13,9} , {7,10,17,11} , {7,10,23,15}
, {7,11,5,8} , {7,11,25,18} , {7,14,5,13} , {7,14,25,23}
, {9,2,9,1} , {9,2,21,7} , {9,3,1,0} , {9,3,11,2}
, {9,3,19,6} , {9,3,29,14} , {9,7,9,4} , {9,7,21,10}
, {9,8,9,5} , {9,8,21,11} , {9,12,1,9} , {9,12,11,11}
, {9,12,19,15} , {9,12,29,23} , {9,13,9,12} , {9,13,21,18}
, {10,2,5,0} , {10,2,25,10} , {10,5,1,1} , {10,5,11,3}
, {10,5,19,7} , {10,5,29,15} , {10,7,5,3} , {10,7,25,13}
, {10,8,5,4} , {10,8,25,14} , {10,10,1,6} , {10,10,11,8}
, {10,10,19,12} , {10,10,29,20} , {10,13,5,11} , {10,13,25,21}
, {13,1,15,3} , {13,4,15,4} , {13,5,3,1} , {13,5,27,13}
, {13,6,5,2} , {13,6,25,12} , {13,9,5,5} , {13,9,25,15}
, {13,10,3,6} , {13,10,27,18} , {13,11,15,11} , {13,14,15,16}
, {13,15,7,15} , {13,15,13,17} , {13,15,17,19} , {13,15,23,23}
, {14,1,7,0} , {14,1,13,2} , {14,1,17,4} , {14,1,23,8}
, {14,4,7,1} , {14,4,13,3} , {14,4,17,5} , {14,4,23,9}
, {14,11,7,8} , {14,11,13,10} , {14,11,17,12} , {14,11,23,16}
, {14,14,7,13} , {14,14,13,15} , {14,14,17,17} , {14,14,23,21}
} ;

static const todo for6[] = {
  {1,1,2,0} , {1,1,8,1} , {1,1,22,8} , {1,1,28,13}
, {1,3,10,2} , {1,3,20,7} , {1,7,10,4} , {1,7,20,9}
, {1,9,2,4} , {1,9,8,5} , {1,9,22,12} , {1,9,28,17}
, {5,1,4,0} , {5,1,14,3} , {5,1,16,4} , {5,1,26,11}
, {5,5,2,1} , {5,5,8,2} , {5,5,22,9} , {5,5,28,14}
, {5,9,4,4} , {5,9,14,7} , {5,9,16,8} , {5,9,26,15}
, {8,3,2,0} , {8,3,8,1} , {8,3,22,8} , {8,3,28,13}
, {8,5,4,1} , {8,5,14,4} , {8,5,16,5} , {8,5,26,12}
, {8,7,2,2} , {8,7,8,3} , {8,7,22,10} , {8,7,28,15}
, {11,1,10,1} , {11,1,20,6} , {11,3,4,0} , {11,3,14,3}
, {11,3,16,4} , {11,3,26,11} , {11,7,4,2} , {11,7,14,5}
, {11,7,16,6} , {11,7,26,13} , {11,9,10,5} , {11,9,20,10}
} ;

static const todo for12[] = {
  {2,2,1,0} , {2,2,11,-2} , {2,2,19,-6} , {2,2,29,-14}
, {2,3,4,0} , {2,3,14,-3} , {2,3,16,-4} , {2,3,26,-11}
, {2,5,2,1} , {2,5,8,0} , {2,5,22,-7} , {2,5,28,-12}
, {2,7,4,2} , {2,7,14,-1} , {2,7,16,-2} , {2,7,26,-9}
, {2,8,1,3} , {2,8,11,1} , {2,8,19,-3} , {2,8,29,-11}
, {2,10,7,4} , {2,10,13,2} , {2,10,17,0} , {2,10,23,-4}
, {6,1,10,-2} , {6,1,20,-7} , {6,2,7,-1} , {6,2,13,-3}
, {6,2,17,-5} , {6,2,23,-9} , {6,3,2,0} , {6,3,8,-1}
, {6,3,22,-8} , {6,3,28,-13} , {6,4,5,0} , {6,4,25,-10}
, {6,6,5,1} , {6,6,25,-9} , {6,7,2,2} , {6,7,8,1}
, {6,7,22,-6} , {6,7,28,-11} , {6,8,7,2} , {6,8,13,0}
, {6,8,17,-2} , {6,8,23,-6} , {6,9,10,2} , {6,9,20,-3}
, {12,1,4,-1} , {12,1,14,-4} , {12,1,16,-5} , {12,1,26,-12}
, {12,2,5,-1} , {12,2,25,-11} , {12,3,10,-2} , {12,3,20,-7}
, {12,4,1,0} , {12,4,11,-2} , {12,4,19,-6} , {12,4,29,-14}
, {12,6,1,1} , {12,6,11,-1} , {12,6,19,-5} , {12,6,29,-13}
, {12,7,10,0} , {12,7,20,-5} , {12,8,5,2} , {12,8,25,-8}
, {12,9,4,3} , {12,9,14,0} , {12,9,16,-1} , {12,9,26,-8}
, {15,1,2,-1} , {15,1,8,-2} , {15,1,22,-9} , {15,1,28,-14}
, {15,4,7,-1} , {15,4,13,-3} , {15,4,17,-5} , {15,4,23,-9}
, {15,5,4,0} , {15,5,14,-3} , {15,5,16,-4} , {15,5,26,-11}
, {15,6,7,0} , {15,6,13,-2} , {15,6,17,-4} , {15,6,23,-8}
, {15,9,2,3} , {15,9,8,2} , {15,9,22,-5} , {15,9,28,-10}
, {15,10,1,4} , {15,10,11,2} , {15,10,19,-2} , {15,10,29,-10}
} ;

void primegen_sieve(primegen *pg)
{
  bufLine*buf;
  uint64 L;
  int i;
  uint32 Lmodqq[49];

  buf = pg->buf;
  L = pg->L;

  if (L > 2000000000)
    for (i = 0;i < 49;++i)
      Lmodqq[i] = L % qqtab[i];
  else
    for (i = 0;i < 49;++i)
      Lmodqq[i] = ((uint32) L) % qqtab[i];

  clear(buf);

  doit4prepare((int64)(29-L),&pg->freesteps4);
  for (i = 0;i < 16;++i)
    doit4(buf[0],for4[i].f,for4[i].g,(int64) (for4[i].k - L), &pg->freesteps4);
  squarefreetiny(buf[0],Lmodqq,1);
  for (;i < 32;++i)
    doit4(buf[3],for4[i].f,for4[i].g,(int64) (for4[i].k - L), &pg->freesteps4);
  squarefreetiny(buf[3],Lmodqq,13);
  for (;i < 48;++i)
    doit4(buf[4],for4[i].f,for4[i].g,(int64) (for4[i].k - L), &pg->freesteps4);
  squarefreetiny(buf[4],Lmodqq,17);
  for (;i < 64;++i)
    doit4(buf[7],for4[i].f,for4[i].g,(int64) (for4[i].k - L), &pg->freesteps4);
  squarefreetiny(buf[7],Lmodqq,29);
  for (;i < 80;++i)
    doit4(buf[9],for4[i].f,for4[i].g,(int64) (for4[i].k - L), &pg->freesteps4);
  squarefreetiny(buf[9],Lmodqq,37);
  for (;i < 96;++i)
    doit4(buf[10],for4[i].f,for4[i].g,(int64) (for4[i].k - L), &pg->freesteps4);
  squarefreetiny(buf[10],Lmodqq,41);
  for (;i < 112;++i)
    doit4(buf[13],for4[i].f,for4[i].g,(int64) (for4[i].k - L), &pg->freesteps4);
  squarefreetiny(buf[13],Lmodqq,49);
  for (;i < 128;++i)
    doit4(buf[14],for4[i].f,for4[i].g,(int64) (for4[i].k - L), &pg->freesteps4);
  squarefreetiny(buf[14],Lmodqq,53);

  doit6prepare((int64)(17-L),&pg->freesteps6);
  for (i = 0;i < 12;++i)
    doit6(buf[1],for6[i].f,for6[i].g,(int64) (for6[i].k - L), &pg->freesteps6);
  squarefreetiny(buf[1],Lmodqq,7);
  for (;i < 24;++i)
    doit6(buf[5],for6[i].f,for6[i].g,(int64) (for6[i].k - L), &pg->freesteps6);
  squarefreetiny(buf[5],Lmodqq,19);
  for (;i < 36;++i)
    doit6(buf[8],for6[i].f,for6[i].g,(int64) (for6[i].k - L), &pg->freesteps6);
  squarefreetiny(buf[8],Lmodqq,31);
  for (;i < 48;++i)
    doit6(buf[11],for6[i].f,for6[i].g,(int64) (for6[i].k - L), &pg->freesteps6);
  squarefreetiny(buf[11],Lmodqq,43);

  doit12prepare((int64)(4-L),&pg->freesteps12);
  for (i = 0;i < 24;++i)
    doit12(buf[2],for12[i].f,for12[i].g,(int64) (for12[i].k - L), &pg->freesteps12);
  squarefreetiny(buf[2],Lmodqq,11);
  for (;i < 48;++i)
    doit12(buf[6],for12[i].f,for12[i].g,(int64) (for12[i].k - L), &pg->freesteps12);
  squarefreetiny(buf[6],Lmodqq,23);
  for (;i < 72;++i)
    doit12(buf[12],for12[i].f,for12[i].g,(int64) (for12[i].k - L), &pg->freesteps12);
  squarefreetiny(buf[12],Lmodqq,47);
  for (;i < 96;++i)
    doit12(buf[15],for12[i].f,for12[i].g,(int64) (for12[i].k - L), &pg->freesteps12);
  squarefreetiny(buf[15],Lmodqq,59);

  squarefree49(buf,L,247);
  squarefree49(buf,L,253);
  squarefree49(buf,L,257);
  squarefree49(buf,L,263);
  squarefree1(buf,L,241);
  squarefree1(buf,L,251);
  squarefree1(buf,L,259);
  squarefree1(buf,L,269);
}
