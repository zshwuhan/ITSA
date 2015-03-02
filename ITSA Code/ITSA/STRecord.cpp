////////////////////////////////////////////////////////////////////////////////
//Disclaimer:
//
//The source code is shared to the research community as copyrighted freeware 
//for research purpose only. The authors own the copyright of the source code. 
//The freeware is provided as-is with no warranty or support. We do not take 
//any responsibility for any damage, loss of income, or any problems you might 
//experience from using our software. If you have questions, you are encouraged 
//to consult the paper and the source code. If you find our software useful, 
//please cite our paper.
////////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "STRecord.h"

//************
// CSTRecord *
//************

CSTRecord::CSTRecord()
    : m_pSenConcept(NULL)
{    
}

CSTRecord::~CSTRecord()
{
    m_path.cleanup();
}

CString CSTRecord::toString() const
{
    return _T("[") + m_path.toString() + _T(":") + m_pSenConcept->m_conceptValue + _T("]");
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTRecord::addPair(CSTConcept* pLoc, CSTConcept* pTime)
{
    try {
        CSTPair* pNewPair = new CSTPair(pLoc, pTime);
        m_path.Add(pNewPair);
        return true;
    }
    catch (CMemoryException&) {
        ASSERT(false);
        return false;
    }
}

//*************
// CSTRecords *
//*************

CSTRecords::CSTRecords()
{
}

CSTRecords::~CSTRecords()
{
}

void CSTRecords::cleanup()
{
    for (int i = 0; i < GetSize(); ++i)
        delete GetAt(i);

    RemoveAll();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTRecords::addRecord(CSTRecord* pRec)
{
    try {
		Add(pRec);
        return true;
    }
    catch (CMemoryException&) {
        ASSERT(false);
        return false;
    }
}