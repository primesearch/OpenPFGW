// TextViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WinPFGW.h"
#include "TextViewerDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextViewerDlg dialog


CTextViewerDlg::CTextViewerDlg(const char *FName, CWnd* pParent /*=NULL*/)
   : CDialog(CTextViewerDlg::IDD, pParent)
{
   //{{AFX_DATA_INIT(CTextViewerDlg)
      // NOTE: the ClassWizard will add member initialization here
   //}}AFX_DATA_INIT

   m_bIsFileViewer = false;
   m_bDisplayed = false;
   m_nDlgMinX = 0;

   if (FName && *FName)
   {
      m_bIsFileViewer = true;
      strcpy(m_FName, FName);
   }
}


void CTextViewerDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CTextViewerDlg)
      // NOTE: the ClassWizard will add DDX and DDV calls here
   //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTextViewerDlg, CDialog)
   //{{AFX_MSG_MAP(CTextViewerDlg)
   ON_WM_TIMER()
   ON_WM_SIZE()
   ON_WM_GETMINMAXINFO()
   ON_BN_CLICKED(IDC_VIEW_ABCFILEFORMAT_TXT, OnViewAbcfileformatTxt)
   ON_BN_CLICKED(IDC_VIEW_AUTHORS, OnViewAuthors)
   ON_BN_CLICKED(IDC_VIEW_MANIFEST, OnViewManifest)
   ON_BN_CLICKED(IDC_VIEW_NETWORKFILE2FORMAT_TXT, OnViewNetworkfile2formatTxt)
   ON_BN_CLICKED(IDC_VIEW_NEWPGENFORMATS_TXT, OnViewNewpgenformatsTxt)
   ON_BN_CLICKED(IDC_VIEW_NEWS, OnViewNews)
   ON_BN_CLICKED(IDC_VIEW_OPENPFGW, OnViewOpenpfgw)
   ON_BN_CLICKED(IDC_VIEW_PFGWDOC_TXT, OnViewPfgwdocTxt)
   ON_BN_CLICKED(IDC_VIEW_README_PFGW, OnViewReadmePfgw)
   ON_BN_CLICKED(IDC_VIEW_RELNOTES, OnViewRelnotes)
   ON_BN_CLICKED(IDC_VIEW_SCRIPTFORMAT_TXT, OnViewScriptformatTxt)
   ON_BN_CLICKED(IDC_PRINT_HELPPAGE, OnPrintBtn)
   ON_BN_CLICKED(IDC_VIEW_LICENSCE_PFGW, OnViewLicenscePfgw)
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextViewerDlg message handlers

BOOL CTextViewerDlg::OnInitDialog()
{
   CDialog::OnInitDialog();

   CEdit *pCe = (CEdit*)GetDlgItem(IDC_TEXTVIEW_EDIT);
   if (!pCe)
      MessageBox("Error, Can not find symbol IDC_TEXT_FILEVIEW_EDIT");
   else
   {
      // We found it, but if we add the text here, we end up with ALL of the text being highlighted,
      // and we are viewing the top of the file.  So we have to allow the window to be painted first.
      // We simply set a 100ms timer, and do the actual AddString() function there.  Then it works fine.
      m_smartEdt.AttachCEdit(pCe);
      SetTimer(1,100,NULL);
   }

   CRect r;
   GetWindowRect(&r);      // Needed for the WM_GETMINMAXINFO to "limit" the sizing allowed on the dialog.
   m_nDlgMinX = r.right-r.left;
   m_nDlgMinY = r.bottom-r.top-200;

   return TRUE;
}

/*************************************************************************
*   Purpose:   This function simply makes SURE that the dialog has been
*           fully displayed before adding text to the SmartEdit object
**************************************************************************/
void CTextViewerDlg::OnTimer(UINT_PTR nIDEvent)
{
   KillTimer(nIDEvent);

   m_bDisplayed = true;
   ShowFile("README.pfgw");
   CDialog::OnTimer(nIDEvent);
   CButton *b = (CButton *)GetDlgItem(IDC_VIEW_README_PFGW);
   b->SetCheck(1);
}

