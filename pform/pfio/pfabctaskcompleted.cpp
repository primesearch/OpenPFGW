#include <stdio.h>
#include <string.h>
#include "pfiopch.h"
#include "pfabctaskcompleted.h"

PFABCTaskCompleted::PFABCTaskCompleted(const char *pExpr, const char *pABC)
{
   FILE *f;
   int    valueCount, skipped;
   uint64_t values[26];
   char   scanLine[500];
   char   logLine[500];
   char  *tp;

   m_DoneList = NULL;
   m_nDoneCnt = 0;
   memset(m_DoneListTemp, 0, sizeof(m_DoneListTemp));
   m_nDoneTempCnt = 0;
   m_pCompleted = NULL;

   memset(m_CompletedTemp, 0, sizeof(m_CompletedTemp));
   m_nCompleted = 0;
   m_nCompletedTemp = 0;

   const char *cp = strstr(pExpr, "{number_primes,");
   char c;
   if (!cp)
   {
      if (sscanf(pExpr, "{number_comps,$%c,%d}", &c, &m_NumDones) != 2)
         throw 666;
      if (c < 'a' || c > 'z')
         throw 666;
      m_bPrimes = false;
   }
   else
   {
      if (sscanf(pExpr, "{number_primes,$%c,%d}", &c, &m_NumDones) != 2)
         throw 666;
      if (c < 'a' || c > 'z')
         throw 666;
      m_bPrimes = true;
   }
   m_WhichDone = c-'a';

   if (!m_bPrimes) return;

   if (strlen(pABC) > 300) return;
   cp = strstr(pABC, " ");
   if (!cp) return;
   while (*cp && *cp == ' ') cp++;

   memset(scanLine, 0, sizeof(scanLine));
   tp = scanLine;
   valueCount = 0;
   while (*cp)
   {
      if (*cp != '$') { *tp = *cp; tp++; cp++; continue; }

      strcat(tp, "%llu");
      tp += 4;
      cp += 2;
      valueCount++;
   }

   if (!valueCount) return;

   skipped = 0;

   f = fopen("pfgw.log", "r");
   if (f)
   {
      cp = fgets(logLine, sizeof(logLine), f);
      while (cp && strlen(logLine) < sizeof(logLine)-2)
      {
         if (sscanf(logLine, scanLine, &values[0], &values[1], &values[2], &values[3], &values[4], &values[5], &values[6], &values[7],
                                       &values[8], &values[9], &values[10], &values[11], &values[12], &values[13], &values[14],
                                       &values[15], &values[16], &values[17], &values[18], &values[19], &values[20],
                                       &values[21], &values[22], &values[23], &values[25], &values[25]) == valueCount)
         {
            if (ProcessThisValue(values[m_WhichDone]))
            {
               skipped++;
               AddPrimeOrComposite(values[m_WhichDone]);
            }
         }
         cp = fgets(logLine, sizeof(logLine), f);
      }

      fclose(f);
   }

   f = fopen("pfgw-prime.log", "r");
   if (f)
   {
      cp = fgets(logLine, sizeof(logLine), f);
      while (cp && strlen(logLine) < sizeof(logLine)-2)
      {
         if (sscanf(logLine, scanLine, &values[0], &values[1], &values[2], &values[3], &values[4], &values[5], &values[6], &values[7],
                                       &values[8], &values[9], &values[10], &values[11], &values[12], &values[13], &values[14],
                                       &values[15], &values[16], &values[17], &values[18], &values[19], &values[20],
                                       &values[21], &values[22], &values[23], &values[25], &values[25]) == valueCount)
         {
            if (ProcessThisValue(values[m_WhichDone]))
            {
               skipped++;
               AddPrimeOrComposite(values[m_WhichDone]);
            }
         }
         cp = fgets(logLine, sizeof(logLine), f);
      }

      fclose(f);
   }

   if (skipped)
      PFPrintfStderr("Found %d previous PRPs/Primes for distinct $%c.\n", skipped, c);
}

PFABCTaskCompleted::~PFABCTaskCompleted()
{
   delete[] m_DoneList;
   delete[] m_pCompleted;
}

int u64Search(const void *x, const void *y)
{
   if (*(uint64_t*)x > *(uint64_t*)y)
      return 1;
   if (*(uint64_t*)x == *(uint64_t*)y)
      return 0;
   return -1;
}

