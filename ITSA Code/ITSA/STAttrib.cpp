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
#include "STAttrib.h"

//************
// CSTAttrib *
//************

CSTAttrib::CSTAttrib(LPCTSTR attribName)
    : m_attribName(attribName), m_pConceptRoot(NULL), m_attribIdx(-1), m_bSensitive(false)
{
}

CSTAttrib::~CSTAttrib()
{
    delete m_pConceptRoot;
    m_pConceptRoot = NULL;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTAttrib::initHierarchy(LPCTSTR conceptStr)
{
    ASSERT(!m_pConceptRoot);
    m_pConceptRoot = new CSTConcept(this);
    if (!m_pConceptRoot)
        return false;
    if (!m_pConceptRoot->initHierarchy(conceptStr, 0))
        return false;
    if (!flattenHierarchy())
        return false;
    return true;
}

//---------------------------------------------------------------------------
// Flatten the hierachy to an array of concepts in breath-first style. 
// The root is at the front, the leave are the at back.
//---------------------------------------------------------------------------
bool CSTAttrib::flattenHierarchy()
{
    m_flattenConcepts.RemoveAll();

    CPtrList attList;
    attList.AddTail(m_pConceptRoot);

    CSTConcept* pConcept = NULL;
    while (!attList.IsEmpty()) {
        pConcept = (CSTConcept*) attList.RemoveHead();
        if (!pConcept) {
            ASSERT(false);
            return false;
        }                
        pConcept->m_flattenIdx = (int) m_flattenConcepts.Add(pConcept);
        
        int nChildren = pConcept->getNumChildConcepts();
        for (int i = 0; i < nChildren; ++i) {
            attList.AddTail(pConcept->getChildConcept(i));
        }
    }
    return true;
}

//---------------------------------------------------------------------------
// Find the concept by string. Search from the back.
//---------------------------------------------------------------------------
CSTConcept* CSTAttrib::findConceptByStr(LPCTSTR str)
{
    for (int i = (int) m_flattenConcepts.GetSize() - 1; i >= 0; --i) {
        if (m_flattenConcepts[i]->m_conceptValue.CompareNoCase(str) == 0)
            return m_flattenConcepts[i];
    }
    return NULL;
}

//*************
// CSTAttribs *
//*************

CSTAttribs::CSTAttribs()
{
}

CSTAttribs::~CSTAttribs()
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CSTAttribs::cleanup()
{
    for (int i = 0; i < GetSize(); ++i)
        delete GetAt(i);

    RemoveAll();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
ostream& operator<<(ostream& os, const CSTAttribs& attribs)
{
#ifdef _DEBUG_PRT_INFO
    CSTAttrib* pAttrib = NULL;
    for (int a = 0; a < attribs.GetSize(); ++a) {
        pAttrib = attribs.GetAt(a);
        os << _T("[") << pAttrib->m_attribIdx << _T("]\t") << pAttrib->m_attribName << endl;
    }
#endif
    return os;
}