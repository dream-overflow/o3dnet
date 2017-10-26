/**
 * @file main1.cpp
 * @brief Main entry of the network test.
 * @author Patrice GILBERT (patrice.gilbert@revolutioning.com)
 * @date 2009-10-24
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include <o3d/core/architecture.h>
#include <o3d/core/base.h>
#include <o3d/core/runnable.h>
#include <o3d/core/main.h>
#include "o3d/net/netclient.h"
#include "o3d/net/netmessage.h"
#include "o3d/net/netbuffer.h"
#include <iostream>

using namespace o3d;
using namespace o3d::net;

// Adapter
class IncomingMessage: public NetMessage
{
public:
	NetMessage* writeToBuffer(NetBuffer *buffer)
	{
		// Nothing to do
		return NULL;
	}
};

class ChangeApplicationState : public IncomingMessage
{
private:

	Int8 m_state;
	Char m_message[50];

public:
	NetMessage* readFromBuffer(NetBuffer* buffer)
	{
		if (buffer->getAvailable() >= 1)
		{
			m_state = buffer->readInt8();
			Int16 size = buffer->readInt16();
			if (buffer->readUTF8(m_message, size))
				return NULL;
			return this;
		} else
			return this;
	}

    virtual void run()
	{
		std::cout << "Change application state" << std::endl;
	}
};

class ObjectState: public IncomingMessage
{
private:

	Int32 m_objectID;
	Int32 m_direction;
	Int32 m_x;
	Int32 m_y;
	Int32 m_z;

public:

	NetMessage* readFromBuffer(NetBuffer* buffer)
	{
		if (buffer->getAvailable() >= 20)
		{
			m_direction = buffer->readInt32();
			m_objectID = buffer->readInt32();
			m_x = buffer->readInt32();
			m_y = buffer->readInt32();
			m_z = buffer->readInt32();
			return NULL;
		} else
			return this;
	}

    virtual void run(void* context)
	{
		std::cout << "Running Object State" << std::endl;
	}

};

class Challenge : public IncomingMessage
{
public:

	NetMessage* readFromBuffer(NetBuffer* buffer)
	{
		if (buffer->getAvailable() >= 16)
		{
			buffer->read(m_challenge, 16);
			return NULL;
		} else
			return this;
	}

    virtual void run(void* context)
	{
		std::cout << "Get challenge" << std::endl;
		for (Int32 i = 0; i < 16; ++i)
		{
			printf("%i ", m_challenge[i]);
		}
		printf("\n");
	}

private:

	UInt8 m_challenge[16];
};

class Version : public IncomingMessage
{
public:

    NetMessage* readFromBuffer(NetBuffer* buffer)
    {
        if (buffer->getAvailable() >= 2)
        {
            m_version = buffer->readInt16();
            return NULL;
        } else
            return this;
    }

    virtual void run(void* context)
    {
        std::cout << "Get version " << m_version << std::endl;
    }

private:

    Int16 m_version;
};

class TestFactory : public NetMessageFactory
{
public:

	TestFactory(){

	}

	virtual NetMessage* buildFromBuffer(NetBuffer* buffer)
	{
        Int32 messageType = buffer->readInt8();
		NetMessage* message = NULL;

        Int16 size = buffer->readInt16();

		switch (messageType)
		{
            case (0) :
                message = new Version();
                break;
			case (1) :
				message = new Challenge();
				break;
			case (2) :
				message = new ChangeApplicationState();
				break;
			case (10):
				message = new ObjectState();
				break;
			default:
				O3D_ERROR(E_FactoryError(String("Unknown message type [") << messageType << "]"));
				break;
		}

		return message;
	}

    virtual void registerMsg(AbstractNetMessageIn *msg)
    {

    }
};

class Test : public EvtHandler
{
private:

	NetClient* m_client;
	Bool m_loop;

public:
	Test()
	{
		TestFactory* messageFactory = new TestFactory();


        m_client = new NetClient("127.0.0.1", 40916, messageFactory, nullptr, 10000);

        Debug::instance()->throwMessage.connect(this, &Test::onDebugMessage);
        Debug::instance()->throwWarning.connect(this, &Test::onWarningMessage);
        m_client->connected.connect(this, &Test::onConnected);
        m_client->connectionDenied.connect(this, &Test::onConnectionDenied);
        m_client->disconnected.connect(this, &Test::onDisconnected);
	}

	void onDebugMessage(DebugInfo info)
	{
		std::cout << "MSG " <<  info.Message.toUtf8().getData() << std::endl;
	}

	void onWarningMessage(DebugInfo info){
		std::cout << "WRN " <<  info.Message.toUtf8().getData() << std::endl;

	}

	void onConnected(){
	}

	void onDisconnected(){
		m_loop = false;
	}

	void onConnectionTimeout(){
		m_loop = false;
	}

	void onConnectionDenied(){
		m_loop = false;
	}

	void Run()
	{
		m_loop = true;
		m_client->connect();
		Int32 key;

		// process all incoming data
		while (m_loop)
		{
			NetMessage* message = m_client->popMessage();
			if (message != NULL)
			{
				message->run(NULL);
				message->consume();
				deletePtr(message);
			}
            System::waitMs(100);
		}
		std::cout << "Press ENTER" << std::endl;
		std::cin >> key;
		m_client->shutdown();
		disconnect(Debug::instance());
		disconnect(m_client);
        Debug::instance()->getDefaultLog().writeFooterLog();
	}

	static Int32 main()
	{
        Debug::instance()->setDefaultLog("network1.log");
        Debug::instance()->getDefaultLog().clearLog();
        Debug::instance()->getDefaultLog().writeHeaderLog();

        Socket::init();

		Test test;
		test.Run();

        Socket::quit();

		return 0;
	}
};

O3D_CONSOLE_MAIN(Test, O3D_DEFAULT_CLASS_SETTINGS)

