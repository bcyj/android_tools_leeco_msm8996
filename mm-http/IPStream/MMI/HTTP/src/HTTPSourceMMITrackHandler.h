#ifndef __HTTPSOURCEMMITRACKHANDLER_H__
#define __HTTPSOURCEMMITRACKHANDLER_H__
/************************************************************************* */
/**
 * HTTPSourceMMITrackHandler.h
 * @brief Header file for HTTPSourceMMITrackHandler.
 *
 * COPYRIGHT 2011-2012 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMITrackHandler.h#28 $
$DateTime: 2013/09/03 20:46:31 $
$Change: 4375062 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSourceMMITrackHandler.h
** ======================================================================= */
#include "HTTPCommon.h"
#include "HTTPDataInterface.h"
#include "HTTPDiagInterfaceHandler.h"

#include "filesource.h"
#include <deeplist.h>
#include "HTTPMMIComponent.h"
#include "QOMX_StreamingExtensionsPrivate.h"

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* Numbers of port defines for different domain */
#define MMI_HTTP_NUM_MAX_PORTS (MMI_HTTP_NUM_AUDIO_PORTS + \
                                      MMI_HTTP_NUM_VIDEO_PORTS + \
                                      MMI_HTTP_NUM_IMAGE_PORTS + \
                                      MMI_HTTP_NUM_OTHER_PORTS)


//lets not consider image port and other port for now
#define MMI_HTTP_MAX_PORTS (MMI_HTTP_VIDEO_PORT_INDEX - \
                            MMI_HTTP_PORT_START_INDEX + 1)

//Minimum buffer count for audio, video, image and other ports
#define MMI_HTTP_VIDEO_PORT_MIN_BUFFER_COUNT (3)
#define MMI_HTTP_AUDIO_PORT_MIN_BUFFER_COUNT (3)
#define MMI_HTTP_IMAGE_PORT_MIN_BUFFER_COUNT (3)
#define MMI_HTTP_OTHER_PORT_MIN_BUFFER_COUNT (3)

#define MMI_HTTP_TOTAL_MIN_BUFFER_COUNT MMI_HTTP_VIDEO_PORT_MIN_BUFFER_COUNT+ \
                                MMI_HTTP_AUDIO_PORT_MIN_BUFFER_COUNT+ \
                                MMI_HTTP_IMAGE_PORT_MIN_BUFFER_COUNT+ \
                                MMI_HTTP_OTHER_PORT_MIN_BUFFER_COUNT

//Minimum buffer size for audio, video, image and other ports
#define MMI_HTTP_VIDEO_PORT_DEFAULT_BUFFER_SIZE (250 * 1024)
#define MMI_HTTP_AUDIO_PORT_DEFAULT_BUFFER_SIZE (10 * 1024)
#define MMI_HTTP_IMAGE_PORT_DEFAULT_BUFFER_SIZE (2 * 1024)
#define MMI_HTTP_OTHER_PORT_DEFAULT_BUFFER_SIZE (500 * 1024)

#define MMI_HTTP_VIDEO_PORT_MAX_BUFFER_SIZE ((MAX_RESOLUTION * 3/2) / 2)

#define MMI_HTTP_EXTRA_DATA_PADDING_BITS   3
#define MAX_PSSH_INFO_ENTRY 20

/*
 *  Text track Buffer shuld be able to hold Extra Data Infos
 *  3 (num extraDataInfos including ExtraDataNone) *
 *  OMX_OTHER_EXTRADATATYPE + size of structure to hold sample
 *  Dimensions  + size of Structure to hold sample subs info  +
 *  4*3 Padding bytes needed for each ExtraData)
 *
 */
#define MMI_HTTP_TEXT_EXTRADATA_BUFFER_SIZE    (3 * sizeof(OMX_OTHER_EXTRADATATYPE)) +  \
                                               sizeof(QOMX_SUBTITILE_DIMENSIONS) +  \
                                               sizeof(QOMX_SUBTITLE_SUB_INFOTYPE) +  \
                                               (4 * MMI_HTTP_EXTRA_DATA_PADDING_BITS)

#define MMI_HTTP_MAX_PORT_ENC_BUFFER_SIZE  (3 * sizeof(OMX_OTHER_EXTRADATATYPE))+ \
                                  sizeof(QOMX_EXTRA_SAMPLE_INFO)+ \
                                  sizeof(QOMX_PARAM_STREAMING_PSSHINFO)+ \
                                  (4 * MMI_HTTP_EXTRA_DATA_PADDING_BITS)


