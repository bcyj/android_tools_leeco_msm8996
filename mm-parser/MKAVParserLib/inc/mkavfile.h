#ifndef _MKAV_FILE_H
#define _MKAV_FILE_H
/* =======================================================================
                              MKAVFile.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2011-2014 Qualcomm Technologies Inc, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MKAVParserLib/main/latest/inc/mkavfile.h#23 $
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
#include "mkavfiledatadef.h"
#include "parserinternaldefs.h"
#include "filesourcetypes.h"
#ifdef FEATURE_FILESOURCE_MKV_PARSER
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
class MKAVParser;
class Mpeg4Player;

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
  MKAVFile

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
class MKAVFile : public FileBase, public Parentable
{
public:

  MKAVFile(const FILESOURCE_STRING filename, unsigned char *pFileBuf = NULL,
            uint32 bufSize = 0 ,bool bPlayVideo = false, bool bPlayAudio = true);
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  MKAVFile(video::iStreamPort*,bool bPlayVideo = false, bool bPlayAudio = true);
#endif
  virtual ~MKAVFile();
  virtual uint32 FileGetData(uint64, uint32, uint32, uint8*);
  virtual bool   CheckAvailableDataSize(uint64*,bool*);
  virtual bool   ParseMetaData();
  virtual bool   isWebm();

  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat)
  {
    if(isWebm())
    {
      fileFormat = FILE_SOURCE_WEBM;
    }
    else
    {
      fileFormat = FILE_SOURCE_MKV;
    }
    return FILE_SOURCE_SUCCESS;
  }

  virtual PARSER_ERRORTYPE getNextMediaSample(uint32 ulTrackID, uint8 *pucDataBuf,
                                              uint32 *pulBufSize, uint32 &rulIndex);
  virtual uint64 getMediaTimestampForCurrentSample(uint32 id);

  virtual uint64 resetPlayback(uint64 repos_time, uint32 id, bool bSetToSyncSample,
                               bool *bError, uint64 currentPosTimeStamp);

  virtual uint64 skipNSyncSamples(int , uint32 , bool* , uint64 ){return 0;}

  virtual uint8 randomAccessDenied();

  // Functions to get file related different metadata fields
  PARSER_ERRORTYPE GetClipMetaData(wchar_t *pucDataBuf,
                                   uint32 *pulDatabufLen,
                                   FileSourceMetaDataType ienumData);
  virtual uint64 getMovieDuration() const
                {return getMovieDurationMsec();};
  virtual uint32 getMovieTimescale() const
  { return MILLISEC_TIMESCALE_UNIT; };
  virtual uint64 getMovieDurationMsec() const;
  virtual uint64 getFileSize(){return m_ullFileSize;}
  virtual void getCodecDatainRawFormat(bool bRawCodec)
                                      {m_bRawCodecData = bRawCodec;};

  //Following functions are required to get different video codec properties
  virtual uint32 getTrackVideoFrameWidth(uint32 id);
  virtual uint32 getTrackVideoFrameHeight(uint32 id);

  // Functions to get track related metadata fields
  virtual int32  getNumTracks();
  virtual uint32 getTrackWholeIDList(uint32 *ids);
  virtual uint64 getTrackMediaDuration(uint32 /*id*/)
  { return getMovieDurationMsec(); };
  virtual uint32 getTrackMediaTimescale(uint32 /*id*/)
  { return MILLISEC_TIMESCALE_UNIT;};
  virtual uint8  getTrackOTIType(uint32 id); // Based on OTI value
  virtual int32  getTrackAverageBitrate(uint32 id);
  virtual int32  getTrackMaxBufferSizeDB(uint32 id);

  virtual PARSER_ERRORTYPE peekCurSample(uint32 trackid,
                                         file_sample_info_type *pSampleInfo);

  //Following functions are required to get different audio codec properties
  virtual uint16        GetFormatTag(int id);
  virtual uint16        GetAudioVirtualPacketSize(int id);
  virtual uint32        GetBlockAlign(int id);
  virtual uint16        GetAudioAdvancedEncodeOptions(int id);
  virtual uint32        GetAudioAdvancedEncodeOptions2(int id);
  virtual uint32        GetAACAudioProfile(uint32);
  virtual uint32        GetAACAudioFormat(uint32);
  virtual uint32        getTrackAudioSamplingFreq(uint32 id);
  virtual uint32        GetAudioEncoderOptions(int id);
  virtual uint32        GetNumAudioChannels(int id);
  virtual uint32        GetAudioChannelMask(int id);
  virtual uint32        GetAudioBitsPerSample(int id);
  virtual uint32        GetFixedAsfAudioPacketSize(int id)
  { return (uint32)getTrackMaxBufferSizeDB((uint32)id);};

  // this returns codec specific header and size //
  virtual uint8 *getTrackDecoderSpecificInfoContent(uint32 id);
  virtual uint32 getTrackDecoderSpecificInfoSize(uint32 id);
  virtual PARSER_ERRORTYPE getTrackDecoderSpecificInfoContent(uint32 id,
                                                              uint8* buf,
                                                             uint32 *pbufSize);

  virtual uint8  MapTrackIdToIndex(bool*,uint32);
  virtual void   SetCriticalSection(MM_HANDLE);

  // APIs to set and get audio output modes
  virtual FileSourceStatus SetAudioOutputMode(FileSourceConfigItemEnum Enum);
  virtual FileSourceStatus GetAudioOutputMode(bool* bRet,
                                              FileSourceConfigItemEnum Enum);
