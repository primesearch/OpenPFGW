#include "pfmathpch.h"
#include <vector>
#include "integer.h"
#include "../pfgwlib/gwcontext.h"
#include "../pfgwlib/gwinteger.h"

#ifndef GW_INLINE_ENABLED
#include "integer.inl"
#endif

#define PRP_ERROR -1
#define PRP_COMPOSITE 0
#define PRP_PRP 1
#define PRP_PRIME 2

extern bool volatile g_bExitNow;

// Initialize the libraries' memory allocation
void memAlloc()
{
}

// Free any static memory allocated by the library
void memFree()
{
}

// multiplication
void Integer::m_mul(const int32_t n)
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
long Integer::m_div(const int32_t n)
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

   // Not convinced this is right.  What if m_g > 31 bits?
   return (int32_t) mpz_get_si(r.m_g);
}

#ifdef _64BIT
int  Integer::m_mod(const int32_t n) const
{
   return (int32_t) mpz_mod_ui(*(mpz_t*)(&scrap),m_g,n);
}
#else
int Integer::m_mod(const int32_t n) const
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

   r = Imod((uint32_t*)m_a, n, len);

   return r;
}
#endif

void Integer::m_sftr(const int32_t n)
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

int bit(const Integer &x,const uint32_t n)
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

uint32_t crc32(const Integer &x)
{
   mp_limb_t l;
   uint32_t crcval=0;
   char *p=(char *)(&l);

   l=(mp_limb_t)(x.m_len);

   uint32_t i,j;

   for(i=0;i<sizeof(mp_limb_t);i++)
   {
      crcval=crc_byte(p[i],crcval);
   }

   for(j=0;j<(uint32_t)abs(x.m_len);j++)
   {
      l=x.m_a[j];
      for(i=0;i<sizeof(mp_limb_t);i++)
      {
         crcval=crc_byte(p[i],crcval);
      }
   }
   return crcval;
}

/* *********************************************************************************************
 * mpz_sprp: (also called a Miller-Rabin pseudoprime)
 * A "strong pseudoprime" to the base a is an odd composite n = (2^r)*s+1 with s odd such that
 * either a^s == 1 mod n, or a^((2^t)*s) == -1 mod n, for some integer t, with 0 <= t < r.
 * *********************************************************************************************/
int   Integer::mpz_sprp(mpz_t n, mpz_t a)
{
  mpz_t s;
  mpz_t nm1;
  mpz_t mpz_test;
  mp_bitcnt_t r = 0;

  if (mpz_cmp_ui(a, 2) < 0)
    return PRP_ERROR;

  if (mpz_cmp_ui(n, 2) < 0)
    return PRP_COMPOSITE;

  if (mpz_divisible_ui_p(n, 2))
  {
    if (mpz_cmp_ui(n, 2) == 0)
      return PRP_PRIME;
    else
      return PRP_COMPOSITE;
  }

  mpz_init_set_ui(mpz_test, 0);
  mpz_init_set_ui(s, 0);
  mpz_init_set(nm1, n);
  mpz_sub_ui(nm1, nm1, 1);

  /***********************************************/
  /* Find s and r satisfying: n-1=(2^r)*s, s odd */
  r = mpz_scan1(nm1, 0);
  mpz_fdiv_q_2exp(s, nm1, r);


  /******************************************/
  /* Check a^((2^t)*s) mod n for 0 <= t < r */
  mpz_powm(mpz_test, a, s, n);
  if ( (mpz_cmp_ui(mpz_test, 1) == 0) || (mpz_cmp(mpz_test, nm1) == 0) )
  {
    mpz_clear(s);
    mpz_clear(nm1);
    mpz_clear(mpz_test);
    return PRP_PRP;
  }

  while ( --r )
  {
    /* mpz_test = mpz_test^2%n */
    mpz_mul(mpz_test, mpz_test, mpz_test);
    mpz_mod(mpz_test, mpz_test, n);

    if (mpz_cmp(mpz_test, nm1) == 0)
    {
      mpz_clear(s);
      mpz_clear(nm1);
      mpz_clear(mpz_test);
      return PRP_PRP;
    }
  }

  mpz_clear(s);
  mpz_clear(nm1);
  mpz_clear(mpz_test);
  return PRP_COMPOSITE;

}/* method mpz_sprp */


