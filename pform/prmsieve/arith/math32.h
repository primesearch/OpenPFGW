#if defined (__i386__) || defined (__i486__) 
#include "x86/math32.h"
#elif defined(__alpha__)
#include "axp/math32.h"
#endif

#if defined(TOPBITNUM32)
INLINE static int topbitnum32(uint32 n) {int t;TOPBITNUM32(t,n);return t;}
#else
int topbitnum32(uint32 n);
#endif

#if defined(TOPBITVAL32)
INLINE static int topbitval32(uint32 n) {int t;TOPBITVAL32(t,n);return t; }
#else
int topbitnum32(uint32 n);
#endif

#if defined(MOD32)
INLINE static uint32 mod32(uint32 x,uint32 p){uint32 t;MOD32(t,x,p);return t;} 
#else
uint32 mod32(uint32 x, uint32 p);
#endif

#if defined(EXPMOD32_r)
INLINE static uint32 expmod32_r(uint32 x, uint32 n, uint32 p, uint32 rp)
       {uint32 t; EXPMOD32_r(t, x, n, p, rp); return t;}
#else
uint32 expmod32_r(uint32 x, uint32 n, uint32 p, uint32 rp);
#endif

