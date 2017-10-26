/**
 * @file http.h
 * @brief A simple HTTP request emitter/receiver
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2006-06-21
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_HTTP_H
#define _O3D_HTTP_H

#include <o3d/core/memorydbg.h>
#include <o3d/core/filemanager.h>
#include <o3d/core/templatearray.h>

#include "client.h"

namespace o3d {
namespace net {

//---------------------------------------------------------------------------------------
//! @class Http
//-------------------------------------------------------------------------------------
//! A simple HTTP request emitter/receiver
//---------------------------------------------------------------------------------------
class O3D_NET_API Http
{
protected:

	UInt32 m_packetsize; //!< max size of readed packet
	ClientTCP m_client;      //!< used client tcp socket

	Bool m_IsResult;     //!< is there a result

	String m_head;           //!< result head part of a get request

	String m_http;           //!< http version used for request
	String m_mime;           //!< accepted type for a request

	String m_url;            //!< server url
	UInt32 m_port;       //!< server port

	Callback* m_pCallback;   //!< used callback on each block loaded (param: loaded block size)
							 //!< work on the content receive only

public:

	//! Info struct used with callback for get file download progression
	struct CoreInfo
	{
		Int32 len;            //!< length of the core data
		Int32 pos;            //!< cur pos on the core data
	};

	//! returned code when make a request
	enum ReturnCode
	{
		//SOCKET_ERROR=-1
		NO_CLIENT = 1,
		SEND_ERROR = 2,
		RECV_ERROR = 3,
		NO_RESULT_PTR = 4,
		OK = 200,
		CREATED = 201,
		ACCEPTED = 202,
		PARTIAL_INFORMATION = 203,
		NO_RESPONSE = 204,
		RESET_CONTENT = 205,
		PARTIAL_CONTENT = 206,
		MOVED = 301,
		FOUND = 302,
		METHOD = 303,
		NOT_MODIFIED = 304,
		BAD_REQUEST = 400,
		UNAUTHORIZED = 401,
		PAYMENT_REQUIRED = 402,
		FORBIDDEN = 403,
		NOT_FOUND = 404,
		INTERNAL_ERROR = 500,
		NOT_IMPLEMENTED = 501,
		BAD_GATEWAY = 502,
		SERVICE_UNAVAILABLE = 503,
		GATEWAY_TIMEOUT = 504
	};

	//! default constructor
	Http(const String& url,UInt32 port = 80) :
		m_packetsize(1024),
		m_IsResult(False),
		m_http("1.0"),
		m_mime("*/*"),
		m_url(url),
		m_port(port),
        m_pCallback(nullptr)
	{
	}

	//! set/get the packet maxsize
	inline void setPacketSize(UInt32 size) { m_packetsize = size; }
	inline UInt32 getPacketSize()const { return m_packetsize; }

	//! set the block receive callback
	void setRecvCallBack(Callback* pCallback) { m_pCallback = pCallback; }

	//
	// Request methods
	//

	//! set/get HTTP version
	inline void setHttpVersion(const String& vers = "1.0") { m_http = vers; }
	inline String getHttpVersion()const { return m_http; }

	//! set/get the Accept mime for the request
	inline void setAcceptMime(const String& mime = "text/html") { m_mime = mime; }
	inline String getAcceptMime()const { return m_mime; }

	//! send a GET value request and copy its answer to result, and return OK if the request have worked
    ReturnCode get(const String& value, ArrayUInt8& result);
    ReturnCode get(const String& value, InStream *&result);

	//! download a file with a GET request
	Http::ReturnCode download(const String& value,const String& filename);

	//! get a result HEAD of a GET or HEAD request in string format
	inline void getHead(String& head) { if (m_IsResult) head = m_head; }

	//! send a HEAD value request and return OK if the request have worked
    ReturnCode head(const String& value, String& head);
};

} // namespace net
} // namespace o3d

#endif // _O3D_HTTP_H
