#ifndef __PLAYLISTDOWNLOADER_H__
#define __PLAYLISTDOWNLOADER_H__
/************************************************************************* */
/**
 * PlaylistDownloader.h
 * @brief Header file for PlaylistDownloader.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/PlaylistDownloader.h#13 $
$DateTime: 2013/08/16 11:51:00 $
$Change: 4287091 $

========================================================================== */
/* =======================================================================
**               Include files for PlaylistDownloader.h
** ======================================================================= */
#include "HTTPCommon.h"
#include "Scheduler.h"
#include <HTTPStackInterface.h>
#include "HTTPSessionInfo.h"
#include "PlaylistDownloadHelper.h"
namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */
#define MAX_DOWNLOAD_HELPERS 10
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
class PlaylistDownloader
{
public:
    PlaylistDownloader( HTTPSessionInfo&,StreamSourceClock*, HTTPStatusHandlerInterface*, uint32 nRequestIDfromResolver,
                        HTTPStackInterface&);
    virtual ~PlaylistDownloader();
   /*
    * @brief
    * Downloads and updates the playlist. For each download helpers
   */
   HTTPDownloadStatus DownloadAndUpdatePlaylist();
   char *GetRepresentationText(char *url=NULL);
   /* Internally calls seturl on download helper. Sets the url
      for master download helper */
   void SetURL(char* url);
   /* @brief
    * Creates a new download helper and sets the urllist for the download helper
    * param[in] - url list(all failover urls)
    * return - bool
    * true - Successful creation
    * false - otherwise
   */

   HTTPDownloadStatus CloseConnection();

   void SetUpdateAndRefreshTime(char* url,uint32 update_time,uint32 refresh_time);

   int64 GetElapsedTimeForPlaylistUpdate() const;

   /**
   * Set the trigger for the existing playlist downloader task to
   * timeout immediately and request the next playlist.
   */
   void UpdatePlaylist();

private:
   PlaylistDownloader(const PlaylistDownloader&);
   PlaylistDownloader& operator=(const PlaylistDownloader&);
   typedef struct DownloadHelpersInfo
   {
      char *url;
      uint32 update_time;
      uint32 refresh_time;
      PlaylistDownloadHelper* m_pPlaylistDownloadHelper;
   }DownloadHelpersInfo;
   /* This will be used for parallel download of variant lists
      For sequential download, no need of this
      Master downloader can be used */
   DownloadHelpersInfo m_pDownloadHelpersList[MAX_DOWNLOAD_HELPERS];
   void InitializeDownloadHelpers();
   PlaylistDownloadHelper *m_pMasterDownloader;

   char *m_masterurl;
   int m_nNumPlaylistDownloadHelpers;
   bool  m_bMasterDwnlderAvailable;
   uint32 m_nRefreshTime;
   uint32 m_nUpdateTime;
   StreamSourceClock *m_pSourceClock;
   HTTPSessionInfo& m_sessionInfo;
   HTTPStatusHandlerInterface* m_pStackNotificationHandler;

   /**
    * The clock time at which the last mpd started downloading.
    */
   uint32 m_nLastTickCount;

   /**
    * Flag used to reset the tick count to start timing the next
    * mpd download.
    */
   bool m_bShouldResetTickCount;

   /**
    * Amount of time in millisecs taken to download the mpd.
    */
   uint32 m_nDownloadTime;
};
}/* namespace video */
#endif /* __PLAYLISTDOWNLOADER_H__ */
