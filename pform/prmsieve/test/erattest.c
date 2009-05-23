#include "../erat/erat.h"
#include "../atkins/primegen.h"
#include <stdio.h>

primegen pg;
int main()
{
    uint32 p, q;
    primegen_init(&pg);
    erat_init();
    do
    {
	p=primegen_next(&pg);
	q=erat_next();
	if((p&0xfffff)==0xfffff) { printf("Both agree to %u=%x\n", p, p); }
    } while(p==q);
    printf("PG say %u, E say %u\n", p, q);
    return(p!=q);
}
