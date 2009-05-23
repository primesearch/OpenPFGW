// PFGW_ServerV2Dlg.cpp : implementation file
//

#include "stdafx.h"

#include <process.h>
#include <stdarg.h>
#include <io.h>
#include "PFGW_ServerV2.h"
#include "PFGW_ServerV2Dlg.h"
#include "tcp_ver2_xfer.h"
#include "min_pfio.cxx"
#include "pfgw_zlib.h"
#include "ini.h"
#include "pfoutput.h"
#include "winbloz_msg.h"

// this includes zlib.cpp without pfio
#include "zlib.cxx"

//#include "pfgw_globals.cxx"

#define JUSTIFY_CENTER	0
#define JUSTIFY_LEFT	1
#define JUSTIFY_RIGHT	2

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SERVER_ERROR_MESSAGE (WM_USER+666)
#define ADD_CLIENT_MESSAGE   (WM_USER+667)

#pragma comment (lib,"wsock32.lib")

IniFile g_Ini("./PFGW_ServerV2.ini");

WSADATA                         ws_data;
static struct sockaddr_in       http_addr;
char							http_host[256] = {"255.255.255.255"};
static u_short                  http_port  = 8831;
static char                     *da_host   = NULL;


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPFGW_ServerV2Dlg dialog

CPFGW_ServerV2Dlg::CPFGW_ServerV2Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPFGW_ServerV2Dlg::IDD, pParent),
	m_trayIcon(IDR_TRAY_MENU),
	m_InputFilesGrid(5,6,1,1),
	m_ClientsGrid(1,7,1,1)
{
	//{{AFX_DATA_INIT(CPFGW_ServerV2Dlg)
	m_AddFileString = _T("");
	m_nLinesListedForFile = 0;
	m_sNumClients = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_nFiles = 0;
	m_bDieServer = false;
	m_CurFile = 0;
	m_fpOutPrFile = m_fpOutCompFile = m_fpOutSkipFile = 0;
	m_bTrayIcon = false;
	m_bMinimized = false;
	m_nIconPulseCount = 0;
	m_bIconErrorState = false;
	memset(m_CurFiles, 0, sizeof(m_CurFiles));
	m_bIsFirstLineFile = false;
}

CPFGW_ServerV2Dlg::~CPFGW_ServerV2Dlg()
{
	// Save current state
//	SaveIniState();
}

void CPFGW_ServerV2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPFGW_ServerV2Dlg)
	DDX_Control(pDX, IDC_TEXT_MSG, m_TextMessages);
	DDX_Control(pDX, IDC_ADD_FILE_EDIT, m_AddFileEdt);
	DDX_Control(pDX, IDC_ADD_FILE, m_AddFileBtn);
	DDX_Control(pDX, IDC_INP_FILES_GRID, m_InputFilesGrid);
	DDX_Control(pDX, IDC_CLIENTS_GRID, m_ClientsGrid);
	DDX_Text(pDX, IDC_ADD_FILE_EDIT, m_AddFileString);
	DDX_Text(pDX, IDC_ADD_FILE_LINES_EDIT, m_nLinesListedForFile);
	DDX_Text(pDX, IDC_NUM_CLIENTS, m_sNumClients);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPFGW_ServerV2Dlg, CDialog)
	//{{AFX_MSG_MAP(CPFGW_ServerV2Dlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_ADD_FILE, OnAddFile)
	ON_WM_CLOSE()
	ON_COMMAND(IDC_USE_TRAY_ICON, OnUseTrayIcon)
	ON_UPDATE_COMMAND_UI(IDC_USE_TRAY_ICON, OnUpdateUseTrayIcon)
	ON_COMMAND(IDEXIT, OnExit)
	ON_WM_TIMER()
	ON_COMMAND(IDC_CLEAR_ICON_ERROR, OnClearIconError)
	ON_UPDATE_COMMAND_UI(IDC_CLEAR_ICON_ERROR, OnUpdateClearIconError)
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
    ON_MESSAGE(WM_MY_TRAY_NOTIFICATION, OnTrayNotification)
    ON_MESSAGE(SERVER_ERROR_MESSAGE, OnServerErrorMessage)
    ON_MESSAGE(ADD_CLIENT_MESSAGE, OnAddClientMessage)
	ON_MESSAGE(WinPFGW_MSG, OnPFGW_Message)
	// The damn dialog box does NOT handle menu popup's and ON_UPDATE_COMMAND_UI by default.
	// See MSDN article ID: Q242577 for the code to fix this "by design" BUG
	ON_WM_INITMENUPOPUP()

	// Grid notifincation messages
	ON_NOTIFY(NM_RCLICK, IDC_CLIENTS_GRID, OnClientGridRClick)
	ON_NOTIFY(NM_RCLICK, IDC_INP_FILES_GRID, OnFilesGridRClick)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPFGW_ServerV2Dlg message handlers

/*static*/ CGridCtrl *CPFGW_ServerV2Dlg::m_pClientsGrid;
/*static*/ int CALLBACK CPFGW_ServerV2Dlg::Client_Grid_CellCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParam3)
{
	CGridCellBase* pCell1 = (CGridCellBase*)lParam1;
	CGridCellBase* pCell2 = (CGridCellBase*)lParam2;
	switch(m_pClientsGrid->GetSortColumn())
	{
		case 0:
			// String sort
			return strcmp(pCell1->GetText(), pCell2->GetText());
		case 1:
		case 2:
		case 6:
			{
			// integer sort
			int nValue1 = atoi(pCell1->GetText());
			int nValue2 = atoi(pCell2->GetText());
			if (nValue1 < nValue2) return -1;
			if (nValue1 == nValue2) return 0;
			return 1;
			}
		case 3:
		case 5:
			// Date sort (a string sort does work, and we simply use that)
			return strcmp(pCell1->GetText(), pCell2->GetText());

		case 4:
			{
			// a floating point sort
			double dValue1 = atof(pCell1->GetText());
			double dValue2 = atof(pCell2->GetText());
			if (dValue1 < dValue2) return -1;
			if (dValue1 == dValue2) return 0;
			return 1;
			}

		// else, simply return 0 and DO NOT sort.
	}
	return 0;
}

