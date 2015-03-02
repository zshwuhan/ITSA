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
#include "STMoleTreeBuilder.h"

class CSTController  
{
public:
	CSTController(LPCTSTR rawPathFile,
                  LPCTSTR anonyPathFile, 
                  LPCTSTR configFile, 
                  LPCTSTR moleFile,
                  int minK, 
                  int maxLen, 
                  double maxConf,
				  int winSize,
				  int maxTimeStamp);
    virtual ~CSTController();

// operations
    bool run();
    
protected:
// attributes
	int m_nWinSize;
    // managers
    CSTDataMgr m_dataMgr;
    CSTMoleTreeBuilder m_moleTreeBuilder;
};