/* *********************************************************************************************************
 * mpz_strongselfridge_prp:
 * A "strong Lucas-Selfridge pseudoprime" n is a "strong Lucas pseudoprime" using Selfridge parameters of:
 * Find the first element D in the sequence {5, -7, 9, -11, 13, ...} such that Jacobi(D,n) = -1
 * Then use P=1 and Q=(1-D)/4 in the strong Lucase pseudoprime test.
 * Make sure n is not a perfect square, otherwise the search for D will only stop when D=n.
 * **********************************************************************************************************/
int   Integer::mpz_strongselfridge_prp(mpz_t n)
{
  long int d = 5, p = 1, q = 0;
  int max_d = 1000000;
  int jacobi = 0;
  mpz_t zD;

  if (mpz_cmp_ui(n, 2) < 0)
    return PRP_COMPOSITE;

  if (mpz_divisible_ui_p(n, 2))
  {
    if (mpz_cmp_ui(n, 2) == 0)
      return PRP_PRIME;
    else
      return PRP_COMPOSITE;
  }

  mpz_init_set_ui(zD, d);

  while (1)
  {
    jacobi = mpz_jacobi(zD, n);

    /* if jacobi == 0, d is a factor of n, therefore n is composite... */
    /* if d == n, then either n is either prime or 9... */
    if (jacobi == 0)
    {
      if ((mpz_cmpabs(zD, n) == 0) && (mpz_cmp_ui(zD, 9) != 0))
      {
        mpz_clear(zD);
        return PRP_PRIME;
      }
      else
      {
        mpz_clear(zD);
        return PRP_COMPOSITE;
      }
    }
    if (jacobi == -1)
      break;

    /* if we get to the 5th d, make sure we aren't dealing with a square... */
    if (d == 13)
    {
      if (mpz_perfect_square_p(n))
      {
        mpz_clear(zD);
        return PRP_COMPOSITE;
      }
    }

    if (d < 0)
    {
      d *= -1;
      d += 2;
    }
    else
    {
      d += 2;
      d *= -1;
    }

    /* make sure we don't search forever */
    if (d >= max_d)
    {
      mpz_clear(zD);
      return PRP_ERROR;
    }

    mpz_set_si(zD, d);
  }
  mpz_clear(zD);

  q = (1-d)/4;

  return mpz_stronglucas_prp(n, p, q);

}/* method mpz_strongselfridge_prp */


/* *********************************************************************************************
 * mpz_stronglucas_prp:
 * A "strong Lucas pseudoprime" with parameters (P,Q) is a composite n = (2^r)*s+(D/n), where
 * s is odd, D=P^2-4Q, and (n,2QD)=1 such that either U_s == 0 mod n or V_((2^t)*s) == 0 mod n
 * for some t, 0 <= t < r. [(D/n) is the Jacobi symbol]
 * *********************************************************************************************/
