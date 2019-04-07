/*-------------------------------------------------------------------------
/                                                                         *
/ Erat_Mod.cpp  Copyright Jim Fougeron, 2000/2001.  All rights reseved.   *
/                                                                         *
/ Usage of this code by other developers is granted by the author, Jim    *
/ Fougeron, however, the author does require written notification before  *
/ this code is put into a project.  This copyright notice MUST be         *
/ maintained in its original form within the source code.  Also, if       *
/ the functionality of this code becomes a significant part of the        *
/ finished product/utility, then the author requires a cursory            *
/ acknowledgement in the program's "about" or "startup" text screen.      *
/                                                                         *
/ The author also reqests that all modifications to the code, and         *
/ bug fixes or porting issues be sent to him to incorporate into          *
/ on going versions of this code.                                         *
/                                                                         *
/-------------------------------------------------------------------------*/

// Description:
//
// This file implements a class around the modular sieve of Eratosthenes
// implemented by Jim Fougeron.
//

#include "pflibpch.h"

// #define TESTING

#include "erat_mod.h"
#include "bmap.cxx"

#if defined (TESTING)
#define PFPrintfStderr(a,b) fprintf(stderr, a, b)
#endif

//#define ENTER_EXIT_TEST

#if defined (ENTER_EXIT_TEST)
 #define ENTER(X) fprintf(stderr,"Enter %s\n",X)
 #define EXIT(X)  fprintf(stderr,"Exit %s\n",X)
#else
 #define ENTER(X)
 #define EXIT(X)
#endif
// Todo's
// 1.  Done
// 2.  Done.
// 3.  Trim as much as possible off setup (initilization) times.
// 4.  Keep a "state" version, where the first generation data is stored for a modulus.
// 5.  Done
// 6.  Create an array of uint32_t's needed for the FillSmallPrimes function for each nsp level.
// 7.  Add "exceptions" to the modular sieve.  These will be in the "conditions" set.  Allowable
//     exceptions are:  {f,#} which is all of the prime factors of # and {p,#} which are all
//     primes less equal to than #.
//
// Done's
// 1.  Allow more than N+1 modular sieves
// 2.  Too much work being done regenerating small primes.  The small primes themselves should be a
//     "safe" static member, and the only re-generated thing should be the ixt stuff.  That should
//     cut some time off of the initializaion.
// 3a. Made the SIEVE_BIT_SIZE a member variable.  It is also a 2 variable variable.  The constructor
//     allows setting 2 numers.  The first is the bit size for the bitmap used on the first sieve
//     window.  Once that window has been used, and the next window (and all subsequent windows) are
//     created, they use the second number.  There is also a way to set these values.   Doing this allows
//     one to use a much smaller than optimum number for the first window (say 16 bits).  The primes
//     contained within this window will catch "most" of the quickly factored composites.  If a second
//     primegen window is needed, then the higher size number kicks in.  20 bits is about optimal for
//     all Pentium PC's.   Since the candidate was not factored on the first window, setup taking a little
//     longer for each window will not matter, since the total throughput on the primegen will be optimal.
// 3b. Created an "adjustdepth" function so as to reduce the number of recursive calls to loadnext, et el
//     needed during construction.
// 3c. created a asm version of modInv.  This is about 3.5x faster.  Cuts down about 20% startup overhead.
// 5.  Added a -m for maximal value to the test app.
//
// Bugs fixed
// 1.  Put the counter for the SmallPrimeSieveTable into the class object.   Before this was a "global"
//     being that it was a member of the global object spTab.  Now it lives in each Erat_Mod object.
// 2.  gcc-beos had a code optimizer generation bug.  ChrisN changed the code to work around this, and
//     his fixes are actually faster than before (although the code is not as elegant as before)
// 3.  forgot to set maxSmallPrime to the max in FillSmallPrimes.  This was not a "bug", but certainly
//     caused this function to work much more often that it needed to.
//
uint32_t *Erat_Mod::primetable;
uint32_t  Erat_Mod::maxSmallPrime;

// Timings on my PII-400    2^16384+1 to 8^16384+1 from 0 to 42 bits
// SIEVE_BIT_SIZE 18    81.260s
// SIEVE_BIT_SIZE 19    77.565s
// SIEVE_BIT_SIZE 20    76.911s  ** best for this PC
// SIEVE_BIT_SIZE 21    79.514s
// SIEVE_BIT_SIZE 22    83.436s

// Timings on my Ath-750    2^16384+1 to 8^16384+1 from 0 to 42 bits
// SIEVE_BIT_SIZE 18
// SIEVE_BIT_SIZE 19
// SIEVE_BIT_SIZE 20
// SIEVE_BIT_SIZE 21
// SIEVE_BIT_SIZE 22


