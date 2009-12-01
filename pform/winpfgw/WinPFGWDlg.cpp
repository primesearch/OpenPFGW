// WinPFGWDlg.cpp : implementation file
//

#include "stdafx.h"
#include <float.h>
#include <process.h>

#include "WinPFGW.h"
#include "WinPFGWDlg.h"
#include "MessageDialog.h"

#include "config.h"
#include "pflib.h"
#include "pfmath.h"
#include "pfgwlib.h"
#include "pfglue.h"
#include "pfoo.h"
#define __WinPFGW_MAIN__
#include "pfio.h"
#include "pfini.h"
#include "winbloz_msg.h"
#include "TextViewerDlg.h"

#include "AboutDlg.cxx"
#include "RangeCompleteDlg.h"

extern unsigned long clocks_per_sec;
extern bool volatile g_bExitNow;
extern bool volatile g_bExited;
extern bool g_bWinPFGW_Verbose;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define M_SAVEOPTS 9999

/////////////////////////////////////////////////////////////////////////////
// CWinPFGWDlg dialog

CWinPFGWDlg::CWinPFGWDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWinPFGWDlg::IDD, pParent),
	m_dwPriorityClass(IDLE_PRIORITY_CLASS),
	m_nThreadPriority(THREAD_PRIORITY_IDLE),
	m_trayIcon(IDR_TRAY_MENU)
{
	//{{AFX_DATA_INIT(CWinPFGWDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_GREEN);

	m_Yellow = AfxGetApp()->LoadIcon(IDI_ICON_YELLOW);
	m_Red = AfxGetApp()->LoadIcon(IDI_ICON_RED);
	m_Green = AfxGetApp()->LoadIcon(IDI_ICON_GREEN);

	// Set the "resizing" stuff.
	m_nDlgMinX = m_nDlgMinY = m_nDlgClientMinY = m_nDlgClientMinX = 0;
	m_nEditX = m_nEditY = 0;

	m_bTrayIcon = false;
	m_bStealth = false;
	m_bDONTSAVEOPTIONS = false;

	m_ThreadHandle = 0;

	m_nPRPBase = 3;
	m_nMode = m_nPRPBase;

	m_bIconIsRed = false;
	m_bIconFlashing = false;
	m_bExitedByRequest = false;
}

void CWinPFGWDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWinPFGWDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWinPFGWDlg, CDialog)
	//{{AFX_MSG_MAP(CWinPFGWDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_START, OnBtnStart)
    ON_MESSAGE(WM_MY_TRAY_NOTIFICATION, OnTrayNotification)
	ON_COMMAND(IDM_ABOUTBOX, OnAboutbox)
	ON_MESSAGE(WinPFGW_MSG, OnWinPFGW_Message)
	ON_WM_DESTROY()
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_COMMAND(IDC_ABOUT_RECORDS, OnAboutRecords)
	ON_COMMAND(IDC_ABOUT_CONTENTS, OnAboutContents)
	ON_COMMAND(IDC_USE_TRAY_ICON, OnUseTrayIcon)
	ON_UPDATE_COMMAND_UI(IDC_USE_TRAY_ICON, OnUpdateUseTrayIcon)
	ON_COMMAND(IDC_IDLE, OnIdle)
	ON_UPDATE_COMMAND_UI(IDC_IDLE, OnUpdateIdle)
	ON_COMMAND(IDC_LOW, OnLow)
	ON_UPDATE_COMMAND_UI(IDC_LOW, OnUpdateLow)
	ON_COMMAND(IDC_NORMAL, OnNormal)
	ON_UPDATE_COMMAND_UI(IDC_NORMAL, OnUpdateNormal)
	ON_COMMAND(IDC_BELOW_NORMAL, OnBelowNormal)
	ON_UPDATE_COMMAND_UI(IDC_BELOW_NORMAL, OnUpdateBelowNormal)
	ON_COMMAND(IDC_VERBOSE_SCREEN, OnVerboseScreen)
	ON_UPDATE_COMMAND_UI(IDC_VERBOSE_SCREEN, OnUpdateVerboseScreen)
	ON_COMMAND(IDC_VERBOSE_FILE, OnVerboseFile)
	ON_UPDATE_COMMAND_UI(IDC_VERBOSE_FILE, OnUpdateVerboseFile)
	ON_COMMAND(IDC_PRP_PROOFCHOICE, OnPrpProofchoice)
	ON_UPDATE_COMMAND_UI(IDC_PRP_PROOFCHOICE, OnUpdatePrpProofchoice)
	ON_COMMAND(IDC_Nm1_PROOFCHOICE, OnNm1PROOFCHOICE)
	ON_UPDATE_COMMAND_UI(IDC_Nm1_PROOFCHOICE, OnUpdateNm1PROOFCHOICE)
	ON_COMMAND(IDC_Np1_PROOFCHOICE, OnNp1PROOFCHOICE)
	ON_UPDATE_COMMAND_UI(IDC_Np1_PROOFCHOICE, OnUpdateNp1PROOFCHOICE)
	ON_COMMAND(IDC_FACTORIZE_ONLY, OnFactorizeOnly)
	ON_UPDATE_COMMAND_UI(IDC_FACTORIZE_ONLY, OnUpdateFactorizeOnly)
	ON_COMMAND(IDC_DECIMAL_EXPANSION, OnDecimalExpansion)
	ON_UPDATE_COMMAND_UI(IDC_DECIMAL_EXPANSION, OnUpdateDecimalExpansion)
	ON_COMMAND(IDC_SET_PRP_BASE, OnSetPrpBase)
	ON_UPDATE_COMMAND_UI(IDC_SET_PRP_BASE, OnUpdateSetPrpBase)
	ON_COMMAND(IDC_Pp1Np1_PROOFCHOICE, OnPp1Np1PROOFCHOICE)
	ON_UPDATE_COMMAND_UI(IDC_Pp1Np1_PROOFCHOICE, OnUpdatePp1Np1PROOFCHOICE)
	ON_COMMAND(IDC_FACTORIZE, OnFactorize)
	ON_UPDATE_COMMAND_UI(IDC_FACTORIZE, OnUpdateFactorize)
	ON_COMMAND(IDC_FERMAT_FACTOR_ONLY, OnFermatFactorOnly)
	ON_UPDATE_COMMAND_UI(IDC_FERMAT_FACTOR_ONLY, OnUpdateFermatFactorOnly)
	ON_BN_CLICKED(IDC_FERMAT_FACTOR, OnFermatFactor)
	ON_UPDATE_COMMAND_UI(IDC_FERMAT_FACTOR, OnUpdateFermatFactor)
	ON_COMMAND(IDC_CLEAR_HIGHLIGHT, OnClearHighlight)
	ON_COMMAND(IDC_CLEAR_ALL, OnClearAll)
	ON_COMMAND(IDC_HIDE_THE_APP, OnHideTheApp)
	ON_UPDATE_COMMAND_UI(IDC_HIDE_THE_APP, OnUpdateHideTheApp)
	ON_UPDATE_COMMAND_UI(IDC_NORMAL_SCREEN, OnUpdateNormalScreen)
	ON_UPDATE_COMMAND_UI(IDC_QUIET_SCREEN, OnUpdateQuietScreen)
	ON_COMMAND(IDC_NORMAL_SCREEN, OnNormalScreen)
	ON_COMMAND(IDC_QUIET_SCREEN, OnQuietScreen)
	ON_COMMAND(IDC_GF_FACT_ONLY_SCREEN, OnGfFactOnlyScreen)
	ON_UPDATE_COMMAND_UI(IDC_GF_FACT_ONLY_SCREEN, OnUpdateGfFactOnlyScreen)
	ON_WM_TIMER()
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP

	// The damn dialog box does NOT handle menu popup's and ON_UPDATE_COMMAND_UI by default.
	// See MSDN article ID: Q242577 for the code to fix this "by design" BUG
	ON_WM_INITMENUPOPUP()

	// To catch initial paint, and to NOT paint the window (ie hide it) if we are hidden mode.
	ON_WM_WINDOWPOSCHANGING()

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinPFGWDlg message handlers

bool AreWeFirstAppRunning(HWND hWnd);

