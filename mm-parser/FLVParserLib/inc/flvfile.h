#ifndef _FLV_FILE_H
#define _FLV_FILE_H
/* =======================================================================
                              FLVFile.h
DESCRIPTION
  Class declaration of Flash Video Parser. Contains declaration of
  overriden members as well as parser specific members

Copyright (c) 2012-2014 QUALCOMM Technologies Inc, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FLVParserLib/main/latest/inc/flvfile.h#5 $
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================
                    INCLUDE FILES FOR MODULE
========================================================================== */
#include "parserdatadef.h"
#include "filebase.h"
#include "filesourcestring.h"
#include "oscl_file_io.h"
#include "flvfiledatadef.h"
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
class FLVParser;

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
  FLVFile

DESCRIPTION
  Flash Parser interface for various data operations

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
class FLVFile : public FileBase, public Parentable
{
public:

  FLVFile(const FILESOURCE_STRING filename,
          unsigned char *pFileBuf = NULL,
          uint32 bufSize = 0 ,
          bool bPlayVideo = false,
          bool bPlayAudio = true);

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  FLVFile(video::iStreamPort*,
          bool bPlayVideo = false,
          bool bPlayAudio = true);
  virtual bool      parseHTTPStream( void );
#endif
  virtual ~FLVFile();
  virtual uint32    FileGetData(uint64, uint32, uint32, uint8*);
  virtual bool      ParseMetaData();

  virtual PARSER_ERRORTYPE     getNextMediaSample(uint32 id,
                                                  uint8  *buf,
                                                  uint32 *size,
                                                  uint32 &index);

  virtual uint64    getMediaTimestampForCurrentSample(uint32 id);

  virtual uint64    resetPlayback(uint64 repos_time,
                                  uint32 id,
                                  bool bSetToSyncSample,
                                  bool *bError,
                                  uint64 currentPosTimeStamp);

  virtual uint8     randomAccessDenied(){return 0;}
  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat)
  {
    fileFormat = FILE_SOURCE_FLV;
    return FILE_SOURCE_SUCCESS;
  }
  virtual uint64    GetFileSize() {return m_fileSize;};
  virtual uint64    getMovieDuration()
                    {return getMovieDurationMsec();  };
  virtual uint32    getMovieTimescale()
                    {return MILLISEC_TIMESCALE_UNIT;};
  virtual uint64    getMovieDurationMsec() const;

  // From Track
  virtual int32     getNumTracks() {return m_nNumStreams;};
  virtual uint32    getTrackWholeIDList(uint32 *ids);

  virtual float     getTrackVideoFrameRate(uint32 id);
  virtual uint32    getTrackVideoFrameWidth(uint32 id);
  virtual uint32    getTrackVideoFrameHeight(uint32 id);

  // From MediaHeader
  virtual uint64    getTrackMediaDuration(uint32 /*id*/)
                    {return getMovieDurationMsec();};
  virtual uint32    getTrackMediaTimescale(uint32 /*id*/)
                    {return MILLISEC_TIMESCALE_UNIT;};
  virtual uint64    getFileSize(){return m_fileSize;}
  virtual uint32    getTrackAudioSamplingFreq(uint32 id);

  virtual PARSER_ERRORTYPE peekCurSample(uint32               trackid,
                                         file_sample_info_type *pSampleInfo);

  virtual PARSER_ERRORTYPE getTrackDecoderSpecificInfoContent(uint32 id,
                                                             uint8* buf,
                                                             uint32 *pbufSize);

  virtual uint8     MapTrackIdToIndex(bool*,uint32);
  virtual void      SetCriticalSection(MM_HANDLE);
  virtual uint32    GetNumAudioChannels(int);
  virtual uint8     getTrackOTIType(uint32 id); // Based on OTI value
  virtual int32     getTrackMaxBufferSizeDB(uint32 id);

private:

  void                    InitData();
  bool                    m_playAudio;
  bool                    m_playVideo;
  bool                    m_playText;
  bool                    m_corruptFile;
  bool                    m_bIsMetaDataParsed;
  bool                    m_bHttpStreaming;

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  video::iStreamPort*     m_pPort;
#endif
  uint32                  m_nNumStreams;
  uint64                  m_fileSize;
  OSCL_FILE*              m_pFilePtr;

  // FLV parser handle
  FLVParser*              m_pFLVParser;
  file_sample_info_type   m_sampleInfo[FILE_MAX_MEDIA_STREAMS];
  FLVTrackIdToIndexTable* m_pIndTrackIdTable;

//TO DO WHEN HTTP SUPPORT IS TO BE ADDED
//#if defined(FEATURE_QTV_3GPP_PROGRESSIVE_DNLD)
//   uint32               m_minOffsetRequired;
//   bool                 bHttpStreaming;
//   boolean              bGetMetaDataSize;
//   boolean              bIsMetaDataParsed;
//   ParserStatusCode     parserState;
//   uint32               m_startupTime;
//   uint32               m_maxPlayableTime[FILE_MAX_MEDIA_STREAMS];
//
//   struct tHttpDataOffset
//   {
//      uint32 Offset;
//      boolean bValid;
//   } m_HttpDataBufferMinOffsetRequired;
//
//   FetchBufferedDataSizeT m_fpFetchBufferedDataSize;
//   FetchBufferedDataT     m_fpFetchBufferedData;
//
//   void          sendParserEvent(ParserStatusCode status);
//   void          sendParseHTTPStreamEvent(void);
//
//   virtual void  updateBufferWritePtr ( uint32 writeOffset );
//
//   virtual bool  CanPlayTracks(uint32 pbTime);
//   virtual bool  parseHTTPStream (void);
//
//   void          sendHTTPStreamUnderrunEvent(void);
//   boolean       GetHTTPStreamDownLoadedBufferOffset(uint32 *pOffset);
//   bool          GetTotalAvgBitRate(uint32 * pBitRate);
//
//#endif defined(FEATURE_QTV_3GPP_PROGRESSIVE_DNLD)
};
#endif//#ifndef _FLV_FILE_H