void CTextViewerDlg::OnSize(UINT nType, int cx, int cy)
{
   CDialog::OnSize(nType, cx, cy);
   if (m_bDisplayed)
   {
      // WARNING, the initial draw calls this and we do NOT want to resize there.  We set a value
      // within the timer proc (when we know the window is visible), and from that point on, we
      // resize the edit field.
      CEdit *pCe = (CEdit*)GetDlgItem(IDC_TEXTVIEW_EDIT);
      if (pCe)
         pCe->SetWindowPos(NULL, 0, 0, cx-20, cy-108, SWP_NOZORDER|SWP_NOMOVE);
   }
}

afx_msg void CTextViewerDlg::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
   if (m_nDlgMinX)
   {
      // Set the min size
      lpMMI->ptMinTrackSize.x = m_nDlgMinX;
      lpMMI->ptMinTrackSize.y = m_nDlgMinY;
   }
}

void CTextViewerDlg::ShowFile(const char *FName)
{
   // read last 50000 bytes of the file (or the whole file if smaller than 50000)
   // NOTE the file needs to be in MSDOS \r\n format for SmartEdit to work right.
   char Buf[50001];
   // we MUST open in binary mode, so that no translation of \r\n to \n happens.  NOTE that
   // if a UNIX \n file is viewed, it will not view right :(  Sorry.
   FILE *in = fopen(FName, "rb");
   strcpy(m_FileName, FName);
   m_smartEdt.ClearAll();
   if (in)
   {
      // read and display the last 50k of the file (or the whole file if less than 50k)
      fseek(in, -50000, SEEK_END);
      int Cnt = (int) fread(Buf, 1, 50000, in);
      Buf[Cnt] = 0;
      m_smartEdt.AddString(Buf);
      fclose(in);
   }
   else
      m_smartEdt.AddString("   !!! File not found !!!");
   sprintf (Buf, "Help/Info File Viewer [%s]", FName);
   SetWindowText(Buf);

   m_smartEdt.ShowTopLine();
}

void CTextViewerDlg::OnViewAbcfileformatTxt()
{
   ShowFile("abcfileformats.txt");
}

void CTextViewerDlg::OnViewAuthors()
{
   ShowFile("authors");
}

void CTextViewerDlg::OnViewManifest()
{
   ShowFile("manifest");
}

void CTextViewerDlg::OnViewNetworkfile2formatTxt()
{
   ShowFile("network2format.txt");
}

void CTextViewerDlg::OnViewNewpgenformatsTxt()
{
   ShowFile("newpgenformats.txt");
}

void CTextViewerDlg::OnViewNews()
{
   ShowFile("news");
}

void CTextViewerDlg::OnViewOpenpfgw()
{
   ShowFile("openpfgw");
}

void CTextViewerDlg::OnViewPfgwdocTxt()
{
   ShowFile("pfgwdoc.txt");
}

void CTextViewerDlg::OnViewReadmePfgw()
{
   ShowFile("readme.pfgw");
}

void CTextViewerDlg::OnViewLicenscePfgw()
{
   ShowFile("license.pfgw");
}

void CTextViewerDlg::OnViewRelnotes()
{
   ShowFile("relnotes");
}

void CTextViewerDlg::OnViewScriptformatTxt()
{
   ShowFile("scriptfileformat.txt");
}

void CTextViewerDlg::OnPrintBtn()
{
   char *pp, Buf[512];
   *Buf = 0;
   SearchPath(NULL, "notepad", ".exe", sizeof(Buf), Buf, &pp);
   if (*Buf)
   {
      strcat(Buf, " /p ");
      strcat(Buf, m_FileName);
      WinExec(Buf, SW_HIDE);
   }
}
