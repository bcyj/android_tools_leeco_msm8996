#ifndef _FLAC_FILE_H
#define _FLAC_FILE_H
/* =======================================================================
                              flacfile.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

  Copyright(c) 2009-2015 by Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FlacParserLib/main/latest/inc/flacfile.h#18 $
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
#include "flacfileDataDef.h"

#ifdef FEATURE_FILESOURCE_FLAC_PARSER
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
class FlacParser;
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
  flacfile

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
class flacfile : public FileBase, public Parentable
{
public:

  flacfile(const FILESOURCE_STRING filename, unsigned char *pFileBuf = NULL,
           uint64 bufSize = 0);
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  flacfile(video::iStreamPort*);
#endif
  virtual ~flacfile();
  virtual uint32 FileGetData(uint64, uint32, uint32, uint8*);
  virtual bool ParseMetaData();

  virtual PARSER_ERRORTYPE getNextMediaSample(uint32 id, uint8 *buf,
                                              uint32 *size, uint32 &index);
  virtual uint64 getMediaTimestampForCurrentSample(uint32 id);

  virtual uint64 resetPlayback(uint64 repos_time, uint32 id, bool bSetToSyncSample,
                               bool *bError, uint64 currentPosTimeStamp);

  virtual uint8 randomAccessDenied(){return 0;}
  /* ======================================================================
  FUNCTION:
    flacfile::getAlbumArt

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
  PARSER_ERRORTYPE getAlbumArt(wchar_t *pucDataBuf, uint32 *pulDatabufLen);
  /* ======================================================================
  FUNCTION:
    flacfile::GetClipMetaData

  DESCRIPTION:
    Provides different metadata fields info in the o/p buffer

  INPUT/OUTPUT PARAMETERS:
    pucDataBuf      Buf pointer (NULL during length calculation)
    pulDatabufLen   Buf size pointer
    ienumData       ENUM which is required

  RETURN VALUE:
   PARSER_ErrorNone if Success
   Error status, if failure

  SIDE EFFECTS:
    None.
  ======================================================================*/
  PARSER_ERRORTYPE GetClipMetaData(wchar_t *pucDataBuf,
                                   uint32 *pulDatabufLen,
                                   FileSourceMetaDataType ienumData);

  virtual FS_TEXT_ENCODING_TYPE GetMetaDataEncodingType() {return m_eEncodeType;};
  // Methods to get the sample rate (i.e. timescales) for the streams and
// the overall Movie presentation
  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat)
  {
    fileFormat = FILE_SOURCE_FLAC;
    return FILE_SOURCE_SUCCESS;
  }
  virtual uint64 GetFileSize() {return m_fileSize;};
  virtual uint64 getMovieDuration() {return getMovieDurationMsec();};
  virtual uint32 getMovieTimescale() {return MILLISEC_TIMESCALE_UNIT;};

  // From Track
  virtual int32  getNumTracks(){return m_nNumStreams;}
  virtual uint64 getTrackMediaDuration(uint32 /*id*/)
                 {return getMovieDurationMsec();};
  virtual uint32 getTrackMediaTimescale(uint32 /*id*/)
                 {return MILLISEC_TIMESCALE_UNIT;};
  virtual uint8  getTrackOTIType(uint32 /*id*/)
                 {return (uint8)FLAC_AUDIO;};
  virtual uint64 getFileSize(){return m_fileSize;}
  virtual uint32 getTrackWholeIDList(uint32 *ids);

  // From MediaHeader
  virtual uint64 getMovieDurationMsec() const;

  virtual PARSER_ERRORTYPE peekCurSample(uint32 trackid,
                                         file_sample_info_type *pSampleInfo);

  virtual int32  getTrackMaxBufferSizeDB(uint32 id);
  virtual uint32 getTrackAudioSamplingFreq(uint32 id);
  virtual uint32 GetNumAudioChannels(int id);
  virtual uint32 GetAudioBitsPerSample(int id);

  // this returns codec specific header and size //
  virtual PARSER_ERRORTYPE getTrackDecoderSpecificInfoContent(uint32 id,
                                                             uint8* buf,
                                                             uint32 *pbufSize);

  virtual uint8  MapTrackIdToIndex(bool*,uint32);
  virtual void SetCriticalSection(MM_HANDLE);
  virtual bool GetFlacCodecData(int id,flac_format_data* pData);
  /* ======================================================================
  FUNCTION:
    flacfile::GetStreamParameter

  DESCRIPTION:
    Extracts the stream properties "FLAC" properties.

  INPUT/OUTPUT PARAMETERS:
    ulTrackId     Track Id.
    ulParamIndex  Index "FS_IndexParamAudioFlac"
    pParamStruct  Pointer which contains structure for flac

  RETURN VALUE:
   PARSER_ErrorNone in case of success, else returns corresponding error

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual PARSER_ERRORTYPE GetStreamParameter(uint32 ulTrackId,
                                              uint32 ulParamIndex,
                                              void*  pParamStruct);

  virtual FileSourceStatus SetAudioOutputMode(FileSourceConfigItemEnum Enum);
  virtual FileSourceStatus GetAudioOutputMode(bool *bret,
                                              FileSourceConfigItemEnum Enum);
private:
  FlacTrackIdToIndexTable* m_pFlacIndTrackIdTable;
  void                    InitData();

  FlacParser*     m_pFlacParser;// FLAC parser handle
  OSCL_FILE*      m_pFilePtr;
  unsigned char*  m_pFileBuf;  // pointer to buffer for playback from memory
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  video::iStreamPort* m_pPort;
#endif
  file_sample_info_type   m_sampleInfo[FILE_MAX_MEDIA_STREAMS];
  FILESOURCE_STRING       m_filename;  // EFS file path
  //! Enum to check whether metadata is UTF8 format or not
  FS_TEXT_ENCODING_TYPE   m_eEncodeType;
  uint64                  m_FileBufSize;
  uint64                  m_fileSize;
  uint32                  m_nNumStreams;
  // if we are streaming, rather than playing locally
  bool                    m_bStreaming;
  bool                    m_bIsSingleFrameMode;
  FileSourceConfigItemEnum m_hFrameOutputModeEnum;
};
#endif //#ifdef FEATURE_FILESOURCE_FLAC
#endif//#ifndef _FLAC_FILE_H

