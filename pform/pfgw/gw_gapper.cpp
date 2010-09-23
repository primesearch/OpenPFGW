#if defined(_MSC_VER)

#include "primeformpch.h"

#include <signal.h>
#include "../../pform/pfio/pfini.h"
#include "../../pform/pfio/pfprzfile.h"

#include "pfgw_globals.h"

static int  base=2;
static bool bOddBase=false;
static uint64 MaxNum;      // The maximum k read in from the file
static uint64 MinNum;      // The minimum k read in from the file
static uint32 nvals;    // The number of values read in from the file
static uint64 nvalsleft;   // The number remaining after we remove entries
static uint32 nvalue;      // Value of n (for k.2^n+/-1) read in from the file
static uint32 *_pMap, *pMap;
static int NumPrimes;
static unsigned Count = 0, Cnt1=0, Cnt2=0;
static clock_t start;
static uint64 mid_gap, Low_end, Hi_end;
#if defined (_MSC_VER)
static uint32 WaitTil;
#endif


// Code from Gapper.exe but uses PFGW's GW integers.  NOTE that a very minimal
// read-only subset of the CPAP file handling routines is included.

// Added support for NewPGen format (in a PrZ saved file), since NewPGen now sieves faster than CPAPSieve.

#include "../../pform/pflib/bmap.cxx"

static bool LoadFile(const char *Fname);
static uint64 FindPriorPrime(uint64 i);
static uint64 FindNextPrime(uint64 i);

// Stripped down gwPrp WITHOUT any screen output.  (also without any error checking, or WIP saving.
bool IsPrp(Integer *N)
{
   bool bRetval=false;
   Integer X = (*N);
   --X;           // X is the exponent, we are to calculate 3^X mod N

   uint32 iTotal=lg(X);

   // if iTotal is less than 1000, then use GMP to do exponentaion (we need to work out the exact cutoff's, and
   // different reduction methods will also "change" this"
#if defined (_MSC_VER)
   if (iTotal < 250)
#else
   if (iTotal < 1000)
#endif
   {
      if (!g_bGMPMode)
      {
         g_bGMPMode = true;
         PFPrintfLog ("Switching to Exponentiating using GMP\n");
      }
      Integer b(iBase);
      // This is the "raw" gmp exponentiator.  It if pretty fast up to about 500 digits or so.
      X = powm(b,X,*N);
      if (X == 1)
         return true;
      return false;
   }
   if (g_bGMPMode)
   {
      g_bGMPMode = false;
      PFPrintfLog ("Switching to Exponentiating using Woltman FFT's\n");
   }

   // create a context
   gwinit2(&gwdata, sizeof(gwhandle), GWNUM_VERSION);

   gwsetmaxmulbyconst(&gwdata, iBase); // maximum multiplier
   if (CreateModulus(N)) return false;

   // everything with a GWInteger has a scope brace, so that
   // GWIntegers are destroyed before the context they live in
   {
      // prepare the gw buffers we need
      GWInteger gwX;

      // I think we're ready to go, let's do it.
      gwX=iBase;                       // initialise X to A^1.
      gwsetmulbyconst(&gwdata, iBase);    // and multiplier

      for(;iTotal--;)
      {
         gwsetnormroutine(&gwdata,0,g_bErrorCheckAllTests,bit(X,iTotal));
         gwsquare(gwX);
      }
      Integer XX;
      XX = gwX;
      if(XX==1)
         bRetval=true;
   }
   // zap the gw
   DestroyModulus();
   return bRetval;
}

