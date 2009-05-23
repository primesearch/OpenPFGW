#include "pfboolean.h"
#include "ipfsystem.h"
#include "pfptrarray.h"

#include <be/app/Looper.h>

class BPFEvent
{
	PFBoolean m_bState;
	PFBoolean m_bManual;
	
	int m_iReferenceCount;
	int m_iKernelCount;

	friend class BPFSystem;
	friend class BPFThread;
private:
	virtual ~BPFEvent();
protected:
	BPFEvent(const PFBoolean &bState=PFBoolean::b_false,const PFBoolean &bManual=PFBoolean::b_false);
	
	void AddRef();
	void ReleaseRef();
	void AddKernelRef();
	void ReleaseKernelRef();
	
	void SetEvent();
	void ResetEvent();	

	const PFBoolean &IsSet() const;
	const PFBoolean &IsManual() const;
	
	PFHANDLE AsHandle() const;	
};

class BPFThread
{
	BPFEvent *m_pThreadEvent;
	THREAD_FUNCTION m_pfEntry;
	LPVOID m_pParam;
	PFHANDLE m_hClone;
private:
	BPFThread(const BPFThread &);
	BPFThread &operator=(const BPFThread &);

protected:
	friend int32 thread_entry_point(LPVOID);
	void Startup();

	friend class BPFSystem;
	BPFThread(THREAD_FUNCTION,LPVOID);
public:
	virtual ~BPFThread();
	
	PFHANDLE AsHandle() const;
};

#define	WAIT_OBJECT_0	0
#define	WAIT_UNKNOWN	0xFFFFFFFFUL
#define	WAIT_FAILED		0x80000000UL
#define	WAIT_TIMEOUT	0x7FFFFFFFUL

class WaitEventMessage
{
	PFHANDLE *m_pHandles;		// the list of handles waiting on
	DWORD m_dwCount;					// the number of handles waiting on
	DWORD m_dwRetval;
	PFBoolean m_bWaitAll;		// whether we are waiting for all of them
	
	bigtime_t	m_btStartTime;	// when we received the request
	bigtime_t	m_btDuration;	// how long this request is to remain
	sem_id		m_smBlock;		// how we are blocking the caller
	
	WaitEventMessage(const WaitEventMessage &);
	WaitEventMessage &operator=(const WaitEventMessage &);
public:
	WaitEventMessage(PFHANDLE *phandles,DWORD dwCount,const PFBoolean &bWaitAll,DWORD dwTimeout);
	virtual ~WaitEventMessage();
	
	DWORD GetCount() const;
	PFHANDLE GetItem(DWORD dwItem) const;

	void SetReturnCode(DWORD dwRetval);
	DWORD GetReturnCode() const;
	
	void AcquireSemaphore();
	void ReleaseSemaphore();
	
	const PFBoolean &WaitAll() const;
	
	const bigtime_t &GetDuration() const;
	const bigtime_t &GetStart() const;
};

class BPFSystem : public BLooper, public IPFSystem
{
	PFPtrArray<WaitEventMessage>	m_wemList;
	BMessageRunner *m_pMessageRunner;
	BMessenger *m_pMessenger;
	BMessage *m_pTimeoutMessage;
private:
	virtual ~BPFSystem();
	
	void MessageReceived(BMessage *pMessage);
	
	BPFSystem(const BPFSystem &);
	BPFSystem &operator=(const BPFSystem &);
	
public:
	BPFSystem();
	
	virtual void _StartupSystem();
	virtual void _ShutdownSystem();
	
	virtual PFHANDLE _CreateEvent(const PFBoolean &bState=PFBoolean::b_false,const PFBoolean &bManual=PFBoolean::b_false);
	virtual PFHANDLE _CreateThread(THREAD_FUNCTION pF,LPVOID pParam);
	virtual void _SetEvent(PFHANDLE hHandle);
	virtual void _ResetEvent(PFHANDLE hHandle);
	virtual void _CloseHandle(PFHANDLE hHandle);
	virtual PFHANDLE _DuplicateHandle(PFHANDLE hhandle);	
	virtual DWORD _WaitForSingleObject(PFHANDLE hWait,DWORD dwTimeout=TIMEOUT_INFINITE);
	virtual DWORD _WaitForMultipleObjects(PFHANDLE *ahWait,DWORD dwCount,const PFBoolean &bWaitAll=PFBoolean::b_true,DWORD dwTimeout=TIMEOUT_INFINITE);
};

#define	BPF_SET				'bpfs'
#define	BPF_RESET			'bpfr'
#define	BPF_WAIT				'bpfw'
#define	BPF_TICK				'bpft'
#define	BPF_ADDREF			'bpf+'
#define	BPF_DEREF			'bpf-'

#define	BPF_WAIT_OBJECT	"wwww"
#define	BPF_EVENT_OBJECT	"eeee"



// Note how we use WaitEventMessage. 
// Create the object and send it in a message.
// receive it and stick it on the queue.
// every time that BLooper gets a message
//		it checks all pending WaitEventMessages
//		it checks to see if any timers need to be set up
// Any Waits that need to be fired do so by releasing the sem
	
	



