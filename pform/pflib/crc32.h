#include <inttypes.h>

#if !defined (OPENPFGW__CRC_32_H)
#define OPENPFGW__CRC_32_H

#define CRC_VALUE   0x04C11DB7

// note we use endian considerations here, for optimization
// purposes. The message will always be considered as little
// endian, but these dictate where a longword will appear in
// a character array
#ifndef BIG_ENDIAN
#define BYTE32  0
#define BYTE40  1
#define BYTE48  2
#define BYTE56  3
#else
#define BYTE32  3
#define BYTE40  2
#define BYTE48  1
#define BYTE56  0
#endif

void crc_init();
uint32_t crc_byte(unsigned char c, uint32_t crc=0);


#endif
