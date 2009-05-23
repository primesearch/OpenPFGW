// WinPFGW.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WinPFGW.h"
#include "WinPFGWDlg.h"

#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinPFGWApp

BEGIN_MESSAGE_MAP(CWinPFGWApp, CWinApp)
	//{{AFX_MSG_MAP(CWinPFGWApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinPFGWApp construction

CWinPFGWApp::CWinPFGWApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWinPFGWApp object

CWinPFGWApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWinPFGWApp initialization

// We need a global mutex handle (could be a member of WinApp)
HANDLE	g_hMutexInst = NULL;
LONG	g_MutexNum = 0;
BOOL CALLBACK MyEnumProc (HWND hwnd, LPARAM lParam)
{
	if (GetWindowLong (hwnd, GWL_USERDATA) != g_MutexNum)
		return (TRUE);
	* (HWND *) lParam = hwnd;
	return (FALSE);
}

BOOL CWinPFGWApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

/* Change the working directory to the same directory that */
/* the executable is located.  Important if running as a win95 "service" */
#if !defined (_DEBUG)
	char	Dirbuf[256];
	GetModuleFileName (NULL, Dirbuf, sizeof (Dirbuf));
	strrchr (Dirbuf, '\\')[1] = 0;
	_chdir (Dirbuf);
#endif

	CWinPFGWDlg dlg;
	m_pMainWnd = &dlg;

	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

bool AreWeFirstAppRunning(HWND hWnd)
{
// Make sure only one copy is running at a time in a "given" directory
	HWND	hwndPrevInst;
	char	*p;

// Turn directory name into a (likely) unique integer
	char Dirbuf[256];
	_getcwd (Dirbuf, 255);
	for (p = Dirbuf; *p; p++)
		g_MutexNum = g_MutexNum * 19 + *p;

// Create our mutex
	sprintf (Dirbuf, "WinPFGW %ld", g_MutexNum);
	g_hMutexInst = CreateMutex (NULL, FALSE, Dirbuf);

// Test for failure
	if (g_hMutexInst == NULL)
		return 0;	// Hmm, problems!  Probably OUT of resources

// If mutex existed before another inst is running

	if (GetLastError () == ERROR_ALREADY_EXISTS)
	{
// Allow other instance to display it's main window
		Sleep (750);

// Find the window handle
		EnumWindows (&MyEnumProc, (LPARAM) &hwndPrevInst);

// Unhide the other instance's window
		ShowWindow (hwndPrevInst, SW_HIDE);
		ShowWindow (hwndPrevInst, SW_SHOWMINIMIZED);
		ShowWindow (hwndPrevInst, SW_SHOWNORMAL);

		//::ShowWindow(hWnd, SW_SHOWDEFAULT);
		//::BringWindowToTop(hWnd);
		//::SetForegroundWindow(hWnd);

		return false;
	}

// Set the window user data so we can be identified by
// another instance of this program.
	SetWindowLong (hWnd, GWL_USERDATA, g_MutexNum);

	return true;
}

int CWinPFGWApp::ExitInstance() 
{
	CloseHandle(g_hMutexInst);
	return CWinApp::ExitInstance();
}
