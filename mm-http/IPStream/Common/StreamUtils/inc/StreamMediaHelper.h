#ifndef STREAMMEDIAHELPER_H
#define STREAMMEDIAHELPER_H
/************************************************************************* */
/**
 * StreamMediaHelper.h
 * @brief Helper class for StreamMedia
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                                Edit History


$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/StreamMediaHelper.h#13 $ $DateTime: 2012/03/20 07:46:30 $ $Author: kamit $
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "AEEStdDef.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
/* Max Sample Sizes by the type of track */
#define MAX_VIDEO_SAMPLE_SIZE 0x0C000
#define MAX_AUDIO_SAMPLE_SIZE 0x01000
#define MAX_TEXT_SAMPLE_SIZE  0x00400

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

class StreamMediaHelper
{
public:
  //Media components
  enum MediaFileType {
    CONTENT_AUDIO,
    CONTENT_VIDEO,
    CONTENT_AUDIO_VIDEO,
    CONTENT_VIDEO_STILL_IMAGE,
    CONTENT_AUDIO_VIDEO_STILL_IMAGE,
    CONTENT_VIDEO_TEXT,
    CONTENT_AUDIO_TEXT,
    CONTENT_AUDIO_VIDEO_TEXT,
    CONTENT_VIDEO_STILL_IMAGE_TEXT,
    CONTENT_AUDIO_VIDEO_STILL_IMAGE_TEXT,
    CONTENT_TEXT,
    CONTENT_UNKNOWN
  };

  //Type of media track
  enum TrackType
  {
    UNKNOWN_TRACK,
    AUDIO_TRACK,
    VIDEO_TRACK,
    TEXT_TRACK
  };

  //Type of codec
  enum CodecType {
    UNKNOWN_CODEC,
    EVRC_CODEC,
    QCELP_CODEC,
    AAC_CODEC,
    BSAC_CODEC,
    GSM_AMR_CODEC,
    MPEG4_CODEC,
    H263_CODEC,
    H264_CODEC,
    STILL_IMAGE_CODEC,
    TIMED_TEXT_CODEC,
    JPEG_CODEC,
    MP3_CODEC,
    WMA_CODEC,
    WMA_PRO_CODEC,
    WMA_PRO_PLUS_CODEC,
    WMV1_CODEC,
    WMV2_CODEC,
    WMV3_CODEC,
    OSCAR_CODEC,
    CONC_CODEC, // audio codec of concurrent image not available for Qtv
    COOK_CODEC,
    SIPR_CODEC,
    RV30_CODEC,
    RV40_CODEC,
    AMR_WB_CODEC,
    AMR_WB_PLUS_CODEC,
    NONMP4_MP3_CODEC,
    QCP_CODEC,
    MIDI_CODEC,
    NONMP4_AAC_CODEC,
    NONMP4_AMR_CODEC,
    MAX_CODEC
  };

  typedef struct
  {
    uint32          trackID;
    uint32          actualTrackID;
    CodecType       codec;
    int32           bitrate;
    int32           numAudioChannels;
    int             dependsOnID;
    int32           frameWidth;
    int32           frameHeight;
    bool            isPlayable;
    bool            isTrackSelected;
    bool            isTrackMuted;
    int32           frameRate;
  } TrackInfo;

  //NAL stream format type
  enum NALStreamFormatType
  {
    NAL_STREAM_FORMAT_START_CODES        = 1,
    NAL_STREAM_FORMAT_NONE               = 2,
    NAL_STREAM_FORMAT_ONE_BYTE_PREFIX    = 4,
    NAL_STREAM_FORMAT_TWO_BYTE_PREFIX    = 8,
    NAL_STREAM_FORMAT_FOUR_BYTE_PREFIX   = 16
  };

  /*
   * Constructors/destructor
   */
  StreamMediaHelper(){ };

  ~StreamMediaHelper(){ };

  // Determine if the codec is audio.
  static bool IsAudioCodec(CodecType codec);
  // Determine if the codec is video
  static bool IsVideoCodec(CodecType codec);
  // Determine if the codec is text
  static bool IsTextCodec(CodecType codec);
  // Determine if the codec is Windows Media audio or video.
  static bool IsWindowsMediaCodec(CodecType codec);
  // Determine if the codec is video or audio or text track
  static TrackType GetTrackType(const CodecType codec);

  static MediaFileType GetFileTypeByComponents(bool hasAudio,
                                               bool hasVideo,
                                               bool hasStillImage,
                                               bool hasText
                                              );

  // Determine whether the player supports this track.
  static bool CanPlay(const TrackInfo *pTrackInfo);

private:

};
#endif /* STREAMMEDIAHELPER_H */