BOOL CWinPFGWDlg::OnInitDialog()
{
	// Setup the global ini object to the PFGW.INI file.  NOTE that any program written which will be
	// calling pfgw_main() will need to open the "correct" ini file.  PFGW.EXE opens PFGW.ini.  WinPFGW.exe
	// may open something different and foobars_speed_siever.exe may open up something altogether different.

	CDialog::OnInitDialog();

	// Create a non-console Win32GUI output object.
	pOutputObj = new PFWin32GUIOutput((int)m_hWnd);

	g_pIni = new PFIni("PFGW.INI");
	m_ScreenMode = eNormal;
	LoadOptions(g_pIni);
	m_bHideOnStart=false;
	if (m_bStealth)
		m_bHideOnStart=true;

	// Make sure only one copy is running at a time in a "given" directory
	// This check MUST be done after the window is created. The joys of a Dialog app :(
	if (!AreWeFirstAppRunning(m_hWnd))
	{
		m_bDONTSAVEOPTIONS = true;
		EndDialog(IDCANCEL);
		return FALSE;
	}

	// Set up tray icon   
	m_trayIcon.SetNotificationWnd(this, WM_MY_TRAY_NOTIFICATION);

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// toggle the tray icon bool to the "wrong" value, and then simply simply simulate a "click" to the other (the right) state.
	if (!m_bStealth)
	{
		ChangeIcon(IconYellow);
		m_bTrayIcon = !m_bTrayIcon;
		OnUseTrayIcon();
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here


	CEdit *pCe = (CEdit*)GetDlgItem(IDC_EDIT_OUTPUT);
	m_SmartEdit.AttachCEdit(pCe);

	// Calculate dimensions needed for my simple resizing logic to work.  I simply don't allow any
	// resizing in the X direction, and only allow sizes "larger" than starting value for changes in
	// the Y direction.  The edit box also resizes based upon resize of the dialog.
	CRect r;
	GetWindowRect(&r);		// Needed for the WM_GETMINMAXINFO to "limit" the sizing allowed on the dialog.
	m_nDlgMinX = r.right;
	m_nDlgMinY = r.bottom;

	GetClientRect(&r);		// Needed in the OnSize to resize the edit box.  Note that onSize gives us "client" dimensions.
	m_nDlgClientMinY = r.bottom-r.top;
	m_nDlgClientMinX = r.right-r.left;

	pCe->GetWindowRect(&r);
	m_nEditX = r.right-r.left;
	m_nEditY = r.bottom-r.top;

	clocks_per_sec=CLOCKS_PER_SEC;

	// Make sure that VC uses 64 bit FPU instructions for high level FPU code
#if defined (_MSC_VER)
	_control87(_PC_64, _MCW_PC);	// 64 bits precision (instead of 53 bit default precision)
	_control87(_RC_NEAR, _MCW_RC);	// make SURE that we round numbers to the nearest, and not floor or ceil
#endif

	if (!m_bStealth)
		m_trayIcon.SetToolTipString("WinPFGW");

	if (m_bRunning)
	{
		PostMessage(WM_COMMAND, IDC_BTN_START, 0);
	}
	// Only allow a "minimized state if running.  NOW minimize no matter what if we are "supposed to"
	if (m_bMinimized)
	{
		PostMessage(WM_SYSCOMMAND, 0xF020, 0);	// post a minimize command.
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CWinPFGWDlg::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CDialog::PreCreateWindow(cs);
}

void CWinPFGWDlg::ChangeIcon(IconColor color, const char *Msg)
{
	switch(color)
	{
		case IconRed:
			m_bIconFlashing = true;
			m_bIconIsRed = true;
			SetIcon(m_Red, false);
			m_trayIcon.SetIcon(IDI_ICON_RED);
			break;
		case IconGreen:
			m_bIconFlashing = false;
			m_bIconIsRed = false;
			KillTimer(666);
			SetIcon(m_Green, false);
			m_trayIcon.SetIcon(IDI_ICON_GREEN);
			break;
		case IconYellow:
			m_bIconFlashing = true;
			m_bIconIsRed = false;
			SetIcon(m_Yellow, false);
			m_trayIcon.SetIcon(IDI_ICON_YELLOW);
			break;
	}
	if (Msg)
		m_trayIcon.SetToolTipString(Msg);
}


void CWinPFGWDlg::OnDestroy() 
{
	// We need to try to "exit" the worker thread here (at least to flush the .ini) and
	// save any "temp" work.  However, we should NOT wait for network transmissions (if
	// working on a NETWORK2 file, so the PFNetwork2File code should just "flush" and 
	// exit.

	// the variables g_bExitNow and m_bExitedByRequest need to be addressed, and possible
	// a g_bExitNow_Forced should also be "added"

	// We have now added a 5.5 second wait to send data to the server

	CString txt;
	CWnd *btnStartStop = GetDlgItem(IDC_BTN_START);
	btnStartStop->GetWindowText(txt);
	btnStartStop->EnableWindow(FALSE);
	if (txt == "&Stop")
	{
		g_bExitNow = true;
		m_bExitedByRequest = true;
		Sleep(500);
		for (int i = 0; !g_bExited && i < 50; i++)
		{
			MSG m;
			// If there is a print message in queue, kill it, to allow the PFGW thread to be able to exit.
			while (PeekMessage(&m, 0, 0, 0xFFFFFFFF, PM_NOREMOVE))
			{
				GetMessage(&m, 0, 0, 0xFFFFFFFF);
				TranslateMessage(&m);
				DispatchMessage(&m);
			}
			Sleep(100);
		}
		Sleep(100);
	}
	delete pOutputObj;
	SaveOptions(g_pIni);
	delete g_pIni;
	m_trayIcon.SetIcon(0);
	CDialog::OnDestroy();
}

// The damn dialog box does NOT handle menu popup's and ON_UPDATE_COMMAND_UI by default.
// See MSDN article ID: Q242577 for the code to fix this "by design" BUG
void CWinPFGWDlg::OnInitMenuPopup(CMenu *pPopupMenu, UINT /*nIndex*/,BOOL /*bSysMenu*/)
{
    ASSERT(pPopupMenu != NULL);
    // Check the enabled state of various menu items.

    CCmdUI state;
    state.m_pMenu = pPopupMenu;
    ASSERT(state.m_pOther == NULL);
    ASSERT(state.m_pParentMenu == NULL);

    // Determine if menu is popup in top-level menu and set m_pOther to
    // it if so (m_pParentMenu == NULL indicates that it is secondary popup).
    HMENU hParentMenu;
    if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
        state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
    else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
    {
        CWnd* pParent = this;
		// Child windows don't have menus--need to go to the top!
        if (pParent != NULL && (hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
        {
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
        }
    }

    state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
    for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++)
    {
        state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
        if (state.m_nID == 0)
			continue; // Menu separator or invalid cmd - ignore it.
		
        ASSERT(state.m_pOther == NULL);
        ASSERT(state.m_pMenu != NULL);
        if (state.m_nID == (UINT)-1)
        {
			// Possibly a popup menu, route to first item of that popup.
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL || (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 || state.m_nID == (UINT)-1)
			{
				continue;       // First item of popup can't be routed to.
			}
			state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
        }
        else
        {
			// Normal menu item.
			// Auto enable/disable if frame window has m_bAutoMenuEnable
			// set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
        }

        // Adjust for menu deletions and additions.
        UINT nCount = pPopupMenu->GetMenuItemCount();
        if (nCount < state.m_nIndexMax)
        {
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount && pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
        }
        state.m_nIndexMax = nCount;
    }
} 

afx_msg void CWinPFGWDlg::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
	if (m_nDlgMinX)
	{
		// Set the min size
		lpMMI->ptMinTrackSize.x = m_nDlgMinX;
		lpMMI->ptMinTrackSize.y = m_nDlgMinY;
		// Do not allow the width of the dialog to expand.  ONLY the height is allowed to expand.
		//lpMMI->ptMaxTrackSize.x = m_nDlgMinX;
	}
}


void CWinPFGWDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	// Adjust the size of the edit box within the window.
	if (m_nDlgClientMinY)
	{
		CEdit *pCe = (CEdit*)GetDlgItem(IDC_EDIT_OUTPUT);
		pCe->SetWindowPos(NULL, 0, 0, m_nEditX+(cx-m_nDlgClientMinX), m_nEditY+(cy-m_nDlgClientMinY), SWP_NOZORDER|SWP_NOMOVE);
	}
}

void CWinPFGWDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if ((nID & 0xFFF0) == SC_MINIMIZE && m_bStealth)
	{
		// This actually hides the program
		ShowWindow(SW_HIDE);
		m_bMinimized = true;
		if (m_bTrayIcon)
			m_trayIcon.SetIcon(0);
		m_bTrayIcon = false;
	}
	else if ((nID & 0xFFF0) == SC_MINIMIZE && m_bTrayIcon)
	{
		// This actually sends the program to the tray
		ShowWindow(SW_HIDE);
		m_bMinimized = true;
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
	SaveOptions(g_pIni);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWinPFGWDlg::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == 666 && !m_bStealth)
	{
		// We are in "Flash" icon mode!!!
		if (m_bIconIsRed)
			ChangeIcon(IconYellow);
		else
			ChangeIcon(IconRed);
	}
	CDialog::OnTimer(nIDEvent);
}

void CWinPFGWDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWinPFGWDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

afx_msg void CWinPFGWDlg::OnAboutbox() 
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

afx_msg LRESULT CWinPFGWDlg::OnTrayNotification(WPARAM /*uID*/, LPARAM lEvent)
{
    if (WM_MOUSEFIRST <=lEvent && lEvent<=WM_MOUSELAST)
    {
        if (lEvent-WM_MOUSEFIRST == 3)
        {
			bool bWasFlashing = m_bIconFlashing;

            ShowWindow(SW_SHOWNORMAL);
            ::SetForegroundWindow(m_hWnd);
            SetFocus();
			m_bMinimized = false;
			SaveOptions(g_pIni);
			// Clear the flash icon IF it is set.
			if (m_bIconFlashing || bWasFlashing)
			{
				// Kill the flashing timer
				KillTimer(666);

				// Restart the flashing since the OnActivateWindow has "killed" it already.  We are simply
				// "rearming" the OnActiveWindow to to it's work again (now that our window is showing 
				if (!m_bStealth)
					ChangeIcon(IconRed, "WinPFGW (NOT running!!!!)");
				// Turn on the FLASH icon
				SetTimer(666, 1000, NULL);
				m_bIconFlashing = true;
				m_bIconIsRed = true;

				CRangeCompleteDlg Dlg;
				Dlg.DoModal();
			}
        }
    }
    return 0;
}

afx_msg void CWinPFGWDlg::OnActivate( UINT nState, CWnd* /*pWndOther*/, BOOL /*bMinimized*/ )
{
	/*
	char Buf[512];
	sprintf (Buf, "****  Win Activated.  State is : ");
	if (nState == WA_INACTIVE)
		strcat(Buf, "WA_INACTIVE");
	if (nState == WA_ACTIVE)
		strcat(Buf, "WA_ACTIVE");
	if (nState == WA_CLICKACTIVE)
		strcat(Buf, "WA_CLICKACTIVE");

	if (bMinimized)
		strcat(Buf, " Minimized\n");
	else
		strcat(Buf, " not-Minimized\n");
	TRACE(Buf);
	*/
	if (nState != WA_INACTIVE)
	{
		// Clear the flash icon IF it is set.
		if (m_bIconFlashing)
		{
			// Kill the flashing timer
			KillTimer(666);
			if (!m_bStealth)
				ChangeIcon(IconYellow);
			// Remove the flashing state
			m_bIconFlashing = false;
			m_bIconIsRed = false;
		}
	}
}

