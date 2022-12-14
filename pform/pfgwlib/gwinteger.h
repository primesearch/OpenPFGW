#if !defined (GW_INTEGER_H)
#define GW_INTEGER_H

#include "gwcontext.h"
#undef gwadd
#undef gwaddsub

class Integer;

class GWInteger
{
protected:
	gwnum g;

	mpz_t		scrap;

public:
	GWInteger();
	virtual ~GWInteger();
	
	GWInteger(const GWInteger &);
	GWInteger &operator=(const GWInteger &);
	GWInteger &operator=(const Integer &i);
	GWInteger &operator=(int i);

	double suminp();
	double sumout();
	double sumdiff();

	friend class Integer;
	friend void inl_gwcopy(const GWInteger &s,GWInteger &d);
	friend void inl_gwfft(const GWInteger &s,GWInteger &d);
	friend void inl_gwsquare2(GWInteger &s);
   friend void inl_gwsquare2_carefully(GWInteger &s);
	friend void inl_gwmul(GWInteger &s, GWInteger &d);
	friend void inl_gwmul_carefully(GWInteger &s, GWInteger &d);
	friend void inl_gwfftmul(GWInteger &s,GWInteger &d);
	friend void inl_gwfftfftmul(const GWInteger &s,const GWInteger &s2,GWInteger &d);
	friend void inl_gwadd(const GWInteger &s,GWInteger &d);
	friend void inl_gwadd3(const GWInteger &s1,const GWInteger &s2,GWInteger &d);
	friend void inl_gwsub3(const GWInteger &s1,const GWInteger &s2,GWInteger &d);
	friend void inl_gwaddsub(GWInteger &a,GWInteger &b);
	friend void inl_gwaddsub4(const GWInteger &s1,const GWInteger &s2,GWInteger &d1,GWInteger &d2);
	friend void inl_gwsmallmul(double m,GWInteger &s);
};
	
/*	Include inline functions	*/
#ifdef GW_INLINE_ENABLED
#include "../../pform/pfgwlib/gwinteger.inl"
#endif

#endif