BOOL CPFGW_ServerV2Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_sNumClients.Format("0 Active clients");

	m_pClientsGrid = &m_ClientsGrid;

	m_ClientsGrid.SetHeaderSort(TRUE);
	m_ClientsGrid.SetCompareFunction(Client_Grid_CellCompare);

	m_InputFilesGrid.SetEditable(FALSE);
	m_InputFilesGrid.SetRowResize(FALSE);
	m_InputFilesGrid.EnableSelection(FALSE);

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	// Set up tray icon   
	m_trayIcon.SetNotificationWnd(this, WM_MY_TRAY_NOTIFICATION);

	if (!bLoad_zLibDLL())
	{
		MessageBox("Error loading zLib.dll, so I will be exiting NOW!");
		return FALSE;
	}


	SetWindowText("PFGW Server Version 2 - Not serving");
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	SetupGridTitles();
	LoadIniState();

	// toggle the tray icon bool to the "wrong" value, and then simply simply simulate a "click" to the other (the right) state.
	if (m_bTrayIcon)
	{
		m_trayIcon.SetIcon(IDR_MAINFRAME);
		m_bTrayIcon = !m_bTrayIcon;
		OnUseTrayIcon();
	}
	
	m_trayIcon.SetToolTipString("PFGW_ServerV2");

    if (!http_init())
    {
        MessageBox("can't call http_init() this machine!!!");
        return FALSE;
    }
    if (http_open() == -1)
    {
        MessageBox("can't call http_open() this machine!!!");
        return FALSE;
    }

	// Only allow a "minimized state if running.  NOW minimize no matter what if we are "supposed to"
	if (m_bMinimized)
	{
		PostMessage(WM_SYSCOMMAND, 0xF020, 0);	// post a minimize command.
	}

	// Create a non-console Win32GUI output object.
	pOutputObj = new PFWin32GUIOutput((int)m_hWnd);

	// Startup the server listening thread
    m_ThreadHandle = (HANDLE)_beginthread(ListenThread, 0, (void*)this);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// A message coming from PFGW (currently only printf's and fprintf(stderr) messages
// NOTE we don't do anything with this, but we MUST delete the buffer.  Failure to
// do this is a MEMORY LEAK!!!!!  This has been a "possible" leak in the ServerV2
// for a LONG time.  Now that we are opening the files using the PFIO classes, they
// send us a PFPrintfStderr message (the file type) in the stage2_constructor.  Thus
// we MUST capture the message, and delete it.
afx_msg LRESULT CPFGW_ServerV2Dlg::OnPFGW_Message(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
		case M_STDERR:
		case M_PRINTF:
		{
			char *cp = (char*)lParam;
			// We MUST clean this up, or we will leak.  
			delete[] cp;
		}
	}
	return 0;
}

void CPFGW_ServerV2Dlg::OnClose() 
{
	m_bServerDead = false;
	m_bDieServer = true;
    closesocket(m_sMain);
    Sleep(200);
	// Now make sure listening thread "quits"
	int Cnt = 0;
	while (!m_bServerDead && ++Cnt < 10)
		Sleep(100);
    WSACleanup();
	CDialog::OnClose();
	delete pOutputObj;
}

void CPFGW_ServerV2Dlg::LoadIniState()
{
	CString s;
	g_Ini.IniGetString("Networking", "HostIP", &s, "bogus");
	if (s == "bogus")
	{
		g_Ini.IniSetString("Networking", "HostIP", "255.255.255.0");
		s = "255.255.255.0";
	}
	strcpy(http_host, s);

	int i;
	g_Ini.IniGetInt("Networking", "HostPort", &i, 888888888);
	if (i == 888888888)
	{
		g_Ini.IniSetInt("Networking", "HostPort", 23147);
		i = 23147;
	}
	http_port = (u_short)i;

	s.Format("PFGW Server Version 2 (%s:%d) Not Serving", http_host, http_port);
	SetWindowText(s);

	uint32 nFiles;
	g_Ini.IniGetUInt("WIP", "nfiles", &nFiles, 0);
	g_Ini.IniGetBool("WIP", "TrayIcon", &m_bTrayIcon, true);
	g_Ini.IniGetBool("WIP", "Minimized", &m_bMinimized, false);

	if (nFiles)
	{
		for (uint32 i = 0; i < nFiles; i++)
		{
			char Key[40];
			CString sFName;
			sprintf (Key, "File%d", i);
			g_Ini.IniGetString(Key, "FileName", &sFName, "");
			if (sFName == "")
			{
				char Buf[256];
				sprintf (Buf, "Error, section %s does not exist, or does not have a file name listed", Key);
				MessageBox(Buf, "Critical Error", 0);
				PostQuitMessage(-1);
				return;
			}
			strcpy(m_CurFiles[i].FName, (const char*)sFName);
			g_Ini.IniGetUInt(Key, "nLines", &(m_CurFiles[i].nLines), 0);
			g_Ini.IniGetUInt(Key, "nWip", &(m_CurFiles[i].nWip), 0);
			g_Ini.IniGetUInt(Key, "nDone", &(m_CurFiles[i].nDone), 0);
			g_Ini.IniGetUInt(Key, "nCurOffset", &(m_CurFiles[i].nCurOffset), 0);
			g_Ini.IniGetUInt(Key, "nSkipped", &(m_CurFiles[i].nSkipped), 0);

			FILE *in = fopen(m_CurFiles[i].FName, "rt");
			if (in)
			{
				fclose(in);	// All we want to do is see if the file is there or not.
				m_InputFilesGrid.SetItemText(i+1,0,m_CurFiles[i].FName);


				if (i == 0)
				{
					m_CurFile = fopen(m_CurFiles[0].FName, "rt");
					Check_First_Line();

					if (m_CurFiles[0].nCurOffset)
						// This will NOT work with ABCD format
						fseek(m_CurFile, m_CurFiles[0].nCurOffset, SEEK_SET);
					else
					{
						// Ini file without the nCurOffset element being set.
						char Line[256];
						uint32 nCurLine = 0, nDoneLines;
						nDoneLines = m_CurFiles[i].nDone + m_CurFiles[i].nSkipped + m_CurFiles[i].nWip;
						while (nCurLine < nDoneLines)
						{
							fgets(Line, sizeof(Line), m_CurFile);
							++nCurLine;
						}
					}
				}

				char Buf[40];
				_itoa(m_CurFiles[i].nLines, Buf, 10);
				m_InputFilesGrid.SetItemText(i+1,1,Buf);

				_itoa(m_CurFiles[i].nWip, Buf, 10);
				m_InputFilesGrid.SetItemText(i+1,2,Buf);

				_itoa(m_CurFiles[i].nDone, Buf, 10);
				m_InputFilesGrid.SetItemText(i+1,3,Buf);

				_itoa(m_CurFiles[i].nSkipped, Buf, 10);
				m_InputFilesGrid.SetItemText(i+1,4,Buf);

				int ToDo = m_CurFiles[i].nLines;
				ToDo -=	m_CurFiles[i].nWip;
				ToDo -= m_CurFiles[i].nDone;
				ToDo -=	m_CurFiles[i].nSkipped;
				_itoa(ToDo, Buf, 10);
				m_InputFilesGrid.SetItemText(i+1,5,Buf);

				// Now we have a "successfully" loaded file, so increase the count by 1.
				m_nFiles++;
			}
		}
		if (m_nFiles > 3)
		{
			m_AddFileEdt.EnableWindow(FALSE);
			m_AddFileBtn.EnableWindow(FALSE);
		}
		s.Format("PFGW Server Version 2 (%s:%d) Serving", http_host, http_port);
		SetWindowText(s);
	}
	OpenOutputFiles();
}

