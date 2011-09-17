#include "fp_stchk.h"

/* I need signed ints, I have to be able to underflow */
#define i52 int64

// Can make return values sloppier (out by +/-p, say) by changing this
//
#define RET_SUB52(a,b,p) { i52 stab=a-b; if(stab<0) return stab+p; if(stab>=p) return stab-p; return stab; }

//------------------------------------------------------
// mod104_52_pr  | 
//---------------+
// xyl=low 64 bits of 'xy', a double-word integer
// xyf=approx value of xy as a float
// p  =prime
// prf=approx 1.0/prime
//
static inline i52 mod104_52_pr(int64 xyl, double xyf, i52 p, double prf)
{
    int64 rounded=(int64)p*(int64)(xyf*prf);  /* p*(xy/p) */
    RET_SUB52(xyl,rounded,p);
}

//------------------------------------------------------
// mulmod52_pr   | 
//---------------+
// x,y=integer
// p  =prime
// prf=approx 1.0/prime
//
static inline i52 muladdmod52_pr(i52 x, i52 y, i52 a, i52 p, double prf)
{
    return mod104_52_pr((int64)x*y+a, (double)x*y+a, p, prf);
}

GW_INLINE Integer::Integer()
{
	mpz_init(m_g);
#if defined(_64BIT)
	mpz_init(scrap);
#endif
}

GW_INLINE Integer::Integer(int32 n)
{
	mpz_init_set_si(m_g,n);
#if defined(_64BIT)
	mpz_init(scrap);
#endif
}

GW_INLINE Integer::Integer(uint32 n)
{
	mpz_init_set_ui(m_g,n);
#if defined(_64BIT)
	mpz_init(scrap);
#endif
}

GW_INLINE Integer::Integer(uint64 n64)
{
	uint32 n32 = uint32(n64>>32);
	if (!n32)
		// Simple case, n64 is only 32 bits.
		mpz_init_set_ui(m_g,(uint32)(n64&UINT_MAX));
	else
	{
		mpz_init_set_ui(m_g,n32);
		mpz_mul_2exp(m_g, m_g, 32);
		mpz_add_ui(m_g,m_g,(uint32)(n64&UINT_MAX));
	}

#if defined(_64BIT)
	mpz_init(scrap);
#endif
}

GW_INLINE Integer::Integer(const Integer & x)
{
	mpz_init_set(m_g,x.m_g);
#if defined(_64BIT)
	mpz_init(scrap);
#endif
}

GW_INLINE Integer::~Integer()
{
	mpz_clear(m_g);
#if defined(_64BIT)
	mpz_clear(scrap);
#endif
}

GW_INLINE void Integer::m_set(const int32 n)
{
	mpz_set_si(m_g,n);
}

GW_INLINE void Integer::m_set(const Integer &y)
{
	mpz_set(m_g,y.m_g);
}

// comparison
GW_INLINE int32 Integer::m_cmp(const int32 n) const
{
	return mpz_cmp_si(m_g,n);
}

GW_INLINE int32 Integer::m_cmp(const uint64 n64) const
{
   uint32 n32 = uint32(n64>>32);

   mpz_set_ui(*(mpz_t*)(&scrap), n32);
	mpz_mul_2exp(*(mpz_t*)(&scrap), scrap, 32);
	mpz_add_ui(*(mpz_t*)(&scrap), scrap, (uint32)(n64&UINT_MAX));

   return mpz_cmp(m_g,scrap);
}

GW_INLINE int32 Integer::m_cmp(const Integer &y) const
{
	return mpz_cmp(m_g,y.m_g);
}

GW_INLINE void Integer::m_add(const Integer &y)
{
	mpz_add(m_g,m_g,y.m_g);
}

GW_INLINE void Integer::m_add(const int32 n)
{
	if(n>=0)	mpz_add_ui(m_g,m_g,n);
	else		mpz_sub_ui(m_g,m_g,-n);
}

