/**
 * @file netserver.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/precompiled.h"
#include "o3d/core/architecture.h"
#include "o3d/net/netserver.h"

using namespace o3d;
using namespace net;

NetServer::NetServer(UInt16 port, NetSessionAcceptor *acceptor) :
    m_port(port),
    m_af(0),
    m_acceptor(acceptor),
    m_socket(nullptr),
    m_thread(nullptr),
    m_state(STATE_UNACTIVE),
    m_running(False)
{
    O3D_ASSERT(acceptor);
}

NetServer::~NetServer()
{
}

void NetServer::listen(UInt32 af)
{
    m_state = STATE_STARTING;
    m_af = af;

    m_thread = new Thread(this);
    m_thread->start();
}

void NetServer::close()
{
    if (m_running)
    {
        m_running = False;

        m_thread->waitFinish();

        deletePtr(m_thread);

        m_state = STATE_UNACTIVE;
    }
}

Bool NetServer::isReady()
{
    return (m_state == STATE_LISTENING);
}

Int32 NetServer::run(void *data)
{
    Bool run = m_running = True;

    // create and bind the socket
    if (m_af == AF_INET)
        m_socket = new Socket(new SockAddr4("", m_port, SOCK_STREAM, m_af));
    else
        m_socket = new Socket(new SockAddr6("", m_port, SOCK_STREAM, m_af));
    m_socket->socket();

    if (!m_socket->bind())
    {
        deletePtr(m_socket);
        return -1;
    }

    // queue of 128 connections
    if (!m_socket->listen(SOMAXCONN))
    {
        deletePtr(m_socket);
        return -1;
    }

    m_state = STATE_LISTENING;

    while (run)
    {
        // verify if the server can accept 1 or more client connections (10000us timeout)
        if (m_socket->select(10000) >= 1)
        {
            // acceptor
            Socket *client = nullptr;

            try {
                client = Socket::accept(*m_socket);
            }
            catch (E_SocketNotCreated &e)
            {
                client = nullptr;
            }

            if (client)
            {
                m_acceptor->accepted(client);
            }
        }

        run = m_running;
    }

    m_state = STATE_STOPPING;

    deletePtr(m_socket);

    return 0;
}

UInt32 NetServer::getAf() const
{
    return m_af;
}

UInt16 NetServer::getPort() const
{
    return m_port;
}

NetSessionAcceptor::~NetSessionAcceptor()
{

}

