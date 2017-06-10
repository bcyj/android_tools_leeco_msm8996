/************************************************************************* */
/**
 * HTTPDownloader.cpp
 * @brief implementation of HTTPDownloader.
 *  HTTPDownloader class primarily controls the initial connection to the
 *  remote server and the actual download of HTTP media data. It interfaces
 *  with HTTPDataManager to get the write buffers and notifies HTTPController
 *  of all status updates. Internally it uses two helper classes for
 *  Progressive Download and Fast Track specific functionality.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPDownloader.cpp#44 $
$DateTime: 2013/09/25 17:37:20 $
$Change: 4496018 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPDownloader.cpp
** ======================================================================= */
#include "HTTPDownloader.h"
#include "HTTPDataManager.h"
#include "HTTPDASHAdaptor.h"
#include <StreamSourceClock.h>
#include <SourceMemDebug.h>
#include <MMFile.h>
#include <MMTimer.h>

namespace video {
/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */
/** @brief HttpDownloader Constructor.
  *
  * @param[in] pStatusHandler - HTTP status handler IQI for notifications
  *                             from HTTPStack (e.g. redirection) - implemented
  *                             by MMI
  * @param[out] bOk - Status of initialization
  */
HTTPDownloader::HTTPDownloader
(
 void* pStatusHandler,
 bool& bOk
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HTTPDownloader::HTTPDownloader" );

  //Initialization
  m_pHttpResolver = NULL;
  m_pScheduler = NULL;
  m_pHTTPStack = NULL;
  m_pDownloadHelper = NULL;
  m_pSourceClock = NULL;
  m_pDownloaderDataLock = NULL;

  m_HTTPFlavor = HTTPCommon::HTTP_NONE;
  m_bSessionInterrupted = false;
  m_nTotalBytesReceived = 0;
  m_bSeekInProgress = false;
  m_pStackNotificationHandler =reinterpret_cast<HTTPStatusHandlerInterface *>(pStatusHandler);
  m_bPaused = false;

  int result = -1;

  bOk = (MM_CriticalSection_Create(&m_pDownloaderDataLock) == 0);

  //Create the scheduler
  if (bOk)
  {
    m_pScheduler = QTV_New_Args(Scheduler, (result));
    bOk = (result == 0 && m_pScheduler);
  }

  //Create the HTTP stack
  if (bOk)
  {
    result = HTTPStackInterface::CreateInstance(&m_pHTTPStack,
                                               (m_pStackNotificationHandler),
                                               m_sessionInfo.GetCookieStore());
    bOk = (result == 0 && m_pHTTPStack);
    if (bOk)
    {
      //Set socket mode to NON_BLOCKING
      (void)m_pHTTPStack->SetSocketMode(HTTP_NON_BLOCKING);
    }
  }
}

/** @brief HTTPDownloader Destructor.
  *
  */
HTTPDownloader::~HTTPDownloader()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPDownloader::~HTTPDownloader" );

  if (m_pDownloadHelper)
  {
    QTV_Delete(m_pDownloadHelper);
    m_pDownloadHelper = NULL;
  }

  if (m_pScheduler)
  {
    QTV_Delete(m_pScheduler);
    m_pScheduler = NULL;
  }

  if (m_pHTTPStack)
  {
    QTV_Delete(m_pHTTPStack);
    m_pHTTPStack = NULL;
  }

  if (m_pSourceClock)
  {
    QTV_Delete(m_pSourceClock);
    m_pSourceClock = NULL;
  }

  if (m_pDownloaderDataLock)
  {
    (void)MM_CriticalSection_Release(m_pDownloaderDataLock);
    m_pDownloaderDataLock = NULL;
  }
}

/** @brief Start the scheduler.
*
* @return
* TRUE - Scheduler successfully started
* FALSE - Otherwise
*/
bool HTTPDownloader::StartScheduler()
{
  bool bOk = false;

  if (m_pScheduler)
  {
    bOk = m_pScheduler->Start();
  }

  return bOk;
}

/** @brief Stop the scheduler.
*
* @return
* TRUE - Scheduler successfully stopped
* FALSE - Otherwise
*/
bool HTTPDownloader::StopScheduler()
{
  bool bOk = false;

  if (m_pScheduler)
  {
    m_pScheduler->Stop();
  }

  return bOk;
}

/** @brief Adds a new scheduler task.
*
* @param[in] pTask - Task ptr for the new scheduler task
* @param[in] pTaskParam - Task parameter
* @return
* TRUE - Scheduler task successfully added
* FALSE - Otherwise
*/
bool HTTPDownloader::AddSchedulerTask
(
 SchedulerTask pTask,
 void* pTaskParam
 )
{
  bool bOk = false;

  if (pTask && pTaskParam && m_pScheduler)
  {
    int taskID = m_pScheduler->AddTask(pTask, pTaskParam);
    if (taskID)
    {
      QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
        "Scheduler task %d added ", taskID );
      bOk = true;
    }
    ((SchedulerTaskParamBase*)pTaskParam)->ntaskID = taskID;
  }

  return bOk;
}

/** @brief Removes a scheduler task.
*
* @param[in] taskID - Scheduler task ID for the task to be deleted
* @return
* TRUE - Scheduler task successfully deleted
* FALSE - Otherwise
*/
bool HTTPDownloader::DeleteSchedulerTask
(
 const int taskID
 )
{
  bool bOk = false;

  if (m_pScheduler)
  {
    bOk = m_pScheduler->DeleteTask(taskID);
  }

  return bOk;
}

HTTPDownloadStatus HTTPDownloader::CreateConnectAndDownloadHeaderTask
(
 uint32 nStartTime,
 const tOpenParams& /* params */,
 HTTPControllerCb pCallback,
 const HTTPControllerCbData& callbackData
 )
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;

  /* Add  TaskConnectAndDownloadHeader task to scheduler   */
  SchedulerTask pTask = TaskConnectAndDownloadHeader;

  //Fill out the task param structure - to be passed to the task
  HTTPDownloaderTaskParam* pTaskParam =
    QTV_New_Args(HTTPDownloaderTaskParam,((void*)this,nStartTime,
                 pCallback,callbackData));
  if (pTaskParam == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Memory allocation failed for taskParam" );
    status = HTTPCommon::HTTPDL_OUT_OF_MEMORY;
  }
  else
  {
    if (!AddSchedulerTask(pTask, (void*)pTaskParam))
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "ConnectAndDownloadHeader task could not be added" );
      status = HTTPCommon::HTTPDL_ERROR_ABORT;
      if (pTaskParam)
      {
        QTV_Delete(pTaskParam);
        pTaskParam = NULL;
      }
    }
  }// if (pTaskParam == NULL)

  return status;
}

