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

#include "STPair.h"
#include "STWindowTree.h"

class CSTMoleTreeNode;
class CRFCandTable;

typedef CTypedPtrList<CPtrList, CSTMoleTreeNode*> CSTMoleTreeNodeList;
class CSTMoleTreeNodes : public CSTMoleTreeNodeList
{
public:
    CSTMoleTreeNodes();
    virtual ~CSTMoleTreeNodes();
    void cleanup();
};

//---------------------------------------------------------------------------
// This class represents the Minimal Mole Tree.
//---------------------------------------------------------------------------
class CSTMoleTreeNode
{
public:
	CSTMoleTreeNode();
    CSTMoleTreeNode(CSTMoleTreeNode* pParent, CSTConcept* pLoc, CSTConcept* pTime);
    virtual ~CSTMoleTreeNode();

// operations
    inline bool isEmpty() const { return m_childNodes.GetSize() == 0; }
    inline CRFCandTable* getCandTable() const { return m_pCandTable; }
    CString toString() const;
    bool addMole(const CSTPairs& mole);
    bool killNode(CRFCandTable* pCandTable);

protected:
    bool addMoleHelper(const CSTPairs& mole, int pairIdx, CRFCandTable* pCandTable);
    bool deregisterDescendants(CRFCandTable* pCandTable);
    bool suicide(CRFCandTable* pCandTable);

// attributes
    CSTMoleTreeNode* m_pParent;					// Root of tree.
    CSTMoleTreeNodes m_childNodes;				// Child nodes of the current node.
    POSITION m_childPos;                        // position of the current node in the m_childNodes of parent node.
    POSITION m_linkPos;                         // position in the link.
    CSTConcept* m_pLoc;
    CSTConcept* m_pTime;
    int m_moleCount;                            // Number of moles containing this pair. 
    CRFCandTable* m_pCandTable;                 // Candidate table resides in root node only

    friend CRFCandTable;
};

//---------------------------------------------------------------------------
// An entry in the CRFCandTable. This class keep tracks of the number of 
// instances. It also links up all the nodes that contain this pair.
//---------------------------------------------------------------------------
class CRFCandInfo
{
public:
    CRFCandInfo() : m_nInstances(0) {};
    virtual ~CRFCandInfo() {};

    int m_nInstances;	
    CSTMoleTreeNodes m_linkNodes;
};

//---------------------------------------------------------------------------
// Keep track of a list of candidates for suppression.
//---------------------------------------------------------------------------
typedef CTypedPtrMap<CMapStringToPtr, CString, CRFCandInfo*> CRFStringToCandInfoMap;
class CRFCandTable : public CRFStringToCandInfoMap
{
public:
	CRFCandTable();
    virtual ~CRFCandTable();
    void cleanup();
    
// operations
    bool pickWinner(CString& winner, CRFWinCandTable* pWinTable) const;
    bool suppress(LPCTSTR winner, int& nDistortions);
    bool registerNode(CSTMoleTreeNode* pTargetNode);
    bool deregisterNode(const CSTMoleTreeNode* pTargetNode);
};