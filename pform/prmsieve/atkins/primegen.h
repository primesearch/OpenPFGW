#ifndef PRIMEGEN_H
#define PRIMEGEN_H

#include "../types/uint32.h"
#include "../types/uint64.h"
#include "../types/int64.h"

#define PRIMEGEN_WORDS 1000

typedef uint32 bufLine[PRIMEGEN_WORDS];
typedef struct 
{
  int xsteps;
  int xsteps45;
  long xdelta;
  int64 idelta;
} freesteps_s;

typedef struct {
  /*uint32 buf[16][PRIMEGEN_WORDS];*/
  bufLine buf[16];
  uint64 p[512]; /* p[num-1] ... p[0], in that order */
  int num;
  int pos; /* next entry to use in buf; WORDS to restart */
  uint64 base;
  uint64 L;
  freesteps_s freesteps4, freesteps6, freesteps12;

} primegen;

extern void primegen_sieve(primegen *);
extern void primegen_fill(primegen *);

extern void primegen_init(primegen *);
extern uint64 primegen_next(primegen *);
extern uint64 primegen_peek(primegen *);
extern uint64 primegen_count(primegen *,uint64 to);
extern void primegen_skipto(primegen *,uint64 to);

#endif
