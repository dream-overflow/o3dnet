/**
 * @file socketwin32.cpp
 * @brief Implementation of Socket.h for WIN32 part.
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2005-03-01
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/precompiled.h"

#include "o3d/net/socket.h"

#ifdef O3D_WIN_SOCKET

#include <o3d/core/debug.h>
#include <o3d/core/application.h>

using namespace o3d;
using namespace o3d::net;

static Bool ms_socketLibState = False;
static UInt32 ms_socketLibRefCount = 0;

// socket initialization (WSA winsock2 for WIN32)
void Socket::init()
{
    if (!ms_socketLibState)
    {
        WSADATA wsaData;
        WORD wVersionRequested = MAKEWORD(2,2);

        if (WSAStartup(wVersionRequested,&wsaData) != 0)
            O3D_ERROR(E_InvalidResult("Unable to initialize WinSock 2"));

        if ((LOBYTE(wsaData.wVersion) != 2) || (HIBYTE(wsaData.wVersion) != 2))
        {
            WSACleanup();
            O3D_ERROR(E_InvalidResult("Invalid WinSock version (need 2.2)"));
        }

        Application::registerObject("o3d::Socket", nullptr);
        ms_socketLibState = True;
    }
}

void Socket::quit()
{
    if (ms_socketLibState)
    {
        if (ms_socketLibRefCount != 0)
            O3D_ERROR(E_InvalidOperation("Trying to quit winSock library but some socket still exists"));

        WSACleanup();

        Application::unregisterObject("o3d::Socket");
        ms_socketLibState = False;
    }
}

#endif // O3D_WIN_SOCKET

