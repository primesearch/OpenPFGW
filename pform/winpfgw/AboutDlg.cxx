// AboutDlg.cpp : implementation file
//

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
	virtual BOOL OnInitDialog();
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// Woltman v22 stuff.
void getCpuInfo();
void getCpuDescription (char	*buf, int bufferSize);

#include "..\..\pform\pfgw\pfgw_version.h"

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CString S;
	S.LoadString(IDS_WINPFGW_VERSION_STRING);
	GetDlgItem(IDC_STATIC_WINPFGW_VERSION)->SetWindowText(S);

	S.Format("PFGW Version %s [GWNUM %s]", WINPFGW_ABOUT_VERSION_STRING, GWNUM_VERSION);
	GetDlgItem(IDC_STATIC_PFGW_VERSION)->SetWindowText(S);

	getCpuInfo();
	char Buf[1024], Buf2[1024];
	getCpuDescription(Buf, sizeof(Buf));

	char *cp = strtok(Buf, "\r\n");
	char *cp1 = Buf2;
	while (cp)
	{
		cp1 += sprintf(cp1, "%s\r\n", cp);
		cp = strtok(NULL, "\r\n");
	}
	--cp1;
	while (*cp1 == '\r' || *cp1 == '\n')
		*cp1-- = 0;

	GetDlgItem(IDC_STATIC_WOLTMAN_VERSION)->SetWindowText(Buf2);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