extern "C" int pfgw_main(int, char **);

void CWinPFGWDlg::OnBtnStart() 
{
	CString txt;
	CWnd *btnStartStop = GetDlgItem(IDC_BTN_START);
	btnStartStop->GetWindowText(txt);
	btnStartStop->EnableWindow(FALSE);
	if (txt == "&Stop")
	{
		g_bExitNow = true;
		m_bExitedByRequest = true;
		Sleep(500);
		for (int i = 0; !g_bExited && i < 150; i++)
		{
			MSG m;
			// If there is a print message in queue, kill it, to allow the PFGW thread to be able to exit.
			while (PeekMessage(&m, 0, 0, 0xFFFFFFFF, PM_NOREMOVE))
			{
				GetMessage(&m, 0, 0, 0xFFFFFFFF);
				TranslateMessage(&m);
				DispatchMessage(&m);
			}
			Sleep(200);
		}
		Sleep(200);
		m_ThreadHandle = 0;
		m_bRunning = false;
		btnStartStop->SetWindowText("&Start");
	}
	else
	{
		m_bRunning = true;
		m_bExitedByRequest = false;
        m_ThreadHandle = (HANDLE)_beginthread(ThreadProc, 0, (void*)((CWinPFGWDlg*)this));
		btnStartStop->SetWindowText("&Stop");
		Sleep(100);
		switch(m_nPriority)
		{
			case 0:  ::SetThreadPriority(m_ThreadHandle, THREAD_PRIORITY_IDLE); break;
			case 1:  ::SetThreadPriority(m_ThreadHandle, THREAD_PRIORITY_LOWEST); break;
			case 2:  ::SetThreadPriority(m_ThreadHandle, THREAD_PRIORITY_BELOW_NORMAL); break;
			case 3:  ::SetThreadPriority(m_ThreadHandle, THREAD_PRIORITY_NORMAL); break;
		}

	}

	SaveOptions(g_pIni);
	btnStartStop->EnableWindow();
}

// A message coming from PFGW (currently only printf's and fprintf(stderr) messages
afx_msg LRESULT CWinPFGWDlg::OnWinPFGW_Message(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
		case M_SAVEOPTS:
			SaveOptions(g_pIni);
			break;
		case M_STDERR:
		case M_PRINTF:
		{
			char *cp = (char*)lParam;
			if (cp)
			{
				static DWORD NextUpdate = 0;
				bool bShowStr=true;
				if (m_ScreenMode != eVerbose && !(m_nMode <=1 && m_nMode >= -1) )
				{
					// I know this code should be in smarteditfield class, but it is here for now.
					char *cpp = &cp[strlen(cp)-1];
					if (*cpp == '\n')
					{
						extern bool GF_b_DoGFFactors;
						if (m_ScreenMode == eQuiet)
						{
							*cpp = '\r';
							bShowStr = false;
						}
						else if (strstr(cp, "composite") || strstr(cp, "factor"))
						{
							*cpp = '\r';
							bShowStr = false;
						}
						else if ( (m_ScreenMode==eGFFactors && GF_b_DoGFFactors) && strstr(cp, "-PRP!"))
						{
							*cpp = '\r';
							bShowStr = false;
						}
					}
				}
            if (bShowStr)
					m_SmartEdit.AddString(cp);
				else if (NextUpdate < GetTickCount())
				{
					NextUpdate = GetTickCount() + 2000;
					char Buf[40];
					sprintf(Buf, "%d", g_pIni->GetFileLineNum());
					GetDlgItem(IDC_LINE_IN_FILE)->SetWindowText(Buf);
					m_SmartEdit.AddString(cp);
				}
				else
					/* If in "SuperQuiet" mode, we MUSt clean up this string, or we will leak to death! */
					delete[] cp;
			}
			break;
		}
		case M_THREAD_EXITING:
		{
			char *cp = new char[20];
			strcpy(cp, "\nDone.\n");
			m_SmartEdit.AddString(cp);
			m_ThreadHandle = 0;
			m_bRunning = false;
			SaveOptions(g_pIni);
			GetDlgItem(IDC_BTN_START)->SetWindowText("&Start");

			// If we are NOT in stealth mode, then start the ICON flashing!!!

			if (!m_bStealth)
			{
				if (!m_bExitedByRequest)
				{
					ChangeIcon(IconRed, "WinPFGW (NOT running!!!!)");
					// Turn on the FLASH icon
					SetTimer(666, 1000, NULL);
					m_bIconFlashing = true;
					m_bIconIsRed = true;
					if (!m_bMinimized)
					{
						CRangeCompleteDlg Dlg;
						Dlg.DoModal();
					}
				}
				else
				{
					ChangeIcon(IconYellow, "WinPFGW (NOT running!!!!)");
					m_bIconFlashing = false;
					m_bIconIsRed = false;
				}
			}
			m_bExitedByRequest = false;

			break;
		}
	}
	return 0;
}


