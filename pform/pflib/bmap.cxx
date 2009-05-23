/*-------------------------------------------------------------------------
/                                                                         *
/ Bitmap.cxx  Copyright Jim Fougeron, 2000/2001.  All rights reseved.     *
/                                                                         *
/ Usage of this code by other developers is granted by the author, Jim    *
/ Fougeron, however, the author does require written notification before  *
/ this code is put into a project.  This copyright notice MUST be         *
/ maintained in its original form within the source code.  Also, if       *
/ the functionality of this code becomes a significant part of the        *
/ finished product/utility, then the author requires a cursory            *
/ acknowledgement in the program's "about" or "startup" text screen.      *
/                                                                         *
/ The author also reqests that all modifications to the code, and         *
/ bug fixes or porting issues be sent to him to incorporate into          *
/ on going versions of this code.                                         *
/                                                                         *
/-------------------------------------------------------------------------*/


// Description:
//
// Enhanced BITMAP functions.  We will build a bit-map using unsigned longs
// to map which values we care about.  Then a simple index lookup will do
// the work.  This code is about as fast as can be made, without dropping
// into asm.  Also, these are done as #defines so that the code works for
// the i element being ints, longs, unsigned, __int64, ...


const uint32 Bits[] =   {0x00000001,0x00000001,0x00000002,0x00000002,0x00000004,0x00000004,0x00000008,0x00000008, 
                         0x00000010,0x00000010,0x00000020,0x00000020,0x00000040,0x00000040,0x00000080,0x00000080,
                         0x00000100,0x00000100,0x00000200,0x00000200,0x00000400,0x00000400,0x00000800,0x00000800,
                         0x00001000,0x00001000,0x00002000,0x00002000,0x00004000,0x00004000,0x00008000,0x00008000,
                         0x00010000,0x00010000,0x00020000,0x00020000,0x00040000,0x00040000,0x00080000,0x00080000,
                         0x00100000,0x00100000,0x00200000,0x00200000,0x00400000,0x00400000,0x00800000,0x00800000,
                         0x01000000,0x01000000,0x02000000,0x02000000,0x04000000,0x04000000,0x08000000,0x08000000,
                         0x10000000,0x10000000,0x20000000,0x20000000,0x40000000,0x40000000,0x80000000,0x80000000};

const uint32 Bits2[] =  {0x00000001,0x00000002,0x00000004,0x00000008, 
                         0x00000010,0x00000020,0x00000040,0x00000080,
                         0x00000100,0x00000200,0x00000400,0x00000800, 
                         0x00001000,0x00002000,0x00004000,0x00008000,
                         0x00010000,0x00020000,0x00040000,0x00080000,
                         0x00100000,0x00200000,0x00400000,0x00800000,
                         0x01000000,0x02000000,0x04000000,0x08000000,
                         0x10000000,0x20000000,0x40000000,0x80000000};

// Note that i MUST be >= than MinNum BEFORE calling any of these macros!!!!!

// These macros work on a bitmap which "skips" even's.  Odd's can also be skipped because of the duplication
// of values in the Bits[] array. This odd skipping is needed in CPAPSieve when sieving a range of solid 
// numbers of the form b^n+k  where b is odd.  All k's must be even for the resultant number to be odd.
#ifndef USE_BITMAP_INLINES
#define  IsBitSet(sv,i) (sv[(i)>>6] &  Bits[(i)&0x3F] )
#define  ClearBit(sv,i) (sv[(i)>>6] ^= Bits[(i)&0x3F] )
#define    SetBit(sv,i) (sv[(i)>>6] |= Bits[(i)&0x3F] )
#else
inline uint32 IsBitSet(uint32 *sv, uint32 i) {return (sv[(i)>>6] & Bits[(i)&0x3F]);}
inline uint32 IsBitSet(uint32 *sv, uint64 i) {return (sv[(i)>>6] & Bits[(i)&0x3F]);}
inline void ClearBit(uint32 *sv, uint32 i)   {sv[(i)>>6] ^= Bits[(i)&0x3F];}
inline void ClearBit(uint32 *sv, uint64 i)   {sv[(i)>>6] ^= Bits[(i)&0x3F];}
inline void SetBit(uint32 *sv, uint32 i)     {sv[(i)>>6] |= Bits[(i)&0x3F];}
inline void SetBit(uint32 *sv, uint64 i)     {sv[(i)>>6] |= Bits[(i)&0x3F];}
#endif

// These macros work on a "solid" bitmap.  One which contains both even's and odd's.
// These now use new array Bits2 since the IsBitSet version uses a larger table.
#ifndef USE_BITMAP_INLINES
#define  IsBitSet2(sv,i) (sv[(i)>>5] &  Bits2[(i)&0x1F] )
#define  ClearBit2(sv,i) (sv[(i)>>5] ^= Bits2[(i)&0x1F] )
#define    SetBit2(sv,i) (sv[(i)>>5] |= Bits2[(i)&0x1F] )
#else
//  gcc-beos had some optimizer problems with the #defines above if used in this manner (with uint64 for m_uNext)
//  while (!isBitSet2(pmap,m_uNext) m_uNext++;
//  These inline functions work around this issue, but they are slower by about 5-15% with a VC build.
inline uint32 IsBitSet2(uint32 *sv, uint32 i) {return (sv[(i)>>5] & Bits2[(i)&0x1F]);}
inline uint32 IsBitSet2(uint32 *sv, uint64 i) {return (sv[(i)>>5] & Bits2[(i)&0x1F]);}
inline void ClearBit2(uint32 *sv, uint32 i)   {sv[(i)>>5] ^= Bits2[(i)&0x1F];}
inline void ClearBit2(uint32 *sv, uint64 i)   {sv[(i)>>5] ^= Bits2[(i)&0x1F];}
inline void SetBit2(uint32 *sv, uint32 i)     {sv[(i)>>5] |= Bits2[(i)&0x1F];}
inline void SetBit2(uint32 *sv, uint64 i)     {sv[(i)>>5] |= Bits2[(i)&0x1F];}
#endif

