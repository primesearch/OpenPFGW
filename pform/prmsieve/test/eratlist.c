#include "../erat/erat.h"
#include "../types/uint32.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char**argv)
{
  uint32 total;
  erat_modulus(argv[1]?strtoul(argv[1],0,0):2,
	       argv[1]&&argv[2]?strtoul(argv[2],0,0):1);
  erat_init();
  while((total=erat_next())<1000000000) { printf("%u\n", total); }
  return 0;
}
