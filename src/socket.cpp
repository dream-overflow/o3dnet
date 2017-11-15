/**
 * @file socket.cpp
 * @brief Implementation of Socket.h
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2005-03-01
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/precompiled.h"

#include "o3d/net/socket.h"
#include "o3d/net/netbuffer.h"
#include <o3d/core/debug.h>
#include <o3d/core/application.h>

#include <errno.h>

using namespace o3d;
using namespace o3d::net;

// POSIX socket initialization, nothing to do at all
#ifndef O3D_WIN_SOCKET

#ifndef O3D_VC_COMPILER
#include <fcntl.h>
#endif

static Bool ms_socketLibState = False;
static UInt32 ms_socketLibRefCount = 0;

//---------------------------------------------------------------------------------------
//Socket initialization.
//---------------------------------------------------------------------------------------
void Socket::init()
{
    if (!ms_socketLibState)
    {
        Application::registerObject("o3d::Socket", nullptr);
        ms_socketLibState = True;
    }
}

void Socket::quit()
{
    if (ms_socketLibState)
    {
        if (ms_socketLibRefCount != 0)
            O3D_ERROR(E_InvalidOperation("Trying to quit socket library but some socket still exists"));

        Application::unregisterObject("o3d::Socket");
        ms_socketLibState = False;
    }
}

#endif // O3D_WIN_SOCKET

#ifdef O3D_WIN_SOCKET
    #include <ws2tcpip.h>
	#define SOCKET_ERRNO WSAGetLastError()
#else
	#define SOCKET_ERRNO errno
#endif // O3D_WIN_SOCKET

//! Get the socket error string
String getSocketErrorString(int errorCode)
{
	String result;

#ifdef O3D_WIN_SOCKET
    WChar buffer[512];
    memset(buffer, 0, sizeof(buffer));

	FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		0,
		buffer,
		512,
		NULL);

	result = buffer;
#else
	result = strerror(errorCode);
#endif // O3D_WIN_SOCKET
	return result;
}

//---------------------------------------------------------------------------------------
// class Socket
//-------------------------------------------------------------------------------------
// constructor
//---------------------------------------------------------------------------------------
Socket::Socket() :
	m_socket_id(O3D_INVALID_SOCKET),
	m_sockAddr(NULL)
{
}

Socket::Socket(UInt32 adress_family, UInt32 type) :
	m_socket_id(O3D_INVALID_SOCKET),
	m_sockAddr(NULL)
{
	if (adress_family == AF_INET)
		m_sockAddr = new SockAddr4(AF_INET, type);
	else if (adress_family == AF_INET6)
		m_sockAddr = new SockAddr6(AF_INET6, type);
	else
		O3D_ERROR(E_InvalidParameter("af must be AF_INET or AF_INET6"));
}

Socket::Socket(SockAddr *sockAddr) :
	m_socket_id(O3D_INVALID_SOCKET),
	m_sockAddr(NULL)
{
	m_sockAddr = sockAddr;

	if (sockAddr == nullptr)
		O3D_ERROR(E_InvalidParameter("sockAddr must be valid"));
}

//---------------------------------------------------------------------------------------
// destructor
//---------------------------------------------------------------------------------------
Socket::~Socket()
{
	destroy();
}

//---------------------------------------------------------------------------------------
// Create a new socket by accept method
//---------------------------------------------------------------------------------------
Socket* Socket::accept(const Socket &listener)
{
	Socket *pNewSocket = new Socket(
				listener.getSockAddr()->getAf(),
				listener.getSockAddr()->getType());

    Int32 srcAdrLen = sizeof(*pNewSocket->getSockAddr());
    _SOCKET newId;

    if ((newId = ::accept(
             listener.getID(),
             const_cast<sockaddr*>(pNewSocket->getSockAddr()->getSockAddr()),
             (socklen_t*) &srcAdrLen)) == O3D_INVALID_SOCKET)
	{
		deletePtr(pNewSocket);
		O3D_ERROR(E_SocketNotCreated(""));
	}

    pNewSocket->setID(newId);

	return pNewSocket;
}

//---------------------------------------------------------------------------------------
//! Create a new socket by connect method
//---------------------------------------------------------------------------------------
Socket* Socket::connect(SockAddr *sockAddr)
{
	if (sockAddr == nullptr)
		O3D_ERROR(E_NullPointer("sockAddr must be valid"));

	if ((sockAddr->isAny()))
		O3D_ERROR(E_InvalidParameter("Hostname can not be any"));

	Socket *pNewSocket = new Socket(sockAddr);

	// create the new socket
	try
	{
		pNewSocket->socket();
	}
	catch (E_SocketNotCreated &e)
	{
		deletePtr(pNewSocket);
		throw;
	}

	Int32 src_adr_len = sockAddr->getSizeOf();
	if (::connect(pNewSocket->getID(), sockAddr->getSockAddr(),
			src_adr_len) == SOCKET_ERROR)
	{
		deletePtr(pNewSocket);
		return NULL;
	}
	
	return pNewSocket;
}

Socket* Socket::connect(const String &_hostname, UInt16 _port, UInt32 af, UInt32 type)
{
	int sockfd = -1;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = af; // use AF_INET6 to force IPv6
	hints.ai_socktype = type;
    //hints.ai_protocol = type == SOCK_STREAM ? IPPROTO_TCP : IPPROTO_UDP;

	CString hostname = _hostname.toAscii();
	CString port = String::print("%u", _port).toAscii();

    if ((rv = getaddrinfo(hostname.getData(), port.getData(), &hints, &servinfo)) != 0)
	{
		String err = gai_strerror(rv);
		O3D_ERROR(E_InvalidHostName(err));
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue;

		if (::connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			::closesocket(sockfd);
			continue;
		}

		// if we get here, we must have connected successfully
		break;
    }

	if (p == NULL)
	{
		freeaddrinfo(servinfo);
		O3D_ERROR(E_SocketNotCreated(_hostname));
    }

	Socket *pNewSocket = new Socket;
	if (p->ai_addr->sa_family == AF_INET)
		pNewSocket->m_sockAddr = new SockAddr4((sockaddr_in*)p->ai_addr, type);
	else if (p->ai_addr->sa_family == AF_INET6)
		pNewSocket->m_sockAddr = new SockAddr6((sockaddr_in6*)p->ai_addr, type);
	else
	{
		deletePtr(pNewSocket);
		freeaddrinfo(servinfo);
		O3D_ERROR(E_SocketNotCreated("Unsupported address family when connecting to " + _hostname));
	}

	freeaddrinfo(servinfo);

	pNewSocket->m_socket_id = sockfd;

	return pNewSocket;
}

//---------------------------------------------------------------------------------------
// destroy the socket
//---------------------------------------------------------------------------------------
void Socket::destroy()
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
		::closesocket(m_socket_id);
		m_socket_id = O3D_INVALID_SOCKET;
		deletePtr(m_sockAddr);
	}
}

//---------------------------------------------------------------------------------------
// set somes sockets options
//---------------------------------------------------------------------------------------
Bool Socket::setParameters(
		Int32 socket_level,
		Int32 option,
		const CString& value)
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
		if (::setsockopt(m_socket_id, socket_level, option, value.getData(), value.length()) != 0)
			return False;

		return True;
	}
	return False;
}

//---------------------------------------------------------------------------------------
// send a packet
//---------------------------------------------------------------------------------------
Int32 Socket::send(const UInt8* pData, Int32 len, Int32 option)
{
	if (m_socket_id != O3D_INVALID_SOCKET)
    {
		Int32 size;
		if ((size = ::send(m_socket_id, (const char*) pData, len, option)) == SOCKET_ERROR)
			return SOCKET_ERROR;
		return size;
	}
	return SOCKET_ERROR;
}

//---------------------------------------------------------------------------------------
// send Data from buffer
//---------------------------------------------------------------------------------------
Int32 Socket::sendFromBuffer(NetBuffer* buffer, Int32 option)
{
	Int32 result = send(
			reinterpret_cast<UInt8*>(buffer->getBuffer()),
			buffer->getAvailable(),
			option);

	if (result >= 0)
	{
        //O3D_MESSAGE(String("SendFromBuffer ") << result);
		buffer->setPosition(result);
		return result;
	}

	O3D_ERROR(E_SocketError("Socket::SendFromBuffer", result));
	return SOCKET_ERROR;
}

//---------------------------------------------------------------------------------------
// receive a packet
//---------------------------------------------------------------------------------------
Int32 Socket::receive(UInt8* pData, Int32 len, Int32 option)
{
	if (m_socket_id != O3D_INVALID_SOCKET)
    {
		Int32 size;

		if ((size = ::recv(m_socket_id, (char*) pData, len, option)) == SOCKET_ERROR)
			return SOCKET_ERROR;

		return size;
	}

	return SOCKET_ERROR;
}

//---------------------------------------------------------------------------------------
// Read data and store them into buffer
//---------------------------------------------------------------------------------------
Int32 Socket::receiveIntoBuffer(NetBuffer* buffer, Int32 option)
{
	Int32 result = receive(
		reinterpret_cast<UInt8*>(buffer->getWriteBuffer()),
		buffer->getFree(),
		option);

	if (result >= 0)
	{
		if (result > 0)
		{
			buffer->setLimit(buffer->getLimit() + result);
			return result;
		}

		O3D_ERROR(E_SocketError("Socket is closed", 0));
	}

	Int32 err = SOCKET_ERRNO;
#ifdef O3D_WIN_SOCKET
    if (err != WSAEWOULDBLOCK) // TODO maybee remove this test, select should be use
#else
	if (err != EAGAIN)
#endif
	{
		O3D_ERROR(E_SocketError(
			String("Socket::receiveIntoBuffer ") << getSocketErrorString(err) << " (" << err << ")",
			result));
	}

	return 0;
}

//---------------------------------------------------------------------------------------
// send a packet to a specific address
//---------------------------------------------------------------------------------------
Int32 Socket::sendTo(
		const SockAddr &sockaddr,
		const UInt8* pData,
		Int32 len,
		Int32 option)
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
		Int32 size;
		Int32 src_adr_len = sizeof(sockaddr);

		if ((size = ::sendto(
						m_socket_id,
						(const char*)
						pData,
						len,
						option,
						sockaddr.getSockAddr(),
						src_adr_len)) == SOCKET_ERROR)
		{
			return SOCKET_ERROR;
		}
		return size;
	}
	return SOCKET_ERROR;
}

//---------------------------------------------------------------------------------------
// receive a packet and obtain the sender address
//---------------------------------------------------------------------------------------
Int32 Socket::receiveFrom(
		SockAddr &sockaddr,
		UInt8* pData,
		Int32 len,
		Int32 option)
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
		Int32 size;
		Int32 src_adr_len = sizeof(sockaddr);

		if ((size = ::recvfrom(
						m_socket_id,
						(char*)pData,
						len,
						option,
						sockaddr.getSockAddr(),
						(socklen_t*)&src_adr_len)) == SOCKET_ERROR)
		{
			return SOCKET_ERROR;
		}
		return size;
	}
	return SOCKET_ERROR;
}

//---------------------------------------------------------------------------------------
// close the socket
//---------------------------------------------------------------------------------------
Bool Socket::close()
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
		if (::closesocket(m_socket_id) == SOCKET_ERROR)
			return False;
		return True;
	}
	return False;
}

//---------------------------------------------------------------------------------------
// shutdown the socket
//---------------------------------------------------------------------------------------
Bool Socket::shutdown(Int32 how)
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
		if (::shutdown(m_socket_id, how) == SOCKET_ERROR)
			return False;
		return True;
	}
	return False;
}

//---------------------------------------------------------------------------------------
// perform a select and return the number of handles that are ready and contained in the
// fd_set
//---------------------------------------------------------------------------------------
Int32 Socket::select(
        fd_set *readfds,
        fd_set *writefds,
        fd_set *exceptfds,
		UInt32 timeout)
{
	Int32 ret;

    timeval time;

    time.tv_sec = 0;
    time.tv_usec = timeout;

    if ((ret = ::select(FD_SETSIZE, readfds, writefds, exceptfds, &time)) == SOCKET_ERROR)
	{
		return SOCKET_ERROR;
	}

	return ret;
}

Int32 Socket::select(UInt32 timeout)
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
		fd_set fd;

		FD_ZERO(&fd);
		FD_SET(m_socket_id, &fd);

		return select(&fd, NULL, NULL, timeout);
	}
	return SOCKET_ERROR;
}

//---------------------------------------------------------------------------------------
// get the host name for this socket (should be computer host name)
//---------------------------------------------------------------------------------------
String Socket::getHostName()
{
	if (m_sockAddr != nullptr)
		return m_sockAddr->getHostName();
	/*if (m_socket_id != O3D_INVALID_SOCKET)
	{
		Int32 src_adr_len = sizeof(sockaddr_in);

		if (getsockname(m_socket_id, (struct sockaddr*)m_sockAddr->getSockAddr(),
				(socklen_t*) &src_adr_len) != 0)
			O3D_ERROR(E_InvalidHostName(""));

		Char host[1024];
		Char serv[32];

		host[0] = 0;
		serv[0] = 0;

		getnameinfo(m_sockAddr->getSockAddr(), m_sockAddr->getSizeOf(), host, sizeof host, serv, sizeof serv, 0);

		return host;
	}*/
	return "";
}

