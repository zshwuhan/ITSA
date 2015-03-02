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
#include "STPair.h"

//**********
// CSTPair *
//**********

CSTPair::CSTPair(CSTConcept* pLoc, CSTConcept* pTime)
    : m_pLoc(pLoc), m_pTime(pTime), m_cpyPairCount(0), m_currentPos(NULL)
{
}

CSTPair::CSTPair(CSTConcept* pLoc, CSTConcept* pTime, POSITION pos)
    : m_pLoc(pLoc), m_pTime(pTime), m_cpyPairCount(0), m_currentPos(pos)
{
}

CSTPair::CSTPair(CSTConcept* pLoc, CSTConcept* pTime, const CPosArray& posArray)
    : m_pLoc(pLoc), m_pTime(pTime), m_cpyPairCount(0)
{
	m_allPositions.Copy(posArray);
}

CSTPair::CSTPair(CSTConcept* pLoc, CSTConcept* pTime, POSITION pos, int pairCount, const CRFIntArray& senCounts)
    : m_pLoc(pLoc), m_pTime(pTime), m_cpyPairCount(pairCount), m_currentPos(pos)
{
	m_allPositions.SetSize(0);
	m_cpySenCounts.Copy(senCounts);
}

CSTPair::CSTPair(CSTConcept* pLoc, CSTConcept* pTime, POSITION pos, const CPosArray& posArray, int pairCount, const CRFIntArray& senCounts)
    : m_pLoc(pLoc), m_pTime(pTime), m_cpyPairCount(pairCount), m_currentPos(pos)
{
	m_allPositions.Copy(posArray);
	m_cpySenCounts.Copy(senCounts);
}

CSTPair::CSTPair(CSTConcept* pLoc, CSTConcept* pTime, const CPosArray& posArray, int pairCount, const CRFIntArray& senCounts)
    : m_pLoc(pLoc), m_pTime(pTime), m_cpyPairCount(pairCount), m_currentPos(0)
{
	m_allPositions.Copy(posArray);
	m_cpySenCounts.Copy(senCounts); 
}

CSTPair::~CSTPair()
{
}

CString CSTPair::toString() const
{
    return m_pLoc->m_conceptValue + RF_PATHDATA_LOCTIME_SEPARATOR + RF_PATHDATA_TIME_DELIMETER + m_pTime->m_conceptValue;
}

//---------------------------------------------------------------------------
// Check whether target is equal to this pair.
//---------------------------------------------------------------------------
bool CSTPair::isEqual(const CSTPair& target) const
{
	return m_pTime == target.m_pTime && m_pLoc == target.m_pLoc;
}

//---------------------------------------------------------------------------
// Break "Concordia23" to "Concordia" and "23"
//      OR
// Break "L1.T23" to "L1" and "23"
//---------------------------------------------------------------------------
// static 
bool CSTPair::parsePair(const CString& pairStr, CString& locStr, CString& timeStr)
{
    locStr.Empty();
    timeStr.Empty();
    int dotPos = pairStr.Find(RF_PATHDATA_LOCTIME_SEPARATOR);
    if (dotPos == -1) {
        // Break "Concordia23" to "Concordia" and "23"
        int i = 0;
        int len = pairStr.GetLength();
        for (i = len - 1; i >= 0; --i) {
            if (!IsAnsiDigit(pairStr[i]))
                break;

            timeStr.Insert(0, pairStr[i]);
        }
        locStr = pairStr.Left(i + 1);
    }
    else {
        // Break "L1.T23" to "L1" and "23"
        int len = pairStr.GetLength();
        for (int i = len - 1; i >= 0; --i) {
            if (!IsAnsiDigit(pairStr[i]))
                break;

            timeStr.Insert(0, pairStr[i]);
        }
        locStr = pairStr.Left(dotPos);
    }
    return true;
}

//***********
// CSTPairs *
//***********

CSTPairs::CSTPairs()
{
}

CSTPairs::~CSTPairs()
{
}

void CSTPairs::cleanup()
{
    for (int i = 0; i < GetSize(); ++i)
        delete GetAt(i);

    RemoveAll(); 
}

CString CSTPairs::toString() const
{
    CString res;
    for (int i = 0; i < GetSize(); ++i) {
        res += GetAt(i)->toString();
        if (i < GetSize() - 1)
            res += _T(",");
    }
    return res;
}

//---------------------------------------------------------------------------
// Make copy of this array of pairs.
//---------------------------------------------------------------------------
CSTPairs* CSTPairs::makeCopy() const
{
    CSTPair* pNewPair = NULL;
    CSTPairs* pCopy = new CSTPairs();
    for (int i = 0; i < GetSize(); ++i) {
        pNewPair = new CSTPair(GetAt(i)->m_pLoc, GetAt(i)->m_pTime);
        pCopy->Add(pNewPair);
    }
    return pCopy;
}

//---------------------------------------------------------------------------
// Check whether target has the same prefix as this array of pairs.
// {a1,b2,c3} <<< {a1,b2} is prefix.
//---------------------------------------------------------------------------
bool CSTPairs::equalPrefix(const CSTPairs& target) const
{
    if (target.GetSize() != GetSize())
        return false;

    for (int i = 0; i < GetSize() - 1; ++i) {	
        if (!GetAt(i)->isEqual(*target[i]))
            return false;
    }
    return true;
}

//---------------------------------------------------------------------------
// Check whether target is a super sequence of this array of pairs.
//---------------------------------------------------------------------------
bool CSTPairs::isSuperseq(const CSTPairs& target) const
{
    if (target.GetSize() < GetSize())
        return false;

    int startPos = 0;
    CSTPair* pThisPair = NULL;
    CSTPair* pTargetPair = NULL;
    for (int i = 0; i < GetSize(); ++i) {
        pThisPair = GetAt(i);

        bool bFound = false;
        for (int j = startPos; j < target.GetSize(); ++j) {
            pTargetPair = target.GetAt(j);
            if (pTargetPair->m_pTime->m_timestamp > pThisPair->m_pTime->m_timestamp)
                return false;

            if (pThisPair->isEqual(*pTargetPair)) {
                bFound = true;
                startPos = ++j; 
                break;
            }
        }

        if (!bFound)
            return false;
    }
    return true;
}

//---------------------------------------------------------------------------
// Check whether target is a super sequence of this array of pairs.
// Used in subsequent windows only because itemsets are arranged 
// starting from the higher timestamp, unlike the first window.
//---------------------------------------------------------------------------
bool CSTPairs::isSuperseq2(const CSTPairs& target) const
{
    if (target.GetSize() < GetSize())
        return false;

    int startPos = 0;
    CSTPair* pThisPair = NULL;
    CSTPair* pTargetPair = NULL;
    for (int i = 0; i < GetSize(); ++i) {
        pThisPair = GetAt(i);

        bool bFound = false;
        for (int j = startPos; j < target.GetSize(); ++j) {
            pTargetPair = target.GetAt(j);
            if (pTargetPair->m_pTime->m_timestamp < pThisPair->m_pTime->m_timestamp)
                return false;

            if (pThisPair->isEqual(*pTargetPair)) {
                bFound = true;
                startPos = ++j; 
                break;
            }
        }

        if (!bFound)
            return false;
    }
    return true;
}


//*********************
// CSTPairsCollection *
//*********************

CSTPairsCollection::CSTPairsCollection()
{
}

CSTPairsCollection::~CSTPairsCollection()
{
}

void CSTPairsCollection::cleanup()
{
    for (int i = 0; i < GetSize(); ++i) {
        GetAt(i)->cleanup();
        delete GetAt(i);
    }
    RemoveAll();
}