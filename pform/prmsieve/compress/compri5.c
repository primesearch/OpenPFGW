#if !defined(_COMPRI5_H)
#include "compri5.h"
#endif

#include "../tables/pmod30.h"

unsigned int* cp5expand(unsigned int* into, unsigned int base, unsigned char i)
{
    int j;
    /*printf("%02x -> ", i);*/
    for(j=0; j<8; ++j)
    {
      /*if(i&1) printf("%u\n", base+pmod30[j]);*/
	if(i&1) *into++=base+pmod30[j];
	i>>=1;
    }
    /*printf("\n");*/
    return into;
}
