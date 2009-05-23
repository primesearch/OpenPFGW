/* Fairly Fast Sieve of Eratosthenes
 * (c) Phil Carmody 2000 fatphil@fatphil.org
 * 
 * NOTE: This sieve generates composites when it has reached 1G this is 
 * because presently it only throws out numbers which have factors below
 * 32768. This limit is a 'soft' limit, even if a little hairy to change.
 * It really ought not go above 65535, as we rely on quick squaring.
 * 
 * Based somewhat on Bernstein's eratspeed.
 * However, I believe that no two consecutive lines are Bernsteins!
 * i.e every function has had a total rewrite. All for a measly 10% gain.
 * 
 * Basically runs 8 sieves over the 8 different remainders modulo 30
 * i.e you never have to bother storing or processing 
 * multiples of 3 and 5 (and evens obviously)
 * Memory is therefore reduced to 8 bits per 30 candidates, whereas 
 * an ordinary sieve would use 15.
 * 
 * It uses a growing window.
 * The window is never larger than it needs to be
 * It tops out at 32767 (3509 primes) presently. 
 * This could change.
 * 
 * While the bitmap may be small, there's a huge amount of auxiliary
 * data to control the (effectively 8) window(s). 
 * Every access to this data is certainly a cache miss. 
 * However, the important thing is the bitmaps, and they stay in
 * cache the majority of the time.
 * 
 * Version History:
 *
 * 0.01 11/11/00 
 *    (Hmm, 11am too) Released for evaluation to OpenPFGW
 *    It sems to work, and seems fast, what else do you want?
 */
#include "erat.h"
#include "../types/uint32.h"
#include "../tables/twiddles.h"
#include "../tables/onemeg_cp7.h"
#include "../tables/pmod30.h"
#include "../compress/compri7.h"
#include "../arith/math16.h"

#include <stdlib.h>

#if defined(__cplusplus)
#define INLINE inline
#else
#define INLINE 
#endif



/* B32 32bit words must fit in the L1 cache for optimality */
#define ERAT_W 1000 /* must be even for double-counting */
#define ERAT_BITS (ERAT_W * 32)


/* We need to store the starting primes in a compact form 
 * If we use primes to 2^20 then the compressed table is 22960 bytes
 * And we need 82025 words to store it in.
 */
#define PRECALC_COMPRESSED_DATA 22960
#define PRECALC_EXPANDED_DATA 82025



/* These are the start/finish limits of the sieving window */
#define WINDOW_START 3  /* no 2, 3 or 5 */
#define WINDOW_END 3512 /* to <32768 but could be moved */
#define WINDOW_PRIMES (WINDOW_END-WINDOW_START)

/* When the window grows above 65535, then primes squared will overflow
 * so change this typedef to uint64 when our window becomes too large
 */
#if (WINDOW_END>6539)
#define SMALL_SQUARED_ARE_BIG
typedef uint64 smallprimesquared_t; /* need 64 bits */
typedef uint32 smallprime_t;        /* need 32 bits */
#else
typedef uint32 smallprimesquared_t; /* only need 32 bits */
typedef unsigned int smallprime_t;  /* only need 16 bits */
#endif
typedef uint64 prime_t;  /* returned primes are this type */
typedef uint32 row_t;    /* but we limit ourselves to 4G rows of 30 */


/* How many primes, maximum, will we enumerate in one go?
 * as the sieves are 32 bit words, and there are 8 sieves
 * bits per word * number of sieves = 256. 
 */
#define PRIME_BATCH_SIZE (32*8) 

/* CONSTANTS and LOOKUP TABLES */

/* table of small primes generated on bootstrap */
static smallprime_t smallptab[PRECALC_EXPANDED_DATA];
static smallprime_t*const windowptab=smallptab+3; /* don't use 2,3,5 */
static int maxsmallprimeindex;


