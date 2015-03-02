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

#pragma once

#include "STConcept.h"

class CSTPair
{
public:
    CSTPair(CSTConcept* pLoc, CSTConcept* pTime);
	CSTPair(CSTConcept* pLoc, CSTConcept* pTime, POSITION pos);
	CSTPair(CSTConcept* pLoc, CSTConcept* pTime, const CPosArray& posArray);
	CSTPair(CSTConcept* pLoc, CSTConcept* pTime, POSITION pos, int pairCount, const CRFIntArray& senCounts);
	CSTPair(CSTConcept* pLoc, CSTConcept* pTime, POSITION pos, const CPosArray& posArray, int pairCount, const CRFIntArray& senCounts);
	CSTPair(CSTConcept* pLoc, CSTConcept* pTime, const CPosArray& posArray, int pairCount, const CRFIntArray& senCounts);
    virtual ~CSTPair();
    CString toString() const;
    bool isEqual(const CSTPair& target) const;
    static bool parsePair(const CString& pairStr, CString& locStr, CString& timeStr);
	inline static bool IsAnsiDigit (char c) {return (IsCharAlphaNumeric (c) && !IsCharAlpha (c));}

// attributes
    CSTConcept* m_pLoc;
    CSTConcept* m_pTime;
	POSITION m_currentPos;
	CPosArray m_allPositions;	// Stores the POSITIONs where an itemset containing this pair exists in a window tree.
								// Values of POSITIONs are taken from m_linkNodes in a WindowTreeNode instance.
	CRFIntArray m_cpySenCounts;
	int m_cpyPairCount; 

};

typedef CTypedPtrArray<CPtrArray, CSTPair*> CSTPairArray;
class CSTPairs : public CSTPairArray
{
public:
    CSTPairs();
    virtual ~CSTPairs();
    void cleanup();
    CString toString() const;
    CSTPairs* makeCopy() const;
    bool equalPrefix(const CSTPairs& target) const;
    bool isSuperseq(const CSTPairs& target) const;	// Used in first window.
	bool isSuperseq2(const CSTPairs& target) const; // Used in subsequent windows.
};

typedef CTypedPtrArray<CPtrArray, CSTPairs*> CSTPairsArray;
class CSTPairsCollection : public CSTPairsArray
{
public:
    CSTPairsCollection();
    virtual ~CSTPairsCollection();
    void cleanup();
};