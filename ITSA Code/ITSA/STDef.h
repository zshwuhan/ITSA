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


// pre-processor
#ifdef _DEBUG
    #define _DEBUG_PRT_INFO
#endif

#define DEBUGPrint _tprintf       // print in console

#define P_ASSERT(p) if (!p) { ASSERT(false); return false; }

// Common types
typedef CArray<int, int>       CRFIntArray;
typedef CArray<POSITION, POSITION>	CPosArray;
typedef CArray<Int64u, Int64u> CRFInt64uArray;
typedef CArray<bool, bool>     CRFBoolArray;



// Constants
#define RF_RAW_PATHFILE_EXT                 _T("txt")
#define RF_ANONY_PATHFILE_EXT               _T("aym")
#define RF_CONFIGFILE_EXT                   _T("cfg")
#define RF_MOLEFILE_EXT                     _T("mol")

#define RF_CONHCHY_OPENTAG                  TCHAR('{')
#define RF_CONHCHY_CLOSETAG                 TCHAR('}')
#define RF_CONHCHY_COMMENT                  TCHAR('|')
#define RF_CONHCHY_SENSITIVE                TCHAR('*')
#define RF_PATHDATA_COMMENT                 TCHAR('|')
#define RF_PATHDATA_SVAL_SEPARATOR          TCHAR(':')
#define RF_PATHDATA_VAL_DELIMETER           TCHAR(',')
#define RF_PATHDATA_LOCTIME_SEPARATOR       TCHAR('.')
#define RF_PATHDATA_TIME_DELIMETER          TCHAR('T')

#define RF_LOCATION_ATTRIB                  _T("location")
#define RF_TIME_ATTRIB                      _T("time")
#define RF_SENSITIVE_ATTRIB                 _T("sensitive")

#define STEP_SIZE							1
#define FIRST_WIN_STARTING_TIMESTAMP		1