/* this picks out the 8 useful numbers from q^2+2nq, n=0..14 */
static const uint32 getrightmodulus[8] = 
{ 
    /* Firstly the nybbles are in reverse order 
     * So look at the comments for a more readable interpretation
     * If p == x mod 30, look in row rd30tab[x]
     * To get the p^2+2np which has residue == y mod 30
     * Then n is in the 'column'/nybble given by rd30tab[y]
     */
    0xeb986530 /* { 0, 3, 5, 6, 8, 9, 11, 14 } */ , 
    0x5b0268c3 /* { 3, 12, 8, 6, 2, 0, 11, 5 } */ ,
    0x419d6a30 /* { 0, 3, 10, 6, 13, 9, 1, 4 } */ ,
    0x5e08923c /* { 12, 3, 2, 9, 8, 0, 14, 5 } */ , 
    0xa1076dc3 /* { 3, 12, 13, 6, 7, 0, 1, 10 } */ ,
    0xbe6295c0 /* { 0, 12, 5, 9, 2, 6, 14, 11 } */ , 
    0xa40d973c /* { 12, 3, 7, 9, 13, 0, 4, 10 } */ ,
    0x14679ac0 /* { 0, 12, 10, 9, 7, 6, 4, 1 } */
};

/* Quick generation of a fully populated 7 sieve for all 7 starting posns */
static const uint32 bit32_7s[7] = 
{
    0x81020408,
    0x40810204, 
    0x20408102, 
    0x10204081, 
    0x08102040, 
    0x04081020, 
    0x02040810, 
};

/* This needs reversing if we do 11 optimisation in the same way we do 7s
 * 
static const bit32_11s[11] = { 0x00400801, 0x00801002, 0x01002004, 0x02004008,
    0x04008010, 0x08010020, 0x10020040, 0x20040080, 0x40080100, 0x80100200,
    0x00200400 };
 */



/* Bugs? 
 * Find the first non-prime this generates, and factor it
 * Then set DEBUG_PRIME to be its smallest factor.
 * If that's 2, 3 or 5 then you are in deep doodoos.
 * Otherwise you will be told of every bit that this prime wants to clear
 * and a whole lot more.
 * It's a boon and a half!
 */

#include <stdio.h>
/*#define DEBUG_PRIME 1009*/

#if defined(DEBUG_PRIME)
#define PRINTF(ARGS) printf ARGS 
#define DEBUG(ARGS)  if(DEBUG_PRIME==0 || q==DEBUG_PRIME) PRINTF(ARGS)
#else
#define PRINTF(ARGS)
#define DEBUG(ARGS) 
#endif




/* WORKING DATA 
 *
 * Phase (uninit, qtab, bitmap)
 * Start position
 * Useless prime inicator (haven't reached p^2 yet)
 * Modulus mode (2n+1 is default, yielding rows of span 30)
 * Next values
 * Bitmap arrays
 * Batch prime extraction from the bitmap
 * Cursor in enumeration/primetable
 */

/* Phase
 * -2 = uninitialised, cursor means nothing
 * -1 = qtab mode, cursor is index into table (counts up)
 * 0..ERAT_BITS-1 = bitmap row, cursor is index into scratch table (counts down)
 */
static int phase=-2;

/* Start position */
static row_t Llo = (row_t)(0-1); /* garbage value, don't use */
static row_t Lhi = (row_t)(0-1); /* garbage value, don't use */
static prime_t Lbase = (prime_t)(0-1); /* ditto */

/* Modulus values */
static int modulusbase=2;
static int modulusremainder=1;
static uint32 modulusrowdelta=30;   /* = LCM(30,base) */
static unsigned char moduluscolumns=0xff;
static int modulusbadjs[10]; /* primes which don't help */
static int modulusbadprimes=0; /* how many primes don't help */

/* Tables to go between column 0-7 and residue mod 30 
 * When doing an ax+b sieve, these change from the defaults which
 * are copied from tables/dmod30.h
 */
static uint32 rowoffsets[8] = { 1, 7, 11, 13, 17, 19, 23, 29 } ;



/* Next values */
/* q-tab to 32768 bits means 120K, 14K/residue. int [WINDOW_PRIMES] */
static uint32* next[8];
static int uselessprime;
static int uselesscolumn; /* normally 0, put q^2 here before use */


/* Bitmap Windows */
/* each column in the window is now off the heap. int [ERAT_W] 
 * ERAT_W=1000 means 32K bitmap 4K/residue
 * Each column is separate.
 */
static uint32* window[8];


/* Batch prime extraction from the bitmap */
static prime_t batch[PRIME_BATCH_SIZE];


/* cursor in enumeration/primetable */
static int cursor;