struct SmallPrimeSieveTable
{
   enum {nMax=10};
   int nsp[nMax];
   uint64_t u64Pure[nMax];   // this is MaxPrime^2
   double dBits[nMax];
} static spTab = {
   {5000,           10000,          25000,          50000,           100000,           200000,           400000,            600000,            1000000,            1500000         },
#ifdef _MSC_VER
   {2363029321,     10971096049,    82447656769,    374491369849,    1689274677841,    7563385525921,    33641612419321,    80289968858089,    239812076741689,    570232382852521 },
#else
   {2363029321ULL,  10971096049ULL, 82447656769ULL, 374491369849ULL, 1689274677841ULL, 7563385525921ULL, 33641612419321ULL, 80289968858089ULL, 239812076741689ULL, 570232382852521ULL },
#endif
   {31.1,           33.3,           36.3,           38.4,            40.6,             42.7,             44.9,              46.1,              47.7,               49.0            },
   };

/*
SmallPrimeSieveTable *spTab =
{
   {5000,   2363029321     }, // 31.1 bits pure.  MaxPrime is 48611
   {10000,  10971096049    }, // 33.3 bits pure.  MaxPrime is 104743
   {25000,  82447656769    }, // 36.3 bits pure.  MaxPrime is 287137
   {50000,  374491369849   }, // 38.4 bits pure.  MaxPrime is 611957
   {100000, 1689274677841  }, // 40.6 bits pure.  MaxPrime is 1299721
   {200000, 7563385525921  }, // 42.7 bits pure.  MaxPrime is 2750161
   {400000, 33641612419321 }, // 44.9 bits pure.  MaxPrime is 5800139
   {600000, 80289968858089 }, // 46.1 bits pure.  MaxPrime is 8960467
   {1000000,239812076741689}, // 47.7 bits pure.  MaxPrime is 15485867
   {1500000,570232382852521}, // 49.0 bits pure.  MaxPrime is 23879539
   {0, 0}                  // Stop here. Above this we simply use a 49 bit pure sieve.
}
*/

#ifdef _MSC_VER
#pragma warning( push )
// ignore the warning about not returning a value.  The ASM version of the egcd does return a value (it fills in eax).
#pragma warning( disable : 4035 )
#endif

// egcd
uint32_t Erat_Mod::modInv(const uint32_t x, const uint32_t m)
{
#if !defined(_MSC_VER) || defined(_64BIT)
   // C version takes about 3089 clock cycles for modInv(21314, 158617)
   int d0 = (int)x;
   int d1 = (int)m;
   int s0 = 1;
   int s1 = 0;
   while (d1 != 0)
   {
      const int q = d0 / d1;
      const int t0 = d1;
      d1 = d0 - q * d1;
      d0 = t0;
      const int t1 = s1;
      s1 = s0 - q * s1;
      s0 = t1;
   }
   if (d0 < 0) { d0 = -d0; s0 = -s0; }
   if (s0 < 0) s0 += m;
   if (d0 != 1) s0 = 0; // impossible if m is prime
   return s0;
#else
   // Asm version takes about 600 clock cycles for modInv(21314, 158617)
   // This code is probably not optimal, but it is pretty fast (much better than the C)
   int q, t0, t1;
   __asm {

      mov ebx, DWORD PTR x // d0
      mov esi, 1           // s0
      xor edi, edi         // s1
      mov ecx, DWORD PTR m // d1

      // Assume that this function will NEVER be called with m==0
      // do {
      DoLoop:;

      // q = d0/d1
      mov   eax, ebx
      // t0 = d1
      mov   DWORD PTR t0, ecx
      cdq
      idiv  ecx
      mov   DWORD PTR q, eax

      // d1 = d0-q*d1
      imul  ecx
      mov   edx, ebx
      // d0 = t0
      mov   ebx, DWORD PTR t0
      sub   edx, eax
      mov   ecx, edx

      // t1 = s1
      mov   DWORD ptr t1, edi

      //s1 = s0 - q * s1;
      mov   eax, DWORD ptr q
      imul  edi
      mov   edx, esi
      sub   edx, eax
      mov   edi, edx

      //s0 = t1;
      mov   esi, DWORD PTR t1

      // } while (d1 != 0);
      or    ecx, ecx
      jnz   DoLoop

      test  ebx, ebx
      jns   D0_NotLessThan0
      neg   ebx
      neg   esi

      D0_NotLessThan0:
      mov   eax, esi
      test  eax, eax
      jns   S0_NotLessThan0
      add   eax, DWORD PTR m

      S0_NotLessThan0:

      // Ignore this code for this test.  We always send a prime m
      //if (d0 != 1) // impossible if m is prime
      //s0 = 0;

   }
   // Note that eax is setup correctly.
   return;
#endif
}
#ifdef _MSC_VER
#pragma warning( pop )
#endif

bool Erat_Mod::isValid()
{
   return m_bvalid;
}