void gw_gapper(const char *FName, int gap, uint64 i)
{
   if (!gap)
   {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr ("Min Gap not entered\n");
      return;
   }
   if (!LoadFile(FName))
      return;

   if (i < MinNum)
      i = MinNum;
   if (i >= MaxNum)
   {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr("Sorry, no data above "ULL_FORMAT", so I am exiting now.\n", i);
      return;
   }

   if (bOddBase && (i & 1))
      i++;
   else if (!bOddBase && !(i & 1))
      i++;

#if defined (_MSC_VER)
   WaitTil = GetTickCount()-1;
#endif

   start = clock();

   for (Low_end = i; Low_end < MaxNum; Low_end += 2)
   {
      if (!IsBitSet(pMap, Low_end))
         continue;
      Cnt1++;

      // Ok now we have found the "possible" number.  Check to see if it is prp-3
      // Assign original "base" N value to Temp
      char Format[120];
      sprintf(Format, "%d^%d+"ULL_FORMAT"", base, nvalue, Low_end);
      Integer n;
      n.Ipow(base, nvalue);
      n += Low_end;

      if (!IsPrp(&n))
         // the ONLY thing we try to do here is to find the first prime.  After that, we jump
         // into the while loop below, which is stayed in from that point on.
         continue;

      // Ok, once we get here, we NEVER leave (until we run out of file, or until the user exits.
      while(Low_end < MaxNum)
      {
         Hi_end = Low_end + gap-2;

         // It is PRP
         Count++;
         // Ok, now start at Low_end+gap

         if (Low_end+gap > MaxNum)
         {
            // We are done processing the full file.
            Low_end = MaxNum;
            break;
         }
         mid_gap = FindPriorPrime(Low_end+gap-2);
         if (mid_gap == Low_end)
         {
            if (gap > 50000)
            {
               double duration = (double)(clock() - start) / CLOCKS_PER_SEC;
               PFPrintfStderr("\rT#/C#/Gaps %u/%u/%u L="ULL_FORMAT" C="ULL_FORMAT" H="ULL_FORMAT" %0.3f/s\n", Cnt1, Cnt2, Count, Low_end, mid_gap, Hi_end, Cnt1/duration);
            }

            // Found a BIG gap.
            mid_gap = FindNextPrime(Low_end+gap);
            if (mid_gap == 0)
            {
               // We are done processing the full file.
               Low_end = MaxNum;
               PFPrintfLog("%70.70s\r** Found gap of at least "ULL_FORMAT" at %d^%d+"ULL_FORMAT" to %d^%d+"ULL_FORMAT". but ran out of file\n", " ", MaxNum-Low_end-2, base, nvalue, Low_end, base, nvalue, MaxNum-2);
               break;
            }
            PFPrintfLog("%70.70s\r** Found gap of "ULL_FORMAT" at %d^%d+"ULL_FORMAT" to %d^%d+"ULL_FORMAT"\n", " ", mid_gap-Low_end, base, nvalue, Low_end, base, nvalue, mid_gap);
         }
         Low_end = mid_gap;

#if defined (_MSC_VER)
         if (GetTickCount() > WaitTil)
#endif
         {
            double duration = (double)(clock() - start) / CLOCKS_PER_SEC;
            PFPrintfStderr("\rT#/C#/Gaps %u/%u/%u L="ULL_FORMAT" C="ULL_FORMAT" H="ULL_FORMAT" %0.3f/s\r", Cnt1, Cnt2, Count, Low_end, mid_gap, Hi_end, Cnt1/duration);
#if defined (_MSC_VER)
            WaitTil = GetTickCount() + 10000;
#endif
         }
      }
   }
}

uint64 FindNextPrime(uint64 i)
{
   Integer n;
   n.Ipow(base, nvalue);

   for (; i < MaxNum; i += 2)
   {
      if (!IsBitSet(pMap, i))
         continue;
      // Ok now we have found the "possible" number.  Check to see if it is prp-3
      // Assign original "base" N value to Temp

      Cnt1++;
      Cnt2++;

      Integer N(n);
      N += i;

      if (IsPrp(&N))
      {
         Cnt2 = 0;
         return i;
      }
      ClearBit(pMap, i);
#if defined (_MSC_VER)
      if (GetTickCount() > WaitTil)
      {
#endif
         double duration = (double)(clock() - start) / CLOCKS_PER_SEC;
         PFPrintfStderr("\rT#/C#/Gaps %u/%u/%u L="ULL_FORMAT" C="ULL_FORMAT"** H="ULL_FORMAT" %0.3f/s\r", Cnt1, Cnt2, Count, Low_end, i, Hi_end, Cnt1/duration);
#if defined (_MSC_VER)
         WaitTil = GetTickCount() + 10000;
      }
#endif
   }
   Cnt2 = 0;
   return 0;
}

