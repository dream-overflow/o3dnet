/**
 * @file netreadwriteadapter.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2001-12-25
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_NETREADWRITEADAPTER_H
#define _O3D_NETREADWRITEADAPTER_H

#include "netmessage.h"

namespace o3d {
namespace net {

class ArrayNetBuffer;
class NetBuffer;

/**
 * @brief The NetReadWriteAdapter class
 */
class O3D_NET_API NetReadWriteAdapter
{
public:

    virtual ~NetReadWriteAdapter() = 0;

    virtual NetMessage* readFrom(NetBuffer* buffer, NetMessage* message) = 0;
    virtual NetMessage* writeTo(NetBuffer* buffer, NetMessage* message) = 0;
};

} // namespace net
} // namespace o3d

#endif // _O3D_NETREADWRITEADAPTER_H
