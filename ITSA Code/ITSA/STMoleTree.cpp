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
#include "STMoleTree.h"

//*******************
// CSTMoleTreeNodes *
//*******************

CSTMoleTreeNodes::CSTMoleTreeNodes()
{
}

CSTMoleTreeNodes::~CSTMoleTreeNodes()
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CSTMoleTreeNodes::cleanup()
{
    while (!IsEmpty())
        delete RemoveHead();
}


//******************
// CSTMoleTreeNode *
//******************

CSTMoleTreeNode::CSTMoleTreeNode()
    : m_pParent(NULL),
      m_pLoc(NULL),
      m_pTime(NULL),
      m_childPos(NULL),
      m_linkPos(NULL),
      m_pCandTable(NULL),
      m_moleCount(0)
{
}

CSTMoleTreeNode::CSTMoleTreeNode(CSTMoleTreeNode* pParent, CSTConcept* pLoc, CSTConcept* pTime)
    : m_pParent(pParent),
      m_pLoc(pLoc),
      m_pTime(pTime),
      m_childPos(NULL),
      m_linkPos(NULL),
      m_pCandTable(NULL),
      m_moleCount(0)
{
}

CSTMoleTreeNode::~CSTMoleTreeNode()
{
    delete m_pCandTable;
    m_pCandTable = NULL;
    m_childNodes.cleanup();
}

CString CSTMoleTreeNode::toString() const
{
    return m_pLoc->m_conceptValue + RF_PATHDATA_LOCTIME_SEPARATOR + RF_PATHDATA_TIME_DELIMETER + m_pTime->m_conceptValue;
}

