// SmartEditField.cpp: implementation of the CSmartEditField class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WinPFGW.h"
#include "SmartEditField.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSmartEditField::CSmartEditField()
{
	m_pEdit = 0;
}

CSmartEditField::~CSmartEditField()
{
}

int CSmartEditField::AddString(char *cpp, bool bDelete)
{
	int Len = strlen(cpp);
	char *cp = new char [Len+20];
	memset(cp, 0, Len+20);
	strcpy(cp, cpp);
	if (bDelete)
		delete[] cpp;
	cpp = cp;
	char *cpEnd = &cp[strlen(cp)-1];
	int Eol = 0;
	while (*cpEnd == '\n')
	{
		++Eol;
		*cpEnd-- = 0;
	}
	static bool bLastLF=false; 
	while (*cpEnd == '\r')
	{
		bLastLF = true;
		*cpEnd-- = 0;
	}

	int OrigLen = m_pEdit->GetWindowTextLength();
	while (*cp == '\n')
	{
		m_pEdit->SetSel(OrigLen, OrigLen, TRUE);
		m_pEdit->ReplaceSel("\r\n",2);
		OrigLen += 2;
		cp++;
		bLastLF = false;
	}
	if (OrigLen+strlen(cp) > 50000)
	{
		int LastChar = 10000+strlen(cp);
		if (LastChar > OrigLen)
			LastChar = OrigLen;
		m_pEdit->SetSel(0, LastChar, TRUE);
		m_pEdit->SetReadOnly(FALSE);
		m_pEdit->Clear();
		m_pEdit->SetReadOnly(TRUE);
		OrigLen = m_pEdit->GetWindowTextLength();
	}
	int SelStartLen = OrigLen;
	if (*cp == '\r' || bLastLF)
	{
		SelStartLen = m_pEdit->LineIndex(m_pEdit->GetLineCount()-1);
		while (*cp == '\r')
			cp++;
	}

	static char *CRLF = "\r\n";
	while (Eol--)
		strcat(cp, CRLF);

	m_pEdit->SetSel(SelStartLen, OrigLen, TRUE);
	m_pEdit->ReplaceSel(cp,strlen(cp));
	m_pEdit->EmptyUndoBuffer();

	delete[] cpp;
	return 0;
}

int CSmartEditField::AttachCEdit(CEdit *pCe)
{
	m_pEdit = pCe;
	// Get as much as possible out of the edit control.  We can get over 50k by doing this.  Unfortunately, we do
	// not know "when" we will run out of edit space (more than just the raw text take up the 64k of localheap),
	// so we could trap the EN_MAXTEXT message to "know" that we have overflowed, or we can simply check to see
	// if we are over 50k, and if so, remove the first 10k of data.  The simpler second method is what we will do.
	pCe->LimitText(65000);		
	return 0;
}

void CSmartEditField::ClearHighlight()
{
	m_pEdit->SetReadOnly(FALSE);
	m_pEdit->Clear();
	m_pEdit->SetReadOnly(TRUE);
}

void CSmartEditField::ClearAll()
{
	int Len = m_pEdit->GetWindowTextLength();
	m_pEdit->SetSel(0, Len, TRUE);
	m_pEdit->SetReadOnly(FALSE);
	m_pEdit->Clear();
	m_pEdit->SetReadOnly(TRUE);
}

void CSmartEditField::ShowTopLine()
{
	m_pEdit->SetSel(0, 1, FALSE);
	m_pEdit->SetSel(-1,-1,FALSE);
}