int   Integer::mpz_stronglucas_prp(mpz_t n, long int p, long int q)
{
  mpz_t zD;
  mpz_t s;
  mpz_t nmj; /* n minus jacobi(D/n) */
  mpz_t res;
  mpz_t uh, vl, vh, ql, qh, tmp; /* these are needed for the LucasU and LucasV part of this function */
  long int d = p*p - 4*q;
  mp_bitcnt_t r = 0;
  int ret = 0;
  int j = 0;

  if (d == 0) /* Does not produce a proper Lucas sequence */
    return PRP_ERROR;

  if (mpz_cmp_ui(n, 2) < 0)
    return PRP_COMPOSITE;

  if (mpz_divisible_ui_p(n, 2))
  {
    if (mpz_cmp_ui(n, 2) == 0)
      return PRP_PRIME;
    else
      return PRP_COMPOSITE;
  }

  mpz_init_set_si(zD, d);
  mpz_init(res);

  mpz_mul_si(res, zD, q);
  mpz_mul_ui(res, res, 2);
  mpz_gcd(res, res, n);
  if ((mpz_cmp(res, n) != 0) && (mpz_cmp_ui(res, 1) > 0))
  {
    mpz_clear(zD);
    mpz_clear(res);
    return PRP_COMPOSITE;
  }

  mpz_init(s);
  mpz_init(nmj);

  /* nmj = n - (D/n), where (D/n) is the Jacobi symbol */
  mpz_set(nmj, n);
  ret = mpz_jacobi(zD, n);
  if (ret == -1)
    mpz_add_ui(nmj, nmj, 1);
  else if (ret == 1)
    mpz_sub_ui(nmj, nmj, 1);

  r = mpz_scan1(nmj, 0);
  mpz_fdiv_q_2exp(s, nmj, r);

  /* make sure U_s == 0 mod n or V_((2^t)*s) == 0 mod n, for some t, 0 <= t < r */
  mpz_init_set_si(uh, 1);
  mpz_init_set_si(vl, 2);
  mpz_init_set_si(vh, p);
  mpz_init_set_si(ql, 1);
  mpz_init_set_si(qh, 1);
  mpz_init_set_si(tmp,0);

  for (j = (int) mpz_sizeinbase(s,2)-1; j >= 1; j--)
  {
    /* ql = ql*qh (mod n) */
    mpz_mul(ql, ql, qh);
    mpz_mod(ql, ql, n);
    if (mpz_tstbit(s,j) == 1)
    {
      /* qh = ql*q */
      mpz_mul_si(qh, ql, q);

      /* uh = uh*vh (mod n) */
      mpz_mul(uh, uh, vh);
      mpz_mod(uh, uh, n);

      /* vl = vh*vl - p*ql (mod n) */
      mpz_mul(vl, vh, vl);
      mpz_mul_si(tmp, ql, p);
      mpz_sub(vl, vl, tmp);
      mpz_mod(vl, vl, n);

      /* vh = vh*vh - 2*qh (mod n) */
      mpz_mul(vh, vh, vh);
      mpz_mul_si(tmp, qh, 2);
      mpz_sub(vh, vh, tmp);
      mpz_mod(vh, vh, n);
    }
    else
    {
      /* qh = ql */
      mpz_set(qh, ql);

      /* uh = uh*vl - ql (mod n) */
      mpz_mul(uh, uh, vl);
      mpz_sub(uh, uh, ql);
      mpz_mod(uh, uh, n);

      /* vh = vh*vl - p*ql (mod n) */
      mpz_mul(vh, vh, vl);
      mpz_mul_si(tmp, ql, p);
      mpz_sub(vh, vh, tmp);
      mpz_mod(vh, vh, n);

      /* vl = vl*vl - 2*ql (mod n) */
      mpz_mul(vl, vl, vl);
      mpz_mul_si(tmp, ql, 2);
      mpz_sub(vl, vl, tmp);
      mpz_mod(vl, vl, n);
    }
  }
  /* ql = ql*qh */
  mpz_mul(ql, ql, qh);

  /* qh = ql*q */
  mpz_mul_si(qh, ql, q);

  /* uh = uh*vl - ql */
  mpz_mul(uh, uh, vl);
  mpz_sub(uh, uh, ql);

  /* vl = vh*vl - p*ql */
  mpz_mul(vl, vh, vl);
  mpz_mul_si(tmp, ql, p);
  mpz_sub(vl, vl, tmp);

  /* ql = ql*qh */
  mpz_mul(ql, ql, qh);

  mpz_mod(uh, uh, n);
  mpz_mod(vl, vl, n);

  /* uh contains LucasU_s and vl contains LucasV_s */
  if ((mpz_cmp_ui(uh, 0) == 0) || (mpz_cmp_ui(vl, 0) == 0))
  {
    mpz_clear(zD);
    mpz_clear(s);
    mpz_clear(nmj);
    mpz_clear(res);
    mpz_clear(uh);
    mpz_clear(vl);
    mpz_clear(vh);
    mpz_clear(ql);
    mpz_clear(qh);
    mpz_clear(tmp);
    return PRP_PRP;
  }

  for (j = 1; j < r; j++)
  {
    /* vl = vl*vl - 2*ql (mod n) */
    mpz_mul(vl, vl, vl);
    mpz_mul_si(tmp, ql, 2);
    mpz_sub(vl, vl, tmp);
    mpz_mod(vl, vl, n);

    /* ql = ql*ql (mod n) */
    mpz_mul(ql, ql, ql);
    mpz_mod(ql, ql, n);

    if (mpz_cmp_ui(vl, 0) == 0)
    {
      mpz_clear(zD);
      mpz_clear(s);
      mpz_clear(nmj);
      mpz_clear(res);
      mpz_clear(uh);
      mpz_clear(vl);
      mpz_clear(vh);
      mpz_clear(ql);
      mpz_clear(qh);
      mpz_clear(tmp);
      return PRP_PRP;
    }
  }

  mpz_clear(zD);
  mpz_clear(s);
  mpz_clear(nmj);
  mpz_clear(res);
  mpz_clear(uh);
  mpz_clear(vl);
  mpz_clear(vh);
  mpz_clear(ql);
  mpz_clear(qh);
  mpz_clear(tmp);
  return PRP_COMPOSITE;

}/* method mpz_stronglucas_prp */

