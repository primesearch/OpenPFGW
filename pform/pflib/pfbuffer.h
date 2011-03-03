/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pfbuffer.h
 *
 * Description:
 * Template class for a growing buffer of T objects
 *======================================================================
 */
#ifndef PFLIB_PFBUFFER_H
#define PFLIB_PFBUFFER_H

#include "config.h"

#define 	NOT_FOUND	((DWORD)0xFFFFFFFFUL)

template<class T>
class PFBuffer
{
protected:
	DWORD m_dwGrow;
	T*		m_pBuffer;
	DWORD m_dwAllocated;


	// Phil - non-const access is too dangerous, I had to 'hide' it here
	// gcc is b0rked and won't let a const [] coexist
	T& AccessElement(DWORD dwIndex);

public:
	PFBuffer(DWORD dwAllocation=64,DWORD dwGrow=256);
	virtual ~PFBuffer();

	PFBuffer(const PFBuffer<T> &pfb);
	PFBuffer<T> & operator=(const PFBuffer<T> &pfb);
	
	// member access, const only to the rabble clients
	const T& operator[](DWORD dwIndex) const;

	const T* GetBasePointer() const;

	// class maintenance
	virtual void Resize(DWORD dwNewSize);
	void Allocate(DWORD dwNewSize);

	virtual void InsertCell(DWORD dwIndex,DWORD dwMax=0);
	virtual void RemoveCell(DWORD dwIndex,DWORD dwMax=0);

	DWORD GrowSize(DWORD dwMinimum) const;
	virtual DWORD Find(const T* pData,DWORD dwLength,DWORD dwMax) const;
};

template<class T>
PFBuffer<T>::PFBuffer(DWORD dwAllocation,DWORD dwGrow)
	: m_dwGrow(dwGrow),  m_pBuffer(NULL), m_dwAllocated(dwAllocation)
{
	m_pBuffer=new T[m_dwAllocated];
}

template<class T>
PFBuffer<T>::~PFBuffer()
{
	if(m_pBuffer)
	{
		delete[] m_pBuffer;
      m_pBuffer = 0;
	}
}

template<class T>
PFBuffer<T>::PFBuffer(const PFBuffer<T> &pfb)
	: m_dwAllocated(pfb.m_dwAllocated), m_dwGrow(pfb.m_dwGrow)
{
	m_pBuffer=new T[m_dwAllocated];
	for(DWORD dw=0;dw<m_dwAllocated;dw++)
	{
		m_pBuffer[dw]=pfb.m_pBuffer[dw];
	}
}

template<class T>
PFBuffer<T> &PFBuffer<T>::operator=(const PFBuffer<T> &pfb)
{
	// allow x=x
	if(this == &pfb)
		return *this;
	delete[] m_pBuffer;
	m_dwAllocated=pfb.m_dwAllocated;
	m_pBuffer=new T[m_dwAllocated];
	m_dwGrow=pfb.m_dwGrow;
	for(DWORD dw=0;dw<m_dwAllocated;dw++)
	{
		m_pBuffer[dw]=pfb.m_pBuffer[dw];
	}
	// This was NOT in the file before.  It suggests (HEAVILY) that this
	// operator has never been used (yet). VC would puke on no return
	// statement on a function with a reference, but since it is inline,
	// if it is not used, then it would simply may not emit the error.
	return *this;
}

template<class T>
const T* PFBuffer<T>::GetBasePointer() const
{
	return m_pBuffer;
}

template<class T>
DWORD PFBuffer<T>::GrowSize(DWORD dwMinimum) const
{
	DWORD dwRetval=m_dwAllocated;

	if(m_dwAllocated>=dwMinimum)
	{
	}
	else
	{
		if(m_dwGrow==0)
		{
			while(dwRetval<dwMinimum)
			{
				dwRetval<<=1;
			}
		}
		else
		{
			DWORD dwGrowthRequired=dwMinimum-dwRetval;
			dwGrowthRequired+=(m_dwGrow-1);
			dwGrowthRequired/=m_dwGrow;
			dwRetval+=(dwGrowthRequired*m_dwGrow);
		}
	}
	return dwRetval;
}

template<class T>
T& PFBuffer<T>::AccessElement(DWORD dwIndex)
{
	Resize(dwIndex+1);
	return m_pBuffer[dwIndex];
}

template<class T>
const T& PFBuffer<T>::operator[](DWORD dwIndex) const
{
	return m_pBuffer[dwIndex];
}

template<class T>
void PFBuffer<T>::Allocate(DWORD dwSize)
{
	DWORD dwTarget=GrowSize(dwSize);
	if(dwTarget!=m_dwAllocated)
	{
		if(m_pBuffer)
		{
			delete[] m_pBuffer;
			m_pBuffer=NULL;
		}
		m_pBuffer=new T[dwTarget];
		m_dwAllocated=dwTarget;
	}
}

template<class T>
void PFBuffer<T>::Resize(DWORD dwSize)
{
	if(dwSize>m_dwAllocated)
	{
		DWORD dwOldSize=m_dwAllocated;
		T* pOldData=m_pBuffer;
		m_pBuffer=NULL;
		Allocate(dwSize);

		for(DWORD dw=0;dw<dwOldSize;dw++)
		{
			m_pBuffer[dw]=pOldData[dw];
		}
		delete[] pOldData;
	}
}

template<class T>
void PFBuffer<T>::RemoveCell(DWORD dwIndex,DWORD dwMax)
{
	if(dwMax==0)
	{
		dwMax=m_dwAllocated;
	}
	for(DWORD dw=dwIndex;dw<dwMax-1;dw++)
	{
		m_pBuffer[dw]=m_pBuffer[dw+1];
	}
}

template<class T>
void PFBuffer<T>::InsertCell(DWORD dwIndex,DWORD dwMax)
{
	if(dwMax==0)
	{
		dwMax=m_dwAllocated;
	}
	for(DWORD dw=dwMax-1;dw>dwIndex;dw--)
	{
		m_pBuffer[dw]=m_pBuffer[dw-1];
	}
}

template<class T>
DWORD PFBuffer<T>::Find(const T* pData,DWORD dwLength,DWORD dwMax) const
{
	DWORD dwFindPosition=NOT_FOUND;

	for(DWORD dwOffset=0;dwOffset+dwLength<=dwMax;dwOffset++)
	{
		DWORD dwIdentical;
		for(dwIdentical=0;dwIdentical<dwLength;dwIdentical++)
		{
			if(pData[dwIdentical]!=m_pBuffer[dwOffset+dwIdentical])
			{
				break;
			}
		}
		if(dwIdentical<dwLength)
		{
		}
		else
		{
			dwFindPosition=dwOffset;
			break;
		}
	}
	return dwFindPosition;
}
	
#endif
