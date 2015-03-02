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
#include "STWindowTree.h"


//*********************
// CSTWindowTreeNodes *
//*********************

CSTWindowTreeNodes::CSTWindowTreeNodes()
{
}

CSTWindowTreeNodes::~CSTWindowTreeNodes()
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CSTWindowTreeNodes::cleanup()
{
    while (!IsEmpty())
        delete RemoveHead();
}


//********************
// CSTWindowTreeNode *
//********************

CSTWindowTreeNode::CSTWindowTreeNode()
    : m_pParent(NULL),
      m_pLoc(NULL),
      m_pTime(NULL),
      m_childPos(NULL),
      m_linkPos(NULL),
      m_pCandTable(NULL),
	  m_pHistoricPairs(NULL),
      m_pairCount(0),
	  m_firstNewTimestamp(1),
	  m_winUpperBound(0),
	  m_nSensitiveValues(0)
{
}

CSTWindowTreeNode::CSTWindowTreeNode(CSTWindowTreeNode* pParent, CSTWindowTreeNode* targetNode)
    : m_pParent(pParent),
	  m_pLoc(targetNode->m_pLoc),
	  m_pTime(targetNode->m_pTime),
      m_childPos(NULL),
	  m_linkPos(targetNode->m_linkPos),
	  m_pCandTable(NULL),
	  m_pHistoricPairs(NULL),
	  m_pairCount(targetNode->m_pairCount),
	  m_firstNewTimestamp(targetNode->m_firstNewTimestamp),
	  m_winUpperBound(targetNode->m_winUpperBound),
	  m_nSensitiveValues(targetNode->m_nSensitiveValues)
{
	for (POSITION pos = targetNode->m_childNodes.GetHeadPosition(); pos !=NULL;)
	{
		m_childNodes.SetAt(pos, targetNode->m_childNodes.GetAt(pos));
		targetNode->m_childNodes.GetNext(pos);
	}
}

CSTWindowTreeNode::CSTWindowTreeNode(CSTWindowTreeNode* pParent, CSTConcept* pLoc, CSTConcept* pTime)
    : m_pParent(pParent),
      m_pLoc(pLoc),
      m_pTime(pTime),
      m_childPos(NULL),
      m_linkPos(NULL),
      m_pCandTable(NULL),
	  m_pHistoricPairs(NULL),
      m_pairCount(0),
	  m_firstNewTimestamp(pParent->m_firstNewTimestamp),
	  m_winUpperBound(0),
	  m_nSensitiveValues(pParent->m_nSensitiveValues)
{
	m_senCounts.SetSize(m_nSensitiveValues);
    for (int i = 0; i < m_nSensitiveValues; ++i)
		m_senCounts[i] = 0;
}

CSTWindowTreeNode::~CSTWindowTreeNode()
{
    delete m_pCandTable;
    m_pCandTable = NULL;
	delete m_pHistoricPairs;
	m_pHistoricPairs = NULL;
    m_childNodes.cleanup();
	m_senCounts.RemoveAll();
}

