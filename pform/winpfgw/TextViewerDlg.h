#if !defined(AFX_TEXTVIEWERDLG_H__6E34D001_4C5E_11D6_9412_00045A93297A__INCLUDED_)
#define AFX_TEXTVIEWERDLG_H__6E34D001_4C5E_11D6_9412_00045A93297A__INCLUDED_

#include "SmartEditField.h"
#include "resource.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextViewerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextViewerDlg dialog

class CTextViewerDlg : public CDialog
{
// Construction
public:
	CTextViewerDlg(const char *FName=NULL, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTextViewerDlg)
	enum { IDD = IDD_TEXTVIEWER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextViewerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	char m_FName[256];
	CSmartEditField m_smartEdt;
	bool m_bIsFileViewer;
	bool m_bDisplayed;

	int m_nDlgMinX;
	int m_nDlgMinY;
	char m_FileName[256];

	void ShowFile(const char *FName);

	// Generated message map functions
	//{{AFX_MSG(CTextViewerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI );
	afx_msg void OnViewAbcfileformatTxt();
	afx_msg void OnViewAuthors();
	afx_msg void OnViewManifest();
	afx_msg void OnViewNetworkfile2formatTxt();
	afx_msg void OnViewNewpgenformatsTxt();
	afx_msg void OnViewNews();
	afx_msg void OnViewOpenpfgw();
	afx_msg void OnViewPfgwdocTxt();
	afx_msg void OnViewReadmePfgw();
	afx_msg void OnViewRelnotes();
	afx_msg void OnViewScriptformatTxt();
	afx_msg void OnPrintBtn();
	afx_msg void OnViewLicenscePfgw();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTVIEWERDLG_H__6E34D001_4C5E_11D6_9412_00045A93297A__INCLUDED_)