/* Create the 8 windows */
static void allocateWindows()
{
    int i;
    /* Make sure we have exactly the windows we need, no more */
    for(i=0;i<8;++i)
    {
        if(moduluscolumns & (1<<i))
        {
            if(!window[i]) window[i] = (uint32*)malloc(ERAT_W*sizeof(uint32));
            if(!next[i]) next[i] = (uint32*)malloc(WINDOW_PRIMES*sizeof(uint32));
        }
        else
        {
            if(window[i]) { free(window[i]); window[i]=NULL; }
            if(next[i]) { free(next[i]); next[i]=NULL; }
        }
    }
}

static void freeWindows()
{
    int i;
    for(i=7;i>=0;--i)
    {
		// malloc? free?  God I hate C allocation!
        if(next[i])
		{
			free(next[i]); 
			next[i] = 0; 
		}
        if(window[i])
		{
			free(window[i]); 
			window[i] = 0;
		}
    }
}

/* bootstrap
 * 
 * This must be called exactly once, erat_init calls it.
 * Its job is to expand the compressed primes from scratch[] into qtab[]
 */
static void bootstrap()
{
    row_t x;
    row_t y;
    /* generate small primes, 2 3 5 7 being done separately */
    maxsmallprimeindex = 4 + cp7expandstream(smallptab+4, 
                                             onemeg_cp7, 
                                             onemeg_cp7_size());
    --maxsmallprimeindex;
    x = smallptab[maxsmallprimeindex]/30;
    while((y = smallptab[maxsmallprimeindex-1]/30) == x)
    {
        PRINTF(("#Small primes end at %u (%u:%u) rather than %u (%u:%u)\n",
           smallptab[maxsmallprimeindex-1], y,
           smallptab[maxsmallprimeindex-1]%30,
           smallptab[maxsmallprimeindex], x,
           smallptab[maxsmallprimeindex]%30));
        --maxsmallprimeindex;
    }
    PRINTF(("Small primes end after %u (%u:%u) but before %u (%u:%u)\n",
           smallptab[maxsmallprimeindex-1], y,
           smallptab[maxsmallprimeindex-1]%30,
           smallptab[maxsmallprimeindex], x,
           smallptab[maxsmallprimeindex]%30));

    smallptab[0]=2; smallptab[1]=3; smallptab[2]=5; smallptab[3]=7;

    allocateWindows();
}

void erat_modulus(int base, int rem)
{
    int gcd=gcd30[base%30];
    int extra30=30/gcd, extrabase=base/gcd;
    int numres=0;
    int i;
    int tmp;

    modulusbase=base;
    modulusremainder=rem%base;    /* Bring into range */
    modulusrowdelta=extrabase*30; /* LCM(30,base) */
    tmp=modulusremainder;
    for(i=0; i<extra30; ++i)
    {
        int trem=pmod30inv[tmp%30];
        if(trem>=0)
        {
            PRINTF(("#%u is %u mod 30\n", tmp, pmod30[trem]));
            rowoffsets[numres] = tmp;
	    ++numres;
        }
        tmp+=base;
    }
    moduluscolumns = (unsigned char)((1<<numres)-1);

    if(phase==-2) bootstrap(); 
    else allocateWindows();

    phase=-1;
    if(!moduluscolumns)
    {
	printf("%u X + %u is a non-workable sieve\n",
	       base, rem);
    }
}

/* Move both the front and back markers */
static INLINE void advanceRange()
{
    Llo=Lhi;
    Lhi+=ERAT_BITS;
}


/* initOneSmallPrime
 * Called by initNextVals, and fillBitmap
 * This is used to fit the right mod30 values into the 8 next slots
 */