CString CSTWindowTreeNode::toString() const
{
    //return m_pLoc->m_conceptValue + m_pTime->m_conceptValue;
    return m_pLoc->m_conceptValue + RF_PATHDATA_LOCTIME_SEPARATOR + RF_PATHDATA_TIME_DELIMETER + m_pTime->m_conceptValue;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CSTWindowTreeNode::printTreeWindow(const CSTDataMgr* pData, int depth) const
{	
	CSTWindowTreeNodes children;
	if (depth == 1)
	{
		cout << "Root: " << m_pairCount << endl;
		cout << "Root's m_firstNewTimestamp : " << m_firstNewTimestamp << endl;
	}

	for (POSITION pos = m_childNodes.GetHeadPosition(); pos != NULL;) 
	{
		CSTWindowTreeNode* pChildNode = m_childNodes.GetNext(pos);
		children.AddTail(pChildNode);
		cout << "[" << depth << "]" << pChildNode->m_pLoc->m_conceptValue
			 << ".T"
			 << pChildNode->m_pTime->m_conceptValue << _T(": ") 
			 << pChildNode->m_pairCount << ". ";

		for (int i = 0; i < pChildNode->m_senCounts.GetSize(); ++i)
			cout << pData->getSenAttrib()->getConceptRoot()->getChildConcept(i)->m_conceptValue
				 << ": " <<pChildNode-> m_senCounts[i] << ", ";

		cout << endl;
	}

	cout << "---------------------------------" << endl;
	for (POSITION childPos = children.GetHeadPosition(); childPos != NULL;) 
	{
		cout << "Child Nodes of " << "[" << depth << "]" 
			 << children.GetAt(childPos)->m_pLoc->m_conceptValue 
			 << ".T"
			 << children.GetAt(childPos)->m_pTime->m_conceptValue << ":\n";
		
		children.GetNext(childPos)->printTreeWindow(pData, depth + 1);
	}
}


//---------------------------------------------------------------------------
// Remove from prospective path, pairs that no longer exist in the tree.
//---------------------------------------------------------------------------
void CSTWindowTreeNode::sanitizeSubPath(CSTPairs& currSubPath)
{
	for (int i = 0; i < currSubPath.GetSize(); ++i)
	{
		CRFWinCandInfo* pCandInfo = NULL;
		CSTPair* pPair = currSubPath.GetAt(i);

		if ((pPair->m_pTime->m_timestamp < m_firstNewTimestamp) &&
			!m_pCandTable->Lookup(pPair->toString(), pCandInfo)) 
		{	
			currSubPath.RemoveAt(i--);
		}
	}
}

//---------------------------------------------------------------------------
// Reading pairs from raw data to construct the first window only.
// "offset" in this case is normally 1.
//---------------------------------------------------------------------------
void CSTWindowTreeNode::readFirstWindowPairs(CSTDataMgr* pDataMgr, int offset, int winSize)
{
	if (!m_pHistoricPairs)
		m_pHistoricPairs = new CHistoricPairs();

	m_winUpperBound = offset + winSize - 1;
	 
	CString tempLoc, tempTime;
    CSTPairs* pEntirePath = NULL;
    CSTRecords* pRecs = pDataMgr->getRecords();
	int currentTimeStamp;
	int p;
	int pathLen;
	int senValueIdx;

    for (int r = 0; r < pRecs->GetSize(); ++r) {
		pEntirePath = pRecs->GetAt(r)->getPath();
		CSTPairs subPath;
		pathLen = pEntirePath->GetSize();
		senValueIdx = pRecs->GetAt(r)->getSensitiveConcept()->m_childIdx;
		p = 0;

		// Read pairs with timestamps that satisfy the current window.
        while (p < pathLen) 
		{
			CSTPair* pPair = pEntirePath->GetAt(p);
			CSTPair::parsePair(pPair->toString(), tempLoc, tempTime);
			currentTimeStamp = int (StrToInt(tempTime));
			if (currentTimeStamp > m_winUpperBound)
				break;
			else
			{
				if (currentTimeStamp < offset) 
				{
					cerr << _T("CSTWindowTreeNode::readFirstWindowPairs(): currentTimeStamp < offset. Terminating.") << endl;
					return;
				}
			}
			// Add current pair to a temp path that is to be added to the tree later.
			subPath.Add(pPair);  
			++p;
		}
		
		if (!subPath.IsEmpty())
			addFirstWindowPairs(subPath, senValueIdx);
		else
		{
			//std::cout << _T("(Empty subPath in Record ") << r << _T(".)\n") << std::endl;
		}
	}
}

//---------------------------------------------------------------------------
// "offset" is the starting timestamp of the window.
//---------------------------------------------------------------------------
bool CSTWindowTreeNode::slideWindow(CSTDataMgr* pDataMgr, const int stepSize, const int offset, const int winSize, long& readTime)
{
	long removeOldNodesTime = getTime();

	// Update m_firstNewTimestamp and m_winUpperBound.
	m_winUpperBound = offset + winSize - 1;
	m_firstNewTimestamp = m_winUpperBound - (stepSize - 1);

	// Remove old pairs from Candidate Table and Historic Array.
	CStringArray oldPairs;
	if (!removeOldNodes(offset, oldPairs)) {
		cerr << _T(" CSTWindowTreeNode::slideWindow(). Error in removing old pairs.") << endl;
		return false;
	}
	removeOldNodesTime = getTime() - removeOldNodesTime;

	// Read new pairs.
	std::cout << _T(" Window# ") << offset << _T(": Reading new pairs from memory into the window tree.") << std::endl;
	long winReadTime = 0;
	readWindowPairs(pDataMgr, offset, winSize, winReadTime);

	readTime = removeOldNodesTime + winReadTime;
	return true;
}

//-----------------------------------------------------------------------------
// Any node with timestamp < offset is an old node.
//-----------------------------------------------------------------------------
bool CSTWindowTreeNode::removeOldNodes(const int offset, CStringArray& oldPairs)
{
	// remove old pairs from Historic Array.
	bool allOldPairs = false;
	CHistoricPairs* updatedHistPairs = new CHistoricPairs();

	m_pHistoricPairs->removeOldHistPairs(offset, updatedHistPairs, allOldPairs);
	if (!updatedHistPairs->IsEmpty())
	{
		delete m_pHistoricPairs;
		m_pHistoricPairs = updatedHistPairs;
	}
	else
	{
		if (allOldPairs)
		{
			// All pairs are old and do not fit current window.
			delete m_pHistoricPairs;
			m_pHistoricPairs = new CHistoricPairs();
		}
		else
		{
			// All pairs fit current window.
			delete updatedHistPairs;
			updatedHistPairs = NULL;
		}
	}

	// Delete old nodes.
	CSTWindowTreeNode* pNode = NULL;
	for (POSITION pos = m_childNodes.GetHeadPosition(); pos != NULL;) 
	{	
		pNode = m_childNodes.GetNext(pos);
		if (pNode->m_pTime->m_timestamp >= offset)
			break;

		// We assume STEP_SIZE == 1.
		// If node "n" is an old node, none of its descendants would be an old pair.
		oldPairs.Add(pNode->toString());
	}

	if (oldPairs.IsEmpty())
		return true;

	removeNodes(oldPairs);
	return true;
}


//---------------------------------------------------------------------------
// Read from raw data pairs that fit in the window.
//---------------------------------------------------------------------------
void CSTWindowTreeNode::readWindowPairs(CSTDataMgr* pDataMgr, int offset, int winSize, long& winReadTime)
{		
	bool isNewPair = false;	
	CString tempLoc, tempTime;
    CSTPairs* pEntirePath = NULL;
    CSTRecords* pRecs = pDataMgr->getRecords();

	long time1, time2, time3, time4, readNewRawDataTime, addWindowPairsTime;
	int currentTimeStamp;
	int pathLen; 
	int senValueIdx;
	int p;
    for (int r = 0; r < pRecs->GetSize(); ++r) {
		isNewPair = false;
		pEntirePath = pRecs->GetAt(r)->getPath();
		CSTPairs subPath;
		pathLen = pEntirePath->GetSize();
		senValueIdx = pRecs->GetAt(r)->getSensitiveConcept()->m_childIdx;

		// Read window pairs only 
		// The first proper pair starts at timestamp = offset,
		// therefore, a timestamp of a pair should fall in the interval [offset, winUpperBound].
		p = 0;
		while (p < pathLen) 
		{
			CSTPair* pPair = pEntirePath->GetAt(p);
			CSTPair::parsePair(pPair->toString(), tempLoc, tempTime);
			currentTimeStamp = int (StrToInt(tempTime));
			if (currentTimeStamp > m_winUpperBound)
				break;
			else
			{
				if (currentTimeStamp < offset) 
				{
					++p; continue;	
				}
			}
			// Current pair fits in the current window. 
			// Add the current pair to a temp path that is to be added to the tree later.
			// Check if the current pair is a new pair.
			if (currentTimeStamp >= m_firstNewTimestamp)
			{
				if (isNewPair == false) 
				{
					isNewPair = true;
					time1 = getTime();
				}
			}
			subPath.Add(pPair);  
			++p;
		}
		if ((!subPath.IsEmpty()) && isNewPair)
		{
			time2 = getTime();
			readNewRawDataTime = time2 - time1;

			// Remove older winnerPairs that are non-existent in current tree, if any.
			sanitizeSubPath(subPath);

			time3 = getTime();
			addWindowPairs(subPath, senValueIdx);
			time4 = getTime();
			addWindowPairsTime = time4 - time3;

			winReadTime = winReadTime + readNewRawDataTime + addWindowPairsTime;
		}
		else
		{
			//std::cout << _T("\nNo new pairs in Record ") << r << _T(".\n") << std::endl;
		}
	}
}

//---------------------------------------------------------------------------
// Build the window tree by adding new pairs to the tree.
// Child nodes are sorted by timestamps.
//---------------------------------------------------------------------------
bool CSTWindowTreeNode::addFirstWindowPairs(const CSTPairs& newPairs, int senValueIdx)
{
    if (!m_pCandTable)
        m_pCandTable = new CRFWinCandTable();
	
    return addFirstWindowPairsHelper(newPairs, 0, m_pCandTable, senValueIdx);
}

//---------------------------------------------------------------------------
// Build the window tree by adding new pairs to the tree.
// Child nodes are sorted by timestamps.
//---------------------------------------------------------------------------
bool CSTWindowTreeNode::addWindowPairs(const CSTPairs& newPairs, int senValueIdx)
{
    if (!m_pCandTable)
	{
		cerr << _T(" CSTWindowTreeNode::addWindowPairs(): Warning: m_pCandTable should have been created beforehand.") << endl;
        m_pCandTable = new CRFWinCandTable();
	}
	
    return addWindowPairsHelper(newPairs, 0, m_pCandTable, senValueIdx, m_firstNewTimestamp);
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTWindowTreeNode::addFirstWindowPairsHelper(const CSTPairs& newPairs, int pairIdx, CRFWinCandTable* pCandTable, int senValueIdx)
{
	// Count number of subpaths sharing this node as a prefix.
    // If this is a root node, then pairCount is the total number of paths.
    ++m_pairCount;
	if (m_pParent)
		++m_senCounts[senValueIdx];

    // This is the leaf node.
    if (pairIdx == newPairs.GetSize())
        return true;

    // Search each child node.
    CSTWindowTreeNode* pNewNode = NULL;
    CSTWindowTreeNode* pChildNode = NULL;
    CSTPair* pCurrPair = newPairs[pairIdx];
    for (POSITION pos = m_childNodes.GetHeadPosition(); pos != NULL;) {
        POSITION currPos = pos;
        pChildNode = m_childNodes.GetNext(pos);
        if (pChildNode->m_pTime == pCurrPair->m_pTime && pChildNode->m_pLoc == pCurrPair->m_pLoc) {
            // Matched pair found, no need to add. Descend to child node.  
            return pChildNode->addFirstWindowPairsHelper(newPairs, pairIdx + 1, pCandTable, senValueIdx);
        }
        else if (pChildNode->m_pTime->m_timestamp > pCurrPair->m_pTime->m_timestamp) {
            // No matched found. Insert it before the current node. Then decend to child node.
            pNewNode = new CSTWindowTreeNode(this, pCurrPair->m_pLoc, pCurrPair->m_pTime);
            pNewNode->m_childPos = m_childNodes.InsertBefore(currPos, pNewNode);
            if (!pCandTable->registerNode(pNewNode))
                return false;
            return pNewNode->addFirstWindowPairsHelper(newPairs, pairIdx + 1, pCandTable, senValueIdx);
        }
    }

    // No matched pair found in m_childNodes
    // Create a new node at the end.
    pNewNode = new CSTWindowTreeNode(this, pCurrPair->m_pLoc, pCurrPair->m_pTime);
    pNewNode->m_childPos = m_childNodes.AddTail(pNewNode);	
    if (!pCandTable->registerNode(pNewNode))
        return false;
    return pNewNode->addFirstWindowPairsHelper(newPairs, pairIdx + 1, pCandTable, senValueIdx);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTWindowTreeNode::addWindowPairsHelper(const CSTPairs& newPairs, int pairIdx, CRFWinCandTable* pCandTable, int senValueIdx, int firstNewTimestamp)
{
    // If root node, then pairCount is the total number of paths.
	// If first pair in newPairs is a new pair, add new record in the current window.
	// If new pair is not the first one in newPairs, record already exists.
	if ((pairIdx == 0) && (newPairs[pairIdx]->m_pTime->m_timestamp >= firstNewTimestamp))
		++m_pairCount; // Root.
	else
	{
		if ((pairIdx > 0) && (m_pTime->m_timestamp >= firstNewTimestamp))
		{
			++m_pairCount;
			++m_senCounts[senValueIdx];
		}
	}

    // This is the leaf node.
    if (pairIdx == newPairs.GetSize())
        return true;

    // Search each child node.
    CSTWindowTreeNode* pNewNode = NULL;
    CSTWindowTreeNode* pChildNode = NULL;
    CSTPair* pCurrPair = newPairs[pairIdx];
    for (POSITION pos = m_childNodes.GetHeadPosition(); pos != NULL;) {
        POSITION currPos = pos;
        pChildNode = m_childNodes.GetNext(pos);
        if (pChildNode->m_pTime == pCurrPair->m_pTime && pChildNode->m_pLoc == pCurrPair->m_pLoc) {
            // Matched pair found, no need to add. Descend to child node.  
            return pChildNode->addWindowPairsHelper(newPairs, pairIdx + 1, pCandTable, senValueIdx, firstNewTimestamp);
        }
        else if (pChildNode->m_pTime->m_timestamp > pCurrPair->m_pTime->m_timestamp) {
            // No matched found. Insert it before the current node. Then decend to child node.
            pNewNode = new CSTWindowTreeNode(this, pCurrPair->m_pLoc, pCurrPair->m_pTime);
            pNewNode->m_childPos = m_childNodes.InsertBefore(currPos, pNewNode);
            if (!pCandTable->registerNode(pNewNode))
                return false;
            return pNewNode->addWindowPairsHelper(newPairs, pairIdx + 1, pCandTable, senValueIdx, firstNewTimestamp);
        }
    }

    // No matched pair found in m_childNodes
    // Create a new node at the end.
    pNewNode = new CSTWindowTreeNode(this, pCurrPair->m_pLoc, pCurrPair->m_pTime);
    pNewNode->m_childPos = m_childNodes.AddTail(pNewNode);																
    if (!pCandTable->registerNode(pNewNode))
        return false;
    return pNewNode->addWindowPairsHelper(newPairs, pairIdx + 1, pCandTable, senValueIdx, firstNewTimestamp);
}


//---------------------------------------------------------------------------
// Generate candidate set 1. Follow the Link of every pair in m_pCandTable
// If offset > 1, it is not first window. Consider itemsets with new pairs only.
//---------------------------------------------------------------------------
void CSTWindowTreeNode::divideC1(int offset, CSTPairsCollection& minMoles, 
								   CSTPairsCollection& nonMoles, int minK, 
								   double maxConf, CSTAttrib* pSenAttrib)
{
	CSTWindowTreeNode* pNode = NULL;
    for (POSITION pos = m_pCandTable->GetStartPosition(); pos != NULL;) {	
		CString sPair;
		CRFWinCandInfo* pCandInfo = NULL;
		m_pCandTable->GetNextAssoc(pos, sPair, pCandInfo);	

		// First window considers all pairs. Subsequent windows consider new pairs only.
		if (offset == FIRST_WIN_STARTING_TIMESTAMP || 
			pCandInfo->m_linkNodes.GetHead()->m_pTime->m_timestamp >= m_firstNewTimestamp)
		{
			int nInstances;
			CRFIntArray totalSenCounts;
			totalSenCounts.SetSize(m_nSensitiveValues);
			CPosArray posArray;
			posArray.SetSize(pCandInfo->m_linkNodes.GetSize());


			// Initialize.
			nInstances = 0;
			for (int i = 0; i < m_nSensitiveValues; ++i)
				totalSenCounts[i] = 0;

			// Count the support and confidence of the current pair.
			int idx = 0;
			for (POSITION linkPos = pCandInfo->m_linkNodes.GetHeadPosition(); linkPos != NULL;) {	
				if (idx == posArray.GetSize()) {
					cerr << " CSTWindowTreeNode::divideC1(). Warning: index exceeded array size" << endl;
					break;
				}
				posArray.SetAt(idx, linkPos);
				pNode = pCandInfo->m_linkNodes.GetNext(linkPos);	
				ASSERT(pNode->m_pairCount > 0);
				nInstances += pNode->m_pairCount;
				for (int j = 0; j < m_nSensitiveValues; ++j)
					totalSenCounts[j] += pNode->m_senCounts[j];
				++idx;
			}

			CSTPairs* tempPath = new CSTPairs();
			tempPath->Add(new CSTPair(pNode->m_pLoc, pNode->m_pTime, posArray, nInstances, totalSenCounts));
			// Decide if minMole or nonMole.
			if (isPrivacyViolated(minK, maxConf, pSenAttrib, nInstances, totalSenCounts))
				minMoles.Add(tempPath);
			else
			{
				// If first window, add pairs in an ascending order.
				if (offset == FIRST_WIN_STARTING_TIMESTAMP)
				{ 
					if (nonMoles.IsEmpty())
						nonMoles.Add(tempPath);
					else 
					{
						bool added = false;
						for (int n = 0; n <= nonMoles.GetUpperBound(); ++n) 
						{
							if (nonMoles.GetAt(n)->GetAt(0)->m_pTime->m_timestamp > tempPath->GetAt(0)->m_pTime->m_timestamp) 
							{
								nonMoles.InsertAt(n, tempPath, 1);
								added = true;
								break; 
							}
						}
						if (!added)
							nonMoles.Add(tempPath);

					} // End else
				} // End if

				else
					nonMoles.Add(tempPath);
			} // End else
		} // End if
	} // End for
}

//---------------------------------------------------------------------------
// Generate candidate itemsets of size "size_of_current_nonMoles + 1" from previous nonMoles.
// The first pair in a nonMole is always a new pair. If STEP_SIZE = 2, then the second pair is a new pair too, etc.
//---------------------------------------------------------------------------
void CSTWindowTreeNode::genWinCandidates(CSTPairsTreeNode& c, const CSTPairsCollection& minMoles, const CSTPairsCollection& nonMoles)
{
	CSTPairs* pCurrentNonMole = NULL;
	if (nonMoles.GetSize() == 0)
	{
		std::cout << _T(" CSTWindowTreeNode::genWinCandidates(). Empty nonMoles.") << std::endl;
		return;
	}

	for (int i = 0; i < nonMoles.GetSize(); ++i) 
	{
		bool flag = false;
		pCurrentNonMole = nonMoles.GetAt(i);
		int currNonMoleSize = pCurrentNonMole->GetSize();
		CSTPair* pLastPair = pCurrentNonMole->GetAt(currNonMoleSize - 1);
		if (pCurrentNonMole->GetAt(0)->m_pTime->m_timestamp >= m_firstNewTimestamp)
		{
			CRFWinCandInfo* pCandInfo = NULL;
            if (m_pCandTable->Lookup(pCurrentNonMole->GetAt(0)->toString(), pCandInfo))
			{
				POSITION linkPos;

				// Only go to branches where current nonMole itemset belongs to.
				for (int p = 0; p < pLastPair->m_allPositions.GetSize(); ++p)
				{
					linkPos =  pLastPair->m_allPositions.GetAt(p);

					// Set m_currentPos in last pair.
					pLastPair->m_currentPos = linkPos;

					CSTWindowTreeNode* pNode = NULL;
					pNode = pCandInfo->m_linkNodes.GetAt(linkPos);	
					CSTWindowTreeNode* pAncestor = pNode->m_pParent;
					while (pAncestor->m_pParent) 
					{
						// The augmented pair.
						// It takes the Loc and Time of pAncestor, but the pairCount and senCounts of pNode.
						// I.e., the supp & conf of current candidate itemset is stored in the last pair.
						// The last pair is set up with the proper supp & conf.
						CSTPair* tempPair = new CSTPair(pAncestor->m_pLoc, pAncestor->m_pTime, linkPos,
														pNode->m_pairCount, pNode->m_senCounts);
						// If tempPair is not in pCurrentNonMole, add it properly. E.g., d4b2 + c3 = d4c3b2.
						bool matchFound = false;
						for (int j = 0; j < currNonMoleSize; ++j)
						{
							if (pCurrentNonMole->GetAt(j)->isEqual(*tempPair))
							{
								matchFound = true;
								break;
							}
						}
						if (!matchFound)
						{
							// Properly add tempPair.
							CSTPairs candItemset;
							candItemset.Append(*pCurrentNonMole);
							for (int n = candItemset.GetUpperBound(); n >= 0; --n) {
								if (candItemset.GetAt(n)->m_pTime->m_timestamp > tempPair->m_pTime->m_timestamp) {
									candItemset.InsertAt(n + 1, tempPair, 1);
									break; }
							}	


							// If tempPair was not added at the tail of the currentNonMole,
							// then the last pair in the itemset needs to be set for the
							// proper supp and conf.
							candItemset.GetAt(candItemset.GetUpperBound())->m_cpyPairCount = pNode->m_pairCount;
							candItemset.GetAt(candItemset.GetUpperBound())->m_cpySenCounts.Copy(pNode->m_senCounts);

							c.addPath(candItemset, m_nSensitiveValues, 1);						
						}
						else 
						{
							delete tempPair; 
						}
						pAncestor = pAncestor->m_pParent;
					}
				}
			}
			else
				cerr << _T(" CSTWindowTreeNode::genWinCandidates(). ") << pCurrentNonMole->GetAt(0)->toString() << _T(" is not registered in CandTable.") << endl;
		}
		else
			cerr << _T(" CSTWindowTreeNode::genWinCandidates(). ") << pCurrentNonMole->toString() << _T(" does not contain new pairs") << endl;
	}
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
bool CSTWindowTreeNode::isPrivacyViolated(int minK, double maxConf, 
										CSTAttrib* pSenAttrib, 
										int support, const CRFIntArray& totalSenCounts) const
{
    if (support < minK)	
        return true;

    CSTConcept* pSenRoot = pSenAttrib->getConceptRoot();
    for (int i = 0; i < pSenRoot->getNumChildConcepts(); ++i) {
        if (pSenRoot->getChildConcept(i)->m_bSensitive)
            if ((double) totalSenCounts[i] / support > maxConf)	
                return true;
    }
    return false;
}

//---------------------------------------------------------------------------
// Remove specific nodes from the window tree.
//---------------------------------------------------------------------------
void CSTWindowTreeNode::removeNodes(CStringArray& sWinnerPairs)
{
	if (sWinnerPairs.IsEmpty())
	{
		cerr << _T(" CSTWindowTreeNode::removeNodes(): Empty array.") << endl;
		return ;
	}

	for (int i = 0; i < sWinnerPairs.GetSize(); ++i)
	{
		CString sWinner = sWinnerPairs.GetAt(i);
		CRFWinCandInfo* pCandInfo = NULL;
		if (!m_pCandTable->Lookup(sWinner, pCandInfo)) 
		{	
			cerr << _T(" CSTWindowTreeNode::removeNodes(): winner pair ") << sWinner << _T(" is not in Candidate Table.") << endl;
			ASSERT(false);
			return;
		}

		CSTWindowTreeNode* pNode = NULL;
		CSTWindowTreeNode* pRoot = this;
		for (POSITION pos = pCandInfo->m_linkNodes.GetHeadPosition(); pos != NULL;) 
		{
			pNode = pCandInfo->m_linkNodes.GetNext(pos);
			
			// Remove itself from parent. 
			pNode->m_pParent->m_childNodes.RemoveAt(pNode->m_childPos);
			
			if (!pNode->removeNodesHelper(pNode->m_pParent, pRoot)) 
			{
				cerr << _T(" CSTWindowTreeNode::removeNodesHelper(). Failed to remove node: ") << sWinner << _T(" from window tree.") << endl;
				return;
			}

			delete pNode;
		}

		// Remove the entry.
		delete pCandInfo;
		pCandInfo = NULL;
		if (!m_pCandTable->RemoveKey(sWinner)) 
		{
			cerr << _T("CSTWindowTreeNode::removeNodes(). Failed to find: ") << sWinner << _T(" for removal.") << endl;
			ASSERT(false);
			return;
		}
	}

	// Update root's m_pairCount (number of records)
	m_pairCount = 0;
	for (POSITION childPos = m_childNodes.GetHeadPosition(); childPos != NULL;)
		m_pairCount += m_childNodes.GetNext(childPos)->m_pairCount;

	return;
}

//---------------------------------------------------------------------------
// The current node "n" is to be removed. Its child nodes will raplce it (case 1).
// If a match found, a child node of "n" will be merged with a sibling of "n" (case 2).
//---------------------------------------------------------------------------
bool CSTWindowTreeNode::removeNodesHelper(CSTWindowTreeNode* pTargetParent, CSTWindowTreeNode* pRoot)
{
	bool matchFound = false;
	bool nodeAdded = false;
	CSTWindowTreeNode* pChildNode = NULL;
	CSTWindowTreeNode* pSiblingNode = NULL;
	CSTWindowTreeNode* pNode = NULL;

    for (POSITION childPos = m_childNodes.GetHeadPosition(); childPos != NULL;) 
	{	
        pChildNode = m_childNodes.GetNext(childPos);
		for (POSITION siblingPos = pTargetParent->m_childNodes.GetHeadPosition(); siblingPos != NULL;)
		{
			POSITION currentPos = siblingPos;
			pSiblingNode = pTargetParent->m_childNodes.GetNext(siblingPos);
			matchFound = false;
			nodeAdded = false;
			if (pChildNode->m_pTime == pSiblingNode->m_pTime && pChildNode->m_pLoc == pSiblingNode->m_pLoc)
			{
				// Match found. Merge the child node with the target sibling node.				
				pSiblingNode->m_pairCount += pChildNode->m_pairCount;
				for (int q = 0; q < m_nSensitiveValues; ++q)
					pSiblingNode->m_senCounts[q] += pChildNode->m_senCounts[q];
				
				CRFWinCandInfo* pCandInfo = NULL;
				if (!pRoot->m_pCandTable->Lookup(pChildNode->toString(), pCandInfo)) 
				{	
					cerr << _T(" CSTWindowTreeNode::removeNodes(): child node ") << pChildNode->toString() << _T(" is not in Candidate Table.") << endl;
					ASSERT(false);
					return false;
				}
				pCandInfo->m_linkNodes.RemoveAt(pChildNode->m_linkPos);
				
				pChildNode->removeNodesHelper(pSiblingNode, pRoot);	// Merge.

				matchFound = true;
				nodeAdded = false;
				break;
			}
			else
			{
				if (pChildNode->m_pTime->m_timestamp < pSiblingNode->m_pTime->m_timestamp)
				{
					// A sibling node with higher timestamp found. 
					// Remove from parent, then add this child node before current sibling node.
					this->m_childNodes.RemoveAt(pChildNode->m_childPos);
					pChildNode->m_pParent = pTargetParent;
					pChildNode->m_childPos = pTargetParent->m_childNodes.InsertBefore(currentPos, pChildNode);

					matchFound = false;
					nodeAdded = true;
					break;
				}
			}
		}
		if (!matchFound && !nodeAdded)
		{
			// pChildNode has highest timestamp.
			// Remove it from its parent, then add it at the tail of target parent's child nodes.
			this->m_childNodes.RemoveAt(pChildNode->m_childPos);
			pChildNode->m_pParent = pTargetParent;
			pChildNode->m_childPos = pTargetParent->m_childNodes.AddTail(pChildNode);
		}
	}
	
	return true;
}

//---------------------------------------------------------------------------
// Count the number of instances of each unique pair in the current window.
// Historic pairs are accounted for.
// Return the total number of instances.
//---------------------------------------------------------------------------
int CSTWindowTreeNode::getTotalNumInstances()
{
	int totalNumInstances = 0;

	// Check Historic Pairs.
	totalNumInstances = m_pHistoricPairs->getNumInstances();

	if (!m_pCandTable)
	{
		cerr << _T(" CSTWindowTreeNode::getDistortion(): Candidate Table not initialized. Return 0.") << endl;
		return 0;
	}

	if (m_pCandTable->GetSize() == 0)
	{
		cerr << _T(" CSTWindowTreeNode::getDistortion(): Empty Candidate Table (Empty window). Return HistoricPairs.") << endl;
		return totalNumInstances;
	}

	for (POSITION pos = m_pCandTable->GetStartPosition(); pos != NULL;) 
	{	
		CString sPair;
		CRFWinCandInfo* pCandInfo = NULL;
		m_pCandTable->GetNextAssoc(pos, sPair, pCandInfo);	

		for (POSITION linkPos = pCandInfo->m_linkNodes.GetHeadPosition(); linkPos != NULL;) 
		{	
			CSTWindowTreeNode* pNode = pCandInfo->m_linkNodes.GetNext(linkPos);	
            ASSERT(pNode->m_pairCount > 0);
			pCandInfo->m_nInstances += pNode->m_pairCount;
			totalNumInstances += pNode->m_pairCount;
        }
	}
	return totalNumInstances;
}

//---------------------------------------------------------------------------
// winnerPairs are to be removed from current window.
// Distortion = the total number of instances of the pairs in winnerPairs and m_pHistoricPairs.
//---------------------------------------------------------------------------
int CSTWindowTreeNode::getDistortion(CStringArray& winnerPairs)
{
	int totalDistortion = 0;

	// Check Historic Pairs.
	totalDistortion = m_pHistoricPairs->getNumInstances();

	// Check current Candidate Table
	if (!m_pCandTable)
	{
		cerr << _T(" CSTWindowTreeNode::getDistortion(): Candidate Table not initialized. Return 0.") << endl;
		return 0;
	}

	if (m_pCandTable->GetSize() == 0)
	{
		cerr << _T(" CSTWindowTreeNode::getDistortion(): Empty Candidate Table (Empty window). Return HistoricPairs") << endl;
		return totalDistortion;
	}

	for (int i = 0; i < winnerPairs.GetSize(); ++i)
	{
		LPCTSTR winner = winnerPairs.GetAt(i);
		CRFWinCandInfo* pCandInfo = NULL;
		if (!m_pCandTable->Lookup(winner, pCandInfo)) 
		{	
			cerr << _T(" CSTWindowTreeNode::getDistortion(): winner pair ") << winner << _T(" is not in Candidate Table. Return -1.") << endl;
			ASSERT(false);
			return -1;
		}
		totalDistortion += pCandInfo->m_nInstances;

		// Add winner to Historic Pairs.
		// Addition is sorted in ascending order.
		m_pHistoricPairs->addHistoricPair(winner, pCandInfo->m_nInstances);
	}
	return totalDistortion;
}

//---------------------------------------------------------------------------
// Calculate the distortion from the Historic Array.
//---------------------------------------------------------------------------
int CSTWindowTreeNode::getHistoricDistortion()
{ 
	int distortion = 0;
	if (!m_pHistoricPairs->IsEmpty())
	{
		for (int i = 0; i < m_pHistoricPairs->GetSize(); ++i)
			distortion += m_pHistoricPairs->GetAt(i)->m_nInstances;
	}
	return distortion;
}


//******************
// CRFWinCandTable *
//******************

CRFWinCandTable::CRFWinCandTable()
{
}

CRFWinCandTable::~CRFWinCandTable()
{
    cleanup();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CRFWinCandTable::cleanup()
{
    CString sPair;
    CRFWinCandInfo* pCandInfo = NULL;
    for (POSITION pos = GetStartPosition(); pos != NULL;) {
        GetNextAssoc(pos, sPair, pCandInfo);	
        delete pCandInfo;
        pCandInfo = NULL;
    }
    RemoveAll();	
}


//---------------------------------------------------------------------------
// Add the node to the link.
// Increment m_nInstances.
//---------------------------------------------------------------------------
bool CRFWinCandTable::registerNode(CSTWindowTreeNode* pTargetNode)
{
    CRFWinCandInfo* pCandInfo = NULL;
    if (!Lookup(pTargetNode->toString(), pCandInfo)) {
        pCandInfo = new CRFWinCandInfo();
        SetAt(pTargetNode->toString(), pCandInfo);
    }
    pTargetNode->m_linkPos = pCandInfo->m_linkNodes.AddTail(pTargetNode);
    return true;
}

//---------------------------------------------------------------------------
// Remove the node from the link.
//---------------------------------------------------------------------------
bool CRFWinCandTable::deregisterNode(const CSTWindowTreeNode* pTargetNode)
{
    CRFWinCandInfo* pCandInfo = NULL;
    if (!Lookup(pTargetNode->toString(), pCandInfo)) {
        cerr << _T("CRFWinCandTable::deregisterNode. Failed to find: ") << pTargetNode->toString() << endl;
        ASSERT(false);
        return false;
    }

    pCandInfo->m_linkNodes.RemoveAt(pTargetNode->m_linkPos);	
    if (pCandInfo->m_linkNodes.GetSize() == 0) {
        // Remove the entry if there are no linked nodes.
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


////////////////////////////////////////////////////////////////////////////////

//*****************
// CHistoricPairs *
//*****************	

CHistoricPairs::CHistoricPairs()
{
}

CHistoricPairs::~CHistoricPairs()
{
    cleanup();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CHistoricPairs::cleanup()
{
    for (int i = 0; i < GetSize(); ++i)
        delete GetAt(i);

    RemoveAll();
}

//---------------------------------------------------------------------------
// Return the total number of instances of all historic pairs in this array.
//---------------------------------------------------------------------------
int CHistoricPairs::getNumInstances() const
{
	int nHistInstances = 0;
	if (!IsEmpty())
	{
		for (int i = 0; i < GetSize(); ++i)
			nHistInstances += GetAt(i)->m_nInstances;
	}
	return nHistInstances;
}

//---------------------------------------------------------------------------
// Add historic pairs in an ascending order, d2 b5 a6 ...
//---------------------------------------------------------------------------
bool CHistoricPairs::addHistoricPair(CString winner, int instances)
{
	bool added = false;
    CString tempLoc, tempTime;
	CSTPair::parsePair(winner, tempLoc, tempTime);
	int winnerTimestamp = int (StrToInt(tempTime));

	CHistoricPair* pHistPair = new CHistoricPair(winner, instances);
	if (IsEmpty())
		Add(pHistPair);
	
	else
	{
		for (int j = GetUpperBound(); j >= 0; --j)
		{
			if(GetAt(j)->m_timestamp < winnerTimestamp)
			{
				InsertAt(j + 1, pHistPair, 1);
				added = true;
				break;
			}
		}
		if (!added)
		{
			InsertAt(0, pHistPair, 1);
			added = true;
		}
	}
    
    return true;
}

//---------------------------------------------------------------------------
// Remove historic pairs that do not fit in curret window anymore.
// I.E., timestamp < offset.
// Three cases exist, see below.
//---------------------------------------------------------------------------
bool CHistoricPairs::removeOldHistPairs(int offset, CHistoricPairs* tempArray, bool& allOld)
{
	int i = 0;
	int arraySize = GetSize();
	for (i; i < arraySize; ++i)
	{
		if (GetAt(i)->m_timestamp >= offset)
			break;
	}

	// Case1: the entire array fits in current window. Don't do copy.
	if (i == 0)
		return true;

	// Case 2: if i == GetSize(), that means the entire array does not fit current window.
	if (i == arraySize)
	{
		allOld = true;
		return true;
	}

	// Case 3: copy values starting from index i. Perform deep copy.
	CHistoricPair* tempHistPair;
	for (; i < arraySize; ++i)
	{
		tempHistPair = new CHistoricPair(GetAt(i));
		tempArray->Add(tempHistPair);
	}
	return true;
}