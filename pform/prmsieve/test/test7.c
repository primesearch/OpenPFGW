#include <stdio.h>
#include <stdlib.h>
#include "compri7.h"

unsigned char cp7array[] =
{
#include "../tables/onemeg.cp7"
};

int main()
{
    int* primes=(int*)malloc(82021*sizeof(int));
    int np=cp7expandstream(primes, cp7array, sizeof(cp7array));
    printf("primes=%u\n", np);
    free(primes);
    return np!=82021;
}