void Erat_Mod::FillSmallPrimes(uint32_t _nsp)
{
   ENTER("FillSmallPrimes");
   uint32_t Nsp, p;
   if (maxSmallPrime >= _nsp)
      return;
   delete[] primetable;

   unsigned Stop = ((_nsp*20)>>5);
   // We have a "buffer" of 2000*32 additional numbers beyond the _nsp*20 size.
   // This will allow the program not to crash here, if we get to prime density
   // average less than 1 prime per every 20 numbers.  In a debug testing mode,
   // this code will printf if that happens (it never does).  We could probably
   // trim this down considerably, or even create a table of how many uint32_t's
   // are needed for each "nsp" we deal with.  I will add that as a todo
   uint32_t *sv = new uint32_t[Stop+2000];
   primetable = new uint32_t[_nsp];

   primetable[0] = 2;

   // Back to the "bone headed way of doing it ;)
   for (unsigned i = 0; i < Stop+2000; )
      sv[i++] = 0xFFFFFFFF;
   Nsp = 1;
   p = 0;

   uint32_t Prime;
   for (; Nsp < _nsp; Nsp++)
   {
      Prime = p*2+3;
      primetable[Nsp] = Prime;
      uint32_t Cur = p+Prime;
      while(Cur < _nsp*20)
      {
         if (IsBitSet2(sv, Cur))
            ClearBit2(sv,Cur);
         Cur += Prime;
      }
      p++;
      while (!IsBitSet2(sv, p))
         p++;
   }
   maxSmallPrime = _nsp;
#if defined (TESTING)
  #if defined (_DEBUG)
   if (Prime > _nsp*20)
   {
      PFPrintfStderr("\nWhoops, the size of the small prime creation array was not large enough!!!\n",0);
      exit(0);
   }
   PFPrintfStderr("\nMax SmallPrime is %d ", Prime);
   PFPrintfStderr("size of sieve buffer %d\n", _nsp*20);
  #endif
#endif
   delete[] sv;
   EXIT("FillSmallPrimes");
}

void Erat_Mod::FillSmall_ixt()
{
   ENTER("FillSmall_ixt");
   // This calculated size is safe.  At 100k small primes, we only need 1299721 bytes, but the extra does not hurt much
   uint32_t _nsp = spTab.nsp[m_n_spTabnCur];
   uint32_t p, X, b;
   delete[] m_ixt;
   m_ixt = new uint64_t[_nsp];
   FillSmallPrimes(_nsp);

   // This code fills in EVERYTHING.  Both the primetable[] and the ixt[] arrays.  primetable[] is an array
   // of small primes from 3 to whatever the max prime we are using is, and ixt[] is the "starting place" for
   // that prime within the next chunk to fill.

   for (p = 0;  p < _nsp; p++)
   {
      uint32_t Prime = primetable[p];
      // Now find the starting 'b' value for this Mod sieve.
      X = m_nModBase%Prime;
      if (!X)
#ifdef _MSC_VER
         m_ixt[p] = 0xFFFFFFFFFFFFFFFF;  // b can NOT be a factor of ANY N+1 value.
#else
         m_ixt[p] = 0xFFFFFFFFFFFFFFFFULL;  // b can NOT be a factor of ANY N+1 value.
#endif
      else
      {
         b = (modInv(Prime-X,Prime)-1)%Prime;
         if (m_bIsModPlus1)
            m_ixt[p] = b;
         else
            m_ixt[p] = Prime-b-2;
      }
   }

   m_bSModLoadNextSieveChunk_Adjusted = false;
   EXIT("FillSmall_ixt");
}



Erat_Mod::Erat_Mod(uint32_t ModBase, bool bIsModPlus1, uint32_t nSieveBitSize, uint32_t Continuing_SIEVE_BIT_SIZE)
: m_ModConditions(NULL), m_nModConditions(0), m_SModpMap(NULL), m_SMod_pMap(NULL), m_nModBase(ModBase),
  m_bIsModPlus1(bIsModPlus1), m_bvalid(false), m_bAdjusted(false), m_n_spTabnCur(0),
  m_SIEVE_BIT_SIZE(nSieveBitSize), m_ContinuingSIEVE_BIT_SIZE(Continuing_SIEVE_BIT_SIZE),
  m_ixt(NULL), m_Exceptions(NULL), m_nCurException(0), m_nNumExceptions(0),
  m_bSModLoadNextSieveChunk_Adjusted(false), m_uiNext(0), m_uiLast(0), m_SModMaxNum(0)
{
   ENTER("constructor");
   //m_bvalid = false;
   //Erat_Mod_init(ModBase, bIsModPlus1, nSieveBitSize, Continuing_SIEVE_BIT_SIZE);
   EXIT("constructor");
}

void Erat_Mod::Erat_Mod_init(uint32_t ModBase, bool bIsModPlus1, uint32_t nSieveBitSize, uint32_t Continuing_SIEVE_BIT_SIZE)
{
   ENTER("Erat_mod_init");
   m_SModpMap = NULL;
   m_SMod_pMap = NULL;
   m_ModConditions = NULL;
   m_nModConditions = 0;

   m_Exceptions = NULL;
   m_nCurException = 0;
   m_nNumExceptions = 0;

   m_ixt = NULL;

   m_uiNext=0;
   m_uiLast=0;
   m_SModMaxNum=0;

   m_nModBase = ModBase;
   m_bIsModPlus1 = bIsModPlus1;
   m_SIEVE_BIT_SIZE = nSieveBitSize;
   m_ContinuingSIEVE_BIT_SIZE = Continuing_SIEVE_BIT_SIZE;

   m_n_spTabnCur = 0;
   m_bSModLoadNextSieveChunk_Adjusted = false;

// AdjustDepth();
// FillSmall_ixt();
   m_bAdjusted = false;
   m_bvalid = true;
   EXIT("Erat_mod_init");
}

