#ifndef FACTORNODE_H
#define FACTORNODE_H

class FactorNode
{
	Integer f;	// the factor
	DWORD p;		// the power to which it occurs
	
	friend class Exponentiator;
protected:
	FactorNode *child1;
	FactorNode *child2;	// binary tree factors
   FactorNode *parent;
public:
	FactorNode(const Integer &ff,DWORD p);
	FactorNode(FactorNode *p1,FactorNode *p2);
	~FactorNode();
	
	FactorNode(const FactorNode &fn);
	FactorNode &operator=(const FactorNode &fn);

	int compare(const FactorNode &,const PFBoolean &bUsePower);	// returns 1 if this node is nearer the top

	DWORD		powval() const;
	Integer &prime();
	Integer power() const;

   int treeDepth() const;
   DWORD powerDepth() const;

   PFBoolean isLeaf() const;              // checks to see if this is a leaf node

	void detach();
   void makeRoot();
   void deleteRoot();
};

typedef FactorNode *FNP;
#endif
