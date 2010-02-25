/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pfptrarray.h
 *
 * Description:
 * An array of pointers
 *======================================================================
 */
#ifndef PFLIB_PFPTRARRAY_H
#define PFLIB_PFPTRARRAY_H

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "pfarray.h"
#include "pffastbuffer.h"

typedef PFFastBuffer<LPVOID> PFPointerBuffer;

template<class T>
class PFPtrArray : public PFArray<PFPointerBuffer,LPVOID>
{
   PFBoolean m_bOwnsPointers;
public:
   PFPtrArray(const PFBoolean &bOwns=PFBoolean::b_false);
   virtual ~PFPtrArray();

   T* operator[](DWORD dwIndex);
};

template<class T>
T* PFPtrArray<T>::operator[](DWORD dwIndex)
{
   return((T*)(PFArray<PFPointerBuffer,LPVOID>::operator[](dwIndex)));
}

template<class T>
PFPtrArray<T>::PFPtrArray(const PFBoolean &bOwns)
   : m_bOwnsPointers(bOwns)
{
}

template<class T>
PFPtrArray<T>::~PFPtrArray()
{
   if(m_bOwnsPointers)
   {
      DWORD dwSize=GetSize();
      for(DWORD dw=0;dw<dwSize;dw++)
      {
         delete (T*)operator[](dw);
      }
   }
}
#endif
