/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pflistnode.cpp
 *
 * Description:
 * A list node
 *======================================================================
 */
#include "pflibpch.h"
#include "pflistnode.h"

/*======================================================================
 * METHOD: PFListNode::PFListNode
 * PURPOSE:
 * Create a list node
 * PARAMETERS:
 * LPVOID object to attach
 * RETURNS:
 * implicit
 *======================================================================
 */
PFListNode::PFListNode(LPVOID pData)
{
	m_pSucc=NULL;
	m_pPred=NULL;
	m_pData=pData;
}

/*======================================================================
 * METHOD: PFListNode::~PFListNode
 * PURPOSE:
 * Destructor
 * PARAMETERS:
 * none
 * RETURNS:
 * implicit
 *======================================================================
 */
PFListNode::~PFListNode()
{
}

PFListNode *PFListNode::Next() const
{
	return (PFListNode*)m_pSucc;
}

PFListNode *PFListNode::Previous() const
{
	return (PFListNode*)m_pPred;
}

void PFListNode::ListLinker(ListNode *p1,ListNode *p2)
{
	p1->m_pSucc=p2;
	p2->m_pPred=p1;
}

void PFListNode::Remove()
{
	ListNode *pSucc=m_pSucc;
	ListNode *pPred=m_pPred;
	
	ListLinker(pPred,pSucc);
}

LPVOID PFListNode::GetData() const
{
	return m_pData;
}
