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

//---------------------------------------------------------------------------
// This class represents the Candidate sets as a tree.
//---------------------------------------------------------------------------

#pragma once

#include "STPair.h"

class CSTPairsTreeNode;

typedef CTypedPtrList<CPtrList, CSTPairsTreeNode*> CSTPairsTreeNodeList;
class CSTPairsTreeNodes : public CSTPairsTreeNodeList
{
public:
    CSTPairsTreeNodes();
    virtual ~CSTPairsTreeNodes();
    void cleanup();
};

typedef CTypedPtrArray<CPtrArray, CSTPairsTreeNode*> CSTPairsTreeNodesIndex;

class CSTPairsTreeNode
{
public:
    CSTPairsTreeNode();
    CSTPairsTreeNode(CSTPairsTreeNode* pParent, CSTConcept* pLoc, CSTConcept* pTime);
	CSTPairsTreeNode(CSTPairsTreeNode* pParent, CSTConcept* pLoc, CSTConcept* pTime, const POSITION pos, int pairCount, const CRFIntArray& senCounts);
	CSTPairsTreeNode(CSTPairsTreeNode* pParent, CSTConcept* pLoc, CSTConcept* pTime, const CPosArray& posArray, int pairCount, const CRFIntArray& senCounts);
    virtual ~CSTPairsTreeNode();
    void cleanup();
    void printChildNodes();

// operations    
    inline bool isEmpty() const { return m_childNodes.GetSize() == 0; }
    bool addPath(const CSTPairs& path, int nSensitiveValues, int window = 0);	// window = 0 means it is the first window by default.
    bool incrementCount(const CSTPairs& targetPath, int sensitiveIdx);
    bool filterMoles(CSTPairsCollection& minMoles, CSTPairsCollection& nonMoles, int minK, double maxConf, CSTAttrib* pSenAttrib, int window = 0) const;

protected:
    bool addPathHelper(const CSTPairs& path, int pairIdx, int nSensitiveValues, int window = 0, bool nodeExists = true);
    bool incrementCountHelper(const CSTPairs& targetPath, int pairIdx, int sensitiveIdx);
    bool filterMolesHelper(CSTPairsCollection& minMoles, CSTPairsCollection& nonMoles, int minK, double maxConf, CSTAttrib* pSenAttrib, int window) const;
    bool createPath(CSTPairs* pNewPath) const;
    bool isPrivacyViolated(int minK, double maxConf, CSTAttrib* pSenAttrib, int support, const CRFIntArray& senCounts, int window) const;

// attributes
    CSTPairsTreeNode* m_pParent;
    CSTPairsTreeNodes m_childNodes;
    CSTPairsTreeNodesIndex m_timestampIndex;    // To speed-up the search process.
    POSITION m_childPos;                        // position in the m_childNodes of parent node.

    CSTConcept* m_pLoc;
    CSTConcept* m_pTime;

    int m_supCount;                             // Support count of this path.
    CRFIntArray m_senCounts;                    // Support count of sensitive value in this path.
	CPosArray m_allPosArray;
};