static void initOneSmallPrime(int j, smallprime_t q, smallprimesquared_t qz)
{
    const smallprime_t q2=q+q;

    /* Our value is relative to the starting point Lbase */
    /* Basic form:
     * int skip=(base-qz)/(q*30);
     * qz += q30*skip+q30;   takes it past, maybe 15 step beyond
     * qz -= base;
     */
    /* qz += q30*(Lbase-qz)/(q30)+q30-Lbase;           */
    /* qz += ((Lbase-qz) - (Lbase-qz)%(q30))+q30-Lbase; */
    /* qz = Lbase - (Lbase-qz)%(q30)+q30-base;         */
    /* qz = q30 - (Lbase-qz)%(q30);                   */
    /* This uses 64bit instructions,
     * note - won't overflow as modulusrowdelta is always 30! 
     * could use a mod6432() optimisation if the compiler's crap
     */
    const uint32 q30 = q*modulusrowdelta;
    uint32 qzq =  (uint32)(q30 - (Lbase-qz)%(q30));
    

    /* rdtab[q%30] is which of the 8 classes its in.
     * map is therefore the list of 2q multiples that must be applied 
     * to q^2 in order to reach each of our 8 favourite residues
     */
    uint32 map = getrightmodulus[pmod30inv[q%30]];


    /* we know this is the smallest 
     * q^2+2.15.n.q  
     * greater than base for n>=0
     */
    /* generate the 15 next numbers with all possible odd remainders */
    uint32 ds[15];  /* the 15 possible values */
    int d=15;

    DEBUG(("#small init %i ^2=%lu -> %lu offset %lu class %08x;\n",
           q, qz, qzq+Lbase, qzq, map));
    ds[0]=qzq;
    while(d-- >0)
    { 
        if(qzq<q2) 
            qzq+=q30; 
        qzq-=q2; 
        ds[d] = qzq;
    }
    
    for(d=0; d<8; ++d) 
    {
        /* depending on the original remainder mod 30, the 
         * remainders will follow a set pattern. map shows where.
         */
	if(next[d])
	{
	    next[d][j] = (ds[map&0x0f])/modulusrowdelta;
	    DEBUG(("#S i=%d, j=%d: q=%u val=%lu(%lu)-%i -> %lu\n",
		   d, j, q, ds[map&0x0f], ds[map&0x0f]%30, pmod30[d], next[d][j]));
	}
        map>>=4; /* get the next nybble in place */
    }
}


static void initOneBigPrime(int j, smallprime_t q, smallprimesquared_t qz)
{
    const smallprime_t q2 = q+q;
    int d;
    uint32 ds[15];  /* the 15 possible values */
    
    int map = getrightmodulus[pmod30inv[q%30]]; /* as above */

    uint32 qzq = (uint32)(qz-Lbase);

    DEBUG(("#B init %i ^2=%lu has offset %lu and is class %08x;\n",
           q, qz, qzq, map));
        
    /* generate the 15 next numbers with all possible odd remainders */
    for(d=0; d<15; ++d) { ds[d] = qzq; qzq+=q2; }
    
    for(d=0; d<8; ++d) 
    {
        if(next[d])
        {
            /* depending on the original remainder mod 30, the 
             * remainders will follow a set pattern. map shows where.
             * Note we no longer subtract L here, as we subtracted L30 earlier
             */
            next[d][j] = (ds[map&0x0f])/modulusrowdelta;
            DEBUG(("#B i=%d, j=%d: q=%u val=%lu(%lu)-%i -> %lu\n",
                   d, j, q, 
                   ds[map&0x0f], ds[map&0x0f]%modulusrowdelta, 
                   pmod30[d], next[d][j]));
        }
        map>>=4; /* shift next nybble into place */
    }
}


/* initNextVals
 *
 * Here Lhi must be set to the number to start at divided by modulusrowdelta
 * This is only called once per sieve. i.e. restarting calls it.
 * This must only be called when not in modular mode
 */
