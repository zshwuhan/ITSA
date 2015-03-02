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
#include "stdafx.h"
#include "STPair.h"
#include "STDataMgr.h"
#include "STPairsTree.h"


class CSTWindowTreeNode;
class CRFWinCandTable;
class CRFWinCandInfo;
class CHistoricPairs;

typedef CTypedPtrList<CPtrList, CSTWindowTreeNode*> CSTWindowTreeNodeList;
class CSTWindowTreeNodes : public CSTWindowTreeNodeList
{
public:
    CSTWindowTreeNodes();
    virtual ~CSTWindowTreeNodes();
    void cleanup();
};

//---------------------------------------------------------------------------
// This class is a tree representation of the sliding window.
//---------------------------------------------------------------------------
class CSTWindowTreeNode
{
public:
	CSTWindowTreeNode();
	CSTWindowTreeNode(CSTWindowTreeNode* pParent, CSTWindowTreeNode* targetNode);	// To make a copy of targetNode.
    CSTWindowTreeNode(CSTWindowTreeNode* pParent, CSTConcept* pLoc, CSTConcept* pTime);
    virtual ~CSTWindowTreeNode();

	// Operations
    inline bool isEmpty() const { return m_childNodes.GetSize() == 0; }
	inline int getNumChildNoes() const { return m_childNodes.GetSize(); }
    inline CRFWinCandTable* getCandTable() const { return m_pCandTable; }
    CString toString() const;

	void setNumSenValues(int nSenValues) { m_nSensitiveValues = nSenValues; }
	void printTreeWindow(const CSTDataMgr* pData, int depth = 1) const;
	void sanitizeSubPath(CSTPairs& pairs);
	bool removeOldNodes(const int offset, CStringArray& oldPairs);

	bool slideWindow(CSTDataMgr* pDataMgr, const int stepSize, const int offset, const int winSize, long& readTime);
	void removeNodes(CStringArray& sPairs);
	void divideC1(int offset, CSTPairsCollection& minMoles, CSTPairsCollection& nonMoles, int minK, double maxConf, CSTAttrib* pSenAttrib);
	bool isPrivacyViolated(int minK, double maxConf, CSTAttrib* pSenAttrib, int support, const CRFIntArray& totalSenCounts) const;
	
	inline int getPairCount() const { return m_pairCount; }
	inline const CRFIntArray& getSenCounts() const { return m_senCounts; }	
	int getTotalNumInstances();
	int getDistortion(CStringArray& winnerPairs);
	int getHistoricDistortion();

	// First window operations
	void readFirstWindowPairs(CSTDataMgr* pDataMgr, int offset, int winSize);	// Read raw data.
	bool addFirstWindowPairs(const CSTPairs& newPairs, int senValueIdx);	// Add pairs to the tree.

	// Subsequent windows operations
	void readWindowPairs(CSTDataMgr* pDataMgr, int offset, int winSize, long& winReadTime);	// Reading raw data into the current window.
	bool addWindowPairs(const CSTPairs& newPairs, int senValueIdx);	// Add pairs to the tree.
	void genWinCandidates(CSTPairsTreeNode& candidates, const CSTPairsCollection& minMoles, const CSTPairsCollection& nonMoles);
	

	CHistoricPairs* m_pHistoricPairs;	// Array in root node only.

protected:
    bool addFirstWindowPairsHelper(const CSTPairs& newPairs, int pairIdx, CRFWinCandTable* pCandTable, int senValueIdx);
	bool addWindowPairsHelper(const CSTPairs& newPairs, int pairIdx, CRFWinCandTable* pCandTable, int senValueIdx, int firstNewTimestamp);
	bool removeNodesHelper(CSTWindowTreeNode* pTargetParent, CSTWindowTreeNode* pRoot);

// attributes
    CSTWindowTreeNode* m_pParent;				// Root of tree.
    CSTWindowTreeNodes m_childNodes;			// Child nodes of the current node.
    POSITION m_childPos;                        // position of the current node in the m_childNodes of parent node.
    POSITION m_linkPos;                         // position in the link.
    CSTConcept* m_pLoc;
    CSTConcept* m_pTime;
	
	CRFIntArray m_senCounts;                    // Support count of sensitive value in the paths this pair has instances in.
    int m_pairCount;                            // Number of paths containing this pair.
												
    CRFWinCandTable* m_pCandTable;              // Candidate table resides in root node only.


	int m_firstNewTimestamp;					// First new timestamp due to a window slide.
	int m_winUpperBound;						// Last possible timestamp in the window.
	int m_nSensitiveValues;						// Total number of sensitive values.

    friend CRFWinCandTable;
	friend CHistoricPairs;
};


//////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
// An entry in the CRFWinCandTable. This class keep tracks of the number of 
// instances. It also links up all the nodes that contain this pair.
//---------------------------------------------------------------------------
class CRFWinCandInfo
{
public:
    CRFWinCandInfo() : m_nInstances(0) {};
    virtual ~CRFWinCandInfo() {};

    int m_nInstances;	
    CSTWindowTreeNodes m_linkNodes;
};

//---------------------------------------------------------------------------
// Keep track of a list of candidates for suppression.
//---------------------------------------------------------------------------
typedef CTypedPtrMap<CMapStringToPtr, CString, CRFWinCandInfo*> CRFWinStringToCandInfoMap;
class CRFWinCandTable : public CRFWinStringToCandInfoMap
{
public:
	CRFWinCandTable();
    virtual ~CRFWinCandTable();
    void cleanup();
    
// operations
    bool registerNode(CSTWindowTreeNode* pTargetNode);
    bool deregisterNode(const CSTWindowTreeNode* pTargetNode);
};


//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
// Historic Array contains pairs that were supressed in previous windows
// but still fit in the current window.
// Used to calculate the distortion of current window.
// CHistoricPair is an entry in CHistPairsArray.
//----------------------------------------------------------------------------

class CHistoricPair
{
public:
    CHistoricPair(CString sHistPair, int instances) 
		: m_sPair(sHistPair), m_nInstances(instances)
	{
		CString tempLoc, tempTime;
		CSTPair::parsePair(m_sPair, tempLoc, tempTime);
		m_timestamp = int (StrToInt(tempTime));
	};
    
	CHistoricPair(CHistoricPair* src)
	{
		m_sPair = src->m_sPair;
		m_nInstances = src->m_nInstances;
		m_timestamp = src->m_timestamp;
	};

	virtual ~CHistoricPair() {};

	CString m_sPair;
    int m_nInstances;
	int m_timestamp;
};


typedef CTypedPtrArray<CPtrArray, CHistoricPair*> CHistoricPairsArray;
class CHistoricPairs : public CHistoricPairsArray
{
public:
    CHistoricPairs();
    virtual ~CHistoricPairs();
    void cleanup();
	bool addHistoricPair(CString sHistPair, int instances);
	int getNumInstances() const;
	bool removeOldHistPairs(int offset, CHistoricPairs* tempArray, bool& allAreOldPairs);
};