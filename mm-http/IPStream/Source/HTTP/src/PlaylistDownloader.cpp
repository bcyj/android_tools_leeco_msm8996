
/************************************************************************* */
/**
 * PlaylistDownloader.cpp
 * @brief implementation of PlaylistDownloader.
 *  PlaylistDownloader with the help of PlaylistDownloadHelper initiates the
 *  download of playlist file.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/PlaylistDownloader.cpp#15 $
$DateTime: 2013/06/12 21:28:48 $
$Change: 3913306 $

========================================================================== */
/* =======================================================================
**               Include files for PlaylistDownloader.cpp
** ======================================================================= */
#include "PlaylistDownloader.h"
#include <SourceMemDebug.h>

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
/** @brief PlaylistDownloader Constructor.  */

PlaylistDownloader::PlaylistDownloader
(
  HTTPSessionInfo& sessionInfo,
  StreamSourceClock* pSourceClock,
  HTTPStatusHandlerInterface* pStackNotificationHandler,
  uint32 nRequestIDfromResolver,
  HTTPStackInterface& HTTPStack
)
:m_nNumPlaylistDownloadHelpers(0),
  m_bMasterDwnlderAvailable(true),
  m_nRefreshTime(0),
  m_nUpdateTime(0),
  m_pSourceClock(pSourceClock),
  m_sessionInfo(sessionInfo),
  m_pStackNotificationHandler(pStackNotificationHandler),
  m_nLastTickCount(0),
  m_bShouldResetTickCount(true),
  m_nDownloadTime(0)
{
  m_pMasterDownloader = QTV_New_Args(PlaylistDownloadHelper,(sessionInfo,pStackNotificationHandler, nRequestIDfromResolver,
                                                             &HTTPStack));
  if(m_pMasterDownloader == NULL)
  {
    QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "Error: PlaylistDownloadHelper creation failed" );
  }
  InitializeDownloadHelpers();
}
/* @brief - PlaylistDownloader destructor
 */
