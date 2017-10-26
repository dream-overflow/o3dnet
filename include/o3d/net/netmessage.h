/**
 * @file netmessage.h
 * @brief Base class for network messaging
 * @author Patrice GILBERT (patrice.gilbert@revolutioning.com)
 * @date 2009-10-27
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_NETMESSAGE_H
#define _O3D_NETMESSAGE_H

#include "net.h"
#include <o3d/core/error.h>

#ifdef O3D_WINDOWS
#include <o3d/core/architecture.h>
#endif

namespace o3d {
namespace net {

class NetBuffer;

/**
 * @brief E_RunMessage invoke an error during run a net message.
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-12-04
 */
class O3D_NET_API E_RunMessage : public E_BaseException
{
    O3D_E_DEF_CLASS(E_RunMessage)

    //! Ctor
    E_RunMessage(const String& msg) throw() : E_BaseException(msg)
        O3D_E_DEF(E_RunMessage,"Error during message run")
};


/**
 * @brief The NetMessage class
 */
class O3D_NET_API NetMessage
{
public:

    virtual ~NetMessage()
    {
	}

	//! Read message data from buffer
	//! Return this message if need additional data
	virtual NetMessage* readFromBuffer(NetBuffer* buffer) = 0;

	//! Write message data from buffer
	//! Return this message if buffer is too small
	virtual NetMessage* writeToBuffer(NetBuffer* buffer) = 0;

	//! Convenient method to allow application to process message
    virtual void run(void *context)
    {
	}

    /** Called when a message is consumed. That mean, just after a run for read message,
     * and just after a write for a write message.
     * Default always returns true.
     * @return True if the message can be deleted, else false.
     */
    virtual Bool consume()
    {
        return True;
	}
};

} // namespace net
} // namespace o3d

#endif // _O3D_NETMESSAGE_H
