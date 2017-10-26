/**
 * @file netsession.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/netsession.h"
#include "o3d/net/netbuffer.h"
#include <o3d/core/debug.h>

using namespace o3d;
using namespace net;

NetSession::NetSession(Socket *socket,
        NetMessageFactory* messageFactory,
        NetReadWriteAdapter* adapter,
        Int32 readTimeout) :
    m_socket(socket),
    m_readTimeout(readTimeout),
    m_shutdown(False),
    m_shutdownCause(SHUTDOWN_CAUSE_UNKNOW),
    m_readPendingMessage(nullptr),
    m_writePendingMessage(nullptr),
    m_nextState(1),
    m_currentState(0)
{
    O3D_CHECKPTR(messageFactory);

    m_readBuffer = new ArrayNetBuffer(2048);
    m_writeBuffer = new ArrayNetBuffer(2048);

    m_messageFactory = messageFactory;
    m_outgoingList = new PCQueue<NetMessage*> ();
    m_incomingList = new PCQueue<NetMessage*> ();
    m_readWriteAdapter = adapter;
}

NetSession::~NetSession()
{
    if (m_socket)
        deletePtr(m_socket);

    deletePtr(m_readBuffer);
    deletePtr(m_writeBuffer);

    NetMessage* message;
    while ((message = popIncomingMessage()) != nullptr)
    {
        deletePtr(message);
    }

    while ((message = popOutgoingMessage()) != nullptr)
    {
        deletePtr(message);
    }

    if (m_writePendingMessage != nullptr)
    {
        deletePtr(m_writePendingMessage);
    }

    if (m_readPendingMessage != nullptr)
    {
        deletePtr(m_readPendingMessage);
    }

    deletePtr(m_outgoingList);
    deletePtr(m_incomingList);
}

void NetSession::shutdown(const String &cause)
{
    if (m_socket)
    {
        deletePtr(m_socket);

        m_shutdownCause = SHUTDOWN_CAUSE_UNKNOW;
        m_shutdown = True;
    }
}

void NetSession::shutdown(const String &cause, NetSession::ShutdownCause id)
{
    if (m_socket)
    {
        deletePtr(m_socket);

        O3D_MESSAGE(String("Shutdown Request : NetSession::Shutdown ") + cause);

        m_shutdownCause = id;
        m_shutdown = True;
    }
}

void NetSession::pushMessage(NetMessage *message)
{
    pushOutgoingMessage(message);
}

NetMessage* NetSession::popMessage()
{
    return popIncomingMessage();
}

void NetSession::execute(NetMessage *message)
{
    pushIncomingMessage(message);
}

Int32 NetSession::run(void *data)
{
    if (!m_socket)
        return -1;

    if (m_nextState != m_currentState)
    {
        m_currentState = m_nextState;
        switch (m_nextState)
        {
            case (1):
            {
                // Fetch some data to the client (default is 10ms)
                m_socket->setReadTimeout(m_readTimeout);

                // Byte Order
                ArrayNetBuffer byteOrderBuffer(4);
                byteOrderBuffer.writeInt32(1);
                byteOrderBuffer.flip();

                // Send an Int32 set to 1
                m_socket->send(
                            byteOrderBuffer.getBuffer(),
                            4,
                            (Int32) MSG_WAITALL);

                m_nextState = 2;
            }
            break;

            case (2):
                if (!m_socket->setNonBlocking())
                {
                    m_shutdown = True;
                    m_shutdownCause = SHUTDOWN_INTERNAL_ERROR;
                }
                else
                {
                    O3D_MESSAGE("NetSession : process data exchange");
                }
            break;
        }
    }

    switch (m_currentState)
    {
    case (2):
        try {
            handleRead();
            handleWrite();
        }
        catch (E_SocketError &exception)
        {
            O3D_WARNING(exception.getMsg());
            m_shutdown = True;
            m_shutdownCause = SHUTDOWN_SOCKET_CLOSED;
        }
        catch (E_FactoryError &exception)
        {
            O3D_WARNING(exception.getMsg());
            m_shutdown = True;
            m_shutdownCause = SHUTDOWN_INTERNAL_ERROR;
        }

        break;
    default:
        break;
    }

    return 0;
}

NetReadWriteAdapter *NetSession::getReadWriteAdapter()
{
    return m_readWriteAdapter;
}

void NetSession::deleteReadWriteAdapter()
{
    deletePtr(m_readWriteAdapter);
}

void NetSession::pushIncomingMessage(NetMessage* message)
{
    O3D_CHECKPTR(message);
    m_incomingMutex.lock();
    m_incomingList->push(message);
    m_incomingMutex.unlock();

}

NetMessage* NetSession::popIncomingMessage()
{
    NetMessage* message;
    m_incomingMutex.lock();
    message = m_incomingList->pop();
    m_incomingMutex.unlock();
    return message;
}

void NetSession::pushOutgoingMessage(NetMessage* message)
{
    O3D_CHECKPTR(message);
    m_outgoingMutex.lock();
    m_outgoingList->push(message);
    m_outgoingMutex.unlock();
}

NetMessage* NetSession::popOutgoingMessage()
{
    NetMessage* message;
    m_outgoingMutex.lock();
    message = m_outgoingList->pop();
    m_outgoingMutex.unlock();
    return message;
}

void NetSession::handleRead()
{
    if (m_socket->receiveIntoBuffer(m_readBuffer, 0) > 0)
    {
        // A message is waiting for additional data
        if (m_readPendingMessage != nullptr)
        {
            NetMessage* pending;
            if (m_readWriteAdapter != nullptr)
            {
                pending = m_readWriteAdapter->readFrom(m_readBuffer, m_readPendingMessage);
            }
            else
                pending = m_readPendingMessage->readFromBuffer(m_readBuffer);

            if (pending == nullptr)
            {
                // Message is ready to be processed
                pushIncomingMessage(m_readPendingMessage);
                m_readPendingMessage = nullptr;
            }
            else
            {
                m_readPendingMessage = pending; // in case where the message do not return himself
            }
        }

        while ((m_readPendingMessage == nullptr) && (m_readBuffer->getAvailable() > 0))
        {
            NetMessage* message = m_messageFactory->buildFromBuffer(m_readBuffer);
            if (message != nullptr)
            {
                if (m_readWriteAdapter != nullptr)
                {
                    m_readPendingMessage = m_readWriteAdapter->readFrom(
                            m_readBuffer,
                            message);
                }
                else
                    m_readPendingMessage = message->readFromBuffer(m_readBuffer);

                if (m_readPendingMessage == nullptr)
                {
                    // Message is ready to be processed
                    pushIncomingMessage(message);
                }
            }
        }
        if ((m_readPendingMessage == nullptr) && (m_readBuffer->getAvailable() > 0))
        {
            O3D_WARNING("There is no pending read message and there is still data on read buffer");
        }
        m_readBuffer->compact();
    }
}

void NetSession::handleWrite()
{
    NetMessage* message;

    while ((message = popOutgoingMessage(), message != nullptr) && (m_writeBuffer->getFree() > 2))
    {
        if (m_readWriteAdapter != nullptr)
        {
            m_readWriteAdapter->writeTo(m_writeBuffer, message);
        }
        else
            message->writeToBuffer(m_writeBuffer);

        if (message->consume())
            deletePtr(message);
    }

    if (m_writeBuffer->getAvailable() > 0)
    {
        m_socket->sendFromBuffer(m_writeBuffer, 0);
    }
    m_writeBuffer->compact();
}

Bool NetSession::isReady()
{
    return (!m_shutdown);
}

