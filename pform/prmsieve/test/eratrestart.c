#include "../erat/erat.h"
#include "../atkins/primegen.h"
#include "../types/uint64.h"
#include "../types/uint32.h"
#include <stdio.h>

/* These are the number of times we will go through the loop for each 
 * number in the default test (no scale)
 * They are used so that you can run the right number of loops with
 * slow %'s and compare either ieve to no sieve at all
 */
int cycles[]= { 3, 4, 2, 12, 6, 2, 7, 137, 2, 500286, 3, 2, 24, 693,
2, 3, 198530, 2, 43, 37, 2, 5, 4, 2, 400, 3, 2, 2063690, 118, 2, 3,
104, 2, 28, 11621, 2, 4, 29, 2, 498, 3, 2, 22, 4, 2, 3, 25, 2, 9,
2063690, 2, 256659, 20, 2, 5, 3, 2, 4, 106424, 2, 3, 11, 2, 8, 4, 2,
80, 43906, 2, 6, 3, 2, 2063690, 1440631, 2, 3, 5, 2, 4, 2063690, 2,
15078, 6, 2, 10, 3, 2, 5, 1323, 2, 3, 7, 2, 21, 9, 2, 2063690, 772490,
2, 4, 3, 2, 2063690, 70504, 2, 3, 4, 2, 6, 5, 2, 2063690, 16, 2, 12,
3, 2, 9, 53, 2, 3, 6, 2, 11, 47, 2, 55, 4, 2, 103, 3, 2, 2063690,
573215, 2, 3, 208, 2, 236238, 8, 2, 4, 5, 2, 204, 3, 2, 6, 4, 2, 3,
12, 2, 5, 11, 2, 15, 35, 2, 7, 3, 2, 4, 9, 2, 3, 21, 2, 2063690, 4, 2,
10, 2063690, 2, 6196, 3, 2, 8, 8055, 2, 3, 67, 2, 4, 30, 2, 5, 717, 2,
31, 3, 2, 2063690, 7, 2, 3, 8, 2, 2063690, 6, 2, 255, 26, 2, 4, 3, 2,
17168, 5, 2, 3, 4, 2, 42, 23, 2, 11, 2063690, 2, 5, 3, 2, 65, 209425,
2, 3, 60, 2, 802, 10, 2, 27, 4, 2, 8, 3, 2, 14, 6, 2, 3, 5, 2, 241, 7,
2, 4, 11, 2, 2063690, 3, 2, 5, 4, 2, 3, 99, 2, 10, 476186, 2, 7, 12,
2, 6, 3, 2, 4, 75, 2, 3, 16, 2, 41, 4, 2, 90303, 6, 2, 22, 3, 2,
2063690, 552642, 2, 3, 1085417, 2, 4, 543617, 2, 8, 2063690, 2, 555,
3, 2, 15, 2063690, 2, 3, 9, 2, 6, 2063690, 2, 35, 5, 2, 4, 3, 2, 7,
66, 2, 3, 4, 2, 5, 2063690, 2, 2063690, 2063690, 2, 9, 3, 2, 177, 13,
2, 3, 2063690, 2, 37, 27, 2, 12, 4, 2, 61017, 3, 2, 6, 2063690, 2, 3,
7, 2, 8, 288038, 2, 4, 17, 2, 28, 3, 2, 2063690, 4, 2, 3, 36, 2, 7,
29, 2, 14, 8, 2, 13, 3, 2, 4, 5, 2, 3, 1982107, 2, 19, 4, 2, 6,
2063690, 2, 5, 3, 2, 2063690, 57, 2, 3, 15, 2, 4, 6, 2, 1868, 7, 2,
382, 3, 2, 11, 10, 2, 3, 5, 2, 1882, 14, 2, 17, 2063690, 2, 4, 3, 2,
5, 32, 2, 3, 4, 2, 59, 8, 2, 2063690, 191, 2, 374, 3, 2, 10, 6, 2, 3,
2063690, 2, 15, 5, 2, 2063690, 4, 2, 2063690, 3, 2, 12, 7, 2, 3, 13,
2, 553640, 2063690, 2, 4, 24, 2, 6, 3, 2, 8, 4, 2, 3, 2063690, 2, 276,
251, 2, 98, 5, 2, 92, 3, 2, 4, 20, 2, 3, 8, 2, 5, 4, 2, 114, 46, 2,
42308, 3, 2, 13, 16119, 2, 3, 2063690, 2, 4, 7, 2, 
}; 

static primegen pg;

int main(int argc, char**argv) 
{
    char mode=argv[1]?argv[1][0]:'0';
    int scale=(argv[1]&&argv[2])?strtoul(argv[2],0,0):1;
    int*loops=cycles;
    uint64 factortest=(((uint64)scale)<<50)+1;
    int test=0;
    printf("Pretending to find factors of 1000 numbers following %llu\n"
	   "Using sieve '%c' ('p'=atkins, 'e'=eratosthenes, '0'=none)\n",
	   factortest, mode);
   
    while(test<1000)
    { 
        uint64 factorme=factortest+test*scale*2; 
	uint32 prime=1;
	switch(mode) /* power supply */
	{
	case 'e':
	    erat_init(); 
	    do { 
		prime=erat_next(); 
	    } while(prime<(1<<25) && factorme%prime); 
	    break;
	case 'p':
	    primegen_init(&pg); 
	    do { 
		prime=primegen_next(&pg); 
	    } while(prime<(1<<25) && factorme%prime); 
	    break;
	case '0':
	    do { 
		prime++; 
	    } while(prime<=*loops && factorme%prime);
	    ++loops;
	    break;
	}
        printf("%llu has %s %u\n",
	       factorme, (prime<(1<<25))?"factor":"no factor up to", prime);
	++test; 
    } 
    if(mode=='0') printf("The above factors/limits are bollocks\n");
    return 0; 
}

