/**
 * @file proxyclient.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_NET_PROXYCLIENT_H
#define _O3D_NET_PROXYCLIENT_H

#include "netclient.h"

namespace o3d {
namespace net {

/**
 * @brief Proxy client
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-01-07
 */
class O3D_NET_API ProxyClient : public o3d::Runnable
{
public:

    /**
     * @brief ProxyClient Create a new proxy client using a NetClient.
     * @param host
     * @param port
     * @param messageFactory Valid message factory with at least ProxyChallengeIn registred
     * @param messageAdapter Should use the DefaultNetMessageAdapter.
     */
    ProxyClient(
            const o3d::String &host,
            o3d::UInt32 port,
            NetMessageFactory *messageFactory,
            NetReadWriteAdapter *messageAdapter = nullptr);

    virtual ~ProxyClient();

    /**
     * @brief connect Connect to the proxy server.
     */
    virtual void connect(o3d::UInt32 af);

    /**
     * @brief disconnect Disconnect from the proxy server. All unprocessed messages
     *                   are lost.
     */
    virtual void disconnect();

    /**
     * @brief send Send a message to the proxy server and consume it one time.
     * @param msg
     * @note Never send a message that could be delete before its processing.
     */
    void send(NetMessage *msg);

    /**
     * @brief setVersion The the supported protocol version.
     * @param version
     */
    void setVersion(o3d::Int32 version) { m_version = version; }

    /**
     * @brief getVersion Get the supported protocol version.
     * @note Must be the same as the server.
     * @return
     */
    o3d::Int32 getVersion() const { return m_version; }

    /**
     * @brief getChallenge Get the session challenge (16 bytes).
     * @return
     */
    const o3d::SmartArrayUInt8& getChallenge() const { return m_challenge; }

    /**
     * @brief setCertificate Defines the client certificates to be validate by the server.
     * @param cert
     */
    void setCertificate(const o3d::SmartArrayUInt8& cert) { m_certificate = cert; }

    /**
     * @brief getCertificate Get the client certificate (if previously set).
     * @return
     */
    const o3d::SmartArrayUInt8& getCertificate() const { return m_certificate; }

protected:

    /**
     * @brief process Run received messages.
     */
    virtual Int32 run(void *);

    o3d::UInt16 m_port;
    o3d::String m_host;

    o3d::net::NetClient *m_netClient;

    o3d::Int32 m_version;
    o3d::SmartArrayUInt8 m_challenge;

    o3d::SmartArrayUInt8 m_certificate;

    o3d::Bool m_cancel;
    o3d::Thread *m_thread;
};

} // namespace net
} // namespace o3d

#endif // _O3D_NET_PROXYCLIENT_H