Erat_Mod::Erat_Mod(const char* StartupString)
: m_ModConditions(NULL), m_nModConditions(0), m_SModpMap(NULL), m_SMod_pMap(NULL), m_nModBase(0),
  m_bIsModPlus1(false), m_bvalid(false), m_bAdjusted(false), m_n_spTabnCur(0),
  m_SIEVE_BIT_SIZE(0), m_ContinuingSIEVE_BIT_SIZE(0),
  m_ixt(NULL), m_Exceptions(NULL), m_nCurException(0), m_nNumExceptions(0),
  m_bSModLoadNextSieveChunk_Adjusted(false), m_uiNext(0), m_uiLast(0), m_SModMaxNum(0)
{
   ENTER("constructor s");
   // Ok, the format is {mod[,-1]}[{cond_mod,cond_val}...]
// PFPrintf ("Modular sieve based on %s\n", StartupString);
   int n;
   uint32_t mod;
   int Plus;
   bool b_Plus1 = true;
   n = sscanf(StartupString, "{%d,%d}", &mod, &Plus);
   if (n == 0)
   {
      Erat_Mod_init(3, b_Plus1, 16, 20);
      m_bvalid = false;
      return;
   }
   if (n == 2 && Plus == -1)
      b_Plus1 = false;

   Erat_Mod_init(mod, b_Plus1, 16, 20);

   const char *cp = StartupString;
   while(cp && *cp == '{')
   {
      cp = strchr(cp, '}');
      if (cp)
      {
         cp++;
         uint32_t val;
         char YN;
         n = sscanf(cp, "{%c,%d,%d}", &YN, &mod, &val);
         if (n != 3)
         {
            n = sscanf(cp, "{%c,%d}", &YN, &val);
            if (n != 2)
            {
               cp = 0;
            }
            else
            {
               if (YN == 'f')
                  AddFactorsExceptions(val);
               else if (YN == 'p')
                  AddSmallPrimesExceptions(val);
               else
                  cp = 0;
            }
         }
         else
            AddModCondition(mod, val, YN=='y'||YN=='Y');
      }
   }
   EXIT("constructor s");
}

void Erat_Mod::SetSieveBitMapSize(uint32_t s)
{
   ENTER("SetSieveBitMapSize");
   m_SIEVE_BIT_SIZE = m_ContinuingSIEVE_BIT_SIZE = s;
   EXIT("SetSieveBitMapSize");
}

bool Erat_Mod::AdjustDepth()
{
   ENTER("AdjustDepth");
   m_SModMaxNum = m_uiNext + (1ULL<<m_SIEVE_BIT_SIZE);

   if (m_n_spTabnCur < spTab.nMax-1 && spTab.u64Pure[m_n_spTabnCur] < m_SModMaxNum*m_nModBase)
   {
      do
      {
         m_n_spTabnCur++;
      }
      while(m_n_spTabnCur < spTab.nMax-1 && spTab.u64Pure[m_n_spTabnCur] < m_SModMaxNum*m_nModBase);
#if defined (_DEBUG) && defined (TESTING)
      PFPrintfStderr("\nExpanding small prime table for pure prime candidates up to %0.1f bits\n", spTab.dBits[m_n_spTabnCur]);
#endif
      EXIT("AdjustDepth");
      return true;
   }
   EXIT("AdjustDepth");
   return false;
}