/** @brief Creates the download helper class based on HTTP flavor.
*
* @return
* HTTPDL_SUCCESS - Helper class successfully created
* HTTPDL_UNSUPPORTED - Neither Progressive Download nor FastTrack
* HTTPDL_OUT_OF_MEMORY - Memory allocation failure
*/
HTTPDownloadStatus HTTPDownloader::CreateDownloadHelper()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPDownloader::CreateDownloadHelper()" );
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  if (m_pDownloadHelper)
  {
    QTV_Delete(m_pDownloadHelper);
    m_pDownloadHelper = NULL;
  }

  if (m_HTTPFlavor == HTTPCommon::HTTP_DASH)
  {
    bool bOk = false;

    m_pDownloadHelper = QTV_New_Args(DASHAdaptor, (bOk,
                                                   m_sessionInfo, m_pHttpResolver->GetResolverRequestID(), *m_pHTTPStack,
                                                   m_pStackNotificationHandler,
                                                   m_pScheduler));

    if (false == bOk)
    {
      QTV_Delete(m_pDownloadHelper);
      m_pDownloadHelper = NULL;
    }

    if (m_pDownloadHelper == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "Create DASH Adaptor failed" );
      status = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
           "Error: Unsupported HTTP flavor=%d", m_HTTPFlavor);
    status = HTTPCommon::HTTPDL_UNSUPPORTED;
  }

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return status;
}

/** @brief Gets URN info, parses SDP/PVX files if necessary, creates the download
* helper classes based on HTTP flavor and queues up the scheduler task for
* connecting to server.
*
* @param[in] urn - URN to be opened
* @param[in] params - Open parameters
* @param[in] pPlaybackHandler - Reference to MAPI source playback handler
* @param[in] pCallback - Reference to HTTPController callback
* @param[in] callbackData - Reference to HTTPController callback data
* @return
*/
HTTPDownloadStatus HTTPDownloader::StartSession
(
 const URL& urn,
 const tOpenParams& params,
 void* pPlaybackHandler,
 HTTPControllerCb pCallback,
 const HTTPControllerCbData& callbackData
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPDownloader::StartSession()" );
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;

  if (m_pSourceClock)
  {
    QTV_Delete(m_pSourceClock);
    m_pSourceClock = NULL;
  }

  //Create the reference clock for HTTP source
  bool bOk = false;
  m_pSourceClock = QTV_New_Args(StreamSourceClock, (bOk));

  if (m_pSourceClock == NULL || !bOk)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Stream source clock creation failed" );
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
  }
  else if (pCallback == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Callback ptr is NULL" );
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
  }
  else
  {
    URL url;
    if (!GetURNInfo(urn, url))
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "GetURNInfo failed" );
      status = HTTPCommon::HTTPDL_ERROR_ABORT;
    }
    else
    {
      m_bSessionInterrupted = false;
      m_nTotalBytesReceived = 0;
      if (!SetSessionInfo(url, params, pPlaybackHandler))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "SetSessionInfo failed" );
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
      }
      else
      {
        // Populate the HTTPStack Instance with Netpolicy and proxy info before
        // brining up the connection
        if(HTTPCommon::ConfigureHTTPStack(m_sessionInfo,*m_pHTTPStack))
        {
            /* Add  TaskResolveHTTPFlavor task to scheduler   */
            SchedulerTask pTask = TaskResolveHTTPFlavor;

            HTTPDownloaderTaskParam* pTaskParam =
                  QTV_New_Args(HTTPDownloaderTaskParam,((void*)this,m_pSourceClock->GetTickCount(),
                               pCallback,callbackData));
            if( pTaskParam == NULL )
            {
              QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                            "Error: Memory allocation failed for taskParam" );
              status = HTTPCommon::HTTPDL_OUT_OF_MEMORY;
            }
            else
            {
              pTaskParam->openParams = params;
              if( !AddSchedulerTask(pTask, (void*)pTaskParam) )
              {
                QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                              "ResolveHTTPFlavor task could not be added" );
                status = HTTPCommon::HTTPDL_ERROR_ABORT;
                if( pTaskParam )
                {
                  QTV_Delete(pTaskParam);
                  pTaskParam = NULL;
                }
              }
            }// if (pTaskParam == NULL)
          }
          else
          {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                       "ConfigureHTTPStack failed" );
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
        }
      } // if (!SetSessionInfo())
    }// if (!GetURNInfo())
  }

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return status;
}

/** @brief Initializes the downloader
*
* @param[in] pCallback - Reference to HTTPController callback
* @param[in] callbackData - Reference to HTTPController callback data
* @return
*/

HTTPDownloadStatus HTTPDownloader::InitializeDownloader
(
  HTTPControllerCb pCallback,
  const HTTPControllerCbData& callbackData
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPDownloader::InitializeDownloader()" );
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  if (pCallback == NULL || m_pSourceClock == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Either callback ptr or m_pSourceClock is NULL");
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
  }
  else
  {
    //Add DownloadData task
    SchedulerTask pTask = TaskInitializeDownloader;

    //Fill out the task param structure - to be passed to the task
    HTTPDownloaderTaskParam* pTaskParam =
                  QTV_New_Args(HTTPDownloaderTaskParam,((void*)this,m_pSourceClock->GetTickCount(),
                  pCallback,callbackData));

    if (pTaskParam == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: Memory allocation failed for taskParam" );
      status = HTTPCommon::HTTPDL_OUT_OF_MEMORY;
    }
    else
    {
      pTaskParam->pCbSelf = (void*)this;

      //Used as reference for timing out
      pTaskParam->nStartTime = m_pSourceClock->GetTickCount();
      pTaskParam->callbackUserData.pControllerCb = pCallback;
      pTaskParam->callbackUserData.controllerCbData = callbackData;

      if (!AddSchedulerTask(pTask, (void*)pTaskParam))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Initialize Downloader task could not be added" );
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
        if (pTaskParam)
        {
          QTV_Delete(pTaskParam);
          pTaskParam = NULL;
        }
      }
    }// if (pTaskParam == NULL)
  }// if (pCallback == NULL || m_pSourceClock == NULL)

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return status;

}

/** @brief These are no operations for Progressive download helper and
* Fast track download helper but will have an explicit implementation
* in adaptive stream helper.
*
* @return
* HTTPDL_SUCCESS: success.
* HTTPDL_ERROR_ABORT: failure
*/
HTTPDownloadStatus HTTPDownloader::PauseSession()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPDownloader::PauseSession()");
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  HTTPDownloadHelper* pDownloadHelper = m_pDownloadHelper;

  if (pDownloadHelper == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Dwld helper is NULL" );
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
  }
  else
  {
    // explicit implementation in Adaptive Stream helper.
    status = pDownloadHelper->Pause();
    (void)MM_CriticalSection_Enter(m_pDownloaderDataLock);
     m_bPaused = true;
    (void)MM_CriticalSection_Leave(m_pDownloaderDataLock);
  }

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return status;
}

/** @brief These are no operations for Progressive download helper and
* Fast track download helper but will have an explicit implementation
  * in Live Download helper.
*
* @return
* HTTPDL_SUCCESS: success.
* HTTPDL_ERROR_ABORT: failure
*/
HTTPDownloadStatus HTTPDownloader::ResumeSession()
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPDownloader::ResumeSession()" );
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  HTTPDownloadHelper* pDownloadHelper = m_pDownloadHelper;

  if (pDownloadHelper == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Dwld helper is NULL" );
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
     }
     else
     {
    // explicit implementation in Adaptive Stream helper.
    status = pDownloadHelper->Resume();
        (void)MM_CriticalSection_Enter(m_pDownloaderDataLock);
         m_bPaused= false;
        (void)MM_CriticalSection_Leave(m_pDownloaderDataLock);
     }

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return status;
}


