#ifndef __AC3File_H__
#define __AC3File_H__
/* =======================================================================
                              ac3file.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2009-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AC3ParserLib/main/latest/inc/ac3file.h#31 $
$DateTime: 2014/03/03 22:26:49 $
$Change: 5382439 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "oscl_file_io.h"
#include "filebase.h"

#ifdef FEATURE_FILESOURCE_AC3
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
/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */
#define AUDIO_AC3_MAX_TRACKS      1
// This is from the spec
#define AUDIO_AC3_MAX_FRAME_SIZE  1920
// Amount of data to read from input file.
// This data is to validate whether bitstream is AC3 compliant or not
#define AC3_AUDIO_BUF_SIZE 8192
//Max we give 19K this is from the standard parser
#define AC3_DEFAULT_AUDIO_BUF_SIZE 19 * 1024
// Read ahead two frames
#define FILE_READ_BUFFER_SIZE_FOR_AC3 19 * 1024 * 2
// Need 8 bytes to parse and get the required information
#define AC3_HEADER_SIZE 8
// Number of frames that will be given as output in single instance
#define AC3_NUM_FRAMES 10

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
  AC3File

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

class AC3File : public FileBase, public Parentable
{
public:
  uint32 FileGetData(uint64 nOffset,uint32 nNumBytesRequest,
                     uint32 nMaxSize,uint8 *ppData);

  AC3File(); //Default construnctor

  AC3File(const FILESOURCE_STRING &filename,
          unsigned char *pFileBuf, uint64 bufSize);
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  AC3File(video::iStreamPort*);
#endif
  virtual ~AC3File(); // destructor

  virtual PARSER_ERRORTYPE getNextMediaSample(uint32 ulTrackID,
                                              uint8 *pucDataBuf,
                                              uint32 *pulBufSize,
                                              uint32 &rulIndex);
  virtual  uint64 getMediaTimestampForCurrentSample(uint32 id) ;
  virtual uint64 getMovieDuration() const;
  virtual uint32 getMovieTimescale() const;
  virtual uint64 getTrackMediaDuration(uint32 id);
  virtual uint32 getTrackMediaTimescale(uint32 id);

  virtual uint32 getTrackWholeIDList( uint32 *ids);
  virtual uint32 getTrackAudioSamplingFreq(uint32 id);
  virtual uint32 GetNumAudioChannels(int id);
  virtual int32  getTrackMaxBufferSizeDB(uint32 id);
  virtual int32  getTrackAverageBitrate(uint32 id);
  virtual uint64 resetPlayback(uint64 pos, uint32 id,
                               bool bSetToSyncSample, bool *bError,
                               uint64 currentPosTimeStamp);
  virtual uint8 randomAccessDenied() {return 0;}
  virtual PARSER_ERRORTYPE peekCurSample(uint32 trackid,
                                         file_sample_info_type *pSampleInfo);
  virtual int32 getNumTracks(){return AUDIO_AC3_MAX_TRACKS;};
  virtual uint8  getTrackOTIType(uint32 /*id*/) {return (uint8)m_OTIType;};
  virtual bool isAC3File(void) {return m_bIsAC3File;};
  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat)
  {
    fileFormat = FILE_SOURCE_AC3;
    return FILE_SOURCE_SUCCESS;
  }
  virtual uint64 GetFileSize() {return m_fileSize;};
  virtual void   SetCriticalSection(MM_HANDLE);
   /*!
  @brief      Get Audio/Video stream parameter

  @details    This function is used to get Audio/Video stream parameter i.e.
              codec configuration, profile, level from specific parser.

  @param[in]  ulTrackId           TrackID of media
  @param[in]  ulParamIndex        Parameter Index of the structure to be
                                  filled.It is from the FS_MEDIA_INDEXTYPE
                                  enumeration.
  @param[in]  pParameterStructure Pointer to client allocated structure to
                                  be filled by the underlying parser.

  @return     PARSER_ErrorNone in case of success otherwise Error.
  @note
  */
  virtual PARSER_ERRORTYPE GetStreamParameter(uint32 ulTrackId,
                                             uint32 ulParamIndex,
                                             void* pParameterStructure);
  virtual FileSourceStatus SetAudioOutputMode(FileSourceConfigItemEnum Enum);
  virtual FileSourceStatus GetAudioOutputMode(bool *bret,
                                              FileSourceConfigItemEnum Enum);

private:
  unsigned char*  m_pFileBuf;
  uint64          m_FileBufSize;
  uint64          m_fileSize;
  bool            m_bIsBigEndian;
  bool            m_bStreaming;
  bool            m_endOfFile;
  bool            m_bIsAC3File;
  bool            m_bHdrParsingDone;
  uint64          m_nStartOffset;
  uint64          m_nOffset;
  uint64          m_totalDuration;
  uint32          m_nOutputFrames;
  OTI_VALUES      m_OTIType;
  OSCL_FILE*      m_AC3FilePtr;
  FILESOURCE_STRING        m_filename;
  file_sample_info_type    m_audsampleinfo;
  FS_AUDIO_PARAM_AC3TYPE   m_SyncFrameInfo;
  FileSourceConfigItemEnum m_hFrameOutputModeEnum;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  video::iStreamPort* m_pPort;
#endif
  PARSER_ERRORTYPE ParseAC3Header();
  PARSER_ERRORTYPE ParseFrameHeaderAC3(uint8* pucFrameBuff, uint8 ucOffset);
  PARSER_ERRORTYPE ParseFrameHeaderEAC3(uint8* pucFrameBuff,uint8 ucOffset);
  void   InitData();
  uint32 getFrameSize(uint8 *pucFrameBuff);
  uint16 GetExtraChannelCount(uint16 usCustChannelMap);

};

#endif /* FEATURE_FILESOURCE_AC3 */

#endif  /* __AC3File_H__ */