void Erat_Mod::SModLoadNextSieveChunk()
{
   ENTER("SModLoadNextSieveChunk");
#if defined (_DEBUG) && defined (TESTING)
   PFPrintfStderr("\nMaking next sieve chunk\n", spTab.dBits[m_n_spTabnCur]);
#endif

   // First free any memory allocated to the old chunk.
   Free_pMap2(&m_SMod_pMap);

   if (AdjustDepth())
      FillSmall_ixt();

   uint32_t nvalsleft;

   m_uiLast = m_SModMaxNum;

   Init_pMap2(m_uiNext, m_SModMaxNum, &m_SMod_pMap, &m_SModpMap);
   Set_All_bits_true2(m_uiNext, m_SModMaxNum, nvalsleft, m_SModpMap);

   uint32_t _nsp = spTab.nsp[m_n_spTabnCur];

   if (!m_bSModLoadNextSieveChunk_Adjusted)
   {
      m_bSModLoadNextSieveChunk_Adjusted=true;
      // This only happens the first time, and make a 1 time adjustment for sieves not starting at 0.

      if (m_uiNext != 0)
      {
         for (uint32_t i = 0; i < _nsp; i++)
         {
            if (m_uiNext > m_ixt[i])
               m_ixt[i] += ((m_uiNext-m_ixt[i])/primetable[i])*primetable[i];
//          m_ixt[i] += (m_uiNext/primetable[i])*primetable[i];
            if (m_ixt[i] < m_uiNext)
               m_ixt[i] += primetable[i];
         }
      }
      else
      {
         // For sieves starting at 0, we MUST be very careful with correct modular primes less than max number in primetable.
         for (uint32_t i = 0; i < _nsp; i++)
         {
            uint64_t j = m_ixt[i];
#ifdef _MSC_VER
            if (j == 0xFFFFFFFFFFFFFFFF)
#else
            if (j == 0xFFFFFFFFFFFFFFFFULL)
#endif
               continue;

            // without this adjustment, we "lose" the modular primes which are less than the max small prime of the sieve.
            if (m_bIsModPlus1)
            {
               if (primetable[i] % m_nModBase == 1 )
                  j += primetable[i];
            }
            else
            {
               if (primetable[i] % m_nModBase == m_nModBase-1 )
                  j += primetable[i];
            }
            for (; j <= m_SModMaxNum; j += primetable[i])
            {
               if (IsBitSet2(m_SModpMap, j))
                  ClearBit2(m_SModpMap, j);
            }
            // This prime will start at this location in the "next" sieve chunk.
            if (j == m_SModMaxNum + primetable[i])
               m_ixt[i] = m_SModMaxNum;
            else
               m_ixt[i] = j;
         }
         // Bail out here, since we are done filling the SModpMap bit-array
         SetBit2(m_SModpMap, m_SModMaxNum+1);
         return;
      }
   }

   for (uint32_t i = 0; i < _nsp; i++)
   {
      uint64_t j = m_ixt[i];
      for (; j <= m_SModMaxNum; j += primetable[i])
      {
         if (IsBitSet2(m_SModpMap, j))
            ClearBit2(m_SModpMap, j);
      }
      // This prime will start at this location in the "next" sieve chunk.
      if (j == m_SModMaxNum + primetable[i])
         m_ixt[i] = m_SModMaxNum;
      else
         m_ixt[i] = j;
   }
   // Set 1 bit out of range this is our "flag" bit. It is set so we do not have to check for end of buffer
   // conditions after every check for a bit, but simply check for end of buffer when a set bit has been found
   // in the SModNextPrime() function
   SetBit2(m_SModpMap, m_SModMaxNum+1);
   EXIT("SModLoadNextSieveChunk");
}

uint64_t Erat_Mod::next()
{
   uint64_t Ret;

   if (m_nCurException < m_nNumExceptions)
      return m_Exceptions[m_nCurException++];

   if (!m_bAdjusted)
   {
      AdjustDepth();
      FillSmall_ixt();
      SModLoadNextSieveChunk();
      m_SIEVE_BIT_SIZE = m_ContinuingSIEVE_BIT_SIZE;
      m_bAdjusted = true;
   }

   for(;;)
   {
      // The workaround attempted here is to make the bit test
      // relative to the previous search position. This doesn't
      // break the addressing offset that's already in effect.
      // The 64-bit downshift was getting generated anyway.

      // Note it's now required however that the memory map is
      // no more than 2^27 cells/2^29 bytes/2^32 bits long. But
      // larger memory maps would only work with 64-bit addressing
      // anyway.

      // This does however break transparency somewhat of the m_uiNext
      // cursor (tthis code assumes its a uint64_t).
      uint32_t *pSieveBase=&m_SModpMap[m_uiNext>>5];
      uint32_t uiIndex=((uint32_t)m_uiNext)&0x1F;
      m_uiNext^=uiIndex;
      while(!IsBitSet2(pSieveBase,uiIndex))
         ++uiIndex;
      m_uiNext+=uiIndex;

      // Check for end of buffer condition.  NOTE that a guard bit was added to the end of the bit array, so
      // we only have to check for end of buffer once a bit has been found.  This guard bit allows us to avoid
      // having to check within the while loop above.
      if (m_uiNext >= m_SModMaxNum)
      {
         // not sure about this.  This may cause a prime to be tried twice.  It will not hurt anything, and is safer this way
         m_uiNext = m_SModMaxNum;

         SModLoadNextSieveChunk();
         return next();    // Careful, recursion.
      }
      m_uiNext++;
      if (m_bIsModPlus1)
         Ret=((uint64_t)m_uiNext)*m_nModBase+1;
      else
         Ret=((uint64_t)m_uiNext)*m_nModBase-1;

      if (m_nModConditions)
      {
         // Checking for conditions will be done as a set of OR's.  As soon as a condition is met,
         // then the resultant number is returned.   We may need to add additional logic to
         // allow or'ing and and'ing syntax, but for this first cut, OR'ing is all that is done.
         for (uint32_t i = 0; i < m_nModConditions; i++)
         {
            bool bYes = ((int64_t)Ret%m_ModConditions[i].Mod) == m_ModConditions[i].Val;
            if (bYes && m_ModConditions[i].bOnlyAcceptIfTrue)
               return Ret;
            if (!bYes && !m_ModConditions[i].bOnlyAcceptIfTrue)
               return Ret; 
         }
      }
      else
         break;
   }
 
   return Ret;
}

