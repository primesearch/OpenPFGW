//   PFNewPGenFile
//
//   This class is based upon the PFSimpleFile, and adds NewPGen logic

#include "pfiopch.h"
#include <stdio.h>
#include <string.h>
#include "pfnewpgenfile.h"

char PFNewPGenFile::NormFormats[30][22] =
{
   {ULL_FORMAT"*%d^"ULL_FORMAT"+1"},      //k*b^n+1
   {ULL_FORMAT"*%d^"ULL_FORMAT"-1"},      //k*b^n-1
   {ULL_FORMAT"*%d^("ULL_FORMAT"+1)+1"},  //k*b^(n+1)+1
   {ULL_FORMAT"*%d^("ULL_FORMAT"+1)-1"},  //k*b^(n+1)-1
   {ULL_FORMAT"*%d^("ULL_FORMAT"-1)+1"},  //k*b^(n-1)+1
   {ULL_FORMAT"*%d^("ULL_FORMAT"-1)-1"},  //k*b^(n-1)-1
   {ULL_FORMAT"*%d^("ULL_FORMAT"+2)+1"},  //k*b^(n+2)+1
   {ULL_FORMAT"*%d^("ULL_FORMAT"+2)-1"},  //k*b^(n+2)-1
   {ULL_FORMAT"*%d^("ULL_FORMAT"+3)+1"},  //k*b^(n+3)+1
   {ULL_FORMAT"*%d^("ULL_FORMAT"+3)-1"},  //k*b^(n+3)-1
   {ULL_FORMAT"*%d^("ULL_FORMAT"+4)+1"},  //k*b^(n+4)+1
   {ULL_FORMAT"*%d^("ULL_FORMAT"+4)-1"},  //k*b^(n+4)-1
   {"%d^"ULL_FORMAT"+2*"ULL_FORMAT"-1"},  //b^n+2k-1     (Revesed k and n values)
   {"2*"ULL_FORMAT"*%d^"ULL_FORMAT"+1"},  //2*k*b^n+1     BiTwin correct for non base-2 search     (Not used)
   {"2*"ULL_FORMAT"*%d^"ULL_FORMAT"-1"},  //2*k*b^n-1     BiTwin correct for non base-2 search     (Not used)
   {"("ULL_FORMAT"*%d^"ULL_FORMAT")/2+1"},   //(k*b^n)/2-1   BiTwin correct (long) for non base-2 search (Not used)
   {"("ULL_FORMAT"*%d^"ULL_FORMAT")/2-1"},   //(k*b^n)/2-1   BiTwin correct (long) for non base-2 search (Not used)

   // Extra stuff for the non-generalised chains
   {"2*"ULL_FORMAT"*%d^"ULL_FORMAT"+1"},  //2k*b^n+1
   {"2*"ULL_FORMAT"*%d^"ULL_FORMAT"-1"},  //2k*b^n-1
   {"4*"ULL_FORMAT"*%d^"ULL_FORMAT"+1"},  //4k*b^n+1
   {"4*"ULL_FORMAT"*%d^"ULL_FORMAT"-1"},  //4k*b^n-1
   {"8*"ULL_FORMAT"*%d^"ULL_FORMAT"+1"},  //8k*b^n+1
   {"8*"ULL_FORMAT"*%d^"ULL_FORMAT"-1"},  //8k*b^n-1
   {"16*"ULL_FORMAT"*%d^"ULL_FORMAT"+1"}, //16k*b^n+1
   {"16*"ULL_FORMAT"*%d^"ULL_FORMAT"-1"}, //16k*b^n-1

   // Extra stuff for the 3-tuple/4-tuple
   {ULL_FORMAT"*%d^"ULL_FORMAT"+5"},      //k*b^n+5
   {ULL_FORMAT"*%d^"ULL_FORMAT"+7"},      //k*b^n+7

   // This is for the SG of the form k.b^n+1, 2k.b^n+3
   {"2*"ULL_FORMAT"*%d^"ULL_FORMAT"+3"},  //2k*b^n+3

   {"%d^"ULL_FORMAT"+"ULL_FORMAT},        //2^n+k  (fixed k)         (Revesed k and n values)
   {"%d^"ULL_FORMAT"-"ULL_FORMAT}         //2^n-k  (fixed k)         (Revesed k and n values)

};

char PFNewPGenFile::PrimorialFormats[22][21] =
{
   {ULL_FORMAT"*"ULL_FORMAT"#+1"},       //k*n#+1
   {ULL_FORMAT"*"ULL_FORMAT"#-1"},       //k*n#-1
   {"2*"ULL_FORMAT"*"ULL_FORMAT"#+1"},     //2*k*n#+1
   {"2*"ULL_FORMAT"*"ULL_FORMAT"#-1"},     //2*k*n#-1
   {"("ULL_FORMAT"*"ULL_FORMAT"#)/2+1"},   //(k*n#)/2+1
   {"("ULL_FORMAT"*"ULL_FORMAT"#)/2-1"},   //(k*n#)/2-1
   {"4*"ULL_FORMAT"*"ULL_FORMAT"#+1"},     //4*k*n#+1
   {"4*"ULL_FORMAT"*"ULL_FORMAT"#-1"},     //4*k*n#-1
   {"8*"ULL_FORMAT"*"ULL_FORMAT"#+1"},     //8*k*n#+1
   {"8*"ULL_FORMAT"*"ULL_FORMAT"#-1"},     //8*k*n#-1
   {"16*"ULL_FORMAT"*"ULL_FORMAT"#+1"},    //16*k*n#+1
   {"16*"ULL_FORMAT"*"ULL_FORMAT"#-1"},    //16*k*n#-1
   {ULL_FORMAT"#+2*"ULL_FORMAT"-1"},     //n#+2k-1

   // extra stuff for the 3-tuple / 4-tuple searches.
   {"("ULL_FORMAT"*"ULL_FORMAT"#)/5+1"},  //(k*n#)/5+1
   {"("ULL_FORMAT"*"ULL_FORMAT"#)/5-1"},  //(k*n#)/5-1
   {"("ULL_FORMAT"*"ULL_FORMAT"#)/5+5"},  //(k*n#)/5+5
   {"("ULL_FORMAT"*"ULL_FORMAT"#)/35+1"}, //(k*n#)/35+1
   {"("ULL_FORMAT"*"ULL_FORMAT"#)/35-1"}, //(k*n#)/35-1
   {"("ULL_FORMAT"*"ULL_FORMAT"#)/35+5"}, //(k*n#)/35+5
   {"("ULL_FORMAT"*"ULL_FORMAT"#)/35+7"},  //(k*n#)/35+7

   // Extra stuff for the primoproth searches
   {ULL_FORMAT"#*2^"ULL_FORMAT"+1"},       //k#*2^n+1
   {ULL_FORMAT"#*2^"ULL_FORMAT"-1"}        //k#*2^n-1
};


