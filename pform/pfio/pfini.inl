// Inlined functions for PFIni class (and helpers)

#define PFIO_INLINE inline

#if defined (_MSC_VER)
void PFIO_INLINE PFIni::CS_Lock()
{
	EnterCriticalSection(&m_sCS);
}

void PFIO_INLINE PFIni::CS_Release()
{
	LeaveCriticalSection(&m_sCS);
}

void PFIO_INLINE PFIni::CS_Init()
{
	// Note when starting out, m_CntCS is -1, so if the incemented return is 0, then
	// we are the "first" thread to increment the variable.  All Win32 systems will
	// correctly return 0.
	if (InterlockedIncrement(&m_CntCS) == 0)
		InitializeCriticalSection(&m_sCS);
}

void PFIO_INLINE PFIni::CS_Free()
{
	// Note that m_CntCS == -1 means that we are the last thread to decrement m_CntCS, so
	// we need to free it.  NOTE that Win95/WinNT3.51 may not return -1, but WILL return a
	// negative value.  Note that the first negative value seen is our "trigger" value.
	if (InterlockedDecrement(&m_CntCS) < 0)
		DeleteCriticalSection(&m_sCS);
}
#else
// Do nothing functions if multi-threading is not an issue (console app)
PFIO_INLINE void PFIni::CS_Lock() {}
PFIO_INLINE void PFIni::CS_Release() {}
PFIO_INLINE void PFIni::CS_Init() {}
PFIO_INLINE void PFIni::CS_Free() {}
#endif

