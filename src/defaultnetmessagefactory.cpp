/**
 * @file defaultnetmessagefactory.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/precompiled.h"
#include <o3d/core/architecture.h>

#include <o3d/net/defaultnetmessagefactory.h>
#include <o3d/core/debug.h>
#include "o3d/net/netbuffer.h"
#include "o3d/net/genericmessagein.h"

using namespace o3d;
using namespace o3d::net;

NetMessageFactory::~NetMessageFactory()
{

}

DefaultNetMessageFactory::DefaultNetMessageFactory()
{
    registerMsg(new GenericMessageIn);
}

void DefaultNetMessageFactory::registerMsg(AbstractNetMessageIn *msg)
{
    UInt32 type = msg->getMessageCode();

    if (type >= m_msg.size())
        m_msg.resize(type+1, nullptr);

    if (m_msg[type] != nullptr)
        throw new std::exception();

    m_msg[type] = msg;
}

DefaultNetMessageFactory::~DefaultNetMessageFactory()
{
    for (AbstractNetMessageIn *msg : m_msg)
    {
        deletePtr(msg);
    }
}

NetMessage* DefaultNetMessageFactory::buildFromBuffer(NetBuffer* buffer)
{
    // multi-bytes message code (like UTF8)

    // first byte message byte
    Int8 c0 = buffer->readInt8();
    UInt32 code = 0;

    // F0      80       80       80
    //11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    if ((c0 & 0xFE) == 0xFE)
    {
        Int8 c1 = buffer->readInt8();
        Int8 c2 = buffer->readInt8();
        Int8 c3 = buffer->readInt8();

        code = (static_cast<UInt32>(
                    ((c0 & 7) << 18) |
                    ((c1 & 0x3F) << 12) |
                    ((c2 & 0x3F) << 6) |
                    (c3 & 0x3F)));
    }
    // E0       80       80
    // 1110xxxx 10xxxxxx 10xxxxxx
    else if ((c0 & 0xE0) == 0xE0)
    {
        Int8 c1 = buffer->readInt8();
        Int8 c2 = buffer->readInt8();

        code = (static_cast<UInt32>(
                    ((c0 & 0x0F) << 12) |
                    ((c1 & 0x3F) << 6) |
                    (c2 & 0x3F)));
    }
    // C0       80
    // 110xxxxx 10xxxxxx
    else if ((c0 & 0xC0) == 0xC0)
    {
        Int8 c1 = buffer->readInt8();

        code = (static_cast<UInt32>(
                    ((c0 & 0x1F) << 6) |
                    (c1 & 0x3F)));
    }
    // 0xxxxxxx
    else// if (c0 < 0x80)
    {
        code = (static_cast<UInt32>(c0));
    }

    if (code >= m_msg.size())
    {
        O3D_WARNING(String("Undefined net message type [") << code << "]");
        return new GenericMessageIn();
    }

    AbstractNetMessageIn *msg = m_msg[code];

    if (msg != nullptr)
        return msg->makeInstance();
    else
    {
        O3D_WARNING(String("Undefined net message type [") << code << "]");
        return new GenericMessageIn();
    }
}