void CWinPFGWDlg::ThreadProc(void *m_ThisPointer)
{
	CWinPFGWDlg *This = (CWinPFGWDlg*)m_ThisPointer;
	char *argv[40];
	int argc = 0;

	// Add argv
	argv[argc] = new char[20];
	strcpy(argv[argc++], "./WinPFGW.exe");

	if (This->m_nMode == 1)
	{
		argv[argc] = new char[20];
		strcpy(argv[argc++], "-tp");
	}
	else if (This->m_nMode == -1)
	{
		argv[argc] = new char[20];
		strcpy(argv[argc++], "-tm");
	}
	else if (This->m_nMode == 0)
	{
		argv[argc] = new char[20];
		strcpy(argv[argc++], "-tc");
	}
	else if (This->m_nMode == -2)
	{
		argv[argc] = new char[20];
		strcpy(argv[argc++], "-f0");
	}

	if (This->m_bFactor)
	{
		argv[argc] = new char [20];
		strcpy(argv[argc++], "-f");
	}

	if (This->m_bVerboseFile)
	{
		argv[argc] = new char [20];
		strcpy(argv[argc++], "-l");
	}

	// Ok add the "real" data element
	CString csEdit;
	CWnd *wEdit = This->GetDlgItem(IDC_EDIT_INPUTFILE);
	wEdit->GetWindowText(csEdit);

	char *cp = new char [csEdit.GetLength()+1];
	strcpy(cp, csEdit);
	// Now split it up
	char *cp1 = cp;
	while (cp1 && *cp1)
	{
		if (*cp1 == '"')	// quoted stuff
		{
			char *cp2 = strchr(&cp1[1], '"');
			if (!cp2)
			{
				delete[] cp;
				This->MessageBox("Error, non-matching quotes");
				return;
			}
			*cp2++ = 0;
			cp1++;
			argv[argc] = new char [strlen(cp1)+1];
			strcpy(argv[argc++], cp1);
			cp1 = cp2;
		}
		else
		{
			char *cp2 = strchr(&cp1[1], ' ');
			if (cp2)
				*cp2++ = 0;
			argv[argc] = new char [strlen(cp1)+1];
			strcpy(argv[argc++], cp1);
			cp1 = cp2;
		}
	}

	delete[] cp;
	// Run the damn thing.

	// Ok, now make the "magic" command line param.
	argv[argc] = new char [120];
	sprintf(argv[argc++], "-$HWND=%X -Verbose=%d", This->m_hWnd, This->m_ScreenMode == eVerbose);

	This->PostMessage(WinPFGW_MSG, M_SAVEOPTS, 0);

	if (!This->m_bStealth)
		This->ChangeIcon(IconGreen, "WinPFGW (Running)");
	pfgw_main(argc, argv);
	Sleep(250);
	while(argc)
		delete[] argv[--argc];

	This->PostMessage(WinPFGW_MSG, M_STDERR, 0);
	Sleep(100);
	This->PostMessage(WinPFGW_MSG, M_THREAD_EXITING, 0);
	Sleep(100);
}


void CWinPFGWDlg::OnAboutRecords() 
{
	MessageBox("Hmm, no records yet ;)");
}

void CWinPFGWDlg::OnAboutContents() 
{
	CTextViewerDlg Dlg;
	Dlg.DoModal();
}

void CWinPFGWDlg::OnUseTrayIcon() 
{
	m_bTrayIcon = !m_bTrayIcon;
	if (m_bTrayIcon && !m_bStealth)
	{
		if (m_bRunning)
			ChangeIcon(IconGreen, "WinPFGW (Running)");
		else
			ChangeIcon(IconYellow, "WinPFGW (NOT Running!!!)");
	}
	else
	{
		m_trayIcon.SetIcon(0);
	}
	SaveOptions(g_pIni);
}

void CWinPFGWDlg::OnUpdateUseTrayIcon(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bTrayIcon);
}

void CWinPFGWDlg::OnHideTheApp() 
{
	m_bStealth = !m_bStealth;
	if (!m_bStealth)
	{
		if (m_bTrayIcon)
		{
			if (m_bRunning)
				ChangeIcon(IconGreen, "WinPFGW (Running)");
			else
				ChangeIcon(IconYellow, "WinPFGW (NOT Running!!!)");
		}
		else
		{
			m_trayIcon.SetIcon(0);
		}
	}
	else
	{
		m_trayIcon.SetIcon(0);
	}
	SaveOptions(g_pIni);
}

void CWinPFGWDlg::OnUpdateHideTheApp(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bStealth);
}


