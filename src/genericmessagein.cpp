/**
 * @file genericmessagein.cpp
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/precompiled.h"
#include <o3d/core/architecture.h>

#include <o3d/net/genericmessagein.h>
#include <o3d/net/netbuffer.h>
#include <o3d/core/debug.h>

using namespace o3d;
using namespace o3d::net;

GenericMessageIn::GenericMessageIn() :
    m_rest(0)
{
}

GenericMessageIn::~GenericMessageIn()
{
}

void GenericMessageIn::setMessageSize(UInt16 dataSize)
{
    m_messageDataSize = dataSize;
    m_rest = m_messageDataSize;
}

NetMessage* GenericMessageIn::readFromBuffer(NetBuffer* buffer)
{
//    O3D_MESSAGE("Read GenericMessageIn");

    if (buffer->getAvailable() < m_rest)
    {
        Int16 len = buffer->getAvailable();
        m_rest -= buffer->getAvailable();

        if (len > 0)
            buffer->setPosition(buffer->getPosition() + len);

        return this;
    }
    else
    {
        buffer->setPosition(buffer->getPosition() + m_rest);
    }

    return nullptr;
}

void GenericMessageIn::run(void *context)
{
//    O3D_MESSAGE("Run GenericMessageIn");
}

