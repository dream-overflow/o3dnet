/**
 * @file client.cpp
 * @brief Implementation of Client.h
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2005-03-04
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/precompiled.h"

#include "o3d/core/architecture.h"
#include "o3d/net/client.h"
#include <o3d/core/debug.h>

using namespace o3d;
using namespace o3d::net;

//---------------------------------------------------------------------------------------
// class Client
//-------------------------------------------------------------------------------------
// constructor
//---------------------------------------------------------------------------------------
Client::Client(UInt32 adress_family, UInt32 type) :
	Socket(adress_family, type)
{
}

//---------------------------------------------------------------------------------------
// create the new socket
//---------------------------------------------------------------------------------------
Bool Client::create(const String& hostname, UInt16 port)
{
	// check the hostname format and convert it in IP adress
	if (m_sockAddr->isAny())
		O3D_ERROR(E_InvalidParameter("Hostname can not be any"));

	if (m_sockAddr->getType() == SOCK_STREAM)
		m_sockAddr->setTcp(hostname, port);
	else if (m_sockAddr->getType() == SOCK_DGRAM)
		m_sockAddr->setUdp(hostname, port);
	else
		return False;

	// create the new socket
	socket();

	return True;
}

//---------------------------------------------------------------------------------------
// connect the socket to the hostname
//---------------------------------------------------------------------------------------
Bool Client::connect()
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
		Int32 src_adr_len = m_sockAddr->getSizeOf();
		if (::connect(m_socket_id, m_sockAddr->getSockAddr(), src_adr_len) == SOCKET_ERROR)
			return False;
		return True;
	}
	return False;
}
