#ifndef _INTEGER_H
#define _INTEGER_H

void memAlloc();
void memFree();

#define	m_a	m_g->_mp_d
#define	m_len	m_g->_mp_size

#ifndef _64BIT

extern "C" int32_t Imod(uint32_t * const a,const int32_t n,const int32_t len);
extern "C" int32_t Imod2(uint32_t * const a,const int32_t n1,const int32_t n2,const int32_t len, int32_t *p1,int32_t *p2);

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

	void        m_setu(const int32_t n);
	void        m_setu(const Integer & y);
	int32_t       m_cmpu(const int32_t n) const;
	int32_t       m_cmpu(const Integer & y) const;
	void        m_addu(const int32_t n);
	void        m_addu(const Integer & y);
	void        m_subu(const int32_t n);
	void        m_subu(const Integer & y);
	void        m_mulu(const int32_t n);
	void        m_mulu(const Integer & y);
	int32_t       m_divu(const int32_t n);
	Integer     m_divu(const Integer & y);
	void        m_sftru(const int32_t n);
	GW_INLINE	int32_t       m_andu(const int32_t & n) const;
	GW_INLINE	uint32_t      m_andu(const uint32_t & n) const;
	GW_INLINE	uint64_t		m_andu(const uint64_t & n) const;
	GW_INLINE	void        m_andu(const Integer & y);
	GW_INLINE	int32_t       m_oru(const int32_t n) const;
	GW_INLINE	void        m_oru(const Integer & y);

	void		   m_clr();
	GW_INLINE	void        m_set(const int32_t n);
	GW_INLINE	void        m_set(const Integer & y);
	GW_INLINE	int32_t       m_cmp(const int32_t n) const;
	GW_INLINE	int32_t       m_cmp(const uint64_t n) const;
	GW_INLINE	int32_t       m_cmp(const Integer & y) const;
	GW_INLINE	void        m_add(const int32_t n);
	GW_INLINE	void        m_add(const Integer & y);
	GW_INLINE	void        m_sub(const int32_t n);
	GW_INLINE	void        m_sub(const Integer & y);
	void        m_mul(const int32_t n);
	GW_INLINE	void        m_mul(const Integer & y);


	long        m_div(const int32_t n);
	GW_INLINE	void	m_div(const Integer & y,Integer &q) const;
	GW_INLINE	void	m_div(const Integer & y,Integer &q,Integer &r) const;
	void        m_sftr(const int32_t n);
	void        m_neg();
	void        m_abs();
	GW_INLINE	void	m_shl(const int32_t n);

   int   mpz_sprp(mpz_t n, mpz_t a);
   int   mpz_strongselfridge_prp(mpz_t n);
   int   mpz_stronglucas_prp(mpz_t n, long int p, long int q);