/* ============================================================================
  @brief  getBufferedDuration.

  @details    This function is used to calculate the playback time based on the
              given Offset value.

  @param[in]      ulTrackId           Track Id.
  @param[in]      sllAvailBytes       Available offset.
  @param[in/out]  pullBufferedTime    Playback Time.

  @return  "TRUE" if successful in calculating the approximate playback time
           else returns "FALSE".
  @note       None.
=============================================================================*/
  bool                      getBufferedDuration(uint32  ulTrackId,
                                                int64   sllOffset,
                                                uint64* pullBufferedTime);
/* ============================================================================
  @brief  GetOffsetForTime.

  @details    This function is used to calculate the approximate offset value
              based on the given Playback timestamp value.

  @param[in]      ullPBTime           Given Playback Time.
  @param[in/out]  pullFileOffset      Parameter to store o/p Offset Value.
  @param[in]      ulTrackId           Track Id.
  @param[in]      currentPosTimeStamp Current Playback Time.
  @param[in]      reposTime           Reposition Time.

  @return  "TRUE" if successful in calculating the approximate offset value
           else returns "FALSE".
  @note       None.
=============================================================================*/
  bool                      GetOffsetForTime(uint64  ullPBTime,
                                             uint64* pullFileoOffset,
                                             uint32  ulTrackID,
                                             uint64  /*currentPosTimeStamp*/,
                                             uint64& /*reposTime*/);
private:

  void                    InitData();

  file_sample_info_type    m_sampleInfo[FILE_MAX_MEDIA_STREAMS];
  uint32                   m_nDecodedDataSize[FILE_MAX_MEDIA_STREAMS];
  uint32                   m_nLargestFrame[FILE_MAX_MEDIA_STREAMS];
  MKAVTrackIdToIndexTable* m_pIndTrackIdTable;

  uint32              m_nNumStreams;
  uint64              m_ullFileSize;
  OSCL_FILE*          m_pFilePtr;

  // MKAV parser handle
  MKAVParser*         m_pMKAVParser;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  video::iStreamPort* m_pPort;
#endif

  bool                m_bHttpStreaming;
  bool                m_playAudio;
  bool                m_playVideo;
  bool                m_playText;
  bool                m_bRawCodecData;

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
   bool bIsMetaDataParsed;
   uint64  m_nAvailableOffset;
   virtual void updateBufferWritePtr ( uint64 writeOffset );
   virtual bool parseHTTPStream ( void );
#endif//#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
};
#endif//#ifndef _MKAV_FILE_H
#endif //#ifdef FEATURE_FILESOURCE_MKV_PARSER

