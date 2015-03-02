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
#include "STController.h"

//---------------------------------------------------------------------------
// Construction/Destruction
//---------------------------------------------------------------------------

CSTController::CSTController(LPCTSTR rawPathFile,
                             LPCTSTR anonyPathFile, 
                             LPCTSTR configFile, 
                             LPCTSTR moleFile,
                             int minK, 
                             int maxLen, 
                             double maxConf,
							 int winSize,
							 int maxTimeStamp)
    : m_dataMgr(rawPathFile, anonyPathFile, configFile, moleFile),
      m_moleTreeBuilder(minK, maxLen, maxConf, winSize, maxTimeStamp),
	  m_nWinSize(winSize)
{
    if (!m_dataMgr.initialize())
        ASSERT(false);
    if (!m_moleTreeBuilder.initialize(&m_dataMgr))
        ASSERT(false);
}

CSTController::~CSTController()
{
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool CSTController::run()
{	
    std::cout << _T("********************************************") << std::endl;
    std::cout << _T("* Incremental Trajectory Stream Anonymizer *") << std::endl;
    std::cout << _T("********************************************") << std::endl;

    // Read in configuration file.
    if (!m_dataMgr.readConfigFile())
        return false;

    // Read in raw data.
    if (!m_dataMgr.readRawData())
        return false;


	// Build the mole tree.
	long startBuildMoleTreeTime = getTime();    
	if (!m_moleTreeBuilder.buildMinimalMoleTree())
		return false;    

	return true; 
}
