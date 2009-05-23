#include "pflibpch.h"
#include "pfsymboltable.h"

PFSymbolTable::PFSymbolTable(PFSymbolTable *pParent)
	: PFList<IPFSymbol>(PFBoolean::b_true), m_pParent(pParent)
{
}

PFSymbolTable::PFSymbolTable(const PFSymbolTable &st)
	: PFList<IPFSymbol>(PFBoolean::b_true), m_pParent(st.m_pParent)
{
}

PFSymbolTable &PFSymbolTable::operator=(const PFSymbolTable &st)
{
        // allow x=x
        if (this == &st)
                return *this;
	RemoveAll();
	m_pParent=st.m_pParent;
	return *this;
}

PFSymbolTable::~PFSymbolTable()
{
}

PFListNode *PFSymbolTable::_LookupSymbolNode(const PFString &sK)
{
	PFListNode *pRetval=NULL;
	// just run through with a list iterator for now
	PFForwardIterator pffi;
	StartIterator(pffi);
	PFListNode *pNode;

	while(pffi.Iterate(pNode))
	{
		IPFSymbol *pSymbol=(IPFSymbol*)pNode->GetData();
		if(pSymbol->GetKey()==sK)
		{
			pRetval=pNode;
			break;
		}
	}	
	
	return pRetval;
}
	
IPFSymbol *PFSymbolTable::_LookupSymbol(const PFString &sK)
{
	IPFSymbol *pRetval=NULL;
	PFListNode *pNode=_LookupSymbolNode(sK);
	
	if(pNode)
	{
		pRetval=(IPFSymbol*)pNode->GetData();
	}
	return pRetval;
}

IPFSymbol *PFSymbolTable::RemoveSymbol(const PFString &sKey)
{
	PFString sK(sKey);
	sK.ToUpper();
	IPFSymbol *pRetval=NULL;
	
	PFListNode *pNode=_LookupSymbolNode(sK);

	if(pNode)
	{
		pRetval=(IPFSymbol*)pNode->GetData();
		pNode->Remove();
		delete pNode;
	}
	
	return pRetval;
}

void PFSymbolTable::EraseSymbol(const PFString &sKey)
{
	IPFSymbol *pRetval=RemoveSymbol(sKey);
	if(pRetval)
	{
		delete pRetval;
	}
}

IPFSymbol *PFSymbolTable::LookupSymbol(const PFString &sKey)
{
	PFString sK(sKey);
	sK.ToUpper();
	
	IPFSymbol *pRetval=_LookupSymbol(sK);
	
	if((pRetval==NULL)&&(m_pParent!=NULL))
	{
		pRetval=m_pParent->LookupSymbol(sK);
	}
	return pRetval;
}

PFBoolean PFSymbolTable::AddSymbol(IPFSymbol *pSymbol)
{
	PFBoolean bNewSymbol=PFBoolean::b_true;
	
	PFString sK=pSymbol->GetKey();
	PFListNode *pNode=_LookupSymbolNode(sK);
	if(pNode)
	{
		IPFSymbol *pObject=(IPFSymbol *)pNode->GetData();
		pNode->Remove();
		delete pNode;
		delete pObject;
		bNewSymbol=PFBoolean::b_false;
	}
	AddTail(pSymbol);

	return(bNewSymbol);
}

#ifdef _DEBUG
#include "pfio.h"

void PFSymbolTable::ListSymbols() {
	// just run through with a list iterator for now
	PFForwardIterator pffi;
	StartIterator(pffi);
	PFListNode *pNode;

	while(pffi.Iterate(pNode))
	{
		IPFSymbol *pSymbol=(IPFSymbol*)pNode->GetData();
		PFOutput::EnableOneLineForceScreenOutput();
		PFPrintfStderr("Reserved symbol %s\n",LPCTSTR(pSymbol->GetKey()));
	}
}
#endif
