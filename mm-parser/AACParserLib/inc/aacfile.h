#ifndef __AACFile_H__
#define __AACFile_H__
/* =======================================================================
                              aacfile.h
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
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AACParserLib/main/latest/inc/aacfile.h#36 $
$DateTime: 2013/08/28 01:52:54 $
$Change: 4345281 $

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
#include "aacheaders.h"
#include "parserdatadef.h"
#include "filesourcetypes.h"
class IxStream;
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
class aacParser;
/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */
#define AUDIO_AAC_MAX_TRACKS      1
#define AAC_STREAM_TIME_SCALE 1000  //milisec
#define AUDIO_AAC_MAX_FRAME_SIZE  3000
#define AAC_DEFAULT_AUDIO_BUF_SIZE 3000

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
  AACFile

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

class AACFile : public FileBase, public Parentable
{
private:
  uint8*           m_pFileBuf;  // pointer to buffer for playback from memory
  uint64            m_FileBufSize;
  uint64            m_fileSize;
  uint64            m_uSeektime;
  OSCL_FILE*        m_AACFilePtr;  //pointer to the file
  aacParser*        m_paacParser;  // handle to amrparser module
  IxStream*         m_pIxStream;
  FILESOURCE_STRING m_filename;    // EFS file path //
  FS_TEXT_ENCODING_TYPE m_eEncodeType; //! Metadata Encoding type
  bool              m_bStreaming;
  bool              m_SEEK_DONE;
  bool              m_bHttpStreaming;
  file_sample_info_type m_audsampleinfo;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  video::iStreamPort* m_pPort;
#endif
private:
  PARSER_ERRORTYPE ParseAACHeader(); //calls new on AAC parser
  void InitData();

public:

  AACFile(); //Default construnctor

  AACFile(const FILESOURCE_STRING &filename, unsigned char *pFileBuf,
          uint64 bufSize);

  AACFile(IxStream*);
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  AACFile(video::iStreamPort*);
  virtual bool parseHTTPStream();
#endif
  virtual ~AACFile(); // destructor

  // Metadata related
  virtual PARSER_ERRORTYPE getAlbumArt(wchar_t *pucDataBuf,
                                       uint32 *pulDatabufLen);
  virtual PARSER_ERRORTYPE GetClipMetaData(wchar_t* pMetaData, uint32* pLength,
                                           FileSourceMetaDataType ienumData);

   // Time related
  virtual uint64 getMovieDuration() const;
  virtual uint32 getMovieTimescale()
                 {return MILLISEC_TIMESCALE_UNIT;};
  virtual uint64 getTrackMediaDuration(uint32 /*id*/)
                 {return getMovieDuration();};
  virtual uint32 getTrackMediaTimescale(uint32 /*id*/)
                 {return MILLISEC_TIMESCALE_UNIT;};
  virtual bool   getBaseTime(uint32 id, uint64* nBaseTime);
  virtual bool   setBaseTime(uint32 id, uint64 nBaseTime);
  virtual uint64 getMediaTimestampForCurrentSample(uint32 id);

  // Audio Track properties
  virtual uint32 getTrackAudioSamplingFreq(uint32 id);
  virtual uint32 GetNumAudioChannels(int id);
  virtual uint32 GetAACAudioProfile(uint32 id);
  virtual uint32 GetAACAudioFormat(uint32 id);
  virtual uint8  getTrackOTIType(uint32 id);// Based on OTI value
  virtual uint32 getTrackWholeIDList( uint32 *ids);
  virtual int32  getTrackMaxBufferSizeDB(uint32 id);
  virtual int32  getTrackAverageBitrate(uint32 id);
  virtual uint64 GetLastRetrievedSampleOffset(uint32 id);
  virtual bool   CheckAacFormat(); //check aac format
  virtual FS_TEXT_ENCODING_TYPE GetMetaDataEncodingType() {return m_eEncodeType;};

  // Inline functions
  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat)
  {
    fileFormat = FILE_SOURCE_AAC;
    return FILE_SOURCE_SUCCESS;
  }
  virtual uint64 GetFileSize() {return m_fileSize;};
  virtual int32  getNumTracks() {return AUDIO_AAC_MAX_TRACKS;};

  //API to read Data from buffer/input file
  uint32 FileGetData(uint64 nOffset,uint32 nNumBytesRequest,
                     uint32 nMaxSize,uint8 *ppData, bool &bendofdata);

  // APIs to set and get audio output modes
  virtual FileSourceStatus SetAudioOutputMode(FileSourceConfigItemEnum Enum);
  virtual FileSourceStatus GetAudioOutputMode(bool* bRet,
                                              FileSourceConfigItemEnum Enum);

  virtual PARSER_ERRORTYPE getNextMediaSample(uint32 ulTrackId,
                                              uint8 *pucDataBuf,
                                              uint32 *pulBufSize,
                                              uint32 &rulIndex);
  virtual PARSER_ERRORTYPE peekCurSample(uint32 trackid,
                                         file_sample_info_type *pSampleInfo);

  virtual PARSER_ERRORTYPE getTrackDecoderSpecificInfoContent(uint32 id,
                                                             uint8* buf,
                                                             uint32 *pbufSize);

  virtual uint64 resetPlayback(uint64 pos, uint32 id,
                               bool bSetToSyncSample, bool *bError,
                               uint64 currentPosTimeStamp);
  virtual uint8 randomAccessDenied();

  virtual void SetCriticalSection(MM_HANDLE);

};

#endif  /* __AACFile_H__ */

