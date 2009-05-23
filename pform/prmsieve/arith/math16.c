#include "math16.h"

uint32 sqmod16(uint32 x, uint32 p)
{
    return (x*x)%p;
}
     
uint32 mulmod16(uint32 x, uint32 y, uint32 p)
{
    return (x*y)%p;
}

uint32 expmod16(uint32 x, uint32 n, uint32 p)
{
    uint32 res = x%=p;
    uint32 tbv = 1;
    while(tbv<=n) 
    {
	tbv<<=1; 
    }
    tbv>>=1;
    /* Note - the top bit is already handled by starting at res=x
     * Otherwise, we'd start at 1, square it, and then multiply by x
     * which is pointless.
     * However, we cannot handle n=0, i.e. no bits set.
     */
    while(tbv>>=1)
    {
	res = sqmod16(res,p);
	if(n&tbv) 
	{
            res = mulmod16(res, x, p);
	}
    }
    return res;
}

