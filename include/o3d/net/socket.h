/**
 * @file socket.h
 * @brief Sockets management for TCP and UDP protocols
 * @author Patrice GILBERT (patrice.gilbert@revolutining.com) 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2005-03-01
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_SOCKET_H
#define _O3D_SOCKET_H

#include <o3d/core/memorydbg.h>
#include <o3d/core/base.h>

#include <list>
#include "sockaddr.h"
#include <o3d/core/error.h>

//---------------------------------------------------------------------------------------
// Specific defines for UNIX based platforms
//---------------------------------------------------------------------------------------
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#ifndef SD_SEND
	#define SD_SEND 1
#endif

#ifndef SD_RECV
	#define SD_RECV 0
#endif

#ifdef O3D_WIN_SOCKET
	#ifndef socklen_t
	#define socklen_t Int32
	#endif
#else
	#ifndef HOSTENT
	#define HOSTENT hostent
	#endif

	#ifndef closesocket
	#define closesocket close
	#endif
#endif

namespace o3d {
namespace net {

class NetBuffer;

//---------------------------------------------------------------------------------------
//! @class Socket
//-------------------------------------------------------------------------------------
//! Base class for socket
//---------------------------------------------------------------------------------------
class O3D_NET_API Socket
{
public:

	//! Socket support initialization (WSA winsock2 for WIN32).
    static void init();
	//! Socket support terminate.
    static void quit();

	//! Create a new socket by accept method
	//! @param listener The listener socket that can accept the new socket.
	static Socket* accept(const Socket &listener);

	//! Create a new socket by connect method using an existing sockAddr.
	//! @param sockaddr Valid socked address.
	static Socket* connect(SockAddr *sockAddr);

	//! Create a new socket by connect method using the most appropriated address family.
	//! @param hostname Peer hostname.
	//! @param port Port.
	//! @param af Address family. Set AF_INET6 to force IPv6.
	//! @pamam type Socket type.
	static Socket* connect(
			const String &hostname,
			UInt16 port,
			UInt32 af = AF_UNSPEC,
			UInt32 type = SOCK_STREAM);

	//! Create a new sockAddr using given info (AF_INET,AF_INET6 | SOCK_STREAM,SOCK_DGRAM)
	Socket(UInt32 adress_family, UInt32 type);
	//! Construc using an exising sockAddr.
	Socket(SockAddr *sockAddr);
	//! Destructor
	virtual ~Socket();

	//! Destroy the socket and the sockAddr.
	void destroy();

	//! Set somes sockets options
	Bool setParameters(Int32 socket_level, Int32 option, const CString& value);

	//! Send a packet
	Int32 send(const UInt8* pData,Int32 len,Int32 option = 0);

	//! Send data from buffer
	Int32 sendFromBuffer(NetBuffer* buffer, Int32 option = 0);

	//! Receive a packet
	Int32 receive(UInt8* pData,Int32 len,Int32 option = 0);

	//! Receive data into O3DBuffer
	Int32 receiveIntoBuffer(NetBuffer* buffer, Int32 option = 0);

	//! Send a packet to a specific address
	Int32 sendTo(const SockAddr &sockaddr, const UInt8* data,Int32 len,Int32 option = 0);
	//! Receive a packet and obtain the sender address
	Int32 receiveFrom(SockAddr &sockaddr, UInt8* data, Int32 len, Int32 option = 0);

	//! close the socket
	Bool close();

	//! Shutdown the socket
	Bool shutdown(Int32 how);

	//! Perform a select and return the number of handles that are ready and
	//! contained in the fd_set timeout is in microsecond
	Int32 select(fd_set *readfds, fd_set *writefds, fd_set *exceptfds, UInt32 timeout);

	//! Perform a select and return the number of handles that are ready and
	//! @param timeout Time out in microsecond
	Int32 select(UInt32 timeout);

	//! Define the sock_addr to bind or connect
	void setSockAddr(SockAddr *sockAddr);

	//! Create the socket
	//! @throw E_SocketNotCreated if error
	void socket();

	//! Bind the socket
	//! @return True if the bind success
	Bool bind();

	//! Set the socket blocking or not
	//! @return True if success
	Bool setNonBlocking(Bool nonBlocking = True);

    //! set reading timeout (in microsecond)
	//! @Throw O3D_E_InvalidParameter exception if failed to set ReadTimeout
	void setReadTimeout(UInt32 timeout);

    //! set writing timeout (in microsecond)
	//! @Throw O3D_E_InvalidParameter exception if failed to set WriteTimeout
	void setWriteTimeout(UInt32 timeout);

	//! Put the socket in listening mode
	//! @param backlog The maximum length to which the queue of pending connections can grow.
	//! Valid range is from 1 to 5.
	//! @return True if the operation success.
	Bool listen(Int32 backlog);

	//! Set socket id
	inline void setID(_SOCKET socket_id) { m_socket_id = socket_id; }
	//! Get socket id
	inline _SOCKET getID() const { return m_socket_id; }

	//! Get the sockAddr (contains af, port, type, protocol, hostname...)
	inline const SockAddr* getSockAddr() const { return m_sockAddr; }

	//! Get the host name for this socket
	String getHostName();
	//! Get the host name for the peer connected to this socket
	String getPeerName();

	//! Get this socket address
	String getHostAddress();
	//! Get the peer address
	String getPeerAddress();

protected:

	Socket();

	_SOCKET m_socket_id;      //!< socket handle ID
	SockAddr *m_sockAddr;     //!< socket address struct
};

//! @class _E_SocketNotCreated Unable to create a new socket
class O3D_NET_API E_SocketNotCreated : public E_BaseException
{
	O3D_E_DEF_CLASS(E_SocketNotCreated)

	//! Ctor
	E_SocketNotCreated(const String& msg) : E_BaseException(msg)
		O3D_E_DEF(E_SocketNotCreated,"Unable to create a new socket")
};

//! @class _E_InvalidHostName Invalid hostname exception
class O3D_NET_API E_InvalidHostName : public E_BaseException
{
	O3D_E_DEF_CLASS(E_InvalidHostName)

	//! Ctor
	E_InvalidHostName(const String& msg) : E_BaseException(msg)
		O3D_E_DEF(E_InvalidHostName,"Unable to find the hostname")
};

//! @class _E_InvalidPeerName Invalid peername exception
class O3D_NET_API E_InvalidPeerName : public E_BaseException
{
	O3D_E_DEF_CLASS(E_InvalidPeerName)

	//! Ctor
	E_InvalidPeerName(const String& msg) : E_BaseException(msg)
		O3D_E_DEF(E_InvalidPeerName,"Unable to find the peername")
};

class O3D_NET_API E_SocketError : public E_BaseException
{
	O3D_E_DEF_CLASS(E_SocketError)

	//! Ctor
	E_SocketError(const String& msg, Int32 errorCode) : E_BaseException(msg)
		O3D_E_DEF(E_SocketError,"Socket Error")

};

} // namespace net
} // namespace o3d

#endif // _O3D_SOCKET_H
