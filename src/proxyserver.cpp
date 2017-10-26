/**
 * @file proxyserver.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/precompiled.h"
#include <o3d/core/architecture.h>
#include "o3d/net/proxyserver.h"
#include "o3d/net/proxymessages.h"
#include <o3d/core/debug.h>

using namespace o3d;
using namespace o3d::net;

//
// ProxyServer
//
ProxyServer::ProxyServer(
        NetMessageFactory *factory,
        NetReadWriteAdapter *adapter,
        o3d::UInt32 port,
        o3d::UInt32 poolSize,
        o3d::UInt32 delay,
        o3d::TimeUnit timeUnit) :
    m_port(port),
    m_poolSize(poolSize),
    m_delay(delay),
    m_timeUnit(timeUnit),
    m_netMessageFactory(factory),
    m_readWriteAdapter(adapter),
    m_server(nullptr),
    m_acceptor(nullptr),
    m_executor(nullptr)
{
    O3D_ASSERT(m_netMessageFactory);
}

ProxyServer::~ProxyServer()
{
    deletePtr(m_server);
    deletePtr(m_acceptor);
    deletePtr(m_executor);
}

void ProxyServer::start(UInt32 af)
{
    if (!m_executor)
        m_executor = new ScheduledThreadPool(m_poolSize, nullptr);

    if (!m_acceptor)
        m_acceptor = new ProxyServerAcceptor(this, m_executor);

    if (!m_server)
        m_server = new NetServer(m_port, m_acceptor);

    m_server->listen(af);
}

void ProxyServer::stop()
{
    m_server->close();
    m_executor->terminate();
}

void ProxyServer::send(Int32 sessionId, NetMessage *msg)
{
    FastMutexLocker locker(m_mutex);

    auto it = m_sessions.find(sessionId);
    if (it == m_sessions.end())
        O3D_ERROR(E_InvalidParameter("Session id"));

    locker.unlock();

    // and send the message
    it->second->send(msg);
}

void ProxyServer::multicast(NetMessage *msg)
{
    FastMutexLocker locker(m_mutex);

    for (std::pair<Int32, ProxyServerSession*> entry : m_sessions)
    {
        entry.second->send(msg);
    }
}

o3d::UInt32 ProxyServer::getNumSessions() const
{
    FastMutexLocker locker(m_mutex);
    return (o3d::UInt32)m_sessions.size();
}

o3d::Int32 ProxyServer::getNextId()
{
    FastMutexLocker locker(m_mutex);
    return m_ids.getID();
}

void ProxyServer::releaseID(o3d::Int32 id)
{
    FastMutexLocker locker(m_mutex);
    m_ids.releaseID(id);
}

void ProxyServer::setVersion(o3d::Int32 version)
{
     FastMutexLocker locker(m_mutex);
     m_version = version;
}

void ProxyServer::setCertificate(const SmartArrayUInt8 &cert)
{
    FastMutexLocker locker(m_mutex);
    m_certificate = cert;
}

void ProxyServer::terminateSession(o3d::Int32 sessionId)
{
    FastMutexLocker locker(m_mutex);

    auto it = m_sessions.find(sessionId);
    if (it == m_sessions.end())
        O3D_ERROR(E_InvalidParameter("Session id"));

    // cancel the session if not already done
    it->second->cancel();
}

o3d::Int32 ProxyServer::schedule(ProxyServerSession *session)
{
    FastMutexLocker locker(m_mutex);

    const Int32 id = m_ids.getID();

    m_executor->schedule(session, 0, m_delay, m_timeUnit);
    m_sessions.insert(std::make_pair(id, session));

    return id;
}

void ProxyServer::removeSession(o3d::Int32 sessionId)
{
    FastMutexLocker locker(m_mutex);

    auto it = m_sessions.find(sessionId);
    if (it == m_sessions.end())
        O3D_ERROR(E_InvalidParameter("Session id"));

    // id
    m_ids.releaseID(sessionId);

    // erase it
    m_sessions.erase(it);
}

//
// ProxyServerAcceptor
//
ProxyServerAcceptor::ProxyServerAcceptor(
        ProxyServer *proxyServer,
        o3d::ScheduledThreadPool *executor) :
    m_proxyServer(proxyServer),
    m_executor(executor)
{

}

void ProxyServerAcceptor::accepted(Socket *client)
{
    O3D_MESSAGE("Accept a proxy client from " + client->getHostAddress());

    ProxyServerSession *session = new ProxyServerSession(m_proxyServer, client);

    Int32 id = m_proxyServer->schedule(session);
    if (id >= 0)
        session->setId(id);
    else
        deletePtr(session);

    // should receive the certificate shortly
}

//
// ProxyServerSession
//
ProxyServerSession::ProxyServerSession(ProxyServer *proxyServer, Socket *client) :
    m_proxyServer(proxyServer),
    m_id(-1),
    m_netSession(nullptr),
    m_cancel(False),
    m_valid(False)
{
    O3D_ASSERT(m_proxyServer != nullptr);

    if (m_proxyServer->getReadWriteAdapter() == nullptr)
        m_netSession = new NetSession(
                    client,
                    m_proxyServer->getNetMessageFactory(),
                    new DefaultNetMessageAdapter());
    else
        m_netSession = new NetSession(
                    client,
                    m_proxyServer->getNetMessageFactory(),
                    m_proxyServer->getReadWriteAdapter());

    const UInt32 cha[] = { (UInt32)rand(), (UInt32)rand(), (UInt32)rand(), (UInt32)rand() };
    m_challenge.allocate(16);

    memcpy(m_challenge.getData(), cha, 16);

    // Version and challenge for the client
    ProxyChallengeOut *challenge = new ProxyChallengeOut;
    challenge->setVersion(m_proxyServer->getVersion());
    challenge->setChallenge((UInt8*)cha);

    m_netSession->pushMessage(challenge);
}

ProxyServerSession::~ProxyServerSession()
{
    deletePtr(m_netSession);
}

Int32 ProxyServerSession::run(void *context)
{
    // manualy cancel the task (deleted when returns -1).
    if (m_cancel)
    {
        m_proxyServer->removeSession(m_id);
        return -1;
    }

    if (m_netSession)
    {
        m_netSession->run(nullptr);

        // shutdown...
        if (!m_netSession->isReady())
        {
            m_proxyServer->removeSession(m_id);

            // -1 cancel the task (deleted when returns -1).
            return -1;
        }

        // TODO may we use a while ?
        NetMessage *message = m_netSession->popMessage();
        if (message)
            try {
            message->run(this);

            // delete if zero is reached
            if (message->consume())
                deletePtr(message);

        } catch(E_RunMessage &e)
        {
            m_proxyServer->removeSession(m_id);

            // and delete it (returns -1)
            return -1;
        }
    }

    return 0;
}

void ProxyServerSession::cancel()
{
    // TODO what about not runned message, and no sent messages,
    // and what about on the other side of the not sent messages ?
    m_cancel = True;
}