PFNewPGenFile::PFNewPGenFile(const char* FileName)
   : PFSimpleFile(FileName), m_k(0), m_n(0), m_base_n(0),
     m_bGoOnToNextMuli(false), m_bPrimorial(false), m_bLuckyPrime(false),
     m_bBiTwinPrime(false), m_bConsecutive(false),
     m_bMultiPrimeSearch(false), m_nCurrentMultiPrime(0),
     m_nNumMultiPrime(0), m_npg_c(0), m_npg_len(0), m_npg_b(0),
     m_npg_bitmap(0), m_pBaseInteger(0), m_TmpInteger(0),
     m_Integer_one(1), m_Integer_neg_one(-1),
     m_Integer_five(5), m_Integer_seven(7),
     m_bBaseIntegerValid(false)

{
   m_SigString = "NewPGen file: ";
}

void Primorial(Integer *p, uint32 pp)
{
    Integer mm;
    mm=1;
    uint32 i, q = 0;

    for (i=1; q<pp; i++)
    {
       q = (uint32) primeserver->ByIndex(i);

       if (q > pp) break;

       mm *= q;
    }

    *p=mm;
}

void PFNewPGenFile::LoadFirstLine()
{
   char Line[1024];
   uint64 kvalue, n;

   if (ReadLine(Line, sizeof(Line)))
   {
      fclose(m_fpInputFile);
      throw "Error, Not a valid NewPGen file";
   }

   // Get the "first" line to allow us to make the "base" expression
   if (ReadLine(Line, sizeof(Line)))
   {
      fclose(m_fpInputFile);
      throw "Empty NewPGen file";
   }
   if (strchr(Line, ' ') == NULL)
   {
      fclose(m_fpInputFile);
      throw "Invalid NewPGen file.  The first line is not of a valid format!\n";
   }
   sscanf (Line, ULL_FORMAT" "ULL_FORMAT"\n", &kvalue, &n);

   // Ok, rewind file to the "first" line.
   fseek(m_fpInputFile, 0, SEEK_SET);
   ReadLine(Line, sizeof(Line));
   Line[sizeof(Line)-1] = 0;

   // We had to separate out the "processing" of the first line, from the
   // reading of it, so that we can make derive the compressed PrZ NewPGen format
   // processing format code.  It calls ProcessFirstLine when it gets the "data"
   // from that compressed file.
   ProcessFirstLine(Line, kvalue, n);
}

