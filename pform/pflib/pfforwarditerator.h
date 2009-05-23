/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pfforwarditerator.h
 *
 * Description:
 * Forward direction list iterator
 *======================================================================
 */
#ifndef PFLIB_PFFORWARDITERATOR_H
#define PFLIB_PFFORWARDITERATOR_H

class PFListNode;

class PFForwardIterator
{
	PFListNode *m_pCurrent;
	PFListNode *m_pNext;
public:
	PFForwardIterator();
	virtual ~PFForwardIterator();
	
	PFForwardIterator(const PFForwardIterator &pffi);
	PFForwardIterator& operator=(const PFForwardIterator &pffi);

	void Start(PFListNode *pStart);
	PFListNode *Iterate(PFListNode *&pCurrent);
	PFListNode *GetCurrentNode();
};
#endif