GW_INLINE void Integer::m_sub(const Integer &y)
{
	mpz_sub(m_g,m_g,y.m_g);
}

GW_INLINE void Integer::m_sub(const int32 n)
{
	if(n>=0)	mpz_sub_ui(m_g,m_g,n);
	else		mpz_add_ui(m_g,m_g,-n);
}

GW_INLINE void Integer::m_mul(const Integer &y)
{
	mpz_mul(m_g,m_g,y.m_g);
}

GW_INLINE void Integer::m_div(const Integer &y,Integer &q) const
{
	mpz_tdiv_q(q.m_g,m_g,y.m_g);
}

GW_INLINE void Integer::m_div(const Integer &y,Integer &q,Integer &r) const
{
	mpz_tdiv_qr(q.m_g,r.m_g,m_g,y.m_g);
}

#ifdef _64BIT
GW_INLINE void Integer::m_mod(const uint64 n1, uint64 *p1) const
{
   // On Win64, mpz_init_set_ui takes a unsigned long int, which is 32 bits, not 64 bits as it is
   // on MacIntel and Linux.  This will work on all platforms with a minimal sacrifice of speed.
   uint32 n32 = uint32(n1>>32);

	mpz_set_ui(*(mpz_t*)(&scrap), n32);
	mpz_mul_2exp(*(mpz_t*)(&scrap), scrap, 32);
	mpz_add_ui(*(mpz_t*)(&scrap), scrap, (uint32)(n1&0xFFFFFFFF));
   mpz_mod(*(mpz_t*)(&scrap), m_g, scrap);
   *p1 = mpz_get_ui(scrap);
}
#else
GW_INLINE void Integer::m_mod(const uint64 n1, uint64 *p1) const
{
    uint32 const*pbdata=(uint32*)m_a;

    int blen=m_len;
    const i52 prime1=n1;
    const double prf1=1.0/prime1;
    const i52 scale=((uint64)1)<<48;
    if(blen<=0) { *p1=0; return; }
    i52 run1=0, datum;
    switch(blen%3)
    {
       case 0: break;
       case 1: run1=pbdata[--blen]; if(run1>prime1) run1%=prime1; break;
       case 2: run1=pbdata[blen-1]>>16; blen-=2; goto trailinghalf;
    }
    while((blen-=3) >= 0)
    {
       datum=((i52)pbdata[blen+2]<<16)+(pbdata[blen+1]>>16);
       run1=muladdmod52_pr(run1, scale, datum, prime1, prf1);
       trailinghalf:
       datum=((i52)(pbdata[blen+1]&0xffff)<<32)+(pbdata[blen]);
       run1=muladdmod52_pr(run1, scale, datum, prime1, prf1);
    } 
    if(run1>=prime1) run1-=prime1;
    *p1=run1;
}
#endif

#if defined(_64BIT)
GW_INLINE void Integer::m_mod2(const int32 n1,const int32 n2,int32 *p1,int32 *p2) const
{
   *p1 = (int32) mpz_mod_ui(*(mpz_t*)(&scrap),m_g,n1);
	*p2 = (int32) mpz_mod_ui(*(mpz_t*)(&scrap),m_g,n2);
}

GW_INLINE void Integer::m_mod2(const uint64 n1,const uint64 n2,uint64 *p1,uint64 *p2) const
{
#if defined(_MSC_VER)
   // On Win64, mpz_mod_ui takes a unsigned long int, which is 32 bits, not 64 bits as it is
   // on MacIntel and Linux.  Call m_mod to properly handle 64-bit mods.
   m_mod(n1, p1);
   m_mod(n2, p2);
#else
   *p1 = mpz_mod_ui(*(mpz_t*)(&scrap),m_g,n1);
	*p2 = mpz_mod_ui(*(mpz_t*)(&scrap),m_g,n2);
#endif
}
#else
GW_INLINE void Integer::m_mod2(const int32 n1,const int32 n2,int32 *p1,int32 *p2) const
{
	DEBUG_ADJUST_FP_STACK(__FILE__, __LINE__);
	Imod2((uint32*)m_a,n1,n2,m_len,p1,p2);
}

