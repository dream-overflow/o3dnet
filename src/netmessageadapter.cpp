/**
 * @file netmessageadapter.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/precompiled.h"
#include <o3d/core/architecture.h>

#include "o3d/net/netmessageadapter.h"
#include "o3d/net/netbuffer.h"
#include <o3d/core/debug.h>

using namespace o3d;
using namespace o3d::net;

UInt16 AbstractNetMessage::getMessageSize() const
{
    return m_messageDataSize;
}

void AbstractNetMessage::setMessageSize(UInt16 dataSize)
{
    m_messageDataSize = dataSize;
}

String AbstractNetMessage::getDump() const
{
    return String("");
}

NetMessage* AbstractNetMessageIn::writeToBuffer(NetBuffer* buffer)
{
    return nullptr;
}


NetMessage *AbstractNetMessageOut::readFromBuffer(NetBuffer *buffer)
{
    return nullptr;
}

DefaultNetMessageAdapter::DefaultNetMessageAdapter()
{
}

DefaultNetMessageAdapter::~DefaultNetMessageAdapter()
{
}

NetMessage* DefaultNetMessageAdapter::readFrom(NetBuffer* buffer, NetMessage* message)
{
    AbstractNetMessage* m = reinterpret_cast<AbstractNetMessage*> (message);
    Int16 size = buffer->readInt16();

    //O3D_MESSAGE(String("ReadMessage ") << m->getMessageCode() << " " << (Int32)size);

    if (buffer->getAvailable() < size)
        return nullptr;

    m->setMessageSize(size);
    message->readFromBuffer(buffer);

    return nullptr;
}

NetMessage* DefaultNetMessageAdapter::writeTo(NetBuffer* buffer, NetMessage* message)
{
    AbstractNetMessage* m = reinterpret_cast<AbstractNetMessage*>(message);
    Int16 size = m->getMessageSize();

    // we need at laest size + 2 bytes of message size + [1..4] bytes of message code
    if (buffer->getFree() < size + 6)
    {
        return message;
    }

    // multi-byte message code (like for UTF8)
    UInt32 c = m->getMessageCode();

    if (c < 0x80)
    {
        // 0xxxxxxx
        buffer->writeInt8(static_cast<Int8>(c));
    }
    else if (c < 0x800)
    {
        // C0          80
        // 110xxxxx 10xxxxxx
        buffer->writeInt8(static_cast<Int8>(0xC0 | (c >> 6)));
        buffer->writeInt8(static_cast<Int8>(0x80 | (c & 0x3F)));
    }
    else if (c < 0x8000)
    {
        // E0       80       80
        // 1110xxxx 10xxxxxx 10xxxxxx
        buffer->writeInt8(static_cast<Int8>(0xE0 | (c >> 12)));
        buffer->writeInt8(static_cast<Int8>(0x80 | (c >> 6 & 0x3F)));
        buffer->writeInt8(static_cast<Int8>(0x80 | (c & 0x3F)));
    }
    else
    {
        // F0      80       80       80
        //11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        buffer->writeInt8(static_cast<Int8>(0xF0 | (c >> 12) >> 6));
        buffer->writeInt8(static_cast<Int8>(0x80 | (c >> 12 & 0x3F)));
        buffer->writeInt8(static_cast<Int8>(0x80 | (c >> 6 & 0x1F)));
        buffer->writeInt8(static_cast<Int8>(0x80 | (c & 0x3F)));
    }

    // size
    buffer->writeInt16(size);

    Int32 start = buffer->getLimit();
    message->writeToBuffer(buffer);

    Int32 stop = buffer->getLimit();
    if ((stop - start) != size)
        O3D_WARNING(String("Invalid Message Size detected ") << m->getDump() << " " << (stop - start) << " " << size);

    return nullptr;
}

Bool o3d::net::AbstractNetMessage::consume()
{
    O3D_ASSERT(m_consume >= 1);
    --m_consume;

    return m_consume == 0;
}

