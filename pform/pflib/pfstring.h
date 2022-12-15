/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pfstring.h
 *
 * Description:
 * String classes
 *======================================================================
 */
#ifndef PFLIB_PFSTRING_H
#define PFLIB_PFSTRING_H

#include <string.h>
#include "pffastbuffer.h"
#include "pfboolean.h"
#include "pfarray.h"

enum eOutputMode {eQuiet=0, eGFFactors=1, eNormal=2, eVerbose=3};

template<class T>
class PFGenericString : public PFFastBuffer<T>
{
   DWORD dwLength; // Phil

   // Phil - initialises the dwLength member the dumb way
   void SetLength();

   // Phil - initialises the invariant the trusting way
   void SetLength(DWORD dwForcedLength);

   static DWORD LengthHelper(const T* pT);
   PFGenericString<T> SlicerHelper(DWORD dwStart,DWORD dwEnd) const;
   int ComparisonHelper(const PFGenericString<T> &) const;

public:
   PFGenericString();
   PFGenericString(const T* pT);
   PFGenericString(const T& t);
   virtual ~PFGenericString();

   // Phil - Need an op
   PFGenericString<T>& operator=(const PFGenericString<T> &t2);

   // Phil - This is used to resize the buffer if necessary.
   void GuaranteeBuffer(DWORD dwSize);

   DWORD GetLength() const;
   PFBoolean IsEmpty() const;

   PFGenericString<T> &operator+=(const PFGenericString<T> &t2);
   PFGenericString<T> operator+(const PFGenericString<T> &t2) const;

   PFGenericString<T> Left(DWORD dwSize) const;
   PFGenericString<T> Mid(DWORD dwStart,DWORD dwCount=0x80000000) const;
   PFGenericString<T> Right(DWORD dwSize) const;

   PFBoolean operator==(const PFGenericString<T>&) const;
   PFBoolean operator!=(const PFGenericString<T>&) const;

   PFBoolean operator==(const T* pt) const;
   PFBoolean operator!=(const T* pt) const;

   operator const T*() const;

   void Set(DWORD dwValue,const PFBoolean &bHex=PFBoolean::b_false);
   void Set(long int iValue);

   DWORD Find(const PFGenericString<T> &tFind) const;

   int CompareNoCase(const PFGenericString<T> &t2) const;
   void ToUpper();
};

// Phil - preserves dwLength == GetLength() invariant.
template<class T>
void PFGenericString<T>::Set(DWORD dwValue,const PFBoolean &bHex)
{
   char sBuffer[12];
   if(bHex)
   {
      snprintf(sBuffer, sizeof(sBuffer), "%08lX", dwValue);
   }
   else
   {
      snprintf(sBuffer, sizeof(sBuffer), "%lu", dwValue);
   }

   GuaranteeBuffer(12);
   T* p=&this->AccessElement(0);

   char *pCopy=sBuffer;
   dwLength=0;
   while( ( (*p++)=(T)(*pCopy++)) != 0 ) // Phil - was NULL
      ++dwLength;
}

// Phil - preserves dwLength == GetLength() invariant.
template<class T>
void PFGenericString<T>::Set(long int iValue)
{
   char sBuffer[12];
   snprintf(sBuffer, sizeof(sBuffer), "%ld",iValue);

   GuaranteeBuffer(12);
   T* p=&this->AccessElement(0);

   char *pCopy=sBuffer;
   dwLength=0;
   while( ((*p++)=(T)(*pCopy++)) != 0 ) // Phil - was NULL
      ++dwLength;
}

// Phil - true const methods preserve all invariants
template<class T>
PFGenericString<T>::operator const T*() const
{
   return this->GetBasePointer();
}

template<class T>
int PFGenericString<T>::ComparisonHelper(const PFGenericString<T> &s) const
{
// a<b means a-b is negative
   DWORD dwIndex=0;
   int iRetval=0;

   while(iRetval==0) // until a decision is made
   {
      T t1=this->operator[](dwIndex);
      T t2=s.operator[](dwIndex);
      dwIndex++;

      if(t1<t2)
      {
         iRetval=-1;
      }
      else if(t1>t2)
      {
         iRetval=1;
      }
      else if(t1==0)
      {
         break;
      }
   }

   return(iRetval);
}

