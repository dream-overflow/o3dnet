/**
 * @file client.h
 * @brief Sockets client management for connected protocols.
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2005-03-04
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_CLIENT_H
#define _O3D_CLIENT_H

#include <o3d/core/memorydbg.h>
#include <o3d/core/base.h>

#include "socket.h"

namespace o3d {
namespace net {

//---------------------------------------------------------------------------------------
//! @class Client
//-------------------------------------------------------------------------------------
//! Sockets client management for connected protocols.
//---------------------------------------------------------------------------------------
class O3D_NET_API Client : public Socket
{
public:

	//! constructor (AF_INET,AF_INET6)
	Client(UInt32 adress_family = AF_INET ,UInt32 type = SOCK_STREAM);

	//! create the new socket, hostname: xxx.xxx.xxx.xxx in ipv4 or domain name
	Bool create(const String& hostname,UInt16 port);

	//! connect the socket to the hostname
	Bool connect();
};

//---------------------------------------------------------------------------------------
//! @class ClientTCP
//-------------------------------------------------------------------------------------
//! Sockets client management for the connected TCP protocol.
//---------------------------------------------------------------------------------------
class O3D_NET_API ClientTCP : public Client
{
public:

	//! constructor (AF_INET,AF_INET6)
	ClientTCP(UInt32 adress_family = AF_INET):
	  Client(adress_family, SOCK_STREAM) {}
};

} // namespace net
} // namespace o3d

#endif // _O3D_CLIENT_H