void PFNewPGenFile::ProcessFirstLine(char *Line, uint64 k, uint64 n)
{
   char cpTmpOutput[256];
   uint64 kvalue;

   m_base_n = n;
   kvalue = k;

   m_nCurrentLineNum = 1;
   m_nCurrentPhysicalLineNum = 1;

   uint64 u64tmp; // Unused right now, but may be useful in the future (tells the depth of the sieving)
   int count = sscanf(Line, ULL_FORMAT":%c:%d:%d:%d\n", &u64tmp, &m_npg_c, &m_npg_len, &m_npg_b, &m_npg_bitmap);
   if (count != 5)
   {
      fclose(m_fpInputFile);
      throw "Invalid NewPGen file.  The first line is not of a valid format!\n";
   }

   // See NewPGen-Formats.txt for more detailed description
   if (m_npg_bitmap & MODE_PRIMORIAL)
      m_bPrimorial = true;

   // Assume the search to be a multiple prime search.
   m_bMultiPrimeSearch = true;

   // This is a LONG complex piece-o-crap, but it werks (kind of) so I am not prone to changing it
   //  (until Paul adds to NewPGen that is ;)
   if (m_bPrimorial)
       m_SigString += "primorial ";

   if ((m_npg_bitmap & MODE_KSIEVE) == 0)
   {
      m_bBaseIntegerValid = true;
   }
   else
   {
      if (m_npg_bitmap & MODE_PRIMORIAL)
          m_SigString += "PrimoProth ";
      else
          m_SigString += "mode-K sieve ";
   }

   switch (m_npg_c)
   {
      // Twin (or 3-tuple or 4-tuple)
      case 'T':
      {

         if (!(m_npg_bitmap & MODE_PLUS5))
         {
            m_nNumMultiPrime = 2;      // Twin
            if (m_bPrimorial)
            {
               m_pFormat[0] = PrimorialFormats[0];
               m_pFormat[1] = PrimorialFormats[1];
               sprintf (cpTmpOutput, "Twin k*"ULL_FORMAT"#+-1 ", m_base_n);
               m_SigString += cpTmpOutput;
            }
            else
            {
               m_pFormat[0] = NormFormats[0];
               m_pFormat[1] = NormFormats[1];
               sprintf (cpTmpOutput, "Twin k*%d^"ULL_FORMAT"+-1 ", m_npg_b, m_base_n);
               m_SigString += cpTmpOutput;
            }
         }
         else
         {
            if ((m_npg_bitmap & MODE_4TUPLE) == MODE_3TUPLE)
            {
               m_nNumMultiPrime = 3;      // 3-tuple
               if (m_bPrimorial)
               {
                  m_pFormat[0] = PrimorialFormats[13];
                  m_pFormat[1] = PrimorialFormats[14];
                  m_pFormat[2] = PrimorialFormats[15];
               }
               else
               {
                  m_pFormat[0] = NormFormats[0];
                  m_pFormat[1] = NormFormats[1];
                  m_pFormat[2] = NormFormats[25];
               }
               m_SigString += "3-Tuple ";
            }
            else if ((m_npg_bitmap & MODE_4TUPLE) == MODE_4TUPLE)
            {
               m_nNumMultiPrime = 4;   // 4-tuple
               if (m_bPrimorial)
               {
                  m_pFormat[0] = PrimorialFormats[16];
                  m_pFormat[1] = PrimorialFormats[17];
                  m_pFormat[2] = PrimorialFormats[18];
                  m_pFormat[3] = PrimorialFormats[19];
               }
               else
               {
                  m_pFormat[0] = NormFormats[0];
                  m_pFormat[1] = NormFormats[1];
                  m_pFormat[2] = NormFormats[25];
                  m_pFormat[3] = NormFormats[26];
               }
               m_SigString += "4-Tuple ";
            }
            else
            {
               fclose(m_fpInputFile);
               throw "Error! Unknown NewPGen Twin Type file";
            }
         }
         break;
      }

      // Consecutive AP (solid sieve). NOTE that n, k (and b) have orders swapped.
      case 'A':
      {
         m_bBaseIntegerValid = false;
         m_bMultiPrimeSearch = false;
         m_bConsecutive = true;
         if (!m_bPrimorial)
         {
            m_pFormat[0] = NormFormats[12];
            // Should this be "b^n+2k-1" or "b^n+2k-2"? -2 if b is odd; -1 if b is even.
            if (m_npg_b & 1)
               m_pFormat[0][strlen (m_pFormat[0])-1] = '2';
            else
               m_pFormat[0][strlen (m_pFormat[0])-1] = '1';
         }
         else
            m_pFormat[0] = PrimorialFormats[12];
         m_SigString += "Consecutive Primes in AP ";
         break;
      }

      // +1 sieve
      case 'P':
      {
         // Check for the k.b^n+1, 2k.b^n+3 SG sieve.
         if (m_npg_bitmap & MODE_2PLUS3)
         {
            m_nNumMultiPrime = 2;      // SG
            m_pFormat[0] = NormFormats[0];
            m_pFormat[1] = NormFormats[27];
            sprintf (cpTmpOutput, "SG k*%d^"ULL_FORMAT"+1 & 2k*%d^"ULL_FORMAT"+3 ", m_npg_b, m_base_n, m_npg_b, m_base_n);
            m_SigString += cpTmpOutput;
            break;
         }

         m_bMultiPrimeSearch = false;
         if (!m_bPrimorial)
         {
            m_pFormat[0] = NormFormats[0];

            if (m_npg_bitmap&MODE_KSIEVE)
               sprintf (cpTmpOutput, ULL_FORMAT"*%d^n+1 ", kvalue, m_npg_b);
            else
               sprintf (cpTmpOutput, "k*%d^"ULL_FORMAT"+1 ", m_npg_b, m_base_n);
            // new 2^n+k (fixed k) mode.  NOTE that n, k (and b) have orders swapped.
            if (m_npg_bitmap&MODE_BASE2FIXEDK)
            {
               m_bBaseIntegerValid = false;
               m_bConsecutive = true;  // triggers the "reversed n/k output format).
               sprintf (cpTmpOutput, "2^n+"ULL_FORMAT" Fixed_K ", kvalue);
               m_pFormat[0] = NormFormats[28];
            }
            m_SigString += cpTmpOutput;
         }
         else
         {
            if (m_npg_bitmap&MODE_KSIEVE)
            {
               m_pFormat[0] = PrimorialFormats[20];
               sprintf (cpTmpOutput, ULL_FORMAT"#*2^n+1 ", kvalue);
            }
            else
            {
               m_pFormat[0] = PrimorialFormats[0];
               sprintf (cpTmpOutput, "k*"ULL_FORMAT"#+1 ", m_base_n);
            }
            m_SigString += cpTmpOutput;
         }
         break;
      }

      // -1 sieve
      case 'M':
      {
         m_bMultiPrimeSearch = false;
         if (!m_bPrimorial)
         {
            m_pFormat[0] = NormFormats[1];

            if (m_npg_bitmap&MODE_KSIEVE)
               sprintf (cpTmpOutput, ULL_FORMAT"*%d^n-1 ", kvalue, m_npg_b);
            else
               sprintf (cpTmpOutput, "k*%d^"ULL_FORMAT"-1 ", m_npg_b, m_base_n);
            // new 2^n+k (fixed k) mode. NOTE that n, k (and b) have orders swapped.
            if (m_npg_bitmap&MODE_BASE2FIXEDK)
            {
               m_bBaseIntegerValid = false;
               m_bConsecutive = true;  // triggers the "reversed n/k output format).
               sprintf (cpTmpOutput, "2^n-"ULL_FORMAT" Fixed_K ", kvalue);
               m_pFormat[0] = NormFormats[29];
            }
            m_SigString += cpTmpOutput;
         }
         else
         {
            if (m_npg_bitmap&MODE_KSIEVE)
            {
               m_pFormat[0] = PrimorialFormats[21];
               sprintf (cpTmpOutput, ULL_FORMAT"#*2^n-1 ", kvalue);
            }
            else
            {
               m_pFormat[0] = PrimorialFormats[1];
               sprintf (cpTmpOutput, "k*"ULL_FORMAT"#-1 ", m_base_n);
            }
            m_SigString += cpTmpOutput;
         }
         break;
      }

      // Sophie Germain sieve
      case 'S':
      {
         m_bBaseIntegerValid = false;
         m_nNumMultiPrime = 2;
         if (!m_bPrimorial)
         {
            if (m_npg_bitmap & MODE_NOTGENERALISED)
            {
               m_pFormat[0] = NormFormats[1];
               m_pFormat[1] = NormFormats[18];
               sprintf (cpTmpOutput, "SG k*%d^"ULL_FORMAT"-1 & 2k*%d^"ULL_FORMAT"-1 ", m_npg_b, m_base_n, m_npg_b, m_base_n);
            }
            else
            {
               m_pFormat[0] = NormFormats[1];
               m_pFormat[1] = NormFormats[3];
               sprintf (cpTmpOutput, "SG k*%d^"ULL_FORMAT"-1 & k*%d^("ULL_FORMAT"+1)-1 ", m_npg_b, m_base_n, m_npg_b, m_base_n);
            }
            m_SigString += cpTmpOutput;
         }
         else
         {
            m_pFormat[0] = PrimorialFormats[1];
            m_pFormat[1] = PrimorialFormats[3];
            sprintf (cpTmpOutput, "SG k*"ULL_FORMAT"#-1 & 2k*"ULL_FORMAT"#-1 ", m_base_n, m_base_n);
            m_SigString += cpTmpOutput;
         }
         break;
      }

      // Cunningham type 2 sieve
      case 'C':
      {
         m_bBaseIntegerValid = false;
         m_nNumMultiPrime = 2;
         if (!m_bPrimorial)
         {
            if (m_npg_bitmap & MODE_NOTGENERALISED)
            {
               m_pFormat[0] = NormFormats[0];
               m_pFormat[1] = NormFormats[17];
               sprintf (cpTmpOutput, "CC k*%d^"ULL_FORMAT"+1 & 2k*%d^"ULL_FORMAT"+1 ", m_npg_b, m_base_n, m_npg_b, m_base_n);
            }
            else
            {
               m_pFormat[0] = NormFormats[0];
               m_pFormat[1] = NormFormats[2];
               sprintf (cpTmpOutput, "CC k*%d^"ULL_FORMAT"+1 & k*%d^("ULL_FORMAT"+1)+1 ", m_npg_b, m_base_n, m_npg_b, m_base_n);
            }
            m_SigString += cpTmpOutput;
         }
         else
         {
            m_pFormat[0] = PrimorialFormats[0];
            m_pFormat[1] = PrimorialFormats[2];
            sprintf (cpTmpOutput, "CC k*"ULL_FORMAT"#+1 & 2k*"ULL_FORMAT"#+1 ", m_base_n, m_base_n);
            m_SigString += cpTmpOutput;
         }
         break;
      }

      // BiTwin
      case 'B':
      {
         m_bBaseIntegerValid = false;
         m_bBiTwinPrime = true;
         m_nNumMultiPrime = 4;
         if (!m_bPrimorial)
         {
            if (m_npg_bitmap & MODE_NOTGENERALISED)
            {
               m_pFormat[0] = NormFormats[0];
               m_pFormat[1] = NormFormats[1];
               m_pFormat[2] = NormFormats[17];
               m_pFormat[3] = NormFormats[18];
               sprintf (cpTmpOutput, "BiTwin k*%d^"ULL_FORMAT"+1 ", m_npg_b, m_base_n);
            }
            else
            {
               m_pFormat[0] = NormFormats[0];
               m_pFormat[1] = NormFormats[1];
               m_pFormat[2] = NormFormats[2];
               m_pFormat[3] = NormFormats[3];
               sprintf (cpTmpOutput, "BiTwin k*%d^"ULL_FORMAT"+1 ", m_npg_b, m_base_n);
            }
            m_SigString += cpTmpOutput;
         }
         else
         {
            m_pFormat[0] = PrimorialFormats[0];
            m_pFormat[1] = PrimorialFormats[1];
            m_pFormat[2] = PrimorialFormats[2];
            m_pFormat[3] = PrimorialFormats[3];
            sprintf (cpTmpOutput, "BiTwin k*"ULL_FORMAT"#+1 ", m_base_n);
            m_SigString += cpTmpOutput;
         }
         break;
      }

      // "Lucky Prime" +1
      case 'Y':
      {
         // "fix" the m_npg_bitmap from 11101 to 1111 so that the Twin, SG, and CC are counted correctly.
         m_bBaseIntegerValid = false;
         m_nNumMultiPrime = 4;
         m_bLuckyPrime = true;
         m_npg_bitmap = 0xF;
         if (!m_bPrimorial)
         {
            m_pFormat[0] = NormFormats[0];
            m_pFormat[1] = NormFormats[1];
            m_pFormat[2] = NormFormats[4];
            m_pFormat[3] = NormFormats[2];
            sprintf (cpTmpOutput, "Lucky Plus k*%d^"ULL_FORMAT"+1 & k*b^(n-1)+1 & k*b^(n+1)+1 ", m_npg_b, m_base_n);
            m_SigString += cpTmpOutput;
         }
         else
         {
            m_pFormat[0] = PrimorialFormats[0];
            m_pFormat[1] = PrimorialFormats[1];
            m_pFormat[2] = PrimorialFormats[4];
            m_pFormat[3] = PrimorialFormats[2];
            sprintf (cpTmpOutput, "Lucky Plus k*"ULL_FORMAT"#+1 & (k*n#)/2+1 2kn#+1 ", m_base_n);
            m_SigString += cpTmpOutput;
         }
         break;
      }

      // "Lucky Prime" -1
      case 'Z':
      {
         // "fix" the m_npg_bitmap from 11101 to 1111 so that the Twin, SG, and CC are counted correctly.
         m_bBaseIntegerValid = false;
         m_nNumMultiPrime = 4;
         m_bLuckyPrime = true;
         m_npg_bitmap = 0xF;
         if (!m_bPrimorial)
         {
            m_pFormat[0] = NormFormats[1];
            m_pFormat[1] = NormFormats[0];
            m_pFormat[2] = NormFormats[5];
            m_pFormat[3] = NormFormats[3];
            sprintf (cpTmpOutput, "Lucky Minus k*%d^"ULL_FORMAT"+1 & k*b^(n-1)-1 & k*b^(n+1)-1 ", m_npg_b, m_base_n);
            m_SigString += cpTmpOutput;
         }
         else
         {
            m_pFormat[0] = PrimorialFormats[1];
            m_pFormat[1] = PrimorialFormats[0];
            m_pFormat[2] = PrimorialFormats[5];
            m_pFormat[3] = PrimorialFormats[3];
            sprintf (cpTmpOutput, "Lucky Minus k*"ULL_FORMAT"#+1 & (k*n#)/2-1 2kn#-1 ", m_base_n);
            m_SigString += cpTmpOutput;
         }
         break;
      }

      // Twin SG
      case 'J':
      {
         m_bBaseIntegerValid = false;
         m_nNumMultiPrime = 3;
         if (!m_bPrimorial)
         {
            if (m_npg_bitmap & MODE_NOTGENERALISED)
            {
               m_pFormat[0] = NormFormats[1];
               m_pFormat[1] = NormFormats[0];
               m_pFormat[2] = NormFormats[18];
               sprintf (cpTmpOutput, "Twin SG k*%d^"ULL_FORMAT"+-1 & 2k*%d^"ULL_FORMAT"-1 ", m_npg_b, m_base_n, m_npg_b, m_base_n);
            }
            else
            {
               m_pFormat[0] = NormFormats[1];
               m_pFormat[1] = NormFormats[0];
               m_pFormat[2] = NormFormats[3];
               sprintf (cpTmpOutput, "Twin SG k*%d^"ULL_FORMAT"+-1 & k*%d^("ULL_FORMAT"+1)-1 ", m_npg_b, m_base_n, m_npg_b, m_base_n);
            }
            m_SigString += cpTmpOutput;
         }
         else
         {
            m_pFormat[0] = PrimorialFormats[1];
            m_pFormat[1] = PrimorialFormats[0];
            m_pFormat[2] = PrimorialFormats[3];
            sprintf (cpTmpOutput, "Twin SG k*"ULL_FORMAT"#+-1 & 2k*"ULL_FORMAT"#-1 ", m_base_n, m_base_n);
            m_SigString += cpTmpOutput;
         }
         break;
      }

      // Twin CC
      case 'K':
      {
         m_bBaseIntegerValid = false;
         m_nNumMultiPrime = 3;
         if (!m_bPrimorial)
         {
            if (m_npg_bitmap & MODE_NOTGENERALISED)
            {
               m_pFormat[0] = NormFormats[0];
               m_pFormat[1] = NormFormats[1];
               m_pFormat[2] = NormFormats[17];
               sprintf (cpTmpOutput, "Twin CC k*%d^"ULL_FORMAT"+-1 & 2k*%d^"ULL_FORMAT"+1 ", m_npg_b, m_base_n, m_npg_b, m_base_n);
            }
            else
            {
               m_pFormat[0] = NormFormats[0];
               m_pFormat[1] = NormFormats[1];
               m_pFormat[2] = NormFormats[2];
               sprintf (cpTmpOutput, "Twin CC k*%d^"ULL_FORMAT"+-1 & k*%d^("ULL_FORMAT"+1)+1 ", m_npg_b, m_base_n, m_npg_b, m_base_n);
            }
            m_SigString += cpTmpOutput;
         }
         else
         {
            m_pFormat[0] = PrimorialFormats[0];
            m_pFormat[1] = PrimorialFormats[1];
            m_pFormat[2] = PrimorialFormats[2];
            sprintf (cpTmpOutput, "Twin CC k*"ULL_FORMAT"#+-1 & 2k*"ULL_FORMAT"#+1 ", m_base_n, m_base_n);
            m_SigString += cpTmpOutput;
         }
         break;
      }

      // CC Chain (1st kind, Len > 2)
      case '1':
      {
         m_bBaseIntegerValid = false;
         m_nNumMultiPrime = m_npg_len;
         if (!m_bPrimorial)
         {
            if (m_npg_bitmap & MODE_NOTGENERALISED)
            {
               m_pFormat[0] = NormFormats[1];
               m_pFormat[1] = NormFormats[18];
               m_pFormat[2] = NormFormats[20];
               sprintf (cpTmpOutput, "CC-1st Len-%d k*%d^"ULL_FORMAT"-1 ... ", m_nNumMultiPrime, m_npg_b, m_base_n-1);
            }
            else
            {
               // CC chains start 1/2 of "normal"
               m_pFormat[0] = NormFormats[5];
               m_pFormat[1] = NormFormats[1];
               m_pFormat[2] = NormFormats[3];
               sprintf (cpTmpOutput, "Generalized CC-1st Len-%d k*%d^"ULL_FORMAT"-1 ... ", m_nNumMultiPrime, m_npg_b, m_base_n-1);
            }
            m_SigString += cpTmpOutput;
         }
         else
         {
            m_pFormat[0] = PrimorialFormats[1];
            m_pFormat[1] = PrimorialFormats[3];
            m_pFormat[2] = PrimorialFormats[7];
            sprintf (cpTmpOutput, "CC-1st Len-%d k*"ULL_FORMAT"#-1 ... ", m_nNumMultiPrime, m_base_n);
            m_SigString += cpTmpOutput;
         }
         if (m_nNumMultiPrime > 5)
         {
            m_SigString = "\nNotice! Maximum CC 1st kind Chain length PFGW will look for is 5 ";
            m_nNumMultiPrime = 5;
         }
         for (int i = 3; i < m_nNumMultiPrime; i++)
         {
            if (!m_bPrimorial)
            {
               if (m_npg_bitmap & MODE_NOTGENERALISED)
                  m_pFormat[i] = NormFormats[i*2+16]; // 22, 24
               else
                  m_pFormat[i] = NormFormats[i*2+1];  // 7, 9
            }
            else
               m_pFormat[i] = PrimorialFormats[i*2+3];  // 9, 11
         }
         break;
      }
      // CC Chain (2nd kind, Len > 2)
      case '2':
      {
         m_bBaseIntegerValid = false;
         m_nNumMultiPrime = m_npg_len;
         if (!m_bPrimorial)
         {
            if (m_npg_bitmap & MODE_NOTGENERALISED)
            {
               m_pFormat[0] = NormFormats[0];
               m_pFormat[1] = NormFormats[17];
               m_pFormat[2] = NormFormats[19];
               sprintf (cpTmpOutput, "CC-2nd Len-%d k*%d^"ULL_FORMAT"+1 ... ", m_nNumMultiPrime, m_npg_b, m_base_n-1);
            }
            else
            {
               // CC chains start 1/2 of "normal"
               m_pFormat[0] = NormFormats[4];
               m_pFormat[1] = NormFormats[0];
               m_pFormat[2] = NormFormats[2];
               sprintf (cpTmpOutput, "Generalized CC-2nd Len-%d k*%d^"ULL_FORMAT"+1 ... ", m_nNumMultiPrime, m_npg_b, m_base_n-1);
            }
            m_SigString += cpTmpOutput;
         }
         else
         {
            m_pFormat[0] = PrimorialFormats[0];
            m_pFormat[1] = PrimorialFormats[2];
            m_pFormat[2] = PrimorialFormats[6];
            sprintf (cpTmpOutput, "CC-2nd Len-%d k*"ULL_FORMAT"#+1 ... ", m_nNumMultiPrime, m_base_n);
            m_SigString += cpTmpOutput;
         }
         if (m_nNumMultiPrime > 5)
         {
            m_SigString += "Notice! Maximum CC 2nd kind Chain length PFGW will look for is 5\n";
            m_nNumMultiPrime = 5;
         }
         for (int i = 3; i < m_nNumMultiPrime; i++)
         {
            if (!m_bPrimorial)
            {
               if (m_npg_bitmap & MODE_NOTGENERALISED)
                  m_pFormat[i] = NormFormats[i*2+15];  // 21, 23
               else
                  m_pFormat[i] = NormFormats[i*2];  // 6, 8
            }
            else
               m_pFormat[i] = PrimorialFormats[i*2+2]; // 8, 10
         }
         break;
      }

      // BiTwin (Len > 2)
      case '3':
      {
         m_bBaseIntegerValid = false;
         m_bBiTwinPrime = true;
         m_nNumMultiPrime = m_npg_len*2;
         if (!m_bPrimorial)
         {
            if (m_npg_bitmap & MODE_NOTGENERALISED)
            {
               m_pFormat[0] = NormFormats[0];
               m_pFormat[1] = NormFormats[1];
               m_pFormat[2] = NormFormats[17];
               m_pFormat[3] = NormFormats[18];
               m_pFormat[4] = NormFormats[19];
               m_pFormat[5] = NormFormats[20];
               sprintf (cpTmpOutput, "BiTwin Len-%d k*%d^"ULL_FORMAT"+1 ... ", m_nNumMultiPrime/2, m_npg_b, m_base_n-1);
            }
            else
            {
               // BiTwin chains start 1/2 of "normal"
               m_pFormat[0] = NormFormats[4];
               m_pFormat[1] = NormFormats[5];
               m_pFormat[2] = NormFormats[0];
               m_pFormat[3] = NormFormats[1];
               m_pFormat[4] = NormFormats[2];
               m_pFormat[5] = NormFormats[3];
               sprintf (cpTmpOutput, "Generalized BiTwin Len-%d k*%d^"ULL_FORMAT"+1 ... ", m_nNumMultiPrime/2, m_npg_b, m_base_n-1);
            }
            m_SigString += cpTmpOutput;
         }
         else
         {
            m_pFormat[0] = PrimorialFormats[0];
            m_pFormat[1] = PrimorialFormats[1];
            m_pFormat[2] = PrimorialFormats[2];
            m_pFormat[3] = PrimorialFormats[3];
            m_pFormat[4] = PrimorialFormats[6];
            m_pFormat[5] = PrimorialFormats[7];
            sprintf (cpTmpOutput, "BiTwin Len-%d k*"ULL_FORMAT"#+1 ... ", m_nNumMultiPrime/2, m_base_n);
            m_SigString += cpTmpOutput;
         }
         if (m_nNumMultiPrime > 10)
         {
            m_SigString += "Notice! Maximum BiTwin length PFGW will look for is 5\n";
            m_nNumMultiPrime = 10;
         }
         for (int i = 6; i < m_nNumMultiPrime; i+=2)
         {
            if (!m_bPrimorial)
            {
               if (m_npg_bitmap & MODE_NOTGENERALISED)
               {
                  m_pFormat[i] = NormFormats[i+15];
                  m_pFormat[i+1] = NormFormats[i+16];
               }
               else
               {
                  m_pFormat[i] = NormFormats[i];
                  m_pFormat[i+1] = NormFormats[i+1];
               }
            }
            else
            {
               m_pFormat[i] = PrimorialFormats[i+2];
               m_pFormat[i+1] = PrimorialFormats[i+3];
            }
         }
         break;
      }

      default:
      {
         fclose(m_fpInputFile);
         throw "Unknown NewPGen sieve Type";
      }
   }

   if (m_bBaseIntegerValid)
   {
      m_pBaseInteger = new Integer;
      if (!m_bPrimorial)
         m_pBaseInteger->Ipow(m_npg_b,(int32)m_base_n);
      else
      {
          Primorial(m_pBaseInteger, (int32)m_base_n);
         if ( (m_npg_bitmap & (MODE_PLUS5|MODE_PLUS7)) == MODE_PLUS5)      // 3-tuple
            *m_pBaseInteger /= 5;
         else if ( (m_npg_bitmap & (MODE_PLUS5|MODE_PLUS7)) == (MODE_PLUS5|MODE_PLUS7))   // 4-tuple
            *m_pBaseInteger /= 35;
      }
   }

   // don't count this "first" line, we have to "reset" to line 0 numbering.
   m_nCurrentLineNum = 1;
}

