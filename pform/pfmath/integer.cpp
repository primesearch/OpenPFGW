#include "pfmathpch.h"
#include "integer.h"
#include "../pfgwlib/gwcontext.h"
#include "../pfgwlib/gwinteger.h"

#ifndef GW_INLINE_ENABLED
#include "integer.inl"
#endif

// Initialize the libraries' memory allocation
void memAlloc()
{
}

// Free any static memory allocated by the library
void memFree()
{
}

// multiplication
void Integer::m_mul(const int32 n)
{
   if(n>=0)
   {
      mpz_mul_ui(m_g,m_g,n);
   }
   else
   {
      mpz_mul_ui(m_g,m_g,-n);
      mpz_neg(m_g,m_g);
   }
}

// division
long Integer::m_div(const int32 n)
{
   Integer r;
   if(n>=0)
   {
      mpz_tdiv_qr_ui(m_g,r.m_g,m_g,n);
   }
   else
   {
      mpz_tdiv_qr_ui(m_g,r.m_g,m_g,-n);
      mpz_neg(m_g,m_g);
   }
   return mpz_get_si(r.m_g);
}

#ifdef _64BIT
int  Integer::m_mod(const int32 n) const
{
   return (int32) mpz_mod_ui(*(mpz_t*)(&scrap),m_g,n);
}
#else
int Integer::m_mod(const int32 n) const
{
   int   r;

   if (n == 0)
      return (1 / n); // generate int divide by zero error;

   if ((m_len == 0) || (n == 1))
      return 0;

   // fixup.  GMP uses a negative length to designate a negative number.  We now have to take
   // mod's on negative numbers (restoring), so we MUST fix this number, or our code will puke.
   int len = m_len;
   if (len < 0)
      len = -len;

   r = _Imod((uint32*)m_a, n, len);

   return r;
}
#endif

void Integer::m_sftr(const int32 n)
{
   if(n>=0)
   {
      mpz_tdiv_q_2exp(m_g,m_g,n);
   }
   else
   {
      mpz_mul_2exp(m_g,m_g,-n);
   }
}


Integer &Integer::operator=(const GWInteger &I)
{
   int   bits_in_array;

   // This isn't a great call. This single bit sits in
   // a vast chunk of memory, just to force gmp to allocate.
   bits_in_array = (int) gwdata.bit_length + 100;
   *this = 1;
   *this <<= bits_in_array;

   if (sizeof (mp_limb_t) == sizeof (uint32_t))
      m_g->_mp_size = gwtobinary (&gwdata, I.g, (uint32_t *) m_g->_mp_d, m_g->_mp_size);
   else
      m_g->_mp_size = gwtobinary64 (&gwdata, I.g, (uint64_t *) m_g->_mp_d, m_g->_mp_size);

   return *this;
}

int bit(const Integer &x,const uint32 n)
{
   int r=0;

   if(n<unsigned(x.m_len*mp_bits_per_limb))
   {
      int i=n/mp_bits_per_limb;
      int m=n%mp_bits_per_limb;

#ifdef _64BIT
      r=(x.m_a[i]&(1ULL<<m))?1:0;
#else
      r=(x.m_a[i]&(1UL<<m))?1:0;
#endif
   }

   return r;
}

uint32 crc32(const Integer &x)
{
   mp_limb_t l;
   uint32 crcval=0;
   char *p=(char *)(&l);

   l=(mp_limb_t)(x.m_len);

   uint32 i,j;

   for(i=0;i<sizeof(mp_limb_t);i++)
   {
      crcval=crc_byte(p[i],crcval);
   }

   for(j=0;j<(uint32)abs(x.m_len);j++)
   {
      l=x.m_a[j];
      for(i=0;i<sizeof(mp_limb_t);i++)
      {
         crcval=crc_byte(p[i],crcval);
      }
   }
   return crcval;
}
