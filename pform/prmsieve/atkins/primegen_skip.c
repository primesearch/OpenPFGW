#include "primegen.h"
#include "primegen_impl.h"
#include "../tables/twiddles.h"


uint64 primegen_count(primegen *pg,uint64 to)
{
  uint64 count = 0;
  register int pos;
  register int j;
  register uint32 bits;
  register uint32 smallcount;

  for (;;) {
    while (pg->num) {
      if (pg->p[pg->num - 1] >= to) return count;
      ++count;
      --pg->num;
    }

    smallcount = 0;
    pos = pg->pos;
    while ((pos < B32) && (pg->base + 1920 < to)) {
      for (j = 0;j < 16;++j) {
	bits = ~pg->buf[j][pos];
	smallcount += pop[bits & 255]; bits >>= 8;
	smallcount += pop[bits & 255]; bits >>= 8;
	smallcount += pop[bits & 255]; bits >>= 8;
	smallcount += pop[bits & 255];
      }
      pg->base += 1920;
      ++pos;
    }
    pg->pos = pos;
    count += smallcount;

    if (pos == B32)
      while (pg->base + B * 60 < to) {
        primegen_sieve(pg);
        pg->L += B;
  
	smallcount = 0;
        for (j = 0;j < 16;++j)
	  for (pos = 0;pos < B32;++pos) {
	    bits = ~pg->buf[j][pos];
	    smallcount += pop[bits & 255]; bits >>= 8;
	    smallcount += pop[bits & 255]; bits >>= 8;
	    smallcount += pop[bits & 255]; bits >>= 8;
	    smallcount += pop[bits & 255];
	  }
        count += smallcount;
        pg->base += B * 60;
      }

    primegen_fill(pg);
  }
}

void primegen_skipto(primegen *pg,uint64 to)
{
  int pos;

  for (;;) {
    while (pg->num) {
      if (pg->p[pg->num - 1] >= to) return;
      --pg->num;
    }

    pos = pg->pos;
    while ((pos < B32) && (pg->base + 1920 < to)) {
      pg->base += 1920;
      ++pos;
    }
    pg->pos = pos;
    if (pos == B32)
      while (pg->base + B * 60 < to) {
        pg->L += B;
        pg->base += B * 60;
      }

    primegen_fill(pg);
  }
}