//---------------------------------------------------------------------------------------
// get the host name for the peer connected to this socket
//---------------------------------------------------------------------------------------
String Socket::getPeerName()
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
		sockaddr *peer_sockaddr = NULL;
		Int32 src_adr_len = 0;

		if (m_sockAddr->getAf() == AF_INET)
		{
			peer_sockaddr = (struct sockaddr*)new sockaddr_in;
			src_adr_len = sizeof(sockaddr_in);
		}
		else if (m_sockAddr->getAf() == AF_INET6)
		{
			peer_sockaddr = (struct sockaddr*)new sockaddr_in6;
			src_adr_len = sizeof(sockaddr_in6);
		}
#ifndef O3D_WINDOWS
        else if (m_sockAddr->getAf() == AF_LOCAL)
        {
            return "";
        }
#endif
		else
			O3D_ERROR(E_InvalidFormat("Support IPv4 and IPv6 only"));

		if (getpeername(m_socket_id, peer_sockaddr, (socklen_t*) &src_adr_len) != 0)
			O3D_ERROR(E_InvalidPeerName(""));

		/*unsigned long adrr = inet_addr(inet_ntoa(peer_sockaddr.sin_addr));
		HOSTENT *pHost = gethostbyaddr((Char*) &adrr, sizeof(struct in_addr), m_sockAddr->getType());
		if (pHost)
			result = pHost->h_name;*/

		Char host[1024];
		Char serv[32];

		host[0] = 0;
		serv[0] = 0;

		getnameinfo(peer_sockaddr, src_adr_len, host, sizeof host, serv, sizeof serv, 0);

		if (m_sockAddr->getAf() == AF_INET)
		{
			deletePtr((sockaddr_in*)peer_sockaddr);
		}
		else if (m_sockAddr->getAf() == AF_INET6)
		{
			deletePtr((sockaddr_in6*)peer_sockaddr);
		}

		return host;
	}
	return "";
}

