// Cyclic redundancy checks based on the CRC-32
#include "pflibpch.h"
#include "crc32.h"

static unsigned long crcXOR32[256];
static unsigned long crcXOR40[256];
static unsigned long crcXOR48[256];
static unsigned long crcXOR56[256];

void crc_init()
{
// calculate the crc tables
// basic CRC algorithm:
// next message byte is shifted into the current CRC, to give a
// 40 bit result. This is reduced to a 32-bit remainder, the
// high 8 bits determine how to subtract the CRC_VALUE. Since
// subtraction in this field is "exclusive or" then the XOR
// table can be prebuilt.
    unsigned long idx,remainder;
    unsigned long crc;

    // note the idea is to calculate CRC(idx.x^32).
    unsigned long crcBit[8];    // the crc of bit 32+i
    crcBit[0]=CRC_VALUE;

	for(idx=1;idx<8;idx++)
    {
        crc=crcBit[idx-1];
        // multiply by x and reduce.
        if(crc&0x80000000)
        {
            crc<<=1;
            crc^=CRC_VALUE;
        }
        else
        {
            crc<<=1;
        }
        crcBit[idx]=crc;
    }

    for(idx=0;idx<256;idx++)
    {
        crc=0;
        for(int i=0;i<8;i++)
        {
            if(idx&(1<<i))
            {
                crc^=crcBit[i];
            }
        }
        crcXOR32[idx]=crc;
    }
// compute 40, 48, 56 tables. These correspond to the effect
// of CRC reduction on higher message bytes

    for(idx=0;idx<256;idx++)
    {
        crc=crcXOR32[idx];      // this is the CRC of idx<<32

        remainder=crc>>24;
        crc<<=8;                  // now unreduced CRC of idx<<40
        crc^=crcXOR32[remainder]; // reduced
        crcXOR40[idx]=crc;

        remainder=crc>>24;
        crc<<=8;                  // now unreduced CRC of idx<<48
        crc^=crcXOR32[remainder]; // reduced
        crcXOR48[idx]=crc;

        remainder=crc>>24;
        crc<<=8;                  // now unreduced CRC of idx<<56
        crc^=crcXOR32[remainder]; // reduced
        crcXOR56[idx]=crc;
    }
}

uint32 crc_byte(unsigned char c,uint32 crc)
{
    crc=(crc<<8)^((c)&(0x000000FFUL))^crcXOR32[(crc>>24)];
    return crc;
}