template<class T>
PFBoolean PFGenericString<T>::operator==(const PFGenericString<T>& s) const
{
   int iComparison=ComparisonHelper(s);
   return((iComparison==0)?PFBoolean::b_true:PFBoolean::b_false);
}

template<class T>
PFBoolean PFGenericString<T>::operator!=(const PFGenericString<T>& s) const
{
   int iComparison=ComparisonHelper(s);
   return((iComparison!=0)?PFBoolean::b_true:PFBoolean::b_false);
}

template<class T>
PFBoolean PFGenericString<T>::operator==(const T* pT) const
{
   PFGenericString<T> s(pT);
   return operator==(s);
}

template<class T>
PFBoolean PFGenericString<T>::operator!=(const T* pT) const
{
   PFGenericString<T> s(pT);
   return operator!=(s);
}

// Phil - initially dwLength == GetLength()
template<class T>
PFGenericString<T>::PFGenericString()
   : dwLength(0)  // Phil - the value 0 means GetLength() ==> 0
{
   GuaranteeBuffer(0);
}

//// Phil - brute-forces the invariant into the new string using SetLength()
template<class T>
PFGenericString<T> PFGenericString<T>::SlicerHelper(DWORD dwStart,DWORD dwEnd) const
{
   PFGenericString<T> sRetval;

   DWORD dwMyLength=GetLength();

   if(dwStart&0x80000000)
   {
      dwStart=0;
   }
   if(dwEnd>dwMyLength)
   {
      dwEnd=dwMyLength;
   }
   if(dwEnd>dwStart)
   {
      sRetval.GuaranteeBuffer(dwEnd-dwStart);
      T* p=&sRetval.AccessElement(0);
      memcpy(p,&this->operator[](dwStart),(dwEnd-dwStart)*sizeof(T));
      sRetval.SetLength();
   }
   return sRetval;
}

template<class T>
PFGenericString<T> PFGenericString<T>::Left(DWORD dwSize) const
{
   return SlicerHelper(0,dwSize);
}

template<class T>
PFGenericString<T> PFGenericString<T>::Mid(DWORD dwStart,DWORD dwCount) const
{
   return SlicerHelper(dwStart,dwStart+dwCount);
}

template<class T>
PFGenericString<T> PFGenericString<T>::Right(DWORD dwSize) const
{
   DWORD dwMyLength=GetLength();
   return SlicerHelper(dwMyLength-dwSize,dwMyLength);
}

// Phil - 0 has length 0, but "0" has length 1.
// Phil - static, no invariant. The 'dwLength' is not a member
// Jim  - bug fix.  if pT NULL, was crashing.  Now pT=NULL and pT="" work the same.
template<class T>
DWORD PFGenericString<T>::LengthHelper(const T* pT)
{
   DWORD dwTmpLength=0;
   if (pT)
   {
      // if NOT, then allocate normal len, but do NOT copy anything (i.e. the request is NULL).
      while(pT[dwTmpLength])
      {
         dwTmpLength++;
      }
   }
   return dwTmpLength;
}

// Phil - brute force initialisation of dwLength
template<class T>
void PFGenericString<T>::SetLength()
{
   dwLength = LengthHelper(this->GetBasePointer());
}

// Phil - blid faith initialisation of dwLength
template<class T>
void PFGenericString<T>::SetLength(DWORD dwForcedLength)
{
   dwLength = dwForcedLength;
}

// Phil - initially dwLength == GetLength()
template<class T>
PFGenericString<T>::PFGenericString(const T* pT)
   : dwLength(LengthHelper(pT)) // Phil - get member right from the outset
{
   GuaranteeBuffer(dwLength);
   T* p=&this->AccessElement(0);
   if(dwLength)
   {
      memcpy(p,pT,dwLength*sizeof(T));
   }
}

