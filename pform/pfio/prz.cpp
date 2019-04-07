#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pfiopch.h"
#include "prz.h"
#include "cmpr_x_bit.cxx"


PrZ_File_Header::PrZ_File_Header()
{
   memset(this, 0, sizeof(*this));
}
PrZ_SubFileSizes_t::PrZ_SubFileSizes_t()
{
   *((uint8_t*)this) = 0;
}
PrZ_SubFile_Header::PrZ_SubFile_Header()
   : PrZ_FSeek(0),PrZ_OffKadj(0),PrZ_Nadj(0),PrZ_nvalsleft(0)
{ }

// protected:
//    uint32_t PrZ_Next_Section_Offset;   // SEEK_SET offset to the next header
//    void *p_Bits;
//    uint64_t PrZ_OffsetK;

// When "read" from a PrZ file
PrZ_Section_Header_Base::PrZ_Section_Header_Base(FILE *in)
{
   fread(&PrZ_OffsetK, 1, 8, in);
   m_cpFirstLine = 0;
}

// When "read" from an ABCD file (to create a PrZ)
PrZ_Section_Header_Base::PrZ_Section_Header_Base(const char *pLine1, int /*nLine1Len*/, FILE * /*in*/, bool bIsABCD)
{
   m_cpFirstLine = new char [strlen(pLine1)+1];
   strcpy(m_cpFirstLine, pLine1);
   strtok(m_cpFirstLine, "\r\n");
   if (bIsABCD)
   {
      const char *cp = strchr(pLine1, '[');
      if (!cp)
         throw "Error, can't find the '[' char of the ABCD file\n";
      PrZ_OffsetK = _atoi64(&cp[1]);
   }
   else
      PrZ_OffsetK = 0;
}

/*virtual*/
PrZ_Section_Header_Base::~PrZ_Section_Header_Base()
{
   delete[] m_cpFirstLine;
}
uint64_t PrZ_Section_Header_Base::KOffset()
{
   return PrZ_OffsetK;
}
/*virtual*/
uint32_t PrZ_Section_Header_Base::getPrZ_Base()
{
   return 0;
}
const char *PrZ_Section_Header_Base::PrZ_GetFirstLine()
{
   return m_cpFirstLine;
}

/*pure virtual*/
void PrZ_Section_Header_Base::WriteSection(FILE *out)
{
   fwrite(&PrZ_OffsetK, 1, 8, out);
}

//            n         OffK                      max_pr             nvals       MinK     MaxK       n    n
// ABCD $a*2^2000+1 [16777237] // FFact {prime,1912060467253}{vals,00000000}{k,16777215,31999999){n,2000,2000}
// uint32_t PrZ_n;
// uint64_t PrZ_MinK;
// uint64_t PrZ_MaxK;
// uint64_t PrZ_nvalsleft;
// uint64_t PrZ_max_pr;

// When "read" from a PrZ file
PrZ_FermFact_Section_Header::PrZ_FermFact_Section_Header(FILE *in, uint32_t nValsLeft)
: PrZ_Section_Header_Base(in), PrZ_nvalsleft(nValsLeft)
{
   fread(&PrZ_n, 1, 4, in);
   fread(&PrZ_MinK, 1, 8, in);
   fread(&PrZ_MaxK, 1, 8, in);
   fread(&PrZ_max_pr, 1, 8, in);
   m_cpFirstLine = new char[500];
   sprintf (m_cpFirstLine, "ABCD $a*2^%u+1 [%" PRIu64"] // FFact {prime,%" PRIu64"}{vals,%u}{k,%" PRIu64",%" PRIu64"){n,%u,%u}",
      PrZ_n, PrZ_OffsetK, PrZ_max_pr, nValsLeft, PrZ_MinK, PrZ_MaxK, PrZ_n, PrZ_n);
}

// When "read" from an ABCD file (to create a PrZ)
PrZ_FermFact_Section_Header::PrZ_FermFact_Section_Header(const char *pLine1, int nLine1Len, FILE *in)
: PrZ_Section_Header_Base(pLine1, nLine1Len, in, true)
{
   uint64_t xx;
   uint32_t x;
   sscanf(pLine1, "ABCD $a*2^%u+1 [%" PRIu64"] // FFact {prime,%" PRIu64"}{vals,%u}{k,%" PRIu64",%" PRIu64"){n,%u,%u}",
      &PrZ_n, &xx, &PrZ_max_pr, &PrZ_nvalsleft, &PrZ_MinK, &PrZ_MaxK, &x, &x);
}

bool PrZ_FermFact_Section_Header::GetValues(uint64_t &MinK, uint64_t &MaxK,uint64_t &nVals, uint64_t &MaxPR)
{
   MaxPR = PrZ_max_pr;
   MinK = PrZ_MinK;
   MaxK = PrZ_MaxK;

   nVals = PrZ_nvalsleft;

   return true;
}

