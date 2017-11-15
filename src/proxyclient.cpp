/**
 * @file proxyclient.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/precompiled.h"
#include <o3d/core/architecture.h>
#include "o3d/net/proxyclient.h"
#include "o3d/net/proxymessages.h"
#include <o3d/core/thread.h>
#include <o3d/core/architecture.h>

using namespace o3d;
using namespace o3d::net;

//
// ProxyClient
//
ProxyClient::ProxyClient(
        const String &host,
        o3d::UInt32 port,
        NetMessageFactory *messageFactory,
        NetReadWriteAdapter *messageAdapter) :
    m_port(port),
    m_host(host),
    m_cancel(False),
    m_thread(nullptr)
{
    m_netClient = new NetClient(m_host, m_port, messageFactory, messageAdapter, 10000);
}

ProxyClient::~ProxyClient()
{
    deletePtr(m_netClient);
}

void ProxyClient::connect(UInt32 af)
{
    m_netClient->connect(af);

    // and begin a new thread to run messages
    m_thread = new Thread(this);
    m_thread->start();
}

void ProxyClient::disconnect()
{
    m_netClient->disconnect();

    m_cancel = True;
    m_thread->waitFinish();
    deletePtr(m_thread);

    m_cancel = False;
}

void ProxyClient::send(NetMessage *msg)
{
    m_netClient->pushMessage(msg);
}

Int32 ProxyClient::run(void *)
{
    while (!m_cancel)
    {
        NetMessage *message = m_netClient->popMessage();
        if (message)
            try {
            message->run(this);

            // delete if zero is reached
            if (message->consume())
                deletePtr(message);

        } catch(E_RunMessage &e)
        {
        }

        System::waitMs(0);
    }

    return 0;
}