//---------------------------------------------------------------------------------------
// get this socket address
//---------------------------------------------------------------------------------------
String Socket::getHostAddress()
{
	if (m_sockAddr != nullptr)
		return m_sockAddr->getHostAddress();

	return "";
}

//---------------------------------------------------------------------------------------
// get the peer adress
//---------------------------------------------------------------------------------------
String Socket::getPeerAddress()
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
		sockaddr *peer_sockaddr = NULL;
		Int32 src_adr_len = 0;

		if (m_sockAddr->getAf() == AF_INET)
		{
			peer_sockaddr = (struct sockaddr*)new sockaddr_in;
			src_adr_len = sizeof(sockaddr_in);
		}
		else if (m_sockAddr->getAf() == AF_INET6)
		{
			peer_sockaddr = (struct sockaddr*)new sockaddr_in6;
			src_adr_len = sizeof(sockaddr_in6);
		}
#ifndef O3D_WINDOWS
        else if (m_sockAddr->getAf() == AF_LOCAL)
        {
            return "";
        }
#endif
		else
			O3D_ERROR(E_InvalidFormat("Support IPv4 and IPv6 only"));

		if (getpeername(m_socket_id, peer_sockaddr, (socklen_t*) &src_adr_len) != 0)
			O3D_ERROR(E_InvalidPeerName(""));

		if (m_sockAddr->getAf() == AF_INET)
		{
			Char ip[INET_ADDRSTRLEN] = { 0 };
            inet_ntop(AF_INET, &(((sockaddr_in*)m_sockAddr->getSockAddr())->sin_addr), ip, INET_ADDRSTRLEN);

			deletePtr((sockaddr_in*)peer_sockaddr);

			return ip;
		}
		else if (m_sockAddr->getAf() == AF_INET6)
		{
            Char ip[INET6_ADDRSTRLEN] = { 0 };
            inet_ntop(AF_INET6, &(((sockaddr_in6*)m_sockAddr->getSockAddr())->sin6_addr), ip, INET6_ADDRSTRLEN);

			deletePtr((sockaddr_in6*)peer_sockaddr);

			return ip;
		}
	}
	return "";
}

