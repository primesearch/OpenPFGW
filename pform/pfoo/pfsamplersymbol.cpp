#include "pfoopch.h"
#include "symboltypes.h"
#include "pfsamplersymbol.h"
#include "primeserver.h"

PFSamplerSymbol::PFSamplerSymbol()
   :  IPFSymbol("_SAMPLER"), m_dwLastCRC(0), m_dwLargePrime(255),
      m_dwSmallIndex(0), m_dwAcceptIndex(0), m_dwSmallCount(0)
{
   // the sampler works in a simple way. You 'ask' for a prime, if
   // you use it, you 'accept' it. When you ask, tables are reset.
   // When you accept small primes (<256) they are moved to the end
   // of the small primes queue

   // there are 54 primes less than 256, I think
   primeserver->restart();
   uint32 p;
   do
   {
      primeserver->next(p);
      if(p<256)
      {
         m_dwSmallPrimes[m_dwSmallCount++]=p;
      }
   }
   while(p<256);
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
uint32 PFSamplerSymbol::ask(const Integer &N)
{
   uint32 newcrc=crc32(N);

   if(newcrc!=m_dwLastCRC)
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
uint32 PFSamplerSymbol::askagain()
{
   uint32 r;

   if(m_dwSmallIndex<m_dwSmallCount)
   {
      r=m_dwSmallPrimes[m_dwSmallIndex++];
   }
   else
   {
      // ran out of little ones, so get a big one
      if(m_dwLargePrime<256)
      {
         primeserver->restart();
         primeserver->skip(m_dwLargePrime+1);
      }
      primeserver->next(m_dwLargePrime);
      r=m_dwLargePrime;
   }
   return r;
}

// accept the current prime
void PFSamplerSymbol::accept(uint32 p)
{
   if(p<256)
   {
      m_dwAcceptedPrimes[m_dwAcceptIndex++]=p;
   }
}

// rearrange so accepted primes are deferred for later
void PFSamplerSymbol::rearrange()
{
   uint32 iCopyTo=0;
   uint32 i,j;

   for(i=0;i<m_dwSmallCount;i++)
   {
      uint32 p=m_dwSmallPrimes[i];
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