/** @brief Get the URN info - urn passed in could be a HTTP URL or SDP/PVX file name.
* Progressive Download could use a SDP input or HTTP URL and Fast Track
* typically starts off ONLY with a pvx file.
*
* @param[in] urn - URN to be opened
* @param[out] url - HTTP URL structure to use to connect to the server
* @return
* TRUE - If the SDP info has been extracted (when SDP is passed in) or
*        if the URN passed in is a valid HTTP URL
* FALSE - otherwise
*/
bool HTTPDownloader::GetURNInfo
(
 const URL& urn,
 URL& url
 )
{
  QTV_MSG_SPRINTF_PRIO_1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPDownloader::GetURNInfo()- %s", urn.GetUrlBuffer() );
  bool bOk = false;

  m_HTTPFlavor = HTTPCommon::HTTP_NONE;

  //Check the URN prefix first and then the suffix
  if (urn == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: URN is NULL" );
  }
  else if (urn.StartsWith("http://"))
  {
    if(urn.EndsWith(".mpd"))
    {
      //MPD file - HTTP DASH session
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "Opening HTTP DASH URL" );

      m_HTTPFlavor = HTTPCommon::HTTP_DASH;
      url = urn;
      bOk = true;
    }
    else
    {
      //HTTP URL - Download session
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
        "Opening HTTP URL" );

      url = urn;
      bOk = true;
    }
  }

  return bOk;
}

/** @brief Set HTTP session information - to be passed to the download helper
*
* @param[in] url - HTTP URL to use to connect to server
* @param[in] params - Open parameters
* @param[in] pPlaybackHandler - Reference to MAPI source playback handler
* @return
* TRUE - Session info successfully set
* FALSE - Otherwise
*/
bool HTTPDownloader::SetSessionInfo
(
 const URL& url,
 const tOpenParams& params,
 void* pPlaybackHandler
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPDownloader::SetSessionInfo()" );
  bool bOk = true;

  //Set common attributes
  m_sessionInfo.SetURL(url);
  m_sessionInfo.SetUserAgent("QualcommTechnologiesHTTPClient");

  QTV_MSG_SPRINTF_PRIO_1( QTVDIAG_HTTP_STREAMING ,QTVDIAG_PRIO_HIGH,
    "HTTP URL for data download - %s", url.GetUrlBuffer() );

  //OEM must provide the data storage type and content handler iface (if it
  //wants to be the playback client) before Open
  DataStorageType tempDataStorageType = iHTTPAPI::HEAP_FILE_SYSTEM;
  if (params.dataStorageType != iHTTPAPI::DEFAULT_STORAGE)
  {
    tempDataStorageType = params.dataStorageType;
  }
  QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
    "Data storage option for data download - %d", tempDataStorageType );
  m_sessionInfo.SetDataStorageType(tempDataStorageType);
  m_sessionInfo.SetDataHeapStorageLimit(params.nHeapStorageLimit);

  m_sessionInfo.SetPlaybackHandler(pPlaybackHandler);
  return bOk;
}

/** @brief ResolveHTTPFlavor task
*
* @param[in] pTaskParam - Task parameter
* @return
* 0 - SUCCESS
* -1 - FAILURE
*/
int HTTPDownloader::TaskResolveHTTPFlavor
(
 void* pTaskParam
 )
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPDownloaderTaskParam* parameter =
    (HTTPDownloaderTaskParam*) pTaskParam;

  if (parameter == NULL || parameter->pCbSelf == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Invalid taskParam" );
  }
  else
  {
    HTTPDownloader* pSelf = (HTTPDownloader*)parameter->pCbSelf;
    StreamSourceClock* pSourceClock = pSelf->m_pSourceClock;
    HTTPControllerCb pCallback = parameter->callbackUserData.pControllerCb;
    HTTPControllerCbData callbackData = parameter->callbackUserData.controllerCbData;

    //Create HttpResolver to resolve between LIVE, DASH and PD
    if (pSelf->m_pHttpResolver == NULL && pSelf->m_pHTTPStack)
    {
      pSelf->m_pHttpResolver =
        QTV_New_Args(HTTPResolver, (pSelf->m_sessionInfo,
                                    *(pSelf->m_pHTTPStack),
                                    HTTP_RESOLVER_REQUEST_LENGTH));
    }

    if (pSourceClock == NULL || pCallback == NULL || pSelf->m_pHttpResolver == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: pSourceClock or pCallback or pHttpResolver is NULL" );
    }
    else
    {
      status = HTTPCommon::HTTPDL_SUCCESS;

      if (pSelf->m_sessionInfo.IsTaskTimeoutDisabled())
      {
        parameter->nStartTime = pSourceClock->GetTickCount();
      }

      //Check if current task has timed out or been interrupted
      if (HTTPCommon::GetElapsedTime(pSourceClock, parameter->nStartTime)
          > callbackData.nTimeout)
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Error: ResolveHTTPFlavor task timed out=%u ms", callbackData.nTimeout);
        status = HTTPCommon::HTTPDL_TIMEOUT;
      }
      else if (pSelf->m_bSessionInterrupted)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Error: ResolveHTTPFlavor task interrupted" );
        status = HTTPCommon::HTTPDL_INTERRUPTED;
      }

      if (HTTPSUCCEEDED(status))
      {
        const char* pContentType = NULL;;
        HTTPResolver* pHttpResolver = (pSelf->m_pHttpResolver);

        status = pHttpResolver->ResolveHTTPFlavor();
        if (status == HTTPCommon::HTTPDL_SUCCESS)
        {
          //Get the content type in sessionInfo and add TaskConnectAndDowloadHeader task to Scheduler
          pContentType = pHttpResolver->GetContentType();

          if (HTTPCommon::HTTP_NONE == pSelf->m_HTTPFlavor && pContentType)
          {
            const char cDASHMimeType[] = "application/dash+xml";
            if (0 == std_strnicmp(pContentType, cDASHMimeType, std_strlen(cDASHMimeType)))
            {
              pSelf->m_HTTPFlavor = HTTPCommon::HTTP_DASH;
            }
            QTV_MSG_SPRINTF_PRIO_2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                                    "HTTPFlavor resolved mimeType %s, flavor %d",
                                    pContentType, pSelf->m_HTTPFlavor );
          }
          status = pSelf->CreateDownloadHelper();
          if (HTTPSUCCEEDED(status))
          {
            status = pSelf->CreateConnectAndDownloadHeaderTask(pSourceClock->GetTickCount(),
                                                               parameter->openParams,
                                                               pCallback, callbackData);
          }
        }
      }// if (HTTPSUCCEEDED(status)
    }// if (pSourceClock == NULL || pCallback == NULL || pSelf->m_pHttpResolver == NULL)

    if (status != HTTPCommon::HTTPDL_WAITING)
    {
      //Delete ResolveHTTPFlavor task
      if(!pSelf->DeleteSchedulerTask(parameter->ntaskID))
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "ResolveHTTPFlavor task could not be deleted, sts=%d", status);
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
      }

      //Notify HTTPController of any failure, TaskConnectAndDownloadHeader will notify
      //OPEN status
      if (pCallback && !HTTPSUCCEEDED(status))
      {
        (void)pCallback(status, callbackData);
      }

      //Destroy HTTP Resolver object
      if (pSelf->m_pHttpResolver)
      {
        QTV_Delete(pSelf->m_pHttpResolver);
        pSelf->m_pHttpResolver = NULL;
      }
    }
  }// if (parameter == NULL || parameter->pCbSelf == NULL)

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return (HTTPSUCCEEDED(status) ? 0 : -1);
}


