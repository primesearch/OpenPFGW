// PFGW_ServerV2.h : main header file for the PFGW_SERVERV2 application
//

#if !defined(AFX_PFGW_SERVERV2_H__71FA9A05_2B82_11D6_9411_00045A93297A__INCLUDED_)
#define AFX_PFGW_SERVERV2_H__71FA9A05_2B82_11D6_9411_00045A93297A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CPFGW_ServerV2App:
// See PFGW_ServerV2.cpp for the implementation of this class
//

class CPFGW_ServerV2App : public CWinApp
{
public:
	CPFGW_ServerV2App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPFGW_ServerV2App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPFGW_ServerV2App)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PFGW_SERVERV2_H__71FA9A05_2B82_11D6_9411_00045A93297A__INCLUDED_)
