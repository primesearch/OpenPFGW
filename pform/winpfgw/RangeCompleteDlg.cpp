// RangeCompleteDlg.cpp : implementation file
//

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "stdafx.h"
#include "WinPFGW.h"
#include "RangeCompleteDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRangeCompleteDlg dialog


CRangeCompleteDlg::CRangeCompleteDlg(CWnd* pParent /*=NULL*/)
   : CDialog(CRangeCompleteDlg::IDD, pParent)
{
   //{{AFX_DATA_INIT(CRangeCompleteDlg)
      // NOTE: the ClassWizard will add member initialization here
   //}}AFX_DATA_INIT
}


void CRangeCompleteDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CRangeCompleteDlg)
      // NOTE: the ClassWizard will add DDX and DDV calls here
   //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRangeCompleteDlg, CDialog)
   //{{AFX_MSG_MAP(CRangeCompleteDlg)
      // NOTE: the ClassWizard will add message map macros here
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRangeCompleteDlg message handlers