GW_INLINE void Integer::m_mod2(const uint64 n1,const uint64 n2,uint64 *p1,uint64 *p2) const
{
    uint32 const*pbdata=(uint32*)m_a;
    int blen=m_len;
    const i52 prime1=n1, prime2=n2;
    const double prf1=1.0/prime1, prf2=1.0/prime2;
    const i52 scale=((uint64)1)<<48;
    if(blen<=0) { *p1=*p2=0; return; }
    i52 run1=0, run2=0, datum;
    switch(blen%3)
    {
    case 0: break;
    case 1: 
	run1=run2=pbdata[--blen]; 
	if(run1>prime1) run1%=prime1; 
	if(run2>prime2) run2%=prime2; 
	break;
    case 2: run1=run2=pbdata[blen-1]>>16; blen-=2; goto trailinghalf;
    }
    while((blen-=3) >= 0)
    {
	datum=((i52)pbdata[blen+2]<<16)+(pbdata[blen+1]>>16);
	run1=muladdmod52_pr(run1, scale, datum, prime1, prf1);
	run2=muladdmod52_pr(run2, scale, datum, prime2, prf2);
    trailinghalf:
	datum=((i52)(pbdata[blen+1]&0xffff)<<32)+(pbdata[blen]);
	run1=muladdmod52_pr(run1, scale, datum, prime1, prf1);
	run2=muladdmod52_pr(run2, scale, datum, prime2, prf2);
    } 
    if(run1>=prime1) run1-=prime1;
    if(run2>=prime2) run2-=prime2;
    *p1=run1;
    *p2=run2;
}
#endif

GW_INLINE int32 Integer::m_andu(const int32 & n) const
{
	int32 l = mpz_get_si(m_g);
#if defined (TEST_ME)
	Integer x(n);
	mpz_and(x.m_g,x.m_g,m_g);
	if (mpz_get_si(x.m_g) != (l&n))
		printf ("Damn, this did not work like I thought!!!\n");
#endif
	return l & n;
}

GW_INLINE uint32 Integer::m_andu(const uint32 & n) const
{
	uint32 l = (uint32) mpz_get_ui(m_g);
	return l & n;
}

GW_INLINE uint64 Integer::m_andu(const uint64 & n) const
{
	// This is a quick hack.  It works "fine" for a little endian.
	uint64 l;
	if (m_len < 2)
		l = mpz_get_ui(m_g);
	else
		l = *(uint64*)(m_a);
	return l & n;
}

GW_INLINE void Integer::m_andu(const Integer & y)
{
	// function not used.
	mpz_and(m_g,m_g,y.m_g);
}

GW_INLINE int32 Integer::m_oru(const int32 n) const
{
	// function not used
	Integer x(n);
	mpz_ior(x.m_g,x.m_g,m_g);
	return mpz_get_si(x.m_g);
}

GW_INLINE void Integer::m_oru(const Integer & y)
{
	// function not used.
	mpz_ior(m_g,m_g,y.m_g);
}

GW_INLINE Integer pow(const Integer &x,const int32 n)
{
	Integer y;
	mpz_pow_ui(y.m_g,x.m_g,n);
	return y;
}

GW_INLINE Integer shl(const Integer &x,const int32 n)
{
	Integer y;
	mpz_mul_2exp(y.m_g,x.m_g,n);
	return y;
}

GW_INLINE void Integer::m_shl(const int32 n)
{
	mpz_mul_2exp(m_g,m_g,n);
}

GW_INLINE void Integer::Ipow(const int32 xx,const int32 n)
{
	// function not used.
	Integer x(xx);
	mpz_pow_ui(m_g,x.m_g,n);
}

