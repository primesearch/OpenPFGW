#include <stdio.h>
#include <stdlib.h>

#include "../tables/pmod30.h"
/*#include "compri5.h"*/
#include "../compress/compri7.h"

/* This is an experimental area for testing small block-block conversions
 * Due to the shared premises and painfully granular stage-by-stage
 * nature of these routines, none of them are worthy of going in the main
 * development files. The whole scheme here is stdin->munge->stdout
 * which is not what you're after inside code really.
 *
 * This is the assumed program for generation of C arrays!
 *
 * If you wish to add a new filter, add it here first, and after you are
 * happy each stage works, then write the optimised version (one function 
 * to compress, one to decompress), in its own file.
 */

/* These are global variables as I wanted the functions to share only one
 * copy of them, and maybe be able to modify them.
 * These really shouldn't all be separate functions.
 * Take the function pipe you want, and combine all of them into 
 * one function, then add these as local variables to the top.
 */
static int inpoffs=0;
static int base=0;
static div_t split;
static int lastp=0;
static int nump=0;

/* To write data to an array rather than printing, these 2 are wanted
static int totalrows=0;
static int*ptr=NULL;
*/
typedef unsigned char byte;
typedef int(*fn76)(byte inp[7], byte out[6], int rows);
typedef fn76 methods[5];
/* Was
struct
{
    fn76 read;
    fn76 compres;
    fn76 verify;
    fn76 print;
    fn76 clear;
} methods;
*/

int prime7in(byte inp[7], byte out[6], int rows)
{
    int gotrows=7; /* good default? */
    char buff[20];
    if(split.quot>=7) 
    {
	inpoffs+=210;
	split.quot-=7;
    }
    if(split.quot<7)
    {
	if(lastp) inp[split.quot] |= 1 << (pmod30inv[split.rem]);
        split.quot=-1;
	while(fgets(buff, sizeof(buff), stdin))
	{
	    unsigned p=strtoul(buff,0,0);
	    if(p<=7) continue;
	    lastp=p;
	    split=div(lastp-inpoffs,30);
	    /*printf("Read %i->%i:%i\n", lastp, split.quot, split.rem);*/
	    if(split.quot >= 7) { /*printf("%u Too big\n", lastp);*/ break; }
	    inp[split.quot] |= 1 << (pmod30inv[split.rem]);
	}
	if(split.quot<7) gotrows=split.quot+1;
    }
    /*printf("prime7in got %i rows\n", gotrows);*/
    return gotrows;
}
/* need a global variable if we want to 'print' to an array */
int prime7out(byte inp[7], byte out[6], int rows)
{
    int pgot=0, r;
    for(r=0; r<rows; ++r)
    {
	byte b=inp[r];
	int i;
	for(i=0; i<8; ++i) 
	    if(b&(1<<i)) 
	    { 
		printf("%u\n", base+pmod30[i]); 
		/*ptr[pgot]=base+pmod30[i];*/
		++pgot; /* not needed */
	    }
	base+=30;
    }
    return rows; /* 7 bytes processed */
}



int hex6in(byte*inp, byte*out, int rows)
{ 
    char buf[6*sizeof("0xff, ")+50]; /* allow comments */
    char*c=buf, *c2;
    unsigned int i;
    rows=0;
    if(!fgets(buf, sizeof(buf), stdin)) *buf=0; 
    /*printf("hex6in read %s", buf);*/
    while((i=strtoul(c,&c2,0)) < 0x100 && rows<6 && c2!=c)
    {
	out[rows++] = i;
	c=++c2;
    }
    /*printf("hex6in got %i rows of bits\n", rows);*/
    return rows;
}
int hex7in(byte*inp, byte*out, int rows)
{ 
    char buf[7*sizeof("0xff, ")+50]; /* allow comments*/
    char*c=buf, *c2;
    unsigned int i;
    rows=0;
    if(!fgets(buf, sizeof(buf), stdin)) *buf=0; 
    /*printf("hex7in read %s", buf);*/
    while((i=strtoul(c,&c2,0)) < 0x100 && rows<7 && c2!=c)
    {
	inp[rows++] = i;
	c=++c2;
    }
    /*printf("hex7in got %i rows of bits\n", rows);*/
    return rows;
}