/** @brief InitializeDownloader task
*
* @param[in] pTaskParam - Task parameter
* @return
* 0 - SUCCESS
* -1 - FAILURE
*/

int HTTPDownloader::TaskInitializeDownloader
(
  void* pTaskParam
)
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
   HTTPDownloaderTaskParam* parameter =
    (HTTPDownloaderTaskParam*) pTaskParam;

  if (parameter == NULL || parameter->pCbSelf == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Invalid taskParam" );
  }
  else
  {
    HTTPDownloader* pSelf = (HTTPDownloader*)parameter->pCbSelf;
    StreamSourceClock* pSourceClock = pSelf->m_pSourceClock;
    HTTPControllerCb pCallback = parameter->callbackUserData.pControllerCb;
    HTTPControllerCbData callbackData = parameter->callbackUserData.controllerCbData;
    HTTPDownloadHelper* pDownloadHelper = pSelf->m_pDownloadHelper;
    if (pSourceClock == NULL || pCallback == NULL || pDownloadHelper == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: pSourceClock or pCallback or pDownloadHelper is NULL" );
    }
    else
    {
      status = HTTPCommon::HTTPDL_SUCCESS;
      if (pSelf->m_sessionInfo.IsTaskTimeoutDisabled())
      {
        parameter->nStartTime = pSourceClock->GetTickCount();
      }

      //Check if current task has timed out or been interrupted
      if (HTTPCommon::GetElapsedTime(pSourceClock, parameter->nStartTime)
          > callbackData.nTimeout)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Error: TaskInitializeDownloader task timed out" );
        status = HTTPCommon::HTTPDL_TIMEOUT;
      }

      if (pSelf->m_bSessionInterrupted)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Error: TaskInitializeDownloader task interrupted" );
        status = HTTPCommon::HTTPDL_INTERRUPTED;
      }

      if (HTTPSUCCEEDED(status))
      {
        status = pDownloadHelper->InitiateHTTPConnection();
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "InitiateHTTPConnection status %d ", status );
      }
    }// if (pSourceClock == NULL || pCallback == NULL || pDownloadHelper == NULL)
    if (status != HTTPCommon::HTTPDL_WAITING)
    {
      if (status != HTTPCommon::HTTPDL_SUCCESS)
      {
         if(pDownloadHelper)
         {
            pDownloadHelper->SetSessionInterruptFlag();
         }
      }
      //Delete InitializeDownloader task
      if (!pSelf->DeleteSchedulerTask(parameter->ntaskID))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "InitializeDownloader task could not be deleted" );
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
      }

      //Notify HTTPController of the download status
      if (pCallback)
      {
        (void)pCallback(status, callbackData);
      }
    }// if (status != HTTPDL_WAITING)
  }// if (parameter == NULL || parameter->pCbSelf == NULL)
  HTTPCommon::ShowHTTPDownloadStatus(status);
  return (HTTPSUCCEEDED(status) ? 0 : -1);
}

/** @brief ConnectAndDownloadHeader task
*
* @param[in] pTaskParam - Task parameter
* @return
* 0 - SUCCESS
* -1 - FAILURE
*/
int HTTPDownloader::TaskConnectAndDownloadHeader
(
 void* pTaskParam
 )
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPDownloaderTaskParam* parameter =
    (HTTPDownloaderTaskParam*) pTaskParam;

  if (parameter == NULL || parameter->pCbSelf == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Invalid taskParam" );
  }
  else
  {
    HTTPDownloader* pSelf = (HTTPDownloader*)parameter->pCbSelf;
    StreamSourceClock* pSourceClock = pSelf->m_pSourceClock;
    HTTPControllerCb pCallback = parameter->callbackUserData.pControllerCb;
    HTTPControllerCbData callbackData = parameter->callbackUserData.controllerCbData;
    HTTPDownloadHelper* pDownloadHelper = pSelf->m_pDownloadHelper;
    if (pSourceClock == NULL || pCallback == NULL || pDownloadHelper == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: pSourceClock or pCallback or pDownloadHelper is NULL" );
    }
    else
    {
      status = HTTPCommon::HTTPDL_SUCCESS;

      if (pSelf->m_sessionInfo.IsTaskTimeoutDisabled())
      {
        parameter->nStartTime = pSourceClock->GetTickCount();
      }

      //Check if current task has timed out or been interrupted
      if (HTTPCommon::GetElapsedTime(pSourceClock, parameter->nStartTime)
          > callbackData.nTimeout)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Error: ConnectAndDownloadHeader task timed out" );
        status = HTTPCommon::HTTPDL_TIMEOUT;
      }
      else if (pSelf->m_bSessionInterrupted)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Error: ConnectAndDownloadHeader task interrupted" );
        status = HTTPCommon::HTTPDL_INTERRUPTED;
      }

      if (HTTPSUCCEEDED(status))
      {
        status = pDownloadHelper->InitiateHTTPConnection();
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "InitiateHTTPConnection status %d ", status );
      }
    }// if (pSourceClock == NULL || pCallback == NULL || pDownloadHelper == NULL)

    if (status != HTTPCommon::HTTPDL_WAITING)
    {
      if (status != HTTPCommon::HTTPDL_SUCCESS)
      {
         if(pDownloadHelper)
         {
            pDownloadHelper->SetSessionInterruptFlag();
         }
      }
      //Delete ConnectAndDownloadHeader task
      if (!pSelf->DeleteSchedulerTask(parameter->ntaskID))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "ConnectAndDownloadHeader task could not be deleted" );
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
      }

      //Notify HTTPController of the download status
      if (pCallback)
      {
        (void)pCallback(status, callbackData);
      }
    }// if (status != HTTPDL_WAITING)
  }// if (parameter == NULL || parameter->pCbSelf == NULL)

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return (HTTPSUCCEEDED(status) ? 0 : -1);
}

