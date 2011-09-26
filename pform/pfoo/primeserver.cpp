#include "pfoopch.h"
#include "primeserver.h"

#define RANGE_BYTES     100000

#define S_BYTE(i)       ((i>>1)>>3)
#define S_BIT(i)        (1<<((i>>1)&7))

#define RANGE_SIZE(r)   (8*(r<<1))
#define RANGE_BITS(r)   (8*(r))

PrimeServer::PrimeServer(uint64 upperLimit)
{
   m_UpperLimit = upperLimit;

   Initialize();

   BuildWindow(false, true);
}

PrimeServer::PrimeServer(double upperLimit)
{
   if (upperLimit > (double) ULLONG_MAX)
      ::PrimeServer((uint64) ULLONG_MAX);
   else
      ::PrimeServer((uint64) upperLimit);
}

PrimeServer::~PrimeServer()
{
   if (m_pPrimeTable) delete [] m_pPrimeTable;
   if (m_pCompositeTable) delete [] m_pCompositeTable;
   if (m_pSieve) delete [] m_pSieve;
}

void    PrimeServer::SetUpperLimit(double upperLimit)
{
   if (m_UpperLimit == ULLONG_MAX)
   {
      if (!m_OutputWarningSent)
         PFPrintfStderr("Reached max sieving limit of %llu.  Sieve will return potential composites above that value.", m_UpperLimit);
      m_OutputWarningSent = true;
      return;
   }

   if (m_pPrimeTable) delete [] m_pPrimeTable;
   if (m_pCompositeTable) delete [] m_pCompositeTable;
   if (m_pSieve) delete [] m_pSieve;

   if (upperLimit > (double) ULLONG_MAX)
      m_UpperLimit = ULLONG_MAX;
   else
      m_UpperLimit = (uint64) upperLimit;

   PFPrintfStderr("Sieve re-allocated with a limit of %llu.", m_UpperLimit);

   Initialize();

   BuildWindow(false, true);
}

void    PrimeServer::Initialize(void)
{
   uint64   prevPrime, thisPrime;
   uint32   sqrtMax, i, composite;
   uint32   p, minp, *lowPrimes, lowPrimeCount;
   uint8   *sievePtr, *temp, *sieve;

   m_pPrimeTable = 0;
   m_pCompositeTable = 0;
   m_pSieve = new uint8[RANGE_BYTES];
   m_PrimesUsedInWindow = 0;

   m_LowEndOfWindow = m_HighEndOfWindow = 0;
   m_LastPrimeReturned = 0;
   m_IndexOfLastPrimeReturned = 0;
   m_LastSearchValue = 0;
   m_MaxPrimeUsed = 0;
   m_IndexInWindow = false;
   m_OutputWarningSent = false;

   // Find all primes less than sqrt(MAX_PRIME)
   sqrtMax = (uint32) sqrt(sqrt((double) m_UpperLimit));
   sqrtMax /= 1000;
   sqrtMax *= 1000;
   sqrtMax += 1000;

   // Make sure this is large enough to hold all of the low
   // primes we need.
   lowPrimes = new uint32[1000000];
   lowPrimes[0] = 3;
   lowPrimeCount = 1;
   for (p=5; p<sqrtMax; p+=2)
   {
      for (minp=0; minp<=lowPrimeCount; minp++)
      {
         if (lowPrimes[minp] * lowPrimes[minp] > p)
         {
            lowPrimes[lowPrimeCount] = p;
            lowPrimeCount++;
            break;
         }
         if (p % lowPrimes[minp] == 0)
             break;
      }
   }

   sqrtMax = (uint32) sqrt((double) m_UpperLimit);
   sqrtMax /= 1000;
   sqrtMax *= 1000;
   sqrtMax += 1000;

   // Divide sqrtMax by 2 to save memory, also because already know
   // that all even numbers in the sieve are composite
   sieve = new uint8[sqrtMax >> 1];
   memset(sieve, 1, (sqrtMax >> 1));

   for (i=0; i<lowPrimeCount; i++)
   {
      // Get the current low prime.  Start sieving at 3x that prime
      // since 1x is prime and 2x is divisible by 2.
      // sieve[1] = 3, sieve[2] = 5, etc.
      composite = lowPrimes[i] * 3;
      sievePtr = &sieve[(composite - 1) >> 1];

      while (composite < sqrtMax)
      {
         // composite will always be odd, so add 2*lowPrimes[i]
         *sievePtr = 0;
         sievePtr += lowPrimes[i];
         composite += (lowPrimes[i] << 1);
      }
   }

   m_MaxEntriesInPrimeTable = sqrtMax / 20;
   m_MaxEntriesInPrimeTable /= 1000;
   m_MaxEntriesInPrimeTable *= 1000;
   m_MaxEntriesInPrimeTable += 1000;

   m_pPrimeTable = new uint8[m_MaxEntriesInPrimeTable];
   m_PrimesInPrimeTable = 0;
   thisPrime = prevPrime = 0;
   for (i=1; i<(sqrtMax >> 1); i++)
   {
      if (sieve[i])
      {
         // Convert the value back to an actual prime.  Note that primeTable[0] = 3.
         thisPrime = (i << 1) + 1;
         m_pPrimeTable[m_PrimesInPrimeTable] = (uint8) (thisPrime - prevPrime);
         m_PrimesInPrimeTable++;
         prevPrime = thisPrime;

         if (m_PrimesInPrimeTable == m_MaxEntriesInPrimeTable)
         {
            m_MaxEntriesInPrimeTable += 1000;
            temp = m_pPrimeTable;
            m_pPrimeTable = new uint8[m_MaxEntriesInPrimeTable];
            memcpy(m_pPrimeTable, temp, m_PrimesInPrimeTable * sizeof(uint8));
            delete [] temp;
         }
      }
   }

   m_MaxPrimeInPrimeTable = thisPrime;

   delete [] sieve;
   delete [] lowPrimes;
}