/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class HTTPController;
class HTTPSourceMMIPropertiesHandler;

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
class HTTPSourceMMITrackHandler
{
//Constructors/destructor
public:
  HTTPSourceMMITrackHandler(HTTPController* pHTTPController,
                            HTTPSourceMMIPropertiesHandler* pHTTPSourceMMIPropertiesHandler,
                            bool &bResult);
  virtual ~HTTPSourceMMITrackHandler()
  {
    for (uint32 id = 0; id < MAX_PSSH_INFO_ENTRY; id++ )
    {
      DeleteTrackInfoDrmMem(TrackDrmInfo[id]);
    }
    if (m_pTrackHandlerDataLock)
    {
      (void)MM_CriticalSection_Release(m_pTrackHandlerDataLock);
    }
  };

public:
  static const uint32 m_nFormatBlockBufferSize = 256;
  enum TrackPlayState
  {
    UNKNOWN,
    BUFFERING,
    PLAYING
  };

  struct tTrackState
  {
    // A track is selected if data for that track is on the transport
    // stream arriving at the input of the media source.
    bool isSelected;
    // A track is muted if the data for that track is not being
    // demultiplexed out of the transport stream and delivered by the source.
    bool isMuted;
  };

  struct tConfigData
  {
    byte data[m_nFormatBlockBufferSize];
    uint32 size;
  };

  struct TrackDescription
  {
    TrackDescription()
    {
      trackID = -1;
      selectionState.isMuted = selectionState.isSelected = false;
      playState = UNKNOWN;
      majorType = FILE_SOURCE_MJ_TYPE_UNKNOWN;
      minorType = FILE_SOURCE_MN_TYPE_UNKNOWN;
      layerID = timeScale = duration = baseTime = maxSampleSize = numChannels = samplingRate = bitRate = 0;
      reBuffering = bEndOfTrack = false;
      frameHeight = frameWidth = 0;
      std_memset(&config, 0x0, sizeof(tConfigData));
    };

    int32 trackID;
    tTrackState selectionState;
    TrackPlayState playState;
    FileSourceMjMediaType majorType;
    FileSourceMnMediaType minorType;
    uint64 layerID;
    uint64 timeScale;
    uint64 duration;
    bool   reBuffering;
    uint64 baseTime;
    bool bEndOfTrack;
    uint64 maxSampleSize;
    // number of audio channels
    uint64      numChannels;
    // sampling rate
    uint64      samplingRate;
    // config data
    tConfigData config;
    uint32 frameHeight;
    uint32 frameWidth;
    bool   isDRMProtected;
    uint64 bitRate;
  };

  enum BufferDurationUnitsType
  {
    BUFFER_DURATION_UNKNOWN,
    BUFFER_DURATION_TIME,
    BUFFER_DURATION_DATA
  };

  friend class HTTPSourceMMI;

public:
  //init, close methods
  void SetDataInterface( HTTPDataInterface* pDataInterface);

  void Close();

  //Track selection control
  virtual bool SetTrackState(const int trackIdentifier,
                             HTTPMediaType majorType,
                             const tTrackState trackState,
                             void *pUserData);

//Data Read methods - Sample extraction (data path)
public:
 virtual OMX_U32 Read(const int32 trackIdentifier,
                      HTTPMediaType majorType,
                      const int64 timeout,
                      OMX_BUFFERHEADERTYPE *pBuffHdr,
                      int32 ofset,
                      bool& bSwitch);
  virtual bool GetBufferingProgress(const int32 trackID,
                                    HTTPMediaType majorType,
                                    uint64& buffProgress,
                                    uint64& curPos);

  OMX_U32 GetFormatBlock(const HTTPMediaType mediaType, uint8 *buf, uint32 &size);
  HTTPDownloadStatus GetConfigData(const HTTPMediaType mediaType, uint8 *buf, uint32 &size);

//HTTPSourceMMITrackHandler methods
public:
  void ProcessGetTracksStatus();
  void ProcessSetTrackStateStatus(const HTTPDownloadStatus HTTPStatus,
                                  void* pResultRecipient);
  HTTPDownloadStatus CompareAndUpdateTrack(const HTTPMediaType mediaType);

  void SetSeekPending(bool value);
  bool IsSeekPending();
  void SetAbsoluteSeekOffset(const int64 timeToSeek);

  bool IsDownloadComplete(HTTPMediaType mediaType);
  uint32 GetNumberOfTracks(OMX_U32 nIndex);
  bool IsBuffering();
  bool GetMediaDuration(OMX_U32 portIdx,OMX_U32 &duration);
  bool GetTrackID(OMX_U32 portIdx, uint32 &trackID);

  TrackPlayState GetTrackPlayState(uint32 trackID, HTTPMediaType mediaType);
  void MapPortIDToMediaType(OMX_U32 portIdx,HTTPMediaType &mediaType);
  void MapHTTPMinorTypeToFileSourceMinorType( HTTPMediaMinorType httpMinorType,
                                              FileSourceMnMediaType& fsMinorType );
  void MapMediaTypeToPortID(OMX_U32 &portIdx, HTTPMediaType mediaType);
  HTTPDownloadStatus CanPlayTrack(const int32 trackIdentifier,
                                  HTTPMediaType majorType);


