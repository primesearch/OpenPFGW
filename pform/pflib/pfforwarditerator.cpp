/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pfforwarditerator.cpp
 *
 * Description:
 * Forward direction list iterator
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
#include "pfforwarditerator.h"

#include "pflistnode.h"

PFForwardIterator::PFForwardIterator()
   : m_pCurrent(NULL), m_pNext(NULL)
{
}

PFForwardIterator::~PFForwardIterator()
{
}

PFForwardIterator::PFForwardIterator(const PFForwardIterator &pffi)
   :  m_pCurrent(pffi.m_pCurrent), m_pNext(pffi.m_pNext)
{
}

PFForwardIterator & PFForwardIterator::operator=(const PFForwardIterator &pffi)
{
   m_pCurrent=pffi.m_pCurrent;
   m_pNext=pffi.m_pNext;
   return *this;
}

void PFForwardIterator::Start(PFListNode *pNode)
{
   m_pCurrent=pNode;
   m_pNext=pNode->Next();
}

PFListNode *PFForwardIterator::Iterate(PFListNode *&pCurrent)
{
   m_pCurrent=m_pNext;
   pCurrent=m_pCurrent;
   m_pNext=m_pCurrent->Next();
   return m_pNext;
}

PFListNode *PFForwardIterator::GetCurrentNode()
{
   return m_pCurrent;
}

