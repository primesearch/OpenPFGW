#if !defined(_PMOD30_H)
#define _PMOD30_H

/* These are the 8 values that primes can take modulo 30 */
extern const int pmod30[8];

/* This is the reverse of the above, with -1 meaning no inverse */
extern const int pmod30inv[30];

/* give this thing n%30 and it will tell you gcd(n,30) */
extern const int gcd30[30];

#endif
