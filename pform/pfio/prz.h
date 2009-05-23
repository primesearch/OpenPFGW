#if !defined (__PrZ__H__)
#define __PrZ__H__

// Format of an PrZ file   (This is an RLE bit level compressed ABCD file)

#include <stdio.h>
#include <string.h>

#pragma pack(1)

/* Added in Version 2 header */
struct PrZ_SubFileSizes_t		// WIP
{
	uint8 PrZ_FSeekSize     : 2;			// 0-3 == 1 to 4 bytes  (this MIGHT not be needed)
	uint8 PrZ_OffKadjSize   : 2;			// 0-3 == 1 to 4 bytes
	uint8 PrZ_NadjSize      : 2;			// 0-3 == 0 to 3 bytes  (only FermFact/NewPGen uses this)
	uint8 PrZ_nvalsleftSize : 2;			// 0-3 == 1 to 4 bytes
	PrZ_SubFileSizes_t();
};

/* Version 1 header   (Version 2 when we utilize the PrZ_SubFileSizes) */
struct PrZ_File_Header
{
	PrZ_File_Header();
	char   PrZ_sig[4];	// PrZ\1a is signature
	uint8  PrZ_ver;		// Version 1 currently

	// Anon structs are not "legal" ISO C++, but the anon struct has been left
	// commented out in the source to "show" some additional "intentions" of how
	// this data is to "lay out".

//	struct
//	{
		uint8  PrZ_Bits : 5;	// add 3 to this value for 3 to 34 bits supported.  3 bit handles 2 to 16 (1 to 7 is not skip evens), 5 bit handles 2 to 62 (1 to 31 if not skip even) and 13 handles 2 to 16384 (or 1 to 8192 if not skip evens)
		uint8  Prz_ESCs : 3;	// 0 to 7 extra ESC's (plus the 1 default esc). This reduces the range of "valid gaps" by 1 for each extra ESC.  We already are 1 less than 2^bits since we have 1 ESC at a minimum
//	};
//	struct
//	{
		uint16 PrZ_SkipEvens       : 1;
		uint16 PrZ_IsFermFactABCD  : 1;
		uint16 PrZ_IsAPSieveABCD   : 1;
		uint16 PrZ_IsNewPGen       : 1;
		uint16 PrZ_IsMultiSection  : 1;			// WIP
		uint16 PrZ_reservedBits    : 11;
//	};
	uint32 PrZ_nvalsleft;

	// For multi-section files
	PrZ_SubFileSizes_t PrZ_SubFileSizes;

	uint8  Prz_reserved0;   // for alignment and reserve
	uint16 PrZ_reserved1;	// for alignment and reserve
};

struct PrZ_SubFile_Header		// WIP
{
	uint32 PrZ_FSeek;		// In the file, may actually be from 1 to 4 bytes (depends upon PrZ_File_Header.PrZ_SubFileSizes.Prz_FseekSize)    A value of -1 (all bits set) means this is an EOF marker (the section does not actually exist) Otherwise, it is a "relative" offset to the next section header.
	uint32 PrZ_OffKadj;		// In the file, may actually be from 1 to 4 bytes (depends upon PrZ_File_Header.PrZ_SubFileSizes.PrZ_OffKadjSize)
	uint32 PrZ_Nadj;		// In the file, may actually be from 0 to 3 bytes (depends upon PrZ_File_Header.PrZ_SubFileSizes.PrZ_NadjSize)     A value of 0 means it DOES not exist (only exists in FermFact/NewPGen files)
	uint32 PrZ_nvalsleft;	// In the file, may actually be from 1 to 4 bytes (depends upon PrZ_File_Header.PrZ_SubFileSizes.PrZ_nvalsleftSize)
	PrZ_SubFile_Header();
};

