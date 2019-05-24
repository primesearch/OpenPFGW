/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pflist.h
 *
 * Description:
 * Template class for a list
 *======================================================================
 */
#ifndef PFLIST_H

#include "listbase.h"
#include "pfforwarditerator.h"
#include "pfboolean.h"
#include "pflistnode.h"

template<class T>
class PFList : private ListHead
{
	PFBoolean m_bOwnsObjects;
public:
	PFList(const PFBoolean &bOwns=PFBoolean::b_false);
	virtual ~PFList();
	
	void StartIterator(PFForwardIterator &pffi);
	void RemoveAll();
	
	DWORD GetSize();
	void AddTail(T* pObject);
	T* RemoveTail();
	T* Tail();
	
	PFListNode *HeadNode();
	PFListNode *TailNode();
};

template<class T>
PFList<T>::PFList(const PFBoolean& bOwns)
	: m_bOwnsObjects(bOwns)
{
	m_pHead=(ListNode*)&m_pTail;
	m_pTail=NULL;
	m_pTailpred=(ListNode*)&m_pHead;
}

template<class T>
PFList<T>::~PFList()
{
	RemoveAll();
}

template<class T>
DWORD PFList<T>::GetSize()
{
	// the destructor destroys the nodes
	PFForwardIterator liIterator;
	StartIterator(liIterator);
	DWORD dwSize=0;

	PFListNode *pNode;

	while(liIterator.Iterate(pNode))
	{
		dwSize++;
	}
	
	return dwSize;
}

template<class T>
void PFList<T>::RemoveAll()
{
	// the destructor destroys the nodes
	PFForwardIterator liIterator;
	StartIterator(liIterator);

	PFListNode *pNode;

	while(liIterator.Iterate(pNode))
	{
		pNode->Remove();
		if(m_bOwnsObjects)
		{
			T* pObject=(T*)pNode->GetData();
			delete pObject;
		}
		delete pNode;
	}
}
template<class T>
void PFList<T>::StartIterator(PFForwardIterator &pffi)
{
	// make the current node point to us
	pffi.Start(HeadNode());
}

template<class T>
void PFList<T>::AddTail(T* pObject)
{
	PFListNode *pNewNode=new PFListNode(pObject);
	
	PFListNode *pSucc=(PFListNode *)TailNode();
	PFListNode *pPred=pSucc->Previous();
	
	PFListNode::ListLinker(pPred,pNewNode);
	PFListNode::ListLinker(pNewNode,pSucc);
}

template<class T>
T* PFList<T>::RemoveTail()
{
	T* pRetval=NULL;
	
	PFListNode *pSucc=(PFListNode *)TailNode();
	PFListNode *pTail=pSucc->Previous();
	
	if(pTail->Previous())
	{
		pRetval=(T*)pTail->GetData();
		pTail->Remove();
		delete pTail;
	}
	return pRetval;
}

template<class T>
T* PFList<T>::Tail()
{
	T* pRetval=NULL;
	
	PFListNode *pSucc=(PFListNode *)TailNode();
	PFListNode *pTail=pSucc->Previous();
	
	if(pTail->Previous())
	{
		pRetval=(T*)pTail->GetData();
	}
	return pRetval;
}
	
template<class T>
PFListNode *PFList<T>::HeadNode()
{
	return (PFListNode*)(ListNode*)(ListHead*)this;
}

template<class T>
PFListNode *PFList<T>::TailNode()
{
	return (PFListNode*)(ListNode*)&m_pTail;
}
#define PFLIST_H
#endif
