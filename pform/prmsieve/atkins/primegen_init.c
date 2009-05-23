#include "primegen.h"
#include "primegen_impl.h"

static void blankfreesteps(freesteps_s* pfs)
{
  pfs->xsteps=0;
  pfs->xsteps45=0;
  pfs->xdelta=0l;
  pfs->idelta=0;
}

void primegen_init(primegen *pg)
{
  blankfreesteps(&pg->freesteps4);
  blankfreesteps(&pg->freesteps6);
  blankfreesteps(&pg->freesteps12);
   
  pg->L = 1;
  pg->base = 60;

  pg->pos = PRIMEGEN_WORDS;

  pg->p[0] = 59;
  pg->p[1] = 53;
  pg->p[2] = 47;
  pg->p[3] = 43;
  pg->p[4] = 41;
  pg->p[5] = 37;
  pg->p[6] = 31;
  pg->p[7] = 29;
  pg->p[8] = 23;
  pg->p[9] = 19;
  pg->p[10] = 17;
  pg->p[11] = 13;
  pg->p[12] = 11;
  pg->p[13] = 7;
  pg->p[14] = 5;
  pg->p[15] = 3;
  pg->p[16] = 2;

  pg->num = 17;
}