public:
	Integer();
	Integer(int32_t i);
	Integer(uint32_t);
	Integer(uint64_t);
	Integer(const Integer &);
	
	virtual ~Integer();

	Integer & operator = (const int32_t);
	Integer & operator = (const uint32_t);
	Integer & operator = (const uint64_t);
	Integer & operator = (const Integer & y);
	Integer & operator = (const GWInteger & y);

	friend Integer operator - (const Integer & x);

	friend int32_t operator == (const Integer & x, int32_t n);
	friend int32_t operator == (const Integer & x, const Integer & y);
	friend int32_t operator != (const Integer & x, int32_t n);
	friend int32_t operator != (const Integer & x, uint64_t n);
	friend int32_t operator != (const Integer & x, const Integer & y);
	friend int32_t operator <  (const Integer & x, int32_t n);
	friend int32_t operator <  (const Integer & x, const Integer & y);
	friend int32_t operator <= (const Integer & x, int32_t n);
	friend int32_t operator <= (const Integer & x, const Integer & y);
	friend int32_t operator >  (const Integer & x, int32_t n);
	friend int32_t operator >  (const Integer & x, const Integer & y);
	friend int32_t operator >= (const Integer & x, int32_t n);
	friend int32_t operator >= (const Integer & x, const Integer & y);

	GW_INLINE Integer & operator ++ ();    // prefix
	GW_INLINE Integer   operator ++ (int32_t); // postfix
	GW_INLINE Integer & operator -- ();    // prefix
	GW_INLINE Integer   operator -- (int32_t); // postfix
	GW_INLINE Integer & operator += (const int32_t n);
	GW_INLINE Integer & operator += (const uint32_t n);
	GW_INLINE Integer & operator += (const uint64_t n);
	GW_INLINE Integer & operator += (const Integer & y);
	GW_INLINE Integer & operator -= (const int32_t n);
	GW_INLINE Integer & operator -= (const Integer & y);
	GW_INLINE Integer & operator *= (const int32_t n);
	GW_INLINE Integer & operator *= (const uint32_t n);
	GW_INLINE Integer & operator *= (const uint64_t n);
	GW_INLINE Integer & operator *= (const Integer & y);
	GW_INLINE Integer & operator /= (const int32_t n);
	GW_INLINE Integer & operator /= (const Integer & y);
	GW_INLINE Integer & operator %= (const int32_t n);
	GW_INLINE Integer & operator %= (const Integer & y);
	GW_INLINE Integer & operator <<= (const int32_t n);
	GW_INLINE Integer & operator >>= (const int32_t n);
	GW_INLINE Integer & operator &= (const int32_t n);
	GW_INLINE Integer & operator &= (const uint64_t n);
	GW_INLINE Integer & operator &= (const Integer & y);
	GW_INLINE Integer & operator |= (const int32_t n);
	GW_INLINE Integer & operator |= (const Integer & y);

	friend Integer operator +  (const Integer & x, const int32_t n);
	friend Integer operator +  (const Integer & x, const Integer & y);
	friend Integer operator -  (const Integer & x, const int32_t n);
	friend Integer operator -  (const Integer & x, const Integer & y);
	friend Integer operator *  (const Integer & x, const int32_t n);
	friend Integer operator *  (const Integer & x, const Integer & y);
	friend Integer operator /  (const Integer & x, const int32_t n);
	friend Integer operator /  (const Integer & x, const Integer & y);
	friend int32_t   operator %  (const Integer & x, const int32_t n);
	friend Integer operator %  (const Integer & x, const Integer & y);
	friend Integer operator << (const Integer & x, const int32_t n);
	friend Integer operator >> (const Integer & x, const int32_t n);
	friend int32_t   operator &  (const Integer & x, const int32_t n);
	friend uint32_t  operator &  (const Integer & x, const uint32_t n);
	friend uint64_t  operator &  (const Integer & x, const uint64_t n);
	friend Integer operator &  (const Integer & x, const Integer & y);
	friend int32_t   operator |  (const Integer & x, const int32_t n);
	friend Integer operator |  (const Integer & x, const Integer & y);
	

	void  divmod(const Integer &d,Integer &q,Integer &r) const;
	int32_t	m_mod(const int32_t n) const;
	void  m_mod(const uint64_t n, uint64_t*p) const;
	void	m_mod2(const int32_t n1,const int32_t n2,int32_t *p1,int32_t *p2) const;
	void	m_mod2(const uint64_t n1,const uint64_t n2,uint64_t *p1,uint64_t *p2) const;

	char * Itoa(const int32_t base = 10) const;	// the caller MUST use operator delete[] to free up the returned string.
	void atoI(const char * s, const int32_t base = 10);
	void Ipow(const int32_t x, const int32_t n); // x to the n as Integer 
   Integer nextprime(void);
   Integer prevprime(void);

	friend Integer abs(const Integer & x);
	friend void    swap(Integer & a, Integer & b);
	friend int32_t   numbits(const Integer & x);
	friend int32_t   numbits(const uint64_t & x);
	friend int32_t   bit(const Integer & x, const uint32_t n); // n should be <= lg(x)

	friend uint32_t	crc(const Integer & x);
	friend uint32_t	crc32(const Integer &x,uint32_t crc);

	friend Integer pow(const Integer & x, const int32_t n);
	friend Integer shl(const Integer & x, const int32_t n);	// returns x << n
	friend Integer squareroot(const Integer & x); // floor of square root
	friend Integer gcd(const Integer & x, const Integer & y);
	friend int32_t   kro(const Integer & x, const Integer & y); // Kronecker symbol
	friend uint32_t  crc32(const Integer &x);

	friend Integer powm(const Integer & x, const int32_t n, const Integer & N);  // modular power
	friend Integer powm(const Integer & x, const Integer & y, const Integer & N);  // modular power
	friend Integer modinv(const Integer & x, const Integer & N); // modular inversion

	const mpz_ptr gmp() const;
	mpz_ptr	gmp();
};

#ifdef GW_INLINE_ENABLED
#include "integer.inl"
#endif

#endif // _INTEGER_H
