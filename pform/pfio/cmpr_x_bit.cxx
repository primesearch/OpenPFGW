#if !defined (__COMPR_X_BIT_CXX)
#define __COMPR_X_BIT_CXX

//
// This file contains routines to do simple RLE compression targeted at the sieve data for 
// prime searches.  This is heavily modified version first put out in CPAPSieve.  It now
// encapsulates ABCD file and NewPGen files. These file types can be compressed and 
// decompressed or run "natively" in compressed format (i.e. this code) within PFGW.

//************************************************************
// Buffering file IO functions
//************************************************************
// Work buffer, so we don't have to write/read from the disk too frequently
static uint8_t RLE_Buf[0xF000];
// Counters to tell us where in the buffer the reads/writes are taking place.
static uint32_t  RLE_BufCnt, RLE_BufInCnt;
// Buffered read of a byte.
inline uint32_t Comp_GetByte(FILE *fp)
{
	if (RLE_BufCnt == RLE_BufInCnt)
	{
		RLE_BufCnt = (uint32_t) fread(RLE_Buf, 1, sizeof(RLE_Buf), fp);
		RLE_BufInCnt = 0;
		if (!RLE_BufCnt && feof(fp))
			return 0xFFFFFFFF;
	}
	return RLE_Buf[RLE_BufInCnt++];
}

// Here is a WORKING bit de-compression. (code taken from ABCZ project)

static uint32_t cmp_Bits;
static uint32_t cmp_Accum;
static uint32_t cmp_nAccum;
static uint32_t cmp_MAX_ESC;
static uint32_t bits_set;
static uint32_t cmp_MAX_ESCs[40], cmp_MAX_ESC2[40], cmp_ESC_min;
static uint64_t OffsetK;
static uint32_t cmpr_nvalsleft;
static uint32_t Base;
static uint32_t EscapeLevel;
static bool   bSkipEvens, bNewPGenFile;

inline uint32_t Comp_GetBitsX(FILE *fp)
{
	uint32_t RetVal=0;
	uint32_t BitsNeeded=cmp_Bits;
	uint32_t BitsAccum=0;
	while (BitsNeeded > cmp_nAccum)
	{
		RetVal |= (cmp_Accum<<BitsAccum);
		BitsAccum += cmp_nAccum;
		BitsNeeded -= cmp_nAccum;
		cmp_Accum = Comp_GetByte(fp);
		if (cmp_Accum == 0xFFFFFFFF)
			return cmp_Accum;
		cmp_nAccum = 8;
	}
	RetVal |= (cmp_Accum<<BitsAccum);
	cmp_nAccum -= BitsNeeded;
	if (!cmp_nAccum)
	{
		cmp_Accum = Comp_GetByte(fp);
		if (cmp_Accum != 0xFFFFFFFF)
			/* don't reset this if we are at the end of file */
			/* what we do at end of file is process this value, then the next call will fail */
			cmp_nAccum = 8;
	}
	else
		cmp_Accum >>= BitsNeeded;
	return (RetVal & cmp_MAX_ESC);
}

bool DeCompress_bits_From_PrZ_setup(uint32_t BitLevel, uint32_t _EscapeLevel, uint64_t _OffsetK, uint32_t _Base, uint32_t _nvalsleft, char *cpp, bool _bSkipEvens, bool _bNewPGenFile)
{
	RLE_BufCnt = RLE_BufInCnt = 0;
	bits_set=1;
	cmp_Accum=0;

	cmp_MAX_ESC = 1;
	cmp_MAX_ESC <<= BitLevel;
	--cmp_MAX_ESC;

	// Setup so that nothing is in the accumulator
	cmp_nAccum = 0;
	cmp_Bits = BitLevel;

	// Handles multiple escapes.
	EscapeLevel = _EscapeLevel;
	cmp_ESC_min = cmp_MAX_ESC - EscapeLevel;
	for (uint32_t i = 0; i <= EscapeLevel; ++i)
	{
		cmp_MAX_ESCs[i] = cmp_MAX_ESC - (EscapeLevel-i);
		cmp_MAX_ESC2[i] = cmp_ESC_min * (EscapeLevel-i+1);
		if (_bSkipEvens)
			cmp_MAX_ESC2[i] *= 2;
	}
	OffsetK = _OffsetK;
	Base = _Base;
	bSkipEvens = _bSkipEvens;
	bNewPGenFile = _bNewPGenFile;
	cmpr_nvalsleft = _nvalsleft;
	if (bNewPGenFile)
	{
		snprintf (cpp, 30, "%" PRIu64" %u", OffsetK, Base);
		return true;
	}
	return false;
}

bool DeCompress_bits_From_PrZ(FILE *fpIn, char *cpp, bool bDontOutput)
{
	uint32_t Zeros = Comp_GetBitsX(fpIn);
	if (Zeros == 0xFFFFFFFF || bits_set == cmpr_nvalsleft)
		return false;

	uint32_t i, Bit = 0;
	while (Zeros >= cmp_ESC_min)
	{
		for (i = 0; i < EscapeLevel; ++i)
			if (Zeros == cmp_MAX_ESCs[i])
				break;
		Bit += cmp_MAX_ESC2[i];
		Zeros = Comp_GetBitsX(fpIn);
	}
	if (bSkipEvens)
		OffsetK += Bit + (Zeros << 1) + 2;
	else
		OffsetK += Bit + Zeros + 1;
	if (!bDontOutput)
	{
		if (bNewPGenFile)
			snprintf (cpp, 30, "%" PRIu64" %u", OffsetK, Base);
		else
			sprintf (cpp, "%" PRIu64"", OffsetK);
	}
	++bits_set;
	return true;
}

#endif
