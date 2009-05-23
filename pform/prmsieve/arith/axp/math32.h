#if defined (__alpha__)

# if defined(__GNUC__)
#  define MOD32_r(result, n, divisor, recip)       \
    __asm__                                    \
( "mulq  %1, %3, %0\n\t"                       \
  "umulh %0, %2, %0\n\t"                       \
  : "=r" (result)                              \
  : "r" (n), "r" (divisor), "r" (recip)        \
    )

#  define SQMOD32_r(result, u, divisor, recip)     \
    __asm__                                    \
( "mulq  %1, %1, %0\n\t"                       \
  "mulq  %0, %3, %0\n\t"                       \
  "umulh %0, %2, %0\n\t"                       \
  : "=r" (result)                              \
  : "r" (u), "r" (divisor), "r" (recip)        \
    )

#  define MULMOD32_r(result, u, v, divisor, recip) \
    __asm__                                    \
( "mulq  %1, %2, %0\n\t"                       \
  "mulq  %0, %4, %0\n\t"                       \
  "umulh %0, %3, %0\n\t"                       \
  : "=r" (result)                              \
  : "r" (u), "r" (v), "r" (divisor), "r" (recip) \
    )


# endif
#endif
