#ifdef PFGW_BEOS_FULL

#include "pflibpch.h"
#include "ipfsystem.h"
#include "bpfsystem.h"

IPFSystem *IPFSystem::m_pSystem=NULL;

IPFSystem::IPFSystem()
{
}

IPFSystem::~IPFSystem()
{
}

IPFSystem *IPFSystem::StartupSystem()
{
   m_pSystem=new BPFSystem;
   m_pSystem->_StartupSystem();
   return m_pSystem;
}

void IPFSystem::ShutdownSystem()
{
   m_pSystem->_ShutdownSystem();
   m_pSystem=NULL;
}

PFHANDLE IPFSystem::CreateEvent(const PFBoolean &bState,const PFBoolean &bManual)
{
   return m_pSystem->_CreateEvent(bState,bManual);
}

PFHANDLE IPFSystem::CreateThread(THREAD_FUNCTION pF,LPVOID pParam)
{
   return m_pSystem->_CreateThread(pF,pParam);
}

void IPFSystem::SetEvent(PFHANDLE hHandle)
{
   m_pSystem->_SetEvent(hHandle);
}

void IPFSystem::ResetEvent(PFHANDLE hHandle)
{
   m_pSystem->_ResetEvent(hHandle);
}

void IPFSystem::CloseHandle(PFHANDLE hHandle)
{
   m_pSystem->_CloseHandle(hHandle);
}

PFHANDLE IPFSystem::DuplicateHandle(PFHANDLE hHandle)
{
   return m_pSystem->_DuplicateHandle(hHandle);
}

DWORD IPFSystem::WaitForSingleObject(PFHANDLE hWait,DWORD dwTimeout)
{
   return m_pSystem->_WaitForSingleObject(hWait,dwTimeout);
}

DWORD IPFSystem::WaitForMultipleObjects(PFHANDLE *ahWait,DWORD dwCount,const PFBoolean &bWaitAll,DWORD dwTimeout)
{
   return m_pSystem->_WaitForMultipleObjects(ahWait,dwCount,bWaitAll,dwTimeout);
}

#endif      /* PFGW_BEOS */
