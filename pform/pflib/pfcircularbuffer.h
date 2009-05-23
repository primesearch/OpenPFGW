/*================== PrimeForm (c) 1999-2000 ===========================
 * File: pfcircularbuffer.h
 *
 * Description:
 * A circular buffer object
 *======================================================================
 */
#include "pfarray.h"
#include "pffastbuffer.h"
#include "pfboolean.h"

class PFCircularBuffer : public PFArray<PFFastBuffer<BYTE>,BYTE>
{
	DWORD m_dwHead;
	DWORD m_dwTail;
	DWORD m_dwBufferSize;
	
	void NormalizeBuffer();
public:
	PFCircularBuffer(DWORD dwSize=4096);
	virtual ~PFCircularBuffer();
	
	PFBoolean ReadData(LPBYTE pData,DWORD dwCount);
	PFBoolean WriteData(LPBYTE pData,DWORD dwCount);
	
	DWORD GetBufferFree() const;
	DWORD GetBufferContent() const;
	DWORD GetBufferSize() const;
	
	LPCBYTE GetReadPointer() const;
	LPBYTE GetWritePointer();

	void MarkBytesRead(DWORD dwRead);
	void MarkBytesWritten(DWORD dwWritten);
};
