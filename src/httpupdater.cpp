/**
 * @file httpupdater.cpp
 * @brief Implementation of HttpUpdater.h
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2006-06-22
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/net/precompiled.h"

#include "o3d/core/architecture.h"
#include "o3d/core/md5.h"
#include "o3d/net/httpupdater.h"

#ifdef WIN32
	#include <direct.h>
	#include <shellapi.h>
#endif

using namespace o3d;
using namespace o3d::net;

/*---------------------------------------------------------------------------------------
  destructor
---------------------------------------------------------------------------------------*/
HttpUpdater::~HttpUpdater()
{
	quit();
}

/*---------------------------------------------------------------------------------------
  download file listing
---------------------------------------------------------------------------------------*/
Bool HttpUpdater::initAndDownloadListing(
		const String &server,
		const String &baseuri)
{
	if (m_thread) // already running
		return False;

	m_server_url = server;
	m_base_uri = baseuri;

	Http request(m_server_url,80);

    if (request.get(baseuri + "/dump", m_filelist) == Http::OK)
	{
		// count the num of file to update
		String line;
		UInt32 nbrfile = 0;

		// read each line of the file listing
        while (m_filelist->readLine(line) != -1)
		{
			// a valid line (not a comm)
            if (line.sub("file=",0) == 0)
				nbrfile++;
		}

        m_filelist->reset(0);
		*m_nbrfile = nbrfile; !m_nbrfile;
		m_totalfile = nbrfile;

		return True;
	}

	*m_nbrfile = 0; !m_nbrfile;
	m_totalfile = 0;
	return False;
}

/*---------------------------------------------------------------------------------------
  make complete update
---------------------------------------------------------------------------------------*/
void HttpUpdater::makeUpdate(const String& rootpath)
{
	if (m_thread) // already running
		return;

	m_callback = new CallbackMethod<HttpUpdater>(this,&HttpUpdater::resfresh);
	m_thread = new Thread(this);

	m_rootpath = rootpath;
	m_rootpath.replace('\\','/');
	if (m_rootpath[m_rootpath.length()-1] != '/')
		m_rootpath += '/';

	m_rootpathBis = m_rootpath;

	*m_isupdate = True; !m_isupdate;

	m_thread->start();
}

/*---------------------------------------------------------------------------------------
  quit connection and delete the thread object
---------------------------------------------------------------------------------------*/
void HttpUpdater::quit()
{
	if (m_thread)
	{
		m_thread->waitFinish();
		deletePtr(m_thread);
	}

	deletePtr(m_callback);
    deletePtr(m_filelist);

	m_totalfile = 0;
}

/*---------------------------------------------------------------------------------------
  is updating
---------------------------------------------------------------------------------------*/
Bool HttpUpdater::isUpdating()
{
	Bool bOOL = *m_isupdate; !m_isupdate;
	return bOOL;
}

/*---------------------------------------------------------------------------------------
  get the nbr of file whose stay to update
---------------------------------------------------------------------------------------*/
UInt32 HttpUpdater::getNumFileToUpdate()
{
	UInt32 nbrfile = *m_nbrfile; !m_nbrfile;
	return nbrfile;
}

/*---------------------------------------------------------------------------------------
  get the current file percent (useful for current file progress bar)
---------------------------------------------------------------------------------------*/
UInt32 HttpUpdater::getCurrentFilePercent()
{
	UInt32 nbrfile = *m_nbrfile; !m_nbrfile;

	if (nbrfile == 0)
		return 100;

	UInt32 percent = *m_currentfilepercent; !m_currentfilepercent;
	return percent;
}

/*---------------------------------------------------------------------------------------
  get the current file name (useful for draw info)
---------------------------------------------------------------------------------------*/
String HttpUpdater::getCurentFileName()
{
	String filename = *m_currentfile; !m_currentfile;
	return filename;
}

/*---------------------------------------------------------------------------------------
  get the current operation (useful for draw info)
---------------------------------------------------------------------------------------*/
String HttpUpdater::getCurentOperation()
{
	String op = *m_currentop; !m_currentop;
	return op;
}

/*---------------------------------------------------------------------------------------
  get global update percent (useful for global progress bar)
---------------------------------------------------------------------------------------*/
UInt32 HttpUpdater::getGlobalProgress()
{
	UInt32 nbrfile = *m_nbrfile; !m_nbrfile;

	if (nbrfile == 0)
		return 100;

	UInt32 totalpercent = *m_totalpercent; !m_totalpercent;
	return totalpercent;
}

/*---------------------------------------------------------------------------------------
  called by the Http::Get callback
---------------------------------------------------------------------------------------*/
Int32 HttpUpdater::resfresh(void* info)
{
	Http::CoreInfo *coreinfo = (Http::CoreInfo*)info;

	// current file percent
	if (coreinfo->len > 0)
		*m_currentfilepercent = (UInt32)(100.f * ((Float)coreinfo->pos / (Float)coreinfo->len));
	else
		*m_currentfilepercent = 0;

	!m_currentfilepercent;

	m_Semaphore.postSignal();
	return 0;
}

/*---------------------------------------------------------------------------------------
  wait semaphore signal
---------------------------------------------------------------------------------------*/
void HttpUpdater::synchronize()
{
	m_Semaphore.waitSignal();
}

