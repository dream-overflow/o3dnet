/**
 * @file genericmessagein.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_GENERICMESSAGEIN_H
#define _O3D_GENERICMESSAGEIN_H

#include "netmessageadapter.h"

namespace o3d {
namespace net {

/**
 * @brief Instancied when the message type is unknown.
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-07-21
 */
class GenericMessageIn : public NetMessageInHelper<GenericMessageIn, 0xFFFF>
{
public:

    GenericMessageIn();
    virtual ~GenericMessageIn();

    virtual NetMessage* readFromBuffer(NetBuffer* buffer);

    virtual void run(void *context);

    virtual void setMessageSize(UInt16 dataSize);

private:

    Int16 m_rest;
};

} // namespace net
} // namespace o3d

#endif // _O3D_GENERICMESSAGEIN_H
