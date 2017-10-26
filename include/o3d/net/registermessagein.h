/**
 * @file registermessagein.h
 * @brief 
 * @author Patrice GILBERT (patrice.gilbert@revolutining.com)  
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_REGISTERNETMESSAGEIN_H
#define _O3D_REGISTERNETMESSAGEIN_H

#include "netmessageadapter.h"
#include "defaultnetmessagefactory.h"

namespace o3d {
namespace net {

/**
 * @brief Helper to register a message.
 * Usage: in a cpp file do RegisterNetMessageIn<MessageClassNameIn>::R inst(factory);
 * @date 2013-07-21
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 */
template <class T>
struct RegisterNetMessageIn
{
    RegisterNetMessageIn(NetMessageFactory *factory)
    {
        AbstractNetMessageIn *msg = T::createInstance();
        factory->registerMsg(msg);
    }

    typedef RegisterNetMessageIn R;
};

} // namespace net
} // namespace o3d

#endif // _O3D_REGISTERNETMESSAGEIN_H
