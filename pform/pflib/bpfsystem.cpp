#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#ifdef PFGW_BEOS_FULL

// The BLooper below manages all requests for event handles
// wait events
#include "pflibpch.h"
#include "bpfsystem.h"

#include <be/app/Message.h>
#include <be/app/Messenger.h>
#include <be/app/MessageRunner.h>

// events
BPFEvent::BPFEvent(const PFBoolean &bState,const PFBoolean &bManual)
   : m_bState(bState), m_bManual(bManual), m_iReferenceCount(1), m_iKernelCount(0)
{
}

BPFEvent::~BPFEvent()
{
}

PFHANDLE BPFEvent::AsHandle() const
{
   return (PFHANDLE)this;
}

void BPFEvent::AddRef()
{
   m_iReferenceCount++;
}

void BPFEvent::ReleaseRef()
{
   m_iReferenceCount--;
   if(m_iReferenceCount==0)
   {
      m_bState=PFBoolean::b_true;
      if(m_iKernelCount==0)
      {
         delete this;
      }
   }
}

void BPFEvent::AddKernelRef()
{
   m_iKernelCount++;
}

void BPFEvent::ReleaseKernelRef()
{
   m_iKernelCount--;
   if(m_iKernelCount==0)
   {
      if(m_iReferenceCount==0)
      {
         delete this;
      }
   }
}

void BPFEvent::SetEvent()
{
   m_bState=PFBoolean::b_true;
}

void BPFEvent::ResetEvent()
{
   m_bState=PFBoolean::b_false;
}

const PFBoolean &BPFEvent::IsSet() const
{
   return m_bState;
}

const PFBoolean &BPFEvent::IsManual() const
{
   return m_bManual;
}

// threads
// A thread is a tricky thing.
int32 thread_entry_point(LPVOID pParam)
{
   BPFThread *pThread=(BPFThread*)pParam;
   pThread->Startup();
}

void BPFThread::Startup()
{
   PFHANDLE hHandle=AsHandle();
   m_hClone=IPFSystem::DuplicateHandle(hHandle);

   DWORD dwRetval=(m_pfEntry)(m_pParam);

   IPFSystem::SetEvent(hHandle);    // set thread complete event
   IPFSystem::CloseHandle(hHandle);
   delete this;
}

BPFThread::BPFThread(THREAD_FUNCTION pF,LPVOID pParam)
   : m_pThreadEvent(NULL), m_pfEntry(pF), m_pParam(pParam), m_hClone(NULL)
{
   m_pThreadEvent=new BPFEvent(PFBoolean::b_false,PFBoolean::b_true);

   // spawn the thread
   thread_id thID=spawn_thread(thread_entry_point,"pfthread",B_NORMAL_PRIORITY,(LPVOID)this);
   resume_thread(thID);
}

BPFThread::~BPFThread()
{
}

PFHANDLE BPFThread::AsHandle() const
{
   return m_pThreadEvent->AsHandle();
}

// wait event message
WaitEventMessage::WaitEventMessage(PFHANDLE *p,DWORD dwCount,const PFBoolean &bWaitAll,DWORD dwTimeout)
   : m_pHandles(p), m_dwCount(dwCount), m_dwRetval(WAIT_UNKNOWN), m_bWaitAll(bWaitAll),
      m_btStartTime(0), m_btDuration(B_INFINITE_TIMEOUT), m_smBlock(0)
{
   m_btStartTime=system_time();
   if(dwTimeout!=TIMEOUT_INFINITE)
   {
      m_btDuration=dwTimeout;
      m_btDuration*=1000;
   }
   m_smBlock=create_sem(1,"pfsem");
}

WaitEventMessage::~WaitEventMessage()
{
   delete_sem(m_smBlock);
}

const PFBoolean &WaitEventMessage::WaitAll() const
{
   return m_bWaitAll;
}

DWORD WaitEventMessage::GetCount() const
{
   return m_dwCount;
}

PFHANDLE WaitEventMessage::GetItem(DWORD dw) const
{
   return m_pHandles[dw];
}

const bigtime_t &WaitEventMessage::GetStart() const
{
   return m_btStartTime;
}

const bigtime_t &WaitEventMessage::GetDuration() const
{
   return m_btDuration;
}

void WaitEventMessage::AcquireSemaphore()
{
   acquire_sem(m_smBlock);
}

void WaitEventMessage::ReleaseSemaphore()
{
   release_sem(m_smBlock);
}

DWORD WaitEventMessage::GetReturnCode() const
{
   return m_dwRetval;
}

void WaitEventMessage::SetReturnCode(DWORD dw)
{
   m_dwRetval=dw;
}