GW_INLINE Integer powm(const Integer &x,const int32 n,const Integer &m)
{
	Integer y;
	mpz_powm_ui(y.m_g,x.m_g,n,m.m_g);
	return y;
}

GW_INLINE Integer powm(const Integer &x,const Integer &n,const Integer &m)
{
	Integer y;
	mpz_powm(y.m_g,x.m_g,n.m_g,m.m_g);
	return y;
}


GW_INLINE Integer gcd(const Integer &x,const Integer &y)
{
	Integer z;
	mpz_gcd(z.m_g,x.m_g,y.m_g);
	return z;
}

GW_INLINE void Integer::m_neg()
{	
	mpz_neg(m_g,m_g);
}

GW_INLINE Integer squareroot(const Integer &x)
{
	Integer r;
	mpz_sqrt(r.m_g,x.m_g);
	return r;
}

GW_INLINE const mpz_ptr Integer::gmp() const
{
	return (const mpz_ptr) &m_g;
}

GW_INLINE mpz_ptr Integer::gmp()
{
	return (mpz_ptr) &m_g;
}

GW_INLINE int32 kro(const Integer &X,const Integer &Y)
{
	int i=mpz_jacobi(X.m_g,Y.m_g);
	return i;
}

GW_INLINE Integer & Integer::operator = (const int32 n)
{
	mpz_set_si(m_g,n);
	return *this;
}

GW_INLINE Integer & Integer::operator = (const uint32 n)
{
	mpz_set_ui(m_g,n);
	return *this;
}

GW_INLINE Integer & Integer::operator = (const uint64 n64)
{
	uint32 n32 = uint32(n64>>32);
	if (!n32)
		// Simple case, n64 is only 32 bits.
		mpz_set_ui(m_g,(uint32)(n64&0xFFFFFFFF));
	else
	{
		mpz_set_ui(m_g,n32);
		mpz_mul_2exp(m_g, m_g, 32);
		mpz_add_ui(m_g,m_g,(uint32)(n64&0xFFFFFFFF));
	}
	return *this;
}

GW_INLINE Integer & Integer::operator = (const Integer & x)
{
	m_set(x);
	return *this;
}


GW_INLINE Integer operator - (const Integer & x)
{
	Integer z(x);
	z.m_neg();
	return z;
}


GW_INLINE int32 operator == (const Integer & x, int32 n)
{
	return (x.m_cmp(n) == 0);
}

GW_INLINE int32 operator == (const Integer & x, const Integer & y)
{
	return (x.m_cmp(y) == 0);
}

GW_INLINE int32 operator != (const Integer & x, int32 n)
{
	return (x.m_cmp(n) != 0);
}

GW_INLINE int32 operator != (const Integer & x, uint64 n)
{
	return (x.m_cmp(n) != 0);
}

GW_INLINE int32 operator != (const Integer & x, const Integer & y)
{
	return (x.m_cmp(y) != 0);
}

GW_INLINE int32 operator < (const Integer & x, int32 n)
{
	return (x.m_cmp(n) < 0);
}

GW_INLINE int32 operator < (const Integer & x, const Integer & y)
{
	return (x.m_cmp(y) < 0);
}

GW_INLINE int32 operator <= (const Integer & x, int32 n)
{
	return (x.m_cmp(n) <= 0);
}

GW_INLINE int32 operator <= (const Integer & x, const Integer & y)
{
	return (x.m_cmp(y) <= 0);
}

GW_INLINE int32 operator > (const Integer & x, int32 n)
{
	return (x.m_cmp(n) > 0);
}

GW_INLINE int32 operator > (const Integer & x, const Integer & y)
{
	return (x.m_cmp(y) > 0);
}

GW_INLINE int32 operator >= (const Integer & x, int32 n)
{
	return (x.m_cmp(n) >= 0);
}

GW_INLINE int32 operator >= (const Integer & x, const Integer & y)
{
	// function not used.
	return (x.m_cmp(y) >= 0);
}


