#ifndef PRIMESERVER_H
#define PRIMESERVER_H

class PrimeServer;

extern PrimeServer *primeserver;

class PrimeServer
{
public:
	PrimeServer(uint64 upperLimit = 4000000000ULL);
	PrimeServer(double upperLimit);

   ~PrimeServer();

   // The input to this function could be prime or composite.
   void     SkipTo(uint64 searchValue) { SetWindow(searchValue); };

   // Get the next prime after a call to SkipToValue().  One can then iterate through
   // the primes starting from searchValue+1.
   uint64   NextPrime(bool isIndexing = false);

   // If m_IndexOfLastPrimeReturned = 0, then index to the given prime.  PrimeServer
   // can then iterate through the primes as long as each successive call increments
   // index by 1.  Note that ByIndex(1) = 2.
   uint64   ByIndex(int64 index);

   // If the last (or next) prime might be composite
   bool     GetMightBeComposite(void) { return m_OutputWarning; };

   uint64   GetUpperLimit(void) { return m_UpperLimit; };

private:
   void     Initialize(void);

   void     SetWindow(uint64 searchValue);
   void     BuildWindow(bool nextWindow = true, bool restart = false, uint64 searchValue = 0);
   
   void     SetupSieve(void);

   uint64   m_UpperLimit;

   // This holds all primes < sqrt(m_UpperLimit).  Instead of storing the
   // primes themselves, it stores the difference between consecutive primes.
   uint8   *m_pPrimeTable;

   // This is the largest prime in m_pPrimeTable.
   uint64   m_MaxPrimeInPrimeTable;
   
   // This is the number of entries allocated for m_pPrimeTable.
   uint32   m_MaxEntriesInPrimeTable;

   // This is the number of entries with values in m_pPrimeTable.
   uint32   m_PrimesInPrimeTable;

   // This is the largest prime used to populate the current window.
   uint32   m_MaxPrimeUsed;

   // This table has the same number of entries as primeTable.  The values in
   // this table correspond to the smallest composite greater than the low end
   // of the window.
   uint32  *m_pCompositeTable;

   // This is the current number of primes in primeTable that are being
   // used to sieve the current window.
   uint32   m_PrimesUsedInWindow;

   // These the lowest and highest values in the current window.
   uint64   m_LowEndOfWindow;
   uint64   m_HighEndOfWindow;

   // These are used to hold the current sieve window.
   uint8   *m_pSieve;

   // This indicates if we have output a message indicating that the primeserver
   // has reached its limit.
   bool     m_OutputWarning;

   // Holds last prime returned by NextPrime and ByIndex so that subsequent
   // calls don't need to start from the beginning.
   uint64   m_LastPrimeReturned;
   uint32   m_LastByte;
   uint8    m_LastBit;
   bool     m_IndexInWindow;

   // Holds last value passed to SkipTo so that subsequent calls to NextPrime
   // don't need to start from the beginning.  This is set to 0 by NextPrime.
   uint64   m_LastSearchValue;

   // Holds index last prime returned by ByIndex so that subsequent calls
   // don't need to start from the beginning.  This is set to 0 by NextPrime
   // if NextPrime is called with "false".
   int64    m_IndexOfLastPrimeReturned;
};

#endif
