/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pfboolean.h
 *
 * Description:
 * PrimeForm safe boolean datatype
 *======================================================================
 */
#ifndef PFLIB_PFBOOLEAN_H
#define PFLIB_PFBOOLEAN_H

class PFBoolean
{
public:
	enum TruthValue
	{
		b_false=0,
		b_true=0xFFFFFFFF
	};
private:
	TruthValue m_bValue;
public:
	PFBoolean(const TruthValue &bTruth=b_false);
	virtual ~PFBoolean();
	
	PFBoolean(const PFBoolean &pfb);
	PFBoolean &operator=(const PFBoolean &pfb);
	operator bool() const;
	PFBoolean & operator &=(const PFBoolean &b);
	PFBoolean & operator !=(const PFBoolean &b);
};
#endif
