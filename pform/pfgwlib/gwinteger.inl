
GW_INLINE GWInteger &GWInteger::operator=(int i)
{
   dbltogw(&gwdata,(double)i,g);
   return *this;
}

GW_INLINE void inl_gwcopy(const GWInteger &s,GWInteger &d)
{
   gwcopy (&gwdata, s.g, d.g);
}

GW_INLINE void inl_gwfft(const GWInteger &s,GWInteger &d)
{
   gwfft (&gwdata, s.g, d.g);
}

GW_INLINE void inl_gwsquare2(GWInteger &s)
{
   gwsquare2(&gwdata, s.g, s.g, oldstartnextfft(&gwdata) | oldmulbyconst(&gwdata) | GWMUL_ADDINCONST);
}

GW_INLINE void inl_gwsquare2_carefully(GWInteger &s)
{
   gwsquare2_carefully (&gwdata, s.g, s.g);
}

GW_INLINE void inl_gwmul(GWInteger &s, GWInteger &d)
{
   gwmul (&gwdata, s.g, d.g);
}

GW_INLINE void inl_gwmul_carefully(GWInteger &s, GWInteger &d)
{
   gwmul_carefully (&gwdata, s.g, d.g);
}

GW_INLINE void inl_gwfftmul(GWInteger &s,GWInteger &d)
{
   gwfftmul (&gwdata, s.g, d.g);
}

GW_INLINE void inl_gwfftfftmul(const GWInteger &s,const GWInteger &s2,GWInteger &d)
{
   gwfftfftmul (&gwdata, s.g, s2.g, d.g);
}

GW_INLINE void inl_gwadd(const GWInteger &s,GWInteger &d)
{
   gwadd3 (&gwdata, s.g, d.g, d.g);
}

GW_INLINE void inl_gwadd3(const GWInteger &s1,const GWInteger &s2,GWInteger &d)
{
   gwadd3 (&gwdata, s1.g, s2.g, d.g);
}

GW_INLINE void inl_gwsub3(const GWInteger &s1,const GWInteger &s2,GWInteger &d)
{
   gwsub3 (&gwdata, s1.g, s2.g, d.g);
}

GW_INLINE void inl_gwaddsub(GWInteger &a,GWInteger &b)
{
   gwaddsub4 (&gwdata, a.g, b.g, a.g, b.g);
}

GW_INLINE void inl_gwaddsub4(const GWInteger &s1,const GWInteger &s2,GWInteger &d1,GWInteger &d2)
{
   gwaddsub4 (&gwdata, s1.g, s2.g, d1.g, d2.g);
}

GW_INLINE void inl_gwsmallmul(double m,GWInteger &s)
{
   gwsmallmul (&gwdata, m, s.g);
}

