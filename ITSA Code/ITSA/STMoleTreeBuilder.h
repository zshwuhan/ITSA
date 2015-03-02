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

#include "STDataMgr.h"
#include "STPairsTree.h"
#include "STMoleTree.h"
#include "STWindowTree.h"
#include "STAnonymizer.h"

class CSTMoleTreeBuilder
{
public:
	CSTMoleTreeBuilder(int     minK,
                       int     maxLen,
                       double  maxConf,
					   int	   winSize,
					   int	   maxTimeStamp);
    virtual ~CSTMoleTreeBuilder();

    bool initialize(CSTDataMgr* pDataMgr);
	void cleanupWinnersArray(CStringArray& winnerPairs);
    inline CSTMoleTreeNode* getMoleTree() const { return m_pMoleTree; };
    bool buildMinimalMoleTree();
	void printCandSet(int level, const CSTPairsCollection& minMoles, const CSTPairsCollection& nonMoles);
    
protected:
    bool computeMinimalMoles(CSTPairsCollection& minMoles);
    bool genC1(CSTPairsTreeNode& c1);
    bool genCandidates(CSTPairsTreeNode& candidates,
                       const CSTPairsCollection& minMoles,
                       const CSTPairsCollection& nonMoles);
    bool countSupports(CSTPairsTreeNode& candidates);    
    bool countInstances();    


// attributes
    CSTDataMgr* m_pDataMgr;

    int m_minK;
    int m_maxLen;
    double m_maxConf;
	int m_winSize;
	int m_maxTimeStamp;	// Max timestamp in dataset. Used to calculate No. of windows.

    CSTMoleTreeNode* m_pMoleTree;		// Root of the mole tree.
	CSTWindowTreeNode* m_pSTWindowTree;	// Root of the sliding window tree.
	CSTAnonymizer m_anonymizer;
};
