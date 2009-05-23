#include <stdio.h>
#include "timing.h"
#include "primegen.h"
#include "primegen_impl.h"
#include "fs64.h"
#include "twiddles.h"

timing told;
timing t;

uint64 timedcount(primegen *pg,uint64 to)
{
  uint64 count = 0;
  bufLine*buf = pg->buf;
  int pos;
  int j;
  register uint32 *bufj;
  register uint32 bits, bits2;
  register uint32 smallcount;

timing_now(&told);

  for (;;) {
    while (pg->num) {
      if (pg->p[pg->num - 1] >= to) return count;
      ++count;
      --pg->num;
    }

    smallcount = 0;
    pos = pg->pos;
    while ((pos < B32) && (pg->base + 1920 < to)) {
      for (j = 0;j < 16;j+=2) {
	bits = ~buf[j][pos]; bits2 = ~buf[j+1][pos];
        smallcount += pop[bits & 255]; bits >>= 8;
	smallcount += pop[bits2 & 255]; bits2 >>= 8;
	smallcount += pop[bits & 255]; bits >>= 8;
	smallcount += pop[bits2 & 255]; bits2 >>= 8;
	smallcount += pop[bits & 255]; bits >>=8;
        smallcount += pop[bits2 & 255]; bits2 >>= 8;
	smallcount += pop[bits];
	smallcount += pop[bits2];
      }
      pg->base += 1920;
      ++pos;
    }
    pg->pos = pos;
    count += smallcount;

    if (pos == B32)
      while (pg->base + B * 60 < to) {
        primegen_sieve(pg);

timing_now(&t);
printf("Finished L=%d: %f\n",(int) pg->L,timing_diff(&t,&told));
told = t;
        pg->L += B;
  
	smallcount = 0;
        for (j = 0;j < 16;++j) {
	  bufj = buf[j];
	  for (pos = 0;pos < B32;++pos) {
	    bits = ~bufj[pos];
	    smallcount += pop[bits & 255]; bits >>= 8;
	    smallcount += pop[bits & 255]; bits >>= 8;
	    smallcount += pop[bits & 255]; bits >>= 8;
	    smallcount += pop[bits];
	  }
	}
	count += smallcount;
        pg->base += B * 60;
      }

    primegen_fill(pg);
  }
}

char strnum[40];

primegen pg;

timing start;
timing_basic startb;
timing finish;
timing_basic finishb;

void main(argc,argv)
int argc;
char **argv;
{
    uint64 low = 2;
    uint64 high = 100000000;
    uint64 result;
    
    if (argv[1]) 
    {
	scan_uint64(argv[1],&low);
	if (argv[2])
	{
	    scan_uint64(argv[2],&high);
	}
	high += low;
    }

  timing_basic_now(&startb);
  timing_now(&start);

  primegen_init(&pg);

  primegen_skipto(&pg,low);

  result = timedcount(&pg,high);

  timing_basic_now(&finishb);
  timing_now(&finish);

  strnum[fmt_uint64(strnum,result)] = 0;
  printf("%s primes up to ",strnum);
  strnum[fmt_uint64(strnum,high)] = 0;
  printf("%s.\n",strnum);

  printf("Timings are in ticks. Nanoseconds per tick: approximately %f.\n",timing_basic_diff(&finishb,&startb) / timing_diff(&finish,&start));
  printf("Overall seconds: approximately %f.\n",0.000000001 * timing_basic_diff(&finishb,&startb));
  printf("Tick counts may be underestimates on systems without hardware tick support.\n");

  exit(0);
}
