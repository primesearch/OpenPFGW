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
	friend void gwcopy(const GWInteger &s,GWInteger &d);
	friend void gwfft(const GWInteger &s,GWInteger &d);
	friend void gwsquare(GWInteger &s);
   friend void gwsquare_carefully(GWInteger &s);
	friend void gwmul(GWInteger &s, GWInteger &d);
	friend void gwmul_carefully(GWInteger &s, GWInteger &d);
	friend void gwsquare(GWInteger &s);
	friend void gwfftmul(GWInteger &s,GWInteger &d);
	friend void gwfftfftmul(const GWInteger &s,const GWInteger &s2,GWInteger &d);
	friend void gwadd(const GWInteger &s,GWInteger &d);
	friend void gwadd3(const GWInteger &s1,const GWInteger &s2,GWInteger &d);
	friend void gwsub3(const GWInteger &s1,const GWInteger &s2,GWInteger &d);
	friend void gwaddsub(GWInteger &a,GWInteger &b);
	friend void gwaddsub4(const GWInteger &s1,const GWInteger &s2,GWInteger &d1,GWInteger &d2);
	friend void gwsmallmul(double m,GWInteger &s);
};
	
/*	Include inline functions	*/
#ifdef GW_INLINE_ENABLED
#include "../../pform/pfgwlib/gwinteger.inl"
#endif

#endif
