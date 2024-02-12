// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include <SDKDDKVer.h>          // Defines the highest available Windows platform.

#ifndef WINVER                  // Specifies that the minimum required platform is Windows Vista.
#define WINVER 0x0600           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 98.
#define _WIN32_WINDOWS 0x0410   // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE               // Specifies that the minimum required platform is Internet Explorer 7.0.
#define _WIN32_IE 0x0700        // Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>       // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>         // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxdb.h>          // MFC ODBC database classes

// TODO: reference additional headers your program requires here
#include <afxmt.h>
#include <winsock2.h>
#include <process.h>        // for THREAD use

#include <cmath>
#include <cstdlib>
#include <cassert>
#include <fstream>
#include <sstream>

#include <map>
#include <deque>
#include <vector>
#include <string>
#include <algorithm>

#include "ALVEnumeration.h"
#include "CraneEnumeration.h"

#include "SimulationSpec.h"
#include "CraneSpec.h"

// Custom Assert Function
#define Assert(exp, description) assert((exp) && (description));

#define COLOR_CEMENT           RGB(230, 230, 230)
#define COLOR_BLACK            RGB(0, 0, 0)
#define COLOR_ATC              RGB(21, 60, 255)
#define COLOR_QC               RGB(255, 50, 50)

#define COLOR_CONTAINER40FTL   RGB(0, 142, 64)
#define COLOR_CONTAINER40FTD   RGB(246, 79, 4)
#define COLOR_CONTAINER20FTL   RGB(255, 204, 0)
#define COLOR_CONTAINER20FTD   RGB(85, 142, 213)

#define COLOR_CONTAINER_HIGH   RGB(246, 149, 4)
#define COLOR_CONTAINER_LOW    RGB(246, 79, 4)

// Actual definitions are in Experiments.cpp
extern CSimulationSpec g_SimulationSpec;
extern CCraneSpec g_CraneSpec;
