
struct tcp_ver2_xfer;

struct ClientData
{
	char	ClientName[40];
	uint32	nWip;
	uint32	nDone;
	time_t	tLastContact;
	double	dAveRate;
	double	Score;  // A "single" day PII-400 is time taken for 3*2^1200000+1
};

struct AccumData
{
	const char *m_Primes[75001];
	uint32 m_nPrimes;
	const char *m_Comps[75001];
	uint32 m_nComps;
	const char *m_Skips[75001];
	uint32 m_nSkips;
	double m_Score;
	class  CClientData *pCClientDataObject;
	char   ClientName[40];
	char   ClientDir[48];
	char   ClientFName[256];
	uint32 CurClient;
};

class CClientData
{
	public:
		CClientData();
		~CClientData();

        void AddNewClient(const char *Name=0);
        uint32 FindMachine(const char *Name);

        uint32 nClients() { return m_nClients; }
        uint32 nCurClient() { return m_nCurClient; }
        uint32 nMaxNumClients() { return m_nMaxNumClients; }

        const ClientData &operator[] (uint32 Index) { return m_Clients[Index]; }
        ClientData &operator[] (uint16 Index) { return m_Clients[Index]; }
        bool bIsThisTheMachinge(const char *Name, uint32 Idx) { return !_stricmp(Name, m_Clients[Idx].ClientName); }


        // Client "statistics" functions
        HANDLE ClientStats_Begin(const char *Name, char *pDataPacket);
        bool   ClientStats_End(HANDLE, FILE *pr, FILE *comp, FILE *skip);
		void   ClientStats_Cleanup();
        bool   ClientStats_AddWorkDone(HANDLE, const char *Work);
        bool   ClientStats_AddWorkSkipped(HANDLE, const char *Work);

        bool   ClientStats_GetNewWork(HANDLE, uint32 &nNumWanted, SOCKET s, FILE *in, bool &bCloseWhenDone, bool bCompress, bool bIsFirstLineFile, char *FirstLine, const char *FName);

        bool   ClientStats_LogIn(HANDLE, const char *Name);
        bool   ClientStats_LogOut(HANDLE, const char *Name);
        bool   ClientStats_Operation(HANDLE, const char *cp);

		const char*ClientStats_CurInFileName();

		void AddNewScore(const char *pClientName, uint32 ClientIdx, double Score);
		void ComputeScores();

    private:
	    volatile uint32 m_nClients;
	    volatile uint32 m_nCurClient;
	    volatile uint32 m_nMaxNumClients;
        volatile HANDLE m_hCurrentHandle;
        volatile uint16 m_iSeq;

		uint32 m_nExistingWork;
		char *m_cpExistingWorkBuf;
		char *m_cpExistingWork[75001];
		void LoadExistingWork();
		bool m_bLoggingOut;

		// This will by the "accumulation" data.
		AccumData *m_pAccum;

	    ClientData *m_Clients; //[MaxNumClients];
        CRITICAL_SECTION m_ClientCritical;
};