static void initNextVals()
{
    /* let us assume we've reached Lhi*30 already by some other means
     * Therefore we need to create values pertinent from after that point
     */
    int i,j;
   
    /* set base. then set top to be the end of the rage we want */
    uint32 top;

    if(modulusbase!=2) { fprintf(stderr, "init modular mode?\n"); exit(1); }

	Lbase=Lhi;
    Lbase*=modulusrowdelta; /* know modulusrowdelta==30 */
    top=(uint32)(Lbase + ERAT_BITS*modulusrowdelta); /* ditto */
   
    /* Want a faster loop for bigger numbers which haven't
     * already been exceeded and a faster loop for smaller numbers
     * which need to catch up. Later...
     */
    for(j=0; j<WINDOW_PRIMES; ++j) 
    {
        const smallprime_t q = windowptab[j];
#if defined(SMALL_SQUARED_ARE_BIG)
#pragma warn Phil, this will overflow
	/* This requires 64 bits if we use a huge window */
#endif
        const smallprimesquared_t qz = (smallprimesquared_t)q*q;
        if(qz>=Lbase) break;
        initOneSmallPrime(j, q, qz);
    }
    /* Now munge our special bitmapping primes into a form ready for
     * the main loop, i.e. grab the explicit bit representation
     */
    for(i=0;i<8;++i)
	next[i][0]=bit32_7s[next[i][0]]; /* seven is special */
    
    for(; j<WINDOW_PRIMES; ++j) 
    {
        const smallprime_t q = windowptab[j];
#if defined(SMALL_SQUARED_ARE_BIG)
#pragma warn Phil, this will overflow
	/* This requires 64 bits if we use a huge window */
#endif
        const smallprimesquared_t qz = q*q;
        if(qz>=top) break;
        initOneBigPrime(j, q, qz);
    }
   
    PRINTF(("#[%i]=%u, ^2=%u > %lu makes sense to stop\n",
             j, windowptab[j], windowptab[j]*windowptab[j], top));
    uselessprime=j;

    /* now do oversize ones */
    for(; j<WINDOW_PRIMES; ++j) 
    {
        /* Our value is relative to the starting point L30 */
        const smallprime_t q = windowptab[j];
#if defined(SMALL_SQUARED_ARE_BIG)
#pragma warn Phil, this just is not going to work
        /* Stick top 32 bits in [0] and bottom 32 bits in [1]?  */
	/* Stick the _row_ of the square in [0]? Rows are 32bit */ 
	/* The latter works, but we need to fill in all 8 columns
	 * rather than just the useless column.
	 */
#else
        next[uselesscolumn][j]=q*q;
#endif
    }
}

static void initOneModuloPrime(int j, smallprime_t q, smallprimesquared_t qz,
			       uint32 rowbase)
{	
    const uint32 rowinv=expmod16(modulusrowdelta, q-2, q);
    int i;
    for(i=0; i<8; ++i)
    {
	if(!next[i]) 
	{
	    continue;
	}
	else if(rowinv==0)
	{
	    next[i][j]=(uint32)(0-1); /* this will never come into range */
	}
	else
	{
	    uint32 minusn= mulmod16(rowoffsets[i], rowinv, q);
	    uint32 n = minusn?(q-minusn):0;
	    uint32 up = rowoffsets[i]+n*modulusrowdelta;
	    PRINTF(("#%u+(_%u_=%u+%i)*%u=(%u)=%u(mod _%u_) %s\n#",
		    rowoffsets[i], n, rowbase, n-rowbase, modulusrowdelta,
		    up, up%q, q, n==0?"eeeek":""));
	    while(up<qz || n<rowbase) 
	    {
		n+=q;
		up+=q*modulusrowdelta;
		PRINTF(("."));
		DEBUG(("#%u+(_%u_=%u+%i)*%u=(%u)=%u(mod _%u_)\n#",
			rowoffsets[i], n, rowbase, n-rowbase, modulusrowdelta,
			up, up%q, q));
	    }
	    PRINTF(("#%u+(_%u_=%u+%i)*%u=(%u)=%u(mod _%u_)\n#",
		    rowoffsets[i], n, rowbase, n-rowbase, modulusrowdelta,
		    up, up%q, q));
	    next[i][j]=n-rowbase;
	}
    }
    if(!rowinv)
    {
	printf("[%i]=%u never does anything for p=%un+%u\n", 
	       j, q, modulusbase, modulusremainder); 
	modulusbadjs[modulusbadprimes++]=j;
    }
}

static void initNextValsModulo()
{
    prime_t top;
    int j, i;

    Lbase=Lhi;
	Lbase*=modulusrowdelta;
    top=Lbase + ERAT_BITS*modulusrowdelta;
    
    modulusbadprimes=0;
    modulusbadjs[0]=0;  /* unnecessary */

    for(j=0; j<WINDOW_PRIMES; ++j) 
    {
        const smallprime_t q = windowptab[j];
#if defined(SMALL_SQUARED_ARE_BIG)
#pragma warn Phil, this will overflow
	/* This requires 64 bits if we use a huge window */
#endif
        const smallprimesquared_t qz = q*q;
        if(qz>=top) { break; }
	initOneModuloPrime(j, q, qz, Lhi);
    }
    /* Now munge our special bitmapping primes into a form ready for
     * the main loop, i.e. grab the explicit bit representation
     */
    if(modulusbadprimes==0 || modulusbadjs[0]!=0)
    {
	for(i=0;i<8;++i)
	    if(next[i]) next[i][0]=bit32_7s[next[i][0]];
    }
    else
    {
	for(i=0;i<8;++i)
	    if(next[i]) next[i][0]=0; /* seven does nothing */
    }

    PRINTF(("#[%i]=%u, ^2=%u > %lu makes sense to stop\n",
             j, windowptab[j], windowptab[j]*windowptab[j], top));
    uselessprime=j;

    /* now do oversize ones */
    for(; j<WINDOW_PRIMES; ++j) 
    {
        /* Our value is relative to the starting point L30 */
        const smallprime_t q = windowptab[j];
#if defined(SMALL_SQUARED_ARE_BIG)
#pragma warn Phil, this just is not going to work
        /* Stick top 32 bits in [0] and bottom 32 bits in [1]? */
	/* Stick the _row_ of the square in [0]? Rows are 32bit */ 
#else
        next[uselesscolumn][j]=q*q;
#endif
    }
}

