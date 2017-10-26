/**
 * @file netmessageadapter.h
 * @brief 
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2013-07-21
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_NETMESSAGEADAPTER_H
#define _O3D_NETMESSAGEADAPTER_H

#include "netmessage.h"
#include "netclient.h"

namespace o3d {
namespace net {

/**
 * @brief Abstract message class
 * @date 2013-07-21
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 */
class O3D_NET_API AbstractNetMessage : public NetMessage
{
public:

    AbstractNetMessage() :
        m_messageDataSize(0),
        m_consume(1)
    {
    }

    virtual UInt32 getMessageCode() const = 0;

    virtual UInt16 getMessageSize() const;
    virtual void setMessageSize(UInt16 dataSize);

    virtual String getDump() const;

    virtual Bool consume();

    /**
     * @brief setForMulticast Set the message for a multicast, that means it will not
     *        be deleted until the counter has not reached 0.
     * @param counter
     */
    void setForMulticast(UInt32 counter)
    {
        m_consume = counter;
    }

    /**
     * @brief setForRetransmission Set the message for a retransmission, that means it
     *        will not be deleted this time.
     */
    void setForRetransmission()
    {
        m_consume = 1;
    }

protected:

    UInt16 m_messageDataSize;
    UInt32 m_consume;
};

/**
 * @brief Abstract message class incoming
 * @date 2013-07-21
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 */
class O3D_NET_API AbstractNetMessageIn : public AbstractNetMessage
{
public:

    virtual NetMessage* writeToBuffer(NetBuffer* buffer);

    //! Make an instance of the message.
    virtual AbstractNetMessageIn* makeInstance() const = 0;
};

/**
 * @brief Abstract message class incoming helper
 * @date 2013-07-21
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 */
template <class CLASS, UInt32 CODE>
class O3D_NET_API_TEMPLATE NetMessageInHelper : public AbstractNetMessageIn
{
public:

    virtual UInt32 getMessageCode() const { return CODE; }
    static AbstractNetMessageIn* createInstance() { return new CLASS; }
    virtual AbstractNetMessageIn* makeInstance() const { return new CLASS; }
};

/**
 * @brief Abstract message class outgoing
 * @date 2013-07-21
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 */
class O3D_NET_API AbstractNetMessageOut : public AbstractNetMessage
{
public:

    virtual NetMessage* readFromBuffer(NetBuffer* buffer);
};

/**
 * @brief Abstract message class outgoing helper
 * @date 2013-07-21
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 */
template <UInt32 CODE>
class O3D_NET_API_TEMPLATE NetMessageOutHelper : public AbstractNetMessageOut
{
public:

    virtual UInt32 getMessageCode() const { return CODE; }
};

/**
 * @brief Default read/write net message adapter.
 * It use of a multi-byte message code from 1 to 4 bytes, and manage the message size
 * in a 16 bits integer. It can be used with the DefaultNetMessageFactory.
 * @date 2013-07-21
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 */
class O3D_NET_API DefaultNetMessageAdapter : public NetReadWriteAdapter
{
public:

    DefaultNetMessageAdapter();
    virtual ~DefaultNetMessageAdapter();

    virtual NetMessage* readFrom(NetBuffer* buffer, NetMessage* message);
    virtual NetMessage* writeTo(NetBuffer* buffer, NetMessage* message);
};

} // namespace net
} // namespace o3d

#endif // _O3D_NETMESSAGEADAPTER_H
