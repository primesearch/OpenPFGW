#include "stdafx.h"
#include "ClientGrid.h"

CClientGrid::CClientGrid(int nRows, int nCols, int nFixedRows, int nFixedCols) : 
	CGridCtrl(nRows, nCols, nFixedRows, nFixedCols)
{
	InitializeCriticalSection(&m_ClientCritical);

	// Setup our "defaults"
	SetEditable(FALSE);
	SetRowResize(FALSE);
	EnableSelection(FALSE);
}

CClientGrid::~CClientGrid()
{
	DeleteCriticalSection(&m_ClientCritical);
}

// Mouse Clicks  Override the "default" for FixedRowClick, so that we can enter (and leave)
// our critical section.
/*virtual*/ void  CClientGrid::OnFixedRowClick(CCellID& cell)
{
    if (!IsValid(cell))
        return;

    if (GetHeaderSort())
    {
		EnterCriticalSection();
        CWaitCursor waiter;
        if (cell.col == GetSortColumn())
            SortItems(cell.col, !GetSortAscending());
        else
            SortItems(cell.col, TRUE);
        Invalidate();
		LeaveCriticalSection();
    }
}
