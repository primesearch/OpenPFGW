#if !defined(_ERAT_H)
#define _ERAT_H

#include "../types/uint64.h"


extern void   erat_modulus(int, int);
extern void   erat_init();
extern void   erat_free();
extern uint64 erat_next();
extern void   erat_skipto(uint64 to);

/*
These to come later
extern uint64 erat_peek();
extern uint64 erat_count(uint64 to);
*/

/* This below strictly is private */
extern void erat_speed(uint64 to);

#endif
