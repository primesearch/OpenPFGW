/*================== PrimeForm (c) 1999-2000 ===========================
 * File: listbase.h
 *
 * Description:
 * Base structures for linked lists
 *======================================================================
 */
#ifndef PFLIB_LISTBASE_H
#define PFLIB_LISTBASE_H

typedef struct _ListNode
{
	struct _ListNode* m_pSucc;
	struct _ListNode* m_pPred;
	LPVOID m_pData;
}
ListNode;

typedef struct
{
	struct _ListNode* m_pHead;
	struct _ListNode* m_pTail;
	struct _ListNode* m_pTailpred;
}
ListHead;

#endif