GW_INLINE Integer & Integer::operator ++ ()
{
	m_add(1);
	return *this;
}

GW_INLINE Integer Integer::operator ++ (int32)
{
	Integer temp = *this;
	m_add(1);
	return temp;
}

GW_INLINE Integer & Integer::operator -- ()
{
	m_sub(1);
	return *this;
}

GW_INLINE Integer Integer::operator -- (int32)
{
	Integer temp = *this;
	m_sub(1);
	return temp;
}

GW_INLINE Integer & Integer::operator += (const int32 n)
{
	m_add(n);
	return *this;
}

GW_INLINE Integer & Integer::operator += (const Integer & y)
{
	m_add(y);
	return *this;
}

GW_INLINE Integer & Integer::operator += (const uint32 n)
{
	mpz_add_ui(m_g,m_g,n);
	return *this;
}

GW_INLINE Integer & Integer::operator += (const uint64 n)
{
	if (n < (1u<<31))
		m_add((int32)n);
	else
	{
		Integer y(n);
		m_add(y);
	}
	return *this;
}

GW_INLINE Integer & Integer::operator -= (const int32 n)
{
	m_sub(n);
	return *this;
}

GW_INLINE Integer & Integer::operator -= (const Integer & y)
{
	m_sub(y);
	return *this;
}

GW_INLINE Integer & Integer::operator *= (const int32 n)
{
	m_mul(n);
	return *this;
}

GW_INLINE Integer & Integer::operator *= (const uint32 n)
{
	mpz_mul_ui(m_g, m_g, n);
	return *this;
}

GW_INLINE Integer & Integer::operator *= (const uint64 n)
{
// Formula used:
// t   = m_g
// m_g =  m_g*(n>>32)*(2^32)
// m_g += t*(n%(2^32))

	uint32 u = uint32(n>>32);
	if (!u)
	{
		// number is "less" than 2^32, so simply use a single mul command)
		u = uint32(n&0xFFFFFFFF);
		mpz_mul_ui(m_g, m_g, u);
	}
	else
	{
		// t   = m_g
		mpz_t t;
		mpz_init_set(t,m_g);

		// m_g =  m_g*(n>>32)*(2^32)
		mpz_mul_ui(m_g, m_g, u);
		mpz_mul_2exp(m_g, m_g, 32);

		// m_g += t*(n%(2^32))
		u = uint32(n&0xFFFFFFFF);
		mpz_addmul_ui(m_g, t, u);

		// don't leak
		mpz_clear(t);
	}
	return *this;
}

GW_INLINE Integer & Integer::operator *= (const Integer & y)
{
	m_mul(y);
	return *this;
}

GW_INLINE Integer & Integer::operator /= (const int32 n)
{
	m_div(n);
	return *this;
}

GW_INLINE Integer & Integer::operator /= (const Integer & y)
{
	m_div(y,*this);
	return *this;
}

GW_INLINE Integer & Integer::operator %= (const int32 n)
{
	long r = m_div(n);
	m_set(r);
	return *this;
}

GW_INLINE Integer & Integer::operator %= (const Integer & y)
{
	Integer z;
	m_div(y,z,*this);
	return *this;
}

GW_INLINE Integer & Integer::operator <<= (const int32 n)
{
	m_sftr(-n);
	return *this;
}

GW_INLINE Integer & Integer::operator >>= (const int32 n)
{
	m_sftr(n);
	return *this;
}

GW_INLINE Integer & Integer::operator &= (const int32 n)
{
	// function not used.
	m_andu(Integer(n));
	return *this;
}

GW_INLINE Integer & Integer::operator &= (const uint64 n)
{
	// function not used.
	m_andu(Integer(n));
	return *this;
}

GW_INLINE Integer & Integer::operator &= (const Integer & y)
{
	// function not used.
	m_andu(y);
	return *this;
}

GW_INLINE Integer & Integer::operator |= (const int32 n)
{
	// function not used.
	m_oru(Integer(n));
	return *this;
}