void CWinPFGWDlg::LoadOptions(PFIni *pIni)
{
	PFString OldSection;
	pIni->GetCurrentSection(OldSection);
	pIni->SetCurrentSection("WinPFGW");

	PFString s;
	s = "TrayIcon";
	pIni->GetIniBool(&m_bTrayIcon, &s, true, true);
	s = "Stealth";
	pIni->GetIniBool(&m_bStealth, &s, false, true);
	s = "Priority";
	pIni->GetIniInt(&m_nPriority, &s, 1, true);
	s = "ScreenMode";
	int n;
	pIni->GetIniInt(&n, &s, eNormal,true);
	g_bWinPFGW_Verbose=false;
	switch(n)
	{
		case 0: m_ScreenMode = eQuiet; break;
		case 1: m_ScreenMode = eGFFactors; break;
		case 3: m_ScreenMode = eVerbose; g_bWinPFGW_Verbose=true; break;
		case 2: default: m_ScreenMode = eNormal; break;
	}
	s = "VerboseFile";
	pIni->GetIniBool(&m_bVerboseFile, &s);
	s = "DontShowFileVerboseMessage";
	pIni->GetIniBool(&m_bDontShowFileVerboseMessage, &s);
	s = "Running";
	pIni->GetIniBool(&m_bRunning, &s);
	s = "Minimized";
	pIni->GetIniBool(&m_bMinimized, &s);
	s = "PRPBase";
	pIni->GetIniInt(&m_nPRPBase, &s, 3, true);
	s = "WorkMode";
	pIni->GetIniInt(&m_nMode, &s, m_nPRPBase, true);
	s = "Factorize";
	pIni->GetIniBool(&m_bFactor, &s, false);
	s = "FermatFactor";
	pIni->GetIniBool(&m_bFFactor, &s, false);

	s = "CommandLine";
	PFString s1;
	pIni->GetIniString(&s1, &s);
	CWnd *w = GetDlgItem(IDC_EDIT_INPUTFILE);
	w->SetWindowText((LPCSTR)s1);

	pIni->SetCurrentSection(OldSection);
}

void CWinPFGWDlg::SaveOptions(PFIni *pIni)
{
	if  (m_bDONTSAVEOPTIONS)
		return;
	PFString OldSection;
	pIni->GetCurrentSection(OldSection);
	pIni->SetCurrentSection("WinPFGW");

	PFString s;
	s = "TrayIcon";
	pIni->SetIniBool(m_bTrayIcon, &s);
	s = "Stealth";
	pIni->SetIniBool(m_bStealth, &s);
	s = "Priority";
	pIni->SetIniInt(m_nPriority, &s);
	s = "ScreenMode";
	pIni->SetIniInt(m_ScreenMode, &s);
	s = "VerboseFile";
	pIni->SetIniBool(m_bVerboseFile, &s);
	s = "DontShowFileVerboseMessage";
	pIni->SetIniBool(m_bDontShowFileVerboseMessage, &s);
	s = "Running";
	pIni->SetIniBool(m_bRunning, &s);
	s = "Minimized";
	pIni->SetIniBool(m_bMinimized, &s);
	s = "PRPBase";
	pIni->SetIniInt(m_nPRPBase, &s);
	s = "WorkMode";
	pIni->SetIniInt(m_nMode, &s);
	s = "Factorize";
	pIni->SetIniBool(m_bFactor, &s);
	s = "FermatFactor";
	pIni->SetIniBool(m_bFFactor, &s);

	s = "CommandLine";
	CString cs;
	CWnd *w = GetDlgItem(IDC_EDIT_INPUTFILE);
	w->GetWindowText(cs);
	PFString s1 = (const char*)cs;
	pIni->SetIniString(&s1, &s);


	pIni->SetCurrentSection(OldSection);

	pIni->ForceFlush();
}

void CWinPFGWDlg::OnIdle() 
{
	m_nPriority = 0;
	if (m_bRunning)
		SetThreadPriority(m_ThreadHandle, THREAD_PRIORITY_IDLE);
	SaveOptions(g_pIni);
}
void CWinPFGWDlg::OnLow() 
{
	m_nPriority = 1;
	if (m_bRunning)
		SetThreadPriority(m_ThreadHandle, THREAD_PRIORITY_LOWEST);
	SaveOptions(g_pIni);
}
void CWinPFGWDlg::OnBelowNormal() 
{
	m_nPriority = 2;
	if (m_bRunning)
		SetThreadPriority(m_ThreadHandle, THREAD_PRIORITY_BELOW_NORMAL);
	SaveOptions(g_pIni);
}
void CWinPFGWDlg::OnNormal() 
{
	m_nPriority = 3;
	if (m_bRunning)
		SetThreadPriority(m_ThreadHandle, THREAD_PRIORITY_NORMAL);
	SaveOptions(g_pIni);
}

void CWinPFGWDlg::OnQuietScreen() 
{
	m_ScreenMode = eQuiet;
	g_bWinPFGW_Verbose=false;
	SaveOptions(g_pIni);
}

