#ifndef PFLIB_IPFSYSTEM_H
#define PFLIB_IPFSYSTEM_H
#include "pfboolean.h"
// Interface for PrimeForm systems.
// This code is the platform-independent interface for platform
// specific system code.
typedef LPVOID PFHANDLE;
const DWORD TIMEOUT_INFINITE=0xFFFFFFFF;

// IPFSystem
// The PrimeForm system object

// The mainstay of this is the thread-event table. Whenever a thread is put into a waiting state, the events it is
// waiting on are written to the thread record. The threads waiting on a given event could be written in an event
// record. But the idea is to instigate a system thread with a message port to catch these requests

typedef DWORD (*THREAD_FUNCTION)(LPVOID);

class IPFSystem
{
	static IPFSystem *m_pSystem;
public:
	IPFSystem();
	virtual ~IPFSystem();
	
	static IPFSystem *StartupSystem();
	static void ShutdownSystem();

	static PFHANDLE CreateEvent(const PFBoolean &bState=PFBoolean::b_false,const PFBoolean &bManual=PFBoolean::b_false);
	static PFHANDLE CreateThread(THREAD_FUNCTION pF,LPVOID pParam);	
	static void SetEvent(PFHANDLE hHandle);
	static void ResetEvent(PFHANDLE hHandle);
	static void CloseHandle(PFHANDLE hhandle);
	static PFHANDLE DuplicateHandle(PFHANDLE hHandle);
	static DWORD WaitForSingleObject(PFHANDLE hWait,DWORD dwTimeout=TIMEOUT_INFINITE);
	static DWORD WaitForMultipleObjects(PFHANDLE *ahWait,DWORD dwCount,const PFBoolean &bWaitAll=PFBoolean::b_true,DWORD dwTimeout=TIMEOUT_INFINITE);
private:
	virtual void _StartupSystem()=0;
	virtual void _ShutdownSystem()=0;
	virtual PFHANDLE _CreateEvent(const PFBoolean &bState=PFBoolean::b_false,const PFBoolean &bManual=PFBoolean::b_false)=0;
	virtual PFHANDLE _CreateThread(THREAD_FUNCTION pF,LPVOID pParam)=0;
	virtual void _SetEvent(PFHANDLE hHandle)=0;
	virtual void _ResetEvent(PFHANDLE hHandle)=0;
	virtual void _CloseHandle(PFHANDLE hHandle)=0;
	virtual PFHANDLE _DuplicateHandle(PFHANDLE hhandle)=0;
	virtual DWORD _WaitForSingleObject(PFHANDLE hWait,DWORD dwTimeout=TIMEOUT_INFINITE)=0;
	virtual DWORD _WaitForMultipleObjects(PFHANDLE *ahWait,DWORD dwCount,const PFBoolean &bWaitAll=PFBoolean::b_true,DWORD dwTimeout=TIMEOUT_INFINITE)=0;
};
#endif
