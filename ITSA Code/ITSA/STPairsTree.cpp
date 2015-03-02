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
#include "STPairsTree.h"
#include "STAttrib.h"

//********************
// CSTPairsTreeNodes *
//********************

CSTPairsTreeNodes::CSTPairsTreeNodes()
{
}

CSTPairsTreeNodes::~CSTPairsTreeNodes()
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CSTPairsTreeNodes::cleanup()
{
    while (!IsEmpty())
        delete RemoveHead();
}

//*******************
// CSTPairsTreeNode *
//*******************

CSTPairsTreeNode::CSTPairsTreeNode()
    : m_pParent(NULL),
      m_pLoc(NULL),
      m_pTime(NULL),
      m_childPos(NULL),
      m_supCount(0)
{
}

CSTPairsTreeNode::CSTPairsTreeNode(CSTPairsTreeNode* pParent, CSTConcept* pLoc, CSTConcept* pTime)
    : m_pParent(pParent),
      m_pLoc(pLoc),
      m_pTime(pTime),
      m_childPos(NULL),
      m_supCount(0)
{
}

CSTPairsTreeNode::CSTPairsTreeNode(CSTPairsTreeNode* pParent, CSTConcept* pLoc, CSTConcept* pTime, const POSITION pos, int pairCount, const CRFIntArray& senCounts)
	: m_pParent(pParent),
      m_pLoc(pLoc),
      m_pTime(pTime),
      m_childPos(NULL),
      m_supCount(pairCount) 
{
	m_allPosArray.Add(pos);
	m_senCounts.Copy(senCounts);
}

CSTPairsTreeNode::CSTPairsTreeNode(CSTPairsTreeNode* pParent, CSTConcept* pLoc, CSTConcept* pTime, const CPosArray& posArray, int pairCount, const CRFIntArray& senCounts)
	: m_pParent(pParent),
      m_pLoc(pLoc),
      m_pTime(pTime),
      m_childPos(NULL),
      m_supCount(pairCount) 
{
	m_allPosArray.Copy(posArray);
	m_senCounts.Copy(senCounts);
}