void  Erat_Mod::AddSmallPrimesExceptions(uint32_t nMaxSmallPrimeWanted)
{
   if (nMaxSmallPrimeWanted < 2)
      return;
   uint32_t nNumSmallPr, i, j;

   if (!maxSmallPrime || primetable[maxSmallPrime] <= nMaxSmallPrimeWanted)
      FillSmallPrimes(nMaxSmallPrimeWanted/20+2000);

   if (!maxSmallPrime || primetable[maxSmallPrime] <= nMaxSmallPrimeWanted)
      FillSmallPrimes(nMaxSmallPrimeWanted/15+2000);

   for (nNumSmallPr = 0; primetable[nNumSmallPr] <= nMaxSmallPrimeWanted; nNumSmallPr++)
      ;
   uint64_t *nTmp = new uint64_t [nNumSmallPr + m_nNumExceptions];
   for (i = 0; i < m_nNumExceptions; i++)
      nTmp[i] = m_Exceptions[i];
   for (j = 0; j < nNumSmallPr; j++)
      nTmp[i+j] = primetable[j];
   delete[] m_Exceptions;
   m_Exceptions = nTmp;
   m_nNumExceptions = i+j;
   m_nCurException = 0;
}

void  Erat_Mod::AddFactorsExceptions(uint64_t nNum)
{
   if (nNum < 2)
      return;
   uint64_t nFacts[16];      // since 53# is greater than 2^64, the most uniq factors of ANY number under 2^64 is 15
   FillSmallPrimes(5000);
   uint32_t i, j, n;
   for (i = 0, n = 0; i < 5000 && nNum != 1; i++)
   {
      if (nNum%primetable[i] == 0)
      {
         while (nNum%primetable[i] == 0)
            nNum /= primetable[i];
         nFacts[n++] = primetable[i];
      }
   }
   uint64_t *nTmp = new uint64_t [n + m_nNumExceptions];
   for (i = 0; i < m_nNumExceptions; i++)
      nTmp[i] = m_Exceptions[i];
   for (j = 0; j < n; j++)
      nTmp[i+j] = nFacts[j];
   delete[] m_Exceptions;
   m_Exceptions = nTmp;
   m_nNumExceptions = i+j;
   m_nCurException = 0;
}

void  Erat_Mod::AddModCondition(int Mod, int Val, bool bOnlyAcceptIfTrue)
{
   ENTER("AddModCondition");
   ModCondition *p = new ModCondition[m_nModConditions+1];
   if (m_nModConditions)
   {
      for (uint32_t i = 0; i < m_nModConditions; i++)
         p[i] = m_ModConditions[i];
   }
   p[m_nModConditions].bOnlyAcceptIfTrue = bOnlyAcceptIfTrue;
   p[m_nModConditions].Mod = Mod;
   p[m_nModConditions].Val = Val;
   m_nModConditions++;
   delete[] m_ModConditions;
   m_ModConditions = p;
   EXIT("AddModCondition");
}

void Erat_Mod::skipto(uint64_t SkipTo)
{
   ENTER("skipto");
   m_uiNext = (SkipTo/m_nModBase);
   if (m_uiNext)
      --m_uiNext;
   if (m_uiNext >= m_SModMaxNum)
   {
      m_bSModLoadNextSieveChunk_Adjusted = false;
      if (m_nCurException >= m_nNumExceptions)
         SModLoadNextSieveChunk();
   }
// m_SIEVE_BIT_SIZE = m_ContinuingSIEVE_BIT_SIZE;
   EXIT("skipto");
}

void Erat_Mod::init()
{
   ENTER("init");
   m_uiNext = 0;
   m_bSModLoadNextSieveChunk_Adjusted = false;
// SModLoadNextSieveChunk();

// m_SIEVE_BIT_SIZE = m_ContinuingSIEVE_BIT_SIZE;
   EXIT("init");
}

// Call this function upon program exit.  Any "static" or cached memory will be cleaned up here.
void Erat_Mod::FreeAllMemory()
{
   ENTER("FreeAllMemory");
   delete[] primetable;
   primetable = 0;
   maxSmallPrime = 0;
   EXIT("FreeAllMemory");
}

Erat_Mod::~Erat_Mod()
{
   ENTER("destructor");

   Free_pMap2(&m_SMod_pMap);
   m_SMod_pMap = m_SModpMap = 0;
   m_bvalid = false;

   delete[] m_ixt;
   m_ixt = 0;

   delete[] m_ModConditions;
   m_ModConditions = 0;
   m_nModConditions = 0;

   delete[] m_Exceptions;
   m_Exceptions = 0;
   m_nCurException = 0;
   m_nNumExceptions = 0;

   m_n_spTabnCur = 0;
   EXIT("destructor");
}


// Test this crap out.  I had a problem dropping primes in the beginning, but that has
// been solved. This program will test (slowly) the modular primes against a very simple
// generator.

#if defined (TESTING)

#include <stdio.h>

#if defined (_MSC_VER)
#include <io.h>
#endif

