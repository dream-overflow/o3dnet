/**
 * @file proxyserver.h
 * @brief 
 * @author Patrice GILBERT (patrice.gilbert@revolutining.com) 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-01-09
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_NET_PROXYSERVER_H
#define _O3D_NET_PROXYSERVER_H

#include "netserver.h"
#include "netsession.h"
#include <o3d/core/scheduledthreadpool.h>
#include <o3d/core/idmanager.h>
#include <o3d/core/smartarray.h>

#include <unordered_map>

namespace o3d {
namespace net {

class ProxyServer;

/**
 * @brief Proxy server net session instance.
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-01-09
 * @todo Use an x509 certificate with openSSL
 */
class O3D_NET_API ProxyServerSession : public o3d::Runnable
{
public:

    /**
     * @brief ProxyServerSession
     * @param proxyServer
     * @param client
     * @param id
     * @param messageFactory
     * @param messageAdapter
     */
    ProxyServerSession(ProxyServer *proxyServer, Socket *client);

    virtual ~ProxyServerSession();

    /**
     * @brief setId Unique id.
     * @param id
     */
    void setId(o3d::Int32 id) { m_id = id; }

    /**
     * @brief getId
     * @return Unique integer identifier.
     */
    inline o3d::Int32 getId() const { return m_id; }

    /**
     * @brief send Send a message to the client proxy.
     * @param msg
     */
    inline void send(NetMessage *msg) { m_netSession->pushMessage(msg); }

    /**
     * @brief run Process input message execution.
     * @return
     */
    virtual o3d::Int32 run(void *context);

    /**
     * @brief cancel Finish it cleanly.
     */
    void cancel();

    /**
     * @brief isCanceled
     * @return True if the session is canceled (previous call to cancel).
     */
    Bool isCanceled() const { return m_cancel; }

    /**
     * @brief setValid Validate the session.
     */
    void setValid() { m_valid = o3d::True; }

    /**
     * @brief isValid
     * @return True if the session is valide (authentified and recognized).
     */
    o3d::Bool isValid() const { return m_valid; }

    /**
     * @brief getProxyServer
     * @return The owner proxy server.
     */
    const ProxyServer* getProxyServer() const { return m_proxyServer; }

protected:

    ProxyServer *m_proxyServer;

    o3d::Int32 m_id;   //!< Unique session identifier per proxy server.
    NetSession *m_netSession;

    o3d::Bool m_cancel;
    o3d::Bool m_valid; //!< True means the session is valid and authentified

    o3d::SmartArrayUInt8 m_challenge;
};


/**
 * @brief Proxy server acceptor abstract class to inherit.
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-01-07
 */
class O3D_NET_API ProxyServerAcceptor : public NetSessionAcceptor
{
public:

    ProxyServerAcceptor(ProxyServer *proxyServer, o3d::ScheduledThreadPool *executor);

    virtual void accepted(Socket *client);

private:

    ProxyServer *m_proxyServer;
    o3d::ScheduledThreadPool *m_executor;
};


/**
 * @brief Proxy server
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-01-07
 */
class O3D_NET_API ProxyServer
{
public:

    /**
     * @brief ProxyServer
     * @param factory A valid net message factory.
     * @param adapter Net message adapter, if null use the default.
     * @param port Server listen port number.
     * @param poolSize Number of execution threads.
     * @param delay Delay of execution of sessions.
     * @param timeUnit Time unit for delay.
     */
    ProxyServer(
            NetMessageFactory *factory,
            NetReadWriteAdapter *adapter,
            o3d::UInt32 port,
            o3d::UInt32 poolSize,
            o3d::UInt32 delay = 50,
            o3d::TimeUnit timeUnit = TIME_MILLISECOND);

    virtual ~ProxyServer();

    /**
     * @brief start Start the proxy serverx
     */
    virtual void start(o3d::UInt32 af = AF_INET);

    /**
     * @brief stop Stop the proxy server, letting down the remaining messagesx
     */
    virtual void stop();

    /**
     * @brief send Send a message to the proxy client and consume it one time.
     * @param sessionId A valid session identifier where to send the message.
     * @param msg A valid message to send that can be consumed 1 time.
     * @note Never send a message that could be delete before its processing.
     *       Multicast lock this mutex during this method.
     */
    void send(o3d::Int32 sessionId, NetMessage *msg);

    /**
     * @brief multicast Send a message to any sessions.
     * @param msg A valid message that can be consumed for any session (@see getNumSessions).
     * @note Multicast lock this mutex during this method.
     */
    void multicast(NetMessage *msg);

    /**
     * @brief getNumSessions
     * @return Number of currents sessions.
     */
    o3d::UInt32 getNumSessions() const;

    /**
     * @brief getNextId Use the the next id to use according to the number of session.
     * @return The next id to use.
     */
    o3d::Int32 getNextId();

    /**
     * @brief releaseId
     * @param id
     */
    void releaseID(o3d::Int32 id);

    //! Delay of execution of sessions.
    o3d::UInt32 getDelay() const { return m_delay; }
    //! Delay time unit of execution of sessions.
    o3d::TimeUnit getTimeUnit() const { return m_timeUnit; }

    NetMessageFactory* getNetMessageFactory() const { return m_netMessageFactory; }
    NetReadWriteAdapter* getReadWriteAdapter() const { return m_readWriteAdapter; }

    /**
     * @brief setVersion Define the protocol version. The client must have the same.
     * @param version
     */
    void setVersion(o3d::Int32 version);

    /**
     * @brief getVersion Get the protocole version.
     * @return
     */
    o3d::Int32 getVersion() const { return m_version; }

    /**
     * @brief setCertificate If a session does not sent the same certificate it is refused.
     * @param cert User defined certificat to compare with client sent certificate.
     */
    void setCertificate(const o3d::SmartArrayUInt8 &cert);

    /**
     * @brief getCertificate
     * @return
     */
    const SmartArrayUInt8& getCertificate() const { return m_certificate; }

    /**
     * @brief terminateSession Terminate (cancel) a session according to its identifier.
     * @param sessionId
     */
    void terminateSession(o3d::Int32 sessionId);

    /**
     * @brief schedule A
     * @param session
     * @return New session id
     */
    o3d::Int32 schedule(ProxyServerSession *session);

    /**
     * @brief removeSession Remove but not delete it.
     * @param sessionId
     */
    void removeSession(o3d::Int32 sessionId);

protected:

    o3d::UInt16 m_port;
    o3d::UInt32 m_poolSize;
    o3d::UInt32 m_delay;
    o3d::TimeUnit m_timeUnit;

    NetMessageFactory *m_netMessageFactory;
    NetReadWriteAdapter *m_readWriteAdapter;

    NetServer *m_server;
    NetSessionAcceptor *m_acceptor;

    o3d::IDManager m_ids;
    std::unordered_map<o3d::Int32, ProxyServerSession*> m_sessions;

    o3d::FastMutex m_mutex;
    o3d::ScheduledThreadPool *m_executor;

    o3d::Int32 m_version;
    o3d::SmartArrayUInt8 m_certificate;
};

} // namespace net
} // namespace o3d

#endif // _O3D_NET_PROXYSERVER_H

