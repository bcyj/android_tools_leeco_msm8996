/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
 /* =======================================================================
 **               Include files for DASHMMIMediaInfo.h
 ** ======================================================================= */
#ifndef DASH_MMI_MEDIA_INFO_H
#define DASH_MMI_MEDIA_INFO_H

#include <utils/RefBase.h>
#include "AEEStdDef.h"
#include "OMX_Audio.h"
#include "OMX_Video.h"
#include "MMCriticalSection.h"
#include <media/stagefright/MetaData.h>
#include "QOMX_StreamingExtensionsPrivate.h"
#include "HTTPMMIComponent.h"

namespace android
{
/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */
#define MAX_PORTS MMI_HTTP_NUM_VIDEO_PORTS + MMI_HTTP_NUM_AUDIO_PORTS +  \
                  MMI_HTTP_NUM_IMAGE_PORTS + MMI_HTTP_NUM_OTHER_PORTS
#define PORT_BASE MMI_HTTP_PORT_START_INDEX
#define MAX_TRACKS MAX_PORTS
#define TRACK_ID_ALL    0

#define IS_VALID_PORT(x) (((x)>=MMI_HTTP_PORT_START_INDEX) && ((x)<=MMI_HTTP_OTHER_PORT_INDEX))

/* -----------------------------------------------------------------------
** Type Declarations, forward declarations
** ----------------------------------------------------------------------- */
//forward declarations
class DASHMMIInterface;
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
class DASHMMIMediaInfo : public RefBase
{
public:
   /** @brief   Constructor of Http Media Extractor.
    *  @return
    */
    DASHMMIMediaInfo(const sp<DASHMMIInterface> &source,  OMX_U32 &nReturn);


    DASHMMIInterface *GetDASHInterface();

    sp<MetaData> getAudioMetadata(int &nTrackID);
    sp<MetaData> getVideoMetadata(int &nTrackID);
    sp<MetaData> getTextMetadata(int &nTrackID);

    status_t getDuration(int64_t *durationUs);

    bool HasAudio() {return bHasAudio;}
    bool HasVideo() {return bHasVideo;}
    bool HasText()  {return bHasText;}

    status_t setMediaPresence(int kWhome, bool mute);

    enum SeekMode {
       SEEK_PREVIOUS_SYNC,
       SEEK_NEXT_SYNC,
       SEEK_CLOSEST_SYNC,
       SEEK_CLOSEST,
    };


public:
  struct TrackTable
  {
    struct TrackInfo
    {
      int32 m_nTrackId;
      int32 m_nPort;
      sp<MetaData> m_metaData;
    };

    TrackInfo m_trackInfo[MAX_TRACKS];

    TrackTable();
    ~TrackTable();
    TrackInfo *FetchTrackInfo(size_t index,uint32_t mPortIndex);
    bool DoesExist(OMX_U32 idx);
    //void ResetTrackTable();
    void ResetTrackInfo(uint32_t nTrackID);
  };

  TrackTable m_trackTable;
  sp<DASHMMIInterface> m_mmDASHMMIInterface;


private:
  int64 GetDuration(OMX_U32 nPort) const;
  void AddAACCodecSpecificData(sp<MetaData> &metaData);
  const char *AudioEncodingToMIME(OMX_AUDIO_CODINGTYPE coding);
  const char *VideoEncodingToMIME(OMX_VIDEO_CODINGTYPE coding);
  const char *TextEncodingToMIME(OMX_OTHER_FORMATTYPE coding);

  bool bHasAudio;
  bool bHasVideo;
  bool bHasText;

  int32_t nVideoTrackID;
  int32_t nAudioTrackID;
  int32_t nTextTrackID;

  /** @brief   Count number of tracks in session.
   *  @param[in]
   *  @return numTracks
   */
  size_t countTracks();

  /** @brief   Gets meta data from http mmi.
   *  @param[in] index track index
   *             flags
   *  @return MetaData smart pointer
   */
   sp<MetaData> getTrackMetaData(size_t index,uint32_t flags, uint32_t &assignedTrackID);

  /** @brief  helps to get meta data from http mmi
   *  @param[in] index track index
   *  @return MetaData smart pointer
   */
   sp<MetaData> GetMetaData(TrackTable::TrackInfo *track,size_t index,uint32_t mPortIndex);

protected :
  /** @brief   Destructor of Http Media Extractor.
   *  @return
   */
    ~DASHMMIMediaInfo();



};

}

#endif  // DASH_MMI_MEDIA_INFO_H