void CPFGW_ServerV2Dlg::SaveIniState()
{
	g_Ini.IniSetUInt("WIP", "nFiles", m_nFiles);
	g_Ini.IniSetBool("WIP", "TrayIcon", m_bTrayIcon);
	g_Ini.IniSetBool("WIP", "Minimized", m_bMinimized);
	for (uint32 i = 0; i < m_nFiles; i++)
	{
		char Key[40];
		CString s;
		sprintf (Key, "File%d", i);
		g_Ini.IniSetString(Key, "FileName", m_CurFiles[i].FName);
		g_Ini.IniSetUInt(Key, "nLines", m_CurFiles[i].nLines);
		g_Ini.IniSetUInt(Key, "nWip", m_CurFiles[i].nWip);
		g_Ini.IniSetUInt(Key, "nDone", m_CurFiles[i].nDone);
		g_Ini.IniSetUInt(Key, "nCurOffset", m_CurFiles[i].nCurOffset);
		g_Ini.IniSetUInt(Key, "nSkipped", m_CurFiles[i].nSkipped);

		g_Ini.Flush();
	}
}

bool CPFGW_ServerV2Dlg::http_init(void)
{
    WORD			wVersionRequested;
	WSADATA			wsaData;
    wVersionRequested = MAKEWORD(1, 1);
	if(WSAStartup(wVersionRequested, &wsaData))
        return false;
    return true;
}

// handle D[DD]sD[DD]sD[DD]sD[DD]   D == digit (base 10 only)  s == . , or -  (peroid comma or dash)
// if the passed in host is not in the above format, this function returns 0, and then a gethostbyname() 
// function call is needed
DWORD DottedQuad(char *host)
{
    DWORD IP = 0, TmpIP;
    char *Buf, *cp;

    Buf = new char[strlen(host)+1];
    strcpy(Buf, host);
    cp = strtok(Buf, ".,-");
    if (cp && atoi(cp) < 256 && atoi(cp) > 0)
    {
        TmpIP = atoi(cp) << 24;
        cp = strtok(0, ".,-");
        if (cp && atoi(cp) < 256 && atoi(cp) >= 0)
        {
            TmpIP |= (atoi(cp) << 16);
            cp = strtok(0, ".,-");
            if (cp && atoi(cp) < 256 && atoi(cp) >= 0)
            {
                TmpIP |= (atoi(cp) << 8);
                cp = strtok(0, "");
                if (cp && atoi(cp) < 256 && atoi(cp) >= 0)
                {
                    char *cp1 = cp;
                    while ( *cp1 >= '0' && *cp1 <= '9')
                        cp1++;
                    if (*cp1 == 0)
                    {
                        TmpIP |= atoi(cp);
                        IP = TmpIP;
                    }
                }
            }
        }
    }
    delete[] Buf;
    return IP;
}

SOCKET CPFGW_ServerV2Dlg::http_open(void)
{
	memset(&http_addr, 0, sizeof(struct sockaddr_in));
	http_addr.sin_family = AF_INET;

    DWORD IPVal = DottedQuad(http_host);
    if (!IPVal)
    {
	    struct hostent	*entry;
	    if((entry = gethostbyname(http_host)) == NULL)
	    {				
            char Buf[256];

		    sprintf (Buf, "\'%s\' not fnd, err(%d)", http_host, WSAGetLastError());
			MessageBox(Buf);
            return -1; 
	    }
        // sanity check code added (this was in the 1.1.0 ecdl108 code from Rob, but Bryan did have have it in.
        if ( entry->h_length < 0 || (size_t)entry->h_length > sizeof(http_addr.sin_addr.s_addr))
        {
            puts("Error: address buffer overflow!");
            return -1;
        }
        memcpy(&http_addr.sin_addr.s_addr, entry->h_addr, entry->h_length);
    }
    else
        http_addr.sin_addr.s_addr = htonl(IPVal);

	http_addr.sin_port = htons(http_port);

	return 0;
}

void CPFGW_ServerV2Dlg::http_close(void)
{
}

void CPFGW_ServerV2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if ((nID & 0xFFF0) == SC_MINIMIZE && m_bTrayIcon)
	{
		// This actually sends the program to the tray
		ShowWindow(SW_HIDE);
		m_bMinimized = true;
		SaveIniState();
	}	
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPFGW_ServerV2Dlg::OnPaint() 
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
HCURSOR CPFGW_ServerV2Dlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

afx_msg LRESULT CPFGW_ServerV2Dlg::OnTrayNotification(WPARAM /*uID*/, LPARAM lEvent)
{
    if (WM_MOUSEFIRST <=lEvent && lEvent<=WM_MOUSELAST)
    {
        if (lEvent-WM_MOUSEFIRST == 3)
        {
            ShowWindow(SW_SHOWNORMAL);
            ::SetForegroundWindow(m_hWnd);
            SetFocus();
			m_bMinimized = false;
			SaveIniState();
        }
    }
    return 0;
}

afx_msg LRESULT CPFGW_ServerV2Dlg::OnServerErrorMessage(WPARAM wParam, LPARAM lParam)
{
	char *cpMsg = (char*)lParam;
	bool bFlashIcon = !!wParam;
	if (!cpMsg)
		return 0;
	char Buf[512];
	time_t t = time(0);
	struct tm *t_tm = localtime(&t);
	sprintf (Buf, "%02d/%02d %02d:%02d:%02d  %s", t_tm->tm_mday, t_tm->tm_mon+1, t_tm->tm_hour, t_tm->tm_min, t_tm->tm_sec, cpMsg);
	m_TextMessages.AddString(Buf);
	m_TextMessages.SetCurSel(0);
	if (bFlashIcon)
		FlashIcon();
	delete[] cpMsg;
	return 0;
}

afx_msg LRESULT CPFGW_ServerV2Dlg::OnAddClientMessage(WPARAM uID, LPARAM lEvent)
{	
	m_ClientsGrid.SetRowCount(m_ClientData.nClients()+2);

	m_sNumClients.Format("%d Active clients", m_ClientData.nClients()+1);
	UpdateData(FALSE);
    m_ClientData.AddNewClient((const char *)lEvent);
    return 0;
}


void CPFGW_ServerV2Dlg::FlashIcon()
{
	m_bIconErrorState = true;
	SetTimer(1, 1000, NULL);
	m_nIconPulseCount = 0;
	m_trayIcon.SetIcon(IDI_ICON_RED);
	CString s;
	m_TextMessages.GetLBText(0, s);
	m_trayIcon.SetToolTipString(s);
}

void CPFGW_ServerV2Dlg::OnTimer(UINT nIDEvent) 
{
	KillTimer(nIDEvent);
	if (nIDEvent == 1)
	{
		m_trayIcon.SetIcon(IDI_ICON_YELLOW);
		SetTimer(2, 1000, NULL);
	}
	else if (nIDEvent == 2)
	{
		m_trayIcon.SetIcon(IDI_ICON_RED);
		if (++m_nIconPulseCount < 5)
			SetTimer(1, 1000, NULL);
	}
	CDialog::OnTimer(nIDEvent);
}


void CPFGW_ServerV2Dlg::OnClearIconError() 
{
	KillTimer(1);
	KillTimer(2);
	m_nIconPulseCount = 0;
	m_bIconErrorState = false;
	m_trayIcon.SetIcon(IDR_MAINFRAME);
}

