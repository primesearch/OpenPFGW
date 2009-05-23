/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pflistnode.h
 *
 * Description:
 * A list node
 *======================================================================
 */
#ifndef PFLIB_PFLISTNODE_H
#define PFLIB_PFLISTNODE_H

#include "listbase.h"

class PFListNode : public ListNode
{
public:
	PFListNode(LPVOID pData=NULL);
	virtual ~PFListNode();

	PFListNode *Next() const;
	PFListNode *Previous() const;
	
	void Remove();
	LPVOID GetData() const;
	
	static void ListLinker(ListNode *p1,ListNode *p2);
};

#endif
