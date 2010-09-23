// MessageDialog.cpp : implementation file
//

#include "stdafx.h"
#include "WinPFGW.h"
#include "MessageDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CMessageDialog dialog


CMessageDialog::CMessageDialog(CWnd* pParent /*=NULL*/)
   : CDialog(CMessageDialog::IDD, pParent)
{
   //{{AFX_DATA_INIT(CMessageDialog)
      // NOTE: the ClassWizard will add member initialization here
   //}}AFX_DATA_INIT
   m_bNeverShowAgain = false;
}


void CMessageDialog::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CMessageDialog)
      // NOTE: the ClassWizard will add DDX and DDV calls here
   //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessageDialog, CDialog)
   //{{AFX_MSG_MAP(CMessageDialog)
   ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageDialog message handlers

void CMessageDialog::SetMessageText(CString Msg)
{
   m_Msg = Msg;
}

bool CMessageDialog::NeverShowThisMessageAgain()
{
   return m_bNeverShowAgain;
}

BOOL CMessageDialog::OnInitDialog()
{
   CDialog::OnInitDialog();

   if (m_Msg == "")
   {
      MessageBox("Programming Error, you must call SetMessageText before the DoModal for a CMessageDialog");
      EndDialog(IDCANCEL);
      return FALSE;
   }
   CWnd *w = GetDlgItem(IDC_MESSAGE_TEXT);
   if (w)
      w->SetWindowText(m_Msg);

   return TRUE;
}

void CMessageDialog::OnCheck1()
{
   m_bNeverShowAgain = !m_bNeverShowAgain;
}
