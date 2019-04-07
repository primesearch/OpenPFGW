#include "pfoopch.h"
#include <vector>
#include <primesieve.hpp>
#include "symboltypes.h"
#include "pfsamplersymbol.h"

PFSamplerSymbol::PFSamplerSymbol()
   :  IPFSymbol("_SAMPLER"), m_dwLastCRC(0), m_dwLargePrime(255),
      m_dwSmallIndex(0), m_dwAcceptIndex(0), m_dwSmallCount(0)
{   
   std::vector<uint64_t> vPrimes;
   std::vector<uint64_t>::iterator it;

   vPrimes.clear();

   primesieve::generate_primes(1, 256, &vPrimes);

   // the sampler works in a simple way. You 'ask' for a prime, if
   // you use it, you 'accept' it. When you ask, tables are reset.
   // When you accept small primes (<256) they are moved to the end
   // of the small primes queue

   it = vPrimes.begin();
   while (it != vPrimes.end()) {
      m_dwSmallPrimes[m_dwSmallCount++] = (uint32_t)*it;
      it++;
   }
}

PFSamplerSymbol::~PFSamplerSymbol()
{
}

DWORD PFSamplerSymbol::GetSymbolType() const
{
   return SAMPLER_SYMBOL_TYPE;
}

PFString PFSamplerSymbol::GetStringValue()
{
   return("**for internal use only**");
}

// ask for a prime
uint32_t PFSamplerSymbol::ask(const Integer &N)
{
   uint32_t newcrc=crc32(N);

   if (newcrc != m_dwLastCRC)
   {
      rearrange();
      m_dwSmallIndex=0;
      m_dwAcceptIndex=0;
      m_dwLargePrime=255;
      m_dwLastCRC=newcrc;
   }
   return askagain();
}

// ask for another prime
uint32_t PFSamplerSymbol::askagain()
{
   uint32_t r;

   if (m_dwSmallIndex < m_dwSmallCount)
   {
      r = m_dwSmallPrimes[m_dwSmallIndex++];
   }
   else
   {
      // ran out of little ones, so get a big one
      m_dwLargePrime = (uint32_t) primesieve::nth_prime(1, m_dwLargePrime);

      r = m_dwLargePrime;
   }

   return r;
}

// accept the current prime
void PFSamplerSymbol::accept(uint32_t p)
{
   if(p<256)
   {
      m_dwAcceptedPrimes[m_dwAcceptIndex++]=p;
   }
}

// rearrange so accepted primes are deferred for later
void PFSamplerSymbol::rearrange()
{
   uint32_t iCopyTo=0;
   uint32_t i,j;

   for(i=0;i<m_dwSmallCount;i++)
   {
      uint32_t p=m_dwSmallPrimes[i];
      // find out if p is accepted
      for(j=0;j<m_dwAcceptIndex;j++)
      {
         if(p==m_dwAcceptedPrimes[j]) break;
      }

      if(j<m_dwAcceptIndex)
      {
      }
      else
      {
         m_dwSmallPrimes[iCopyTo++]=p;
      }
   }

   for(i=0;i<m_dwAcceptIndex;i++)
   {
      m_dwSmallPrimes[iCopyTo++]=m_dwAcceptedPrimes[i];
   }
}
