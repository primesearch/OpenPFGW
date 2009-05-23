// PFGW_ServerV2Dlg.h : header file
//

#include "SmartEditField.h"
#include "trayicon.h"
#include "FileStruct.h"
#include "ClientData.h"
#include "ClientGrid.h"


#if !defined(AFX_PFGW_SERVERV2DLG_H__71FA9A07_2B82_11D6_9411_00045A93297A__INCLUDED_)
#define AFX_PFGW_SERVERV2DLG_H__71FA9A07_2B82_11D6_9411_00045A93297A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CPFGW_ServerV2Dlg dialog

class CPFGW_ServerV2Dlg : public CDialog
{
// Construction
public:
	CPFGW_ServerV2Dlg(CWnd* pParent = NULL);	// standard constructor
	~CPFGW_ServerV2Dlg();

// Dialog Data
	//{{AFX_DATA(CPFGW_ServerV2Dlg)
	enum { IDD = IDD_PFGW_SERVERV2_DIALOG };
	CComboBox	m_TextMessages;
	CEdit	m_AddFileEdt;
	CButton	m_AddFileBtn;
	CClientGrid	m_InputFilesGrid;
	CClientGrid	m_ClientsGrid;
	CString	m_AddFileString;
	UINT	m_nLinesListedForFile;
	CString	m_sNumClients;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPFGW_ServerV2Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	CTrayIcon	m_trayIcon;
	uint32 m_nFiles;
	bool m_bTrayIcon;
	bool m_bMinimized;
	uint32 m_nIconPulseCount;
	bool m_bIconErrorState;

	CurFiles_t m_CurFiles[6];		// 4 are used actually, but we treat this as a 1 based array, so that it matches the flexgrid,  and keep a "blank" trailing entry

	// These 2 vars allow us to send NON-"straight" files.  Now ABC, ABC2 and NewPGen can be processed.
	// NOTE ABCD is not yet valid, but might not be too hard to handle.
	char m_FirstLine[4096];
	bool m_bIsFirstLineFile;
	void Check_First_Line();			// fills in m_bIsFirstLineFile and m_FirstLine[]

    CClientData m_ClientData;

	void UpdClientsGrid();

	private:
		bool http_init();
		void http_free();
		void http_close();
		SOCKET http_open();

		SOCKET m_sMain;
		HANDLE m_ThreadHandle;
		bool   m_bDieServer;
		bool   m_bServerDead;
		FILE   *m_CurFile, *m_fpOutPrFile, *m_fpOutCompFile, *m_fpOutSkipFile;

		static void ListenThread(void *);
		void FileDone();
		void UpdateCurFilesGrid();
		void SetupGridTitles();
		void SaveIniState();
		void LoadIniState();
		void OpenOutputFiles();
		void FlashIcon();

	static CGridCtrl *m_pClientsGrid;
	static int CALLBACK Client_Grid_CellCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParam3);


	// Generated message map functions
	//{{AFX_MSG(CPFGW_ServerV2Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAddFile();
	afx_msg void OnClose();
	afx_msg void OnUseTrayIcon();
	afx_msg void OnUpdateUseTrayIcon(CCmdUI* pCmdUI);
	afx_msg void OnExit();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClearIconError();
	afx_msg void OnUpdateClearIconError(CCmdUI* pCmdUI);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnServerErrorMessage(WPARAM uID, LPARAM lEvent);
    afx_msg LRESULT OnAddClientMessage(WPARAM uID, LPARAM lEvent);
	afx_msg LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);
	afx_msg LRESULT OnPFGW_Message(WPARAM uID, LPARAM lEvent);


	// The damn dialog box does NOT handle menu popup's and ON_UPDATE_COMMAND_UI by default.
	// See MSDN article ID: Q242577 for the code to fix this "by design" BUG
	afx_msg void OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu);

	// Notifications from the GRID
	afx_msg void OnClientGridRClick(NMHDR *pNotifyStruct, LRESULT* /*pResult*/);
	afx_msg void OnFilesGridRClick(NMHDR *pNotifyStruct, LRESULT* /*pResult*/);

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PFGW_SERVERV2DLG_H__71FA9A07_2B82_11D6_9411_00045A93297A__INCLUDED_)