void PrZ_FermFact_Section_Header::WriteSection(FILE *out)
{
   // write out the data for the base class
   PrZ_Section_Header_Base::WriteSection(out);
   // Now write out this class's data
   fwrite(&PrZ_n, 1, 4, out);
   fwrite(&PrZ_MinK, 1, 8, out);
   fwrite(&PrZ_MaxK, 1, 8, out);
   fwrite(&PrZ_max_pr, 1, 8, out);
}


//         expr           OffK                             Max_pr           MaxK
// ABCD $a.23#+111270041 [7] // APSieveV2 Sieved to: 00000000001321730047,[3200000000]
// uint64_t PrZ_max_pr;
// uint64_t PrZ_MaxK;
// char *PrZ_expr;

// When "read" from a PrZ file
PrZ_APSieve_Section_Header::PrZ_APSieve_Section_Header(FILE *in, uint32_t nValsLeft)
: PrZ_Section_Header_Base(in), PrZ_nvalsleft(nValsLeft)
{
   fread(&PrZ_max_pr, 1, 8, in);
   fread(&PrZ_MaxK, 1, 8, in);
   char Buf[5000], *cp = Buf;
   fread(cp, 1, 1, in);
   while (*cp)
   {
      ++cp;
      fread(cp, 1, 1, in);
   }
   m_cpFirstLine = new char[strlen(Buf)+1];
   strcpy(m_cpFirstLine, Buf);
   PrZ_expr = new char[strlen(Buf)+1];
   strcpy(PrZ_expr, Buf);
}

// When "read" from an ABCD file (to create a PrZ)
PrZ_APSieve_Section_Header::PrZ_APSieve_Section_Header(const char *pLine1, int nLine1Len, FILE *in)
: PrZ_Section_Header_Base(pLine1, nLine1Len, in, true)
{
   char Buf[500];
   uint64_t xx;
   sscanf(pLine1, "ABCD $a.%s [%" PRIu64"] // APSieveV2 Sieved to: %" PRIu64",[%" PRIu64"]",
      Buf, &xx, &PrZ_max_pr,  &PrZ_MaxK);
   PrZ_expr = new char[strlen(Buf)+1];
   strcpy(PrZ_expr, Buf);
}

PrZ_APSieve_Section_Header::~PrZ_APSieve_Section_Header()
{
   delete[] PrZ_expr;
}

bool PrZ_APSieve_Section_Header::GetValues(uint64_t &MinK, uint64_t &MaxK,uint64_t &nVals, uint64_t &MaxPR)
{
   MaxPR = PrZ_max_pr;
   MinK = PrZ_OffsetK;
   MaxK = PrZ_MaxK;

   nVals = PrZ_nvalsleft;

   return true;
}

void PrZ_APSieve_Section_Header::WriteSection(FILE *out)
{
   // write out the data for the base class
   PrZ_Section_Header_Base::WriteSection(out);
   // Now write out this class's data
   fwrite(&PrZ_max_pr, 1, 8, out);
   fwrite(&PrZ_MaxK, 1, 8, out);
   fwrite(PrZ_expr, 1, strlen(PrZ_expr)+1, out);
}


//   Max_pr        OffK
//998736546:T:0:2:67
//1006 101                OffK  and Prz_Base on second line.
//1037 101
//...
//9999998 101         MaxK on last line.

// When "read" from a PrZ file
PrZ_NewPGen_Section_Header::PrZ_NewPGen_Section_Header(FILE *in, uint32_t nValsLeft)
: PrZ_Section_Header_Base(in), PrZ_nvalsleft(nValsLeft)
{
   char Buf[5000], *cp = Buf;

   fread(&PrZ_Base, 1, 4, in);
   fread(&PrZ_MaxK, 1, 8, in);

   fread(cp, 1, 1, in);
   while (*cp)
   {
      ++cp;
      fread(cp, 1, 1, in);
   }
   m_cpFirstLine = new char[strlen(Buf)+1];
   sprintf (m_cpFirstLine, "%s", Buf);
}

