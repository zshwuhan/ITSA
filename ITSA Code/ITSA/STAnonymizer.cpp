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
#include "STAnonymizer.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
CSTAnonymizer::CSTAnonymizer()
    : m_nTotalDistortions(0)
{
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
CSTAnonymizer::~CSTAnonymizer()
{    
}


//---------------------------------------------------------------------------
// Suppressing/Removing pairs from the Critical Violation Tree.
//---------------------------------------------------------------------------
bool CSTAnonymizer::cleanMoles(CSTMoleTreeNode* pMoleTree, CStringArray& winnerPairs, CRFWinCandTable* pWinTable)
{    
    if (!pMoleTree)
        return true;    // no minimal moles
    CRFCandTable* pCandTable = pMoleTree->getCandTable();

    std::cout << _T("\n *****\n Cleaning moles.") << std::endl;

    // Repeat until all moles in mole tree are removed.
    m_nTotalDistortions = 0;
    while (!pMoleTree->isEmpty()) {
        // Find the pair that has the highest score.
        CString winnerPair;        
		if (!pCandTable->pickWinner(winnerPair, pWinTable)) {
            cerr << _T(" CSTAnonymizer::killMoles failed to find winner.") << endl;
            ASSERT(false);
            return false;
        }

		winnerPairs.Add(winnerPair);

        // Remove the pair from the mole tree.
        int nDistortions = 0;        
        if (!pCandTable->suppress(winnerPair, nDistortions)) {
            cerr << _T(" CSTAnonymizer::killMoles failed to suppress winner: ") << winnerPair << endl;
            ASSERT(false);
            return false;
        }

        // Compute distortions (number of suppressed instances)
        m_nTotalDistortions += nDistortions;
    }
    if (pCandTable->GetSize() != 0) {
        cerr << _T(" CSTAnonymizer::killMoles candidate table should be empty, but it still has ") << pCandTable->GetSize() << _T(" entries.") << endl;
        ASSERT(false);
        return false;
    }
    std::cout << _T(" Cleaning moles succeeded.\n *****\n\n");
    return true;
}
