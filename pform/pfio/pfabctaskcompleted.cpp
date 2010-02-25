#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "pfiopch.h"
#include <stdio.h>
#include <string.h>

#include "pfabctaskcompleted.h"

PFABCTaskCompleted::PFABCTaskCompleted(const char *pExpr)
{
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
}

PFABCTaskCompleted::~PFABCTaskCompleted()
{
   delete[] m_DoneList;
   delete[] m_pCompleted;
}

int u64Search(const void *x, const void *y)
{
   if (*(uint64*)x > *(uint64*)y)
      return 1;
   if (*(uint64*)x == *(uint64*)y)
      return 0;
   return -1;
}

bool PFABCTaskCompleted::ProcessThisValue(char *s_array[26])
{
   if (!m_nDoneTempCnt && !m_nDoneCnt)
      return true;
   uint64 x;
   sscanf(s_array[m_WhichDone], LL_FORMAT, &x);
   uint32 i;
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

   uint64 x;
   sscanf(s_array[m_WhichDone], LL_FORMAT, &x);
   uint32 i;

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

void PFABCTaskCompleted::AddDone(uint64 Value)
{
   if (m_nDoneTempCnt == sizeof(m_DoneListTemp)/sizeof(m_DoneListTemp[0]))
   {
      uint64  *p = m_DoneList;
      m_DoneList = new uint64[m_nDoneCnt + sizeof(m_DoneListTemp)/sizeof(m_DoneListTemp[0])];
      memcpy(m_DoneList, p, m_nDoneCnt*sizeof(m_DoneList[0]));
      memcpy(&m_DoneList[m_nDoneCnt], m_DoneListTemp, sizeof(m_DoneListTemp));
      m_nDoneCnt += sizeof(m_DoneListTemp)/sizeof(m_DoneListTemp[0]);
      qsort(m_DoneList, m_nDoneCnt, sizeof(m_DoneList[0]), u64Search);
      delete[] p;
      m_nDoneTempCnt = 0;
   }
   m_DoneListTemp[m_nDoneTempCnt++] = Value;
}
