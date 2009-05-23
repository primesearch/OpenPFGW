#if defined (__i386__) || defined (__i486__) 

# if defined (__GNUC__) 
#  define MOD32_r(result, u, divisor, recip)       \
    __asm__                                    \
( "divl %2"                                    \
  : "=&d" (result)                             \
  : "a" (u), "rm" (divisor)                    \
    ) 

#  define SQMOD32_r(result, u, divisor, recip)     \
    __asm__                                    \
( "mull %1\n\t"                                \
  "divl %2"                                    \
  : "=&d" (result)                             \
  : "a" (u), "rm" (divisor)                    \
    ) 

#  define MULMOD32_r(result, u, v, divisor, recip) \
    __asm__                                    \
( "mull %2\n\t"                                \
  "divl %3"                                    \
  : "=&d" (result)                             \
  : "a" (u), "rm" (v), "rm" (divisor)          \
    ) 
# endif /* end not __GCC__ */
#endif

