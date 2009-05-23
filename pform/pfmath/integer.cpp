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

#ifdef GW_EMULATEASSEMBLER
int Integer::m_mod(const int32 n) const
{
	int i=mpz_mod_ui(*(mpz_t*)(&scrap),m_g,n);
	return i;
}
	
#else

int Integer::m_mod(const int32 n) const
{
	if (n == 0)
		return (1 / n); // generate int divide by zero error;

	if ((m_len == 0) || (n == 1))
		return 0;

	// fixup.  GMP uses a negative length to designate a negative number.  We now have to take
	// mod's on negative numbers (restoring), so we MUST fix this number, or our code will puke.
	int len = m_len;
	if (len < 0)
		len = -len;

	int r = Imod((uint32*)m_a, n, len);

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
	int	bits_in_array;

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



#ifdef HISTORICAL_CODE_THAT_DOES_PROTH_MODULAR_REDUCTIONS_ON_GMP_NUMBERS



// Specialized Proth exponentation
//
//  Performs exponenation with reduction being used:
// 
//   P=k*2^n-1   R%P == (R%2^n) + ((R>>n)/k)<<1 + ((r>>n)%k)<<n
void Integer::Proth_m_prp(uint64 k, uint32 n, uint32 base)
{
	if (k <= 0xFFFFFFFF)
	{
		Proth_m_prp((uint32)k, n, base);
		return;
	}
}


// The static temps used by the reduction code(s).
mpz_t Integer::mpz_K;
mpz_t Integer::mpz_Q;
mpz_t Integer::mpz_R;
mpz_t Integer::mpz_N;
mpz_t Integer::mpz_B;
mpz_t Integer::mpz_Y;

// Prepare the temp's used within the modular reduction code.
void Integer::InitProthMods()
{
	mpz_init(mpz_K);
	mpz_init(mpz_Q);
	mpz_init(mpz_R);
	mpz_init(mpz_N);
	mpz_init(mpz_B);
	mpz_init(mpz_Y);
}
// Clean up the temps from the mod reduction code
void Integer::FreeProthMods()
{
	mpz_clear(mpz_Y);
	mpz_clear(mpz_B);
	mpz_clear(mpz_N);
	mpz_clear(mpz_R);
	mpz_clear(mpz_Q);
	mpz_clear(mpz_K);
}

// *this == *this%(k*2^n+1)
GW_INLINE void Integer::Proth_mod(uint64 k, uint32 n)
{
	Proth_mod(k, n);
}

// *this == *this%(k*2^n-1)
GW_INLINE void Integer::Proth_m_mod(uint64 k, uint32 n, const Integer &m)
{
	Proth_m_mod(k, n, m);
}

// *this == *this%(k*2^n-1)
GW_INLINE void Integer::Proth_m_mod(uint32 k, uint32 n, const Integer &m)
{
	Proth_m_mod(k, n, m);
}


// This function is a modular reduction function for where M is of the form  M=k*2^n+1
// The reduction is done by:   R%M ==   R%2^n - (R>>n)/k + ((R>>n)%k)<<n
//
// NOTE this function should ONLY be called when it's value is from 0 to M^2-1.
// This function does not do a "full" perfect reduction.  The value will be
// in the range of M to -M.  This is fully acceptable for the squaring phase of
// proth exponentation.  That being the case (i.e. no bad side effects), the
// check at the end of the function which does:  if (n < 0) n += M 
// has been eliminated from this funcition (i.e. return is from M to -M.
// In the parts of the exponenation which "care" about the negative values,
// one has to add the check for negative, to correct.  Elimination of this
// check within the "squaring only" speeds up the full exponentation quite
// a bit (and for GF checking, the "squaring only" is ALL of the work done.
//
GW_INLINE void mpz_Proth_mod(mpz_t mpz_val, uint32 k, uint32 exp)
{
	//mpz_val = mpz_val%2^exp - (mpz_val>>exp)/k + ((mpz_val>>exp)%k)<<exp
	mpz_tdiv_q_2exp(Integer::mpz_Q,mpz_val,exp);	// q = mpz_val >> exp
	mpz_val->_mp_size = (exp+31) >> 5;				// mpz_val % 2^exp
	mpz_val->_mp_d[exp>>5] &= (1<<(exp&31))-1;		// ...

	mpz_tdiv_qr_ui(Integer::mpz_Q,Integer::mpz_R,Integer::mpz_Q,k);	// q == (mpz_val>>exp)/k  r == (mpz_val>>exp)%k
	mpz_mul_2exp(Integer::mpz_R,Integer::mpz_R,exp);
	mpz_add(mpz_val, mpz_val, Integer::mpz_R);
	mpz_sub(mpz_val, mpz_val, Integer::mpz_Q);
//	if (mpz_val->_mp_size < 0)
//		mpz_add(mpz_val, mpz_val, Integer::mpz_N);
}

GW_INLINE void mpz_Proth_mod64(mpz_t mpz_val, uint32 exp)	// Used when k is > 2^32
{
	//mpz_val = mpz_val%2^exp - (mpz_val>>exp)/k + ((mpz_val>>exp)%k)<<exp
	mpz_tdiv_q_2exp(Integer::mpz_Q,mpz_val,exp);	// q = mpz_val >> exp
	mpz_val->_mp_size = (exp+31) >> 5;				// mpz_val % 2^exp
	mpz_val->_mp_d[exp>>5] &= (1<<(exp&31))-1;		// ...

	mpz_tdiv_qr(Integer::mpz_Q,Integer::mpz_R,Integer::mpz_Q,Integer::mpz_K);	// q == (mpz_val>>exp)/k  r == (mpz_val>>exp)%k
	mpz_mul_2exp(Integer::mpz_R,Integer::mpz_R,exp);
	mpz_add(mpz_val, mpz_val, Integer::mpz_R);
	mpz_sub(mpz_val, mpz_val, Integer::mpz_Q);
//	if (m_g->_mp_size < 0)
//		mpz_add(m_g, m_g, M);
}

// base^(k*2^n)%(k*2^n+1)
void Integer::Proth_prp(uint32 k, uint32 n, uint32 base)
{
	uint32 n_Tmp = n;
	uint32 k_Tmp = k;
	mpz_add_ui(mpz_N, m_g, 1);
	mpz_set_ui(mpz_B, base);

	while (n_Tmp && mpz_B->_mp_size < mpz_N->_mp_size)
	{
		--n_Tmp;
		// initially ignore reduction UNTIL the size of B is very close to (or above) the size of N.  This
		// allows us to skip lg2(lg2(N)) reductions totally, due to knowledge that the number is less than N
		mpz_mul(mpz_B, mpz_B, mpz_B);
	}

	// Since B is most likely larger than N, we need to do our first reduction.  From 
	// this point on, we reduce after every multipy or square.

	// For this whole function, we do not "care" if B is reduced from -1 to -N+1, with the exception of
	// just before we multiply Y by B.  At that time, we may need to adjust B back into the N-1 to 0 
	// range.   Our mpz_Proth_mod reduced the number to N > abs(reduced) >= 0   This is FINE for the
	// sqaring only stages, but needs to be adjusted (with a possible addition of N) for the multiplication
	// stage of this exponentator.
	mpz_Proth_mod(mpz_B,k,n);

	while (n_Tmp)
	{
		--n_Tmp;
		mpz_mul(mpz_B, mpz_B, mpz_B);
		mpz_Proth_mod(mpz_B,k,n);
		// During this loop, we don't care if B ends up being 0 > B > -N
	}

	// From this point on, whenever we work with Y, we DO care that the return from Proth_mod() is 
	// "normalized" from N-1 to 0 and not N-1 to -N+1 For that, we have to check to see if the return
	// from all Proth_mod() functions is negative.  If it is, then simply add a N to it (to put it
	// into the range of 0 to N-1)
	if (mpz_B->_mp_size < 0)
		mpz_add(mpz_Y, mpz_B, mpz_N);		// Y = B  (normalized to N<B<= 0)
	else
		mpz_set(mpz_Y, mpz_B);

	k_Tmp>>=1;
	while (k_Tmp)
	{
		mpz_mul(mpz_B, mpz_B, mpz_B);
		mpz_Proth_mod(mpz_B,k,n);
		// No need to "adjust" B to be normalized here.  We do that just prior to multiplying Y by B
		if (k_Tmp & 1)
		{
			// Both B and Y must be either possitive or both negative.  If one is pos and one is neg, then
			// we do not get correct results.   However, we want to minimze our GMP work.  Checking a 
			// these values saves us 50% of the mpz_add calls.
			if (mpz_B->_mp_size < 0 && mpz_Y->_mp_size > 0)
				// Correct for a negative B at this point (right before multiplying with a "known" possitive Y)
				mpz_add(mpz_B, mpz_B, mpz_N);
			if (mpz_B->_mp_size > 0 && mpz_Y->_mp_size < 0)
				// Correct for a negative Y at this point (right before multiplying with a "known" possitive B)
			mpz_add(mpz_Y, mpz_Y, mpz_N);
			mpz_mul(mpz_Y, mpz_Y, mpz_B);
			mpz_Proth_mod(mpz_Y,k,n);
		}
		k_Tmp >>= 1;
	}
	// We absolulely want to fix
	if (mpz_Y->_mp_size < 0)
		mpz_add(mpz_Y, mpz_Y, mpz_N);
	mpz_set(m_g, mpz_Y);
}

// Specialized Proth exponentation
//
//  Performs exponenation with reduction being used:
// 
//   P=k*2^n+1   R%P == (R%2^n) - (R>>n)/k + ((r>>n)%k)<<n
void Integer::Proth_prp(uint64 k, uint32 n, uint32 base)
{
	if (k <= 0xFFFFFFFF)
	{
		Proth_prp((uint32)k, n, base);
		return;
	}

	// 2^64 > k >= 2^32
	mpz_set_ui(mpz_K, (uint32)(k>>32));
	mpz_mul_2exp(mpz_K, mpz_K, 32);
	mpz_add_ui(mpz_K,mpz_K,(uint32)(k&0xFFFFFFFF));

	uint32 n_Tmp = n;
	mpz_add_ui(mpz_N, m_g, 1);
	mpz_set_ui(mpz_B, base);

	while (n_Tmp && mpz_B->_mp_size < mpz_N->_mp_size)
	{
		--n_Tmp;
		// initially ignore reduction UNTIL the size of B is very close to (or above) the size of N.  This
		// allows us to skip lg2(lg2(N)) reductions totally, due to knowledge that the number is less than N
		mpz_mul(mpz_B, mpz_B, mpz_B);
	}

	// Since B is most likely larger than N, we need to do our first reduction.  From 
	// this point on, we reduce after every multipy or square.

	// For this whole function, we do not "care" if B is reduced from -1 to -N+1, with the exception of
	// just before we multiply Y by B.  At that time, we may need to adjust B back into the N-1 to 0 
	// range.   Our mpz_Proth_mod reduced the number to N > abs(reduced) >= 0   This is FINE for the
	// sqaring only stages, but needs to be adjusted (with a possible addition of N) for the multiplication
	// stage of this exponentator.
	mpz_Proth_mod64(mpz_B,n);

	while (n_Tmp)
	{
		--n_Tmp;
		mpz_mul(mpz_B, mpz_B, mpz_B);
		mpz_Proth_mod64(mpz_B,n);
		// During this loop, we don't care if B ends up being 0 > B > -N
	}

	// From this point on, whenever we work with Y, we DO care that the return from Proth_mod() is 
	// "normalized" from N-1 to 0 and not N-1 to -N+1 For that, we have to check to see if the return
	// from all Proth_mod() functions is negative.  If it is, then simply add a N to it (to put it
	// into the range of 0 to N-1)
	if (mpz_B->_mp_size < 0)
		mpz_add(mpz_Y, mpz_B, mpz_N);		// Y = B  (normalized to N<B<= 0)
	else
		mpz_set(mpz_Y, mpz_B);

	// We "can" modify k here.  DUE to the fact mpz_Proth_mod64 does not need the k value, but uses the mpz_K value.
	k>>=1;
	while (k)
	{
		mpz_mul(mpz_B, mpz_B, mpz_B);
		mpz_Proth_mod64(mpz_B,n);
		// No need to "adjust" B to be normalized here.  We do that just prior to multiplying Y by B
		if (k & 1)
		{
			// Both B and Y must be either possitive or both negative.  If one is pos and one is neg, then
			// we do not get correct results.   However, we want to minimze our GMP work.  Checking a 
			// these values saves us 50% of the mpz_add calls.
			if (mpz_B->_mp_size < 0 && mpz_Y->_mp_size > 0)
				// Correct for a negative B at this point (right before multiplying with a "known" possitive Y)
				mpz_add(mpz_B, mpz_B, mpz_N);
			if (mpz_B->_mp_size > 0 && mpz_Y->_mp_size < 0)
				// Correct for a negative Y at this point (right before multiplying with a "known" possitive B)
			mpz_add(mpz_Y, mpz_Y, mpz_N);
			mpz_mul(mpz_Y, mpz_Y, mpz_B);
			mpz_Proth_mod64(mpz_Y,n);
		}
		k >>= 1;
	}
	// We absolulely want to fix
	while (mpz_Y->_mp_size < 0)
		mpz_add(mpz_Y, mpz_Y, mpz_N);
	mpz_set(m_g, mpz_Y);

}


// base^(k*2^n-2)%(k*2^n-1)
void Integer::Proth_m_prp(uint32 k, uint32 n, uint32 base)
{
	Proth_m_prp(k,n,base);
}


#endif



int bit(const Integer &x,const uint32 n)
{
	int r=0;
	
	if(n<unsigned(x.m_len*mp_bits_per_limb))
	{
		int i=n/mp_bits_per_limb;
		int m=n%mp_bits_per_limb;
		
		r=(x.m_a[i]&(1UL<<m))?1:0;
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
