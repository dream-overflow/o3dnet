/**
 * @file netserver.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-11-28
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_NETSERVER_H
#define _O3D_NETSERVER_H

#include <o3d/core/thread.h>
#include "socket.h"
#include "netmessageadapter.h"

namespace o3d {
namespace net {

/**
 * @brief
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-11-28
 */
class O3D_NET_API NetSessionAcceptor
{
public:

    virtual ~NetSessionAcceptor() = 0;

    /**
     * @brief accept Called once a socket is created.
     * @param client A valid socket to the client.
     */
    virtual void accepted(Socket *client) = 0;
};


/**
 * @brief NetServer Listen and accept incoming connections using an acceptor.
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-11-28
 */
class O3D_NET_API NetServer : public Runnable, public EvtHandler
{
public:

    /**
     * @brief NetServer
     * @param port
     * @param acceptor A valid acceptor, you have to delete it.
     */
    NetServer(
            UInt16 port,
            NetSessionAcceptor *acceptor);

    virtual ~NetServer();

    /**
     * @brief listen  Bind and listen on the socket.
     * @param af Address family.
     */
    void listen(UInt32 af = AF_INET6);

    //! Stop to listen asynchronously and close the socket.
    void close();

    //! @return True if server is listening, configured and can process messages
    Bool isReady();

    virtual Int32 run(void *data);

    //! get the address family.
    UInt32 getAf() const;

    //! get the port.
    UInt16 getPort() const;

private:

    enum State
    {
        STATE_UNACTIVE,
        STATE_STARTING,
        STATE_LISTENING,
        STATE_STOPPING
    };

    UInt16 m_port;
    UInt32 m_af;

    NetSessionAcceptor *m_acceptor;

    Socket *m_socket;
    Thread *m_thread;
    FastMutex m_mutex;

    State m_state;
    Bool m_running;
};

} // namespace net
} // namespace o3d

#endif // _O3D_NETSERVER_H