// system
template<class T>
T* msgpointer(BMessage *p,LPCTSTR s)
{
   void *x;
   p->FindPointer(s,&x);
   return (T*)x;
}

BPFEvent *eventpointer(BMessage *p)
{
   return msgpointer<BPFEvent>(p,BPF_EVENT_OBJECT);
}

WaitEventMessage* waitpointer(BMessage *p)
{
   return msgpointer<WaitEventMessage>(p,BPF_WAIT_OBJECT);
}

BPFSystem::BPFSystem() : BLooper(NULL,B_NORMAL_PRIORITY,60),
   m_wemList(PFBoolean::b_false), m_pMessageRunner(NULL), m_pMessenger(NULL), m_pTimeoutMessage(NULL)
{
   m_pMessenger=new BMessenger(NULL,this);
   m_pTimeoutMessage=new BMessage(BPF_TICK);
   Run();
}

BPFSystem::~BPFSystem()
{
   delete m_pTimeoutMessage;
   delete m_pMessenger;
   if(m_pMessageRunner)
   {
      delete m_pMessageRunner;
   }
}

void BPFSystem::_StartupSystem()
{
}

void BPFSystem::_ShutdownSystem()
{
   BMessage bM(B_QUIT_REQUESTED);
   m_pMessenger->SendMessage(&bM);
}

void BPFSystem::MessageReceived(BMessage *pMessage)
{
   PFBoolean bHandled=PFBoolean::b_true;

   switch(pMessage->what)
   {
   case BPF_ADDREF:
      {
         BPFEvent *pEvent=eventpointer(pMessage);
         pEvent->AddRef();
      }
      break;

   case BPF_DEREF:
      {
         BPFEvent *pEvent=eventpointer(pMessage);
         pEvent->ReleaseRef();
      }
      break;

   case BPF_SET:
      {
         BPFEvent *pEvent=eventpointer(pMessage);
         pEvent->SetEvent();
      }
      break;

   case BPF_RESET:
      {
         BPFEvent *pEvent=eventpointer(pMessage);
         pEvent->ResetEvent();
      }
      break;

   case BPF_WAIT:
      {
         WaitEventMessage *pWaitEvent=waitpointer(pMessage);
         m_wemList.Add(pWaitEvent);
         // when a wait is added you *must* increment kernel references
         for(DWORD dwItem=0;dwItem<pWaitEvent->GetCount();dwItem++)
         {
            PFHANDLE hItem=pWaitEvent->GetItem(dwItem);
            BPFEvent *pEvent=(BPFEvent*)hItem;
            pEvent->AddKernelRef();
         }
         pWaitEvent->AcquireSemaphore();
      }
      break;

   case BPF_TICK:
      // just a kick in the pants to wake you up
      break;

   case B_QUIT_REQUESTED:
      // a quit message is a special case, because it needs to do cleanup
      {
         for(DWORD dwIndex=0;dwIndex<m_wemList.GetSize();dwIndex++)
         {
            WaitEventMessage *pWaitEvent=m_wemList[dwIndex];
            pWaitEvent->SetReturnCode(WAIT_FAILED);
            pWaitEvent->ReleaseSemaphore();
         }
         m_wemList.RemoveAll();

         BLooper::MessageReceived(pMessage);
         bHandled=PFBoolean::b_false;
      }
      break;

   default:
      BLooper::MessageReceived(pMessage);
      bHandled=PFBoolean::b_false;
   }

// check out any events that need to be fired. Note events take priority
// over timeout

   if(bHandled)
   {
      bigtime_t btDelay=0;

      do
      {
         bigtime_t btCheckTime=B_INFINITE_TIMEOUT;

         for(DWORD dwIndex=0;dwIndex<m_wemList.GetSize();dwIndex++)
         {
            DWORD dwReturnCode=WAIT_UNKNOWN;
            WaitEventMessage *pWaitEvent=m_wemList[dwIndex];

            DWORD dwItem;
            DWORD dwFired=0;
            DWORD dwNeeded=pWaitEvent->GetCount();
            if(pWaitEvent->WaitAll())
            {
               dwNeeded=1;
            }

            for(dwItem=0;dwItem<pWaitEvent->GetCount();dwItem++)
            {
               PFHANDLE hItem=pWaitEvent->GetItem(dwItem);
               BPFEvent *pEvent=(BPFEvent*)hItem;

               if(pEvent->IsSet())
               {
                  dwFired++;
                  if(dwFired==dwNeeded) break;
               }
            }

            if(dwFired==dwNeeded)
            {
               dwReturnCode=dwItem;
               // this event fired. So you need to process any autoresets
               for(dwItem=0;(dwFired)&&(dwItem<=dwReturnCode);dwItem++)
               {
                  PFHANDLE hItem=pWaitEvent->GetItem(dwItem);
                  BPFEvent *pEvent=(BPFEvent*)hItem;

                  if(pEvent->IsSet())
                  {
                     dwFired--;
                     if(!pEvent->IsManual())
                     {
                        pEvent->ResetEvent();
                     }
                  }
               }
            }
            else
            {
               // check if this event has a timeout
               bigtime_t btDuration=pWaitEvent->GetDuration();
               if(btDuration!=B_INFINITE_TIMEOUT)
               {
                  // calculate the end time
                  bigtime_t btEnd=btDuration+pWaitEvent->GetStart();
                  if(btEnd>system_time())
                  {
                     dwReturnCode=WAIT_TIMEOUT;
                  }
                  else if((btCheckTime==B_INFINITE_TIMEOUT)||(btEnd<btCheckTime))
                  {
                     btCheckTime=btEnd;
                  }
               }
            }

            if(dwReturnCode!=WAIT_UNKNOWN)
            {
               // release kernel references
               for(dwItem=0;dwItem<pWaitEvent->GetCount();dwItem++)
               {
                  PFHANDLE hItem=pWaitEvent->GetItem(dwItem);
                  BPFEvent *pEvent=(BPFEvent*)hItem;
                  pEvent->ReleaseKernelRef();
               }

               pWaitEvent->SetReturnCode(dwReturnCode);
               pWaitEvent->ReleaseSemaphore();
               m_wemList.RemoveAt(dwIndex);
            }
            else
            {
               dwIndex++;
            }
         }

         // btCheckTime is now the time for the next check

         if(btCheckTime!=B_INFINITE_TIMEOUT)
         {
            btDelay=btCheckTime-system_time();
         }
         else
         {
            btDelay=0;
         }
      }
      while(btDelay<=0);

      // btDelay is now a reasonable time to fire the delay off.
      if(m_pMessageRunner)
      {
         delete m_pMessageRunner;
         m_pMessageRunner=NULL;
      }

      if(btDelay>0)
      {
         // fire off a timeout message coming right at us
         m_pMessageRunner=new BMessageRunner(*m_pMessenger,m_pTimeoutMessage,btDelay,1);
      }
   }
}

