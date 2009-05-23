// PFGW_ServerV2.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "PFGW_ServerV2.h"
#include "PFGW_ServerV2Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPFGW_ServerV2App

BEGIN_MESSAGE_MAP(CPFGW_ServerV2App, CWinApp)
	//{{AFX_MSG_MAP(CPFGW_ServerV2App)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPFGW_ServerV2App construction

CPFGW_ServerV2App::CPFGW_ServerV2App()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPFGW_ServerV2App object

CPFGW_ServerV2App theApp;

/////////////////////////////////////////////////////////////////////////////
// CPFGW_ServerV2App initialization

BOOL CPFGW_ServerV2App::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// First things first.  Check for Msflxgrd.ocx being installed.
	// if it is NOT, then install it now.
	HKEY key;
	if (RegOpenKey(HKEY_CLASSES_ROOT, "TypeLib\\{5E9E78A0-531B-11CF-91F6-C2863C385E30}", &key) != ERROR_SUCCESS)
	{
		char Path[512], Path1[512], *FName;
		if (SearchPath(NULL, "regsvr32.exe", ".exe", sizeof(Path), Path, &FName) != 0 && SearchPath(NULL, "msflxgrd.ocx", ".ocx", sizeof(Path1), Path1, &FName))
		{
			strcat(Path, " /s ");	// silent mode regsrv32.exe
			strcat(Path, Path1);
			WinExec(Path, SW_HIDE);
		}
		else
		{
			MessageBox(0, "Error, the OCX control MsFlxgrd.ocx is not installed,\r\nand I can not install it automatically.\r\nI need regsvr32.exe and MsFlxgrd.ocx to be\r\nin the search path, and they are not.\r\nThese files are located in the pform\\PFGW_ServerV2\r\nfolder of the PFGW source tree.  Get them now", "Error, control not installed", 0);
			return FALSE;
		}
	}
	else
		RegCloseKey(key);

	// Check this out!! It may allow a hidden window display by default. It does so for MDI and SDI at least.
	m_nCmdShow = SW_MINIMIZE;

	CPFGW_ServerV2Dlg dlg;
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