class PrZ_Section_Header_Base
{
	public:
		PrZ_Section_Header_Base(FILE *in);							// When "read" from a PrZ file
		PrZ_Section_Header_Base(const char *pLine1, int nLine1Len, FILE *in, bool bIsABCD);	// When "read" from an ABCD file (to create a PrZ)
		virtual ~PrZ_Section_Header_Base();
		const char *PrZ_GetFirstLine();
		virtual void WriteSection(FILE *out) = 0;
		virtual bool GetValues(uint64 &MinK, uint64 &MaxK, uint64 &nVals, uint64 &MaxPR)=0;

		uint64 KOffset();
		uint32 virtual getPrZ_Base();

	protected:
		uint64 PrZ_OffsetK;
		char *m_cpFirstLine;
};

//            n         OffK                      max_pr             nvals       MinK     MaxK       n    n
// ABCD $a*2^2000+1 [16777237] // FFact {prime,1912060467253}{vals,00000000}{k,16777215,31999999){n,2000,2000}
class PrZ_FermFact_Section_Header : public PrZ_Section_Header_Base
{
	uint32 PrZ_n;
	uint64 PrZ_MinK;
	uint64 PrZ_MaxK;
	uint64 PrZ_max_pr;

	// Not written
	uint32 PrZ_nvalsleft;

	public:
		PrZ_FermFact_Section_Header(FILE *in, uint32 nValsLeft);					// When "read" from a PrZ file
		PrZ_FermFact_Section_Header(const char *pLine1, int nLine1Len, FILE *in);	// When "read" from an ABCD file (to create a PrZ)
		bool GetValues(uint64 &MinK, uint64 &MaxK, uint64 &nVals, uint64 &MaxPR);
		void WriteSection(FILE *out);
};

//         expr           OffK                             Max_pr           MaxK
// ABCD $a.23#+111270041 [7] // APSieveV2 Sieved to: 00000000001321730047,[3200000000]
class PrZ_APSieve_Section_Header : public PrZ_Section_Header_Base
{
	uint64 PrZ_max_pr;
	uint64 PrZ_MaxK;
	char   *PrZ_expr;

	// Not written
	uint32 PrZ_nvalsleft;

	public:
		PrZ_APSieve_Section_Header(FILE *in, uint32 nValsLeft);							// When "read" from a PrZ file
		PrZ_APSieve_Section_Header(const char *pLine1, int nLine1Len, FILE *in);	// When "read" from an ABCD file (to create a PrZ)
		~PrZ_APSieve_Section_Header();
		bool GetValues( uint64 &MinK, uint64 &MaxK, uint64 &nVals, uint64 &MaxPR);
		void WriteSection(FILE *out);
};

class PrZ_NewPGen_Section_Header : public PrZ_Section_Header_Base
{
	uint32 PrZ_Base;
	uint64 PrZ_MaxK;

	// Not written
	uint32 PrZ_nvalsleft;

	public:
		PrZ_NewPGen_Section_Header(FILE *in, uint32 nValsLeft);							// When "read" from a PrZ file
		PrZ_NewPGen_Section_Header(const char *pLine1, int nLine1Len, FILE *in, bool &bSkipEvens);	// When "read" from an ABCD file (to create a PrZ)
		~PrZ_NewPGen_Section_Header();
		bool GetValues(uint64 &MinK, uint64 &MaxK, uint64 &nVals, uint64 &MaxPR);
		uint32 getPrZ_Base();
		void WriteSection(FILE *out);
};

class PrZ_Generic_Section_Header : public PrZ_Section_Header_Base
{
	// Not written
	uint32 PrZ_nvalsleft;

	public:
		PrZ_Generic_Section_Header(FILE *in, uint32 nValsLeft);							// When "read" from a PrZ file
		PrZ_Generic_Section_Header(const char *pLine1, int nLine1Len, FILE *in, bool &bSkipEvens);	// When "read" from an ABCD file (to create a PrZ)
		~PrZ_Generic_Section_Header();
		bool GetValues(uint64 &MinK, uint64 &MaxK, uint64 &nVals, uint64 &MaxPR);
		void WriteSection(FILE *out);
};

#pragma pack()

#endif // !defined (__PrZ__H__)