// Note that testing is "limited" to only 2^42 by this function.
uint64_t mulmodp (const uint64_t a, const uint64_t b, const uint64_t p)
{
   static uint64_t r;
   uint64_t a0  = a & 0x1FFFFF;
   uint64_t a20 = (a >> 21) & 0x1FFFFF;
   uint64_t b0  = b & 0x1FFFFF;
   uint64_t b20 = (b >> 21) & 0x1FFFFF;
   r = (a20*b20) % p;
   r = ((r<<21) + a0*b20 + a20*b0) % p;
   r = ((r<<21) + a0*b0) % p;
   return r;
}

bool prp (const uint64_t p, int b)
{
   static uint64_t x, r;
    r=1, x=b;
    uint64_t n = p-1;

    for (;n!=1;n>>=1)
    {
        if (n & 1)
            r = mulmodp (r, x, p);
        x = mulmodp (x, x, p);
    }
    r = mulmodp (r, x, p);

    return r==1;
}

uint64_t powmod (uint64_t p, uint64_t e, const uint64_t n)
{
   uint64_t y=1;
    for (;;)
    {
        if (e & 1)
            y = mulmodp (y, p, n);
      e >>= 1;
      if (!e)
         return y;
        p = mulmodp (p, p, n);
    }
}

bool MillerTest(const uint64_t n, uint32_t x)
{
   uint64_t t = n-1;
   uint64_t q = t>>1;
   uint32_t k = 1;
   while ( (q & 1) == 0)
   {
      k++;
      q >>= 1;
   }

   uint32_t j = 0;
   uint64_t y = powmod(x, q, n);
   for (;;)
   {
      if ( (j == 0 && y == 1) || y == n-1)
         return true;
      j++;
      if (j < k)
         y = mulmodp(y, y, n);
      else
         return false;
   }
}

int Usage()
{
   fprintf (stderr, "Error, usage is Erat_Mod  mod_to_check[,-1] [-s | -g] [-y#,# || -n#,#] [-m#]\n\n");
   fprintf (stderr, "-s is for a \"speed\" test\n");
   fprintf (stderr, "-g will generate a list of primes to stdout\n");
   fprintf (stderr, "-y#,# adds a modular check which must be TRUE  to satisfy\n");
   fprintf (stderr, "-n#,# adds a modular check which must be FALSE to satisfy\n");
   fprintf (stderr, "-m# stops the program at max value of #\n");

   return 1;
}

// Here is the main testing code.  The command line allows any modular sieve to be handled
// (within the bounds of the program).  There is an optional -s switch which simply does a
// speed check.  It runs through the sieve at full speed, printing out results every 10000
// iterations.  Without the -s, a "check" is done.  this check is simply a stupid wheel sieve
// followed by some Fermat tests, folled by a few Miller tests.  If a modular number passes
// this, it is checked against the sieved modular prime.  If there are descrpancies, they are
// output, and a re-syncing is attempted.  So far, with the Miller tests added, there have been
// no false alarms.