CSTPairsTreeNode::~CSTPairsTreeNode()
{
    cleanup();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CSTPairsTreeNode::cleanup()
{ 
    m_childNodes.cleanup(); 
    m_timestampIndex.RemoveAll();
    m_senCounts.RemoveAll();
    m_pLoc = m_pTime = NULL;
    m_supCount = 0;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CSTPairsTreeNode::printChildNodes()
{
    for (POSITION pos = m_childNodes.GetHeadPosition(); pos != NULL;) {
        CSTPairsTreeNode* pChildNode = m_childNodes.GetNext(pos);
        cout << pChildNode->m_pLoc->m_conceptValue << RF_PATHDATA_LOCTIME_SEPARATOR << RF_PATHDATA_TIME_DELIMETER << pChildNode->m_pTime->m_conceptValue << _T(" Count = ") << pChildNode->m_supCount << endl;
    }
}

//---------------------------------------------------------------------------
// This function adds path to the tree.
// Child nodes are sorted by timestamps. m_timestampIndex is an index on timestamp. 
// Each element in m_timestampIndex points to the first node having the timestamp.
// if win = 1 (default is 0), then it is a subsequent window.
//---------------------------------------------------------------------------
bool CSTPairsTreeNode::addPath(const CSTPairs& path, int nSensitiveValues, int win)
{
    return addPathHelper(path, 0, nSensitiveValues, win);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTPairsTreeNode::addPathHelper(const CSTPairs& path, int pairIdx, int nSensitiveValues, int win, bool nodeExists)
{
    // This is the leaf node.
    if (pairIdx == path.GetSize()) {
        if (!m_childNodes.IsEmpty()) {
            cerr << _T(" CSTPairsTreeNode::addPathHelper: This is supposed to be a leaf node. Path: ") << path.toString() <<  endl;
            ASSERT(false);
            return false;
        }

		// win = 1 means it's a subsequent window. If win = 0, it's the first window. 
		if (win == 1)
		{
			if (nodeExists)
			{				
				// supp and conf of itemset 'path' are stored in its last pair. Refere to tempPair in genWinCandidates().
				// Update the supp and conf of existing itemset only if
				// prospective itemset is from a different tree branch.
				// E.g., b2c3d4 can be formed twice from both c3 + b2d4
				// and b2 + c3d4, all can be in the same branch.

				POSITION currPos = path[pairIdx - 1]->m_currentPos;		 																		
				for (int i = m_allPosArray.GetUpperBound(); i >= 0; --i)
				{
					if (m_allPosArray.GetAt(i) == currPos)
						return true;
				}

				m_allPosArray.Add(currPos);
				m_supCount += path[pairIdx - 1]->m_cpyPairCount;
				for (int f = 0; f < path[pairIdx - 1]->m_cpySenCounts.GetSize(); f++)
					m_senCounts[f] += path[pairIdx - 1]->m_cpySenCounts[f];
			}
			return true;
		}
		else
		{
			// First window uses different cand generation & filtering functions.
			// Every possibe combination is generated first. Might not exist in raw data.
			// Therefore, setup and initialize the array of sensitive counts to 0.
			// Initialize support to 0.
			m_supCount = 0;
			m_senCounts.SetSize(nSensitiveValues);
			for (int i = 0; i < nSensitiveValues; ++i)
				m_senCounts[i] = 0;
			return true;
		}
    }
  
    // Adjust starting position depends on m_timestampIndex
    bool bNewTimeIndex = false;
    POSITION startPos = NULL;
    CSTPairsTreeNode* pNewNode = NULL;
    CSTPair* pCurrPair = path[pairIdx];
    int currTimestampIdx = pCurrPair->m_pTime->m_timestamp - 1;
    if (currTimestampIdx >= m_timestampIndex.GetSize() || m_timestampIndex[currTimestampIdx] == NULL) {
        bNewTimeIndex = true;
        startPos = m_childNodes.GetHeadPosition(); 												  
    }
    else {
        bNewTimeIndex = false;
        startPos = m_timestampIndex[currTimestampIdx]->m_childPos;
    }

    // Search child node starting from startPos.
    CSTPairsTreeNode* pChildNode = NULL;
    for (POSITION pos = startPos; pos != NULL;) {
        POSITION currPos = pos;
        pChildNode = m_childNodes.GetNext(pos); 												
        if (pChildNode->m_pTime == pCurrPair->m_pTime && pChildNode->m_pLoc == pCurrPair->m_pLoc) {
            // Matched pair found, no need to add. Decend to child node. 
            return pChildNode->addPathHelper(path, pairIdx + 1, nSensitiveValues, win, true);
        }
        else if (pChildNode->m_pTime->m_timestamp > pCurrPair->m_pTime->m_timestamp) {
            // No matched found. Insert it before the current node. Then decend to child node.
           if (pairIdx != 0) // 
			   pNewNode = new CSTPairsTreeNode(this, pCurrPair->m_pLoc, pCurrPair->m_pTime, pCurrPair->m_currentPos, pCurrPair->m_cpyPairCount, pCurrPair->m_cpySenCounts);
		   else
			   pNewNode = new CSTPairsTreeNode(this, pCurrPair->m_pLoc, pCurrPair->m_pTime, pCurrPair->m_allPositions, pCurrPair->m_cpyPairCount, pCurrPair->m_cpySenCounts);

            pNewNode->m_childPos = m_childNodes.InsertBefore(currPos, pNewNode); 
            if (bNewTimeIndex) {
                // update the index only if it is the first timestamp
                m_timestampIndex.SetAtGrow(currTimestampIdx, pNewNode); 
            }
            return pNewNode->addPathHelper(path, pairIdx + 1, nSensitiveValues, win, false);
        }
    }

    // Either m_childNodes is empty or the new node has the largest timestamp.
    // Create a new node at the end. 
	// If a new pair, initialize the node with m_allPositions. Otherwise, use m_currentPos. 
	if (pairIdx != 0) // 
		pNewNode = new CSTPairsTreeNode(this, pCurrPair->m_pLoc, pCurrPair->m_pTime, pCurrPair->m_currentPos, pCurrPair->m_cpyPairCount, pCurrPair->m_cpySenCounts);
	else 
		pNewNode = new CSTPairsTreeNode(this, pCurrPair->m_pLoc, pCurrPair->m_pTime, pCurrPair->m_allPositions, pCurrPair->m_cpyPairCount, pCurrPair->m_cpySenCounts);


    pNewNode->m_childPos = m_childNodes.AddTail(pNewNode);
    if (bNewTimeIndex) {
        // update the index only if it is the first timestamp
        m_timestampIndex.SetAtGrow(currTimestampIdx, pNewNode);																
    }
    return pNewNode->addPathHelper(path, pairIdx + 1, nSensitiveValues, win, false);
}

//---------------------------------------------------------------------------
// This function increments the count of a path in this tree if the path is
// a subsequence of targetPath.
//---------------------------------------------------------------------------
bool CSTPairsTreeNode::incrementCount(const CSTPairs& targetPath, int sensitiveIdx)
{
    if (targetPath.IsEmpty()) {
        cerr << _T("CSTPairsTreeNode::incrementCount: Empty target path.") <<  endl;
        ASSERT(false);
        return false;
    }
    if (isEmpty()) {
        cerr << _T("CSTPairsTreeNode::incrementCount: Empty tree.") <<  endl;
        ASSERT(false);
        return false;
    }
    return incrementCountHelper(targetPath, 0, sensitiveIdx);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTPairsTreeNode::incrementCountHelper(const CSTPairs& targetPath, int pairIdx, int sensitiveIdx)
{
    // Check leaf node.
    if (m_childNodes.GetSize() == 0) {
        ++m_supCount;
        ++m_senCounts[sensitiveIdx];
        return true;
    } 
    
    // Check end of targetPath.
    if (pairIdx >= targetPath.GetSize())
        return true;    

    POSITION startPos = NULL;
    CSTPair* pCurrPair = targetPath[pairIdx];
    int currTimestampIdx = pCurrPair->m_pTime->m_timestamp - 1;

    if (currTimestampIdx >= m_timestampIndex.GetSize() || !m_timestampIndex[currTimestampIdx])
        startPos = NULL; 
    else
        startPos = m_timestampIndex[currTimestampIdx]->m_childPos;

    CSTPairsTreeNode* pChildNode = NULL;    
    for (POSITION pos = startPos; pos != NULL;) {
        pChildNode = m_childNodes.GetNext(pos);
        if (pChildNode->m_pTime->m_timestamp > pCurrPair->m_pTime->m_timestamp)
            break;  
        if (pChildNode->m_pTime == pCurrPair->m_pTime && pChildNode->m_pLoc == pCurrPair->m_pLoc) {
            // Matched pair found, decend to this child
            if (!pChildNode->incrementCountHelper(targetPath, pairIdx + 1, sensitiveIdx))
                return false;
            break;
        }
    }

    // Try the next pair in target path.
    return incrementCountHelper(targetPath, pairIdx + 1, sensitiveIdx);
}

//---------------------------------------------------------------------------
// Determine whether the candidtes are minimal moles or non moles.
// Result is stored in minMoles and nonMoles.
// Caller is responsible to deallocate minMoles and nonMoles.
//---------------------------------------------------------------------------
bool CSTPairsTreeNode::filterMoles(CSTPairsCollection& minMoles, 
                                   CSTPairsCollection& nonMoles, 
                                   int minK, double maxConf,
                                   CSTAttrib* pSenAttrib, int win) const
{
    if (isEmpty()) {
        cerr << _T(" CSTPairsTreeNode::filterMoles: Empty tree.") <<  endl;
        ASSERT(false);
        return false;
    }
    return filterMolesHelper(minMoles, nonMoles, minK, maxConf, pSenAttrib, win);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTPairsTreeNode::filterMolesHelper(CSTPairsCollection& minMoles, 
                                         CSTPairsCollection& nonMoles, 
                                         int minK, double maxConf,
                                         CSTAttrib* pSenAttrib, int win) const
{
    // Leaf node
    if (m_childNodes.GetSize() == 0) {
        // If a candidate has support count = 0, it should be ignored.
        if (m_supCount == 0)
            return true;

        CSTPairs* pNewPath = new CSTPairs();            
        if (!createPath(pNewPath))
            return false;

		CSTPair* pPair = pNewPath->GetAt(pNewPath->GetUpperBound());	
		if (isPrivacyViolated(minK, maxConf, pSenAttrib, pPair->m_cpyPairCount, pPair->m_cpySenCounts, win))
            minMoles.Add(pNewPath);
        else
            nonMoles.Add(pNewPath);
        return true;
    }
    
    CSTPairsTreeNode* pChildNode = NULL;
    for (POSITION pos = m_childNodes.GetHeadPosition(); pos != NULL;) {
        pChildNode = m_childNodes.GetNext(pos);
        if (!pChildNode->filterMolesHelper(minMoles, nonMoles, minK, maxConf, pSenAttrib, win))
            return false;
    }
    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTPairsTreeNode::createPath(CSTPairs* pNewPath) const
{
    if (!m_pParent)        
        return true;
    
    if (!m_pParent->createPath(pNewPath))
        return false;
	pNewPath->Add(new CSTPair(m_pLoc, m_pTime, m_allPosArray, m_supCount, m_senCounts));
    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTPairsTreeNode::isPrivacyViolated(int minK, double maxConf, CSTAttrib* pSenAttrib, int support, const CRFIntArray& senCounts, int win) const
{
	if (win == 1)	// Subsequent windows.
	{
		if (support < minK)	
		return true;
			
		CSTConcept* pSenRoot = pSenAttrib->getConceptRoot();
		for (int i = 0; i < pSenRoot->getNumChildConcepts(); ++i) {
			if (pSenRoot->getChildConcept(i)->m_bSensitive)
				if ((double) senCounts[i] / support > maxConf)	
					return true;
		}
		return false;
	}

	else {
		if (win == 0)	// First window.
		{
			if (m_supCount < minK)	
				return true;

			CSTConcept* pSenRoot = pSenAttrib->getConceptRoot();
			for (int i = 0; i < pSenRoot->getNumChildConcepts(); ++i) {
				if (pSenRoot->getChildConcept(i)->m_bSensitive)
					if ((double) m_senCounts[i] / m_supCount > maxConf)	
						return true;
			}
			return false;
		}
	}
	cerr << " CSTPairsTreeNode::isPrivacyViolated(). Error: int window must be 0 or 1." << endl;
	return true;
}