#ifndef _INTEGER_H
#define _INTEGER_H

void memAlloc();
void memFree();

#define	m_a	m_g->_mp_d
#define	m_len	m_g->_mp_size

#ifndef _64BIT

extern "C" int32 Imod(uint32 * const a,const int32 n,const int32 len);
extern "C" int32 Imod2(uint32 * const a,const int32 n1,const int32 n2,const int32 len, int32 *p1,int32 *p2);

#endif

class GWInteger;

class Integer
{

protected:
	mpz_t		m_g;
	mpz_t		scrap;

	// Access to some of these functions would be nice for optimzation issues.
	friend class GWContext;
	friend class GWInteger;

	void        m_setu(const int32 n);
	void        m_setu(const Integer & y);
	int32       m_cmpu(const int32 n) const;
	int32       m_cmpu(const Integer & y) const;
	void        m_addu(const int32 n);
	void        m_addu(const Integer & y);
	void        m_subu(const int32 n);
	void        m_subu(const Integer & y);
	void        m_mulu(const int32 n);
	void        m_mulu(const Integer & y);
	int32       m_divu(const int32 n);
	Integer     m_divu(const Integer & y);
	void        m_sftru(const int32 n);
	GW_INLINE	int32       m_andu(const int32 & n) const;
	GW_INLINE	uint32      m_andu(const uint32 & n) const;
	GW_INLINE	uint64		m_andu(const uint64 & n) const;
	GW_INLINE	void        m_andu(const Integer & y);
	GW_INLINE	int32       m_oru(const int32 n) const;
	GW_INLINE	void        m_oru(const Integer & y);

	void		   m_clr();
	GW_INLINE	void        m_set(const int32 n);
	GW_INLINE	void        m_set(const Integer & y);
	GW_INLINE	int32       m_cmp(const int32 n) const;
	GW_INLINE	int32       m_cmp(const uint64 n) const;
	GW_INLINE	int32       m_cmp(const Integer & y) const;
	GW_INLINE	void        m_add(const int32 n);
	GW_INLINE	void        m_add(const Integer & y);
	GW_INLINE	void        m_sub(const int32 n);
	GW_INLINE	void        m_sub(const Integer & y);
	void        m_mul(const int32 n);
	GW_INLINE	void        m_mul(const Integer & y);


	long        m_div(const int32 n);
	GW_INLINE	void	m_div(const Integer & y,Integer &q) const;
	GW_INLINE	void	m_div(const Integer & y,Integer &q,Integer &r) const;
	void        m_sftr(const int32 n);
	void        m_neg();
	void        m_abs();
	GW_INLINE	void	m_shl(const int32 n);

   int   mpz_sprp(mpz_t n, mpz_t a);
   int   mpz_strongselfridge_prp(mpz_t n);
   int   mpz_stronglucas_prp(mpz_t n, long int p, long int q);

public:
	Integer();
	Integer(int32 i);
	Integer(uint32);
	Integer(uint64);
	Integer(const Integer &);
	
	virtual ~Integer();

	Integer & operator = (const int32);
	Integer & operator = (const uint32);
	Integer & operator = (const uint64);
	Integer & operator = (const Integer & y);
	Integer & operator = (const GWInteger & y);

	friend Integer operator - (const Integer & x);

	friend int32 operator == (const Integer & x, int32 n);
	friend int32 operator == (const Integer & x, const Integer & y);
	friend int32 operator != (const Integer & x, int32 n);
	friend int32 operator != (const Integer & x, uint64 n);
	friend int32 operator != (const Integer & x, const Integer & y);
	friend int32 operator <  (const Integer & x, int32 n);
	friend int32 operator <  (const Integer & x, const Integer & y);
	friend int32 operator <= (const Integer & x, int32 n);
	friend int32 operator <= (const Integer & x, const Integer & y);
	friend int32 operator >  (const Integer & x, int32 n);
	friend int32 operator >  (const Integer & x, const Integer & y);
	friend int32 operator >= (const Integer & x, int32 n);
	friend int32 operator >= (const Integer & x, const Integer & y);

