// WinPFGWDlg.h : header file
//

#if !defined(AFX_WINPFGWDLG_H__1DC3A429_B704_11D4_940F_00105AA797AB__INCLUDED_)
#define AFX_WINPFGWDLG_H__1DC3A429_B704_11D4_940F_00105AA797AB__INCLUDED_

#include "SmartEditField.h"
#include "trayicon.h"

class PFIni;

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CWinPFGWDlg dialog

class CWinPFGWDlg : public CDialog
{
// Construction
public:
	CWinPFGWDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CWinPFGWDlg)
	enum { IDD = IDD_WINPFGW_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinPFGWDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	DWORD m_dwPriorityClass;
	int m_nThreadPriority;

// Implementation
protected:

	HICON		m_hIcon;
	CSmartEditField m_SmartEdit;
	CTrayIcon	m_trayIcon;
	bool		m_bTrayIcon;
	bool		m_bStealth;
	bool		m_bHideOnStart;
	bool		m_bDONTSAVEOPTIONS;
	enum		{eQuiet=0, eGFFactors=1, eNormal=2, eVerbose=3} m_ScreenMode;  //bool		m_bVerbose;
	bool		m_bVerboseFile;
	bool		m_bDontShowFileVerboseMessage;
	bool		m_bRunning;
	bool		m_bMinimized;
	bool		m_bFactor;
	bool		m_bFFactor;
	int			m_nMode;
	int			m_nPRPBase;
	int			m_nPriority;
	HANDLE		m_ThreadHandle;
	// For FLASHING icon
	bool		m_bIconIsRed;
	bool		m_bIconFlashing;
	bool		m_bExitedByRequest;
	HICON		m_Red, m_Green, m_Yellow;

	enum IconColor {IconRed, IconGreen, IconYellow};


	// Resizing crap
	int m_nDlgMinX, m_nDlgMinY, m_nDlgClientMinY, m_nDlgClientMinX;
	int m_nEditX, m_nEditY;

	void LoadOptions(PFIni*);
	void SaveOptions(PFIni*);
	void ChangeIcon(IconColor, const char *Msg=0);


	// Generated message map functions
	//{{AFX_MSG(CWinPFGWDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnStart();
	afx_msg LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);
	afx_msg void OnAboutbox();
	afx_msg LRESULT OnWinPFGW_Message(WPARAM, LPARAM);
	afx_msg void OnDestroy();
	afx_msg void OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI );
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnAboutRecords();
	afx_msg void OnAboutContents();
	afx_msg void OnUseTrayIcon();
	afx_msg void OnUpdateUseTrayIcon(CCmdUI* pCmdUI);
	afx_msg void OnIdle();
	afx_msg void OnUpdateIdle(CCmdUI* pCmdUI);
	afx_msg void OnLow();
	afx_msg void OnUpdateLow(CCmdUI* pCmdUI);
	afx_msg void OnNormal();
	afx_msg void OnUpdateNormal(CCmdUI* pCmdUI);
	afx_msg void OnBelowNormal();
	afx_msg void OnUpdateBelowNormal(CCmdUI* pCmdUI);
	afx_msg void OnVerboseScreen();
	afx_msg void OnUpdateVerboseScreen(CCmdUI* pCmdUI);
	afx_msg void OnVerboseFile();
	afx_msg void OnUpdateVerboseFile(CCmdUI* pCmdUI);
	afx_msg void OnPrpProofchoice();
	afx_msg void OnUpdatePrpProofchoice(CCmdUI* pCmdUI);
	afx_msg void OnNm1PROOFCHOICE();
	afx_msg void OnUpdateNm1PROOFCHOICE(CCmdUI* pCmdUI);
	afx_msg void OnNp1PROOFCHOICE();
	afx_msg void OnUpdateNp1PROOFCHOICE(CCmdUI* pCmdUI);
	afx_msg void OnFactorizeOnly();
	afx_msg void OnUpdateFactorizeOnly(CCmdUI* pCmdUI);
	afx_msg void OnDecimalExpansion();
	afx_msg void OnUpdateDecimalExpansion(CCmdUI* pCmdUI);
	afx_msg void OnSetPrpBase();
	afx_msg void OnUpdateSetPrpBase(CCmdUI* pCmdUI);
	afx_msg void OnPp1Np1PROOFCHOICE();
	afx_msg void OnUpdatePp1Np1PROOFCHOICE(CCmdUI* pCmdUI);
	afx_msg void OnFactorize();
	afx_msg void OnUpdateFactorize(CCmdUI* pCmdUI);
	afx_msg void OnFermatFactorOnly();
	afx_msg void OnUpdateFermatFactorOnly(CCmdUI* pCmdUI);
	afx_msg void OnFermatFactor();
	afx_msg void OnUpdateFermatFactor(CCmdUI* pCmdUI);
	afx_msg void OnClearHighlight();
	afx_msg void OnClearAll();
	afx_msg void OnHideTheApp();
	afx_msg void OnUpdateHideTheApp(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNormalScreen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQuietScreen(CCmdUI* pCmdUI);
	afx_msg void OnNormalScreen();
	afx_msg void OnQuietScreen();
	afx_msg void OnGfFactOnlyScreen();
	afx_msg void OnUpdateGfFactOnlyScreen(CCmdUI* pCmdUI);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG

	// The damn dialog box does NOT handle menu popup's and ON_UPDATE_COMMAND_UI by default.
	// See MSDN article ID: Q242577 for the code to fix this "by design" BUG
	afx_msg void OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu);
	afx_msg void OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized );

	// To TOTALLY hide the window when starting up (if in hide mode)
	afx_msg void OnWindowPosChanging(WINDOWPOS *lpwndpos);

	DECLARE_MESSAGE_MAP()

	private:
		static void ThreadProc(void *);
public:
   afx_msg void OnStnClickedLineInFile();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINPFGWDLG_H__1DC3A429_B704_11D4_940F_00105AA797AB__INCLUDED_)