//---------------------------------------------------------------------------
// Build the mole tree
// Child nodes are sorted by timestamps.
//---------------------------------------------------------------------------
bool CSTMoleTreeNode::addMole(const CSTPairs& mole)
{
    if (!m_pCandTable)
        m_pCandTable = new CRFCandTable();
    return addMoleHelper(mole, 0, m_pCandTable);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTMoleTreeNode::addMoleHelper(const CSTPairs& mole, int pairIdx, CRFCandTable* pCandTable)
{
    // Count number of moles under this node.
    // If this is a root node, then moleCount is the total number of moles.
    ++m_moleCount;

    // This is the leaf node.
    if (pairIdx == mole.GetSize())
        return true;

    // Search each child node.
    CSTMoleTreeNode* pNewNode = NULL;
    CSTMoleTreeNode* pChildNode = NULL;
    CSTPair* pCurrPair = mole[pairIdx];
    for (POSITION pos = m_childNodes.GetHeadPosition(); pos != NULL;) {
        POSITION currPos = pos;
        pChildNode = m_childNodes.GetNext(pos);
        if (pChildNode->m_pTime == pCurrPair->m_pTime && pChildNode->m_pLoc == pCurrPair->m_pLoc) {
            // Matched pair found, no need to add. Descend to child node.  
            return pChildNode->addMoleHelper(mole, pairIdx + 1, pCandTable);
        }
        else if (pChildNode->m_pTime->m_timestamp > pCurrPair->m_pTime->m_timestamp) {
            // No matched found. Insert it before the current node. Then decend to child node.
            pNewNode = new CSTMoleTreeNode(this, pCurrPair->m_pLoc, pCurrPair->m_pTime);
            pNewNode->m_childPos = m_childNodes.InsertBefore(currPos, pNewNode);
            if (!pCandTable->registerNode(pNewNode))
                return false;
            return pNewNode->addMoleHelper(mole, pairIdx + 1, pCandTable);
        }
    }

    // No matched pair found in m_childNodes
    // Create a new node at the end.
    pNewNode = new CSTMoleTreeNode(this, pCurrPair->m_pLoc, pCurrPair->m_pTime);
    pNewNode->m_childPos = m_childNodes.AddTail(pNewNode);								
    if (!pCandTable->registerNode(pNewNode))
        return false;
    return pNewNode->addMoleHelper(mole, pairIdx + 1, pCandTable);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTMoleTreeNode::killNode(CRFCandTable* pCandTable)
{
	// Remove from table.
    // Deregister all its descendants.
	// For every child node, remove it from the m_linkNodes it is associated with.
    if (!deregisterDescendants(pCandTable))
        return false;

    // Suppress this node.
	// Remove from tree.
    return suicide(pCandTable);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTMoleTreeNode::deregisterDescendants(CRFCandTable* pCandTable)
{
    CSTMoleTreeNode* pChildNode = NULL;
    for (POSITION childPos = m_childNodes.GetHeadPosition(); childPos != NULL;) {	
        pChildNode = m_childNodes.GetNext(childPos);
        if (!pChildNode->deregisterDescendants(pCandTable))
            return false;

        // deregister this child node.
		// Remove target node from m_linkNodes in the pCandInfo.
        if (!pCandTable->deregisterNode(pChildNode))
            return false;
    }
    return true;
}

//---------------------------------------------------------------------------
// Remove this node from its parent. Then delete itself.
//---------------------------------------------------------------------------
bool CSTMoleTreeNode::suicide(CRFCandTable* pCandTable)
{
    if (!m_pParent)
        return true;

    // Remove itself from parent    
    m_pParent->m_childNodes.RemoveAt(m_childPos);
    if (m_pParent->m_childNodes.GetSize() == 0 && m_pParent->m_pParent) {
        // Kill the parent node if it has no children and if it is not the root.
        if (!pCandTable->deregisterNode(m_pParent))
            return false;
        if (!m_pParent->suicide(pCandTable))
            return false;
    }
	else {
		// Decrement the count of all its ancestors.
		CSTMoleTreeNode* pAncestor = m_pParent;
		while (pAncestor) {
			pAncestor->m_moleCount -= m_moleCount;
			pAncestor = pAncestor->m_pParent;
		}
	}   
    
    // Delete itself.
    delete this;
    return true;
}

//***************
// CRFCandTable *
//***************

CRFCandTable::CRFCandTable()
{
}

CRFCandTable::~CRFCandTable()
{
    cleanup();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CRFCandTable::cleanup()
{
    CString sPair;
    CRFCandInfo* pCandInfo = NULL;
    for (POSITION pos = GetStartPosition(); pos != NULL;) {
        GetNextAssoc(pos, sPair, pCandInfo);	
        delete pCandInfo;
        pCandInfo = NULL;
    }
    RemoveAll();	
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CRFCandTable::pickWinner(CString& winner, CRFWinCandTable* pWinTable) const
{
    winner.Empty();
    if (IsEmpty()) {
        cerr << _T(" CRFCandTable::pickWinner: Failed to find winner because the candidate table is empty.") << endl;
        ASSERT(false);
        return false;
    }

    double score = 0.0f;
    double highestScore = -1.0f;
    CString sPair;
    CSTMoleTreeNode* pNode = NULL;
    CRFCandInfo* pCandInfo = NULL;
    CRFWinCandInfo* pWinCandInfo = NULL;

    for (POSITION pos = GetStartPosition(); pos != NULL;) {	
        GetNextAssoc(pos, sPair, pCandInfo);	

        // Count number of moles
        int nMoles = 0;
        for (POSITION linkPos = pCandInfo->m_linkNodes.GetHeadPosition(); linkPos != NULL;) {	
            pNode = pCandInfo->m_linkNodes.GetNext(linkPos);	
            ASSERT(pNode->m_moleCount > 0);
            nMoles += pNode->m_moleCount;
        }

		if ( (!pWinTable->Lookup(sPair, pWinCandInfo)) )
			cerr << " CRFCandTable::pickWinner(). Error: could not find pair " << sPair << " in window tree." << endl;


        // Calculate score.
		score = ((double) nMoles) / pWinCandInfo->m_nInstances;
        if (score > highestScore) {
            highestScore = score;
            winner = sPair;
        }
    }
#if 1
    cout << _T(" Suppressing: ") << winner << _T(" with score = ") << highestScore << endl;
#endif
    return true;
}


//---------------------------------------------------------------------------
// Remove all instances of "winner" from the CVT and adjust the Score Table accordingly.
//---------------------------------------------------------------------------
bool CRFCandTable::suppress(LPCTSTR winner, int& nDistortions)
{    
    CRFCandInfo* pCandInfo = NULL;
    if (!Lookup(winner, pCandInfo)) {	
        cerr << _T(" CRFCandTable::suppress. Failed to suppress: ") << winner << endl;
        ASSERT(false);
        return false;
    }

    CSTMoleTreeNode* pNode = NULL;
    for (POSITION pos = pCandInfo->m_linkNodes.GetHeadPosition(); pos != NULL;) {
        pNode = pCandInfo->m_linkNodes.GetNext(pos);
        if (!pNode->killNode(this))
            return false;
    }

    nDistortions = pCandInfo->m_nInstances;

    // Remove the entry.
    delete pCandInfo;
    pCandInfo = NULL;
    if (!RemoveKey(winner)) {
        cerr << _T(" CRFCandTable::suppress. Failed to find: ") << winner << _T(" for removal.") << endl;
        ASSERT(false);
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------
// Add the node to the link.
//---------------------------------------------------------------------------
bool CRFCandTable::registerNode(CSTMoleTreeNode* pTargetNode)
{
    CRFCandInfo* pCandInfo = NULL;
    if (!Lookup(pTargetNode->toString(), pCandInfo)) {
        pCandInfo = new CRFCandInfo();
        SetAt(pTargetNode->toString(), pCandInfo);
    }
    pTargetNode->m_linkPos = pCandInfo->m_linkNodes.AddTail(pTargetNode);
    return true;
}

//---------------------------------------------------------------------------
// Remove the node from the link.
//---------------------------------------------------------------------------
bool CRFCandTable::deregisterNode(const CSTMoleTreeNode* pTargetNode)
{
    CRFCandInfo* pCandInfo = NULL;
    if (!Lookup(pTargetNode->toString(), pCandInfo)) {
        cerr << _T("CRFCandTable::deregisterNode. Failed to find: ") << pTargetNode->toString() << endl;
        ASSERT(false);
        return false;
    }

    pCandInfo->m_linkNodes.RemoveAt(pTargetNode->m_linkPos);	
    if (pCandInfo->m_linkNodes.GetSize() == 0) {
        // Remove the entry from the table if there are no linked nodes.
        delete pCandInfo;
        pCandInfo = NULL;
        if (!RemoveKey(pTargetNode->toString())) {
            cerr << _T("CRFCandTable::deregisterNode. Failed to find: ") << pTargetNode->toString() << _T(" for removal.") << endl;
            ASSERT(false);
            return false;
        }
    }
    return true;
}