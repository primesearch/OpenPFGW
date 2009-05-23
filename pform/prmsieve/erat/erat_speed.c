#include "erat.h"

/*
 * OPTIONAL FUNCTIONS
 */


#if 1
#include "timing.h"

/* We don't need this feature */
uint64 erat_count(uint64 to);

static uint32 countit()
{
  int i;
  register uint32 *ai;
  register int pos;
  register uint32 result;

</* To print the primes (slowly), given Llo:
*/
#if 0
   int k;
   for (k = ERAT_BITS;k-->0;)
    for (i = 0;i < 8;++i)
      if (!(a[i][k / 32] & two[k & 31]))
	printf("%d\n", 30 * (Llo + (ERAT_BITS-1-k)) + dtab[i]);
#endif

  result = 0;
  for (i = 0;i < 8;++i) {
    ai = window[i];
    for (pos = 0;pos < ERAT_W;pos+=2) {
      register uint32 bits = ~ai[pos];
      register uint32 bits2 = ~ai[pos+1];
      result += pop[bits & 255]; bits >>= 8;
      result += pop[bits2 & 255]; bits2 >>= 8;
      result += pop[bits & 255]; bits >>= 8;
      result += pop[bits2 & 255]; bits2 >>= 8;
      result += pop[bits & 255]; bits >>= 8;
      result += pop[bits2 & 255]; bits2 >>= 8;
      result += pop[bits];
      result += pop[bits2];
    }
  }
  Lbase+=ERAT_BITS*30;
  return result;
}

void erat_speed(uint64 to)
{
  uint32 total;
  timing start;
  timing_basic startb;
  timing finish;
  timing_basic finishb;
  timing told;

  timing_basic_now(&startb);
  timing_now(&start);

  timing_now(&told);

  erat_init(); /* calls bootstrap, no more */

  /* don't count the fixed size array, we know how big it is */
  total = maxsmallprimeindex; 

  timing_now(&finish);
  printf("#Init: %f. Counting to %llu\n",
	 timing_diff(&finish,&told), to);
  told = finish;
  to/=30;
  Lhi=(smallptab[maxsmallprimeindex-1]/30)+1;
  initNextVals();
  while(Lhi < to)   /* Lhi as we have not advanced range */
  {
    advanceRange(); /* old Lhi is where we now want to start */
    PRINTF(("#Lbase=%lu, Llo=%lu, Lhi=%lu\n", Lbase, Llo, Lhi));
    fillBitmap();
    total+=countit();
    timing_now(&finish);
    printf("#Finished L=%d: tot=%lu, %f\n",Lhi,total,timing_diff(&finish,&told)); 
    told = finish;
  } 

  timing_basic_now(&finishb);

  printf("%lu primes up to %d.\n",total,30 * Lhi);

  printf("Timings are in ticks. Nanoseconds per tick: approximately %f.\n",timing_basic_diff(&finishb,&startb) / timing_diff(&finish,&start));
  printf("Overall seconds: approximately %f.\n",0.000000001 * timing_basic_diff(&finishb,&startb));
  printf("Tick counts may be underestimates on systems without hardware tick support.\n");

  exit(0);
}
#endif
