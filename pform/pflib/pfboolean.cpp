/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pfboolean.cpp
 *
 * Description:
 * PrimeForm safe boolean datatype
 *======================================================================
 */

#include "pflibpch.h"
#include "pfboolean.h"

/*======================================================================
 * METHOD: PFBoolean::PFBoolean
 * PURPOSE:
 * constructor
 * PARAMETERS:
 * const TruthValue (default false)
 * RETURNS:
 * implicit
 *======================================================================
 */
PFBoolean::PFBoolean(const TruthValue &bTruth)
   : m_bValue(bTruth)
{
}

PFBoolean::~PFBoolean()
{
}

PFBoolean::PFBoolean(const PFBoolean &pfb)
   : m_bValue(pfb.m_bValue)
{
}

PFBoolean &PFBoolean::operator=(const PFBoolean &pfb)
{
   m_bValue=pfb.m_bValue;
   return *this;
}

PFBoolean::operator bool() const
{
   return(m_bValue==b_true);
}

PFBoolean &PFBoolean::operator&=(const PFBoolean &b)
{
   m_bValue=(m_bValue?b.m_bValue:m_bValue);
   return *this;
}

PFBoolean &PFBoolean::operator!=(const PFBoolean &b)
{
   m_bValue=(m_bValue?m_bValue:b.m_bValue);
   return *this;
}