PlaylistDownloader::~PlaylistDownloader()
{
  if(m_pMasterDownloader)
  {
    QTV_Delete(m_pMasterDownloader);
    m_pMasterDownloader = NULL;
  }
  if(m_pDownloadHelpersList)
  {
    for(int i=0;i<m_nNumPlaylistDownloadHelpers;i++)
    {
      QTV_Delete(m_pDownloadHelpersList[i].m_pPlaylistDownloadHelper);
    }
  }

}
void PlaylistDownloader::InitializeDownloadHelpers()
{
  for(int i=0;i<MAX_DOWNLOAD_HELPERS;i++)
  {

    m_pDownloadHelpersList[i].url=NULL;
    m_pDownloadHelpersList[i].update_time=0;
    m_pDownloadHelpersList[i].refresh_time=0;
    m_pDownloadHelpersList[i].m_pPlaylistDownloadHelper=NULL;
  }
  return;
}
/* Sets the url in playlistdownloadhelper
*/
void PlaylistDownloader::SetURL(char *url)
{
  if(m_bMasterDwnlderAvailable && m_pMasterDownloader)
  {
    m_pMasterDownloader->SetURL(url);
  }
  return;
}
/* Gets the representation text from playlistdownload helper
*/
char* PlaylistDownloader::GetRepresentationText(char* url)
{
  char *repText=NULL;
  if(m_bMasterDwnlderAvailable && m_pMasterDownloader)
  {
    repText= m_pMasterDownloader->GetRepresentationText();
  }
  else
  {
    for(int i=0;i<m_nNumPlaylistDownloadHelpers;i++)
    {

      if(url && !std_strcmp(m_pDownloadHelpersList[i].url,url))
      {
        PlaylistDownloadHelper *pDwnldHelper =m_pDownloadHelpersList[i].m_pPlaylistDownloadHelper;
        repText= pDwnldHelper->GetRepresentationText();
      }
    }

  }
  return repText;
}
/* @brief - Initiates the download of playlist file with
* the help of playlist download helper
*/
HTTPDownloadStatus PlaylistDownloader::DownloadAndUpdatePlaylist()
{
  HTTPDownloadStatus status=HTTPCommon::HTTPDL_ERROR_ABORT;

  int success_cnt = 0;
  if(m_bMasterDwnlderAvailable && m_pMasterDownloader)
  {
    if(HTTPCommon::GetElapsedTime(m_pSourceClock,
      m_nUpdateTime) >
      m_nRefreshTime)
    {
      if (m_bShouldResetTickCount)
      {
        // Start timing the mpd download.
        m_nLastTickCount = m_pSourceClock->GetTickCount();
        m_bShouldResetTickCount = false;
      }

      status = m_pMasterDownloader->DownloadAndUpdatePlaylist();

      if (HTTPCommon::HTTPDL_SUCCESS == status)
      {
        uint32 curTickCount = m_pSourceClock->GetTickCount();
        m_nDownloadTime = curTickCount - m_nLastTickCount;
        m_nLastTickCount = curTickCount;
        m_bShouldResetTickCount = true;
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
          "MPD Download time %u, curTicCount %u", m_nDownloadTime, (uint32)curTickCount);
      }
    }
    else
    {
      status = HTTPCommon::HTTPDL_WAITING;
    }
  }
  else
  {
    success_cnt=0;
    for(int i=0;i<m_nNumPlaylistDownloadHelpers;i++)
    {
      if(HTTPCommon::GetElapsedTime(m_pSourceClock,
        m_pDownloadHelpersList[i].update_time) >
        m_pDownloadHelpersList[i].refresh_time)
      {
        PlaylistDownloadHelper *pDwnldHelper =m_pDownloadHelpersList[i].m_pPlaylistDownloadHelper;
        if(pDwnldHelper)
        {
          status=pDwnldHelper->DownloadAndUpdatePlaylist();

        }
      }
      else
      {
        status = HTTPCommon::HTTPDL_WAITING;
      }
      if(status != HTTPCommon::HTTPDL_SUCCESS && status != HTTPCommon::HTTPDL_WAITING)
      {
        break;
      }
      else if(status == HTTPCommon::HTTPDL_SUCCESS)
      {
        success_cnt++;
      }

    }
  }
  if(status == HTTPCommon::HTTPDL_SUCCESS)
  {
    if(success_cnt!=m_nNumPlaylistDownloadHelpers)
    {
      status = HTTPCommon::HTTPDL_WAITING;
    }
    else
    {
      if(m_bMasterDwnlderAvailable && m_pMasterDownloader)
      {
        m_pMasterDownloader->DownloadDone();
      }
      for(int i=0;i<m_nNumPlaylistDownloadHelpers;i++)
      {
        PlaylistDownloadHelper *pDwnldHelper =m_pDownloadHelpersList[i].m_pPlaylistDownloadHelper;
        if(pDwnldHelper)
        {
          pDwnldHelper->DownloadDone();
        }
      }
    }
  }
  return status;
}
void PlaylistDownloader::SetUpdateAndRefreshTime(char* url,uint32 update_time,uint32 refresh_time)
{
  if(m_bMasterDwnlderAvailable && m_pMasterDownloader)
  {
    m_nUpdateTime = update_time;

    // Take into account the time spent in downloading the playlist file.
    m_nRefreshTime = (refresh_time >= m_nDownloadTime
                      ? refresh_time - m_nDownloadTime
                      : 0);

    // Decrease the refresh time by 1 second to account for scheduling overhead etc.
    m_nRefreshTime = (m_nRefreshTime > 1000 ? m_nRefreshTime - 1000 : 0);
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "PlaylistDownloader::SetUpdateAndRefreshTime %u ms, updatetime %u ms",
                  (uint32)m_nRefreshTime, (uint32)m_nUpdateTime);
  }
  else
  {

    for(int i=0;i<m_nNumPlaylistDownloadHelpers;i++)
    {
      if(url && !std_strcmp(m_pDownloadHelpersList[i].url,url))
      {
        m_pDownloadHelpersList[i].update_time=update_time;
        m_pDownloadHelpersList[i].refresh_time=refresh_time;
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,"update time %u refresh time %u",update_time,refresh_time);
        QTV_MSG_SPRINTF_1(QTVDIAG_HTTP_STREAMING,"url is %s",url);
      }
    }
  }

  return;
}

int64 PlaylistDownloader::GetElapsedTimeForPlaylistUpdate() const
{
  QTV_NULL_PTR_CHECK(m_pSourceClock, 0);
  int64 curTime = m_pSourceClock->GetTickCount();
  int64 nElapsedTime = curTime - m_nUpdateTime;
  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
    "IsPlaylistRefreshedOnTime lastMPDRecd %u, curTime %lld, minUpdatePeriod %lld",
    m_nUpdateTime, curTime, nElapsedTime);

  return nElapsedTime;
}

/**
 * Set the trigger for the existing playlist downloader task to
 * timeout immediately and request the next playlist.
 */
void PlaylistDownloader::UpdatePlaylist()
{
  m_nRefreshTime = 0;
}

/* @brief Closes the http connection
*/
HTTPDownloadStatus PlaylistDownloader::CloseConnection()
{
  HTTPDownloadStatus status = HTTPCommon::HTTPDL_ERROR_ABORT;
  if(m_pMasterDownloader)
  {
    status = m_pMasterDownloader->CloseConnection();
  }
  for(int i=0;i<m_nNumPlaylistDownloadHelpers;i++)
  {
    PlaylistDownloadHelper *pDwnldHelper =m_pDownloadHelpersList[i].m_pPlaylistDownloadHelper;
    if(pDwnldHelper)
    {
      status=pDwnldHelper->CloseConnection();

    }
  }
  return status;
}

}