void CPFGW_ServerV2Dlg::OnUpdateClearIconError(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bIconErrorState);
}

// The damn dialog box does NOT handle menu popup's and ON_UPDATE_COMMAND_UI by default.
// See MSDN article ID: Q242577 for the code to fix this "by design" BUG
void CPFGW_ServerV2Dlg::OnInitMenuPopup(CMenu *pPopupMenu, UINT /*nIndex*/,BOOL /*bSysMenu*/)
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

void CPFGW_ServerV2Dlg::OnAddFile() 
{
	UpdateData();
	if (m_nFiles > 3)
	{
		m_AddFileEdt.EnableWindow(FALSE);
		m_AddFileBtn.EnableWindow(FALSE);
		return;
	}
	if (!_access(m_AddFileString, 0))
	{
		FILE *in = fopen(m_AddFileString, "rt");
		if (!in)
			MessageBox("Error Opening the file!, Not added");
		else
		{
			m_InputFilesGrid.SetItemText(m_nFiles+1,0,m_AddFileString);

			unsigned nLines = 0;
			char Buf[4096];
			if (m_nLinesListedForFile)
				nLines = m_nLinesListedForFile;
			else
			{
				fgets(Buf, sizeof(Buf), in);
				while (!feof(in))
				{
					fgets(Buf, sizeof(Buf), in);
					++nLines;
				}
			}
			fclose(in);
			if (!nLines)
			{
				MessageBox("No data in the file!, Not added");
				return;
			}

			sprintf (Buf, "%d", nLines);

			strcpy(m_CurFiles[m_nFiles].FName, m_AddFileString);
			m_CurFiles[m_nFiles].nLines = nLines;
			m_CurFiles[m_nFiles].nWip = 0;
			m_CurFiles[m_nFiles].nDone = 0;
			m_CurFiles[m_nFiles].nCurOffset = 0;
			m_CurFiles[m_nFiles].nSkipped = 0;

			m_InputFilesGrid.SetItemText(m_nFiles+1,1,Buf);
			m_InputFilesGrid.SetItemText(m_nFiles+1,2,"0");
			m_InputFilesGrid.SetItemText(m_nFiles+1,3,"0");
			m_InputFilesGrid.SetItemText(m_nFiles+1,4,"0");
			m_InputFilesGrid.SetItemText(m_nFiles+1,5,Buf);

			// Now enable the thread to read data
			if (++m_nFiles == 1)
			{
				OpenOutputFiles();
				m_CurFile = fopen(m_CurFiles[0].FName, "rt");
				if (m_CurFile)
					Check_First_Line();
			}
			m_nLinesListedForFile = 0;
			m_AddFileString = "";
			UpdateData(FALSE);
		}
	}
	else
		MessageBox("Error, cant find listed file");

	if (m_nFiles > 3)
	{
		m_AddFileEdt.EnableWindow(FALSE);
		m_AddFileBtn.EnableWindow(FALSE);
	}
	// Save the running state.
	SaveIniState();

	CString s;
	if (m_nFiles)
		s.Format("PFGW Server Version 2 (%s:%d) Serving", http_host, http_port);
	else
		s.Format("PFGW Server Version 2 (%s:%d) Not Serving", http_host, http_port);
	SetWindowText(s);
}

void CPFGW_ServerV2Dlg::SetupGridTitles()
{
	// Setup column titles
	m_InputFilesGrid.SetColumnWidth(0, 245);
	m_InputFilesGrid.SetColumnWidth(1, 83);
	m_InputFilesGrid.SetColumnWidth(2, 78);
	m_InputFilesGrid.SetColumnWidth(3, 83);
	m_InputFilesGrid.SetColumnWidth(4, 72);
	m_InputFilesGrid.SetColumnWidth(5, 83);

	// Setup titles in row 0.
	m_InputFilesGrid.SetItemText(0,0,"FileName");
	m_InputFilesGrid.SetItemText(0,1,"Tot Items");
	m_InputFilesGrid.SetItemText(0,2,"WIP Items");
	m_InputFilesGrid.SetItemText(0,3,"Done Items");
	m_InputFilesGrid.SetItemText(0,4,"Skipped");
	m_InputFilesGrid.SetItemText(0,5,"ToDo Items");

	// Setup column titles
	m_ClientsGrid.SetColumnWidth(0, 175);
	m_ClientsGrid.SetColumnWidth(1, 62);
	m_ClientsGrid.SetColumnWidth(2, 70);
	m_ClientsGrid.SetColumnWidth(3, 90);
	m_ClientsGrid.SetColumnWidth(4, 96);
	m_ClientsGrid.SetColumnWidth(5, 90);
	m_ClientsGrid.SetColumnWidth(6, 90);

	// Setup titles in row 0.
	m_ClientsGrid.SetItemText(0,0,"Client Name / IP");
	m_ClientsGrid.SetItemText(0,1,"# WIP");
	m_ClientsGrid.SetItemText(0,2,"# Done");
	m_ClientsGrid.SetItemText(0,3,"Last Seen");
	m_ClientsGrid.SetItemText(0,4,"AveRate");
	m_ClientsGrid.SetItemText(0,5,"Finish Dt");
	m_ClientsGrid.SetItemText(0,6,"Prod Score");
}

