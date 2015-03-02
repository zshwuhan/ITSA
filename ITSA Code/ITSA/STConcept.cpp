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
#include "STConcept.h"
#include "STAttrib.h"

//**************
// CSTConcepts *
//**************

CSTConcepts::CSTConcepts()
{
}

CSTConcepts::~CSTConcepts()
{
}

void CSTConcepts::cleanup()
{
    for (int i = 0; i < GetSize(); ++i)
        delete GetAt(i);

    RemoveAll();
}

//*************
// CSTConcept *
//*************

CSTConcept::CSTConcept(CSTAttrib* pAttrib)
    : m_pParentConcept(NULL), 
      m_pAttrib(pAttrib),
      m_childIdx(-1), 
      m_flattenIdx(-1), 
      m_depth(-1),
      m_timestamp(-1),
      m_bSensitive(false)
{
}

CSTConcept::~CSTConcept()
{
    m_childConcepts.cleanup();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTConcept::addChildConcept(CSTConcept* pConceptNode)
{
    try {
        pConceptNode->m_pParentConcept = this;
        pConceptNode->m_childIdx = (int) m_childConcepts.Add(pConceptNode);
        return true;
    }
    catch (CMemoryException&) {
        ASSERT(false);
        return false;
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int CSTConcept::getNumChildConcepts() const
{ 
    return (int) m_childConcepts.GetSize(); 
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
CSTConcept* CSTConcept::getChildConcept(int idx) const
{
    return m_childConcepts.GetAt(idx); 
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
CSTConcept* CSTConcept::getParentConcept() 
{ 
    return m_pParentConcept; 
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTConcept::initHierarchy(LPCTSTR conceptStr, int depth)
{
    // Parse the conceptValue and the rest of the string.
    CString restStr;
    if (!parseConceptValue(conceptStr, m_conceptValue, restStr)) {
        cerr << _T("CSTConcept: Failed to build hierarchy from ") << conceptStr << endl;
        return false;
    }

    // Convert to interger if it is a time attribute.
    if (m_pAttrib->m_attribName.CompareNoCase(RF_TIME_ATTRIB) == 0)
        m_timestamp = StrToInt(m_conceptValue);
   
    // Check whether it is sensitive.
    if (m_pAttrib->m_bSensitive) {
        int senPos = m_conceptValue.Find(RF_CONHCHY_SENSITIVE);
        m_bSensitive = (senPos != -1 && senPos == m_conceptValue.GetLength() - 1);
        if (m_bSensitive)
            m_conceptValue = m_conceptValue.Left(m_conceptValue.GetLength() - 1);
    }   

    // Depth-first build.
    m_depth = depth;
    CString firstConcept;
    while (!restStr.IsEmpty()) {
        if (!parseFirstConcept(firstConcept, restStr)) {
            cerr << _T("CSTConcept: Failed to build hierarchy from ") << restStr << endl;
            return false;
        }

        CSTConcept* pNewConcept = new CSTConcept(m_pAttrib);
        if (!pNewConcept)
            return false;
        
        if (!pNewConcept->initHierarchy(firstConcept, depth + 1)) {
            cerr << _T("CSTConcept: Failed to build hierarchy from ") << firstConcept << endl;
            return false;
        }

        if (!addChildConcept(pNewConcept))
            return false;
    }
    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// static
bool CSTConcept::parseConceptValue(LPCTSTR str, CString& conceptVal, CString& restStr)
{
    conceptVal.Empty();
    restStr.Empty();

    CString wrkStr = str;
    if (wrkStr.GetLength() < 2 ||
        wrkStr[0] !=  RF_CONHCHY_OPENTAG || 
        wrkStr[wrkStr.GetLength() - 1] !=  RF_CONHCHY_CLOSETAG) {
        ASSERT(false);
        return false;
    }

    // Extract "Canada {BC {Vancouver} {Surrey} {Richmond}} {AB {Calgary} {Edmonton}}"
    wrkStr = wrkStr.Mid(1, wrkStr.GetLength() - 2);
    CBFStrHelper::trim(wrkStr);
    if (wrkStr.IsEmpty()) {
        ASSERT(false);
        return false;
    }

    // Extract "Canada"
    int openPos = wrkStr.Find(RF_CONHCHY_OPENTAG);
    if (openPos < 0) {
        // This is root value, e.g., "Vancouver".
        conceptVal = wrkStr;
        return true;
    }
    else {
        conceptVal = wrkStr.Left(openPos);
        CBFStrHelper::trim(conceptVal);
        if (conceptVal.IsEmpty()) {
            ASSERT(false);
            return false;
        }
    }

    // Extract "{BC {Vancouver} {Surrey} {Richmond}} {AB {Calgary} {Edmonton}}"
    restStr = wrkStr.Mid(openPos);
    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// static
bool CSTConcept::parseFirstConcept(CString& firstConcept, CString& restStr)
{
    firstConcept.Empty();
    int len = restStr.GetLength();
    if (len < 2 ||
        restStr[0] !=  RF_CONHCHY_OPENTAG || 
        restStr[len - 1] !=  RF_CONHCHY_CLOSETAG) {
        ASSERT(false);
        return false;
    }

    // Find the index number of the closing tag of the first concept.
    int tagCount = 0;
    for (int i = 0; i < len; ++i) {
        if (restStr[i] == RF_CONHCHY_OPENTAG)
            ++tagCount;
        else if (restStr[i] == RF_CONHCHY_CLOSETAG) {
            --tagCount;
            ASSERT(tagCount >= 0);
            if (tagCount == 0) {
                // Closing tag of first concept found!
                firstConcept = restStr.Left(i + 1);
                restStr = restStr.Mid(i + 1);
                CBFStrHelper::trim(restStr);
                return true;
            }
        }
    }
    ASSERT(false);
    return false;
}