// When "read" from an ABCD file (to create a PrZ)
PrZ_NewPGen_Section_Header::PrZ_NewPGen_Section_Header(const char *pLine1, int nLine1Len, FILE *in, bool &_bSkipEvens)
: PrZ_Section_Header_Base(pLine1, nLine1Len, in, false)
{
   // Newpgen check
   char c;
   int len, base, bits;
   uint64_t pr;
   int count = sscanf(m_cpFirstLine, "%" PRIu64":%c:%d:%d:%d", &pr, &c, &len, &base, &bits);
   if (count != 5)
      throw "Error, NOT a NewPGen file!!!!\n";
   char Buf[500];
   fgets(Buf, sizeof(Buf), in);

   sscanf(Buf, "%" PRIu64" %u", &PrZ_OffsetK, &PrZ_Base);

   // unknown at this time.
   PrZ_nvalsleft = 0;
   bool bOdd = !!(PrZ_OffsetK & 1);
   _bSkipEvens = true;
   for (int j = 0; !feof(in) && j < 150 && _bSkipEvens; ++j)
   {
      fgets(Buf, sizeof(Buf), in);
      bool bodd = !!(_atoi64(Buf) & 1);
      if (bodd && !bOdd)
         _bSkipEvens = false;
      if (!bodd && bOdd)
         _bSkipEvens = false;
   }

   fseek(in, -60, SEEK_END);
   fgets(Buf, sizeof(Buf), in);
   while (!feof(in))
      fgets(Buf, sizeof(Buf), in);
   sscanf(Buf, "%" PRIu64" %u", &PrZ_MaxK, &PrZ_Base);

   // We must leave the file in it's original state (i.e. the first line had been read.
   fseek(in, 0, SEEK_SET);
   fgets(Buf, sizeof(Buf), in);
}

PrZ_NewPGen_Section_Header::~PrZ_NewPGen_Section_Header()
{
}

bool PrZ_NewPGen_Section_Header::GetValues(uint64_t &MinK, uint64_t &MaxK,uint64_t &nVals, uint64_t &MaxPR)
{
   sscanf(m_cpFirstLine, "%" PRIu64":", &MaxPR);
   MinK = PrZ_OffsetK;
   MaxK = PrZ_MaxK;

   nVals = PrZ_nvalsleft;

   return true;
}

void PrZ_NewPGen_Section_Header::WriteSection(FILE *out)
{
   // write out the data for the base class
   PrZ_Section_Header_Base::WriteSection(out);
   // Now write out this class's data
   fwrite(&PrZ_Base, 1, 4, out);
   fwrite(&PrZ_MaxK, 1, 8, out);
   fwrite(m_cpFirstLine, 1, strlen(m_cpFirstLine)+1, out);
}

/* Uniq function to NewPGen, it is vitual and returns 0 for all but NewPGen */
uint32_t PrZ_NewPGen_Section_Header::getPrZ_Base()
{
   return PrZ_Base;
}

   /* move this above the other items, yet end the structure after this (when minimzing the header size). */
   /* This PrZ_FirstLine is a NULL terminated string */
// char *PrZ_FirstLine;

// When "read" from a PrZ file
PrZ_Generic_Section_Header::PrZ_Generic_Section_Header(FILE *in, uint32_t nValsLeft)
: PrZ_Section_Header_Base(in), PrZ_nvalsleft(nValsLeft)
{
   char Buf[50000], *cp = Buf;
   fread(cp, 1, 1, in);
   while (*cp && cp < &Buf[sizeof(Buf)-2])
   {
      ++cp;
      fread(cp, 1, 1, in);
   }
   *cp = 0;
   m_cpFirstLine = new char[strlen(Buf)+1];
   strcpy(m_cpFirstLine, Buf);
}

// When "read" from an ABCD file (to create a PrZ)
PrZ_Generic_Section_Header::PrZ_Generic_Section_Header(const char *pLine1, int nLine1Len, FILE *in, bool &_bSkipEvens)
: PrZ_Section_Header_Base(pLine1, nLine1Len, in, true)
{
   char Buf[500];
   fgets(Buf, sizeof(Buf), in);
   PrZ_nvalsleft = 0;

   bool bOdd = !!(_atoi64(Buf) & 1);
   _bSkipEvens = true;
   for (int j = 0; !feof(in) && j < 150 && _bSkipEvens; ++j)
   {
      fgets(Buf, sizeof(Buf), in);
      bool bodd = !!(_atoi64(Buf) & 1);
      if (bodd && !bOdd)
         _bSkipEvens = false;
      if (!bodd && bOdd)
         _bSkipEvens = false;
   }
   // We must leave the file in it's original state (i.e. the first line had been read.
   fseek(in, 0, SEEK_SET);
   fgets(Buf, sizeof(Buf), in);
}


PrZ_Generic_Section_Header::~PrZ_Generic_Section_Header()
{
}

bool PrZ_Generic_Section_Header::GetValues(uint64_t &MinK, uint64_t &MaxK, uint64_t &nVals, uint64_t &MaxPR)
{
   nVals = PrZ_nvalsleft;
   MinK = PrZ_OffsetK;
   MaxK = (uint64_t)(int64_t)-1;
   MaxPR = 3;
   return false;
}

void PrZ_Generic_Section_Header::WriteSection(FILE *out)
{
   // write out the data for the base class
   PrZ_Section_Header_Base::WriteSection(out);
   // Now write out this class's data
   fwrite(m_cpFirstLine, 1, strlen(m_cpFirstLine)+1, out);
}
