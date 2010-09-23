#include "pfoopch.h"
#include "factornode.h"

FactorNode::FactorNode(const Integer &ff,unsigned long pp)
   : f(ff), p(pp), child1(NULL), child2(NULL), parent(NULL)
{
}

FactorNode::FactorNode(FactorNode *p1,FactorNode *p2) :
   f(p1->f), p(1), child1(p1), child2(p2), parent(NULL)
{
   f*=p2->f;

   p1->parent=this;
   p2->parent=this;
}

FactorNode::~FactorNode()
{
   if(child1) delete child1;
   if(child2) delete child2;
}

FactorNode::FactorNode(const FactorNode &fn)
   : f(fn.f), p(fn.p), child1(NULL), child2(NULL), parent(NULL)
{
}

int FactorNode::compare(const FactorNode &ff,const PFBoolean &isPowerMode)
{
// in power mode, bigger ones go to the top
   if(isPowerMode)
   {
      return((power()>ff.power())?1:0);
   }
// in factor mode, smaller ones go to the top
   else
   {
      return((f<ff.f)?1:0);
   }
}

void FactorNode::detach()
{
    if(parent!=this)
    {
        if(parent->child1==this)
            parent->child1=NULL;
        else
            parent->child2=NULL;        // detach from the parent
    }
    parent=NULL;
}

void FactorNode::makeRoot()
{
    parent=this;
}

void FactorNode::deleteRoot()
{
    if(parent==this)
        delete this;
}

// calculate the depth of a factor tree
int FactorNode::treeDepth() const
{
   int t1=0,t2=0;
   if(child1) t1=child1->treeDepth();
   if(child2) t2=child2->treeDepth();
   if(t1<t2) t1=t2;
   return(t2+1);
}

// calculate the "power depth" of a factor tree
unsigned long FactorNode::powerDepth() const
{
   unsigned long t1=0,t2=0;
   if(!isLeaf())
   {
      t1=child1->powerDepth()+lg(child2->f)+1;     // get to node 1 and everafter
      t2=child2->powerDepth()+lg(child1->f)+1;
   }
   else t1=lg(f)+1;                          // at a child node just call this again
   return(t1+t2);
}

unsigned long FactorNode::powval() const {return p;}
Integer &FactorNode::prime() {return f;}
Integer FactorNode::power() const
{
   Integer x=pow(f,p);
   return(x);
}

PFBoolean FactorNode::isLeaf() const
{
    PFBoolean bRetval=PFBoolean::b_true;
    if(child1 || child2)
        bRetval=PFBoolean::b_false;
    return(bRetval);
}