uint64 FindPriorPrime(uint64 i)
{
   Cnt2 = 0;
   // Note there MUST be a prime before the passed in i  (and there will always be a prime before)
   Integer n;
   n.Ipow(base, nvalue);

   for (; /*i > MinNum*/; i -= 2)
   {
      if (!IsBitSet(pMap, i))
         continue;
      // Ok now we have found the "possible" number.  Check to see if it is prp-3
      // Assign original "base" N value to Temp

      Cnt1++;
      Cnt2++;
      Integer N(n);
      N += i;
      if (IsPrp(&N))
         return i;

      ClearBit(pMap, i);
#if defined (_MSC_VER)
      if (GetTickCount() > WaitTil)
      {
#endif
         double duration = (double)(clock() - start) / CLOCKS_PER_SEC;
         PFPrintfStderr("\rT#/C#/Gaps %u/%u/%u L="ULL_FORMAT" C="ULL_FORMAT" H="ULL_FORMAT" %0.3f/s\r", Cnt1, Cnt2, Count, Low_end, i, Hi_end, Cnt1/duration);
#if defined (_MSC_VER)
         WaitTil = GetTickCount() + 10000;
      }
#endif

   }
   // will NEVER get here.
   return 0;
}

// NOTE for PFGW, only reading of the files is supported.  Compression
// and creation of the JFCPAP files is not part of this source release.

// Work buffer, so we don't have to write/read from the disk too frequently
uint8 RLE_Buf[0xF000];
// Counters to tell us where in the buffer the reads/writes are taking place.
uint32  RLE_BufCnt, RLE_BufInCnt;

static int _5Cnt;
static uint8 _5Byte[5];

// Buffered read of a byte.
inline uint32 Comp_GetByte_(FILE *fp)
{
   if (RLE_BufCnt == RLE_BufInCnt)
   {
      RLE_BufCnt = (uint32) fread(RLE_Buf, 1, sizeof(RLE_Buf), fp);
      RLE_BufInCnt = 0;
      if (!RLE_BufCnt && feof(fp))
         return 0;
   }
   return RLE_Buf[RLE_BufInCnt++];
}

inline void Reset_Comp_GetByte()
{
   RLE_BufCnt = RLE_BufInCnt = 0;
}

inline uint32 Comp_Get5Bits(FILE *fp)
{
   static int _5CntI;

   switch(_5CntI++)
   {
      case 0:
         // our buffer is empty, so load the next 5 byte block of 8 5-bit chunks.
         _5Byte[0] = (uint8)Comp_GetByte_(fp);
         _5Byte[1] = (uint8)Comp_GetByte_(fp);
         _5Byte[2] = (uint8)Comp_GetByte_(fp);
         _5Byte[3] = (uint8)Comp_GetByte_(fp);
         _5Byte[4] = (uint8)Comp_GetByte_(fp);

         return (_5Byte[0]&0x1F);
      case 1:
         return ( (_5Byte[0]>>5) | ((_5Byte[1]&3)<<3) );
      case 2:
         return ( (_5Byte[1]>>2)&0x1F );
      case 3:
         return ( (_5Byte[1]>>7) | ((_5Byte[2]&0xF)<<1) );
      case 4:
         return ( (_5Byte[2]>>4) | ((_5Byte[3]&1)<<4) );
      case 5:
         return ( (_5Byte[3]>>1)&0x1F );
      case 6:
         return ( (_5Byte[3]>>6) | ((_5Byte[4]&7)<<2) );
      case 7:
         _5CntI = 0;
         return ( (_5Byte[4]>>3) );
   }
   return 0x1F;
}