// Phil - initially dwLength == GetLength()
template<class T>
PFGenericString<T>::PFGenericString(const T& t)
   : dwLength(t!=0) // Phil - value 0 has length zero.
{
   GuaranteeBuffer(1);
   this->AccessElement(0)=t;
}

// Phil - invariant not really important in the destructor
template<class T>
PFGenericString<T>::~PFGenericString()
{
}

// Phil - must be able to copy ourself now we have a data member
// as well as parent data.
template<class T>
PFGenericString<T>& PFGenericString<T>::operator=(const PFGenericString<T> &t2)
{
   DWORD dwItsLength = t2.GetLength();
   PFFastBuffer<T>::operator=(t2);
   SetLength(dwItsLength);
   return *this;
}

// Phil - this even zero-terminates just beyond the end.
template<class T>
void PFGenericString<T>::GuaranteeBuffer(DWORD dwSize)
{
   this->AccessElement(dwSize)=0;
}

// Phil - and now a quick answer to the simple question...
template<class T>
DWORD PFGenericString<T>::GetLength() const
{
   //
   // const T* pT=GetBasePointer();
   // DWORD dwRealLength = LengthHelper(pT);
   // fprintf(stderr, "GetLength %s eliding helper, %li %li\n",
   //         dwRealLength==dwLength?"correct":"b0rked",
   //         dwRealLength, dwLength);
   // }
   return dwLength;
}

// Phil - correctly changes the dwLength member, assuming that
// the strings have not been shortened by clients.
// PFArray unfortunately needs non-const access still
template<class T>
PFGenericString<T> & PFGenericString<T>::operator+=(const PFGenericString<T> &t2)
{
   DWORD l2=t2.GetLength();
   if(l2)
   {
      DWORD l1=GetLength();
      GuaranteeBuffer(l1+l2);
      T* pNew=&this->AccessElement(0);
      const T* pNew2=t2.GetBasePointer();

      memcpy(pNew+l1,pNew2,l2*sizeof(T));
      SetLength(l1+l2);
   }
   return *this;
}

// Phil - this preserves the invariant, however, it's inefficient as first it
// takes a copy of the first string, and then it realises that it needs to
// enlarge the buffer to fit the second string in.
template<class T>
PFGenericString<T> PFGenericString<T>::operator+(const PFGenericString<T> &t2) const
{
   PFGenericString t(*this);
   t+=t2;
   return t;
}

template<class T>
PFBoolean PFGenericString<T>::IsEmpty() const
{
   DWORD dwMyLength=GetLength();
   return((dwMyLength==0)?PFBoolean::b_true:PFBoolean::b_false);
}

template<class T>
DWORD PFGenericString<T>::Find(const PFGenericString<T> &tFind) const
{
   return PFFastBuffer<T>::Find(tFind,tFind.GetLength(),GetLength());
}

template<class T>
int PFGenericString<T>::CompareNoCase(const PFGenericString<T> &t2) const
{
   PFGenericString<T> q1(*this);
   PFGenericString<T> q2(t2);

   q1.ToUpper();
   q2.ToUpper();

   return(q1.ComparisonHelper(q2));
}

template<class T>
void PFGenericString<T>::ToUpper()
{
   DWORD dwMyLength=GetLength();
   const T ta=(T)'a';
   const T tz=(T)'z';
   const T tA=(T)'A';

   for(DWORD dw=0;dw<dwMyLength;dw++)
   {
      T t=this->operator[](dw);
      if((t>=ta)&&(t<=tz))
      {
         // Phil - gcc didn't let const and non-const [] coexist
         this->AccessElement(dw)=(T) (t+(tA-ta));
      }
   }
}

typedef PFGenericString<char> PFString8;
typedef PFGenericString<unsigned short> PFString16;
typedef PFGenericString<unsigned long> PFString32;
typedef PFString8 PFString;
typedef PFArray<PFBuffer<PFString>,PFString> PFStringArray;

#endif