/** @brief Queues up the scheduler task for downloading data.
*
* @param[in] pCallback - HTTPController callback
* @param[in] callbackData - HTTPController callback data
* @return
*/
HTTPDownloadStatus HTTPDownloader::StartDownload
(
 HTTPControllerCb pCallback,
 const HTTPControllerCbData& callbackData
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPDownloader::StartDownload()" );
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  if (pCallback == NULL || m_pSourceClock == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Either callback ptr or m_pSourceClock is NULL");
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
  }
  else
  {
    //Add DownloadData task
    SchedulerTask pTask = TaskDownloadData;

    //Fill out the task param structure - to be passed to the task
    HTTPDownloaderTaskParam* pTaskParam =
                  QTV_New_Args(HTTPDownloaderTaskParam,((void*)this,m_pSourceClock->GetTickCount(),
                  pCallback,callbackData));

    if (pTaskParam == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: Memory allocation failed for taskParam" );
      status = HTTPCommon::HTTPDL_OUT_OF_MEMORY;
    }
    else
    {
      pTaskParam->pCbSelf = (void*)this;

      //Used as reference for timing out
      pTaskParam->nStartTime = m_pSourceClock->GetTickCount();
      pTaskParam->callbackUserData.pControllerCb = pCallback;
      pTaskParam->callbackUserData.controllerCbData = callbackData;

      if (!AddSchedulerTask(pTask, (void*)pTaskParam))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "DownloadData task could not be added" );
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
        if (pTaskParam)
        {
          QTV_Delete(pTaskParam);
          pTaskParam = NULL;
        }
      }
    }// if (pTaskParam == NULL)
  }// if (pCallback == NULL || m_pSourceClock == NULL)

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return status;
}

void HTTPDownloader::SetNetAbort()
{
  if(m_pHTTPStack)
  {
    m_pHTTPStack->SetNetAbort();
  }
  return;
}

/** @brief DownloadData task
*
* @param[in] pTaskParam - Task parameter
* @return
* 0 - SUCCESS
* -1 - FAILURE
*/
int HTTPDownloader::TaskDownloadData
(
 void* pTaskParam
 )
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPDownloaderTaskParam* parameter =
    (HTTPDownloaderTaskParam*) pTaskParam;

  if (parameter == NULL || parameter->pCbSelf == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Invalid taskParam" );
  }
  else
  {
    HTTPDownloader* pSelf = (HTTPDownloader*)parameter->pCbSelf;
    StreamSourceClock* pSourceClock = pSelf->m_pSourceClock;
    HTTPControllerCb pCallback = parameter->callbackUserData.pControllerCb;
    HTTPControllerCbData callbackData = parameter->callbackUserData.controllerCbData;
    HTTPDownloadHelper* pDownloadHelper = pSelf->m_pDownloadHelper;
    if (pSourceClock == NULL || pCallback == NULL || pDownloadHelper == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: pSourceClock or pCallback or pDownloadHelper is NULL" );
    }
    else
    {
      status = HTTPCommon::HTTPDL_SUCCESS;

      //Check if current task has timed out due to data inactivity
      //or been interrupted
      if(pSelf->IsPaused())
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "DownloadData task Paused - reset timer = %u", parameter->nStartTime);
        parameter->nStartTime = pSourceClock->GetTickCount();
      }

      if (HTTPCommon::GetElapsedTime(pSourceClock, parameter->nStartTime)
          > callbackData.nTimeout)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Error: DownloadData task timed out" );
        status = HTTPCommon::HTTPDL_TIMEOUT;
      }
      else if (pSelf->m_bSessionInterrupted)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Error: DownloadData task interrupted" );
        status = HTTPCommon::HTTPDL_INTERRUPTED;
      }
      else if (pSelf->m_bSeekInProgress)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "DownloadData task waiting Seek in progress");
        parameter->nStartTime = pSourceClock->GetTickCount();
        status = HTTPCommon::HTTPDL_WAITING;
      }

      if (HTTPSUCCEEDED(status))
      {
        status = pDownloadHelper->GetData();
        //Check if downloader is constantly making progress
        if (status == HTTPCommon::HTTPDL_WAITING ||
            status == HTTPCommon::HTTPDL_TRACKS_AVALIABLE ||
            status == HTTPCommon::HTTPDL_SUCCESS)
        {
          iHTTPPlaybackHandler* pPlaybackHandler = (pSelf->m_sessionInfo).GetPlaybackHandler();
          uint64 totalBytesReceived = pDownloadHelper->GetTotalBytesReceived();
          if ((totalBytesReceived != pSelf->m_nTotalBytesReceived)||
              (pPlaybackHandler && !(pPlaybackHandler->IsBuffering())))
          {
            parameter->nStartTime = pSourceClock->GetTickCount();
            (void)MM_CriticalSection_Enter(pSelf->m_pDownloaderDataLock);
            pSelf->m_nTotalBytesReceived = totalBytesReceived;
            (void)MM_CriticalSection_Leave(pSelf->m_pDownloaderDataLock);
          }

          //Don't run the download task in a tight loop relying solely on
          //OS scheduling, which might cause delays in pulling samples thereby
          //affecting the render rates!! Determine the yield time based on
          //whether we are buffering/playing. Might have to fine tune based
          //on performance for various bitrates
          int waitTime = 5;

          if (pPlaybackHandler && pPlaybackHandler->IsBuffering())
          {
            //If buffering set waitTime to 1 msec so that data is buffered faster,
            //else for HBR streaming we might see frequent re-buffering!!
            waitTime = 1;
          }

          //Optimize by bringing down TCP when data download is complete.
          //Close is posted for error scenarios anyways!!
          if (status == HTTPCommon::HTTPDL_SUCCESS)
          {
            QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
              "Download complete - Closing down server connection" );
            (void)pDownloadHelper->CloseHTTPConnection();
          }
          else
          {
            if(pSelf->m_HTTPFlavor != HTTPCommon::HTTP_DASH)
            {
              (void)MM_Timer_Sleep(waitTime);
            }
          }
        }
      }
    }// if (pSourceClock == NULL || pCallback == NULL || pDownloadHelper == NULL)

    if (status != HTTPCommon::HTTPDL_WAITING &&
        status != HTTPCommon::HTTPDL_TRACKS_AVALIABLE)
    {
      //Delete DownloadData task
      if (!pSelf->DeleteSchedulerTask(parameter->ntaskID))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "DownloadData task could not be deleted" );
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
      }
    }

    //Notify HTTPController of the download status
    if (pCallback)
    {
      (void)pCallback(status, callbackData);
    }
  }// if (parameter == NULL || parameter->pCbSelf == NULL)

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return (HTTPSUCCEEDED(status) ? 0 : -1);
}

/** @brief Queues up the scheduler task for closing HTTP session.
*
* @param[in] pCallback - HTTPController callback
* @param[in] callbackData - HTTPController callback data
* @return
*/
HTTPDownloadStatus HTTPDownloader::CloseSession
(
 HTTPControllerCb pCallback,
 const HTTPControllerCbData& callbackData
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPDownloader::CloseSession()" );
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  if (pCallback == NULL || m_pSourceClock == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Either callback ptr or m_pSourceClock is NULL");
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
  }
  else
  {
    //Add CloseSession task
    SchedulerTask pTask = TaskCloseSession;

    //Fill out the task param structure - to be passed to the task
    HTTPDownloaderTaskParam* pTaskParam =
                  QTV_New_Args(HTTPDownloaderTaskParam,((void*)this,m_pSourceClock->GetTickCount(),
                               pCallback,callbackData));
    if (pTaskParam == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: Memory allocation failed for taskParam" );
      status = HTTPCommon::HTTPDL_OUT_OF_MEMORY;
    }
    else
    {
      if (!AddSchedulerTask(pTask, (void*)pTaskParam))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "CloseSession task could not be added" );
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
        if (pTaskParam)
        {
          QTV_Delete(pTaskParam);
          pTaskParam = NULL;
        }
      }
      else
      {
        m_bSessionInterrupted = true;
        if ( m_pDownloadHelper )
        {
          m_pDownloadHelper->SetSessionInterruptFlag();
        }
      }// if (!AddSchedulerTask())
    }// if (pTaskParam == NULL)
  }// if (pCallback == NULL || m_pSourceClock == NULL)

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return status;
}

