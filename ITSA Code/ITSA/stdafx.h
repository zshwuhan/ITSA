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

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#include <iostream>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxtempl.h>
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <conio.h>
#include <math.h>
#include <limits.h>
#include <float.h>

using namespace std;

// reference additional headers your program requires here
#define _BFLIB_ENABLE_64BIT
#if !defined(TYPES_HPP)
    #include "types.hpp"
#endif

#if !defined(BFFILEHELPER_H)
    #include "BFFileHelper.h"
#endif

#if !defined(BFSTRHELPER_H)
	#include "BFStrHelper.h"
#endif

#if !defined(BFSTRPSER_H)
	#include "BFStrPser.h"
#endif

#if !defined(BFMATH_H)
    #include "BFMath.h"
#endif

#include "STDef.h"
#include "STMain.h"
