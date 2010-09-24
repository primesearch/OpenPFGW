#include "pfgwlibpch.h"

#include "gwinteger.h"
#include "gwcontext.h"
#ifdef _MSC_VER
#include "wingmp.h"
#else
#include "gmp.h"
#endif

// build the inlines into the C++ if needed
#ifndef GW_INLINE_ENABLED
#include "gwinteger.inl"
#endif

// GWInteger wrappers
GWInteger::GWInteger()
{
   g=gwalloc(&gwdata);
   mpz_init(scrap);
}

GWInteger::GWInteger(const GWInteger &x)
{
   g=gwalloc(&gwdata);
   gwcopy(&gwdata,x.g,g);
   mpz_init(scrap);
}

GWInteger::~GWInteger()
{
   // Ugly hack to make sure gwdone has not already been called
   if (gwdata.gwnum_alloc != NULL)
      gwfree(&gwdata, g);
   mpz_clear(scrap);
}

GWInteger &GWInteger::operator=(const Integer &I)
{
   mpz_ptr gmp = I.gmp();

   if (sizeof (mp_limb_t) == sizeof (uint32_t))
      binarytogw (&gwdata, (uint32_t *) gmp->_mp_d, gmp->_mp_size, g);
   else
      binary64togw (&gwdata, (uint64_t *) gmp->_mp_d, gmp->_mp_size, g);

   return *this;
}

double GWInteger::suminp()
{
   return (gwsuminp (&gwdata, g));
}
double GWInteger::sumout()
{
   return (gwsumout (&gwdata, g));
}
double GWInteger::sumdiff()
{
   return (fabs (suminp() - sumout()));
}