// Skip even's "allocator".  a range of 64 (evens or odd's only) numbers will fit into each uint32
inline void Init_pMap(uint64 MinNum, uint64 MaxNum, uint32 **pMapPhysical, uint32 **pMapVirtual)
{
    uint64 max = (MaxNum>>6) + 10;  // 10 spare longs (just to be safe ;)
    int64 min = (MinNum>>6);

	// Allocate 1/2 the memory (skip evens mode)
	uint64 range = max-min;
	if (range > 0x2FFFFFFF)
	{
		fprintf (stderr, "TOO large of a range, aborting NOW\n");
		exit(0);
	}
	*pMapPhysical = new uint32[(uint32)range+8];

    // NOTE NOTE NOTE that pMap[0 .. min] is INVALID to dereference.  See the note
	// above the macros which states that the i value of EACH of the macros MUST be
	// "fixed" so that it is NOT less than MinNum.  This second pointer (pMap)
    // is actually points to a "sparse" array.  Only the valid range has been
	// allocated, but care MUST be taken to make sure that only the valid part is
	// accessed!!!
	unsigned *p = *pMapPhysical;
    *pMapVirtual = &p[-min+1+4];
}

// Solid range "allocator".  a range of 32 numbers will fit into each uint32
inline void Init_pMap2(uint64 MinNum, uint64 MaxNum, uint32 **pMapPhysical, uint32 **pMapVirtual)
{
    uint64 max = (MaxNum>>5) + 10;  // 10 spare longs (just to be safe ;)
   	int64 min = (MinNum>>5);

	uint64 range = max-min;
	if (range > 0x2FFFFFFF)
	{
		fprintf (stderr, "TOO large of a range, aborting NOW\n");
		exit(0);
	}
	*pMapPhysical = new uint32[(uint32)range+8];

    // NOTE NOTE NOTE that pMap[0 .. min] is INVALID to dereference.  See the note
	// above the macros which states that the i value of EACH of the macros MUST be
	// "fixed" so that it is NOT less than MinNum.  This second pointer (pMap)
    // is actually points to a "sparse" array.  Only the valid range has been
	// allocated, but care MUST be taken to make sure that only the valid part is
	// accessed!!!
	uint32 *p = *pMapPhysical;
   *pMapVirtual = &p[-min+1+4];
}

inline void Set_All_bits_true(uint64 MinNum, uint64 MaxNum, uint32 &nvalsleft, uint32 *pMapVirtual)
{
    uint64 max = (MaxNum>>6) + 5;  // 5 spare longs (just to be safe ;)
    uint64 min = (MinNum>>6);
    register uint32 *p = &pMapVirtual[min-2];
    register uint32 Num = (uint32)(max-min)+4;

	do
	{
		*p++ = 0xFFFFFFFFUL;
	}
	while (--Num > 1);
    nvalsleft = (uint32) (((MaxNum-MinNum))>>1);
	nvalsleft++;
}

inline void Set_All_bits_true2(uint64 MinNum, uint64 MaxNum, uint32 &nvalsleft, uint32 *pMapVirtual)
{
    uint64 max = (MaxNum>>5) + 5;  // 5 spare longs (just to be safe ;)
    uint64 min = (MinNum>>5);
    register uint32 *p = &pMapVirtual[min-2];
    register uint32 Num = (uint32)(max-min)+4;

	do
	{
		*p++ = 0xFFFFFFFFUL;
	}
	while (--Num > 1);
    nvalsleft = (uint32) (((MaxNum-MinNum))>>1);
	nvalsleft++;
}

inline void Set_All_bits_false(uint64 MinNum, uint64 MaxNum, uint32 &nvalsleft, uint32 *pMapVirtual)
{
    uint64 max = (MaxNum>>6) + 5;  // 5 spare longs (just to be safe ;)
    uint64 min = (MinNum>>6);
    register uint32 *p = &pMapVirtual[min-2];
    register uint32 Num = (uint32)(max-min)+4;

	do
	{
		*p++ = 0;
	}
	while (--Num > 1);
    nvalsleft = 0;
}

inline void Set_All_bits_false2(uint64 MinNum, uint64 MaxNum, uint32 &nvalsleft, uint32 *pMapVirtual)
{
    uint64 max = (MaxNum>>5) + 5;  // 5 spare longs (just to be safe ;)
    uint64 min = (MinNum>>5);
    register uint32 *p = &pMapVirtual[min-2];
    register uint32 Num = (uint32)(max-min)+4;

	do
	{
		*p++ = 0;
	}
	while (--Num > 1);
    nvalsleft = 0;
}

inline void Free_pMap(uint32 **pMapPhysical)
{
    delete[] (*pMapPhysical);
    (*pMapPhysical) = 0;
}

inline void Free_pMap2(uint32 **pMapPhysical)
{
    delete[] (*pMapPhysical);
    (*pMapPhysical) = 0;
}