bool DeCompress_5bits_skip_evens(FILE *fp)
{
   RLE_BufCnt = RLE_BufInCnt = 0;
   int BitCarry=0;
   uint32 Bit = 0;
   uint32 NumBytes = (uint32)((MaxNum-MinNum+7)/8)+1;
   uint32 *pMap = &_pMap[5];

   if (!(MinNum & 1))
      Bit++;

   memset(&pMap[-2], 0, (NumBytes>>1)+16);

   for (uint32 i = 0; i < NumBytes;)
   {
      uint32 Zeros = Comp_Get5Bits(fp);
      while (Zeros == 0x1F)
      {
         Bit += (0x1F<<1);
         i += (((0x1F<<1)+BitCarry)/8);
         BitCarry = ((0x1F<<1)+BitCarry) % 8;
         Zeros = Comp_Get5Bits(fp);
         if (i > NumBytes)
            return true;
      }
      Zeros <<= 1;
      Bit += Zeros + 1;
      SetBit(pMap, Bit);
      Bit++;
      i += ( (2+Zeros+BitCarry)/8);
      BitCarry = (2+Zeros+BitCarry) % 8;
   }
   return true;
}

bool IsValid_CPAP_SIG(const char *Line, bool &bIsOld32bitCPAPFileFormat)
{
   bIsOld32bitCPAPFileFormat=false;
   if (Line[7] != '\x1A' || (memcmp(Line, "JFCPap", 6) && memcmp(Line, "JFCap", 5) && memcmp(Line, "JFCAP", 5)))
      return false;
   if (Line[6] != '\n')
   {
      fprintf (stderr, "UNSUPPORTED!!! JFCAPv%c is no longer supported!\n", Line[6]);
      exit(0);
   }
   if (memcmp(Line, "JFCPap", 6) && memcmp(Line, "JFCap", 5))
      bIsOld32bitCPAPFileFormat=true;
   return true;
}

bool Read_JFCPap_File(const char *FName, uint64 *p)
{
   // Got a CPAP file, read it as such.
   FILE *in = fopen(FName, "rb");
   if (!in)
      return !fprintf(stderr, "Can't open file %s\n", FName);
   char Line[36+8];
   fread(Line, 1, 36+8, in);
   bool bIsOld32bitCPAPFileFormat;
   if (!IsValid_CPAP_SIG(Line, bIsOld32bitCPAPFileFormat))
   {
      fprintf(stderr, "File %s is not a valid JFCPap file\n", FName);
      fclose(in);
      return false;
   }

   // Note file is saved in Intel (little endian) format.  It is stored ONLY in the
   // 5 bit rle compression method.

   if (bIsOld32bitCPAPFileFormat)
   {
      fseek(in, -8, SEEK_CUR);   // the header was only the first 36 bytes, so move the pointer back.
      nvalue = *(uint32*)&Line[8];
      nvalsleft = *(uint32*)&Line[12];
      MinNum = (uint64)(*(uint32*)&Line[16]);   // buggy older format.  Only allowed min-max k's to store to 2^32, even though the program worked higher.
      MaxNum = (uint64)(*(uint32*)&Line[20]);
      base   = *(uint32*)&Line[24];
      *p     = *(uint64*)&Line[28];
   }
   else
   {
      nvalsleft = *(uint32*)&Line[8];
      base   = *(uint32*)&Line[12];
      nvalue = *(uint32*)&Line[16];
      MinNum = *(uint64*)&Line[20];
      MaxNum = *(uint64*)&Line[28];
      *p     = *(uint64*)&Line[36];
   }
   Init_pMap(MinNum, MaxNum, &_pMap, &pMap);
   DeCompress_5bits_skip_evens(in);    // bit compressed (bit map skips even numbers)

   fclose(in);

// fprintf (stderr, "File %s loaded.  %I64u values left, sieved to: %I64d\n", FName, nvalsleft, *p);

   return true;
}