int main(int argc, char **argv)
{
   int Mod;
   if (argc < 2)
      return Usage();
   sscanf(argv[1], "%d", &Mod);
   if (!Mod)
      return Usage();
   bool b_isPos = true;
   if (strstr(argv[1], ",-1"))
      b_isPos = false;
   bool bSpeedTest = false;
   bool bModGen = false;
   uint64_t MaxVal = 1;
   MaxVal <<= 62;

   Erat_Mod Sieve(Mod, b_isPos);

   for (int arg = 2; arg < argc; arg++)
   {
      if (!strncmp(argv[arg], "-m", 2))
         MaxVal = _atoi64(&argv[arg][2]); // Not portable.
      else if (!strcmp(argv[arg], "-s"))
         bSpeedTest = true;
      else if (!strcmp(argv[arg], "-g"))
         bModGen = true;
      else if (argv[arg][0] == '-' && (argv[arg][1] == 'y' || argv[arg][1] == 'n') )
      {
         int t1, t2;
         if (sscanf(&argv[arg][2], "%d,%d", &t1, &t2) != 2)
         {
            Erat_Mod::FreeAllMemory();
            return Usage();
         }
         Sieve.AddModCondition(t1, t2, argv[arg][1] == 'y');
      }
      else
      {
         Erat_Mod::FreeAllMemory();
         return Usage();
      }
   }
   Sieve.init();
   fprintf(stderr, "\nTesting Erat_Mod(%d), hit ^C to break\n", Mod);
   uint32_t Cnt=0;
   uint64_t s = 0;
   char u64Buf1[20], u64Buf2[20];
   bool skipme = false;

   if (bModGen)
   {
      bool bIsTTY = true;
#if defined (_MSC_VER)
      bIsTTY = !!isatty(fileno(stdout));
#endif
      for (uint64_t j = 1; ; j++)
      {
         if (!bIsTTY)
         {
            s = Sieve.next();
            if (MaxVal < s)
            {
               fprintf (stderr, "\rWorking at prime # %d  pr=%" PRIu64"\r", Cnt, s);
               break;
            }
            printf ("%" PRIu64"\n", s);
            ++Cnt;
            if ((Cnt%10000) == 1)
               fprintf (stderr, "\rWorking at prime # %d  pr=%" PRIu64"\r", Cnt, s);
         }
         else
         {
            s = Sieve.next();
            if (MaxVal < s)
               break;
            printf ("%" PRIu64"\n", s);
         }
      }
   }
   else if (bSpeedTest)
   {
      for (uint64_t j = 1; j; j++)
      {
         s = Sieve.next();
         if (MaxVal < s)
            break;
         ++Cnt;
         if ((Cnt%10000) == 1)
            fprintf (stderr, "\rWorking at prime # %d  pr=%" PRIu64"\r", Cnt, s);
      }
      fprintf (stderr, "\rWorking at prime # %d  pr=%" PRIu64"\r", Cnt, s);
   }
   else
   {
      // validity test (slow, but pretty good)
      for (uint64_t i = 1; i; i++)
      {
         Continue:;
         uint64_t t = i;
         t *= Mod;
         if (b_isPos)
            t++;
         else
            t--;

         uint32_t m = (uint32_t)(t%(3*5*7*11*13*17*19*23*29U));
         if (m%3==0||m%5==0||m%7==0||m%11==0||m%13==0||m%17==0||m%19==0||m%23==0||m%29==0)
            continue;

         // Check the modular conditions.
         if (Sieve.m_nModConditions)
         {
            bool Ok = false;
            for (int i = 0; !Ok && i < Sieve.m_nModConditions; i++)
            {
               bool bYes = (t%Sieve.m_ModConditions[i].Mod) == Sieve.m_ModConditions[i].Val;
               if (bYes && Sieve.m_ModConditions[i].bOnlyAcceptIfTrue)
                  Ok = true;
               if (!bYes && !Sieve.m_ModConditions[i].bOnlyAcceptIfTrue)
                  Ok = true;
            }
            if (!Ok)
               continue;
         }

         if (!prp(t, 3) || !prp(t, 2))
            continue;

         if (!skipme)
            s = Sieve.next();
         if (MaxVal < s)
            break;
         skipme = false;
         if (s != t)
         {
            static int p[] = {31, 37, 41, 43, 47, 53, 59, 61,67, 71, 73, 79, 83, 89, 97,101,103,
                          107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,
                          199,211,223,227,229,233,239,241,251, 257,263,269,271,277,281,283,293,307,
                          311,313,317,331,337,347,349,353,359, 367,373,379,383,389,397,401,409,419,
                          421,431,433,439,443,449,457,461,463, 467,479,487,491,499,503,509,521,523,
                          541,547,557,563,569,571,577,587,593, 599,601,607,613,617,619,631,641,643,
                          647,653,659,661,673,677,683,691,701, 709,719,727,733,739,743,751,757,761,
                          769,773,787,797,809,811,821,823,827, 829,839,853,857,859,863,877,881,883,
                          887,907,911,919,929,937,941,947,953, 967,971,977,983,991,997, 1009, 1013,
                          1019, 0};
            int *pp = p;
            while (*pp)
            {
               if ( (t % (*pp)) == 0)
               {
                  i++;
                  skipme = true;
                  goto Continue;
               }
               ++pp;
            }
            if (!MillerTest(t, 3)||!MillerTest(t, 5)||!MillerTest(t, 7)||!MillerTest(t, 9)||!MillerTest(t, 11))
            {
               i++;
               skipme = true;
               goto Continue;
            }

            // NOTE that because of the C version of mulmod, we are limited to testing only the first
            // 42 bits.  So if we get an error, make SURE that we have not exceeded that threashold.
            uint64_t max = 1;
            if (s > max<<42)
            {
               Erat_Mod::FreeAllMemory();
               fprintf (stderr, "\rWorking at prime # %d  pr=%" PRIu64"\n\n", Cnt, s);
               return printf ("NOTE that testing is ONLY valid to 42 bits, and we have reached that point\n");
            }

            printf ("\rError sieve=%" PRIu64"  Calc = %" PRIu64"\n", s, t);
            DoAgain:;
            while (s < t)
            {
               s = Sieve.next();
               if (MaxVal < s)
                  break;
               goto DoAgain;
            }
            while (t < s)
            {
               do
               {
                  i++;
                  t = i;
                  t *= Mod;
                  t++;
                  m = (uint32_t)(t%(3*5*7*11*13*17*19*23*29U));
               }
               while (m%3==0||m%5==0||m%7==0||m%11==0||m%13==0||m%17==0||m%19==0||m%23==0||m%29==0||!prp(t,3)||!prp(t,2));
               goto DoAgain;
            }
         }
         ++Cnt;
         if ((Cnt%1000) == 1)
            fprintf (stderr, "\rWorking at prime # %d  pr=%" PRIu64"\r", Cnt, s);
      }
      fprintf (stderr, "\rWorking at prime # %d  pr=%" PRIu64"\r", Cnt, s);
   }

   Erat_Mod::FreeAllMemory();
   return 0;
}

#endif
