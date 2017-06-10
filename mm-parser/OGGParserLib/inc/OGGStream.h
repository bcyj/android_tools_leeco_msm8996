#ifndef _OGG_STREAM_H
#define _OGG_STREAM_H
/* =======================================================================
                              OGGStream.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2009-2014 QUALCOMM Technologies Incorporated, All Rights Reserved.
QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */
/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/OGGParserLib/main/latest/inc/OGGStream.h#21 $
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================
                    INCLUDE FILES FOR MODULE
========================================================================== */
#include "AEEStdDef.h"
#include "filebase.h"
#include "filesourcestring.h"
#include "oscl_file_io.h"
#include "OGGStreamDataDef.h"

#ifdef FEATURE_FILESOURCE_OGG_PARSER
/* ==========================================================================
                       DATA DECLARATIONS
========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class OGGStreamParser;
/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Function Declaration
** ======================================================================= */

/* =======================================================================
**                        Class Declarations
** ======================================================================= */

/* ======================================================================
CLASS
  OGGStream

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class OGGStream : public FileBase, public Parentable
{
public:

  OGGStream(const FILESOURCE_STRING filename, unsigned char *pFileBuf = NULL,
            uint32 bufSize = 0 ,bool bPlayVideo = false, bool bPlayAudio = true);
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  OGGStream(video::iStreamPort*,bool bPlayVideo = false, bool bPlayAudio = true);
#endif
  virtual ~OGGStream();
  virtual uint32 FileGetData(uint64, uint32, uint32, uint8*);
  virtual bool ParseMetaData();

  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat)
  {
    fileFormat = FILE_SOURCE_OGG;
    return FILE_SOURCE_SUCCESS;
  }
  virtual uint64 GetFileSize() {return m_fileSize;};
  virtual PARSER_ERRORTYPE getNextMediaSample(uint32 ulTrackID, uint8 *pucDataBuf,
                                              uint32 *pulBufSize, uint32 &rulIndex);
  virtual uint64 getMediaTimestampForCurrentSample(uint32 id);

  virtual uint64 resetPlayback(uint64 repos_time, uint32 id, bool bSetToSyncSample,
                               bool *bError, uint64 currentPosTimeStamp);

  virtual uint8 randomAccessDenied(){return 0;}

  virtual PARSER_ERRORTYPE  GetClipMetaData(wchar_t *pucDataBuf,
                                            uint32 *pulDatabufLen,
                                            FileSourceMetaDataType ienumData);

  // Methods to get the sample rate (i.e. timescales) for the streams and
  // the overall Movie presentation
  virtual uint64 getMovieDuration()
                 {return getMovieDurationMsec();};
  virtual uint32 getMovieTimescale()
                 {return MILLISEC_TIMESCALE_UNIT;};
  virtual uint64 getTrackMediaDuration(uint32 /*id*/)
                 {return getMovieDurationMsec();};
  virtual uint32 getTrackMediaTimescale(uint32 /*id*/)
                 {return MILLISEC_TIMESCALE_UNIT;};
  virtual uint64 getMovieDurationMsec() const;

  // From Track
  virtual int32 getNumTracks(){return m_nNumStreams;}
  virtual uint32 getTrackWholeIDList(uint32 *ids);

  virtual float  getTrackVideoFrameRate(uint32 id);
  virtual uint32 getTrackVideoFrameWidth(uint32 id);
  virtual uint32 getTrackVideoFrameHeight(uint32 id);

  // From MediaHeader
  virtual uint32 getTrackAudioSamplingFreq(uint32 id);
  virtual uint32 GetAudioBitsPerSample(int id);

  virtual PARSER_ERRORTYPE peekCurSample(uint32 trackid,
                                         file_sample_info_type *pSampleInfo);

  virtual uint8  getTrackOTIType(uint32 id); // Based on OTI value
  virtual int32  getTrackAverageBitrate(uint32 id);
  virtual int32  getTrackMinBitrate(uint32 id);
  virtual int32  getTrackMaxBitrate(uint32 id);
  virtual int32  getTrackMaxBufferSizeDB(uint32 id);
  virtual uint32 GetNumAudioChannels(int id);

  // this returns codec specific header and size //
  virtual PARSER_ERRORTYPE getTrackDecoderSpecificInfoContent(uint32 id,
                                                             uint8* buf,
                                                             uint32 *pbufSize);

  virtual uint8  MapTrackIdToIndex(bool*,uint32);

  virtual void SetCriticalSection(MM_HANDLE);
  virtual bool GetFlacCodecData(int id,flac_format_data* pData);

  virtual FS_TEXT_ENCODING_TYPE GetMetaDataEncodingType() {return m_eEncodeType;};

private:

  void                    InitData();
  OggTrackIdToIndexTable* m_pIndTrackIdTable;
  OSCL_FILE*              m_pFilePtr;
  OGGStreamParser*        m_pOGGStreamParser; // OGG parser handle
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  video::iStreamPort*     m_pPort;
#endif
  //! Enum to check whether metadata is UTF8 format or not
  FS_TEXT_ENCODING_TYPE   m_eEncodeType;
  uint64                  m_fileSize;
  uint8                   m_nNumStreams;
  bool                    m_playAudio;
  bool                    m_playVideo;
  // if we are streaming, rather than playing locally
  bool                    m_bStreaming;
  file_sample_info_type   m_sampleInfo[FILE_MAX_MEDIA_STREAMS];
};
#endif //FEATURE_FILESOURCE_OGG_PARSER
#endif//#ifndef _OGG_STREAM_H

