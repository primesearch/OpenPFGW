#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "primeformpch.h"
#include <signal.h>
#include "../../pform/pfio/pfini.h"

#include "pfgw_globals.h"

// Only save at most one time every 20 minutes.  That means that a set of numbers which require less than
// 20 minutes of time to complete will NEVER have resume files created, unless a ^C happens.
static clock_t NextSave;

#define SAVE_TIMEOUT_MINUTES 20

#define SAVE_SIGNATURE "PFGW Restore V3.3.6\r\n\x1A"
#define SAVE_SIGNATURE_LEN (strlen(SAVE_SIGNATURE)+1)

void CreateRestoreName(Integer *N, char RestoreName[13])
{
   // Neuter the function for now
// strcpy(RestoreName, "foobar");
   // Set the NextSave time
// NextSave = clock() + SAVE_TIMEOUT_MINUTES * 600000 * clocks_per_sec;
// return ;
   // end of Neuter the function for now

   // Restore Name is lower 40 bits of the number (in base-36).  This fits perfectly into 8 of an 8.3 file name.
   // The extension will become .frp  for PfgwSaveFile, but reversed.

   uint32 uRes = (*N) % 2147483629;  // 2^31-19 which is prime.  This give a pretty good 2^31 bit mix of file names
   uRes += (1u << 31);  // make sure that number is larger than 36^6 so that we get 7 characters

   // Now convert to a "string" in base 36
   const static char *Base = "0123456789abcdefghijklmnopqrstuvwxyz";

   int i;
   //for (i = 0; i < 8; ++i)
   for (i = 0; i < 7; ++i)
   {
      RestoreName[i] = Base[(uRes%36)];
      uRes /= 36;
   }
   strcpy(&RestoreName[i], ".pfr"); // PFgwRestore

   // Set the NextSave time
   NextSave = clock() + SAVE_TIMEOUT_MINUTES * 60 * clocks_per_sec;
}


bool CrcInit = false;
#define CRC_32 0xedb88320L    // CRC-32 polynomial
#define TABSIZE 256
uint32 crctab[TABSIZE];
inline uint32 updcrc32 (uint32 crc_val, unsigned char c)
{
   return crctab[(int)((crc_val)^(c))&0xff] ^ ((crc_val) >> 8);
}
inline void  updatecrcbuf (uint32 &crc_val, void *buf, int size)
{
   unsigned char *bp = (unsigned char*)buf;
   for (int i = 0; i < size; i++)
      crc_val = updcrc32 (crc_val, bp[i]);
}

static uint32 onecrc (int item)               // calculates CRC of one item
{
    uint32 accum = 0;
    item <<= 1;
    for (int i = 8; i > 0; i--)
    {
        item >>= 1;
        if ((item ^ accum) & 0x0001)
            accum = (accum >> 1) ^ CRC_32;
        else
            accum >>= 1;
    }
    return (accum);
}

static void mkcrctab (void)                 // Generates CRC table, calling onecrc() to make each term
{
    for (int i = 0; i < TABSIZE; i++)
        crctab[i] = onecrc (i);
}

void initCrc(uint32 &crc_val)
{
   if (!CrcInit)
   {
      CrcInit = true;
      mkcrctab ();
   }
   crc_val = 0xffffffff;
}

