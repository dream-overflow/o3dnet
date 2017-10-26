/**
 * @file netbuffer.cpp
 * @brief Buffer implementation
 * @author Patrice GILBERT (patrice.gilbert@revolutioning.com)
 * @date 2009-10-27
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/precompiled.h"

#include "o3d/net/netbuffer.h"
#include <o3d/core/debug.h>

using namespace o3d;
using namespace o3d::net;

ArrayNetBuffer::ArrayNetBuffer(UInt8* array, UInt32 size)
{
	O3D_CHECKPTR(array);
	m_wrapped = True;
	m_array = array;
	m_size = size;
	m_readPosition = 0;
	m_writePosition = 0;

	setByteOrder(System::getNativeByteOrder());
}

ArrayNetBuffer::ArrayNetBuffer(UInt32 size)
{
	m_wrapped = False;
	m_array = new UInt8[size];
	m_size = size;
	m_readPosition = 0;
	m_writePosition = 0;

	setByteOrder(System::getNativeByteOrder());
}

ArrayNetBuffer::~ArrayNetBuffer()
{
	if (!m_wrapped)
	{
		deleteArray(m_array);
	}
}

void ArrayNetBuffer::writeInt8(Int8 value)
{
	if (m_writePosition+1 > m_size)
	{
		O3D_ERROR(E_BufferOverflow("Write overflow"));
	}
	else
	{
		m_array[m_writePosition] = value;
		m_writePosition++;
	}
}

void ArrayNetBuffer::writeUInt8(UInt8 value)
{
    if (m_writePosition+1 > m_size)
    {
        O3D_ERROR(E_BufferOverflow("Write overflow"));
    }
    else
    {
        m_array[m_writePosition] = value;
        m_writePosition++;
    }
}

void ArrayNetBuffer::writeInt16(Int16 value)
{
	if (m_writePosition+2 > m_size)
	{
		O3D_ERROR(E_BufferOverflow("Write overflow"));
	}
	else
	{
		if (m_swap)
		{
			m_array[m_writePosition+1] = static_cast<UInt8>(*reinterpret_cast<UInt16*>(&value));
            m_array[m_writePosition]   = static_cast<UInt8>(*reinterpret_cast<UInt16*>(&value) >> 8);
		}
		else
		{
            m_array[m_writePosition]   = static_cast<UInt8>(*reinterpret_cast<UInt16*>(&value));
			m_array[m_writePosition+1] = static_cast<UInt8>(*reinterpret_cast<UInt16*>(&value) >> 8);
		}

		m_writePosition += 2;
	}
}


void ArrayNetBuffer::writeUInt16(UInt16 value)
{
    if (m_writePosition+2 > m_size)
    {
        O3D_ERROR(E_BufferOverflow("Write overflow"));
    }
    else
    {
        if (m_swap)
        {
            m_array[m_writePosition+1] = static_cast<UInt8>(value);
            m_array[m_writePosition]   = static_cast<UInt8>(value >> 8);
        }
        else
        {
            m_array[m_writePosition]   = static_cast<UInt8>(value);
            m_array[m_writePosition+1] = static_cast<UInt8>(value >> 8);
        }

        m_writePosition += 2;
    }
}

void ArrayNetBuffer::writeInt32(Int32 value)
{
	if (m_writePosition+4 > m_size)
	{
		O3D_ERROR(E_BufferOverflow("Write overflow"));
	}
	else
	{
		if (m_swap)
		{
			m_array[m_writePosition+3] = static_cast<UInt8>(*reinterpret_cast<UInt32*>(&value));
			m_array[m_writePosition+2] = static_cast<UInt8>(*reinterpret_cast<UInt32*>(&value) >> 8);
			m_array[m_writePosition+1] = static_cast<UInt8>(*reinterpret_cast<UInt32*>(&value) >> 16);
			m_array[m_writePosition]   = static_cast<UInt8>(*reinterpret_cast<UInt32*>(&value) >> 24);
		}
		else
		{
            m_array[m_writePosition]   = static_cast<UInt8>(*reinterpret_cast<UInt32*>(&value));
			m_array[m_writePosition+1] = static_cast<UInt8>(*reinterpret_cast<UInt32*>(&value) >> 8);
			m_array[m_writePosition+2] = static_cast<UInt8>(*reinterpret_cast<UInt32*>(&value) >> 16);
			m_array[m_writePosition+3] = static_cast<UInt8>(*reinterpret_cast<UInt32*>(&value) >> 24);
		}

		m_writePosition += 4;
	}
}

void ArrayNetBuffer::writeUInt32(UInt32 value)
{
    if (m_writePosition+4 > m_size)
    {
        O3D_ERROR(E_BufferOverflow("Write overflow"));
    }
    else
    {
        if (m_swap)
        {
            m_array[m_writePosition+3] = static_cast<UInt8>(value);
            m_array[m_writePosition+2] = static_cast<UInt8>(value >> 8);
            m_array[m_writePosition+1] = static_cast<UInt8>(value >> 16);
            m_array[m_writePosition]   = static_cast<UInt8>(value >> 24);
        }
        else
        {
            m_array[m_writePosition]   = static_cast<UInt8>(value);
            m_array[m_writePosition+1] = static_cast<UInt8>(value >> 8);
            m_array[m_writePosition+2] = static_cast<UInt8>(value >> 16);
            m_array[m_writePosition+3] = static_cast<UInt8>(value >> 24);
        }

        m_writePosition += 4;
    }
}

void ArrayNetBuffer::writeInt64(Int64 value)
{
	if (m_writePosition+8 > m_size)
	{
		O3D_ERROR(E_BufferOverflow("Write overflow"));
	}
	else
	{
		if (m_swap)
		{
			m_array[m_writePosition+7] = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value));
			m_array[m_writePosition+6] = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value) >> 8);
			m_array[m_writePosition+5] = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value) >> 16);
			m_array[m_writePosition+4] = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value) >> 24);
			m_array[m_writePosition+3] = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value) >> 32);
			m_array[m_writePosition+2] = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value) >> 40);
			m_array[m_writePosition+1] = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value) >> 48);
			m_array[m_writePosition]   = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value) >> 56);
		}
		else
		{
            m_array[m_writePosition]   = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value));
			m_array[m_writePosition+1] = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value) >> 8);
			m_array[m_writePosition+2] = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value) >> 16);
			m_array[m_writePosition+3] = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value) >> 24);
			m_array[m_writePosition+4] = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value) >> 32);
			m_array[m_writePosition+5] = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value) >> 40);
			m_array[m_writePosition+6] = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value) >> 48);
			m_array[m_writePosition+7] = static_cast<UInt8>(*reinterpret_cast<UInt64*>(&value) >> 56);
		}

		m_writePosition += 8;
	}
}

void ArrayNetBuffer::writeUInt64(UInt64 value)
{
    if (m_writePosition+8 > m_size)
    {
        O3D_ERROR(E_BufferOverflow("Write overflow"));
    }
    else
    {
        if (m_swap)
        {
            m_array[m_writePosition+7] = static_cast<UInt8>(value);
            m_array[m_writePosition+6] = static_cast<UInt8>(value >> 8);
            m_array[m_writePosition+5] = static_cast<UInt8>(value >> 16);
            m_array[m_writePosition+4] = static_cast<UInt8>(value >> 24);
            m_array[m_writePosition+3] = static_cast<UInt8>(value >> 32);
            m_array[m_writePosition+2] = static_cast<UInt8>(value >> 40);
            m_array[m_writePosition+1] = static_cast<UInt8>(value >> 48);
            m_array[m_writePosition]   = static_cast<UInt8>(value >> 56);
        }
        else
        {
            m_array[m_writePosition] = static_cast<UInt8>(value);
            m_array[m_writePosition+1] = static_cast<UInt8>(value >> 8);
            m_array[m_writePosition+2] = static_cast<UInt8>(value >> 16);
            m_array[m_writePosition+3] = static_cast<UInt8>(value >> 24);
            m_array[m_writePosition+4] = static_cast<UInt8>(value >> 32);
            m_array[m_writePosition+5] = static_cast<UInt8>(value >> 40);
            m_array[m_writePosition+6] = static_cast<UInt8>(value >> 48);
            m_array[m_writePosition+7] = static_cast<UInt8>(value >> 56);
        }

        m_writePosition += 8;
    }
}

void ArrayNetBuffer::writeUTF8(const String &string)
{
    CString utf8 = string.toUtf8();

    Int32 length = utf8.length();
    writeInt16((Int16)length);
    for (int i = 0; i < length; i++)
    {
        writeInt8(utf8[i]);
    }
}

void ArrayNetBuffer::writeUTF8(const Char* string)
{
	Int32 length = strlen(string);
	writeInt16((Int16)length);
	for (int i = 0; i < length; i++)
	{
		writeInt8(string[i]);
	}
}
void ArrayNetBuffer::writeBool(Bool value)
{
	if (value)
		writeInt8(1);
	else
		writeInt8(0);
}

void ArrayNetBuffer::write(const UInt8* buffer, UInt32 size)
{
	if (size > m_size - m_writePosition)
	{
		O3D_ERROR(E_BufferOverflow("Write overflow"));
	}
	else
	{
		memcpy(m_array + m_writePosition, buffer, size);
		m_writePosition += size;
	}
}

Int8 ArrayNetBuffer::readInt8()
{
	if (m_readPosition+1 > m_writePosition)
	{
		O3D_ERROR(E_BufferOverflow("Read overflow"));
		return 0; // Never reach
	}
	else
	{
		Int8 value = m_array[m_readPosition];
		m_readPosition++;
		return value;
	}
}

UInt8 ArrayNetBuffer::readUInt8()
{
    if (m_readPosition+1 > m_writePosition)
    {
        O3D_ERROR(E_BufferOverflow("Read overflow"));
        return 0; // Never reach
    }
    else
    {
        Int8 value = m_array[m_readPosition];
        m_readPosition++;
        return value;
    }
}

Int16 ArrayNetBuffer::readInt16()
{
	if (m_readPosition+2 > m_writePosition)
	{
		O3D_ERROR(E_BufferOverflow("Read overflow"));
		return 0; // Never reach
	}
	else
	{
		Int16 value;
		if (m_swap)
		{
			value = *reinterpret_cast<UInt16*>(&m_array[m_readPosition+1]) |
					*reinterpret_cast<UInt16*>(&m_array[m_readPosition]) << 8;
		}
		else
		{
			value = *reinterpret_cast<UInt16*>(&m_array[m_readPosition]) |
					*reinterpret_cast<UInt16*>(&m_array[m_readPosition+1]) << 8;
		}

		m_readPosition += 2;
		return value;
	}
}

UInt16 ArrayNetBuffer::readUInt16()
{
    if (m_readPosition+2 > m_writePosition)
    {
        O3D_ERROR(E_BufferOverflow("Read overflow"));
        return 0; // Never reach
    }
    else
    {
        UInt16 value;
        if (m_swap)
        {
            value = *reinterpret_cast<UInt16*>(&m_array[m_readPosition+1]) |
                    *reinterpret_cast<UInt16*>(&m_array[m_readPosition]) << 8;
        }
        else
        {
            value = *reinterpret_cast<UInt16*>(&m_array[m_readPosition]) |
                    *reinterpret_cast<UInt16*>(&m_array[m_readPosition+1]) << 8;
        }

        m_readPosition += 2;
        return value;
    }
}

Int32 ArrayNetBuffer::readInt32()
{
	if (m_readPosition+4 > m_writePosition)
	{
		O3D_ERROR(E_BufferOverflow("Read overflow"));
		return 0; // Never reach
	}
	else
	{
		Int32 value;
		if (m_swap)
		{
			value = *reinterpret_cast<UInt32*>(&m_array[m_readPosition+3]) |
					*reinterpret_cast<UInt32*>(&m_array[m_readPosition+2]) << 8 |
					*reinterpret_cast<UInt32*>(&m_array[m_readPosition+1]) << 16 |
					*reinterpret_cast<UInt32*>(&m_array[m_readPosition])   << 24;
		}
		else
		{
			value = *reinterpret_cast<UInt32*>(&m_array[m_readPosition]) |
					*reinterpret_cast<UInt32*>(&m_array[m_readPosition+1]) << 8 |
					*reinterpret_cast<UInt32*>(&m_array[m_readPosition+2]) << 16 |
					*reinterpret_cast<UInt32*>(&m_array[m_readPosition+3]) << 24;
		}

		m_readPosition += 4;
		return value;
	}
}

UInt32 ArrayNetBuffer::readUInt32()
{
    if (m_readPosition+4 > m_writePosition)
    {
        O3D_ERROR(E_BufferOverflow("Read overflow"));
        return 0; // Never reach
    }
    else
    {
        UInt32 value;
        if (m_swap)
        {
            value = *reinterpret_cast<UInt32*>(&m_array[m_readPosition+3]) |
                    *reinterpret_cast<UInt32*>(&m_array[m_readPosition+2]) << 8 |
                    *reinterpret_cast<UInt32*>(&m_array[m_readPosition+1]) << 16 |
                    *reinterpret_cast<UInt32*>(&m_array[m_readPosition])   << 24;
        }
        else
        {
            value = *reinterpret_cast<UInt32*>(&m_array[m_readPosition]) |
                    *reinterpret_cast<UInt32*>(&m_array[m_readPosition+1]) << 8 |
                    *reinterpret_cast<UInt32*>(&m_array[m_readPosition+2]) << 16 |
                    *reinterpret_cast<UInt32*>(&m_array[m_readPosition+3]) << 24;
        }

        m_readPosition += 4;
        return value;
    }
}

Int64 ArrayNetBuffer::readInt64()
{
	if (m_readPosition+8 > m_writePosition)
	{
		O3D_ERROR(E_BufferOverflow("Read overflow"));
		return 0; // Never reach
	}
	else
	{
		Int64 value;
		if (m_swap)
		{
			value = *reinterpret_cast<UInt64*>(&m_array[m_readPosition+7]) |
					*reinterpret_cast<UInt64*>(&m_array[m_readPosition+6]) << 8 |
					*reinterpret_cast<UInt64*>(&m_array[m_readPosition+5]) << 16 |
					*reinterpret_cast<UInt64*>(&m_array[m_readPosition+4]) << 24 |
					*reinterpret_cast<UInt64*>(&m_array[m_readPosition+3]) << 32 |
					*reinterpret_cast<UInt64*>(&m_array[m_readPosition+2]) << 40 |
					*reinterpret_cast<UInt64*>(&m_array[m_readPosition+1]) << 48 |
					*reinterpret_cast<UInt64*>(&m_array[m_readPosition])   << 56;
		}
		else
		{
			value = *reinterpret_cast<UInt64*>(&m_array[m_readPosition]) |
					*reinterpret_cast<UInt64*>(&m_array[m_readPosition+1]) << 8 |
					*reinterpret_cast<UInt64*>(&m_array[m_readPosition+2]) << 16 |
					*reinterpret_cast<UInt64*>(&m_array[m_readPosition+3]) << 24 |
					*reinterpret_cast<UInt64*>(&m_array[m_readPosition+4]) << 32 |
					*reinterpret_cast<UInt64*>(&m_array[m_readPosition+5]) << 40 |
					*reinterpret_cast<UInt64*>(&m_array[m_readPosition+6]) << 48 |
					*reinterpret_cast<UInt64*>(&m_array[m_readPosition+7]) << 56;
		}

		m_readPosition += 8;
		return value;
	}
}

UInt64 ArrayNetBuffer::readUInt64()
{
    if (m_readPosition+8 > m_writePosition)
    {
        O3D_ERROR(E_BufferOverflow("Read overflow"));
        return 0; // Never reach
    }
    else
    {
        UInt64 value;
        if (m_swap)
        {
            value = *reinterpret_cast<UInt64*>(&m_array[m_readPosition+7]) |
                    *reinterpret_cast<UInt64*>(&m_array[m_readPosition+6]) << 8 |
                    *reinterpret_cast<UInt64*>(&m_array[m_readPosition+5]) << 16 |
                    *reinterpret_cast<UInt64*>(&m_array[m_readPosition+4]) << 24 |
                    *reinterpret_cast<UInt64*>(&m_array[m_readPosition+3]) << 32 |
                    *reinterpret_cast<UInt64*>(&m_array[m_readPosition+2]) << 40 |
                    *reinterpret_cast<UInt64*>(&m_array[m_readPosition+1]) << 48 |
                    *reinterpret_cast<UInt64*>(&m_array[m_readPosition])   << 56;
        }
        else
        {
            value = *reinterpret_cast<UInt64*>(&m_array[m_readPosition]) |
                    *reinterpret_cast<UInt64*>(&m_array[m_readPosition+1]) << 8 |
                    *reinterpret_cast<UInt64*>(&m_array[m_readPosition+2]) << 16 |
                    *reinterpret_cast<UInt64*>(&m_array[m_readPosition+3]) << 24 |
                    *reinterpret_cast<UInt64*>(&m_array[m_readPosition+4]) << 32 |
                    *reinterpret_cast<UInt64*>(&m_array[m_readPosition+5]) << 40 |
                    *reinterpret_cast<UInt64*>(&m_array[m_readPosition+6]) << 48 |
                    *reinterpret_cast<UInt64*>(&m_array[m_readPosition+7]) << 56;
        }

        m_readPosition += 8;
        return value;
    }
}

Bool ArrayNetBuffer::read(UInt8* buffer, Int16 size)
{
	if (getAvailable() >= size)
	{
		memcpy(buffer, &(m_array[getPosition()]), size);
		setPosition(getPosition() + size);

		return True;
	}
	return False;
}

Bool ArrayNetBuffer::readUTF8(Char* string, Int16 size)
{
	if (getAvailable() >= 2)
	{
		if (size == 0)
		{
			string[0] = 0x00;
			return True;
		}
		if (getAvailable() >= size)
		{
			// TODO check why size + 1 ?
			memcpy(string, &(m_array[getPosition()]), size/* + 1*/);
			string[size] = 0x00;
			setPosition(getPosition() + size);
		}
	}
	return False;
}

