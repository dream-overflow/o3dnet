/**
 * @file proxymessages.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_NET_PROXY_MESSAGES_H
#define _O3D_NET_PROXY_MESSAGES_H

#include "netmessageadapter.h"
#include <o3d/core/smartarray.h>
#include <cstring>

namespace o3d {
namespace net {

/**
 * @brief Proxy server challenge out.
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2014-01-15
 */
class ProxyChallengeOut : public NetMessageOutHelper<1>
{
public:

    ProxyChallengeOut()
    {
        m_messageDataSize = 20;
    }

    void setVersion(o3d::Int32 version)
    {
        m_version = version;
    }

    void setChallenge(const o3d::UInt8 *_challenge)
    {
        std::memcpy(m_challenge, _challenge, 16);
    }

    virtual NetMessage* writeToBuffer(NetBuffer* buffer);

private:

    o3d::Int32 m_version;
    o3d::UInt8 m_challenge[16];
};

/**
 * @brief Proxy client Challenge in (to register into the ProxyClient).
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2014-01-15
 */
class ProxyChallengeIn : public NetMessageInHelper<ProxyChallengeIn, 1>
{
public:

    virtual NetMessage* readFromBuffer(NetBuffer* buffer);

    virtual void run(void *context);

private:

    o3d::Int32 m_version;
    o3d::UInt8 m_challenge[16];
};

/**
 * @brief Proxy client certificate message out
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2014-01-15
 */
class ProxyCertificateOut : public NetMessageOutHelper<1>
{
public:

    ProxyCertificateOut()
    {
        m_messageDataSize = 0;
    }

    void setCertificate(const SmartArrayUInt8 &cert)
    {
        m_certificate = cert;
        m_messageDataSize = m_certificate.getSizeInBytes();
    }

    virtual NetMessage* writeToBuffer(NetBuffer* buffer);

private:

    o3d::SmartArrayUInt8 m_certificate;
};

/**
 * @brief Proxy server certificate message in (to register into the ProxyServer).
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2014-01-15
 */
class ProxyCertificateIn : public NetMessageInHelper<ProxyCertificateIn, 1>
{
public:

    virtual NetMessage* readFromBuffer(NetBuffer* buffer);

    virtual void run(void *context);

private:

    o3d::SmartArrayUInt8 m_certificate;
};

} // namespace net
} // namespace o3d

#endif // _O3D_NET_PROXY_MESSAGES_H