bool RestoreState(ePRPType ePRP, char *RestoreName, uint32 *iDone, GWInteger *gwX, uint32 _iBase, eContextType eCType)
{
   bool Ret = false;
   uint32 t_iDone;
   uint32 n;
   unsigned char c;
   unsigned char Buffer[256], *cp=Buffer;
   Integer N;
   uint32   bytelen;

   FILE *in = fopen(RestoreName, "rb");
   if (!in)
      return false;

   // Write the signature.  Keep this a "fixed" 24 bytes long.
   fread(Buffer, 1, SAVE_SIGNATURE_LEN, in);
   if (strncmp((char*)Buffer, SAVE_SIGNATURE, SAVE_SIGNATURE_LEN)) goto BailOut;

   // Try to do a few things to distinguish what type of CPU wrote the file.
   fread(&c, 1, 1, in);
   if (c != sizeof(uint32)) goto BailOut;

   // little endian or big endian marker.  Currently there is no code to handle conversion, and most likely
   // if this is run on a big endian, then the FFT will certainly be different, so it does not matter any way.
   fread(&n, 1, sizeof(n), in);
   if (n != 0x12345678) goto BailOut;

   // Read the the saved checksum
   uint32 crc_val, crc_stored;
   initCrc(crc_val);
   fread (&crc_stored, 1, sizeof(crc_stored), in);

   // read the length of the stored string.
   fread(&n, 1, sizeof(n), in);
   updatecrcbuf(crc_val, &n, sizeof(n));
   // Now read the "stored string"
   if (!n || n > sizeof(Buffer)) goto BailOut;
   fread(Buffer, 1, n, in);
   if (strncmp((char*)Buffer, g_cpTestString, n)) goto BailOut;
   updatecrcbuf(crc_val, Buffer, n);

   // now read the file data
   fread(Buffer, 1, 4 + 2*sizeof(uint32) +10*sizeof(uint32), in);

   if (*cp++ != ePRP) goto BailOut;
   if (*cp++ != eCType) goto BailOut;
   // skip the 2 reseved bytes
   cp+=2;
   t_iDone = *(uint32*)cp;
   cp += sizeof(t_iDone);
   if (*(uint32*)cp != _iBase) goto BailOut; // different base is a CRITICAL error
   cp += sizeof(_iBase);

   // Skip any reserved space.
   cp += 10*sizeof(uint32);

   updatecrcbuf(crc_val, Buffer, cp-Buffer);

   // Now read in the GWInteger.

   fread (&bytelen, 1, sizeof(bytelen), in);
   updatecrcbuf(crc_val, &bytelen, sizeof(bytelen));

   // This isn't a great call. This single bit sits in
   // a vast chunk of memory, just to force gmp to allocate.
   N = 1;
   N <<= bytelen * 8 - 1;

   fread (N.gmp()->_mp_d, 1, bytelen, in);
   updatecrcbuf(crc_val, N.gmp()->_mp_d, bytelen);

   *gwX = N;

   if (crc_val == crc_stored)
   {
      Ret = true;
      *iDone = t_iDone;
   }
BailOut:;
   fclose(in);
   return Ret;
}

bool SaveState(ePRPType ePRP, char *RestoreName, uint32 iDone, GWInteger *gwX, uint32 _iBase, eContextType eCType, Integer * /*N*/, bool bForce)
{
   // Neuter the function for now
// return false;
   // End of Neuter the function for now

   if (clock() < NextSave && !bForce)
      return false;

   // Set the NextSave time
   NextSave = clock() + SAVE_TIMEOUT_MINUTES * 60 * clocks_per_sec;

   uint32 str_len;
   unsigned char Buffer[16388], *cp=Buffer;
   FILE *out = fopen(RestoreName, "wb");
   if (!out)
      return false;

   // Write the signature.  Keep this a "fixed" 24 bytes long.
   fwrite(SAVE_SIGNATURE, 1, SAVE_SIGNATURE_LEN, out);

   // Try to do a few things to distinguish what type of CPU wrote the file.
   unsigned char c;
   c = sizeof(uint32);
   fwrite(&c, 1, 1, out);
   // little endian or big endian marker.  Currently there is no code to handle conversion, and most likely
   // if this is run on a big endian, then the FFT will certainly be different, so it does not matter any way.
   uint32 n = 0x12345678;
   fwrite(&n, 1, sizeof(n), out);

   // Store a placeholder for the checksum
   uint32 crc_val;
   initCrc(crc_val);
   long loc = ftell(out);
   fwrite (&crc_val, 1, sizeof(crc_val), out);

   // Write the length of the prime string, and the string.  The string will NOT be null terminated.
   str_len = strlen(g_cpTestString);
   fwrite (&str_len, 1, sizeof(str_len), out);
   if (str_len > 256)
      str_len = 256;
   updatecrcbuf(crc_val, &str_len, sizeof(str_len));
   fwrite(g_cpTestString, 1, str_len, out);
   updatecrcbuf(crc_val, g_cpTestString, str_len);

   // now write the file data
   *cp++ = (uint8)ePRP;
   *cp++ = (uint8)eCType;
   // 2 reserved bytes (re alignes us to 4 bytes again)
   *cp++ = 0;
   *cp++ = 0;
   *(uint32*)cp = iDone;
   cp += sizeof(iDone);
   *(uint32*)cp = _iBase;
   cp += sizeof(_iBase);

   // Resserved space, 10 DWORDS
   memset(cp, 0, 4*10);
   cp += sizeof(uint32)*10;

   updatecrcbuf(crc_val, Buffer, cp-Buffer);
   fwrite(Buffer, 1, cp-Buffer, out);

   // Now write out the GWInteger

   Integer N;
   uint32   bytelen;

   N = *gwX;
   bytelen = N.gmp()->_mp_size * sizeof (mp_limb_t);

   fwrite (&bytelen, 1, sizeof(bytelen), out);
   updatecrcbuf(crc_val, &bytelen, sizeof(bytelen));

   fwrite (N.gmp()->_mp_d, 1, bytelen, out);
   updatecrcbuf(crc_val, N.gmp()->_mp_d, bytelen);

   fseek(out, loc, SEEK_SET);
   fwrite(&crc_val, 1, sizeof(crc_val), out);
   fclose(out);

   return true;
}
