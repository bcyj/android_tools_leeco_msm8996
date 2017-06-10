#ifndef __PLAYLISTDOWNLOADHELPER_H__
#define __PLAYLISTDOWNLOADHELPER_H__
/************************************************************************* */
/**
 * PlaylistDownloadHelper.h
 * @brief Header file for PlaylistDownloadHelper.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/PlaylistDownloadHelper.h#11 $
$DateTime: 2013/06/12 21:28:48 $
$Change: 3913306 $

========================================================================== */
/* =======================================================================
**               Include files for PlaylistDownloadHelper.h
** ======================================================================= */

#include <HTTPStackInterface.h>
#include "HTTPCommon.h"
#include "HTTPSessionInfo.h"
#include <Url.h>
namespace video
{
#define HTTP_DEFAULT_PORT "80"
#define HTTP_MAX_HOSTNAME_LEN 50
#define MAX_REPRESENTATION_LEN 512 * 1024

class PlaylistDownloadHelper
{
public:
  PlaylistDownloadHelper(HTTPSessionInfo& sessionInfo,
             HTTPStatusHandlerInterface* pStackNotificationHandler, uint32 nRequestIDfromResolver,
                         HTTPStackInterface *HTTPStack = NULL);
  virtual ~PlaylistDownloadHelper();
  void SetURL(char* url);
  /*Returns the current url*/
  const char* GetURL()
  {
    return m_url;
  }


  HTTPDownloadStatus DownloadAndUpdatePlaylist();
  HTTPDownloadStatus CloseConnection();
  char* GetRepresentationText()
  {
    return m_RepresentationBuffer;
  }
  void DownloadDone();
private:
   enum State
   {
     IDLE,
     WAITING_FOR_PLAYLIST,
     PLAYLIST_READY
   };
    class BaseStateHandler
    {
    public:
      BaseStateHandler(){ };
      virtual ~BaseStateHandler(){ };
      virtual HTTPDownloadStatus Execute(PlaylistDownloadHelper* pDownloadHelper)=0;
      State GetState()
      {
        return m_state;
      }
    protected:
      State m_state;

    };
    void SetStateHandler(BaseStateHandler* pStatehandler)
    {
      m_pStateHandler = pStatehandler;
    };
    BaseStateHandler* GetStateHandler()
    {
      return m_pStateHandler;
    };
    HTTPDownloadStatus CurrentStateHandler(PlaylistDownloadHelper* pDownloadHelper);
    bool ParseURL(const char* pDefaultPort, char*& pLaunchURL);
    HTTPDownloadStatus IsCloseComplete();


private:
  PlaylistDownloadHelper(const PlaylistDownloadHelper&);
  PlaylistDownloadHelper& operator=(const PlaylistDownloadHelper&);
  char* m_pLaunchURL;
  char *m_RepresentationBuffer;
  int m_nRepresentationBytesRead;
  int m_nRepresentationLen;
  char* m_url;
  int m_nNumUrls;
  int m_ncurrentUrl;
  HTTPStackInterface* m_pHTTPStack;
  HTTPSessionInfo& m_sessionInfo;
  bool SwitchToNextURL();

  class IdleStateHandler : public BaseStateHandler
  {
  public:
    IdleStateHandler(){ m_state = IDLE; };
    virtual ~IdleStateHandler(){ };

    virtual HTTPDownloadStatus Execute(PlaylistDownloadHelper* pDownloadHelper);
  };

  class WaitingForPlaylistStateHandler : public BaseStateHandler
  {
  public:
    WaitingForPlaylistStateHandler(){ m_state = WAITING_FOR_PLAYLIST; };
    virtual ~WaitingForPlaylistStateHandler(){ };

    virtual HTTPDownloadStatus Execute(PlaylistDownloadHelper* pDownloadHelper);
  };

  class PlaylistReadyStateHandler : public BaseStateHandler
  {
  public:
    PlaylistReadyStateHandler(){ m_state = PLAYLIST_READY;};
    virtual ~PlaylistReadyStateHandler(){ };

    virtual HTTPDownloadStatus Execute(PlaylistDownloadHelper* pDownloadHelper);
  };
  BaseStateHandler *m_pStateHandler;
  IdleStateHandler m_IdleStateHandler;
  WaitingForPlaylistStateHandler m_WaitingForPlaylistStateHandler;
  PlaylistReadyStateHandler m_PlaylistReadyStateHandler;
  bool bIsStackLocallyAllocated;
  uint32 m_nRequestID;

};
}
#endif