/** @brief CloseSession task
*
* @param[in] pTaskParam - Task parameter
* @return
* 0 - SUCCESS
* -1 - FAILURE
*/
int HTTPDownloader::TaskCloseSession
(
 void* pTaskParam
 )
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  HTTPDownloaderTaskParam* parameter =
    (HTTPDownloaderTaskParam*) pTaskParam;

  if (parameter == NULL || parameter->pCbSelf == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Invalid taskParam" );
  }
  else
  {
    HTTPDownloader* pSelf = (HTTPDownloader*)parameter->pCbSelf;
    StreamSourceClock* pSourceClock = pSelf->m_pSourceClock;
    HTTPControllerCb pCallback = parameter->callbackUserData.pControllerCb;
    HTTPControllerCbData callbackData = parameter->callbackUserData.controllerCbData;
    HTTPDownloadHelper* pDownloadHelper = pSelf->m_pDownloadHelper;
    if (pSourceClock == NULL || pCallback == NULL || pDownloadHelper == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: pSourceClock or pCallback or pDownloadHelper is NULL" );
    }
    else
    {
      status = HTTPCommon::HTTPDL_SUCCESS;

      if (pSelf->m_sessionInfo.IsTaskTimeoutDisabled())
      {
        parameter->nStartTime = pSourceClock->GetTickCount();
      }

      //Check if current task has timed out
      if (HTTPCommon::GetElapsedTime(pSourceClock, parameter->nStartTime)
          > callbackData.nTimeout)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Error: CloseSession task timed out" );
        status = HTTPCommon::HTTPDL_TIMEOUT;
      }

      if (HTTPSUCCEEDED(status))
      {
        status = pDownloadHelper->CloseHTTPConnection();
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "CloseHTTPConnection status %d ", status );
      }
    }// if (pSourceClock == NULL || pCallback == NULL || pDownloadHelper == NULL)

    if (status != HTTPCommon::HTTPDL_WAITING)
    {
      //Delete CloseSession task
      if (!pSelf->DeleteSchedulerTask(parameter->ntaskID))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Error: CloseSession task could not be deleted" );
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
      }

      //Notify HTTPController of the download status
      if (pCallback)
      {
        (void)pCallback(status, callbackData);
      }
    }// if (status != HTTPDL_WAITING)
  }// if (parameter == NULL || parameter->pCbSelf == NULL)

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return (HTTPSUCCEEDED(status) ? 0 : -1);
}

/** @brief Queues up the scheduler task for seeking in a HTTP session.
*
* @param[in] seekOffset - Seek start offset
* @param[in] pCallback - HTTPController callback
* @param[in] callbackData - HTTPController callback data
* @return
*/
HTTPDownloadStatus HTTPDownloader::SeekSession
(
 const int64 seekTime,
 HTTPControllerCb pCallback,
 const HTTPControllerCbData& callbackData
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPDownloader::SeekSession()" );
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
  if (pCallback == NULL || m_pSourceClock == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Either callback ptr or m_pSourceClock is NULL");
    status = HTTPCommon::HTTPDL_ERROR_ABORT;
  }
  else
  {
    //Add SeekSession task
    SchedulerTask pTask = TaskSeekSession;

    //Fill out the task param structure - to be passed to the task
     SeekSessionTaskParam* pTaskParam =
                  QTV_New_Args(SeekSessionTaskParam,((void*)this,m_pSourceClock->GetTickCount(),
                               pCallback,callbackData,seekTime));
    if (pTaskParam == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: Memory allocation failed for taskParam" );
      status = HTTPCommon::HTTPDL_OUT_OF_MEMORY;
    }
    else
    {
      if (!AddSchedulerTask(pTask, (void*)pTaskParam))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "SeekSession task could not be added" );
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
        if (pTaskParam)
        {
          QTV_Delete(pTaskParam);
          pTaskParam = NULL;
        }
      }
      else
      {
        m_bSeekInProgress = true;
      }
    }// if (pTaskParam == NULL)
  }// if (pCallback == NULL || m_pSourceClock == NULL)

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return status;
}

/** @brief SeekSession task
*
* @param[in] pTaskParam - Task parameter
* @return
* 0 - SUCCESS
* -1 - FAILURE
*/
int HTTPDownloader::TaskSeekSession
(
 void* pTaskParam
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPDownloader::TaskSeekSession()" );
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  SeekSessionTaskParam* parameter =
    (SeekSessionTaskParam*) pTaskParam;

  if (parameter == NULL || parameter->pCbSelf == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: Invalid taskParam" );
  }
  else
  {
    HTTPDownloader* pSelf = (HTTPDownloader*)parameter->pCbSelf;
    StreamSourceClock* pSourceClock = pSelf->m_pSourceClock;
    HTTPControllerCb pCallback = parameter->callbackUserData.pControllerCb;
    HTTPControllerCbData callbackData = parameter->callbackUserData.controllerCbData;
    HTTPDownloadHelper* pDownloadHelper = pSelf->m_pDownloadHelper;
    if (pSourceClock == NULL || pCallback == NULL || pDownloadHelper == NULL)
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Error: pSourceClock or pCallback or pDownloadHelper is NULL" );
    }
    else
    {
      status = HTTPCommon::HTTPDL_SUCCESS;
      if(pSelf->IsPaused())
      {
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "TaskSeek - Paused state - reset timer = %u", parameter->nStartTime);
        parameter->nStartTime = pSourceClock->GetTickCount();
      }

      if (pSelf->m_sessionInfo.IsTaskTimeoutDisabled())
      {
        parameter->nStartTime = pSourceClock->GetTickCount();
      }

      //Check if current task has timed out
      if (HTTPCommon::GetElapsedTime(pSourceClock, parameter->nStartTime)
          > callbackData.nTimeout)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Error: SeekSession task timed out" );
        status = HTTPCommon::HTTPDL_TIMEOUT;
      }
      else if (pSelf->m_bSessionInterrupted)
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "Error: SeekSession task interrupted" );
        status = HTTPCommon::HTTPDL_INTERRUPTED;
      }

      if (HTTPSUCCEEDED(status) &&
          false == pDownloadHelper->IsAdaptationSetChangePending())
      {
        //NOOP when nSeekOffset is within the ranges of the current
        //buffer, else server interaction is initiated and loop around
        //till server responds
        status = pDownloadHelper->Seek(parameter->nSeekTime);
        QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "Seek status %d ", status );
      }
    }// if (pSourceClock == NULL || pCallback == NULL || pDownloadHelper == NULL)

    if (status != HTTPCommon::HTTPDL_WAITING)
    {
      //Delete SeekSession task
      pSelf->m_bSeekInProgress = false;
      if (!pSelf->DeleteSchedulerTask(parameter->ntaskID))
      {
        QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Error: SeekSession task could not be deleted" );
        status = HTTPCommon::HTTPDL_ERROR_ABORT;
      }

      //Notify HTTPController of the download status
      if (pCallback)
      {
        (void)pCallback(status, callbackData);
      }
    }// if (status != HTTPDL_WAITING)
  }// if (parameter == NULL || parameter->pCbSelf == NULL)

  HTTPCommon::ShowHTTPDownloadStatus(status);
  return (HTTPSUCCEEDED(status) ? 0 : -1);
}


