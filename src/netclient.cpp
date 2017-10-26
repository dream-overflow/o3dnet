/**
 * @file netclient.cpp
 * @brief Simple Network data queue management
 * @author Patrice GILBERT (patrice.gilbert@revolutioning.com)
 * @date 2009-10-24
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/precompiled.h"
#include "o3d/core/architecture.h"
#include "o3d/net/netclient.h"

#include <o3d/core/thread.h>
#include "o3d/net/netbuffer.h"
#include <o3d/core/logger.h>
#include <o3d/core/debug.h>
#include <stdio.h>

#ifdef _MSC_VER
#ifdef MSG_WAITALL
#undef MSG_WAITALL
#endif
#define MSG_WAITALL 0
#endif

using namespace o3d;
using namespace o3d::net;

NetClient::NetClient(const String& serverAddress,
        UInt16 port,
        NetMessageFactory* messageFactory,
        NetReadWriteAdapter* adapter,
        Int32 readTimeout) :
            m_socket(nullptr),
			m_running(False),
            m_readTimeout(readTimeout),
            m_readPendingMessage(nullptr),
            m_writePendingMessage(nullptr)
{
	O3D_CHECKPTR(messageFactory);

	m_readBuffer = new ArrayNetBuffer(2048);
	m_writeBuffer = new ArrayNetBuffer(2048);

	m_messageFactory = messageFactory;
	m_serverAddress = serverAddress;
	m_af = AF_UNSPEC;
	m_serverPort = port;
	m_currentState = -1;
	m_nextState = 0;
	m_outgoingList = new PCQueue<NetMessage*> ();
	m_incomingList = new PCQueue<NetMessage*> ();
	m_thread = new Thread(this);
	m_readWriteAdapter = adapter;
}

NetClient::~NetClient()
{
	if (m_running)
	{
		shutdown();
		m_thread->waitFinish();
	}

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

	deletePtr(m_thread);
}

void NetClient::connect(UInt32 af)
{
	if (!m_running)
	{
		m_af = af;
		m_currentState = -1;
		m_nextState = 1;

        m_thread->start();
	}
}

void NetClient::disconnect()
{
	if (m_running)
	{
        m_nextState = 2;
	}
}

void NetClient::shutdown()
{
	if (m_running)
	{
		O3D_MESSAGE(String("Shutdown Request : NetClient::Shutdown"));
		m_shutdownCause = 0;   // must be set before m_shutdown because there is no mutex
		m_shutdown = True;
	}
}

void NetClient::pushMessage(NetMessage *message)
{
	pushOutgoingMessage(message);
}

NetMessage* NetClient::popMessage()
{
	return popIncomingMessage();
}

void NetClient::execute(NetMessage *message)
{
    pushIncomingMessage(message);
}

Int32 NetClient::run(void *data)
{
	if (m_running)
	{
		return 0;
	}

    m_readPendingMessage = nullptr;
    m_writePendingMessage = nullptr;

	O3D_MESSAGE("NetClient 1.0.0 : Running ...");

	m_shutdown = False;
	m_running = True;
	m_shutdownCause = 0;

	while (!m_shutdown)
	{
		if (m_nextState != m_currentState)
		{
			m_currentState = m_nextState;
			switch (m_nextState)
			{
				case (1):
					O3D_MESSAGE("NetClient : Try to connect");
					try
					{
                        m_socket = Socket::connect(
										m_serverAddress,
										m_serverPort,
                                        m_af);

                        if (m_socket != nullptr)
						{					
							// Fetch some data from the server

                            Int32 serverByteOrder = 0x0;

                            // wait 2 second for the byte order
                            m_socket->setReadTimeout(2000000);

							// Byte Order server
                            // Receive an Int32 set to 1
                            Int32 nbByteReceived = m_socket->receive(
                                    reinterpret_cast<UInt8*>(&serverByteOrder),
									4,
                                    (Int32) MSG_WAITALL);

							O3D_MESSAGE(String::print("Server byte order value(%i) size(%i)",
											serverByteOrder, nbByteReceived));

							if (nbByteReceived != 4)
							{
								O3D_WARNING(String::print("Server byte order message size is invalid"));

                                m_shutdown = True;
								m_shutdownCause = 2;
							}
							else
							{
								if (serverByteOrder != 1)
								{
									if (System::getNativeByteOrder() == System::ORDER_LITTLE_ENDIAN)
									{
										m_readBuffer->setByteOrder(System::ORDER_BIG_ENDIAN);
										m_writeBuffer->setByteOrder(System::ORDER_BIG_ENDIAN);

										O3D_MESSAGE(String::print("Server byte order is set to big endian"));
									}
									else if (System::getNativeByteOrder() == System::ORDER_BIG_ENDIAN)
									{
										m_readBuffer->setByteOrder(System::ORDER_LITTLE_ENDIAN);
										m_writeBuffer->setByteOrder(System::ORDER_LITTLE_ENDIAN);

										O3D_MESSAGE(String::print("Server byte order is set to little endian"));
									}
								}

                                m_socket->setReadTimeout(m_readTimeout);
								m_nextState = 3;
							}
						}
						else
						{
							O3D_WARNING("NetClient : Socket creation failed");
                            m_shutdown = True;
							m_shutdownCause = 2;
						}
					}
					catch (E_BaseException &exception)
					{
                        m_shutdown = True;
						m_shutdownCause = 2;
					}
					break;
				case (3):						
					if (!m_socket->setNonBlocking())
					{						
                        m_shutdown = True;
						m_shutdownCause = 2;
					}
					else
					{
						O3D_MESSAGE("NetClient : process data exchange");
						connected();
					}
					break;
			}
		}

		switch (m_currentState)
		{
			case (3):
				try
                {
                    handleRead();
                    handleWrite();
                    System::waitMs(10);
				}
                catch (E_SocketError &exception)
				{
					O3D_WARNING(exception.getMsg());
					m_shutdown = True;
					m_shutdownCause = 1;
				}
                catch (E_FactoryError &exception)
				{
					O3D_WARNING(exception.getMsg());
					m_shutdown = True;
					m_shutdownCause = 1;
				}

				break;
			default:
                System::waitMs(10);
				break;
		}

	}

	O3D_MESSAGE("Net Client : Shutdown ...");

    if (m_socket != nullptr)
	{
		m_socket->close();
		deletePtr(m_socket);
	}

	switch (m_shutdownCause)
	{
		case (0):
			O3D_MESSAGE("Net Client : Shutdown request by application");
			break;
		case (1):
			O3D_MESSAGE("Net Client : Shutdown cause : disconnected");
			disconnected();
			break;
		case (2):
			O3D_MESSAGE("Net Client : Shutdown cause : connection denied");
			connectionDenied();
			break;
	}

	m_running = False;
	return 0;
}

NetReadWriteAdapter *NetClient::getReadWriteAdapter()
{
	return m_readWriteAdapter;
}

void NetClient::deleteReadWriteAdapter()
{
	deletePtr(m_readWriteAdapter);
}

UInt32 NetClient::getAf() const
{
	return m_af;
}

UInt16 NetClient::getPort() const
{
	return m_serverPort;
}

const String &NetClient::getServerAddress() const
{
	return m_serverAddress;
}

void NetClient::pushIncomingMessage(NetMessage* message)
{
	O3D_CHECKPTR(message);
	m_incomingMutex.lock();
	m_incomingList->push(message);
	m_incomingMutex.unlock();

}

NetMessage* NetClient::popIncomingMessage()
{
	NetMessage* message;
	m_incomingMutex.lock();
	message = m_incomingList->pop();
	m_incomingMutex.unlock();
	return message;
}

void NetClient::pushOutgoingMessage(NetMessage* message)
{
	O3D_CHECKPTR(message);
	m_outgoingMutex.lock();
	m_outgoingList->push(message);
	m_outgoingMutex.unlock();
}

NetMessage* NetClient::popOutgoingMessage()
{
	NetMessage* message;
	m_outgoingMutex.lock();
	message = m_outgoingList->pop();
	m_outgoingMutex.unlock();
	return message;
}

void NetClient::handleRead()
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

void NetClient::handleWrite()
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

		//message->consume();
		deletePtr(message);
	}

	if (m_writeBuffer->getAvailable() > 0)
	{
		m_socket->sendFromBuffer(m_writeBuffer, 0);
	}
    m_writeBuffer->compact();
}

Bool NetClient::isReady()
{
    return (m_shutdown == False) && (m_currentState == 3);
}

