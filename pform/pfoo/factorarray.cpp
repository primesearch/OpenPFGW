#include "pfoopch.h"
#include "factorarray.h"
#include "factornode.h"

FactorArray::FactorArray()
   : fA(NULL), nxtFree(0), nxtAvail(0)
{
}

FactorArray::~FactorArray()
{
// for(int i=0;i<nxtFree;i++) delete fA[i];
   if(fA) delete[] fA;
}



FactorNode *& FactorArray::operator[](int x)
{
   if(x>nxtAvail) // the array is too small
   {
      int nA=nxtAvail;
      if(nA==0) nA=1;
      while(x>nA) nA<<=1;     // this is the new array size

      FactorNode **nAA=new FNP[nA];
      for(int i=0;i<nxtAvail;i++) nAA[i]=fA[i];

      if(fA) delete[] fA;
      fA=nAA;
      nxtAvail=nA;
   }
   return(fA[x-1]);
}

void FactorArray::add(FactorNode *f,const PFBoolean &isPowerMode)
{
   int child=++nxtFree;
   operator[](child)=f;    // grow the heap if needed
   int parent;

   while((parent=child>>1)!=0)
   {
// Am I better than the parent?
      if(f->compare(*operator[](parent),isPowerMode))
      {
         operator[](child)=operator[](parent);
         child=parent;
      }
      else break;
   }
   operator[](child)=f;
}

void FactorArray::append(FactorNode *f)
{
   operator[](++nxtFree)=f;
}

void FactorArray::makeHeap(const PFBoolean &isPowerMode)
{
    int oldFree=nxtFree;
    nxtFree=0;              // detach the array

    for(int i=1;i<=oldFree;i++)
    {
        FactorNode *pn=operator[](i);
        add(pn,isPowerMode);
    }
    // that should do it
}

int FactorArray::heapsize()
{
   return nxtFree;
}

FactorNode *FactorArray::remove(const PFBoolean &isPowerMode)
{
   FactorNode *r=NULL;
   if(nxtFree==0) return(r);
   r=operator[](1);

   int parent=1;
   int child;
   FactorNode *f=operator[](nxtFree--);
// parent is the position for reinsertion of f

   while((child=parent<<1)<=nxtFree)
   {
// if there are two kids, get the bigger one
      if(child<nxtFree)
         if(operator[](child+1)->compare(*operator[](child),isPowerMode))
            child++;
      if(f->compare(*operator[](child),isPowerMode))
         break;                        // this'll do

      operator[](parent)=operator[](child);
      parent=child;
   }
   operator[](parent)=f;
   return(r);
}