int HTTPDownloader::TaskSelectRepresentations(void *pTaskParam)
{
  SelectRepresentationsTaskParam* parameter = (SelectRepresentationsTaskParam*) pTaskParam;
  HTTPDownloader* pSelf = (HTTPDownloader*)parameter->pCbSelf;
  HTTPControllerCb pCallback = parameter->callbackUserData.pControllerCb;
  HTTPControllerCbData callbackData = parameter->callbackUserData.controllerCbData;
  HTTPDownloadHelper* pDownloadHelper = pSelf->m_pDownloadHelper;
  QTV_NULL_PTR_CHECK(pCallback, 1);
  QTV_NULL_PTR_CHECK(pDownloadHelper, 1);

  if (false == pSelf->m_bSeekInProgress)
  {
    if (false == parameter->m_bIsRequestQueued)
    {
      if (false == pDownloadHelper->IsAdaptationSetChangePending())
      {
        pDownloadHelper->SelectRepresentations(parameter->m_pSelectionsXML);
        parameter->m_bIsRequestQueued = true;
      }
    }
    else
    {
      bool bIsPending = pDownloadHelper->IsAdaptationSetChangePending();
      if (false == bIsPending)
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "Deleting TaskSelectRepresentations");

        //Delete SelectReresentations task
        HTTPDownloadStatus status = HTTPCommon::HTTPDL_SUCCESS;
        if (!pSelf->DeleteSchedulerTask(parameter->ntaskID))
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                        "Error: SelectRepresentations task could not be deleted" );
          status = HTTPCommon::HTTPDL_ERROR_ABORT;
        }
        else
        {
          QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                        "SelectRepresentations task deleted" );
        }

        pDownloadHelper->UpdateTrackSelections();

        //Notify HTTPController of the download status
        if (pCallback)
        {
          (void)pCallback(status, callbackData);
        }

      }// if (status != HTTPDL_WAITING)
    }
  }

  return 0;

}

/** @brief Save and reset HTTP session.
*
* @return
* TRUE - Session reset successfully
* FALSE - Otherwise
*/
bool HTTPDownloader::ResetSession
(
)
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPDownloader::ResetSession()" );
  bool bOk = true;

  if (m_pSourceClock)
  {
    QTV_Delete(m_pSourceClock);
    m_pSourceClock = NULL;
  }

  if (!bOk)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: ResetSession failed" );
  }

  return bOk;
}

/** @brief Gets the total duration of the clip
*
* @param[out] duration - duration of the clip
* @return
* TRUE - If duration is successfully obtained
* FALSE - Otherwise
*/
bool HTTPDownloader::GetTotalDuration(uint32 &duration)
{
  bool bOk=false;

  if (m_pDownloadHelper)
  {
    bOk = m_pDownloadHelper->GetTotalDuration(duration);
  }

  return bOk;
}
/** @brief Check if session is live streaming session
*
* @param[out] bLiveStreamingSession - true if live streaming session
* @return
* TRUE - Live streaming flag successfully obtained
* FALSE - Otherwise
*/
bool HTTPDownloader::IsLiveStreamingSession()
{
  bool bOk=false;

  if (m_pDownloadHelper)
  {
    bOk = m_pDownloadHelper->IsLiveStreamingSession();
  }

  return bOk;

}

/**
* @breif: Set the ProxyServer info with the HTTP stack
*
* @param[in] Proxyserver address string, includes Port
* @param[in] proxyserver address length
*
* @return True if success, false otherwise
*/
bool  HTTPDownloader::SetProxyServer
(
 const char* ProxyServer,
 size_t ProxyServerLen
 )
{
  return m_sessionInfo.SetProxyServer(ProxyServer,ProxyServerLen);
}


/**
* @breif: Set the Cookies info
*
* @param[in] url
* @param[in] cookie
*
* @return 0 if success, -1 otherwise
*/
bool HTTPDownloader::SetCookie
(
  const char *url,
  const char *cookie
)
{
  return m_sessionInfo.SetCookie(url,cookie);
}

/**
* @breif: Get the Cookies info
*
* @param[in] url
* @param[in] cookie
* @param[in/out] cookie len
*
* @return 0 if success, -1 otherwise
*/
bool HTTPDownloader::GetCookies
(
  const char *url,
  const char *cookies,
  size_t &cookiesLen
)
{
  return m_sessionInfo.GetCookies(url,cookies, cookiesLen);
}

/**
 * @brief Get proxySever info from the stack
 *
 * @param ProxyServer
 * @param ProxyServerLen
 * @param ProxyServerLenReq
 *
 * @return  < 0  if there is an error .
 *           0   if the proxyServer or ProxyServerlen is
 *               invalid i.e less than the actual proxyServer
 *               size.
 *           > 0 on success.
 */
int32 HTTPDownloader::GetProxyServer
(
  char* ProxyServer,
  size_t ProxyServerLen,
  size_t &ProxyServerLenReq
)
{
  int32 rsltCode = -1;

  const char* proxyServer = m_sessionInfo.GetProxyServer();

  if(proxyServer)
  {
    size_t proxyServerStrlen = std_strlen(proxyServer);

    if(!ProxyServer || ProxyServerLen <= 0 || proxyServerStrlen == 0)
    {
      ProxyServerLenReq = proxyServerStrlen;
      rsltCode = 0;
    }
    else
    {
      std_strlcpy(ProxyServer,proxyServer,ProxyServerLen);
      rsltCode = 1;
    }
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "No ProxyServer was set");
    rsltCode = -1;
  }
  return rsltCode;
}


/** @brief Sets oem headers
*
* @param[in] whatToDo action to be taken
* @param[in] whichMethodsAffected list of all methods to which
*            these headers applied for
* @param[in] headerName header name
* @param[in] headerValue header value
*
* @return
* TRUE - if successful
* FALSE - Otherwise
*/
bool HTTPDownloader::SetOemHttpHeaders(IPStreamProtocolHeaderCommand whatToDo,
                                       uint32 whichMethodsAffected,
                                       const char *headerName,
                                       const char *headerValue)
{
  bool ret = false;
  ret = m_sessionInfo.SetOemHttpHeaders(whatToDo, whichMethodsAffected, headerName, headerValue);
  return ret;
}

/** @brief Gets oem headers
*
* @param[in] whichMethodsAffected list of all methods to which
*            these headers applied for
* @param[in] headerName header name
* @param[out] headerValue header value
* @param[out] headerValueSize header value
*
* @return
* TRUE - if successful
* FALSE - Otherwise
*/
bool HTTPDownloader::GetOemHttpHeaders(uint32 whichMethodsAffected,
                                       const char *headerName,
                                       char *headerValue,
                                       int& headerValueSize)
{
  bool ret = false;
  ret = m_sessionInfo.GetOemHttpHeaders(whichMethodsAffected, headerName, headerValue, headerValueSize);
  return ret;
}