void CPFGW_ServerV2Dlg::UpdateCurFilesGrid()
{
	uint32 i=0;
	bool bRemoved = false;
	for(; i < m_nFiles; i++)
	{
		if (m_CurFiles[i].nLines == (m_CurFiles[i].nDone+m_CurFiles[i].nSkipped))
		{
			// Remove this item from the grid.
			CString s, sTime;
			CTime t = CTime::GetCurrentTime();
			sTime = t.Format("%H:%M %a %b %d - ");
			s.Format(" %s completed-1", m_CurFiles[i].FName);
			m_TextMessages.AddString(sTime + s);
			m_TextMessages.SetCurSel(m_TextMessages.GetCount()-1);
			m_ClientsGrid.SetItemText(i+1,0,"");
			m_ClientsGrid.SetItemText(i+1,1,"");
			m_ClientsGrid.SetItemText(i+1,2,"");
			m_ClientsGrid.SetItemText(i+1,3,"");
			m_ClientsGrid.SetItemText(i+1,4,"");
			m_ClientsGrid.SetItemText(i+1,5,"");
			for (unsigned j = i; j < m_nFiles; ++j)
			{
				m_CurFiles[j] = m_CurFiles[j+1];

				m_InputFilesGrid.SetItemText(j+1,0,m_CurFiles[j].FName);

				char Buf[40];
				_itoa(m_CurFiles[j].nLines, Buf, 10);
				m_InputFilesGrid.SetItemText(j+1,1,Buf);

				_itoa(m_CurFiles[j].nWip, Buf, 10);
				m_InputFilesGrid.SetItemText(j+1,2,Buf);

				_itoa(m_CurFiles[j].nDone, Buf, 10);
				m_InputFilesGrid.SetItemText(j+1,3,Buf);

				_itoa(m_CurFiles[j].nSkipped, Buf, 10);
				m_InputFilesGrid.SetItemText(j+1,4,Buf);

				int ToDo = m_CurFiles[j].nLines;
				ToDo -= m_CurFiles[j].nWip;
				ToDo -= m_CurFiles[j].nDone;
				ToDo -= m_CurFiles[j].nSkipped;

				_itoa(ToDo, Buf, 10);
				m_InputFilesGrid.SetItemText(j+1,5,Buf);
			}
			--m_nFiles;
			bRemoved = true;
		}
		else
		{
			m_InputFilesGrid.SetItemText(i+1,0,m_CurFiles[i].FName);

			char Buf[40];
			_itoa(m_CurFiles[i].nLines, Buf, 10);
			m_InputFilesGrid.SetItemText(i+1,1,Buf);

			_itoa(m_CurFiles[i].nWip, Buf, 10);
			m_InputFilesGrid.SetItemText(i+1,2,Buf);

			_itoa(m_CurFiles[i].nDone, Buf, 10);
			m_InputFilesGrid.SetItemText(i+1,3,Buf);

			_itoa(m_CurFiles[i].nSkipped, Buf, 10);
			m_InputFilesGrid.SetItemText(i+1,4,Buf);

			int ToDo = m_CurFiles[i].nLines;
			ToDo -= m_CurFiles[i].nWip;
			ToDo -= m_CurFiles[i].nDone;
			ToDo -= m_CurFiles[i].nSkipped;

			_itoa(ToDo, Buf, 10);
			m_InputFilesGrid.SetItemText(i+1,5,Buf);
		}
	}
	if (bRemoved)
	{
		if (!m_nFiles)
		{
			CString s;
			CTime t = CTime::GetCurrentTime();
			s = t.Format("%H:%M %a %b %d - ALL DONE!! (1)");
			m_InputFilesGrid.SetItemText(1,0,s);
			m_TextMessages.AddString(s);
		}
		else
		{
			m_InputFilesGrid.SetItemText(m_nFiles+1,0,"");
			m_InputFilesGrid.SetItemText(m_nFiles+1,1,"");
			m_InputFilesGrid.SetItemText(m_nFiles+1,2,"");
			m_InputFilesGrid.SetItemText(m_nFiles+1,3,"");
			m_InputFilesGrid.SetItemText(m_nFiles+1,4,"");
			m_InputFilesGrid.SetItemText(m_nFiles+1,5,"");
		}
	}
}

