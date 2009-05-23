// SmartEditField.h: interface for the CSmartEditField class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMARTEDITFIELD_H__F5D3D4C3_42D7_11D5_9410_00105AA797AB__INCLUDED_)
#define AFX_SMARTEDITFIELD_H__F5D3D4C3_42D7_11D5_9410_00105AA797AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSmartEditField  
{
public:
	int AddString(char *cp, bool bDelete=true);
	CSmartEditField();
	~CSmartEditField();
	int AttachCEdit(CEdit *pCe);
	void ClearHighlight();
	void ClearAll();
	void ShowTopLine();

protected:
	CEdit *m_pEdit;
};

#endif // !defined(AFX_SMARTEDITFIELD_H__F5D3D4C3_42D7_11D5_9410_00105AA797AB__INCLUDED_)
