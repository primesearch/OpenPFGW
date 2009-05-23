#include "../erat/erat.h"
#include "../types/uint32.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char**argv)
{
    int i=0;
    uint32 total=argv[1]?strtoul(argv[1],0,0):1;
    erat_modulus(2018,1);
    while(i++<80)
    {
	uint32 pr;
	erat_init();
        erat_skipto(total);
	pr=erat_next();
	printf("%u->%u%s\n", total, pr,(pr<total)?"!":""); 
	total+=(total>>2)+1;
    }
    return 0;
}
