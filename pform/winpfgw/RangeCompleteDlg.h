#if !defined(AFX_RANGECOMPLETEDLG_H__8DA05853_6740_4316_972A_72F4D6367C86__INCLUDED_)
#define AFX_RANGECOMPLETEDLG_H__8DA05853_6740_4316_972A_72F4D6367C86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RangeCompleteDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRangeCompleteDlg dialog

class CRangeCompleteDlg : public CDialog
{
// Construction
public:
	CRangeCompleteDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRangeCompleteDlg)
	enum { IDD = IDD_RANGE_COMPLETE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRangeCompleteDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRangeCompleteDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RANGECOMPLETEDLG_H__8DA05853_6740_4316_972A_72F4D6367C86__INCLUDED_)
