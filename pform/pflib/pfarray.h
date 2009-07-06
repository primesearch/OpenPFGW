/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pfarray.h
 *
 * Description:
 * Template class for an array of T objects
 *======================================================================
 */
#ifndef PFLIB_PFARRAY_H
#define PFLIB_PFARRAY_H

template<class C,class T>
class PFArray : public C
{
	DWORD m_dwSize;
public:
	PFArray();

	void Add(const T& t);
	void InsertAt(const T& t,DWORD dwIndex);
	void RemoveAt(DWORD dwIndex);
	DWORD GetSize() const;
	void RemoveAll();
};

template<class C,class T>
PFArray<C,T>::PFArray()
	: m_dwSize(0)
{
}

template<class C,class T>
void PFArray<C,T>::Add(const T& t)
{
	// gcc is b0rked, and the [] didn't work correctly
	this->AccessElement(m_dwSize)=t;
	m_dwSize++;
}

template<class C,class T>
DWORD PFArray<C,T>::GetSize() const
{
	return m_dwSize;
}

template<class C,class T>
void PFArray<C,T>::InsertAt(const T& t,DWORD dwIndex)
{
	m_dwSize++;
	this->Resize(m_dwSize);
	this->InsertCell(dwIndex,m_dwSize);
	this->AccessElement(dwIndex)=t; // work around gcc const/nonconst [] bug
}

template<class C,class T>
void PFArray<C,T>::RemoveAt(DWORD dwIndex)
{
	this->RemoveCell(dwIndex,m_dwSize);
	m_dwSize--;
}

template<class C,class T>
void PFArray<C,T>::RemoveAll()
{
	m_dwSize=0;
}
#endif
