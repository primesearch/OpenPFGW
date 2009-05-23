#if !defined(AFX_MESSAGEDIALOG_H__250FFB01_54D1_11D5_9CBE_0050DA21B87A__INCLUDED_)
#define AFX_MESSAGEDIALOG_H__250FFB01_54D1_11D5_9CBE_0050DA21B87A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MessageDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMessageDialog dialog

class CMessageDialog : public CDialog
{
// Construction
public:
	CMessageDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMessageDialog)
	enum { IDD = IDD_MESSAGE_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	void SetMessageText(CString Msg);
	bool NeverShowThisMessageAgain();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMessageDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMessageDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnCheck1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CString m_Msg;
	bool m_bNeverShowAgain;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESSAGEDIALOG_H__250FFB01_54D1_11D5_9CBE_0050DA21B87A__INCLUDED_)
