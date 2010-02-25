/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pfcircularbuffer.cpp
 *
 * Description:
 * A circular buffer object
 *======================================================================
 */
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "pflibpch.h"
#include "pfcircularbuffer.h"

PFCircularBuffer::PFCircularBuffer(DWORD dwSize)
   : m_dwHead(0), m_dwTail(0), m_dwBufferSize(dwSize)
{
   Allocate(m_dwBufferSize<<1);  // forces the array to malloc
}

PFCircularBuffer::~PFCircularBuffer()
{
}

DWORD PFCircularBuffer::GetBufferFree() const
{
   return m_dwBufferSize-GetBufferContent();
}

DWORD PFCircularBuffer::GetBufferContent() const
{
   return m_dwTail-m_dwHead;
}

DWORD PFCircularBuffer::GetBufferSize() const
{
   return m_dwBufferSize;
}

void PFCircularBuffer::NormalizeBuffer()
{
   if(m_dwHead>=m_dwBufferSize)
   {
      // Needed due to Phil's changes
      //memmove(&operator[](0),&operator[](m_dwHead),m_dwTail-m_dwHead);
      memmove( &AccessElement(0), &operator[](m_dwHead),m_dwTail-m_dwHead);
      m_dwTail-=m_dwHead;
      m_dwHead=0;
   }
}

PFBoolean PFCircularBuffer::ReadData(LPBYTE pData,DWORD dwCount)
{
   PFBoolean bRetval=PFBoolean::b_false;
   if(dwCount<=GetBufferContent())
   {
      if((pData!=NULL)&&(dwCount>0))
      {
         memcpy(pData,&operator[](m_dwHead),dwCount);
      }
      m_dwHead+=dwCount;
      NormalizeBuffer();
      bRetval=PFBoolean::b_true;
   }
   return bRetval;
}

PFBoolean PFCircularBuffer::WriteData(LPBYTE pData,DWORD dwCount)
{
   PFBoolean bRetval=PFBoolean::b_false;
   if(dwCount<=GetBufferFree())
   {
      if(dwCount!=0)
      {
         // Needed due to Phil's changes
         //memcpy(&operator[](m_dwTail),pData,dwCount);
         memcpy(&AccessElement(m_dwTail),pData,dwCount);
      }
      m_dwTail+=dwCount;
      bRetval=PFBoolean::b_true;
   }
   return bRetval;
}

LPCBYTE PFCircularBuffer::GetReadPointer() const
{
   LPCBYTE pRetval = &operator[](m_dwHead);
   return pRetval;
}

LPBYTE PFCircularBuffer::GetWritePointer()
{
// Needed due to Phil's changes
// LPBYTE pRetval=&operator[](m_dwTail);
   LPBYTE pRetval = &AccessElement(m_dwTail);

   return pRetval;
}

void PFCircularBuffer::MarkBytesRead(DWORD dwRead)
{
   m_dwHead+=dwRead;
   NormalizeBuffer();
}

void PFCircularBuffer::MarkBytesWritten(DWORD dwWritten)
{
   m_dwTail+=dwWritten;
}