/**
 * @brief This Method sets the Network Interface that will be
 *        used while bringing up data call
 *
 * @param networkIface
 *
 * @return bool
 */
bool HTTPDownloader::SetNetworkIface(int32 networkIface)
{
  m_sessionInfo.SetNetworkInterface(networkIface);

  return true;

}

/**
 * @brief This Method sets the primaryPDP ProfileNo that will be
 *        used while bringing up data call
 *
 * @param pdpProfileNo
 *
 * @return bool
 */
bool HTTPDownloader::SetPrimaryPDPProfile(int32 pdpProfileNo)
{
  m_sessionInfo.SetPrimaryPDPProfile(pdpProfileNo);

  return true;
}

/**
 * @brief This Method gets the Network Interface thats being
 *        used for Data call bringup
 *
 * @param networkIface
 *
 * @return bool
 */
bool HTTPDownloader::GetNetworkIface(int32 &networkIface)
{
  networkIface = m_sessionInfo.GetNetworkInterface();
  return true;
}


/**
 * @brief This Method gets the primaryPDP ProfileNo thats being
 *        used for Data call bringup
 *
 * @param pdpProfileNo
 *
 * @return bool
 */
bool HTTPDownloader::GetPrimaryPDPProfile(int32 &pdpProfileNo)
{
  pdpProfileNo = m_sessionInfo.GetPrimaryPDPProfile();
  return true;
}



/**
 * @brief This method provides the download progress
 *
 * @param mediaType
 * @param currStartOffset
 * @param downloadOffset
 * @param eUnitsType
 * @param bEOS
 *
 * @return bool
 */
bool HTTPDownloader::GetDownloadProgress
(
  HTTPMediaType mediaType,
  uint32& currStartOffset,
  uint32& downloadOffset,
  HTTPDownloadProgressUnitsType eUnitsType,
  bool& bEOS
)
{
  bool retVal = false;

  if(m_pDownloadHelper)
  {
    retVal = m_pDownloadHelper->GetDownloadProgress(mediaType,currStartOffset,
                                                    downloadOffset,eUnitsType, bEOS);
  }

  return retVal;
}

/**
 * @brief
 *  Set flag so that tasks can refrain from timing out.
 *
 * @param bShouldDisableTimeout
 */
void HTTPDownloader::DisableTaskTimeout(bool bShouldDisableTimeout)
{
  m_sessionInfo.SetDisableTimeout(bShouldDisableTimeout);
}

/**
 *  Query the download helper for the data interface
 *
 *  @return the data interface
 */
HTTPDataInterface *HTTPDownloader::GetDataInterface()
{
  HTTPDataInterface *pDataInterface = NULL;
  if(m_pDownloadHelper)
  {
    pDataInterface = m_pDownloadHelper->GetDataInterface();
  }
  return pDataInterface;
}


/**
 * Retreive the Dash Adapatation Properties
 *
 * @param pPropertiesStr
 * @param nPropertiesLen
 *
 * @return True if success, false otherwise
 */
bool HTTPDownloader::GetMediaProperties
(
  char *pPropertiesStr,
  uint32 &nPropertiesLen
)
{
  bool rslt = false;
  if(m_pDownloadHelper)
  {
    rslt = m_pDownloadHelper->GetMediaProperties(pPropertiesStr,nPropertiesLen);
  }
  return rslt;
}


/**
 * Select the representations
 *
 * @param SetSelectionsXML
 *
 * @return True if success, false otherwise
 */
HTTPDownloadStatus HTTPDownloader::SelectRepresentations
(
  const char* SetSelectionsXML,
  HTTPControllerCb pCallback,
  const HTTPControllerCbData& callbackData
 )
{
  QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPDownloader::SelectRepresentations()" );
  QTV_NULL_PTR_CHECK(SetSelectionsXML, HTTPCommon::HTTPDL_ERROR_ABORT);
  QTV_NULL_PTR_CHECK(m_pDownloadHelper, HTTPCommon::HTTPDL_ERROR_ABORT);

  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  bool bAddTask = false;

  //Fill out the task param structure - to be passed to the task
  SelectRepresentationsTaskParam* pTaskParam =
    QTV_New_Args(SelectRepresentationsTaskParam,((void *)this, 0,pCallback,callbackData));

  if (pTaskParam == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Error: Memory allocation failed for taskParam" );
    status = HTTPCommon::HTTPDL_OUT_OF_MEMORY;
  }
  else
  {
  size_t reqdBufSize = 1 + std_strlen(SetSelectionsXML);
  pTaskParam->m_pSelectionsXML = (char *)QTV_Malloc(reqdBufSize * sizeof(char));

  if (NULL == pTaskParam->m_pSelectionsXML)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Failed to allocate pTaskParam->m_pSelectionsXML");
  }
  else
  {
    std_strlcpy(pTaskParam->m_pSelectionsXML, SetSelectionsXML, reqdBufSize);

    //Add SelectRepresentations task to poll for completion.
    SchedulerTask pTask = TaskSelectRepresentations;

    if (!AddSchedulerTask(pTask, (void*)pTaskParam))
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "SelectRepresentations task could not be added" );

      if (pTaskParam)
      {
        QTV_Delete(pTaskParam);
        pTaskParam = NULL;
      }
    }
    else
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "SelectRepresentations task added" );
        status = HTTPCommon::HTTPDL_SUCCESS;
    }
  }// if (pTaskParam == NULL)
  }

  return status;
}

/**
 * Retreive the Dash MPDText
 *
 * @param pMPDTextStr
 * @param mpdSize
 *
 * @return True if success, false otherwise
 */
bool HTTPDownloader::GetMPDText(char *pMPDTextStr, uint32 &mpdSize)
{
  bool rslt = false;
  if(m_pDownloadHelper)
  {
    rslt = m_pDownloadHelper->GetMPDText(pMPDTextStr,mpdSize);
  }
  return rslt;
}

/**
 * Retreive QOE data
 *
 * @param bandwidth
 * @param pVideoURL
 * @param nURLSize
 * @param pIpAddr
 * @param nIPAddrSize
 *
 */
void HTTPDownloader::GetQOEData(uint32 &bandwidth, char *pVideoURL, size_t& nURLSize, char *pIpAddr, size_t& nIPAddrSize)
{
  if(m_pDownloadHelper)
  {
    m_pDownloadHelper->GetQOEData(bandwidth,pVideoURL,nURLSize,pIpAddr,nIPAddrSize);
  }
}

/**
 * @brief Indicates if the Session is paused
 *
 * @return bool
 */
bool HTTPDownloader::IsPaused()
{
   bool bPaused = false;
  (void)MM_CriticalSection_Enter(m_pDownloaderDataLock);
   bPaused = m_bPaused;
  (void)MM_CriticalSection_Leave(m_pDownloaderDataLock);

  return bPaused;
}
}/* namespace video */