PFNewPGenFile::~PFNewPGenFile()
{
   delete m_pBaseInteger;
}


int PFNewPGenFile::SeekToLine(int LineNumber)
{
   char Buf[128];
   if (LineNumber < m_nCurrentLineNum)
   {
      fseek(m_fpInputFile, 0, SEEK_SET);
      m_nCurrentLineNum = 1;
      m_nCurrentPhysicalLineNum = 1;
      ReadLine(Buf, sizeof(Buf));
      if (m_pIni)
         m_pIni->SetFileProcessing(true);
      m_bEOF = false;
   }
   int ret = e_ok;
   while (m_nCurrentLineNum < LineNumber)
   {
      if (ReadLine(Buf, sizeof(Buf)))
      {
         if (m_pIni)
            m_pIni->SetFileProcessing(false);
         m_bEOF = true;
         ret = e_eof;
         break;
      }
      m_nCurrentLineNum++;
   }
   if (m_pIni)
      m_pIni->SetFileLineNum(m_nCurrentLineNum);
   return ret;
}

int PFNewPGenFile::GetNextLine(PFString &sLine, Integer *i, bool *b, PFSymbolTable *)
{
   char TmpBuf[128];  // no worry about buffer overflow.  NewPGen expressions will be short.
   sLine = "";
   if (b)
      *b = false;    // assume failure;
   if (m_bEOF)
   {
      m_sCurrentExpression = "";
      return e_eof;
   }

   bool bStoreThisExpression=false;

   if (!(m_bGoOnToNextMuli && m_bMultiPrimeSearch && m_nCurrentMultiPrime < m_nNumMultiPrime))
   {
      // Get the "next" Line
      char Line[128];
      m_nCurrentPhysicalLineNum++;
      m_nCurrentLineNum++;
      bStoreThisExpression = true;
      if (m_pIni)
         m_pIni->SetFileLineNum(m_nCurrentLineNum);
      if (ReadLine(Line, sizeof(Line)))
      {
         if (m_pIni)
            m_pIni->SetFileProcessing(false);
         m_bEOF = true;
         sLine = "";
         m_sCurrentExpression = "";
         return e_eof;
      }

      char *cp = strchr(Line, ' ');
      if (cp)
      {
         sscanf (Line, ULL_FORMAT" "ULL_FORMAT"\n", &m_k, &m_n);
         //printf ("%s was scanned, and "ULL_FORMAT" "ULL_FORMAT" were found\n", Line, m_k, m_n);
      }
      m_nCurrentMultiPrime = 0;
   }

   if (m_bPrimorial)
   {
      if (m_bConsecutive)
         sprintf (TmpBuf, m_pFormat[m_nCurrentMultiPrime], m_n, m_k);
      else
         sprintf (TmpBuf, m_pFormat[m_nCurrentMultiPrime], m_k, m_n);
   }
   else
   {
      if (m_bConsecutive)
         sprintf (TmpBuf, m_pFormat[m_nCurrentMultiPrime], m_npg_b, m_n, m_k);
      else
         sprintf (TmpBuf, m_pFormat[m_nCurrentMultiPrime], m_k, m_npg_b, m_n);
   }
   sLine = TmpBuf;

   if (b && i && m_bBaseIntegerValid)
   {
      // "check to make SURE the n is still the same.
      if (m_n != m_base_n)
      {
         // something dreadful happened.  Most likely someone hand edited a NewPGen file.
         // Do nothing
      }
      else
      {
         Integer *pAddSubInteger = &m_Integer_one;
         char *cp = &TmpBuf[strlen(TmpBuf)-2];
         if (cp[0] == '-' && cp[1] == '1')
            pAddSubInteger = &m_Integer_neg_one;
         else if (cp[0] == '+')
         {
            if (cp[1] == '1')
               ; // do nothing, since the pointer is set to m_Integer_one already
            else if (cp[1] == '5')
               pAddSubInteger = &m_Integer_five;
            else if (cp[1] == '7')
               pAddSubInteger = &m_Integer_seven;
            else
            {
               PFOutput::EnableOneLineForceScreenOutput();
               PFPrintfStderr("NewPGen processing problem! What the heck kind of format is this???\n");
            }
         }
         else
         {
            PFOutput::EnableOneLineForceScreenOutput();
            PFPrintfStderr("NewPGen processing problem! What the heck kind of format is this???\n");
         }
         static Integer K;
         K = m_k;
         (*i) = (*m_pBaseInteger)*K+(*pAddSubInteger);
         *b = true;
      }
   }
   if (bStoreThisExpression)
      m_sCurrentExpression = sLine;
   return e_ok;
}