static void fillBitmap()
{
    int i;
    int j;
    
    /* First find out if any new primes would be useful 
     * briefly set top to where the end of the next block will be
     */
    prime_t top = Lbase+ERAT_BITS*modulusrowdelta;
    PRINTF(("#Entering Fill with Lbase=%lu, setting top= %lu\n", Lbase, top));

    while(   (uselessprime<WINDOW_PRIMES) 
	  && (next[uselesscolumn][uselessprime]<top))
    {
#if defined(SMALL_SQUARED_ARE_BIG)
#pragma warn Phil, this just is not going to work
	/* If prime_t is 64 bits, next[uselessclum] will have crashed and burned */
#endif

        PRINTF(("#Useless %i=%lu ^2=%lu now useful\n", 
                uselessprime, 
		windowptab[uselessprime],
		next[uselesscolumn][uselessprime]));
	if(modulusbase==2)
	{
	    initOneBigPrime(uselessprime, 
			    windowptab[uselessprime], 
			    next[uselesscolumn][uselessprime]);
	}
	else
	{
	    PRINTF(("#[%i]=%u is no longer useless, cross fingers\n", 
		   uselessprime, windowptab[uselessprime]));
	    initOneModuloPrime(uselessprime,
			       windowptab[uselessprime], 
			       next[uselesscolumn][uselessprime],
			       Llo);
	}
        ++uselessprime; /* the next one may well be useless */
    }

    PRINTF(("#Leaving [%i]=%lu ^2=%lu as useless\n", 
            uselessprime, 
	    windowptab[uselessprime], 
	    next[uselesscolumn][uselessprime]));

    for (i = 0;i < 8;++i) 
    {
        uint32 *nexti = next[i];

        /* If we are doing modular sieving, maybe skip this column */
        if(!nexti) {continue;}

        /* The principle behind this optimisation is that 7 will hit every 
         * word 4 or 5 times, why not write all those bits in one go. 
         * We can do that while we are initialising the buffer
         * POrig - only a single pre-loop with 7 only
         */
        /* gratuitous scope for register colouring hints */
        {
            register uint32 *buf = window[i];
            register uint32 bs7 = nexti[0]; /* seven's bits */
            register int k;
            
            /* register uint32 bs11 = bit32_11s[nexti[1]]; */ /* eleven */
	    { /*int q=7*/; DEBUG(("7 begines with bitmap %08x\n", bs7)); }

            for(k=ERAT_W; --k>=0;)
            {
                buf[k] = bs7; /* | bs11; */
                bs7 = (bs7<<4) | (bs7>>3);
                /* bs11 = (bs11<<1) | (bs11>>10); */
            }
            nexti[0] = bs7;
        }
        j=1;

        for (; j<uselessprime; ++j) 
        {
            register uint32 k = nexti[j];
            if (k < ERAT_BITS) 
            {
                register smallprime_t q = windowptab[j];
                register uint32 *buf = window[i];
                register int revk = ERAT_BITS-1-k;
                DEBUG(("#Mapping %lu's %lu onto %i=[%i:%i]\n#", 
                       q, k, revk, revk>>5, revk&31)); 
                do {
                    /*DEBUG(("!%u=[%u:%u]\t", revk, revk>>5, revk&31));*/
                    buf[revk>>5] |= BIT32(revk);
                    revk -= q;  /* go backwards */
                } while (revk >= 0);
                nexti[j] = -1-revk;
                DEBUG(("\n#finishes on %i, maps to %u, reduces to %lu\n",
                       revk, ERAT_BITS-1-revk, nexti[j]));
            }
            else
            {
                DEBUG(("#Dropping %lu's %lu to %lu\n", windowptab[j], k, k-ERAT_BITS)); 
                nexti[j] = k-ERAT_BITS;
            }
        } /* end for j */
    } /* end for i */

    PRINTF(("#Filled b=%lu, %u*Llo= %u\n", 
            Lbase, modulusrowdelta, Llo*modulusrowdelta));
}


