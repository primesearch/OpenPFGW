/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pffastbuffer.h
 *
 * Description:
 * Template class for a growing buffer of T objects (no copy constructor)
 *======================================================================
 */
#ifndef PFLIB_PFFASTBUFFER_H
#define PFLIB_PFFASTBUFFER_H

#include "pfbuffer.h"

template<class T>
class PFFastBuffer : public PFBuffer<T>
{
	using PFBuffer<T>::m_pBuffer;
	using PFBuffer<T>::m_dwAllocated;
	using PFBuffer<T>::m_dwGrow;
public:
	PFFastBuffer(DWORD dwAllocation=64,DWORD dwGrow=256);

	PFFastBuffer(const PFFastBuffer<T> &pfb);
	PFFastBuffer<T> & operator=(const PFFastBuffer<T> &pfb);

	// class maintenance
	virtual void Resize(DWORD dwNewSize);
 	virtual void InsertCell(DWORD dwIndex,DWORD dwMax=0);
	virtual void RemoveCell(DWORD dwIndex,DWORD dwMax=0);
	
	virtual DWORD Find(const T* pData,DWORD dwLength,DWORD dwMax) const;
};

template<class T>
PFFastBuffer<T>::PFFastBuffer(DWORD dwAllocation,DWORD dwGrow)
	: PFBuffer<T>(dwAllocation,dwGrow)
{
}

template<class T>
PFFastBuffer<T>::PFFastBuffer(const PFFastBuffer<T> &pfb)
	: PFBuffer<T>(pfb.m_dwAllocated,pfb.m_dwGrow)
{
	memcpy(m_pBuffer,pfb.m_pBuffer,m_dwAllocated*sizeof(T));
}

template<class T>
PFFastBuffer<T> &PFFastBuffer<T>::operator=(const PFFastBuffer<T> &pfb)
{
	// allow x=x
	if (this == &pfb)
		return (*this);
	if (m_dwAllocated>=pfb.m_dwAllocated)
	{
		memcpy(m_pBuffer,pfb.m_pBuffer,pfb.m_dwAllocated*sizeof(T));
		return(*this);
	}
	delete[] m_pBuffer;
	m_dwAllocated=pfb.m_dwAllocated;
	m_pBuffer=new T[m_dwAllocated];
	m_dwGrow=pfb.m_dwGrow;
	memcpy(m_pBuffer,pfb.m_pBuffer,m_dwAllocated*sizeof(T));
	return(*this);
}

template<class T>
void PFFastBuffer<T>::Resize(DWORD dwSize)
{
	if(dwSize>m_dwAllocated)
	{
		DWORD dwOldSize=m_dwAllocated;
		T* pOldData=m_pBuffer;
		m_pBuffer=NULL;
		this->Allocate(dwSize);

		memcpy(m_pBuffer,pOldData,dwOldSize*sizeof(T));
		delete[] pOldData;
	}
}

template<class T>
void PFFastBuffer<T>::RemoveCell(DWORD dwIndex,DWORD dwMax)
{
	if(dwMax==0)
	{
		dwMax=m_dwAllocated;
	}
	if(dwMax-1>dwIndex)
	{
		memmove(m_pBuffer+dwIndex,m_pBuffer+dwIndex+1,(dwMax-1-dwIndex)*sizeof(T));
	}
}

template<class T>
void PFFastBuffer<T>::InsertCell(DWORD dwIndex,DWORD dwMax)
{
	if(dwMax==0)
	{
		dwMax=m_dwAllocated;
	}
	if(dwMax-1>dwIndex)
	{
		memmove(m_pBuffer+dwIndex+1,m_pBuffer+dwIndex,(dwMax-1-dwIndex)*sizeof(T));
	}
}

template<class T>
DWORD PFFastBuffer<T>::Find(const T* pData,DWORD dwLength,DWORD dwMax) const
{
	DWORD dwFindPosition=NOT_FOUND;

	for(DWORD dwOffset=0;dwOffset+dwLength<=dwMax;dwOffset++)
	{
		if(0==memcmp(pData,m_pBuffer+dwOffset,dwLength*sizeof(T)))
		{
			dwFindPosition=dwOffset;
			break;
		}
	}
	return dwFindPosition;
}

#endif
