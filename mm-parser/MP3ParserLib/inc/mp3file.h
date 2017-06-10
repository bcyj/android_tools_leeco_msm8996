#ifndef __MP3File_H__
#define __MP3File_H__
/* =======================================================================
                              mp3file.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2009-2014 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP3ParserLib/main/latest/inc/mp3file.h#34 $
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "oscl_file_io.h"
#include "qcplayer_oscl_utils.h"

#include "filebase.h"
#include "mp3headers.h"
#include "parserdatadef.h"

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
class mp3Parser;
/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */
#define AUDIO_MP3_MAX_TRACKS       1
#define MP3_STREAM_TIME_SCALE      1000 //milisec
#define AUDIO_MP3_MAX_FRAME_SIZE   3000
#define MP3_DEFAULT_AUDIO_BUF_SIZE 800
#define MP3_FILE_READ_BUFFER_SIZE  10000
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
  MP3File

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

class MP3File : public FileBase, public Parentable
{
public:

  MP3File();//Default construnctor
  MP3File(const FILESOURCE_STRING &filename,unsigned char* buf,uint64 bufsize);
  MP3File(IxStream*);
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  MP3File(video::iStreamPort*);
  virtual bool parseHTTPStream();
#endif
  virtual ~MP3File(); // destructor

  /* ======================================================================
  FUNCTION:
    MP3File::getAlbumArt

  DESCRIPTION:
    Provides picture data (album art) in the o/p buffer

  INPUT/OUTPUT PARAMETERS:
    pucDataBuf and pulDatabufLen.

  RETURN VALUE:
    PARSER_ErrorNone if Success
    Error status, if failure

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual PARSER_ERRORTYPE getAlbumArt(wchar_t *pucDataBuf,
                                       uint32 *pulDatabufLen);
  /* ======================================================================
  FUNCTION:
    MP3File::GetClipMetaData

  DESCRIPTION:
    Provides different metadata fields info in the o/p buffer

  INPUT/OUTPUT PARAMETERS:
    pucDataBuf and pulDatabufLen.

  RETURN VALUE:
    PARSER_ErrorNone if Success
    Error status, if failure

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual PARSER_ERRORTYPE GetClipMetaData(wchar_t* pMetaData, uint32* pLength,
                                           FileSourceMetaDataType ienumData);

  // Duration/Time related
  virtual  uint64 getMediaTimestampForCurrentSample(uint32 id);
  virtual uint64 getMovieDuration() const;
  virtual uint32 getMovieTimescale() const;
  virtual bool   getBaseTime(uint32 id, uint64* nBaseTime);
  virtual bool   setBaseTime(uint32 id, uint64 nBaseTime);
  virtual uint64 getTrackMediaDuration(uint32 id);
  virtual uint32 getTrackMediaTimescale(uint32 id);

  virtual uint32 getTrackAudioSamplingFreq(uint32 id);
  virtual uint32 getAudioSamplesPerFrame(uint32 id);
  virtual uint32 getTrackWholeIDList( uint32 *ids);
  virtual int32  getTrackMaxBufferSizeDB(uint32 id);
  virtual int32  getTrackAverageBitrate(uint32 id);
  virtual int32  getTrackMaxBitrate(uint32 id);
  virtual bool   CheckMP3Format();
  virtual uint32 GetNumAudioChannels(int id);
  virtual uint64 GetLastRetrievedSampleOffset(uint32 id);

  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat)
  {
    fileFormat = FILE_SOURCE_MP3;
    return FILE_SOURCE_SUCCESS;
  }
  virtual uint64 GetFileSize() {return m_nFileSize;};
  virtual uint8  getTrackOTIType(uint32 id);
  virtual int32  getNumTracks(){return AUDIO_MP3_MAX_TRACKS;};
  virtual uint64 resetPlayback(uint64 repos_time, uint32 id,
                               bool bSetToSyncSample, bool *bError,
                               uint64 currentPosTimeStamp);
  virtual uint8  randomAccessDenied() {return 0;}
  virtual uint32 FileGetData(uint64 nOffset,
                             uint32 nNumBytesRequest,
                             uint32 nMaxSize,
                             uint8 *pData,
                             bool  &bendofdata);
  virtual PARSER_ERRORTYPE peekCurSample(uint32 id,file_sample_info_type* info);
  virtual PARSER_ERRORTYPE getNextMediaSample(uint32 ulTrackID,
                                              uint8 *pucDataBuf,
                                              uint32 *pulBufSize,
                                              uint32 &rulIndex);
  virtual PARSER_ERRORTYPE getTrackDecoderSpecificInfoContent(uint32 id,
                                                             uint8* buf,
                                                             uint32 *pbufSize);
  virtual PARSER_ERRORTYPE GetStreamParameter(uint32 ulTrackId,
                                              uint32 ulParamIndex,
                                              void* pParamStruct);

  virtual void SetCriticalSection(MM_HANDLE);

  virtual FS_TEXT_ENCODING_TYPE GetMetaDataEncodingType() {return m_eEncodeType;};

  // APIs to set and get audio output modes
  /* ======================================================================
  FUNCTION:
    SetAudioOutputMode

  DESCRIPTION:
    Called by user to set output mode specified by henum

  INPUT/OUTPUT PARAMETERS:
    henum-Output mode

  RETURN VALUE:
    FILE_SOURCE_SUCCESS if successful in setting output mode
    else returns FILE_SOURCE_FAIL

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual FileSourceStatus SetAudioOutputMode(FileSourceConfigItemEnum Enum);
  /* ======================================================================
  FUNCTION:
    GetAudioOutputMode

  DESCRIPTION:
    Called by user to query output mode specified by henum

  INPUT/OUTPUT PARAMETERS:
    henum-Output mode to query

  RETURN VALUE:
   FILE_SOURCE_SUCCESS if successful in retrieving output mode
   else returns FILE_SOURCE_FAIL

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual FileSourceStatus GetAudioOutputMode(bool* bRet,
                                              FileSourceConfigItemEnum Enum);


  private:
    uint64                m_nFileSize;
    uint64                m_nFileBufSize;
    uint64                m_nSeekTime;
    bool                  m_bStreaming;
    bool                  m_bSeekDone;
    FS_TEXT_ENCODING_TYPE m_eEncodeType;

    unsigned char*        m_pFileBuf;
    FILESOURCE_STRING     m_hFileName;
    file_sample_info_type m_hAudSampleInfo;
    OSCL_FILE*            m_pMP3FilePtr;
    IxStream*             m_pIxStream;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
    video::iStreamPort* m_pPort;
#endif
    mp3Parser*            m_pMP3Parser;
    void                  InitData();
    PARSER_ERRORTYPE      ParseMP3Header();
};

#endif  /* __MP3File_H__ */
