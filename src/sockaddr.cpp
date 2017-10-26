/**
 * @file sockaddr.cpp
 * @brief Implementation of SockAddr.h
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-07-21
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/precompiled.h"

#include "o3d/net/sockaddr.h"
#include "o3d/net/socket.h"
#include <o3d/core/debug.h>

using namespace o3d;
using namespace o3d::net;

//---------------------------------------------------------------------------------------
// class SockAddr
//---------------------------------------------------------------------------------------
Bool SockAddr::operator== (const SockAddr &cmp) const
{
	if (this->getType() != cmp.getType())
        return False;

	if (this->getClassType() == SOCK_ADDR4)
		return (((SockAddr4*)this)->operator ==((SockAddr4&)cmp));
	else if (this->getClassType() == SOCK_ADDR6)
		return (((SockAddr6*)this)->operator ==((SockAddr6&)cmp));
	else
        return False;
}

Bool SockAddr::operator!= (const SockAddr &cmp) const
{
	if (this->getType() != cmp.getType())
        return False;

	if (this->getClassType() == SOCK_ADDR4)
		return (((SockAddr4*)this)->operator !=((SockAddr4&)cmp));
	else if (this->getClassType() == SOCK_ADDR6)
		return (((SockAddr6*)this)->operator !=((SockAddr6&)cmp));
	else
        return False;
}

Bool SockAddr::operator< (const SockAddr &cmp) const
{
	if (this->getType() != cmp.getType())
        return False;

	if (this->getClassType() == SOCK_ADDR4)
		return (((SockAddr4*)this)->operator <((SockAddr4&)cmp));
	else if (this->getClassType() == SOCK_ADDR6)
		return (((SockAddr6*)this)->operator <((SockAddr6&)cmp));
	else
        return False;
}

//---------------------------------------------------------------------------------------
// class SockAddr4
//---------------------------------------------------------------------------------------
SockAddr4::SockAddr4(const sockaddr_in *sockAddr, UInt32 type)
{
	m_af = sockAddr->sin_family;
	m_type = type;
	m_protocol = 0;

	memcpy(&m_sockaddr, sockAddr, sizeof(sockaddr_in));

	// auto-detect the protocol type
	if (m_type == SOCK_STREAM)
		m_protocol = IPPROTO_TCP;
	else if (m_type == SOCK_DGRAM)
		m_protocol = IPPROTO_UDP;
}

SockAddr4::SockAddr4(UInt32 type, UInt32 adress_family)
{
	m_af = adress_family;
	m_type = type;
	m_protocol = 0;

	memset(&m_sockaddr, 0, sizeof(sockaddr_in));

	// auto-detect the protocol type
	if (m_type == SOCK_STREAM)
		m_protocol = IPPROTO_TCP;
	else if (m_type == SOCK_DGRAM)
		m_protocol = IPPROTO_UDP;

	m_sockaddr.sin_family = (short)m_af;
}

SockAddr4::SockAddr4(
		const String &hostname,
        UInt16 port,
        UInt32 type,
        UInt32 adress_family)
{
	m_af = adress_family;
	m_type = type;

	memset(&m_sockaddr, 0, sizeof(sockaddr_in));

	// auto-detect the protocol type
	if (m_type == SOCK_STREAM)
		m_protocol = IPPROTO_TCP;
	else if (m_type == SOCK_DGRAM)
		m_protocol = IPPROTO_UDP;

	setSockAddr(hostname, port);
	m_sockaddr.sin_family = (short)m_af;
}

sockaddr* SockAddr4::getSockAddr()
{
	return (struct sockaddr*)&m_sockaddr;
}

const sockaddr* SockAddr4::getSockAddr() const
{
	return (struct sockaddr*)&m_sockaddr;
}

size_t SockAddr4::getSizeOf() const
{
	return sizeof(sockaddr_in);
}

SockAddr::SockAddrClassType SockAddr4::getClassType() const
{
	return SOCK_ADDR4;
}

Bool SockAddr4::isAny() const
{
	return m_sockaddr.sin_addr.s_addr == INADDR_ANY;
}

void SockAddr4::setSockAddr(const String &hostname, UInt16 port)
{
	// check the hostname format and convert it in IP adress
	if (hostname.isEmpty())
	{
		m_sockaddr.sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		// put host IP
		m_sockaddr.sin_addr.s_addr = inet_addr(hostname.toAscii().getData());

		// if it's not an IP get host by name
		if (m_sockaddr.sin_addr.s_addr == INADDR_NONE)
		{
			hostent *pHostent = gethostbyname(hostname.toAscii().getData());

			if (pHostent == NULL)
				O3D_ERROR(E_InvalidParameter("Hostname not found"));

			m_sockaddr.sin_addr.s_addr = *((unsigned long*)pHostent->h_addr_list[0]);
		}
	}

	m_sockaddr.sin_port = htons(port);
	m_hostname = hostname;
}

String SockAddr4::getHostAddress() const
{
	Char ip[INET_ADDRSTRLEN] = { 0 };
	inet_ntop(m_af, (void*)&m_sockaddr.sin_addr, ip, INET_ADDRSTRLEN);

	return ip;
}

String SockAddr4::getHostName() const
{
	Char host[1024];
	Char serv[32];

	host[0] = 0;
	serv[0] = 0;

	int res = getnameinfo((struct sockaddr*)&m_sockaddr, getSizeOf(), host, sizeof host, serv, sizeof serv, 0);
	if (res != 0)
	{
		String err = gai_strerror(res);
		O3D_ERROR(E_InvalidHostName(err));
	}

	return host;
}

UInt16 SockAddr4::getPort() const
{
	return ntohs(m_sockaddr.sin_port);
}

void SockAddr4::setTcp(const String &hostname, UInt16 port)
{
	m_protocol = IPPROTO_TCP;
	setSockAddr(hostname, port);
}

void SockAddr4::setUdp(const String &hostname, UInt16 port)
{
	m_protocol = IPPROTO_UDP;
	setSockAddr(hostname, port);
}

Bool SockAddr4::operator== (const SockAddr4 &cmp) const
{
	return (memcmp(&m_sockaddr,&cmp.m_sockaddr,sizeof(sockaddr_in)) == 0);
}

Bool SockAddr4::operator!= (const SockAddr4 &cmp) const
{
	return (memcmp(&m_sockaddr,&cmp.m_sockaddr,sizeof(sockaddr_in)) != 0);
}

Bool SockAddr4::operator< (const SockAddr4 &cmp) const
{
	return (memcmp(&m_sockaddr,&cmp.m_sockaddr,sizeof(sockaddr_in)) < 0);
}

//---------------------------------------------------------------------------------------
// class SockAddr6
//---------------------------------------------------------------------------------------
SockAddr6::SockAddr6(const sockaddr_in6 *sockAddr, UInt32 type)
{
	m_af = sockAddr->sin6_family;
	m_type = type;
	m_protocol = 0;

	memcpy(&m_sockaddr, sockAddr, sizeof(sockaddr_in6));

	// auto-detect the protocol type
	if (m_type == SOCK_STREAM)
		m_protocol = IPPROTO_TCP;
	else if (m_type == SOCK_DGRAM)
		m_protocol = IPPROTO_UDP;
}

SockAddr6::SockAddr6(UInt32 type, UInt32 adress_family)
{
	m_af = adress_family;
	m_type = type;

	memset(&m_sockaddr, 0, sizeof(m_sockaddr));

	// auto-detect the protocol type
	if (m_type == SOCK_STREAM)
		m_protocol = IPPROTO_TCP;
	else if (m_type == SOCK_DGRAM)
		m_protocol = IPPROTO_UDP;

	m_sockaddr.sin6_family = (short)m_af;
}

SockAddr6::SockAddr6(
		const String &hostname,
        UInt16 port,
        UInt32 type,
        UInt32 adress_family)
{
	m_af = adress_family;
	m_type = type;

	memset(&m_sockaddr, 0, sizeof(m_sockaddr));

	// auto-detect the protocol type
	if (m_type == SOCK_STREAM)
		m_protocol = IPPROTO_TCP;
	else if (m_type == SOCK_DGRAM)
		m_protocol = IPPROTO_UDP;

	setSockAddr(hostname, port);
	m_sockaddr.sin6_family = (short)m_af;
}

sockaddr* SockAddr6::getSockAddr()
{
	return (struct sockaddr*)&m_sockaddr;
}

const sockaddr* SockAddr6::getSockAddr() const
{
	return (struct sockaddr*)&m_sockaddr;
}

size_t SockAddr6::getSizeOf() const
{
	return sizeof(sockaddr_in6);
}

SockAddr::SockAddrClassType SockAddr6::getClassType() const
{
	return SOCK_ADDR6;
}

UInt16 SockAddr6::getPort() const
{
	return ntohs(m_sockaddr.sin6_port);
}

Bool SockAddr6::isAny() const
{
	return memcmp(&m_sockaddr.sin6_addr, &in6addr_any, sizeof in6addr_any) == 0;
}

void SockAddr6::setSockAddr(const String &hostname, UInt16 _port)
{
	// check the hostname format and convert it in IP adress
	if (hostname.isEmpty())
	{
		struct addrinfo hints, *servinfo;
		int rv;

		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET6;
		hints.ai_socktype = m_type;
        //hints.ai_protocol = m_type == SOCK_STREAM ? IPPROTO_TCP : IPPROTO_UDP;
        hints.ai_flags = AI_PASSIVE; // use my IP address

		CString port = String::print("%u", _port).toAscii();

		if ((rv = getaddrinfo(NULL, port.getData(), &hints, &servinfo)) != 0)
		{
			String err = gai_strerror(rv);
			O3D_ERROR(E_InvalidHostName(err));
		}

		memcpy(&m_sockaddr, servinfo->ai_addr, servinfo->ai_addrlen);

		freeaddrinfo(servinfo);
	}
	else
	{
		struct addrinfo hints, *servinfo;
		int rv;

		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET6;
		hints.ai_socktype = m_type;

		CString hostname = m_hostname.toAscii();
		CString port = String::print("%u", _port).toAscii();

		if ((rv = getaddrinfo(hostname.getData(), port.getData(), &hints, &servinfo)) != 0)
		{
			String err = gai_strerror(rv);
			O3D_ERROR(E_InvalidHostName(err));
		}

		memcpy(&m_sockaddr, servinfo->ai_addr, servinfo->ai_addrlen);

		freeaddrinfo(servinfo);
	}

	m_hostname = hostname;

#ifdef O3D_DBG_NET
	O3D_LOG(String("setSockAddr OK with ") + m_hostname);
#endif
}

void SockAddr6::setTcp(const String &hostname, UInt16 port)
{
	m_protocol = IPPROTO_TCP;
	setSockAddr(hostname, port);
}

void SockAddr6::setUdp(const String &hostname, UInt16 port)
{
	m_protocol = IPPROTO_UDP;
	setSockAddr(hostname, port);
}

String SockAddr6::getHostAddress() const
{
	Char ip[INET6_ADDRSTRLEN] = { 0 };
    inet_ntop(m_af, (void*)&m_sockaddr.sin6_addr, ip, INET6_ADDRSTRLEN);

	return ip;
}

String SockAddr6::getHostName() const
{
	Char host[1024];
	Char serv[32];

	host[0] = 0;
	serv[0] = 0;

	int res = getnameinfo((sockaddr*)&m_sockaddr, getSizeOf(), host, sizeof host, serv, sizeof serv, 0);
	if (res != 0)
	{
		String err = gai_strerror(res);
		O3D_ERROR(E_InvalidHostName(err));
	}

	return host;
}

Bool SockAddr6::operator== (const SockAddr6 &cmp) const
{
	return (memcmp(&m_sockaddr,&cmp.m_sockaddr,sizeof(sockaddr_in6)) == 0);
}

Bool SockAddr6::operator!= (const SockAddr6 &cmp) const
{
	return (memcmp(&m_sockaddr,&cmp.m_sockaddr,sizeof(sockaddr_in6)) != 0);
}

Bool SockAddr6::operator< (const SockAddr6 &cmp) const
{
	return (memcmp(&m_sockaddr,&cmp.m_sockaddr,sizeof(sockaddr_in6)) < 0);
}