// Note this function is unused by the rest of the app at this time.  Plans were to use this in a GUI inteface
// (to keep the screen updated with current k and n's), but that has not yet been done.
int PFNewPGenFile::GetKNB(uint64 &k, uint64 &n, unsigned &b)
{
   k = m_k;
   n = m_n;
   b = m_npg_b;
   return e_ok;
}

void PFNewPGenFile::CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool *p_bMessageStringIsValid, PFString *p_MessageString)
{
   m_bGoOnToNextMuli = (bIsPrime || bIsPRP);
   if (p_bMessageStringIsValid)
      *p_bMessageStringIsValid = false;
   if (!(bIsPrime || bIsPRP))
   {
      // For twin/SG, twin/CC, LP, and LM, do not reset the count. Instead, go on to the next expression.
      if (m_nCurrentMultiPrime && (m_npg_c == 'J' || m_npg_c == 'K' || m_npg_c == 'Y' || m_npg_c == 'Z'))
      {
         m_bGoOnToNextMuli = true;
         m_nCurrentMultiPrime++;
      }
      else
         m_nCurrentMultiPrime = 0;
      return;
   }

   m_nCurrentMultiPrime++;
   if (m_nCurrentMultiPrime < 2)
      return;

   // Ok, we may have a "special message

   // Again, this is a LONG complex piece-o-crap, but it werks (kind of) so I am not prone to
   // changing it (until Paul adds to NewPGen that is ;)

   if (m_nCurrentMultiPrime == 2)
   {
      if ((m_npg_bitmap&MODE_TWIN)==MODE_TWIN)
      {
         *p_MessageString = "  - Twin -";
         *p_bMessageStringIsValid = true;
      }
      else if ((m_npg_bitmap&MODE_CC)==MODE_CC)
      {
         *p_MessageString = "  - CC 2nd kind -";
         *p_bMessageStringIsValid = true;
      }
      else if ((m_npg_bitmap&MODE_SG)==MODE_SG)
      {
         *p_MessageString = "  - SG -";
         *p_bMessageStringIsValid = true;
      }
      return;
   }

   if (m_nCurrentMultiPrime == 3)
   {
      if (m_npg_c == 'J')
      {
         *p_MessageString = "  - SG -";
         *p_bMessageStringIsValid = true;
         return;
      }
      if (m_npg_c == 'K')
      {
         *p_MessageString = "  - CC 2nd kind -";
         *p_bMessageStringIsValid = true;
         return;
      }
      if (m_npg_c == 'Y')
      {
         *p_MessageString = "  - CC 2nd kind -";
         *p_bMessageStringIsValid = true;
         return;
      }
      if (m_npg_c == 'Z')
      {
         *p_MessageString = "  - SG -";
         *p_bMessageStringIsValid = true;
         return;
      }
      if ((m_npg_bitmap&MODE_PLUS5)==MODE_PLUS5)
      {
         *p_MessageString = "  - 3-Tuple -";
         *p_bMessageStringIsValid = true;
         return;
      }
      if ((m_npg_bitmap&MODE_CC23)==MODE_CC23)
      {
         *p_MessageString = "  - CC 2nd kind len 3 -";
         *p_bMessageStringIsValid = true;
      }
      if ((m_npg_bitmap&MODE_CC13)==MODE_CC13)
      {
         if (m_bBiTwinPrime)
            *p_MessageString = "  - CC 1st kind -";
         else
            *p_MessageString = "  - CC 1st kind len 3 -";
         *p_bMessageStringIsValid = true;
      }
      return;
   }

   if (m_nCurrentMultiPrime == 4)      // chance for BiTwins
   {
      if (m_npg_c == 'Y')
      {
         *p_MessageString = "  - CC 2nd kind -";
         *p_bMessageStringIsValid = true;
         return;
      }
      if (m_npg_c == 'Z')
      {
         *p_MessageString = "  - SG -";
         *p_bMessageStringIsValid = true;
         return;
      }
      if ((m_npg_bitmap&MODE_PLUS7)==MODE_PLUS7)
      {
         *p_MessageString = "  - 4-Tuple -";
         *p_bMessageStringIsValid = true;
         return;
      }
      if ((m_npg_bitmap&MODE_CC23)==MODE_CC23)
      {
         *p_MessageString = "  - CC 2nd kind len 4 -";
         *p_bMessageStringIsValid = true;
      }
      if ((m_npg_bitmap&MODE_CC13)==MODE_CC13)
      {
         *p_MessageString = "  - CC 1st kind len 4 -";
         *p_bMessageStringIsValid = true;
      }
      // Note that Lucky primes have their bitmap "changed" so that the Twins, the CC's and any SG primes will
      // be found, but they are NOT BiTwin's
      if ((m_npg_bitmap&MODE_BI)==MODE_BI && !m_bLuckyPrime)
      {
         *p_MessageString = "  - BiTwin -";
         *p_bMessageStringIsValid = true;
      }
      return;
   }
   if (m_nCurrentMultiPrime == 5)
   {
      if ((m_npg_bitmap&MODE_CC23)==MODE_CC23)
      {
         *p_MessageString = "  - CC 2nd kind len 5 -";
         *p_bMessageStringIsValid = true;
      }
      if ((m_npg_bitmap&MODE_CC13)==MODE_CC13)
      {
         if (m_bBiTwinPrime)
            *p_MessageString = "  - CC 1st kind  len 3 -";
         else
            *p_MessageString = "  - CC 1st kind len 5 -";
         *p_bMessageStringIsValid = true;
      }
      return;
   }
   if (m_nCurrentMultiPrime == 6)      // chance for BiTwins
   {
      if ((m_npg_bitmap&MODE_BI3)==MODE_BI3)
      {
         *p_MessageString = "  - BiTwin len 3 -";
         *p_bMessageStringIsValid = true;
      }
      return;
   }

   if (m_nCurrentMultiPrime == 7)
   {
      if (m_bBiTwinPrime)
      {
         *p_MessageString = "  - CC 2nd kind  len 4 -";
         *p_bMessageStringIsValid = true;
      }
      return;
   }

   if (m_nCurrentMultiPrime == 8)      // chance for BiTwins
   {
      if ((m_npg_bitmap&MODE_BI3)==MODE_BI3)
      {
         *p_MessageString = "  - BiTwin len 4 -";
         *p_bMessageStringIsValid = true;
      }
      return;
   }

   if (m_nCurrentMultiPrime == 9)
   {
      if (m_bBiTwinPrime)
      {
         *p_MessageString = "  - CC 2nd kind  len 5 -";
         *p_bMessageStringIsValid = true;
      }
      return;
   }

   if (m_nCurrentMultiPrime == 10)     // chance for BiTwins
   {
      if ((m_npg_bitmap&MODE_BI3)==MODE_BI3)
      {
         *p_MessageString = "  - BiTwin len 5 -";
         *p_bMessageStringIsValid = true;
      }
      return;
   }
}