int do7to6(byte inp[7], byte out[6], int rows)
{
    /* now compress sevens */
    int i;
    int outrows=0;
    byte building=0, builtbits=0;
    
    for(i=0; i<rows; ++i)
    {
	int rowtype=i%7;
	byte lastread = inp[i];
	byte non7bits=~non7s[rowtype];
	byte cantinsertbits=non7bits&builtbits;
	byte caninsertbits=non7bits&~builtbits;
	building |= (caninsertbits & lastread);
	builtbits |= caninsertbits;
	
	if(builtbits==0xff)
	{
	    out[outrows++] = building;
	    building = cantinsertbits & lastread;
	    builtbits = cantinsertbits;
	}
    }
    if(builtbits)
    {
	out[outrows++] = building;
    }
    return outrows;
}
int do6to7(byte inp[7], byte out[6], int rows)
{ 
    int i;
    int outrows=0;
    byte building=0, builtbits=0;
    for(i=0; i<rows; ++i)
    {   
	byte lastread=out[i];
	byte pulledout;
	builtbits |= non7s[i%6];
	pulledout=lastread & builtbits;
	lastread = (lastread&~builtbits) | building;
	building = pulledout;
	inp[outrows++] = lastread;
	if((builtbits | non7s[6]) == 0xff)
	{
	    inp[outrows++] = building;
	    building=0;
	    builtbits=0;
	}
    }
    if(builtbits && building)
    {
	inp[outrows++] = building;
    }
    return outrows;
}

int verify6yields7(byte inp[7], byte out[6], int rows)
{
    byte tmp[7]={0,0,0,0,0,0,0};
    return memcmp(inp, tmp, do6to7(tmp, out, rows)) ? 0 : rows;
}
int verify7yields6(byte inp[7], byte out[6], int rows)
{
    byte tmp[6]={0,0,0,0,0,0};
    return memcmp(tmp, out, do7to6(inp, tmp, rows)) ? 0 : rows;
}
/* should these clear all 7 bytes? 
 * Default - yup 
 */
int clear7(byte*inp, byte*out, int rows)
{ 
    int i;
    for(i=0; i<7;++i) inp[i]=0;
    return rows;
}
int clear6(byte*inp, byte*out, int rows)
{
    int i;
    for(i=0; i<6;++i) out[i]=0;
    return rows;
}

int hex6out(byte*inp, byte*out, int rows)
{ 
    int i;
    for(i=0; i<rows;++i) { printf("%#02x, ", out[i]); }
    base+=210;
#if 0
    if(rows) printf("/* %i bits to <%u */\n", rows*8, base); 
#else
    if(rows) printf("\n"); 
#endif
    return rows;
}

int hex7out(byte*inp, byte*out, int rows)
{
    int i;
    for(i=0; i<rows;++i) { printf("%#02x, ", inp[i]); base+=30; }
#if 0
    if(rows) printf("/* %i bits to %u */\n", rows*8, base);
#else
    if(rows) printf("\n");
#endif
    return rows;
}

methods allmethods[]=
{ 
    { prime7in, NULL,   NULL,           hex7out,   clear7 }, /* enc 7 */
    { hex7in,   NULL,   NULL,           prime7out, NULL   }, /* dec 7 */
    { prime7in, do7to6, verify6yields7, hex6out,   clear7 }, /* enc 6 */
    { hex6in,   do6to7, verify7yields6, prime7out, NULL   }, /* dec 6 */
};

int main(int argc, byte* argv[])
{
    int direction = (argv[1] && argv[1][0]=='d') ? ++argv[1], 1 : 0;
    int size = (argv[1] && argv[1][0]=='7') ? 0 : 2; 
    methods*dostuff = &allmethods[size+direction];
    byte buf7[7] = {0,0,0,0,0,0,0};
    byte buf6[6] = {0,0,0,0,0,0};


    int got=0;
    do {
        int phase;
	for(phase=0; phase<5; ++phase)
	{
	    if((*dostuff)[phase]) got=(*dostuff)[phase](buf7, buf6, got);
	}
    } while(got>0);
    return 0;
}
	    
#if 0
/* expand with */
   for(i=0; i<273; ++i)
     {
	int j;
	for(j=0; j<32; ++j)
	  {
	     if(inp[i]&(1<<j)) have "30*((i<<2)+(j>>3))+pmod30[j&7]"
	  }
     }
#endif
