/**
 * @file net.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_NET_H
#define _O3D_NET_H

#include <objective3dconfig.h>

// If no object export/import mode defined suppose IMPORT
#if !defined(O3D_NET_EXPORT_DLL) && !defined(O3D_NET_STATIC_LIB)
    #ifndef O3D_NET_IMPORT_DLL
        #define O3D_NET_IMPORT_DLL
    #endif
#endif

//---------------------------------------------------------------------------------------
// API define depend on OS and dynamic library exporting type
//---------------------------------------------------------------------------------------

#if (defined(O3D_UNIX) || defined(O3D_MACOSX) || defined(SWIG))
    #define O3D_NET_API
    #define O3D_NET_API_TEMPLATE
#elif defined(O3D_WINDOWS)
    // export DLL
    #ifdef O3D_NET_EXPORT_DLL
        #define O3D_NET_API __declspec(dllexport)
        #define O3D_NET_API_TEMPLATE
    #endif
    // import DLL
    #ifdef O3D_NET_IMPORT_DLL
        #define O3D_NET_API __declspec(dllimport)
        #define O3D_NET_API_TEMPLATE
    #endif
    // static (no DLL)
    #ifdef O3D_NET_STATIC_LIB
        #define O3D_NET_API
        #define O3D_NET_API_TEMPLATE
    #endif
#endif

#endif // _O3D_NET_H