void  PrimeServer::SetWindow(uint64 searchValue)
{
   // If we are in the right window, then do nothing
   if (searchValue > m_LowEndOfWindow && searchValue < m_HighEndOfWindow)
   {
      m_MaxPrimeUsed = 0;
      m_PrimesUsedInWindow = 0;
      m_LastPrimeReturned = 0;
      m_IndexOfLastPrimeReturned = 0;
      m_LastSearchValue = searchValue;
      return;
   }

   BuildWindow(false, true, searchValue);
}

void  PrimeServer::BuildWindow(bool nextWindow, bool restart, uint64 searchValue)
{
   uint64   thisPrime, composite;
   uint32   i;

   if (nextWindow)
      m_LowEndOfWindow = m_LowEndOfWindow + RANGE_SIZE(RANGE_BYTES);
   else
      m_LowEndOfWindow = searchValue - (searchValue % RANGE_BYTES);
   
   m_IndexInWindow = false;
   m_HighEndOfWindow = m_LowEndOfWindow + RANGE_SIZE(RANGE_BYTES) - 1;

   if (restart)
   {
      m_MaxPrimeUsed = 0;
      m_PrimesUsedInWindow = 0;
      m_LastPrimeReturned = 0;
      m_IndexOfLastPrimeReturned = 0;
      m_LastSearchValue = searchValue;
   }

   SetupSieve();

   memset(m_pSieve, 0xff, RANGE_BYTES);

   thisPrime = 0;
   
   for (i=0; i<m_PrimesUsedInWindow; i++)
   {
      thisPrime += m_pPrimeTable[i];
      composite = m_pCompositeTable[i];

      while (composite < RANGE_SIZE(RANGE_BYTES))
      {
         m_pSieve[S_BYTE(composite)] &= ~S_BIT(composite);
         composite += (thisPrime << 1);
      }

      m_pCompositeTable[i] = (uint32) (composite - RANGE_SIZE(RANGE_BYTES));
   }
}

void    PrimeServer::SetupSieve(void)
{
   uint32   i, j, *temp, thisPrime, saveUsedInWindow, saveMaxPrimeUsed;
   uint64   maxValue, lastComposite;
   uint64   highEndOfWindow;

   if (m_PrimesUsedInWindow >= m_PrimesInPrimeTable)
      return;

   saveUsedInWindow = m_PrimesUsedInWindow;
   saveMaxPrimeUsed = m_MaxPrimeUsed;

   if (m_PrimesUsedInWindow == 0)
   {
      m_PrimesUsedInWindow = 40000;

      if (m_PrimesUsedInWindow > m_PrimesInPrimeTable)
         m_PrimesUsedInWindow = m_PrimesInPrimeTable;

      m_MaxPrimeUsed = 0;
      for (i=0; i<m_PrimesUsedInWindow; i++)
         m_MaxPrimeUsed += m_pPrimeTable[i];
   }

   highEndOfWindow = m_LowEndOfWindow + RANGE_SIZE(RANGE_BYTES);

   // Extend the range of primes used for sieving if necessary
   while (m_PrimesUsedInWindow < m_PrimesUsedInWindow)
   {
      maxValue = m_MaxPrimeUsed * m_MaxPrimeUsed;

      // The sieve range does not include even numbers, so it
      // contains twice as many candidates
      if (maxValue > highEndOfWindow) break;

      for (j=0; j<20000; j++)
      {
         m_MaxPrimeUsed += m_pPrimeTable[m_PrimesUsedInWindow];
         m_PrimesUsedInWindow++;
         if (m_PrimesUsedInWindow == m_PrimesUsedInWindow)
            break;
      }
   }

   if (saveUsedInWindow == m_PrimesUsedInWindow)
      return;

   if (m_pCompositeTable)
   {
      temp = m_pCompositeTable;
      m_pCompositeTable = new uint32[m_PrimesUsedInWindow + 1];
      memcpy(m_pCompositeTable, temp, saveUsedInWindow*sizeof(uint32));
      delete [] temp;
   }
   else
      m_pCompositeTable = new uint32[m_PrimesUsedInWindow + 1];

   thisPrime = saveMaxPrimeUsed;

   // Find the largest composite greater than lowEndOfWindow
   for (i=saveUsedInWindow; i<m_PrimesUsedInWindow; i++)
   {
      thisPrime += m_pPrimeTable[i];
      lastComposite = (m_LowEndOfWindow / thisPrime) * thisPrime;
      m_pCompositeTable[i] = (uint32) (lastComposite + thisPrime - m_LowEndOfWindow);

      // We only care about odd composites since the
      // sieve range only refers to odd values
      if (!(m_pCompositeTable[i] & 1))
         m_pCompositeTable[i] += thisPrime;
   }
}