GW_INLINE Integer & Integer::operator |= (const Integer & y)
{
	// function not used.
	m_oru(y);
	return *this;
}

GW_INLINE Integer operator + (const Integer & x, const int32 n)
{
	Integer z(x);
	z.m_add(n);
	return z;
}

GW_INLINE Integer operator + (const Integer & x, const Integer & y)
{
	Integer z(x);
	z.m_add(y);
	return z;
}

GW_INLINE Integer operator - (const Integer & x, const int32 n)
{
	Integer z(x);
	z.m_sub(n);
	return z;
}

GW_INLINE Integer operator - (const Integer & x, const Integer & y)
{
	Integer z(x);
	z.m_sub(y);
	return z;
}

GW_INLINE Integer operator * (const Integer & x, const int32 n)
{
	Integer z(x);
	z.m_mul(n);
	return z;
}

GW_INLINE Integer operator * (const Integer & x, const Integer & y)
{
	Integer z(x);
	z.m_mul(y);
	return z;
}

GW_INLINE Integer operator / (const Integer & x, const int32 n)
{
	// function not used.
	Integer z(x);
	z.m_div(n);
	return z;
}

GW_INLINE Integer operator / (const Integer & x, const Integer & y)
{
	Integer z;
	x.m_div(y,z);
	return z;
}

GW_INLINE int32 operator % (const Integer & x, const int32 n)
{
	long r = x.m_mod(n);
	return r;
}

GW_INLINE Integer operator % (const Integer & x, const Integer & y)
{
	Integer z;
	Integer r;
	x.m_div(y,z,r);
	return r;
}

GW_INLINE void Integer::divmod(const Integer &d,Integer &q,Integer &r) const
{
	m_div(d,q,r);
}

GW_INLINE Integer operator << (const Integer & x, const int32 n)
{
	// function not used.
	Integer z(x);
	z.m_sftr(-n);
	return z;
}

GW_INLINE Integer operator >> (const Integer & x, const int32 n)
{
	// function not used.
	Integer z(x);
	z.m_sftr(n);
	return z;
}

GW_INLINE int32 operator & (const Integer & x, const int32 n)
{
	int32 r = x.m_andu(n);
	return r;
}

GW_INLINE uint32 operator & (const Integer & x, const uint32 n)
{
	uint32 r = x.m_andu(n);
	return r;
}

GW_INLINE uint64  operator &  (const Integer & x, const uint64 n)
{
	uint64 r = x.m_andu(n);
	return r;
}

GW_INLINE Integer operator & (const Integer & x, const Integer & y)
{
	// function not used.
	Integer z(x);
	z.m_andu(y);
	return z;
}

GW_INLINE int32 operator | (const Integer & x, const int32 n)
{
	// function not used.
	int32 r = x.m_oru(n);
	return r;
}

GW_INLINE Integer operator | (const Integer & x, const Integer & y)
{
	// function not used.
	Integer z(x);
	z.m_oru(y);
	return z;
}

GW_INLINE void Integer::atoI(const char * s, const int32 base)
{
	mpz_set_str (m_g, s, base);
}

GW_INLINE char* Integer::Itoa(const int32 base) const
{
	// Since gmp can now be in a DLL, and we may not have the ownership of the malloc, we need to ONLY
	// pass in allocated strings and not NULL.  Then we KNOW to use delete[] to free up the returned
	// string, and it will work without ever causing a GP (since we are the process which allocated
	// it, and is has known allocation.  (i.e. This fixes a possible crash when using the DLL's)
	char *cp = new char[mpz_sizeinbase(m_g, base) + 2];
	return mpz_get_str (cp, base, m_g);
}

GW_INLINE int32 numbits(const Integer &x)
{
	return (int32) mpz_sizeinbase(x.m_g,2)-1;
}

GW_INLINE int32 numbits(const uint64 &x)
{
	Integer t(x);
	return (int32) numbits(t);
}