PFHANDLE BPFSystem::_CreateEvent(const PFBoolean &bState,const PFBoolean &bManual)
{
   BPFEvent *pEvent=new BPFEvent(bState,bManual);
   return pEvent->AsHandle();
}

PFHANDLE BPFSystem::_CreateThread(THREAD_FUNCTION pF,LPVOID pParam)
{
   BPFThread *pThread=new BPFThread(pF,pParam);
   return pThread->AsHandle();
}

void BPFSystem::_SetEvent(PFHANDLE hHandle)
{
   BMessage bM(BPF_SET);
   // set who?
   bM.AddPointer(BPF_EVENT_OBJECT,hHandle);
   m_pMessenger->SendMessage(&bM);
}

void BPFSystem::_ResetEvent(PFHANDLE hHandle)
{
   BMessage bM(BPF_RESET);
   bM.AddPointer(BPF_EVENT_OBJECT,hHandle);
   m_pMessenger->SendMessage(&bM);
}

void BPFSystem::_CloseHandle(PFHANDLE hHandle)
{
   BMessage bM(BPF_DEREF);
   bM.AddPointer(BPF_EVENT_OBJECT,hHandle);
   m_pMessenger->SendMessage(&bM);
}

PFHANDLE BPFSystem::_DuplicateHandle(PFHANDLE hHandle)
{
   BMessage bM(BPF_ADDREF);
   bM.AddPointer(BPF_EVENT_OBJECT,hHandle);
   m_pMessenger->SendMessage(&bM);
   return hHandle;
}

DWORD BPFSystem::_WaitForSingleObject(PFHANDLE hWait,DWORD dwTimeout)
{
   WaitEventMessage wem(&hWait,1,PFBoolean::b_true,dwTimeout);
   BMessage bM(BPF_WAIT);
   bM.AddPointer(BPF_WAIT_OBJECT,&wem);
   m_pMessenger->SendMessage(&bM);
   wem.AcquireSemaphore();
   return wem.GetReturnCode();
}

DWORD BPFSystem::_WaitForMultipleObjects(PFHANDLE *ahWait,DWORD dwCount,const PFBoolean &bWaitAll,DWORD dwTimeout)
{
   WaitEventMessage wem(ahWait,dwCount,bWaitAll,dwTimeout);
   BMessage bM(BPF_WAIT);
   bM.AddPointer(BPF_WAIT_OBJECT,&wem);
   m_pMessenger->SendMessage(&bM);
   wem.AcquireSemaphore();
   return wem.GetReturnCode();
}

#endif   /* PFGW_BEOS */