uint64   PrimeServer::NextPrime(bool isIndexing)
{
   uint64   candidate, searchFor;
   uint32   currentByte, i;
   uint8    currentBit;

   if (m_LastSearchValue > 0)
      searchFor = m_LastSearchValue;
   else
      searchFor = m_LastPrimeReturned;

   // If we aren't indexing, but then call ByIndex, this will restart the sieve.
   if (!isIndexing) m_IndexOfLastPrimeReturned = 0;

   m_LastSearchValue = 0;

   // If we just started our sieve, then return 2 as it is the first prime.  As the
   // tables hold no even values, we need special logic for it here.
   if (searchFor < 2)
   {
      m_IndexInWindow = false;
      m_LastPrimeReturned = 2;
      return m_LastPrimeReturned;
   }

   // If the next prime is in the table of low primes, then grab it from there
   if (searchFor < m_MaxPrimeInPrimeTable)
   {
      candidate = 0;

      for (i=0; i<m_PrimesInPrimeTable; i++)
      {
         candidate += m_pPrimeTable[i];

         if (candidate > searchFor)
         {
            m_IndexInWindow = false;
            m_LastPrimeReturned = candidate;
            return m_LastPrimeReturned;
         }
      }
   }

   // The next prime is going to be in the current window or the next window.
   while (1 == 1)
   {
      if (m_IndexInWindow)
      {
         candidate = m_LastPrimeReturned;
         currentBit = m_LastBit;
         currentByte = m_LastByte;
      }
      else
      {
         candidate = m_LowEndOfWindow + 1;
         currentBit = 0x01;
         currentByte = 0;
      }

      while (candidate < m_HighEndOfWindow)
      {
         if (candidate > searchFor && (m_pSieve[currentByte] & currentBit))
         {
            m_IndexInWindow = true;
            m_LastBit = currentBit;
            m_LastByte = currentByte;
            m_LastPrimeReturned = candidate;

            if (candidate  > m_UpperLimit - 100000)
               SetUpperLimit(100.0 * m_UpperLimit);

            return candidate;
         }

         if (currentBit == 0x80)
         {
            currentBit = 0x01;
            currentByte++;
         }
         else
            currentBit <<= 1;

         candidate += 2;
      }

      BuildWindow(true);
   }
}

uint64   PrimeServer::ByIndex(int64 index)
{
   uint64   candidate;
   uint32   currentByte, currentBit, i;

   if (index == 1)
   {
      m_LastPrimeReturned = 2;
      return m_LastPrimeReturned;
   }

   if (index == m_IndexOfLastPrimeReturned + 1)
   {
      m_IndexOfLastPrimeReturned = index;
      return NextPrime(true);
   }

   // Index values below 1 always return 1 because they aren't valid
   if (index < 1) return 1;

   m_IndexOfLastPrimeReturned = index;

   candidate = 0;

   for (i=0; i<m_PrimesInPrimeTable; i++)
   {
      candidate += m_pPrimeTable[i];

      if (i == index-2)
      {
         m_LastPrimeReturned = candidate;
         return m_LastPrimeReturned;
      }
   }

   // Account for 2, which isn't in the table.
   i = m_PrimesInPrimeTable+1;

   // The next prime is going to be in the current window or the next window.
   while (1 == 1)
   {
      candidate = m_LowEndOfWindow + 1;
      currentBit = 0x01;
      currentByte = 0;

      while (candidate < m_HighEndOfWindow)
      {
         if (candidate >= m_MaxPrimeInPrimeTable)
         {
            if (m_pSieve[currentByte] & currentBit)
            {
               if (++i == index)
               {
                  m_IndexOfLastPrimeReturned = index;

                  if (candidate  > m_UpperLimit - 100000)
                     SetUpperLimit(100.0 * m_UpperLimit);

                  return candidate;
               }
            }
         }

         if (currentBit == 0x80)
         {
            currentBit = 0x01;
            currentByte++;
         }
         else
            currentBit <<= 1;

         candidate += 2;
      }

      BuildWindow(true);
   }
}
