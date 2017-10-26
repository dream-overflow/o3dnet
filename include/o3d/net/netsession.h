/**
 * @file netsession.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-11-29
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_NETSESSION_H
#define _O3D_NETSESSION_H

#include "socket.h"
#include "netmessageadapter.h"
#include "netreadwriteadapter.h"

#include <o3d/core/runnable.h>
#include <o3d/core/mutex.h>

namespace o3d {
namespace net {

/**
 * @brief NetSession A runnable session with input and output socket channel and message processing.
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-11-29
 */
class O3D_NET_API NetSession : public Runnable
{
public:

    enum ShutdownCause
    {
        SHUTDOWN_CAUSE_UNKNOW = 0,
        SHUTDOWN_SOCKET_CLOSED,
        SHUTDOWN_SOCKET_SHUTDOWN,
        SHUTDOWN_INTERNAL_ERROR
    };

    /**
     * @brief NetSession
     * @param socket A connected socket
     * @param messageFactory A valid message factory
     * @param adapter A valid message read and write adapter
     * @param readTimeout Socket read time out in microseconds
     */
    NetSession(
            Socket *socket,
            NetMessageFactory *messageFactory,
            NetReadWriteAdapter *adapter = nullptr,
            Int32 readTimeout = 10000);

    virtual ~NetSession();

    /**
     * @brief shutdown Close socket and cancel run
     * @param cause
     */
    void shutdown(const String &cause);

    /**
     * @brief shutdown Close socket and cancel run
     * @param cause
     * @param id
     */
    void shutdown(const String &cause, ShutdownCause id);

    virtual Int32 run(void *);

    //! @return True if client is connected, configured and can process messages
    Bool isReady();

    //! push a message to the server.
    void pushMessage(NetMessage* message);

    //! pop the next message ready to be run.
    NetMessage* popMessage();

    //! push a message for local execution
    void execute(NetMessage* message);

    //! get the message adapter or null.
    NetReadWriteAdapter* getReadWriteAdapter();

    //! delete the read/write adapter.
    void deleteReadWriteAdapter();

private:

    //! Simple One Producer and One Consumer Queue
    template<typename T>
    class PCQueue
    {
    public:

        void push(T element)
        {
            Int32 next = m_write + 1;
            if (next >= m_size)
                next = 0;
            if (next != m_read)
            {
                m_elements[next] = element;
                m_write = next;
            } else
            {
                // Must throw an exception
            }
        }

        T pop()
        {
            if (m_read == m_write)
                return nullptr;
            Int32 next = m_read + 1;
            if (next >= m_size)
                next = 0;
            T element = m_elements[next];
            m_read = next;
            return element;
        }

    private:

        volatile Int32 m_read;
        volatile Int32 m_write;
        static const Int32 m_size = 50;
        volatile T m_elements[m_size];
    };

    Socket *m_socket;
    Int32 m_readTimeout;

    FastMutex m_incomingMutex;
    FastMutex m_outgoingMutex;

    NetReadWriteAdapter *m_readWriteAdapter;

    Bool m_shutdown;

    ShutdownCause m_shutdownCause;

    ArrayNetBuffer* m_readBuffer;
    ArrayNetBuffer* m_writeBuffer;

    NetMessageFactory* m_messageFactory;

    NetMessage* m_readPendingMessage;
    NetMessage* m_writePendingMessage;

    PCQueue<NetMessage*>* m_outgoingList;
    PCQueue<NetMessage*>* m_incomingList;

    Int32 m_nextState;
    Int32 m_currentState;

private:

    void pushIncomingMessage(NetMessage* message);
    NetMessage* popIncomingMessage();

    void pushOutgoingMessage(NetMessage* message);
    NetMessage* popOutgoingMessage();

    void handleRead();
    void handleWrite();
};

} // namespace net
} // namespace o3d

#endif // _O3D_NETSESSION_H
