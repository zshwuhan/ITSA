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

// STMain.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "STMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "STController.h"

// The one and only application object

CWinApp theApp;

using namespace std;

//----------------------------------------------------------------------------------
// Expected input: ../exp/dataSetName minK maxLength maxConf winSize maxTimeStamp
//----------------------------------------------------------------------------------
bool parseArgs(int      nArgs, 
			   TCHAR*   argv[], 
               CString& dataSetName,
			   int&		minK,
			   int&     maxLength, 
			   double&  maxConf,
			   int&		winSize,
			   int& maxTimeStamp)
{
    if (nArgs == 7) {
        dataSetName = argv[1];        
        minK = int(StrToInt(argv[2]));
        if (maxLength == -1)
            maxLength = INT_MAX;
        maxLength = int(StrToInt(argv[3]));
        maxConf = strtod(argv[4], NULL);
		winSize = int(StrToInt(argv[5]));
		if (winSize < maxLength) {
			std::cout << _T("maxLength should be less than or equal to winSize") << std::endl;
			return false;
		}
		maxTimeStamp = int(StrToInt(argv[6]));
		if (maxTimeStamp < 1) {
			std::cout<< _T("maxTimeStamp should be greater than or equal to 1") << std::endl;
			return false;
		}

        return true;
    }
    else {
        std::cout << _T("Usage: ITSA <dataSetName> <minK> <maxLength> <maxConf> <winSize> <maxTimeStamp>") << std::endl;
        return false;
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
        CString dataSetName;
        int minK = 0, maxLength = 0, winSize = 0;
		int maxTimeStamp = 0;	// Max timestamp in dataset. Used to calculate No. of windows.
        double maxSup = 0.0;
        if (!parseArgs(argc, argv, dataSetName, minK, maxLength, maxSup, winSize, maxTimeStamp)) {
		    std::cout << _T("Input Error: invalid arguments") << std::endl;
		    return 1;
        } 

        // Construct the filenames
        CString inPathFile, outPathFile, configFile, moleFile;
        inPathFile = dataSetName;
        inPathFile += _T(".");
        inPathFile += RF_RAW_PATHFILE_EXT;
        outPathFile = dataSetName;
        outPathFile += _T(".");
        outPathFile += RF_ANONY_PATHFILE_EXT;
        configFile = dataSetName;
        configFile += _T(".");
        configFile += RF_CONFIGFILE_EXT;
        moleFile = dataSetName;
        moleFile += _T(".");
        moleFile += RF_MOLEFILE_EXT;
        CSTController controller(inPathFile, outPathFile, 
                                 configFile, moleFile,
                                 minK, maxLength, 
                                 maxSup, winSize,
								 maxTimeStamp);
        if (!controller.run()) {
            std::cout << _T("Error occured.") << std::endl;
            return 1;
        }
	}
    std::cout << _T("Bye!") << std::endl;
#ifdef _DEBUG
    _getch();
#endif
	return nRetCode;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void printTime()
{
    time_t ltime;
    time(&ltime);
    std::cout << _T("Current System Time = ") << (long) ltime << std::endl;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
long getTime()
{
    time_t ltime;
    time(&ltime);
    return (long) ltime;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
long get_runtime(void)
{
    clock_t start;
    start = clock();
    return((long)((double)start*100.0/(double)CLOCKS_PER_SEC));
}