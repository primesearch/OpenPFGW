unsigned long clocks_per_sec;			// Machine dependent
int g_nIterationCnt=2500;
int g_CompositeAthenticationLevel = -1;
int g_ExtraSQFree = 100;
int g_Cert_Type = -1;
int g_Cert_Delete = -1;
bool g_bVerbose = false;
bool volatile g_bExitNow;
bool volatile g_bExited;

class PFIni *g_pIni;
uint64 g_u64ResidueVal;
char g_ModularSieveString[100];
bool g_bTestingMode;
