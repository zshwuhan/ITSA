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

#include "STMoleTree.h"
#include "STWindowTree.h"

class CSTAnonymizer
{
public:
	CSTAnonymizer();
    virtual ~CSTAnonymizer();
    bool cleanMoles(CSTMoleTreeNode* pMoleTree, CStringArray& winnerPairs, CRFWinCandTable* pWinTable);    
    inline int getTotalDistortions() const { return m_nTotalDistortions; };

protected:
    int m_nTotalDistortions;
};
