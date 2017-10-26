/**
 * @file netbuffer.h
 * @brief Buffer interface
 * @author Patrice GILBERT (patrice.gilbert@revolutioning.com)
 * @date 2009-10-27
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_NETBUFFER_H
#define _O3D_NETBUFFER_H

#include "net.h"

#include <o3d/core/base.h>
#include <o3d/core/error.h>

namespace o3d {
namespace net {

//---------------------------------------------------------------------------------------
//! @class NetBuffer
//-------------------------------------------------------------------------------------
//! Interface for buffer
//---------------------------------------------------------------------------------------
class O3D_NET_API NetBuffer
{
public:

	virtual void writeInt8(Int8 value) = 0;
    virtual void writeUInt8(UInt8 value) = 0;

	virtual void writeInt16(Int16 value) = 0;
    virtual void writeUInt16(UInt16 value) = 0;

	virtual void writeInt32(Int32 value) = 0;
    virtual void writeUInt32(UInt32 value) = 0;

	virtual void writeInt64(Int64 value) = 0;
    virtual void writeUInt64(UInt64 value) = 0;

    virtual void writeUTF8(const String &string) = 0;
    virtual void writeUTF8(const Char* string) = 0;
	virtual void writeBool(Bool value) = 0;

	virtual void write(const UInt8* buffer, UInt32 size) = 0;

	virtual Int8 readInt8() = 0;
    virtual UInt8 readUInt8() = 0;

    virtual Int16 readInt16() = 0;
    virtual UInt16 readUInt16() = 0;

    virtual Int32 readInt32() = 0;
    virtual UInt32 readUInt32() = 0;

    virtual Int64 readInt64() = 0;
    virtual UInt64 readUInt64() = 0;

	virtual Bool read(UInt8* buffer, Int16 size) = 0;

	//! Read a string
    virtual Bool readUTF8(Char* string, Int16 size) = 0;
	virtual Bool readUTF8(String& string) = 0;
	virtual Bool readBool() = 0;

	//! Return how many byte can be read
	virtual Int32 getAvailable() const = 0;

	//! Return how many byte can be write
	virtual Int32 getFree() const = 0;

	//! get logical buffer limit (ie writePosition)
	virtual UInt32 getLimit() const = 0;

	//! set logical buffer limit (ie writePosition)
	virtual void setLimit(UInt32 limit) = 0;

	//! get buffer position (ie readPosition)
	virtual UInt32 getPosition() const = 0;

	//! set buffer position (ie writePosition)
	virtual void setPosition(UInt32 position) = 0;

	//! return buffer for bulk write
	//! equivalent to getBuffer() + getLimit();
	virtual UInt8* getWriteBuffer() = 0;

	//! return buffer
	virtual UInt8* getBuffer() = 0;

	//! optimize space inside buffer
	virtual void compact() = 0;

	//! Set read at the beginning
	virtual void flip() = 0;

	//! Set the byte order (little or big).
	virtual void setByteOrder(System::ByteOrder order) = 0;

	//! Get the byte order.
	virtual System::ByteOrder getByteOrder() const = 0;
};

//---------------------------------------------------------------------------------------
//! @class ArrayNetBuffer
//-------------------------------------------------------------------------------------
//! This buffer wrap a simple byte array. Default byte-order is system native.
//---------------------------------------------------------------------------------------
class O3D_NET_API ArrayNetBuffer : public NetBuffer
{
public:

	ArrayNetBuffer(UInt8* array, UInt32 size);
	ArrayNetBuffer(UInt32 size);
	virtual ~ArrayNetBuffer();

	virtual void writeInt8(Int8 value);
    virtual void writeUInt8(UInt8 value);

	virtual void writeInt16(Int16 value);
    virtual void writeUInt16(UInt16 value);

	virtual void writeInt32(Int32 value);
    virtual void writeUInt32(UInt32 value);

	virtual void writeInt64(Int64 value);
    virtual void writeUInt64(UInt64 value);

    virtual void writeUTF8(const String &string);
    virtual void writeUTF8(const Char* string);
	virtual void writeBool(Bool value);

	virtual void write(const UInt8* buffer, UInt32 size);

    virtual Int8 readInt8();
    virtual UInt8 readUInt8();

    virtual Int16 readInt16();
    virtual UInt16 readUInt16();

    virtual Int32 readInt32();
    virtual UInt32 readUInt32();

    virtual Int64 readInt64();
    virtual UInt64 readUInt64();

    virtual Bool readUTF8(Char* string, Int16 size);
	virtual Bool readUTF8(String& string);
	virtual Bool readBool();

	virtual Bool read(UInt8* buffer, Int16 size);

	virtual Int32 getAvailable() const;
	virtual Int32 getFree() const;

	virtual UInt32 getLimit() const;
	virtual void setLimit(UInt32 limit);

	virtual UInt32 getPosition() const;
	virtual void setPosition(UInt32 position);

	virtual UInt8* getWriteBuffer();
	virtual UInt8* getBuffer();


	virtual void compact();

	virtual void flip();

	//! Set the byte order (little or big).
	virtual void setByteOrder(System::ByteOrder order);

	//! Get the byte order.
	virtual System::ByteOrder getByteOrder() const;

private:

	System::ByteOrder m_byteOrder; //!< buffer byte order for read and write
	Bool m_swap;               //!< true mean swap byte order

	Bool m_wrapped;            //!< Wrap a C array
	UInt32 m_size;             //!< array size
	UInt8* m_array;            //!< An array
	UInt32 m_readPosition;     //!< Current read position
	UInt32 m_writePosition;    //!< Current write position
};

//! @class E_BufferException base class for Buffer Exception
class O3D_NET_API E_BufferException : public E_BaseException
{
	O3D_E_DEF_CLASS(E_BufferException)

	//! Ctor
	E_BufferException(const String& msg) throw() : E_BaseException(msg)
		O3D_E_DEF(E_BufferException,"Buffer Exception")
};

//! @class E_BufferOverflow used for a buffer capacity overflow
class O3D_NET_API E_BufferOverflow : public E_BufferException
{
	O3D_E_DEF_CLASS(E_BufferOverflow)

	//! Ctor
	E_BufferOverflow(const String& msg) throw() : E_BufferException(msg)
		O3D_E_DEF(E_BufferOverflow,"Buffer Overflow Exception")
};

} // namespace net
} // namespace o3d

#endif // _O3D_NETBUFFER_H