//---------------------------------------------------------------------------------------
// Define the sock_addr with an host name
//---------------------------------------------------------------------------------------
void Socket::setSockAddr(SockAddr *sockAddr)
{
	m_sockAddr = sockAddr;
}

//---------------------------------------------------------------------------------------
// Bind the socket
//---------------------------------------------------------------------------------------
Bool Socket::bind()
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
		if (::bind(m_socket_id, m_sockAddr->getSockAddr(),m_sockAddr->getSizeOf()) != 0)
			return False;

		return True;
	}

	return False;
}

//---------------------------------------------------------------------------------------
// Create the socket
//---------------------------------------------------------------------------------------
void Socket::socket()
{
	if (m_sockAddr == nullptr)
		O3D_ERROR(E_SocketNotCreated("Missing sockAddr"));

	// create the new socket
	if ((m_socket_id = ::socket(
			 m_sockAddr->getAf(),
			 m_sockAddr->getType(),
			 m_sockAddr->getProtocol())) == O3D_INVALID_SOCKET)
		O3D_ERROR(E_SocketNotCreated("Invalid socket"));
}

//---------------------------------------------------------------------------------------
//! Put the socket in listening mode
//---------------------------------------------------------------------------------------
Bool Socket::listen(Int32 backlog)
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
		if (::listen(m_socket_id, backlog) != 0)
			return False;

		return True;
	}
	return False;
}

