
GW_INLINE GWInteger &GWInteger::operator=(int i)
{
   dbltogw(&gwdata,(double)i,g);
   return *this;
}

GW_INLINE void gwcopy(const GWInteger &s,GWInteger &d)
{
   gwcopy (&gwdata, s.g, d.g);
}

GW_INLINE void gwfft(const GWInteger &s,GWInteger &d)
{
   gwfft (&gwdata, s.g, d.g);
}

GW_INLINE void gwsquare(GWInteger &s)
{
   gwsquare (&gwdata, s.g);
}

GW_INLINE void gwsquare_carefully(GWInteger &s)
{
   gwsquare_carefully (&gwdata, s.g);
}

GW_INLINE void gwfftmul(GWInteger &s,GWInteger &d)
{
   gwfftmul (&gwdata, s.g, d.g);
}

GW_INLINE void gwfftfftmul(const GWInteger &s,const GWInteger &s2,GWInteger &d)
{
   gwfftfftmul (&gwdata, s.g, s2.g, d.g);
}

GW_INLINE void gwadd(const GWInteger &s,GWInteger &d)
{
   gwadd3 (&gwdata, s.g, d.g, d.g);
}

GW_INLINE void gwadd3(const GWInteger &s1,const GWInteger &s2,GWInteger &d)
{
   gwadd3 (&gwdata, s1.g, s2.g, d.g);
}

GW_INLINE void gwsub3(const GWInteger &s1,const GWInteger &s2,GWInteger &d)
{
   gwsub3 (&gwdata, s1.g, s2.g, d.g);
}

GW_INLINE void gwaddsub(GWInteger &a,GWInteger &b)
{
   gwaddsub4 (&gwdata, a.g, b.g, a.g, b.g);
}

GW_INLINE void gwaddsub4(const GWInteger &s1,const GWInteger &s2,GWInteger &d1,GWInteger &d2)
{
   gwaddsub4 (&gwdata, s1.g, s2.g, d1.g, d2.g);
}

GW_INLINE void gwsmallmul(double m,GWInteger &s)
{
   gwsmallmul (&gwdata, m, s.g);
}

