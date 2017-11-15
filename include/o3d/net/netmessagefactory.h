/**
 * @file netmessagefactory.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2013-07-21
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_NET_NETMESSAGEFACTORY_H
#define _O3D_NET_NETMESSAGEFACTORY_H

#include "netmessage.h"

namespace o3d {
namespace net {

class AbstractNetMessageIn;

/**
 * @brief The NetMessageFactory class
 */
class O3D_NET_API NetMessageFactory
{
public:

    virtual ~NetMessageFactory() = 0;

    /**
     * @brief buildFromBuffer Build a message from buffer
     * @param buffer
     * @return a message instance
     * @remark Use NetBuffer to uncouple networking api from message api
     *         Message will be fully initialized by invoking readFromBuffer
     */
    virtual NetMessage* buildFromBuffer(NetBuffer* buffer) = 0;

    /**
     * @brief registerMsg Register a message to the factory using its message code.
     * @param msg A valid message with a not already registred code.
     */
    virtual void registerMsg(AbstractNetMessageIn *msg) = 0;
};

class O3D_NET_API E_FactoryError : public E_BaseException
{
    O3D_E_DEF_CLASS(E_FactoryError)

    //! Ctor
    E_FactoryError(const String& msg) : E_BaseException(msg)
        O3D_E_DEF(E_FactoryError,"Factory Error")
};

} // namespace net
} // namespace o3d

#endif // _O3D_NET_NETMESSAGEFACTORY_H