bool LoadFile(const char *Fname)
{
   PFOutput::EnableOneLineForceScreenOutput();
   PFPrintfStderr("Loading data from JFCPAP file '%s' ...", Fname);
   FILE *in = fopen(Fname, "rb");
   if (!in)
   {
      PFOutput::EnableOneLineForceScreenOutput();
      PFPrintfStderr ( " Error loading file %s", Fname);
      return false;
   }
   char Line[256];
   // FIRST, check for a CAP file, and if found we have a VERY easy load ;)
   fread(Line, 1, 256, in);
   fclose(in);
   bool bIsOld32bitCPAPFileFormat;
   if (IsValid_CPAP_SIG(Line, bIsOld32bitCPAPFileFormat))
   {
      uint64 p;
      fprintf (stderr, "\n");
      if (!Read_JFCPap_File(Fname, &p))
      {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr ( "\n\nerror, invalid JFCAP file\n");
         return false;
      }
      if (nvalsleft > 0x7FFFFFFF)
      {
         PFOutput::EnableOneLineForceScreenOutput();
         PFPrintfStderr ("Error, too many primes for this program!!!\n");
         return false;
      }
      NumPrimes = (int)nvalsleft;
      bOddBase = base & 1;
      return true;
   }

   // Is this a NewPGen saved file (in PrZ format)
   if (!strncmp(Line, "PrZ", 3))
   {
      // Ok, it is possible the right kind of file.

      PrZ_File_Header *pHead = (PrZ_File_Header *)Line;
      if (pHead->PrZ_IsNewPGen)
      {
         bool bIsRightKind = false;
         char *cp = Line;
         while (cp < &Line[256] && !bIsRightKind)
         {
            cp = (char *)memchr(cp, ':', sizeof(Line)-(cp-Line));
            if (cp)
            {
               ++cp;
               if (!strncmp(cp, "A:0:", 4))
               {
                  // Found it.
                  cp += 4;
                  base = atoi(cp);
                  while (isdigit(*cp))
                     ++cp;
                  if (!strncmp(cp, ":512", 4))
                     bIsRightKind = true;
               }
            }
         }
         const char *ccp;
         try
         {
            PFSimpleFile *pf = new PFPrZ_newpgen_File (Fname);
            pf->SecondStageConstruction(NULL);
            PFPrZ_newpgen_File *pFile = (PFPrZ_newpgen_File *)pf;

            nvalsleft = pFile->nValsLeft();
            nvalue = *(uint32*)&Line[16];
            MinNum = pFile->MinNum();
            MaxNum = pFile->MaxNum();
            MinNum = MinNum * 2 -1;
            MaxNum = MaxNum * 2 -1;
            bOddBase = base & 1;

            Init_pMap(MinNum, MaxNum, &_pMap, &pMap);

            uint32 NumBytes = (uint32)((MaxNum-MinNum+7)/8)+1;
            memset(&pMap[-2], 0, (NumBytes>>1)+16);

            PFString s;

            pFile->GetNextLine(s);
            ccp = (const char *)s;
            ccp = strchr(ccp, '^');
            if (!ccp)
            {
               // Not a CAP file, assume it is a file of numbers.
               PFOutput::EnableOneLineForceScreenOutput();
               PFPrintfStderr ("Error, this program requires a or Prz compressed NewPGen file\n");
               return false;
            }
            ++ccp;
            nvalue = atoi(ccp);
            do
            {
               ccp = (const char *)s;
               ccp = strchr(ccp, '*');
               if (ccp)
               {
                  ++ccp;
                  SetBit(pMap, (atoi(ccp)<<1)-1);
               }
            }
            while (pFile->GetNextLine(s) == PFSimpleFile::e_ok);
            delete pf;
         }
         catch(...)
         {
            printf ("");
         }
      }
      return true;
   }


   // Not a CAP file, assume it is a file of numbers.
   PFOutput::EnableOneLineForceScreenOutput();
   PFPrintfStderr ("Error, this program requires a or Prz compressed NewPGen file\n");
   return false;
}

#endif