//---------------------------------------------------------------------------------------
// Set the socket blocking or not
//---------------------------------------------------------------------------------------
Bool Socket::setNonBlocking(Bool nonBlocking)
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
#ifdef O3D_WIN_SOCKET
		if (nonBlocking)
		{
 O3D_TRACE("");
			u_long argp = 1;
			ioctlsocket(m_socket_id, FIONBIO, &argp);
 O3D_TRACE("");
			if (argp == 0)
				ioctlsocket(m_socket_id, FIONBIO, &argp);
			if (argp == 0)
				return False;
			return True;
		}
		else
		{
 O3D_TRACE("");
			u_long argp = 0;
			ioctlsocket(m_socket_id, FIONBIO, &argp);
 O3D_TRACE("");
			if (argp == 1)
				ioctlsocket(m_socket_id, FIONBIO, &argp);
			if (argp == 1)
				return False;
			return True;
		}
#else
		int status;
		status = fcntl(m_socket_id, F_GETFL);

		if (nonBlocking)
		{
			if (fcntl(m_socket_id, F_SETFL, status | O_NONBLOCK) == -1)
				return False;
			return True;
		}
		else
		{
			if (fcntl(m_socket_id, F_SETFL, status & ~O_NONBLOCK) == -1)
				return False;
			return True;	
		}
#endif
	}

	return False;
}

//---------------------------------------------------------------------------------------
// Set the read time out
//---------------------------------------------------------------------------------------
void Socket::setReadTimeout(UInt32 timeout)
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
#ifdef O3D_WIN_SOCKET
		int timeval = timeout * 1000;
 O3D_TRACE("");
		if (setsockopt(m_socket_id, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeval,
				sizeof(int)) != 0)
		{
			Int32 err = SOCKET_ERRNO;
			O3D_ERROR(E_InvalidParameter(String("Cannot set read timeout cause : [") << getSocketErrorString(err) << "]"));
		}
 O3D_TRACE("");
#else
		struct timeval tv;
        tv.tv_sec = timeout / 1000000;
        tv.tv_usec = timeout % 1000000;

		if (setsockopt(m_socket_id, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &tv,
				sizeof(struct timeval)) != 0)
		{
			Int32 err = SOCKET_ERRNO;
			O3D_ERROR(E_InvalidParameter(String("Cannot set read timeout cause : [") << getSocketErrorString(err) << "]"));
		}
#endif
	}
	else
	{
		O3D_ERROR(E_SocketNotCreated(""));
	}
}

//---------------------------------------------------------------------------------------
// Set the write time out
//---------------------------------------------------------------------------------------
void Socket::setWriteTimeout(UInt32 timeout)
{
	if (m_socket_id != O3D_INVALID_SOCKET)
	{
#ifdef O3D_WIN_SOCKET
		int timeval = timeout * 1000;

		if (setsockopt(
				m_socket_id,
				SOL_SOCKET,
				SO_SNDTIMEO,
				(const char *) &timeval,
				sizeof(int)) != 0)
		{
			Int32 err = SOCKET_ERRNO;
			O3D_ERROR(E_InvalidParameter(String("Cannot set write timeout cause : [") << getSocketErrorString(err) << "]"));
		}
#else
		struct timeval tv;
        tv.tv_sec = timeout / 1000000;
        tv.tv_usec = timeout % 1000000;

		if (setsockopt(
				m_socket_id,
				SOL_SOCKET,
				SO_SNDTIMEO,
				(struct timeval *) &tv,
				sizeof(struct timeval)) != 0)
		{
			Int32 err = SOCKET_ERRNO;
			O3D_ERROR(E_InvalidParameter(String("Cannot set write timeout cause : [") << getSocketErrorString(err) << "]"));
		}
#endif
	}
	else
	{
		O3D_ERROR(E_SocketNotCreated(""));
	}

}

