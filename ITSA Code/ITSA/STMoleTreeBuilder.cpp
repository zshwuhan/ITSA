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

#include "stdafx.h"
#include "STMoleTreeBuilder.h"
#include <vector>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
CSTMoleTreeBuilder::CSTMoleTreeBuilder(int     minK,
                                       int     maxLen,
                                       double  maxConf,
									   int	   winSize,
									   int	   maxTimeStamp)
    : m_minK(minK),
      m_maxLen(maxLen),
      m_maxConf(maxConf),
	  m_winSize(winSize),
	  m_maxTimeStamp(maxTimeStamp),
      m_pDataMgr(NULL),
      m_pMoleTree(NULL)
{
	m_pSTWindowTree = new CSTWindowTreeNode();
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
CSTMoleTreeBuilder::~CSTMoleTreeBuilder()
{    
    delete m_pMoleTree;	
	delete m_pSTWindowTree; 

    m_pMoleTree = NULL;	
	m_pSTWindowTree = NULL;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTMoleTreeBuilder::initialize(CSTDataMgr* pDataMgr)
{
    if (!pDataMgr) {
        ASSERT(false);
        return false;
    }
    m_pDataMgr = pDataMgr;
	
    return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CSTMoleTreeBuilder::cleanupWinnersArray(CStringArray& winnerPairs)
{
	cout << "winnerPairs.GetSize(): " << winnerPairs.GetSize() << endl;
	for (int i = 0; i < winnerPairs.GetSize(); ++i) {
		cout << "winnerPairs.GetAt(" << i << "): " << winnerPairs.GetAt(i) << endl;
		delete winnerPairs.GetAt(i);
	}

    winnerPairs.RemoveAll();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTMoleTreeBuilder::buildMinimalMoleTree()
{
	std::cout << _T("--------------------------------------------------------------------") << std::endl;
	std::cout << _T("CSTMoleTreeBuilder::buildMinimalMoleTree().") << std::endl;
	std::cout << _T("Note:") << std::endl;
	std::cout << _T("Window#x means that this window starts at timestamp x.") << std::endl;
	std::cout << _T("Window#x does not necessarily reflect the window sequence number.") << std::endl;
	std::cout << _T("--------------------------------------------------------------------") << std::endl;
	cout << _T(">> Minimum Support    K: ") << m_minK << endl;
	cout << _T(">> Maximum Length     L: ") << m_maxLen << endl;
	cout << _T(">> Maximum Confidence C: ") << m_maxConf * 100 << "%" << endl;
	cout << _T(">> Window Size         : ") << m_winSize << endl;
	cout << _T(">> maxTimeStamp        : ") << m_maxTimeStamp << endl;
	cout << _T(">> Window Step Size    : ") << STEP_SIZE << endl;
	cout << _T(">> First timestamp     : ") << FIRST_WIN_STARTING_TIMESTAMP << endl << endl << endl;

	m_pSTWindowTree->setNumSenValues(m_pDataMgr->getSenAttrib()->getConceptRoot()->getNumChildConcepts());

	int firstTimestamp = FIRST_WIN_STARTING_TIMESTAMP;	// The very first timestamp. Must be >= 1.
	CSTPairsTreeNode* pCandidates = NULL;
	CSTPairsCollection minMoles;
	CSTPairsCollection* nonMoles = new CSTPairsCollection();
	CStringArray* pWinnerPairs = new CStringArray(); 
	m_pMoleTree = new CSTMoleTreeNode(); 

	long sumSlideTimes		  = 0;	// sums are for calculating avg.
	long sumSuppTimes		  = 0;
	long sumFindWinnersTimes  = 0;
	long slideTime			  = 0;	
	long suppTime			  = 0;
	long suppStartTime		  = 0;
	long findWinnersStartTime = 0;
	long findWinnersTime      = 0;
	long readTime			  = 0;
	long startWin1;
	long endWin1;
	long winStartTime;
	long winEndTime;
	long sumTotalTimes;		// Reading new data to updating tree.
	double sumDistoRatios;	// To calculate avg distortion from all windows.
	bool noMoles    = false;
	int suppCounter = 0;	// Number of windows with suppressions. Used in calculating avg supp time.
	typedef CArray<double, double>       CDistArray;
	

	std::cout << _T("Window# ") << 1 << _T(": Start of anonymization.") << std::endl;
	startWin1 = getTime();
	m_pSTWindowTree->readFirstWindowPairs(m_pDataMgr, firstTimestamp, m_winSize);	// firstTimestamp sets the starting timestamp of first window. Must be >= 1.

	int totalInstances = m_pSTWindowTree->getTotalNumInstances();
	int distortion = 0;

	// C1 is m_pCandTable
	// Finding the support and conf of every 1-itemset is achieved by following m_linkNodes.
	// Divide the 1-itemset candidates into minMoles (Critical Violations) and nonMoles.
	m_pSTWindowTree->divideC1(firstTimestamp, minMoles, *nonMoles, m_minK, m_maxConf, m_pDataMgr->getSenAttrib());

	// Window 1, Level 1: minMoles and nonMoles are ready.
	
	int level = 2;
	while (level <= m_maxLen)
	{
		pCandidates = new CSTPairsTreeNode();
		if (nonMoles->IsEmpty()) {
			cout << "Window1, empty nonMoles at level " << level << endl;
			delete pCandidates;
			pCandidates = NULL;
			break;
		}

		genCandidates(*pCandidates, minMoles, *nonMoles);
		nonMoles->cleanup();
		delete nonMoles;
		nonMoles = NULL;
		nonMoles = new CSTPairsCollection();
		if (!pCandidates->isEmpty()) 
		{
			countSupports(*pCandidates);
			pCandidates->filterMoles(minMoles, *nonMoles, m_minK, m_maxConf, m_pDataMgr->getSenAttrib());
		}
		else
		{
			delete pCandidates;
			pCandidates = NULL;
			break;
		}
		delete pCandidates;
		pCandidates = NULL;
		++level;
	}

	delete nonMoles;
	nonMoles = NULL;

	if (minMoles.GetSize() == 0) 
	{
		std::cout << _T("Window# ") << 1 << _T(": No minimal moles.") << std::endl << std::endl; 
		noMoles = true; 
		endWin1 = getTime();
	}
	else
	{
		for (int i = 0; i < minMoles.GetSize(); ++i) 
			m_pMoleTree->addMole(*minMoles.GetAt(i));

		// Find all winner pairs, then remove them from current window tree.
		m_anonymizer.cleanMoles(m_pMoleTree, *pWinnerPairs, m_pSTWindowTree->getCandTable());
		distortion = m_pSTWindowTree->getDistortion(*pWinnerPairs);
		suppStartTime = getTime();
		m_pSTWindowTree->removeNodes(*pWinnerPairs);
		endWin1 = suppTime = getTime();
	}

	if (!noMoles)  {
	cout << _T("Spent: ") << suppTime - suppStartTime << _T("s on suppression (only removing nodes).") << endl; 
	sumSuppTimes = sumSuppTimes + (suppTime - suppStartTime); 
	suppCounter++; }
	cout << _T("Spent: ") << endWin1 - startWin1 << _T("s on window1 (reading data --> masking).") << endl;
	cout << _T("Total number of instances = ") << totalInstances << endl;
	cout << _T("Total distortion = ") << distortion << endl;
	cout << _T("Distortion ratio = ") << (totalInstances > 0 ? (((double) distortion / totalInstances) * 100) : 0) << "%." << endl;
	std::cout << _T("Window# ") << 1 << _T(": End of anonymization.\n") << std::endl;
	
	sumTotalTimes = endWin1 - startWin1;
	sumDistoRatios = (totalInstances > 0 ? (((double) distortion / totalInstances) * 100) : 0);
	

	//										>>>>>>> END OF FIRST WINDOW ANONYMIZATION <<<<<<<
	// 1. Slide window: Read new pairs into window tree, remove old pairs, and update historic pairs.
	// 2. while (l < m_maxLen && !pCandidates.isEmpty()) { Generate candSet[l]: only l-itemsets with new pairs. Generate CVs: store winner pairs. }
	// 3. Update window tree by removing the winner pairs.
	// 4. Repeat Step 1.

	int i;
	int numOfAllWinds = m_maxTimeStamp - m_winSize + 1;
	for (i = firstTimestamp + STEP_SIZE; i <= numOfAllWinds; i += STEP_SIZE)
	{
		delete pWinnerPairs;
		pWinnerPairs = new CStringArray();
		delete m_pMoleTree;
		m_pMoleTree = new CSTMoleTreeNode();
		CSTPairsCollection minMoles;
		CSTPairsCollection* nonMoles = new CSTPairsCollection();
		distortion = 0;
		noMoles = false;
		CSTPairsTreeNode* pCandidates = NULL;


		std::cout << _T("\n=================== Sliding the window ==================") << std::endl;
		winStartTime = getTime();
		if (!m_pSTWindowTree->slideWindow(m_pDataMgr, STEP_SIZE, i, m_winSize, readTime))
		{
			cerr << _T(" Window#") << i << _T(": Error in slideWindow().") << endl;
			return false;
		}

		findWinnersStartTime = getTime();
		totalInstances = m_pSTWindowTree->getTotalNumInstances();

		// Consider new pairs only.
		m_pSTWindowTree->divideC1(i, minMoles, *nonMoles, m_minK, m_maxConf, m_pDataMgr->getSenAttrib());

		level = 2;
		while (level <= m_maxLen)
		{
			pCandidates = new CSTPairsTreeNode();
			// For every previous nonMole (of size "level - 1") that contains a new pair, go to the branch where that itemset belongs 
			// and generate all possible level-itemsets by adding a non-existing pair to the current itemset,
			// and adding the newly generated candidate itemset to a PairsTreeNode tree.

			// Add the level-itemset to pCandidates starting with the new pair.
			// As itemsets are added to pCandidates, the supp and conf of unique itemsets are updated and stored in the leaf nodes.
			// When done generating candidate itemsets and adding them to pCandidates, supp and conf of all level-itemset candidates 
			// that include new pairs are stored in the leaf nodes.

			if (nonMoles->IsEmpty())
				break;

			m_pSTWindowTree->genWinCandidates(*pCandidates, minMoles, *nonMoles);

			nonMoles->cleanup();
			delete nonMoles;
			nonMoles = NULL;
			nonMoles = new CSTPairsCollection();
		
			if (!pCandidates->isEmpty()) 
			{
				// Go through the leaves, create a path (an itemset), and decide if the path is a minMole or a nonMole.
				// 1 means it is a subsequent window.
				pCandidates->filterMoles(minMoles, *nonMoles, m_minK, m_maxConf, m_pDataMgr->getSenAttrib(), 1);
			}
			else
			{
				delete pCandidates;
				break;
			}

			delete pCandidates;
			pCandidates = NULL;
			++level;

		}// End while

		if (minMoles.GetSize() == 0)
		{
			std::cout << _T(" Window#") << i << _T(": No critical violations.") << std::endl << std::endl;
			noMoles = true;
			distortion = m_pSTWindowTree->getHistoricDistortion();
			winEndTime = findWinnersTime = getTime();
		}
		else
		{
			// Build the minimal mole tree from minMoles.
			for (int m = 0; m < minMoles.GetSize(); ++m) 
			{
				if (!m_pMoleTree->addMole(*minMoles.GetAt(m)))
					return false;
			}  
			
			// Should read CVT and return back pairs to be suppressed from window (these are winner pairs).
			if (!m_anonymizer.cleanMoles(m_pMoleTree, *pWinnerPairs, m_pSTWindowTree->getCandTable()))
				return false;
		
			distortion = m_pSTWindowTree->getDistortion(*pWinnerPairs);
			findWinnersTime = getTime();
			
			// Remove winningPairs from STWindowTree.
			// Kill nodes: Deregister and remove from tree.
			m_pSTWindowTree->removeNodes(*pWinnerPairs);
			winEndTime = suppTime = getTime();
		}
	

		cout << _T(" Spent: ") << readTime << _T("s on sliding (updating the tree).") << endl;
		cout << _T(" Spent: ") << findWinnersTime - findWinnersStartTime << _T("s on finding winner pairs.") << endl;	// time includes historic pairs too.
		if (!noMoles)  {
		cout << _T(" Spent: ") << suppTime - findWinnersTime << _T("s on suppression (updating the tree).") << endl; 
		sumSuppTimes = sumSuppTimes + (suppTime - findWinnersTime);
		suppCounter++; }
		cout << _T(" Spent: ") << (winEndTime - findWinnersStartTime) + readTime << _T("s in total.") << endl;
		cout << _T(" Total number of instances = ") << totalInstances << " (Historic pairs included)." << endl;
		cout << _T(" Total distortion = ") << distortion << " (Historic pairs included)." << endl;
		cout << _T(" Distortion ratio = ") << (totalInstances > 0 ? (((double) distortion / totalInstances) * 100) : 0) << "%." << endl;
		cout << _T("========== End of current window anonymization ==========\n\n") << endl;
		
		sumSlideTimes       = sumSlideTimes       + readTime;
		sumFindWinnersTimes = sumFindWinnersTimes + (findWinnersTime - findWinnersStartTime);
		sumTotalTimes       = sumTotalTimes       + ((winEndTime - findWinnersStartTime) + readTime);
		sumDistoRatios = sumDistoRatios + (totalInstances > 0 ? (((double) distortion / totalInstances) * 100) : 0);

		readTime = 0;

		minMoles.cleanup();
	}// End for. Ideally, infinit.

	std::cout << _T("SUMMARY:\n") << std::endl;
	std::cout << _T("Number of windows: ") << numOfAllWinds << std::endl;
	std::cout << _T("Average sliding time: ") << (double)sumSlideTimes / (numOfAllWinds - 1) << _T(" s (reading new data).") << std::endl; // No sliding for last window.
	std::cout << _T("Average finding winner pairs time: ") << (double)sumFindWinnersTimes / (numOfAllWinds - 1) << _T(" s.") << std::endl; // First window not included.
	if (suppCounter > 0) {
	std::cout << _T("Average suppression time: ") << (double)sumSuppTimes / suppCounter << _T(" s (updating tree).") << std::endl;	/* Windows with suppressions only.*/ } 
	else { std::cout << _T("Average suppression time: no suppressions were performed.") << std::endl; }
	std::cout << _T("Average total time: ") << (double)sumTotalTimes / (numOfAllWinds) << _T(" s.") << std::endl;
	std::cout << _T("Average distortion ratio: ") << sumDistoRatios / (numOfAllWinds) << " %." << std::endl;

	delete pWinnerPairs;
	return true;
}


//---------------------------------------------------------------------------
// Generate candidates from nonMoles.
// Join every pair of nonMoles if they share the same prefix.
// A candidate cannot be superset of any moles.
//---------------------------------------------------------------------------
bool CSTMoleTreeBuilder::genCandidates(CSTPairsTreeNode& candidates,
                                       const CSTPairsCollection& minMoles,
                                       const CSTPairsCollection& nonMoles)
{
    CSTPairs* pNonMole1 = NULL;
    CSTPairs* pNonMole2 = NULL;
    CSTPairs* pMole = NULL;
    const int nIntervals = 10;
    int currStep = 1;
    cout << _T(">> Progress:");
    for (int i = 0; i < nonMoles.GetSize(); ++i) {
        if (i > (double(nonMoles.GetSize()) / nIntervals) * currStep) {
            cout << _T(" ") << int(double(i) / nonMoles.GetSize() * 100) << _T("%");
            cout.flush();
            ++currStep;
        }

        pNonMole1 = nonMoles.GetAt(i);
        for (int j = i + 1; j < nonMoles.GetSize(); ++j) {
            pNonMole2 = nonMoles.GetAt(j);
            ASSERT(pNonMole1->GetSize() == pNonMole2->GetSize());

            // Check prefix, and last timestamp1 < last timestamp2
            if (pNonMole1->equalPrefix(*pNonMole2) && 
                pNonMole1->GetAt(pNonMole1->GetUpperBound())->m_pTime->m_timestamp < pNonMole2->GetAt(pNonMole2->GetUpperBound())->m_pTime->m_timestamp) {
                // Join them.
                CSTPairs tempPath;
                tempPath.Append(*pNonMole1); 
                tempPath.Add(pNonMole2->GetAt(pNonMole2->GetUpperBound())); 

       
                bool bSuperset = false;
                for (int x = (int) minMoles.GetUpperBound(); x >= 0; --x) {
                    pMole = minMoles.GetAt(x);
                    if (tempPath.GetSize() - pMole->GetSize() > 1) 
                        break; 
																		
                    if (pMole->isSuperseq(tempPath)) {
                        bSuperset = true;
                        break;
                    }
                }

                // Add to the candidates.
                if (!bSuperset) {
                    if (!candidates.addPath(tempPath, m_pDataMgr->getSenAttrib()->getConceptRoot()->getNumChildConcepts())) {
                        cerr << _T("CSTMoleTreeBuilder:genCandidates Failed to add path: ") << tempPath.toString() << endl;
                        ASSERT(false);
                        return false;
                    }
                }
            }
        }
    }
    cout << _T(" 100%") << endl;
    cout.flush();
    return true;
}

//---------------------------------------------------------------------------
// Count the support of every candidate.
//---------------------------------------------------------------------------
bool CSTMoleTreeBuilder::countSupports(CSTPairsTreeNode& candidates)
{
    CSTRecords* pRecs = m_pDataMgr->getRecords();
    for (int r = 0; r < pRecs->GetSize(); ++r) {
        if (!candidates.incrementCount(*(pRecs->GetAt(r)->getPath()), pRecs->GetAt(r)->getSensitiveConcept()->m_childIdx)) {
            cerr << _T("CSTMoleTreeBuilder:countSupports Failed to increment count: ") << pRecs->GetAt(r)->toString() << endl;
            ASSERT(false);
            return false;
        }
    }
    return true;
}


//---------------------------------------------------------------------------
// Print out the current candidate set (denoted by 'level') from its minMoles and nonMoles.
// Assumption: one sensitive value: HIV that exists in index 0 in the array of sensitive counts.
// Supp and conf that are printed out work for windows > 1.
// You may use it in window 1, but only to print out itemsets.
//---------------------------------------------------------------------------
void CSTMoleTreeBuilder::printCandSet(int level, const CSTPairsCollection& minMoles, const CSTPairsCollection& nonMoles)
{
	if (minMoles.IsEmpty() && nonMoles.IsEmpty())
	{
		cout << "CSTMoleTreeBuilder::printCandSet(). Candidate Set " << level << " is empty." << endl;
		return;
	}

	cout << "Candidate set " << level << " is:" << endl;
	cout << "~nonMoles~" << endl;
	for (int r = 0; r < nonMoles.GetSize(); ++r) {
		int lastIdx = nonMoles.GetAt(r)->GetUpperBound();
		cout << nonMoles.GetAt(r)->toString() << " supp: "
			 << nonMoles.GetAt(r)->GetAt(lastIdx)->m_cpyPairCount 
			 << " conf: "
			 << ((double) nonMoles.GetAt(r)->GetAt(lastIdx)->m_cpySenCounts[0] / nonMoles.GetAt(r)->GetAt(lastIdx)->m_cpyPairCount) * 100 << " %"
			 << endl;
	}

	cout << "~minMoles~" << endl;
	for (int r = minMoles.GetUpperBound(); r >= 0; --r) {
		if (minMoles.GetAt(r)->GetSize() == level)
		{
			int lastIdx = minMoles.GetAt(r)->GetUpperBound();
			cout << minMoles.GetAt(r)->toString() << " supp: "
				 << minMoles.GetAt(r)->GetAt(lastIdx)->m_cpyPairCount 
				 << " conf: "
				 << ((double) minMoles.GetAt(r)->GetAt(lastIdx)->m_cpySenCounts[0] / minMoles.GetAt(r)->GetAt(lastIdx)->m_cpyPairCount) * 100 << " %"
				 << endl;
		}
	}
	cout << endl;
}