	GW_INLINE Integer & operator ++ ();    // prefix
	GW_INLINE Integer   operator ++ (int32); // postfix
	GW_INLINE Integer & operator -- ();    // prefix
	GW_INLINE Integer   operator -- (int32); // postfix
	GW_INLINE Integer & operator += (const int32 n);
	GW_INLINE Integer & operator += (const uint32 n);
	GW_INLINE Integer & operator += (const uint64 n);
	GW_INLINE Integer & operator += (const Integer & y);
	GW_INLINE Integer & operator -= (const int32 n);
	GW_INLINE Integer & operator -= (const Integer & y);
	GW_INLINE Integer & operator *= (const int32 n);
	GW_INLINE Integer & operator *= (const uint32 n);
	GW_INLINE Integer & operator *= (const uint64 n);
	GW_INLINE Integer & operator *= (const Integer & y);
	GW_INLINE Integer & operator /= (const int32 n);
	GW_INLINE Integer & operator /= (const Integer & y);
	GW_INLINE Integer & operator %= (const int32 n);
	GW_INLINE Integer & operator %= (const Integer & y);
	GW_INLINE Integer & operator <<= (const int32 n);
	GW_INLINE Integer & operator >>= (const int32 n);
	GW_INLINE Integer & operator &= (const int32 n);
	GW_INLINE Integer & operator &= (const uint64 n);
	GW_INLINE Integer & operator &= (const Integer & y);
	GW_INLINE Integer & operator |= (const int32 n);
	GW_INLINE Integer & operator |= (const Integer & y);

	friend Integer operator +  (const Integer & x, const int32 n);
	friend Integer operator +  (const Integer & x, const Integer & y);
	friend Integer operator -  (const Integer & x, const int32 n);
	friend Integer operator -  (const Integer & x, const Integer & y);
	friend Integer operator *  (const Integer & x, const int32 n);
	friend Integer operator *  (const Integer & x, const Integer & y);
	friend Integer operator /  (const Integer & x, const int32 n);
	friend Integer operator /  (const Integer & x, const Integer & y);
	friend int32   operator %  (const Integer & x, const int32 n);
	friend Integer operator %  (const Integer & x, const Integer & y);
	friend Integer operator << (const Integer & x, const int32 n);
	friend Integer operator >> (const Integer & x, const int32 n);
	friend int32   operator &  (const Integer & x, const int32 n);
	friend uint32  operator &  (const Integer & x, const uint32 n);
	friend uint64  operator &  (const Integer & x, const uint64 n);
	friend Integer operator &  (const Integer & x, const Integer & y);
	friend int32   operator |  (const Integer & x, const int32 n);
	friend Integer operator |  (const Integer & x, const Integer & y);
	

	void  divmod(const Integer &d,Integer &q,Integer &r) const;
	int32	m_mod(const int32 n) const;
	void  m_mod(const uint64 n, uint64*p) const;
	void	m_mod2(const int32 n1,const int32 n2,int32 *p1,int32 *p2) const;
	void	m_mod2(const uint64 n1,const uint64 n2,uint64 *p1,uint64 *p2) const;

	char * Itoa(const int32 base = 10) const;	// the caller MUST use operator delete[] to free up the returned string.
	void atoI(const char * s, const int32 base = 10);
	void Ipow(const int32 x, const int32 n); // x to the n as Integer 
   Integer nextprime(void);
   Integer prevprime(void);

	friend Integer abs(const Integer & x);
	friend void    swap(Integer & a, Integer & b);
	friend int32   numbits(const Integer & x);
	friend int32   numbits(const uint64 & x);
	friend int32   bit(const Integer & x, const uint32 n); // n should be <= lg(x)

	friend uint32	crc(const Integer & x);
	friend uint32	crc32(const Integer &x,uint32 crc);

	friend Integer pow(const Integer & x, const int32 n);
	friend Integer shl(const Integer & x, const int32 n);	// returns x << n
	friend Integer squareroot(const Integer & x); // floor of square root
	friend Integer gcd(const Integer & x, const Integer & y);
	friend int32   kro(const Integer & x, const Integer & y); // Kronecker symbol
	friend uint32  crc32(const Integer &x);

	friend Integer powm(const Integer & x, const int32 n, const Integer & N);  // modular power
	friend Integer powm(const Integer & x, const Integer & y, const Integer & N);  // modular power
	friend Integer modinv(const Integer & x, const Integer & N); // modular inversion

	const mpz_ptr gmp() const;
	mpz_ptr	gmp();
};

#ifdef GW_INLINE_ENABLED
#include "integer.inl"
#endif

#endif // _INTEGER_H
