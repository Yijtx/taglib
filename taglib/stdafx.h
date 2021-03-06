// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define  _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <shellapi.h>
#include <ShlObj.h>


// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>

// C++ RunTime Header Files
#include <iostream>


// CString
#include <atlstr.h>


// BUFSIZE
#define BUFSIZE 4096


// WinFile
#include "WinFile.h"


// WinSTL
#include <winstl/conversion/char_conversions.hpp>
#include <winstl/error/error_desc.hpp>


//stlsoft
#include <comstl/conversion/interface_cast.hpp> // for comstl::interface_cast (step 2.1)
#include <comstl/util/guid.hpp>                 // for comstl::guid (step 2)
#include <comstl/util/creation_functions.hpp>   // for comstl::co_create_instance (step 2)
#include <comstl/util/initialisers.hpp>         // for comstl::com_init (step 1)
#include <comstl/util/variant.hpp>              // for comstl::variant (step 2.2)
#include <stlsoft/smartptr/ref_ptr.hpp>         // for stlsoft::ref_ptr (steps 2, 2.1 and 2.3)

//glog
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <glog/raw_logging.h>


//VOLE
#include <vole/vole.hpp>