static void prefetchBatch()
{
    uint32 b0, b1, b2, b3, b4, b5, b6, b7;
    uint32 mask;

    if(phase==0) 
    {
        advanceRange(); 
        fillBitmap();
        phase=ERAT_W; 
    }
    --phase;

    PRINTF(("#prefetching bits from word %u with offsets %u to %u\n",
	    phase, phase*32, phase*32+1));
        
    if(moduluscolumns == 0xff)
    {
        b0=~window[0][phase]; b1=~window[1][phase]; 
        b2=~window[2][phase]; b3=~window[3][phase];
        b4=~window[4][phase]; b5=~window[5][phase]; 
        b6=~window[6][phase]; b7=~window[7][phase];
    }
    else
    {
	/* disable non-existant window columns */
        b0=window[0]?~window[0][phase]:0;
	b1=window[1]?~window[1][phase]:0; 
        b2=window[2]?~window[2][phase]:0;
	b3=window[3]?~window[3][phase]:0;
        b4=window[4]?~window[4][phase]:0;
	b5=window[5]?~window[5][phase]:0; 
        b6=window[6]?~window[6][phase]:0; 
	b7=window[7]?~window[7][phase]:0;
    }

    Lbase+=modulusrowdelta*32;
    cursor=0;
    
    for (mask = 1;mask;mask <<= 1) 
    {
	Lbase-=modulusrowdelta;
	/*PRINTF(("#From %lu = bit %x: ", Lbase, mask));*/
/* laziness abounds - quick macro to print each bit found */
#define p(X) /*if(b##X & mask) PRINTF(("%i[%u]%lu ", X, cursor-1, batch[cursor-1]))*/
	if(b7&mask) batch[cursor++]=Lbase+rowoffsets[7]; p(7);
	if(b6&mask) batch[cursor++]=Lbase+rowoffsets[6]; p(6);
	if(b5&mask) batch[cursor++]=Lbase+rowoffsets[5]; p(5);
	if(b4&mask) batch[cursor++]=Lbase+rowoffsets[4]; p(4);
	if(b3&mask) batch[cursor++]=Lbase+rowoffsets[3]; p(3);
	if(b2&mask) batch[cursor++]=Lbase+rowoffsets[2]; p(2);
	if(b1&mask) batch[cursor++]=Lbase+rowoffsets[1]; p(1);
	if(b0&mask) batch[cursor++]=Lbase+rowoffsets[0]; p(0);
#undef p
	/*PRINTF(("\n"));*/
    }
    Lbase+=modulusrowdelta*32;
}

/* PUBLIC INTERFACE */

/* must impliment the following */
void erat_init()
{
   if(phase==-2) bootstrap();
   if(modulusbase==2)
   {
       phase=-1;
       cursor=0;
   }
   else
   {
#if 0
       /* The problem with this is that we are initialising too early */
       Lhi=0; /* have nothing calculated ab initio */
       initNextValsModulo();
       advanceRange();
       cursor=0; /* Force a refresh */
       phase=0; /* force bitmap generation from within prefetch */
#else
       /* Pretend to do phase -1, but make it zero-length */
       phase=-1;
       cursor=maxsmallprimeindex;
#endif
   }
}

