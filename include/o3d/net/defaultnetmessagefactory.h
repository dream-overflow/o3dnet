/**
 * @file defaultnetmessagefactory.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-07-22
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_DEFAULTNETMESSAGEFACTORY_H
#define _O3D_DEFAULTNETMESSAGEFACTORY_H

#include "netmessagefactory.h"
#include <vector>

namespace o3d {
namespace net {

/**
 * @brief The Default net message factory
 * @details Register and manage the version and generic message.
 * Build message from NetBuffer according to registred net message, and using
 * a dynamique net message type from 1 to 4 bytes. It can be used with the
 * DefaultNetMessageAdapter.
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-07-22
 */
class O3D_NET_API DefaultNetMessageFactory : public NetMessageFactory
{
public:

    DefaultNetMessageFactory();

    virtual ~DefaultNetMessageFactory();
    virtual NetMessage* buildFromBuffer(NetBuffer* buffer);

    virtual void registerMsg(AbstractNetMessageIn *msg);

protected:

    std::vector<AbstractNetMessageIn*> m_msg;
};

} // namespace net
} // namespace o3d

#endif // _O3D_DEFAULTNETMESSAGEFACTORY_H
