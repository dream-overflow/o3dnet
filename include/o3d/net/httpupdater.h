/**
 * @file httpupdater.h
 * @brief an http serveur. A dump file on the server side provides the modification to process.
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2006-06-22
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_HTTPUPDATER_H
#define _O3D_HTTPUPDATER_H

#include <o3d/core/memorydbg.h>
#include <o3d/core/base.h>

#include "http.h"
#include <o3d/core/filemanager.h>
#include <o3d/core/runnable.h>
#include <o3d/core/thread.h>
#include <o3d/core/stringlist.h>

namespace o3d {
namespace net {

//---------------------------------------------------------------------------------------
// @class HttpUpdater
//-------------------------------------------------------------------------------------
//! Http update provide a mechanism for update a file hierarchy onto a client from an
//! http serveur. A dump file on the server side provides the modification to process.
//! It process the update using 2 extras threads.
//---------------------------------------------------------------------------------------
class O3D_NET_API HttpUpdater : public Runnable
{
private:

	String m_rootpath;           //!< file destination file tree path
	String m_rootpathBis;        //!< file destination file tree path for the second thread

	String m_server_url;         //!< update serveur url
	String m_base_uri;           //!< base uri for find files to download and dump file

    InStream *m_filelist;       //!< the filelist virtual file

	Thread* m_thread;            //!< the update thread (make check MD5 and dir management)
	Thread* m_downloadThread;    //!< the file download thread

	UInt32 m_totalfile;           //!< nbr of file to update

	PFastMutex<String> m_currentfile;         //!< current updating filename
	PFastMutex<String> m_currentop;           //!< current update operation

	PFastMutex<UInt32>  m_currentfilepercent;  //!< current file percent of update
	PFastMutex<UInt32>  m_totalpercent;        //!< total percent of update
	PFastMutex<UInt32>  m_nbrfile;             //!< nbr file to update

	CallbackMethod<HttpUpdater>* m_callback;       //!< callback used by O3DHttp::Get

	PFastMutex<Bool> m_isupdate;  //!< is the update is in progress
	PFastMutex<T_StringList> m_filetodown; //!< the file list of file to download

	Semaphore m_Semaphore;        //!< synchronization update semaphore

public:

	//! construcor
	HttpUpdater() :
        m_thread(nullptr),
        m_downloadThread(nullptr),
		m_totalfile(0),
        m_callback(nullptr),
		m_Semaphore(0,1)
	{}

	//! destructor
	virtual ~HttpUpdater();

	//! download file listing
	Bool initAndDownloadListing(const String& server,const String& baseuri = "/");

	//! make complete update
	void makeUpdate(const String &rootpath);

	//! quit connection and delete the thread object
	void quit();

	//! wait semaphore signal for synchronize with the main thread
	void synchronize();

	//! get the nbr of file whose stays to update
	UInt32 getNumFileToUpdate();

	//! is updating
	Bool isUpdating();

	//! get the current file percent (useful for current file progress bar)
	UInt32 getCurrentFilePercent();

	//! get the current operation (useful for draw info)
	String getCurentOperation();

	//! get the current downloading file name (useful for draw info)
	String getCurentFileName();

	//! get global update percent (useful for global progress bar)
	UInt32 getGlobalProgress();

	//! called by the O3DHttp::Get callback
	Int32 resfresh(void*);

	//! called by the O3DHttpUpdater::m_thread
	Int32 run(void*);

	//! called by the O3DHttpUpdate::m_downloadThread
	Int32 downThreadFunc(void*);
};

} // namespace net
} // namespace o3d

#endif // _O3D_HTTPUPDATER_H