bool PFABCTaskCompleted::ProcessThisValue(char *s_array[26])
{
   if (!m_nDoneTempCnt && !m_nDoneCnt)
      return true;
   uint64_t x;
   sscanf(s_array[m_WhichDone], "%" SCNu64"", &x);

   return ProcessThisValue(x);
}

bool PFABCTaskCompleted::ProcessThisValue(uint64_t x)
{
   if (!m_nDoneTempCnt && !m_nDoneCnt)
      return true;

   uint32_t i;
   for (i = 0; i < m_nDoneTempCnt; ++i)
   {
      if (x == m_DoneListTemp[i])
         return false;
   }
   if (m_nDoneCnt && bsearch(&x, m_DoneList, m_nDoneCnt, sizeof(m_DoneList[0]), u64Search))
      return false;
   return true;
}

int Complete_t_Search(const void *x, const void *y)
{
   if ( ((Complete_t*)x)->Value > ((Complete_t*)y)->Value)
      return 1;
   if ( ((Complete_t*)x)->Value == ((Complete_t*)y)->Value)
      return 0;
   return -1;
}

void PFABCTaskCompleted::AddPrimeOrComposite(char *s_array[26], bool bIsPrime)
{
   if (bIsPrime && !m_bPrimes)
      return;
   if (!bIsPrime && m_bPrimes)
      return;

   uint64_t x;
   sscanf(s_array[m_WhichDone], "%" SCNu64"", &x);

   AddPrimeOrComposite(x);
}

void PFABCTaskCompleted::AddPrimeOrComposite(uint64_t x)
{
   uint32_t i;

   for (i = 0; i < m_nCompletedTemp; ++i)
   {
      if (m_CompletedTemp[i].Value == x)
      {
         ++m_CompletedTemp[i].NumDone;
         if (m_CompletedTemp[i].NumDone == m_NumDones)
            AddDone(x);
         return;
      }
   }

   if (m_nCompleted)
   {
      Complete_t *p = (Complete_t *)bsearch(&x, m_pCompleted, m_nCompleted, sizeof(m_pCompleted[0]), Complete_t_Search);
      if (p)
      {
         ++p->NumDone;
         if (p->NumDone == m_NumDones)
            AddDone(x);
         return;
      }
   }
   if (m_nCompletedTemp == sizeof(m_CompletedTemp)/sizeof(m_CompletedTemp[0]))
   {
      Complete_t  *p = m_pCompleted;
      m_pCompleted = new Complete_t[m_nCompleted + sizeof(m_CompletedTemp)/sizeof(m_CompletedTemp[0])];
      memcpy(m_pCompleted, p, m_nCompleted*sizeof(m_pCompleted[0]));
      memcpy(&m_pCompleted[m_nCompleted], m_CompletedTemp, sizeof(m_CompletedTemp));
      m_nCompleted += sizeof(m_CompletedTemp)/sizeof(m_CompletedTemp[0]);
      qsort(m_pCompleted, m_nCompleted, sizeof(m_pCompleted[0]), Complete_t_Search);
      delete[] p;
      m_nCompletedTemp = 0;
   }
   if (m_NumDones == 1)
      AddDone(x);
   else
   {
      m_CompletedTemp[m_nCompletedTemp].NumDone = 1;
      m_CompletedTemp[m_nCompletedTemp++].Value = x;
   }
}

void PFABCTaskCompleted::AddDone(uint64_t Value)
{
   if (m_nDoneTempCnt == sizeof(m_DoneListTemp)/sizeof(m_DoneListTemp[0]))
   {
      uint64_t  *p = m_DoneList;
      m_DoneList = new uint64_t[m_nDoneCnt + sizeof(m_DoneListTemp)/sizeof(m_DoneListTemp[0])];
      memcpy(m_DoneList, p, m_nDoneCnt*sizeof(m_DoneList[0]));
      memcpy(&m_DoneList[m_nDoneCnt], m_DoneListTemp, sizeof(m_DoneListTemp));
      m_nDoneCnt += sizeof(m_DoneListTemp)/sizeof(m_DoneListTemp[0]);
      qsort(m_DoneList, m_nDoneCnt, sizeof(m_DoneList[0]), u64Search);
      delete[] p;
      m_nDoneTempCnt = 0;
   }
   m_DoneListTemp[m_nDoneTempCnt++] = Value;
}
