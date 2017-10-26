/**
 * @file sockaddr.h
 * @brief SockAddr.
 * @author Patrice GILBERT (patrice.gilbert@revolutining.com) 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-07-21
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_SOCKADDR_H
#define _O3D_SOCKADDR_H

#include "net.h"

#include <o3d/core/memorydbg.h>
#include <o3d/core/string.h>

#ifdef O3D_WINDOWS
	#include <o3d/core/architecture.h>
	#include <ws2tcpip.h>
	#define O3D_WIN_SOCKET
	#define O3D_INVALID_SOCKET INVALID_SOCKET
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#ifndef INVALID_SOCKET
	#define INVALID_SOCKET  (_SOCKET)(~0)//-1
	#endif
	#define O3D_INVALID_SOCKET INVALID_SOCKET
#endif

namespace o3d {
namespace net {

#ifdef O3D_WINDOWS
	typedef SOCKET _SOCKET;
#else
    typedef o3d::Int32 _SOCKET;
#endif

//---------------------------------------------------------------------------------------
//! @class SockAddr
//-------------------------------------------------------------------------------------
//! A sock_addr structure object
//---------------------------------------------------------------------------------------
class O3D_NET_API SockAddr
{
public:

	enum SockAddrClassType
	{
		SOCK_ADDR4 = 0,
		SOCK_ADDR6 = 1
	};

    virtual ~SockAddr() {}

	//! @brief Set as TCP sock addr
	//! @param hostname The hostname "ALL" and "all" mean ANY_ADDR
	//! @param port The port
    virtual void setTcp(const String &hostname, UInt16 port) = 0;

	//! @brief Set as UDP datagram sock addr
	//! @param hostname The hostname "ALL" and "all" mean ANY_ADDR
	//! @param port The port
    virtual void setUdp(const String &hostname, UInt16 port) = 0;

	//! @return true if the sockaddr refers to any.
    virtual Bool isAny() const = 0;

	//! @brief Return the sock_addr_in structure pointer
	//! @return The sock_addr_in structure pointer
	virtual sockaddr* getSockAddr() = 0;

	//! @brief return the sock_addr_in structure pointer (const version)
	//! @return the sock_addr_in structure pointer (const version)
	virtual const sockaddr* getSockAddr() const = 0;

	//! @brief return the size of the sockaddr_in struct.
	virtual size_t getSizeOf() const = 0;

	//! @brief return the class type of SockAddr.
	virtual SockAddrClassType getClassType() const = 0;

    inline UInt32 getAf() const { return m_af; }
    inline UInt32 getType() const { return m_type; }
    inline UInt32 getProtocol() const { return m_protocol; }

	//! @brief Check if the hostname is valid
	//! @return True if the hostname is valid
    inline Bool isValid() const { return (m_hostname.length() > 0); }

	//! @return Hostname without NS lookup. Like as it was set.
	inline const String& getGivenHostName() const { return m_hostname; }

	//! @brief Human readable host IP address
	virtual String getHostAddress() const = 0;
	//! @brief NS lookup host name if possible.
	virtual String getHostName() const = 0;

	//! @return The port
    virtual UInt16 getPort() const = 0;

	//! @brief Comparaison operator
	//! @return True if equal
    Bool operator== (const SockAddr &cmp) const;

	//! @brief Comparaison operator
	//! @return True if different
    Bool operator!= (const SockAddr &cmp) const;

	//! @brief Comparaison operator
	//! @return True if lesser
    Bool operator< (const SockAddr &cmp) const;

protected:

	String m_hostname;        //!< The hostname

    UInt32 m_af;          //!< adresse family
    UInt32 m_type;        //!< sock type
    UInt32 m_protocol;    //!< The protocol format
};

//---------------------------------------------------------------------------------------
//! @class SockAddr4
//-------------------------------------------------------------------------------------
//! IPv4 version on the SockAddr.
//---------------------------------------------------------------------------------------
class O3D_NET_API SockAddr4 : public SockAddr
{
public:

	//! Duplicate an existing sockaddr struct.
    SockAddr4(const sockaddr_in *sockAddr, UInt32 type);

	//! @brief Default constructor
	SockAddr4(
            UInt32 type = SOCK_STREAM,
            UInt32 adress_family = AF_INET);

	//! @brief Initialisation constructor
	//! @param hostname Empty hostname mean ANY_ADDR.
	//! @param port The port
	//! @param type Socket protocol type. SOCK_STREAM or SOCK_DGRAM
	//! @param adress_family Address family. generaly AF_INET.
	SockAddr4(
			const String &hostname,
            UInt16 port,
            UInt32 type = SOCK_STREAM,
            UInt32 adress_family = AF_INET);

    virtual ~SockAddr4() {}

	//! @brief Set as TCP sock addr
	//! @param hostname The hostname "ALL" and "all" mean ANY_ADDR.
	//! @param port The port
    virtual void setTcp(const String &hostname, UInt16 port);

	//! @brief Set as UDP datagram sock addr
	//! @param hostname The hostname "ALL" and "all" mean ANY_ADDR.
	//! @param port The port
    virtual void setUdp(const String &hostname, UInt16 port);

	//! @return true if the sockaddr refers to any.
    virtual Bool isAny() const;

	//! @brief Return the sock_addr_in structure pointer
	//! @return The sock_addr_in structure pointer
	virtual sockaddr* getSockAddr();

	//! @brief return the sock_addr_in structure pointer (const version)
	//! @return the sock_addr_in structure pointer (const version)
	virtual const sockaddr* getSockAddr() const;

	//! @brief return the size of the sockaddr_in struct.
	virtual size_t getSizeOf() const;

	//! @brief return the class type of SockAddr.
	virtual SockAddrClassType getClassType() const;

	//! @return The port
    virtual UInt16 getPort() const;

	//! @brief Human readable host IP address
	virtual String getHostAddress() const;
	//! @brief NS lookup host name if possible.
	virtual String getHostName() const;

	//! @brief Comparaison operator
	//! @return True if equal
    Bool operator== (const SockAddr4 &cmp) const;

	//! @brief Comparaison operator
	//! @return True if different
    Bool operator!= (const SockAddr4 &cmp) const;

	//! @brief Comparaison operator
	//! @return True if lesser
    Bool operator< (const SockAddr4 &cmp) const;

private:

    void setSockAddr(const String &hostname, UInt16 port);

	sockaddr_in m_sockaddr;   //!< sock host address struct
};

//---------------------------------------------------------------------------------------
//! @class SockAddr6
//-------------------------------------------------------------------------------------
//! IPv6 version on the SockAddr.
//---------------------------------------------------------------------------------------
class O3D_NET_API SockAddr6 : public SockAddr
{
public:

	//! Duplicate an existing sockaddr struct.
    SockAddr6(const sockaddr_in6 *sockAddr, UInt32 type);

	//! @brief Default constructor
	SockAddr6(
            UInt32 type = SOCK_STREAM,
            UInt32 adress_family = AF_INET6);

	//! @brief Initialisation constructor
	//! @param hostname Empty hostname mean ANY_ADDRV6
	//! @param port The port
	//! @param type Socket protocol type. SOCK_STREAM or SOCK_DGRAM
	//! @param adress_family Address family. generaly AF_INET6.
	SockAddr6(
			const String &hostname,
            UInt16 port,
            UInt32 type = SOCK_STREAM,
            UInt32 adress_family = AF_INET6);

    virtual ~SockAddr6() {}

	//! @brief Set as TCP sock addr
	//! @param hostname The hostname "ALL" and "all" mean ANY_ADDR
	//! @param port The port
    virtual void setTcp(const String &hostname, UInt16 port);

	//! @brief Set as UDP datagram sock addr
	//! @param hostname The hostname "ALL" and "all" mean ANY_ADDR
	//! @param port The port
    virtual void setUdp(const String &hostname, UInt16 port);

	//! @return true if the sockaddr refers to any.
    virtual Bool isAny() const;

	//! @brief Return the sock_addr_in structure pointer
	//! @return The sock_addr_in structure pointer
	virtual sockaddr* getSockAddr();

	//! @brief return the sock_addr_in structure pointer (const version)
	//! @return the sock_addr_in structure pointer (const version)
	virtual const sockaddr* getSockAddr() const;

	//! @brief return the size of the sockaddr_in struct.
	virtual size_t getSizeOf() const;

	//! @brief return the class type of SockAddr.
	virtual SockAddrClassType getClassType() const;

	//! @return The port
    virtual UInt16 getPort() const;

	//! @brief Human readable host IP address
	virtual String getHostAddress() const;
	//! @brief NS lookup host name if possible.
	virtual String getHostName() const;

	//! @brief Comparaison operator
	//! @return True if equal
    Bool operator== (const SockAddr6 &cmp) const;

	//! @brief Comparaison operator
	//! @return True if different
    Bool operator!= (const SockAddr6 &cmp) const;

	//! @brief Comparaison operator
	//! @return True if lesser
    Bool operator< (const SockAddr6 &cmp) const;

private:

    void setSockAddr(const String &hostname, UInt16 port);

	sockaddr_in6 m_sockaddr; //!< sock host address struct v6
};

} // namespace net
} // namespace o3d

#endif // _O3D_SOCKADDR_H
