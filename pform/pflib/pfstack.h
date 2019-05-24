/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pfstack.h
 *
 * Description:
 * Template class for a stack
 *======================================================================
 */
#ifndef PFSTACK_H
#define PFSTACK_H

#include "pflist.h"

template<class T>
class PFStack : public PFList<T>
{
public:
	PFStack(const PFBoolean &bOwns=PFBoolean::b_false);
	virtual ~PFStack();

	void Push(T* pObject);
	T* Pop();
	void Remove();
	T* Peek();
};

template<class T>
PFStack<T>::PFStack(const PFBoolean& bOwns)
	: PFList<T>(bOwns)
{
}

template<class T>
PFStack<T>::~PFStack()
{
}

template<class T>
void PFStack<T>::Push(T* pObject)
{
	this->AddTail(pObject);
}

template<class T>
T* PFStack<T>::Pop()
{
	return this->RemoveTail();
}

template<class T>
void PFStack<T>::Remove()
{
	this->RemoveTail();
}

template<class T>
T* PFStack<T>::Peek()
{
	return this->Tail();
}
#endif
