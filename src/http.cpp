/**
 * @file http.cpp
 * @brief Implementation of Http.h
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2006-06-21
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/precompiled.h"

#include "o3d/core/architecture.h"
#include "o3d/net/http.h"

#include <o3d/core/datainstream.h>
#include <o3d/core/fileoutstream.h>

using namespace o3d;
using namespace o3d::net;

/*---------------------------------------------------------------------------------------
  send a request and receive its answer for GET or HEAD, and return OK if the request
  have worked
---------------------------------------------------------------------------------------*/
Http::ReturnCode Http::get(const String& value, ArrayUInt8& result)
{
	m_IsResult = False;
	m_head.destroy();

	if (!m_client.create(m_url,80))
		return NO_CLIENT;

	if (!m_client.connect())
	{
		m_client.destroy();
		return NO_CLIENT;
	}

	String request = String("GET ") + value + " HTTP/" + m_http + "\nAccept: " + m_mime + "\n\n";

	if ((UInt32)m_client.send(
				(UInt8*)request.toUtf8().getData(),
				request.length()) != request.length())
	{
		m_client.shutdown(SD_SEND);
		m_client.destroy();
		return SEND_ERROR;
	}

	UInt8 *buffer = new UInt8[m_packetsize+1000];
	Int32 len;
	Int32 pos;
	Bool in_head = True;

	CoreInfo *coreinfo = new CoreInfo;
	coreinfo->pos = 0;
	coreinfo->len = -1;

	while ((len = m_client.receive(buffer,m_packetsize)) > 0)
	{
		buffer[len] = 0;

		if (in_head)
		{
			m_head += (Char*)buffer;

			// get the core length
            if ((pos = m_head.sub("Content-Length",0)) != -1)
				coreinfo->len = m_head.toUInt32(pos+15);

            if ((pos = m_head.sub("\r\n\r\n",0)) != -1)
			{
                result.pushArray((UInt8*)&buffer[pos+4],len-pos-4);

				coreinfo->pos += len-pos-4;
				if (m_pCallback) m_pCallback->call(coreinfo);

				m_head.truncate(pos+4);
				in_head = False;
			}
		}
		else
		{
            result.pushArray((UInt8*)buffer,len);

			coreinfo->pos += len;
			if (m_pCallback) m_pCallback->call(coreinfo);
		}
	}

	m_client.shutdown(SD_SEND);
	m_client.destroy();

	if (len == SOCKET_ERROR)
	{
		deletePtr(coreinfo);
		deleteArray(buffer);

		return (ReturnCode)SOCKET_ERROR;
	}

	deletePtr(coreinfo);
	deleteArray(buffer);

	ReturnCode code = RECV_ERROR;

    pos = m_head.sub("HTTP/",0);
	if (pos != -1)
	{
		pos += 5 + m_http.length() + 1;

		if (pos < (Int32)m_head.length())
			code = (ReturnCode)m_head.toUInt32(pos);

		if (code == OK)
			m_IsResult = True;
	}

	return code;
}

/*---------------------------------------------------------------------------------------
  send a HEAD value request and return OK if the request have worked
---------------------------------------------------------------------------------------*/
Http::ReturnCode Http::head(const String& value,String& head)
{
	m_IsResult = False;
	m_head.destroy();

	if (!m_client.create(m_url,80))
		return NO_CLIENT;

	if (!m_client.connect())
	{
		m_client.destroy();
		return NO_CLIENT;
	}

	String request = String("HEAD ") + value + "HTTP/" + m_http + "\nAccept: " + m_mime + "\n\n";

	if ((UInt32)m_client.send(
						(UInt8*)request.toUtf8().getData(),
						request.length()) != request.length())
	{
		m_client.shutdown(SD_SEND);
		m_client.destroy();
		return SEND_ERROR;
	}

	UInt8 *buffer = new UInt8[m_packetsize];
	Int32 len;
	Int32 pos;

	while ((len = m_client.receive(buffer,m_packetsize)) > 0)
	{
		buffer[len] = 0;
		m_head += (Char*)buffer;
	}

	if (len == SOCKET_ERROR)
	{
		m_client.shutdown(SD_SEND);
		m_client.destroy();
		return (ReturnCode)SOCKET_ERROR;
	}

	ReturnCode code = RECV_ERROR;

    pos = m_head.sub("HTTP/",0);
	if (pos != -1)
	{
		pos += 5 + m_http.length() + 1;

		if (pos < (Int32)m_head.length())
			code = (ReturnCode)m_head.toUInt32(pos);

		if (code == OK)
			m_IsResult = True;
	}

	m_client.shutdown(SD_SEND);
	m_client.destroy();

	return code;
}

Http::ReturnCode Http::get(const String& value, InStream *&result)
{
	ArrayUInt8 res;
	ReturnCode retcode;

	if ((retcode = get(value,res)) == OK)
        result = new DataInStream(res);

	return retcode;
}

/*---------------------------------------------------------------------------------------
  download a file with a GET request
---------------------------------------------------------------------------------------*/
Http::ReturnCode Http::download(const String& value,const String& filename)
{
	m_IsResult = False;
	m_head.destroy();

	if (!m_client.create(m_url,80))
		return NO_CLIENT;

	if (!m_client.connect())
	{
		m_client.destroy();
		return NO_CLIENT;
	}

	String request = String("GET ") + value + " HTTP/" + m_http + "\nAccept: " + m_mime + "\n\n";

	if ((UInt32)m_client.send(
						(UInt8*)request.toUtf8().getData(),
						request.length()) != request.length())
	{
		m_client.shutdown(SD_SEND);
		m_client.destroy();
		return SEND_ERROR;
	}

	UInt8 *buffer = new UInt8[m_packetsize+1];
	Int32 len;
	Int32 pos;
	Bool in_head = True;

	CoreInfo *coreinfo = new CoreInfo;
	coreinfo->pos = 0;
	coreinfo->len = -1;

    FileOutStream *os = FileManager::instance()->openOutStream(filename, FileOutStream::CREATE);

	while ((len = m_client.receive(buffer,m_packetsize)) > 0)
	{
		buffer[len] = 0;

		if (in_head)
		{
			m_head += (Char*)buffer;

			// get the core length
            if ((pos = m_head.sub("Content-Length",0)) != -1)
				coreinfo->len = m_head.toUInt32(pos+15);

			// find the start of the core
            if ((pos = m_head.sub("\r\n\r\n",0)) != -1)
			{
                os->write(&buffer[pos+4], len-pos-4);

				coreinfo->pos += len-pos-4;
				if (m_pCallback) m_pCallback->call(coreinfo);

				m_head.truncate(pos+4);
				in_head = False;
			}
		}
		else
		{
            os->write(buffer, len);

			coreinfo->pos += len;
			if (m_pCallback) m_pCallback->call(coreinfo);
		}
	}

    deletePtr(os);

	if (len == SOCKET_ERROR)
	{
		deletePtr(coreinfo);
		deleteArray(buffer);

		m_client.shutdown(SD_SEND);
		m_client.destroy();

		return (ReturnCode)SOCKET_ERROR;
	}

	deletePtr(coreinfo);
	deleteArray(buffer);

	ReturnCode code = RECV_ERROR;

    pos = m_head.sub("HTTP/",0);
	if (pos != -1)
	{
		pos += 5 + m_http.length() + 1;

		if (pos < (Int32)m_head.length())
			code = (ReturnCode)m_head.toUInt32(pos);

		if (code == OK)
			m_IsResult = True;
	}

	m_client.shutdown(SD_SEND);
	m_client.destroy();

	return code;
}

