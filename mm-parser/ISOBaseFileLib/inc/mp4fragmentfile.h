#ifndef __Mp4FragmentFile_H__
#define __Mp4FragmentFile_H__
/* =======================================================================
                              mp4FragmentFile.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2008-2013 QUALCOMM Technologies, Inc., All Rights Reserved.
QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/mp4fragmentfile.h#28 $
$DateTime: 2013/09/19 20:51:12 $
$Change: 4465533 $


========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "mpeg4file.h"
#include "videofmt_mp4r.h"

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class AudioPlayer;
#define QTV_MPEG4_MIN_TFRA_REW_LIMIT 2000       //2sec
#define DEFAULTFRAGMENTCOUNT 5000

typedef struct
{
    uint32 fragment_number;
    uint64 fragment_offset;
    boolean fragment_i_frame[VIDEO_FMT_MAX_MEDIA_STREAMS];
    uint64  frames[VIDEO_FMT_MAX_MEDIA_STREAMS];
    uint64  bytes[VIDEO_FMT_MAX_MEDIA_STREAMS];
    uint64  timestamp[VIDEO_FMT_MAX_MEDIA_STREAMS];
} fragment_info_type;

class VideoPlayer; /* forward decl */

class Mp4FragmentFile : public Mpeg4File
{

public:

  Mp4FragmentFile();
  Mp4FragmentFile(  FILESOURCE_STRING filename,
                    unsigned char *pFileBuf=NULL,
                    uint32 bufSize=0,
                    bool bPlayVideo = false,
                    bool bPlayAudio = false,
                    bool bPlayText  = false
#if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || \
    defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
                    ,bool bPseudoStream  = false
                    ,uint32 wBufferOffset = 0
#endif /* defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) */
    ); // local file playback Constructor

#ifdef FEATURE_FILESOURCE_DRM_DCF
   // DCF-file-media playback Constructor
    Mp4FragmentFile(  IxStream* inputStream,
                      bool bPlayVideo = false,
                      bool bPlayAudio = false,
                    bool bPlayText  = false);
#endif
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  Mp4FragmentFile( video::iStreamPort* pPort,
                    bool bPlayVideo = false,
                    bool bPlayAudio = false,
                    bool bPlayText  = false,
                    FileSourceFileFormat eFileFormat = FILE_SOURCE_UNKNOWN);
#endif

  virtual ~Mp4FragmentFile();

  virtual
  bool getTimestampedSampleInfo(video_fmt_stream_info_type *p_track,
                                uint64                      TimeStamp,
                                video_fmt_sample_info_type *sampleInfo,
                                uint64                     *newTimeStamp,
                                bool                        bSetToSyncSample,
                                uint64                      currentPosTimeStamp);

  virtual
  bool getSampleAtTimestamp(video_fmt_stream_info_type *p_track,
                            uint64                      timestamp,
                            bool                        rewind,
                            video_fmt_sample_info_type *sampleInfo);

  virtual uint64 skipNSyncSamples(int offset, uint32 id, bool *bError, uint64 currentTimeStamp);
#ifdef FEATURE_FILESOURCE_PSEUDO_STREAM
  bool parsePseudoStreamLocal( void );
#endif
  virtual uint16 getParseFragmentNum( void );
  virtual uint16 getReadFragmentNum( void );

  virtual uint64 repositionAccessPoint( int32 skipNumber, uint32 id, bool &bError ,uint64 currentPosTimeStampMsec);
  bool getAccessPointSampleInfo(video_fmt_stream_info_type *p_track,
                                int32                      skipNumber,
                                video_fmt_sample_info_type *sampleInfo,
                                uint64                     *newTimeStamp,
                                uint64                      currentPosTimeStampMsec);

  bool getTfraEntryInfo        (uint32                     streamNum,
                                int32                      skipNumber,
                                bool                       rewind,
                                video_fmt_tfra_entry_type  *tfraEntry,
                                uint64                      currentPosTimeStamp);

  bool findSampleFromTfra      (video_fmt_stream_info_type *input_track,
                                bool                       rewind,
                                uint64                     &reqSampleNum,
                                video_fmt_tfra_entry_type  *tfraEntry,
                                boolean                   &fragmentRepositioned);

  /*
    If a user does repeated Rew. and the skipNumber is set to 1, we would always keep going
    to the same previous sync point. In order to avoid this, search for the sync point after a
    pre-defined limit (in the reverse direction) from the current playback pos.
  */
  static uint16 m_minTfraRewindLimit;

  PARSER_ERRORTYPE processFragmentBoundary(video_fmt_stream_info_type *track);

  bool FindKeyFrameFragmentIndex(video_fmt_stream_info_type *pTrack,
                                 uint64                     &rullSampleNum,
                                 bool                       bRewind);
  bool findiFrameFragment(video_fmt_stream_info_type *input_track,
                          uint64                     reqSampleNum,
                          bool                       rewind,
                          boolean                    findiFrame,
                          boolean                    &fragmentParsed);

  virtual uint64 getMovieDuration() const;

private:

  boolean parseUntilSampleFound( video_fmt_stream_info_type *track );
  //void postMessage(QCMessageType *pEvent);//to do
  void locateStreamData(
    video_fmt_stream_info_type  **p_stream_info,
    video_fmt_mp4r_stream_type  **p_stream,
    video_fmt_stream_info_type  *input_track);

  void reinitializeFragmentData(video_fmt_stream_info_type  *input_track,
                                uint32                      fragment_infoindex,
                                uint64                      reqSampleNum,
                                bool                        rewind);
  void reinitializeFragmentStream(video_fmt_mp4r_stream_type  *input_stream,
                                  fragment_info_type          *fragment_info,
                                  uint32                      fragment_infoindex,
                                  uint32                      stream_num,
                                  bool                        rewind);
  void reinitializeFragmentStreamInfo(video_fmt_stream_info_type  *input_streaminfo,
                                      fragment_info_type          *fragment_info,
                                      uint32                      stream_num);

#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
  video_fmt_status_type             m_mp4InitialParseStatus;
#endif

  ZArray<fragment_info_type *> fragmentInfoArray;
  uint32 fragmentinfoCount;
  FileSourceFileFormat m_eFileFormat;
private:

  virtual void InitData();

  virtual PARSER_ERRORTYPE getSampleInfoError(video_fmt_stream_info_type *p_track);

  virtual void process_video_fmt_info(video_fmt_status_type status,
                                      video_fmt_status_cb_info_type *info);

  boolean setMainFragmentBytes();

};

#endif //__Mp4FragmentFile_H__