// This code leaked like hell.  There was no way to "shut" down the erat
void erat_free()
{
	freeWindows();
	phase=-2;

	Llo = (uint32)(0-1);
	Lhi = (uint32)(0-1);
	Lbase = (uint32)(0-1);

	modulusbase=2;
	modulusremainder=1;
	modulusrowdelta=30;   /* = LCM(30,base) */
	moduluscolumns=0xff;
	modulusbadprimes=0; /* how many primes don't help */
}

   
uint64 erat_peek()
{
    /* we work on the "don't calculate it until you need it" principle
     * therefore we advance the pointer, work stuff out, then return
     * the value.
     * Except in phase -2, where it's easier to precalculate!
     */
    uint64 ret;
    if(phase==-1)
    {
	if(cursor == maxsmallprimeindex) 
	{ 
	    if(modulusbase==2)
	    {
		/* This can't be modular mode */
		Lhi=(smallptab[maxsmallprimeindex-1]/30)+1;
		initNextVals();
	    }
	    else
	    {
		Lhi=0; /* have nothing calculated ab initio */
		initNextValsModulo();
	    }
	    advanceRange();
	    cursor=0; /* Force a refresh */
	    phase=0; /* force bitmap generation from within prefetch */
	    /* I would like it to be known that in 12 years of C coding
	     * this is the first time I have ever used 'goto'.
	     * I have even worked in realtime systems where some people
	     * say "if goto is quicker and simpler, then it's OK".
	     * However, I resisted!
	     */
	    goto Label_IsBitmapPhase;
	}
	ret=smallptab[cursor];
    }
    else
    {
       Label_IsBitmapPhase:
	while(!cursor)
	{
	   /* This will run the bitmap if need be
	    * It will then fill the scratch buffer
	    * It then sets the cursor just past where the prime is
	    * it is possible for there to be no prime; if so, repeat.
	    */
	    prefetchBatch();
	}
	ret=batch[cursor-1]; /* now we step back to get the prime */
    }
    return ret;
}

uint64 erat_next()
{
    /* we work on the "don't calculate it until you need it" principle
     * therefore we advance the pointer, work stuff out, then return
     * the value.
     * Except in phase -2, where it's easier to precalculate!
     */
    uint64 ret;
    if(phase==-1)
    {
	if(cursor == maxsmallprimeindex) 
	{ 
	    if(modulusbase==2)
	    {
		/* This can't be modular mode */
		Lhi=(smallptab[maxsmallprimeindex-1]/30)+1;
		initNextVals();
	    }
	    else
	    {
		Lhi=0; /* have nothing calculated ab initio */
		initNextValsModulo();
	    }
	    advanceRange();
	    cursor=0; /* Force a refresh */
	    phase=0; /* force bitmap generation from within prefetch */
	    /* I would like it to be known that in 12 years of C coding
	     * this is the first time I have ever used 'goto'.
	     * I have even worked in realtime systems where some people
	     * say "if goto is quicker and simpler, then it's OK".
	     * However, I resisted!
	     */
	    goto Label_IsBitmapPhase;
	}
	ret=smallptab[cursor++];
    }
    else
    {
       Label_IsBitmapPhase:
	while(!cursor)
	{
	   /* This will run the bitmap if need be
	    * It will then fill the scratch buffer
	    * It then sets the cursor just past where the prime is
	    * it is possible for there to be no prime; if so, repeat.
	    */
	    prefetchBatch();
	}
	ret=batch[--cursor]; /* now we step back to get the prime */
    }
    return ret;
}

void erat_skipto(uint64 to)
{
    /* init has been called - we are either in modular or non-modular mode
     * If modular, then we are in 'big primes' from the outset
     * Else we may be in small primes
     */
    if((modulusbase==2) && (to<=smallptab[PRECALC_EXPANDED_DATA-1]))
    {
	/* small prime mode, non-modular */
	unsigned int ito=(uint32)(to);
	/* binary search hints. index is between p/16 and p/8+58 */
	unsigned int low=ito>>4;
	unsigned int high=(ito>>3)+58; /* MAGIC number see above */
	unsigned int mid;
        if(high>=PRECALC_EXPANDED_DATA) high=PRECALC_EXPANDED_DATA-1;
	
	while(low!=(mid=(low+high)>>1))
	{
	    (void)(ito<=smallptab[mid]?(high=mid):(low=mid));
	}
	/* need to be prepared to backtrack in odd cases */
/*	printf("%u %u %u\n", ito, high, smallptab[high]);	*/
	cursor=high-(ito<=smallptab[mid]);
    }
    else
    {
	/* big prime or modulo mode */
	Lhi=(uint32)(to/modulusrowdelta);
	if(modulusbase != 2)
	{
	    initNextValsModulo();
	}
	else
	{
	    initNextVals();
	}
	advanceRange();
	cursor=0; /* Force a refresh */
	phase=0; /* force bitmap generation from within prefetch */
    }

    /* This is wasteful, but it is simple. */
    while(erat_peek()<to)
    {
	erat_next();
    }      
}

/*
 * MISSING FUNCTIONS
 * 
 * In order of increasing priority
 */ 

