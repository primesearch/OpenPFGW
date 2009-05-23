// Client grid.  Overrides CGridCtrl so that thread safety can be achieved, and for easier sorting
// Thread safety is needed for the sorting, and for the updating of cells by a different thread.

#include "GridCtrl.h"

class CClientGrid : public CGridCtrl
{
	public:
		CClientGrid(int nRows = 0, int nCols = 0, int nFixedRows = 0, int nFixedCols = 0);
		~CClientGrid();

        void EnterCriticalSection() {::EnterCriticalSection(&m_ClientCritical);}
        void LeaveCriticalSection() {::LeaveCriticalSection(&m_ClientCritical);}

	protected:
	    // Mouse Clicks
		virtual void  OnFixedRowClick(CCellID& cell);

        CRITICAL_SECTION m_ClientCritical;
};