  HTTPDownloadStatus GetPsshInfo
  (
    uint32 portIndex,
    int &nUniqueID,
    unsigned char *cDefaultKeyID,
    uint32 &nPsshDataBufSize,
    unsigned char *cPSSHData,
    bool isQueryForSize=false
  );

  OMX_U32 FillExtraDataForExtraSampleInfo
  (
    OMX_U32 nPortIndex,
    OMX_BUFFERHEADERTYPE *pBuffHdr,
    QOMX_PARAM_STREAMING_PSSHINFO *pPsshInfo,
    QOMX_EXTRA_SAMPLE_INFO *pExtraSampleInfo = NULL
  );

  int GenerateUniqueID();
  void PrintTrackDRMInfo();
  void SetLastPsshUniqueIdQueried(FileSourceMjMediaType majorType, int uniqueID);
  int GetLastPsshUniqueIdQueried(FileSourceMjMediaType majorType);
  bool ResetPortInfo();

private:
  //Disallow copy constructor, and assignment operator
  HTTPSourceMMITrackHandler(const HTTPSourceMMITrackHandler& rhs);
  HTTPSourceMMITrackHandler& operator = (const HTTPSourceMMITrackHandler& rhs);

  //auxiliary methods
  bool GetDownloadAndCurrentMediaPos(FileSourceMjMediaType majorType, uint64& downloadPos, uint64& currPos);
  bool MapStreamNumberToTrackID(OMX_U32 portIdx, OMX_U32 streamNo,
                                int32 &trk, HTTPMediaType &majorType);
  bool FindTrack(const int32 trackIdentifier, HTTPMediaType majorType,
                 TrackDescription** ppTrackDescription);
  bool FindMediaType(const HTTPMediaType majorType, TrackDescription** ppTrackDescription);

  OMX_VIDEO_CODINGTYPE GetOmxVideoMinorType(const FileSourceMnMediaType minorType);
  OMX_AUDIO_CODINGTYPE GetOmxAudioMinorType(const FileSourceMnMediaType minorType);
  QOMX_OTHER_CODINGTYPE GetOmxOtherMinorType(const FileSourceMnMediaType minorType);
  OMX_IMAGE_CODINGTYPE  GetOmxImageMinorType(const FileSourceMnMediaType minorType);

  bool UpdateTrackDescription( HTTPMediaTrackInfo &TrackInfo, bool bRebuffering,
                               uint32 &nClipBitrate );
  OMX_U32 FillExtraDataForSubTitles(OMX_U32 nPortIndex, OMX_BUFFERHEADERTYPE *pBuffHdr,
                                    TrackDescription* pTrackDescription,
                                    HTTPSampleInfo &httpSampleInfo);

  HTTPDownloadStatus UpdateTrackDRMInfo(HTTPMediaTrackInfo &TrackInfo);

  void InitializeTrackDescDrmInfo(HTTPDrmInfo &TrackDrmInfo);

  bool CheckDrmInfoUniqueness(FileSourceMjMediaType majorType,
                              HTTPDrmInfo trackInfoDrmInfo);

  bool UpdateTrackDescDRMInfo(HTTPDrmInfo &trackDescDrmInfo,
                              uint32 nTrackID,
                              HTTPCommon::HTTPMediaType majorType,
                              HTTPDrmInfo &trackInfoDrmInfo);

  void DeleteTrackInfoDrmMem(HTTPDrmInfo &trackDrmInfo);

  int GetPsshUniqueId(FileSourceMjMediaType majorType);

private:
  static const HTTPDiagInterfaceHandler::eCodecType MajorTypeToCodecTable[];

  HTTPController* m_pHTTPController;
  HTTPDataInterface* m_pDataInterface;
  HTTPSourceMMIPropertiesHandler* m_pHTTPSourceMMIPropertiesHandler;
  uint32 m_nRebuffPreroll;
  bool m_bInbandH264ParamSet;
  bool m_bEOS;

  //The current absoulte 'Seek'ed location. This is used to help
  //determine if it is okay to pull a sample.
  int64 m_nAbsoluteSeekOffset;

  //Set to true if a Seek operation in pending. Set to false when
  //the pending Seek operation completes.
  bool m_bIsSeekPending;

  DeepList<TrackDescription> m_trackList;

  HTTPDrmInfo TrackDrmInfo[MAX_PSSH_INFO_ENTRY];

  //Critical section used to synchronize access to data members from media threads
  MM_HANDLE m_pTrackHandlerDataLock;

  int m_lastPSSHIndexQueried;
  int m_nUniqueID;
  int m_nLastAudioPsshUniqId;
  int m_nLastVideoPsshUniqId;

};

}/* namespace video */

#endif /* __HTTPSOURCEMMITRACKHANDLER_H__ */