/*---------------------------------------------------------------------------------------
  called by the Http::m_thread
---------------------------------------------------------------------------------------*/
Int32 HttpUpdater::run(void*)
{
	// call the download thread
	CallbackMethod<HttpUpdater> *DownCallback = new CallbackMethod<HttpUpdater>
		(this,&HttpUpdater::downThreadFunc);

	m_downloadThread = new Thread(NULL);
	m_downloadThread->create(DownCallback);

	String line;
	String text;

	// read each line of the file listing
    while (m_filelist->readLine(line) != -1)
	{
		// a valid line (not a comm)
		if (line[0] != '#')
		{
			// create or check a directory
            if (line.sub("mkdir=",0) == 0)
			{
				String command("mkdir ");
                command += m_rootpath + line.sub(6);
//				_mkdir(command.toUtf8().getData());
#ifdef WIN32
				command.replace('/','\\');
				String com("/C ");
				com += command;
				ShellExecuteW(NULL,L"open",L"cmd.exe",com.getData(),NULL,SW_HIDE);
#else
				system(command.toUtf8().getData());
#endif
			}
			// remove a directory
            else if (line.sub("rmdir=",0) == 0)
			{
				String command("rmdir ");
                command += m_rootpath + line.sub(6);
//				_rmdir(command.toUtf8().getData());
#ifdef WIN32
				command.replace('/','\\');
				String com("/C ");
				com += command;
				ShellExecuteW(NULL,L"open",L"cmd.exe",com.getData(),NULL,SW_HIDE);
#else
				system(command.toUtf8().getData());
#endif
			}
			// create or update a file
            else if (line.sub("file=",0) == 0)
			{
                String filename = line.sub(5);
				String MD5;
				Int32 pos = filename.find('|');
				if (pos != -1)
				{
                    MD5 = filename.sub(pos+1);
					filename.truncate(pos);
				}

				text = String("Update: ") + filename;
				*m_currentop = text; !m_currentop;

				String fullpath = m_rootpath + filename;

				File::adaptPath(fullpath);

                InStream* pis = nullptr;

                try {
                    pis = FileManager::instance()->openInStream(fullpath);
                }
                catch (E_BaseException &)
                {
				}

                // download the file
                if (pis == nullptr)
                {
                    m_filetodown->push_back(filename); !m_filetodown;
                }
				// checksum MD5
				else
				{
					text = String("Checksum: ") + filename;
					*m_currentop = text; !m_currentop;

                    MD5Hash sum(*pis);
                    String client_md5 = sum.getHex();

                    deletePtr(pis);

					// invalid client file, download it
					if (client_md5 != MD5)
					{
						m_filetodown->push_back(filename); !m_filetodown;
					}
					// file is ok
					else
					{
						*m_currentfilepercent = 100; !m_currentfilepercent;
						text = filename + ": Ok";
						*m_currentfile = text; !m_currentfile;

						(*m_nbrfile)--;
						!m_nbrfile;
					}
				}
			}
			// remove a file
            else if (line.sub("rm=",0) == 0)
			{
				String command("del ");
                command += line.sub(3);
#ifdef WIN32
//				remove(command.toUtf8().getData());
				command.replace('/','\\');
				String com("/C ");
				com += command;
				ShellExecuteW(NULL,L"open",L"cmd.exe",com.getData(),NULL,SW_HIDE);
#else
//				system(command.toUtf8().getData());
				remove(command.toUtf8().getData());
#endif
			}
		}

		UInt32 nbrfile = *m_nbrfile; !m_nbrfile;
		UInt32 totalfile = m_totalfile;

		// total percent
		if (nbrfile != totalfile)
			*m_totalpercent = (UInt32)(100.f * ((Float)(totalfile - nbrfile) / (Float)totalfile));
		else
			*m_totalpercent = 0;

		!m_totalpercent;

		m_Semaphore.postSignal();
	}

	m_filetodown->push_back("goodbye!"); !m_filetodown;

	m_downloadThread->waitFinish();
	deletePtr(m_downloadThread);

	*m_currentop = "Update finished!"; !m_currentop;
	*m_isupdate = False; !m_isupdate;

	m_Semaphore.postSignal();
	return 0;
}

/*---------------------------------------------------------------------------------------
  called by the HttpUpdatr::m_DownThread
---------------------------------------------------------------------------------------*/
Int32 HttpUpdater::downThreadFunc(void*)
{
	Bool down = *m_isupdate; !m_isupdate;
	String filename;
	String fullpath;
	String text;

	while (down)
	{
		m_filetodown.lock();
		T_StringList* pList = m_filetodown.get();

		if (pList->size() != 0)
		{
			filename = pList->front();
			pList->pop_front();
			m_filetodown.unlock();

			if ((filename == "goodbye!") && (pList->size() == 0))
				break;

			// download the file
			fullpath = m_rootpathBis + filename;
			File::adaptPath(fullpath);

			text = String("Download: ") + filename;
			*m_currentfile = text; !m_currentfile;

			Http downloadfile(m_server_url,80);
			downloadfile.setRecvCallBack(m_callback);
			downloadfile.download(m_base_uri + filename,fullpath);

			text = filename + " downloaded";
			*m_currentfile = text; !m_currentfile;

			(*m_nbrfile)--; !m_nbrfile;
		}
		else
		{
			m_filetodown.unlock();
		}

		down = *m_isupdate; !m_isupdate;
	}

	*m_currentfile = "Here we go!"; !m_currentfile;
	return 0;
}

