#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <stdio.h>

#include "compri5.h"
#include "compri7.h"

const unsigned non7s[7] = { 2, 32, 16, 129, 8, 4, 64 };


int cp7expandstream(unsigned int*into, unsigned char const* buf, unsigned int rows)
{ 
    unsigned int base=0;
    unsigned int i=0;
    unsigned int*originto=into;
    unsigned char building=0, builtbits=0;
    for(; i<rows; ++i)
    {   
	unsigned char lastread=*buf++;
	unsigned char pulledout;
	builtbits |= non7s[i%6]; /* eeek! % is slow */
	pulledout=(unsigned char)(lastread&builtbits);
	lastread = (unsigned char)((lastread&~builtbits) | building);
	building = pulledout;
	into = cp5expand(into, base, lastread); 
	base +=30;
	if((builtbits | non7s[6]) == 0xff)
	{
	    into = cp5expand(into, base, building); 
	    base +=30;
	    building=0;
	    builtbits=0;
	}
    }
    if(builtbits && building)
    {
	into = cp5expand(into, base, building); 
	base +=30;
    }
    return into-originto;
}