PFNewPGenDeepFile::PFNewPGenDeepFile(const char* FileName) : PFNewPGenFile(FileName),
   m_bSpecialDeepFactoringNeeded(false), m_bSimpleDeepFactoringNeeded(false), m_eSearchType(eunk)
{
   ResetFoundPrimes();
}

void PFNewPGenDeepFile::LoadFirstLine()
{
   PFNewPGenFile::LoadFirstLine();

   // Ok, check for "deep" factoring needed stuff.
   switch (m_npg_c)
   {
      case 'T':
      {
         // True twins and 3-tuples don't need ANY deep factoring.  4-tuples do
         if ( (m_npg_bitmap >> 7) == 0x11)
         {
            m_bSpecialDeepFactoringNeeded = true;
            // Test +1 and if prime, test -1, then test +5 and if prime test +7
            // Correctly label either twin, either 3-tuple and a 4-tuple depending upon results.
            m_eSearchType = e4tuple;
            m_SigString += "\nDeep Search Mode ";
         }
         else
            m_SigString += "\nDeep Search Mode (ignored) ";

         break;
      }

      // Skip 'A' 'P' 'M' since there are only single prime searches.
      // Skip 'S' and 'C' since they are 2 prime searches, so they are deep by default.
      case 'A':
      case 'P':
      case 'M':
      case 'S':
      case 'C':
         m_SigString += "\nDeep Search Mode (ignored) ";


      // BiTwin
      case 'B':
      {
         m_bSimpleDeepFactoringNeeded = true;
         // simply search the whole damn thing. there are 2 twins, and SG and a CC possible.
         m_eSearchType = ebitwin;
         m_SigString += "\nDeep Search Mode ";
         break;
      }
      // "Lucky Prime" +1
      case 'Y':
      {
         m_bSpecialDeepFactoringNeeded = true;
         // Test the twin.  IFF the +1 is prime, then test k*base^(n-1)+1 and k*base^(n+1)+1
         m_eSearchType = eluckyp;
         m_SigString += "\nDeep Search Mode ";
         break;
      }
      // "Lucky Prime" -1
      case 'Z':
      {
         m_bSpecialDeepFactoringNeeded = true;
         // Test the twin.  IFF the -1 is prime, then test k*base^(n-1)-1 and k*base^(n+1)-1
         m_eSearchType = eluckym;
         m_SigString += "\nDeep Search Mode ";
         break;
      }
      // Twin SG
      case 'J':
      {
         m_bSpecialDeepFactoringNeeded = true;
         // Test -1. IFF the -1 is prime, then both the other 2
         m_eSearchType = etwinsg;
         m_SigString += "\nDeep Search Mode ";
         break;
      }
      // Twin CC
      case 'K':
      {
         m_bSpecialDeepFactoringNeeded = true;
         // Test +1. IFF the +1 is prime, then both the other 2
         m_eSearchType = etwincc;
         m_SigString += "\nDeep Search Mode ";
         break;
      }
      // CC Chain (1st kind, Len > 2)
      case '1':
      {
         m_bSpecialDeepFactoringNeeded = true;
         // Test order is 2 then 4 then 6 ...  If any of these are found to be prime, then test the
         // adjoining 2 numbers.  Be sure not to test anything more than one time, and be sure to
         // list all CC chains found.
         m_eSearchType = etwincc_c1;
         m_SigString += "\nDeep Search Mode ";
         break;
      }
      // CC Chain (2nd kind, Len > 2)
      case '2':
      {
         m_bSpecialDeepFactoringNeeded = true;
         // Test order is 2 then 4 then 6 ...  If any of these are found to be prime, then test the
         // adjoining 2 numbers.  Be sure not to test anything more than one time, and be sure to
         // list all CC chains found.
         m_eSearchType = etwincc_c2;
         m_SigString += "\nDeep Search Mode ";
         break;
      }
      // BiTwin (Len > 2)
      case '3':
      {
         m_bSimpleDeepFactoringNeeded = true;
         // simply search the whole damn thing. there are 2 twins, and SG and a CC possible.
         m_eSearchType = ebitwin_c;
         m_SigString += "\nDeep Search Mode ";
         break;
      }
   }
}

PFNewPGenDeepFile::~PFNewPGenDeepFile()
{
}

void PFNewPGenDeepFile::CurrentNumberIsPRPOrPrime(bool bIsPRP, bool bIsPrime, bool *p_bMessageStringIsValid, PFString *p_MessageString)
{
   if (!m_bSpecialDeepFactoringNeeded && !m_bSimpleDeepFactoringNeeded)
      PFNewPGenFile::CurrentNumberIsPRPOrPrime(bIsPRP, bIsPrime, p_bMessageStringIsValid, p_MessageString);
   else if (m_bSimpleDeepFactoringNeeded)
   {
   }
}

int PFNewPGenDeepFile::GetNextLine(PFString & /*sLine*/, Integer * /*i*/, bool * /*b*/, PFSymbolTable *)
{
   return 0;
}