void CPFGW_ServerV2Dlg::UpdClientsGrid()
{
	// We really should already BE in a critcal section, but we have left this in the code, none
	// the less, since a thread can enter a crit section as many times as it likes (as long as
	// it leaves the same number of times).
	m_ClientsGrid.EnterCriticalSection();

    if (m_ClientData.nCurClient() == 0xFFFFFFFF || !m_ClientData.nCurClient())
    {
        m_ClientsGrid.LeaveCriticalSection();
        return;
    }

	unsigned which_row;
	CString ClientName = m_ClientData[m_ClientData.nCurClient()].ClientName;
	bool bFound = false;
	for (which_row = 1; which_row <= m_ClientData.nClients()+1; ++which_row)
	{
		if (ClientName == m_ClientsGrid.GetItemText(which_row, 0))
		{
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		// Not sure just how to handle this.  The entry SHOULD already be 
		// created (I think), so I don't think we can get here
		ASSERT(false);
		m_ClientsGrid.LeaveCriticalSection();
        return;
	}

	char Buf[100];
	if ((int)m_ClientData[m_ClientData.nCurClient()].nWip > 0)
	{
		sprintf (Buf, "%d", m_ClientData[m_ClientData.nCurClient()].nWip);
		m_ClientsGrid.SetItemText(which_row, 1, Buf);
	}
	else
	{
		m_ClientsGrid.SetItemText(which_row, 1, "Unk");
		m_ClientData[(uint16)m_ClientData.nCurClient()].nWip = 0;
	}

	sprintf (Buf, "%d", m_ClientData[m_ClientData.nCurClient()].nDone);
	m_ClientsGrid.SetItemText(which_row, 2, Buf);

	struct tm *t_tm = localtime(&m_ClientData[m_ClientData.nCurClient()].tLastContact);
	sprintf (Buf, "%02d/%02d %02d:%02d", t_tm->tm_mon+1, t_tm->tm_mday, t_tm->tm_hour, t_tm->tm_min);
	m_ClientsGrid.SetItemText(which_row, 3, Buf);

	*Buf = 0;
	if (m_ClientData[m_ClientData.nCurClient()].dAveRate > .0001)
		sprintf (Buf, "%0.3f s/per", m_ClientData[m_ClientData.nCurClient()].dAveRate);
	m_ClientsGrid.SetItemText(which_row, 4, Buf);

	if (m_ClientData[m_ClientData.nCurClient()].nWip != -8888888)
	{
		time_t t = m_ClientData[m_ClientData.nCurClient()].tLastContact;
		t += uint32(m_ClientData[m_ClientData.nCurClient()].dAveRate*m_ClientData[m_ClientData.nCurClient()].nWip);
		t_tm = localtime(&t);
		sprintf (Buf, "%02d/%02d %02d:%02d", t_tm->tm_mon+1, t_tm->tm_mday, t_tm->tm_hour, t_tm->tm_min);
		m_ClientsGrid.SetItemText(which_row, 5, Buf);
	}
	strcpy(Buf, "0");
	if (m_ClientData[m_ClientData.nCurClient()].Score > 0)
		sprintf(Buf, "%0.4f", m_ClientData[m_ClientData.nCurClient()].Score);
	m_ClientsGrid.SetItemText(which_row, 6, Buf);

	m_ClientsGrid.Invalidate();

	m_ClientsGrid.LeaveCriticalSection();

}

// Eliminates the first line, and moves up the other lines of the grid, AND it
// switches the data in the CurFiles data to match.
void CPFGW_ServerV2Dlg::FileDone()
{
	m_InputFilesGrid.SetItemText(m_nFiles, 0, "");
	m_InputFilesGrid.SetItemText(m_nFiles, 1, "");
	m_InputFilesGrid.SetItemText(m_nFiles, 2, "");
	m_InputFilesGrid.SetItemText(m_nFiles, 3, "");
	m_InputFilesGrid.SetItemText(m_nFiles, 4, "");
	m_InputFilesGrid.SetItemText(m_nFiles, 5, "");

	uint32 i;
	CurFiles_t Tmp = m_CurFiles[0];
	for (i = 0; i < m_nFiles; ++i)
		m_CurFiles[i] = m_CurFiles[i+1];
	m_nFiles--;

	if (Tmp.nWip)
		m_CurFiles[m_nFiles++] = Tmp;
	else
	{
		CString s, sTime;
		CTime t = CTime::GetCurrentTime();
		sTime = t.Format("%H:%M %a %b %d - ");
		s.Format(" %s completed-2", m_CurFiles[i].FName);
		m_TextMessages.AddString(sTime + s);
		//m_TextMessages.ShowDropDown();
		m_TextMessages.SetCurSel(m_TextMessages.GetCount()-1);
	}

	OpenOutputFiles();

	UpdateCurFilesGrid();
	if (m_nFiles == 0)
	{
		CString s;
		CTime t = CTime::GetCurrentTime();
		s = t.Format("%H:%M %a %b %d - ALL DONE!! (2)");
		m_InputFilesGrid.SetItemText(1,0,s);
		m_TextMessages.AddString(s);
	}
	m_AddFileEdt.EnableWindow(TRUE);
	m_AddFileBtn.EnableWindow(TRUE);

	CString s;
	if (m_nFiles)
		s.Format("PFGW Server Version 2 (%s:%d) Serving", http_host, http_port);
	else
		s.Format("PFGW Server Version 2 (%s:%d) Not Serving", http_host, http_port);
	SetWindowText(s);
}

void CPFGW_ServerV2Dlg::OpenOutputFiles()
{
	CString s;

	if (m_fpOutPrFile)
		fclose(m_fpOutPrFile);
	if (m_fpOutCompFile)
		fclose(m_fpOutCompFile);
	if (m_fpOutSkipFile)
		fclose(m_fpOutSkipFile);

	if (m_nFiles == 0)
	{
		s.Format("PFGW-ServerV2-Default_Primes.log");
		m_fpOutPrFile = fopen(s, "at");
		s.Format("PFGW-ServerV2-Default_Composites.txt");
		m_fpOutCompFile = fopen(s, "at");
		s.Format("PFGW-ServerV2-Default_SkippedWork.txt");
		m_fpOutSkipFile = fopen(s, "at");
	}
	else
	{
		s.Format("%s_Primes.log", m_CurFiles[0].FName);
		m_fpOutPrFile = fopen(s, "at");
		s.Format("%s_Composites.txt", m_CurFiles[0].FName);
		m_fpOutCompFile = fopen(s, "at");
		s.Format("%s_SkippedWork.txt", m_CurFiles[0].FName);
		m_fpOutSkipFile = fopen(s, "at");
	}
}

void CPFGW_ServerV2Dlg::Check_First_Line()
{
	fgets(m_FirstLine, sizeof(m_FirstLine), m_CurFile);
	m_bIsFirstLineFile=false;

	char c;
	uint64 u64tmp;	// Unused right now, but may be useful in the future (tells the depth of the sieving)
	int len, base, bits;
	int count = sscanf(m_FirstLine, "%I64u:%c:%d:%d:%d", &u64tmp, &c, &len, &base, &bits);
	if (count == 5)
		m_bIsFirstLineFile=true;	// newPgen file
	else if (!strncmp(m_FirstLine, "ABC ", 4))
		m_bIsFirstLineFile=true;	// ABC file
	else if (!strncmp(m_FirstLine, "ABCD ", 5))
		m_bIsFirstLineFile=true;	// ABCD file
	if (!m_bIsFirstLineFile)
		rewind(m_CurFile);
}

/*static*/ void CPFGW_ServerV2Dlg::ListenThread(void *p)
{
    CPFGW_ServerV2Dlg *This = (CPFGW_ServerV2Dlg*)p;
    bool bCloseWhenDone, bFirstTry;
    uint32 nCurClient;
    bool bProcess;
    int Requested, WorkDones, Skips;
	char *cpData;


    // local socket returned from accept() function
    SOCKET	s;

    if((This->m_sMain = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {				
        This->MessageBox("Warning: could not start an http request, Socket Dead!");
        return;
    }

    int skLen = sizeof(http_addr);
    int err = bind(This->m_sMain, (struct sockaddr*)&http_addr, skLen);
    err = WSAGetLastError();

    This->OpenOutputFiles();

    DWORD dwSize = 256*1024;
    setsockopt(This->m_sMain, SOL_SOCKET, SO_RCVBUF, (const char*)&dwSize, sizeof(DWORD));
    setsockopt(This->m_sMain, SOL_SOCKET, SO_SNDBUF, (const char*)&dwSize, sizeof(DWORD));

    CWnd *pMsgWnd = (CStatic*)This->GetDlgItem(IDC_TEXT_MSG);

    while (!listen(This->m_sMain, 50))
    {
        s = accept(This->m_sMain, (struct sockaddr*)&http_addr, &skLen);
        if(s == INVALID_SOCKET )
        {
            if (This->m_bDieServer)
            {
                This->m_bServerDead = true;
                return;
            }
            //This->http_error(http_host, "could not connect");
            continue;
        }
		cpData=0;
        static char iBuf[256*1024+sizeof(tcp_ver2_xfer)], CmprBuf[256*1024+sizeof(tcp_ver2_xfer)];
        *iBuf = 0;
        int iBufIn=0;
        int iTimes = 0;
        int Len;
        DWORD dwToGet;
        char *cpCurP;
        tcp_ver2_xfer *pRX = (tcp_ver2_xfer *)iBuf;
        
        char *cpXBase = (char*)pRX;
        
        dwToGet = sizeof(tcp_ver2_xfer)-1;
        cpCurP = pRX->cpData;
        while(dwToGet)
        {
            Len = recv(s, cpXBase, sizeof(tcp_ver2_xfer)-1, 0);
            if (Len == SOCKET_ERROR || Len == 0)
            {
                char *cp = new char[256];
                strcpy(cp,"SOCKERR reading xfer header!");
                This->PostMessage(SERVER_ERROR_MESSAGE, true, (LPARAM)cp);
                goto AbortThisRead;
            }
            dwToGet -= Len;
            cpCurP += Len;
        }

        // NOTE max packet length is "only" 256k
        if (pRX->u32PackedLen < 256*1024)
        {
            dwToGet = pRX->u32PackedLen;
            cpCurP = pRX->cpData;
            while (dwToGet)
            {
                Len = recv(s, cpCurP, dwToGet, 0);
                if (Len == SOCKET_ERROR || Len == 0)
                {
                    if (dwToGet == pRX->u32PackedLen)
                        goto AbortThisRead;
                    pRX->u32PackedLen -= dwToGet;
                    pRX->cpData[pRX->u32PackedLen] = 0;
                    // ERROR Message about a "broken" packet.
                    break;
                }
                cpCurP += Len;
                dwToGet -= Len;
            }
        }
        else
        {
            // Error Message
            goto AbortThisRead;
        }
        bProcess = This->m_CurFile != NULL;
        if (pRX->eCompressed == e_COMPRESSION_ZLIB)
        {
            // decompress the thing first
            memcpy(CmprBuf, iBuf, pRX->u32PackedLen+sizeof(tcp_ver2_xfer));
            tcp_ver2_xfer *pRX2 = (tcp_ver2_xfer *)CmprBuf;
            PFGW_inflate((uint8*)pRX->cpData, (uint8*)pRX2->cpData, pRX->u32PackedLen, &pRX->u32UnpackedLen);
            if (pRX->u32UnpackedLen != pRX2->u32UnpackedLen)
            {
                char *cp = new char[256];
                sprintf(cp, "Error, should have decompressed to %d bytes, not %d bytes", pRX->u32UnpackedLen, pRX2->u32UnpackedLen);
                This->PostMessage(SERVER_ERROR_MESSAGE, true, (LPARAM)cp);
            }
            pRX->u32PackedLen = pRX->u32UnpackedLen;
            pRX->eCompressed = e_COMPRESSION_NONE;
        }
        char *cp, *cp1;
        cp = pRX->cpData;
        cp1 = strchr(pRX->cpData, '\n');
        if (cp1)
            *cp1 = 0;
        Requested=0, WorkDones=0, Skips=0;
        // Ok, now "find" the correct entry for this machine

        This->m_ClientsGrid.EnterCriticalSection();

		bFirstTry=true;
FindMeAgain:;
        nCurClient = This->m_ClientData.FindMachine(pRX->szMachineName);
        if (nCurClient == 0xFFFFFFFF)
        {
			if (!bFirstTry)
				// Bail like crap here!!
				goto AbortThisRead;
            DWORD dwRes;
			DWORD dwNow = This->m_ClientData.nClients();
			static char szMacName[41];
			strcpy(szMacName, pRX->szMachineName);
            ::SendMessageTimeout(This->m_hWnd, ADD_CLIENT_MESSAGE, 0, (LPARAM)szMacName, SMTO_BLOCK, 5000, &dwRes);
			int Cnt = 0;
			while (++Cnt < 10 && dwNow == This->m_ClientData.nClients())
				Sleep(500);
			if (dwNow == This->m_ClientData.nClients())
				// Bail like crap here!!
				goto AbortThisRead;

			nCurClient = This->m_ClientData.nClients();
			This->m_ClientsGrid.SetItemText(nCurClient, 0, pRX->szMachineName);

            //strcpy(This->m_ClientData[(uint16)nCurClient].ClientName, pRX->szMachineName);
            //This->m_ClientData[(uint16)nCurClient].tLastContact = time(0);
            //This->m_ClientData[(uint16)nCurClient].dAveRate = 1;
			bFirstTry=false;
			goto FindMeAgain;
        }
		if (cp && *cp)
		{
			cpData = new char [strlen(cp)+1];
			strcpy(cpData, cp);
			cp = cpData;
		}

        bCloseWhenDone=false;

        if (cpData)
        {
			// Pass in the "allocated" data
            HANDLE hClient = This->m_ClientData.ClientStats_Begin(pRX->szMachineName, cpData);

            if (hClient == INVALID_HANDLE_VALUE)
			{
		        This->m_ClientsGrid.LeaveCriticalSection();
                goto AbortThisRead;
			}

            while (cp && *cp)
            {
                if (cp[2] != ' ')
				{
			        This->m_ClientsGrid.LeaveCriticalSection();
                    goto AbortThisRead;
				}

                // WARNING, little endian specific code!!!! but since server2 is only a VisualC project, then 
                // who cares!
                switch (*(uint16*)cp)
                {
                    case 'RP':  // "PR ..."  command (or Proceed Result)
                    {
                        This->m_ClientData.ClientStats_AddWorkDone(hClient, cp);
                        ++WorkDones;
                        break;
                    }
                    case 'NP':  // "PN ..."  command (or Prep New work)
                    {
						// At this time, do nothing.
						break;
					}
                    case 'NG':  // "GN ..."  command (or Get New work)
                    {
                        //This->m_ClientData.ClientStats_GetNewWork(hClient, cp);
                        if (!bProcess || !This->m_nFiles)
                        {
                            static char NoDataBuf[50];
                            tcp_ver2_xfer *pXfer = (tcp_ver2_xfer*)NoDataBuf;
                            pXfer->u32UnpackedLen = pXfer->u32PackedLen = 8;
                            strcpy(pXfer->cpData, "NoData\n");
                            send(s, NoDataBuf, pXfer->u32PackedLen-1+sizeof(tcp_ver2_xfer), 0);
                        }
                        else
                        {
                            uint32 nNumWanted = atoi(&cp[3]);
							bool bShouldCompress = pRX->eRCompress == e_COMPRESSION_ZLIB;
							This->m_ClientData.ClientStats_GetNewWork(hClient, nNumWanted, s, This->m_CurFile, bCloseWhenDone, bShouldCompress, This->m_bIsFirstLineFile, This->m_FirstLine, This->m_CurFiles[0].FName);
							// NOTE that the call above will "update" nNumWanted variable with the actual count of number SENT!
							Requested = nNumWanted;
                        }
                        break;
                    }
                    case 'DN':  // "ND .. " command (or Not Done  i.e. skipped data)
                    {
                        This->m_ClientData.ClientStats_AddWorkSkipped(hClient, cp);
                        ++Skips;
                        break;
                    }
                    case 'IL':  // "LI ..." command (or LogIn)
                    {
                        // A login attemp.
                        // Find this guy, and log him in
                        This->m_ClientData.ClientStats_LogIn(hClient, pRX->szMachineName);
                        break;
                    }
                    case 'OL':  // "LO ..."  command (or LogOut)
                    {
                        // A login attemp.
                        // Find this guy and log him out
                        This->m_ClientData.ClientStats_LogOut(hClient, pRX->szMachineName);
                        break;
                    }
                    case 'PO':  // "OP ..."  command (or OPeration)
                    {
                        // current valid "ops" are:
                        //  OP CompressionOK
                        //  OP Proto_v_2_01
                        //  OP NOCompression
                        // however I don't do anything with them yet.
                        break;
                    }
                }

                if (cp1)
                {
                    cp = cp1 + 1;
                    cp1 = strchr(cp, '\n');
                    if (cp1)
                        *cp1 = 0;
                }
                else
                    cp = 0;
            }

			bool bUpdateCurFileGrid, bDataCameFromCurrent=true;
			bUpdateCurFileGrid=true;

            if ( (int)This->m_ClientData[(uint16)nCurClient].nWip > 0)
            {
                This->m_ClientData[(uint16)nCurClient].nWip -= WorkDones;
                This->m_ClientData[(uint16)nCurClient].nWip -= Skips;
            }
            else
                This->m_ClientData[(uint16)nCurClient].nWip = 0;
        
            This->m_ClientData[(uint16)nCurClient].nWip += Requested;
            This->m_ClientData[(uint16)nCurClient].nDone += WorkDones;
            if (WorkDones)
            {
                unsigned t = time(0) - This->m_ClientData[nCurClient].tLastContact;
                This->m_ClientData[(uint16)nCurClient].dAveRate = t;
                This->m_ClientData[(uint16)nCurClient].dAveRate /= WorkDones;
                This->m_ClientData[(uint16)nCurClient].tLastContact = time(0);
            }

			int nWhichWasDone = -1;
			if (WorkDones || Skips)
			{
				if (!_stricmp(This->m_ClientData.ClientStats_CurInFileName(), This->m_CurFiles[0].FName))
				{
					nWhichWasDone = 0;
					This->m_ClientData.ClientStats_End(hClient, This->m_fpOutPrFile, This->m_fpOutCompFile, This->m_fpOutSkipFile);
				}
				else
				{
					bool bFnd = false;
					for (nWhichWasDone = 1; nWhichWasDone < (int)This->m_nFiles; ++nWhichWasDone)
					{
						if (!_stricmp(This->m_ClientData.ClientStats_CurInFileName(), This->m_CurFiles[nWhichWasDone].FName))
						{
							FILE *fpCmp, *fpSkip, *fpPr;
							CString s;
							s.Format("%s_Primes.log", This->m_CurFiles[nWhichWasDone].FName);
							fpPr = fopen(s, "at");
							s.Format("%s_Composites.txt", This->m_CurFiles[nWhichWasDone].FName);
							fpCmp = fopen(s, "at");
							s.Format("%s_SkippedWork.txt", This->m_CurFiles[nWhichWasDone].FName);
							fpSkip = fopen(s, "at");
							This->m_ClientData.ClientStats_End(hClient, fpPr, fpCmp, fpSkip);
							fclose(fpPr);
							fclose(fpCmp);
							fclose(fpSkip);
							bFnd = true;
							break;
						}
					}
					if (!bFnd)
						nWhichWasDone = -1;
				}
			}

            This->UpdClientsGrid();
			////////// NOW get out of our critical section.
	        This->m_ClientsGrid.LeaveCriticalSection();

            // Update the grid for this guy.
			if (bUpdateCurFileGrid)
			{
	            This->m_CurFiles[0].nWip += Requested;
				if (nWhichWasDone != -1)
				{
					This->m_CurFiles[nWhichWasDone].nWip -= WorkDones;
					This->m_CurFiles[nWhichWasDone].nWip -= Skips;
					This->m_CurFiles[nWhichWasDone].nDone += WorkDones;
					This->m_CurFiles[nWhichWasDone].nSkipped += Skips;
				}
			}

            if (bCloseWhenDone)
            {
                This->m_CurFile = 0;
                This->FileDone();
				if (This->m_nFiles && This->m_CurFiles[0].nCurOffset == 0)
				{
					This->m_CurFile = fopen(This->m_CurFiles[0].FName, "rb");
					if (This->m_CurFile)
						This->Check_First_Line();
				}
				bUpdateCurFileGrid = false;
            }
            
            if (This->m_CurFile)
                This->m_CurFiles[0].nCurOffset = ftell(This->m_CurFile);
            else
                This->m_CurFiles[0].nCurOffset = 0;
            This->UpdateCurFilesGrid();

            // Save changes to the .ini file.
            This->SaveIniState();
        }
        else
        {
	        This->m_ClientsGrid.LeaveCriticalSection();
            char *cp = new char[256];
            sprintf(cp, "Error reading data.  Return %d WSAError = %d", Len, WSAGetLastError());
            This->PostMessage(SERVER_ERROR_MESSAGE, true, (LPARAM)cp);
        }
AbortThisRead:;
        Sleep(100);
        shutdown(s, SD_SEND);
		This->m_ClientData.ClientStats_Cleanup();
		delete[] cpData;
		cpData = 0;
        for (;;)
        {
          char Buf[256];
          Len=recv(s,Buf,sizeof(Buf),0);
          if (Len==SOCKET_ERROR || Len == 0)
              break;  // all data has been read.
          // Log this error.  We should NOT have gotten any more data here across the socket.
          char *cp = new char[256];
          sprintf(cp, "in before calling closesocket, We got %d bytes of data!\n", Len);
          This->PostMessage(SERVER_ERROR_MESSAGE, true, (LPARAM)cp);
        }
        if(closesocket(s) == SOCKET_ERROR)
        {
          char *cp = new char[256];
          strcpy(cp, "Warning: could not close an http request");
          This->PostMessage(SERVER_ERROR_MESSAGE, true, (LPARAM)cp);
        }
    }
	This->MessageBox("Error calling listen on this IP/Port!");
    return;
}


void CPFGW_ServerV2Dlg::OnUseTrayIcon() 
{
	m_bTrayIcon = !m_bTrayIcon;
	if (m_bTrayIcon)
	{
		m_trayIcon.SetIcon(IDR_MAINFRAME);
		m_trayIcon.SetToolTipString("PFGW_ServerV2");
	}
	else
	{
		m_trayIcon.SetIcon(0);
	}
	SaveIniState();
}

void CPFGW_ServerV2Dlg::OnUpdateUseTrayIcon(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bTrayIcon);
}

void CPFGW_ServerV2Dlg::OnExit() 
{
	EndDialog(0);
}

void CPFGW_ServerV2Dlg::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// We don't currently have any context menus other than clicking on the first column of the grids
//	MessageBox("Got a RB");
	CDialog::OnRButtonDown(nFlags, point);
}

// NM_RCLICK
void CPFGW_ServerV2Dlg::OnClientGridRClick(NMHDR *pNotifyStruct, LRESULT* /*pResult*/)
{
    NM_GRIDVIEW* pItem = (NM_GRIDVIEW*) pNotifyStruct;
//    TRACE2(_T("Right button click on row %d, col %d\n"), pItem->iRow, pItem->iColumn);

	CPoint pt;	                                        
	::GetCursorPos(&pt); //where is the mouse?
//    TRACE2(_T("Cursor at x=%d y=%d\n"), pt.x, pt.y);

	// Find out if we SHOULD do anything with this (i.e. IS a client in this row?)
	if (pItem->iRow > 0)  // note top row is 0, a NON row is -1.
	{
 		CMenu menu; //lets display out context menu :) 
		VERIFY(menu.LoadMenu(IDR_CLIENTGRID_POPUP_MENU) );  
  		CMenu *pMenu = menu.GetSubMenu(0);
  		ASSERT(pMenu != NULL);                                       
  		UINT dwSelect = pMenu->TrackPopupMenu( (TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD), pt.x, pt.y, this);                                
 		pMenu->DestroyMenu(); 
		if (dwSelect)
			PostMessage(dwSelect,0,0);
	}
}

void CPFGW_ServerV2Dlg::OnFilesGridRClick(NMHDR *pNotifyStruct, LRESULT* /*pResult*/)
{
    NM_GRIDVIEW* pItem = (NM_GRIDVIEW*) pNotifyStruct;
//    TRACE2(_T("Right button click on row %d, col %d\n"), pItem->iRow, pItem->iColumn);

	CPoint pt;	                                        
	::GetCursorPos(&pt); //where is the mouse?
//    TRACE2(_T("Cursor at x=%d y=%d\n"), pt.x, pt.y);

	// Find out if we SHOULD do anything with this (i.e. IS there a file in this row?)
	if (pItem->iRow > 0 && m_InputFilesGrid.GetItemText(pItem->iRow, pItem->iRow) != "")  // note top row is 0, a NON row is -1.
	{
 		CMenu menu; //lets display out context menu :) 
		VERIFY(menu.LoadMenu(IDR_FILESGRID_POPUP_MENU) );  
  		CMenu *pMenu = menu.GetSubMenu(0);
  		ASSERT(pMenu != NULL);                                       
  		UINT dwSelect = pMenu->TrackPopupMenu( (TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD), pt.x, pt.y, this);                                

 		pMenu->DestroyMenu(); 
		PostMessage(dwSelect,0,0);
	}

}
