#ifndef __DTSFile_H__
#define __DTSFile_H__
/* =======================================================================
                              dtsfile.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                                    Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/DTSParserLib/main/latest/inc/dtsfile.h#6 $
$DateTime: 2014/01/29 06:47:28 $
$Change: 5183846 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "oscl_file_io.h"
#include "filebase.h"

#ifdef FEATURE_FILESOURCE_DTS
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
#define AUDIO_DTS_MAX_TRACKS      1

/* From the section 2.4.1 (Bitstream Header) in Release Version 3.22 FrameSize
   value will vary from 95 and 16383. */
#define AUDIO_DTS_MAX_FRAME_SIZE  16383
#define AUDIO_DTS_MIN_FRAME_SIZE    95

// Amount of data to read from input file.
// This data is to validate whether bitstream is DTS compliant or not
#define DTS_AUDIO_BUF_SIZE (64 * 1024)
//Max we give 19K this is from the standard parser
#define DTS_DEFAULT_AUDIO_BUF_SIZE (19 * 1024)
// Read ahead two frames
#define FILE_READ_BUFFER_SIZE_FOR_DTS 19 * 1024 * 2
// Need 8 bytes to parse and get the required information
#define DTS_HEADER_SIZE 9
// Number of frames that will be given as output in single instance
#define DTS_NUM_FRAMES 10

/* Minimum number of blocks available per frame */
#define AUDIO_DTS_MIN_BLOCKS_PER_FRAME 5

/* Number of PCM Samples per Block */
#define AUDIO_DTS_PCM_SAMPLES_PER_BLOCK 32

/* Temporary buffer size */
#define TEMP_BUF_SIZE (128)
/* Frame Header Size. This is used to do byte swap the header data in LE clips*/
#define FRAME_HEADER_SIZE (50)
/* Rev2Aux data sync marker */
static const uint8 DTS_REV2_SYNCWORD_LE[FOURCC_SIGNATURE_BYTES] =
                                            {0x70, 0xC0, 0x04, 0x70};
static const uint8 DTS_REV2_SYNCWORD[FOURCC_SIGNATURE_BYTES] =
                                            {0x70, 0x04, 0xC0, 0x70};

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
  DTSFile

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

class cDTSFile : public FileBase, public Parentable
{
public:
  uint32 FileGetData(uint64 nOffset,uint32 nNumBytesRequest,
                     uint32 nMaxSize,uint8 *ppData);

  cDTSFile(); //Default construnctor

  cDTSFile(const FILESOURCE_STRING &filename,
          unsigned char *pFileBuf, uint64 bufSize);
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  cDTSFile(video::iStreamPort*);
#endif
  virtual ~cDTSFile(); // destructor

  virtual uint64 getMediaTimestampForCurrentSample(uint32 /*id*/)
                 {return m_audsampleinfo.time;};
  virtual uint64 getMovieDuration()
                 {return m_totalDuration;};
  virtual uint32 getMovieTimescale()
                 {return MILLISEC_TIMESCALE_UNIT;};
  virtual uint64 getTrackMediaDuration(uint32 /*id*/)
                 {return m_totalDuration;};
  virtual uint32 getTrackMediaTimescale(uint32 /*id*/)
                 {return MILLISEC_TIMESCALE_UNIT;};

  virtual uint32 getTrackWholeIDList( uint32 *ids);
  virtual uint32 getTrackAudioSamplingFreq(uint32 id);
  virtual int32  getTrackMaxBufferSizeDB(uint32 id);
  virtual int32  getTrackAverageBitrate(uint32 id);
  virtual uint32 GetNumAudioChannels(int id);

  virtual PARSER_ERRORTYPE peekCurSample(uint32 trackid,
                                         file_sample_info_type *pSampleInfo);
  virtual PARSER_ERRORTYPE getNextMediaSample(uint32 ulTrackID,
                                              uint8 *pucDataBuf,
                                              uint32 *pulBufSize,
                                              uint32 &rulIndex);
  virtual uint64 resetPlayback(uint64 pos, uint32 id,
                               bool bSetToSyncSample, bool *bError,
                               uint64 currentPosTimeStamp);

  virtual  int32 getNumTracks(){return AUDIO_DTS_MAX_TRACKS;};
  virtual uint8  getTrackOTIType(uint32 /*id*/) {return (uint8)m_OTIType;};
  //DTS Parser supports seek, if BitRate info is Known.
  virtual uint8  randomAccessDenied() {return false;};
  virtual FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat)
  {
    fileFormat = FILE_SOURCE_DTS;
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
  bool            m_bIsRIFFChunk;
  bool            m_bStreaming;
  bool            m_endOfFile;
  bool            m_bIsDTSFile;
  uint64          m_nStartOffset;
  uint64          m_nOffset;
  uint64          m_totalDuration;
  uint32          m_nOutputFrames;
  uint8           m_nFrameSyncMarker[FOURCC_SIGNATURE_BYTES];
  OTI_VALUES      m_OTIType;
  OSCL_FILE*      m_DTSFilePtr;
  FILESOURCE_STRING        m_filename;
  file_sample_info_type    m_audsampleinfo;
  FS_AUDIO_PARAM_DTSTYPE   m_SyncFrameInfo;
  FileSourceConfigItemEnum m_hFrameOutputModeEnum;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  video::iStreamPort* m_pPort;
#endif
  PARSER_ERRORTYPE ParseRev2AuxData(uint8* pucFrameBuff);
  PARSER_ERRORTYPE ParseDTSHeader();
  PARSER_ERRORTYPE ParseFrameHeaderDTS(uint8* pucFrameBuff);
  PARSER_ERRORTYPE ParseFrameHeaderDTSLBR(uint8* pucFrameBuff);
  PARSER_ERRORTYPE ParseFrameHeaderDTSSubStream(uint8* pucFrameBuff);
  PARSER_ERRORTYPE ParseFrameHeaderDTSPCM(uint8* pucFrameBuff);
  void   Convert14bitBufTo16bitBuf(uint8 *pDest, const uint8 *pSrc,
                                   uint32 ulBufLength, bool bIsLittleEndian);
  void   InitData();

};

#endif /* FEATURE_FILESOURCE_DTS */

#endif  /* __DTSFile_H__ */

