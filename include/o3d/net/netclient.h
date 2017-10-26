/**
 * @file netclient.h
 * @brief Simple Network data queue management
 * @author Patrice GILBERT (patrice.gilbert@revolutioning.com)
 * @date 2009-10-24
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_NETCLIENT_H
#define _O3D_NETCLIENT_H

#include <o3d/core/base.h>

#include <o3d/core/types.h>
#include <o3d/core/evthandler.h>
#include <o3d/core/evt.h>
#include <o3d/core/runnable.h>
#include "socket.h"
#include "netmessagefactory.h"
#include "netreadwriteadapter.h"

namespace o3d {

class String;
class Thread;

namespace net {

class ArrayNetBuffer;
class NetBuffer;

/**
 * @brief The NetClient class
 */
class O3D_NET_API NetClient : public Runnable, public EvtHandler
{
public:

	/**
	 * @brief Construct but not connect a net client.
	 * @param server Server name.
	 * @param port Server port.
	 * @param messageFactory A valid message factory instance (manual delete).
	 * @param adapter Null or valid message read/write adapter instance (manual delete).
     * @param readTimeout Socket read time out in microseconds
	 */
	NetClient(
			const String& server,
            UInt16 port,
			NetMessageFactory* messageFactory,
            NetReadWriteAdapter* adapter = nullptr,
            Int32 readTimeout = 10000);

	virtual ~NetClient();

    //! Connect to the server asynchronously.
	//! @param af Address family.
    void connect(UInt32 af = AF_UNSPEC);

    //! Disconnect asynchronously.
	void disconnect();

    //! @return True if client is connected, configured and can process messages.
    Bool isReady();

    //! Shutdown connection process.
	void shutdown();

    //! push a message to the server.
	void pushMessage(NetMessage* message);

    //! pop the next message ready to be run.
	NetMessage* popMessage();

    //! push a message for local execution.
    void execute(NetMessage* message);

    Int32 run(void *data);

	//! get the message adapter or null.
	NetReadWriteAdapter* getReadWriteAdapter();

	//! delete the read/write adapter.
	void deleteReadWriteAdapter();

	//! get the address family.
    UInt32 getAf() const;

	//! get the port.
    UInt16 getPort() const;

	//! get the server address.
	const String& getServerAddress() const;

public:

	Signal<> connected{this};            //!< Connection Successful
	Signal<> disconnected{this};         //!< Connection is closed
	Signal<> connectionDenied{this};     //!< Connection cannot be established
	Signal<> connectionTimeout{this};    //!< Server is busy ?

protected:

	void pushIncomingMessage(NetMessage* message);
	NetMessage* popIncomingMessage();

	void pushOutgoingMessage(NetMessage* message);
	NetMessage* popOutgoingMessage();

	void handleRead();
	void handleWrite();

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

	String m_serverAddress; //!<
    UInt16 m_serverPort; //!<
    UInt32 m_af;
	Socket* m_socket; //!<
    Bool m_running; //!<
    Bool m_shutdown; //!<
    Int32 m_shutdownCause; //!

    Int32 m_readTimeout;

    Int32 m_currentState; //!<
    Int32 m_nextState; //!<

	Thread* m_thread; //!<

	ArrayNetBuffer* m_readBuffer; //!<
	ArrayNetBuffer* m_writeBuffer; //!<

	NetMessage* m_readPendingMessage; //!<
	NetMessage* m_writePendingMessage; //!<

	PCQueue<NetMessage*>* m_outgoingList; //!<
	PCQueue<NetMessage*>* m_incomingList; //!<

	FastMutex m_outgoingMutex; //!<
	FastMutex m_incomingMutex; //!<

	NetMessageFactory* m_messageFactory; //!<

	NetReadWriteAdapter* m_readWriteAdapter; //!<
};

} // namespace net
} // namespace o3d

#endif // _O3D_NETCLIENT_H