Bool ArrayNetBuffer::readUTF8(String &string)
{
	if (getAvailable() >= 2)
	{
		Int16 size = readInt16();
		if ((getAvailable() >= size) && (size > 0))
		{
			string.fromUtf8(reinterpret_cast<Char*>(&(m_array[getPosition()])), size);
			setPosition(getPosition() + size);
		}
		else
		{
			string << "";
		}
		return True;
	}
	return False;
}

Bool ArrayNetBuffer::readBool()
{
	return readInt8() != 0 ? True : False;
}

Int32 ArrayNetBuffer::getAvailable() const
{
	return m_writePosition - m_readPosition;
}

Int32 ArrayNetBuffer::getFree() const
{
	return m_size - m_writePosition + m_readPosition;
}

UInt8* ArrayNetBuffer::getWriteBuffer()
{
	return m_array + m_writePosition;
}

UInt8* ArrayNetBuffer::getBuffer()
{
	return m_array;
}

UInt32 ArrayNetBuffer::getLimit() const
{
	return m_writePosition;
}

void ArrayNetBuffer::setLimit(UInt32 limit)
{
	if (limit < m_size)
	{
		m_writePosition = limit;
	}
	else
	{
		O3D_ERROR(E_BufferOverflow("Limit overflow"));
	}
}

UInt32 ArrayNetBuffer::getPosition() const
{
	return m_readPosition;
}

void ArrayNetBuffer::setPosition(UInt32 position)
{
	if (position >= m_size)
	{
		O3D_ERROR(E_BufferOverflow("Position overflow"));
	}
	else if (position <= m_writePosition)
	{
		m_readPosition = position;
	}
	else
	{
		O3D_ERROR(E_BufferOverflow("Position overflow"));
	}
}

void ArrayNetBuffer::compact()
{
	if ((m_writePosition > m_readPosition) && (m_readPosition > 0))
	{
		// move data unread to the beginning of the array
		memcpy(m_array, m_array + m_readPosition, m_writePosition - m_readPosition);
		m_writePosition -= m_readPosition;
		m_readPosition = 0;
	}
	else
	{
		m_writePosition = 0;
		m_readPosition = 0;
	}
}

void ArrayNetBuffer::flip()
{
	m_readPosition = 0;
}

void ArrayNetBuffer::setByteOrder(System::ByteOrder order)
{
	m_byteOrder = order;
	m_swap = m_byteOrder != System::getNativeByteOrder();
}

System::ByteOrder ArrayNetBuffer::getByteOrder() const
{
	return m_byteOrder;
}