void CWinPFGWDlg::OnNormalScreen() 
{
	m_ScreenMode = eNormal;
	g_bWinPFGW_Verbose=false;
	SaveOptions(g_pIni);
}
void CWinPFGWDlg::OnGfFactOnlyScreen() 
{
	m_ScreenMode = eGFFactors;
	g_bWinPFGW_Verbose=false;
	SaveOptions(g_pIni);
}
void CWinPFGWDlg::OnVerboseScreen() 
{
	m_ScreenMode = eVerbose;
	g_bWinPFGW_Verbose=true;
	SaveOptions(g_pIni);
}
void CWinPFGWDlg::OnVerboseFile() 
{
	m_bVerboseFile = !m_bVerboseFile;
	if (m_bVerboseFile && !m_bDontShowFileVerboseMessage)
	{
		CMessageDialog Dlg;
		Dlg.SetMessageText("Verbose File output will generate a file called PFGW.OUT.  This file will contain all verbose items from a PFGW scan.  All composites, all factors found, etc.");
		Dlg.DoModal();
		m_bDontShowFileVerboseMessage = Dlg.NeverShowThisMessageAgain();
	}
	SaveOptions(g_pIni);
}

void CWinPFGWDlg::OnUpdateIdle(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nPriority==0);
}
void CWinPFGWDlg::OnUpdateBelowNormal(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nPriority==1);
}
void CWinPFGWDlg::OnUpdateLow(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nPriority==2);
}
void CWinPFGWDlg::OnUpdateNormal(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nPriority==3);
}
void CWinPFGWDlg::OnUpdateVerboseScreen(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_ScreenMode==eVerbose);
}
void CWinPFGWDlg::OnUpdateGfFactOnlyScreen(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_ScreenMode==eGFFactors);
}
void CWinPFGWDlg::OnUpdateNormalScreen(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_ScreenMode==eNormal);
}
void CWinPFGWDlg::OnUpdateQuietScreen(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_ScreenMode==eQuiet);
}
void CWinPFGWDlg::OnUpdateVerboseFile(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_bRunning);
	pCmdUI->SetCheck(m_bVerboseFile);
}

void CWinPFGWDlg::OnPrpProofchoice() 
{
	m_nMode = m_nPRPBase;
	SaveOptions(g_pIni);
}
void CWinPFGWDlg::OnNm1PROOFCHOICE() 
{
	m_nMode = -1;
	SaveOptions(g_pIni);
}
void CWinPFGWDlg::OnNp1PROOFCHOICE() 
{
	m_nMode = 1;
	SaveOptions(g_pIni);
}
void CWinPFGWDlg::OnPp1Np1PROOFCHOICE() 
{
	m_nMode = 0;
	SaveOptions(g_pIni);
}
void CWinPFGWDlg::OnFactorizeOnly() 
{
	m_nMode = -2;
	SaveOptions(g_pIni);
}
void CWinPFGWDlg::OnFermatFactorOnly() 
{
	m_nMode = -3;
	SaveOptions(g_pIni);
}
void CWinPFGWDlg::OnUpdatePrpProofchoice(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nMode >= 2);
}
void CWinPFGWDlg::OnUpdateNm1PROOFCHOICE(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nMode == -1);
}
void CWinPFGWDlg::OnUpdateNp1PROOFCHOICE(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nMode == 1);
}
void CWinPFGWDlg::OnUpdatePp1Np1PROOFCHOICE(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nMode == 0);
}
void CWinPFGWDlg::OnUpdateFactorizeOnly(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nMode == -2);
}
void CWinPFGWDlg::OnUpdateFermatFactorOnly(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nMode == -3);
}

void CWinPFGWDlg::OnSetPrpBase() 
{
	/*
	CPrpBaseDlg Dlg;
	if(Dlg.DoModal() == IDOK)
		m_nPRPBase = Dlg.Base();
		*/
}

void CWinPFGWDlg::OnUpdateSetPrpBase(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nMode >= 2);
}
void CWinPFGWDlg::OnDecimalExpansion() 
{
	// TODO: Add your command handler code here
	
}
void CWinPFGWDlg::OnUpdateDecimalExpansion(CCmdUI* /*pCmdUI*/) 
{
	// TODO: Add your command update UI handler code here
	
}
void CWinPFGWDlg::OnFactorize() 
{
	m_bFactor = !m_bFactor;
	SaveOptions(g_pIni);
}
void CWinPFGWDlg::OnUpdateFactorize(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bFactor);
}

void CWinPFGWDlg::OnFermatFactor() 
{
	m_bFFactor = !m_bFFactor;
	SaveOptions(g_pIni);
}

void CWinPFGWDlg::OnUpdateFermatFactor(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bFFactor);
}

void CWinPFGWDlg::OnClearHighlight() 
{
	m_SmartEdit.ClearHighlight();
}

void CWinPFGWDlg::OnClearAll() 
{
	m_SmartEdit.ClearAll();
}

void CWinPFGWDlg::OnWindowPosChanging(WINDOWPOS *lpwndpos)
{
	if((lpwndpos->flags & SWP_SHOWWINDOW) && m_bHideOnStart)
	{
//		lpwndpos->flags &= ~SWP_SHOWWINDOW;
		m_bHideOnStart=FALSE;
	}
	CDialog::OnWindowPosChanging(lpwndpos);
}

