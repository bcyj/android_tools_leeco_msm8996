/* =======================================================================
                         dtsfile.cpp
DESCRIPTION
Meaningful description of the definitions contained in this file.
Description must specify if the module is portable specific, mobile
specific, or common to both, and it should alert the reader if the
module contains any conditional definitions which tailors the module to
different targets.  Include any initialization and synchronizing
requirements.

EXTERNALIZED FUNCTIONS
List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
Detail how to initialize and use this service.  The sequencing aspect
is only needed if the order of operations is important.

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential..
========================================================================== */

/* =======================================================================
                                Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/DTSParserLib/main/latest/src/dtsfile.cpp#12 $
$DateTime: 2014/03/03 22:26:49 $
$Change: 5382439 $

========================================================================== */

/* ==========================================================================

INCLUDE FILES FOR MODULE

========================================================================== */

/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "filebase.h"
#include "dtsfile.h"

#define CHAR_SIZE 8

/* CRC16 predefined Values. This table will help in reducing additional
   overhead in calculating these fields. */
uint16 usCRC16Table[16] = {
  0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
  0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400 };

/*===========================================================================

FUNCTION
  Function to calculate CRC16 value.
DESCRIPTION
  This function is used to calculate CRC-16 value.

DEPENDENCIES
  None

INPUT PARAMETERS:
  pucData: Buffer pointer
  usLen  : Input Buffer Len

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
uint16 CalculateCRC16(uint8 *pucData, uint16 usLen)
{
  uint16 usData   = 0;
  uint16 usCRCVal = 0xFFFF;

  //Validate input params
  if((0 == usLen) || (NULL == pucData))
    return uint16(~usCRCVal);

  do
  {
    /* compute checksum of lower four bits */
    usData = usCRC16Table[usCRCVal & 0xF];
    usCRCVal = (usCRCVal >> 4) & 0x0FFF;
    usCRCVal = usCRCVal ^ usData ^ usCRC16Table[*pucData & 0xF];

    /* now compute checksum of upper four bits */
    usData = usCRC16Table[usCRCVal & 0xF];
    usCRCVal = (usCRCVal >> 4) & 0x0FFF;
    usCRCVal = usCRCVal ^ usData ^ usCRC16Table[(*pucData >> 4) & 0xF];

    /* next... */
    pucData++;
  } while (--usLen);

  return (usCRCVal);
}

#ifdef FEATURE_FILESOURCE_DTS

/* ==========================================================================

DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                            Function Definitions
** ======================================================================= */
/* ======================================================================== */
/* <EJECT> */
/*===========================================================================*/

/*
    DTS-bit stream

    // sync info syntax (Release Version 3.22)
     *************************
     * DTS [section 2.3]*
     *************************
    |-------------|-------|-------|
    |syntax       | size  |offset |
    |             | (bits)|(bits) |
    |-------------|-------|-------|
    |syncinfo()   |       |       |
    |{            |       |       |
    |  syncword;  |  32   |  0    |
    |  frametype  |   1   |  32   |
    |  deficit    |   5   |  33   |
       sample
       count
    |  crc flag   |   1   |  38   |
    |  pcm blocks |   7   |  39   |
    |  frame size |   14  |  46   |
    |  channels   |   6   |  60   |
    |  samp freq  |   4   |  64   |
    |  bit rate   |   5   |  68   |
    |}            |       |       |
    |-------------|-------|-------|
 */

/* ======================================================================
FUNCTION:
  cDTSFile::FileGetData

DESCRIPTION:
  To read the data from the file
INPUT/OUTPUT PARAMETERS:
  nOffset         : Offset from which data is being requested
  nNumBytesRequest: Total number of bytes requested
  ppData          : Buffer to be used for reading the data

RETURN VALUE:
 Number of bytes read

SIDE EFFECTS:
  None.
======================================================================*/
uint32 cDTSFile::FileGetData( uint64 nOffset,
                              uint32 nNumBytesRequest,
                              uint32 nMaxSize,
                              uint8 *pData  )
{
  uint32 nRead = 0;
  if (m_DTSFilePtr != NULL)
  {
    if(!m_bMediaAbort)
    {
      nRead = FileBase::readFile(m_DTSFilePtr, pData, nOffset,
                                 FILESOURCE_MIN(nNumBytesRequest,nMaxSize));
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Breaking, user Abort is true.");
    }
  }
  return nRead;
}

/*===========================================================================

FUNCTION
  cDTSFile::DTSFile

DESCRIPTION
  Default constructer

DEPENDENCIES
  None

INPUT PARAMETERS:


RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
cDTSFile::cDTSFile()
{
  InitData();
}

/*===========================================================================

FUNCTION
  cDTSFile::DTSFile

DESCRIPTION
  creates instnace of DTSFile instance

DEPENDENCIES
  None

INPUT PARAMETERS:


RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
cDTSFile::cDTSFile(const FILESOURCE_STRING &filename,
                  unsigned char *pFileBuf, uint64 bufSize)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "cDTSFile:: DTSFile");
  InitData();
  if (pFileBuf && bufSize)
  {
    m_pFileBuf = pFileBuf;
    m_FileBufSize = bufSize;
    m_DTSFilePtr = OSCL_FileOpen (pFileBuf, bufSize);
  }
  else
  {
     m_filename = filename;
#ifdef FEATURE_FILESOURCE_FILE_IO_CACHING
     /* Calling with 10K cache  buffer size */
     m_DTSFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"),
                                  FILE_READ_BUFFER_SIZE_FOR_DTS );
#else
     m_DTSFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"));
#endif
     m_fileSize = OSCL_FileSize( m_filename );
  }
  if(m_DTSFilePtr != NULL)
  {
    if(PARSER_ErrorNone == ParseDTSHeader())
    {
      _fileErrorCode = PARSER_ErrorNone;
      _success = true;
    }
  }
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
cDTSFile::cDTSFile(video::iStreamPort* pport)
{
  InitData();
  m_pPort = pport;
  m_DTSFilePtr = OSCL_FileOpen(pport);
  if(m_DTSFilePtr != NULL)
  {
    if(m_pPort)
    {
      int64 noffset = 0;
      if(video::iStreamPort::DS_SUCCESS == m_pPort->GetContentLength(&noffset))
      {
        m_fileSize = (uint64)noffset;
      }
    }
    if(m_DTSFilePtr != NULL)
    {
      if(PARSER_ErrorNone == ParseDTSHeader())
      {
        _fileErrorCode = PARSER_ErrorNone;
        _success = true;
      }
    }
  }
}
#endif

/*===========================================================================

FUNCTION
  cDTSFile::InitData

DESCRIPTION
  Initialize class variables

DEPENDENCIES
  None

INPUT PARAMETERS:
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
void cDTSFile::InitData()
{
  m_pFileBuf      = NULL;
  m_filename      = NULL;
  m_DTSFilePtr    = NULL;
  m_FileBufSize   = 0;
  m_fileSize      = 0;
  m_nOffset       = 0;
  m_nStartOffset  = 0;
  m_totalDuration = 0;
  m_bMediaAbort   = false;
  m_bStreaming    = false;
  m_endOfFile     = false;
  m_bIsDTSFile    = false;
  m_bIsRIFFChunk  = false;
  _success        = false;
  m_bIsBigEndian  = true; //By default DTS clips are encoded in BE format.
  _fileErrorCode  = PARSER_ErrorDefault;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  m_pPort         = NULL;
#endif
  m_OTIType       = DTS_AUDIO;
  memset(&m_audsampleinfo,0,sizeof(m_audsampleinfo));
  memset(&m_SyncFrameInfo, 0, sizeof(FS_AUDIO_PARAM_DTSTYPE));

  /* By default, Parser will provide DTS_NUM_FRAMES frames in single
     getNextMediaSample call */
  m_hFrameOutputModeEnum = FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM;
  m_nOutputFrames        = DTS_NUM_FRAMES;
}

/*===========================================================================

FUNCTION
  cDTSFile::SetCriticalSection

DESCRIPTION
  Update critical Section

DEPENDENCIES
  None

INPUT PARAMETERS:
pcriticalsection

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
void cDTSFile::SetCriticalSection(MM_HANDLE pcriticalsection)
{
  if(m_DTSFilePtr)
  {
    m_DTSFilePtr->pCriticalSection = pcriticalsection;
  }
}

/*===========================================================================

FUNCTION
  cDTSFile::~DTSFile

DESCRIPTION
  Destructor

DEPENDENCIES
  None

INPUT PARAMETERS:


RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/

cDTSFile::~cDTSFile()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "cDTSFile::~DTSFile");
  if(m_DTSFilePtr!=NULL)
  {
     OSCL_FileClose(m_DTSFilePtr);
     m_DTSFilePtr = NULL;
  }
}

/*===========================================================================

FUNCTION
  cDTSFile::ParseDTSHeader

DESCRIPTION
  Parse DTS header and validate whether two consecutive frames are DTS type
  or not.

DEPENDENCIES
  None

INPUT PARAMETERS:


RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
PARSER_ERRORTYPE cDTSFile::ParseDTSHeader()
{
  PARSER_ERRORTYPE eRetError = PARSER_ErrorDefault;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "cDTSFile::ParseDTSHeader");
  uint32 ulReadDataSize  = 0;
  uint8 *pucHeaderBuf = (uint8*)MM_Malloc(DTS_AUDIO_BUF_SIZE);

  if ( NULL != pucHeaderBuf )
  {
    uint32 nCount = 0;
    ulReadDataSize = FileGetData(m_nOffset, DTS_AUDIO_BUF_SIZE,
                                 DTS_AUDIO_BUF_SIZE, pucHeaderBuf );
    if(DTS_HEADER_SIZE > ulReadDataSize)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
        "cDTSFile::ParseDTSHeader read failed %d", DTS_HEADER_SIZE);
      eRetError = PARSER_ErrorReadFail;
    }

    /* Check whether min DTS_HEADER_SIZE worth of data is available or not. */
    while(nCount < ulReadDataSize - DTS_HEADER_SIZE)
    {
      bool bIsSubStream = false;
      eRetError = PARSER_ErrorNone;

      // Update Codec as DTS
      m_OTIType = DTS_AUDIO;

      //Update Subtype based on Frame Sync Marker
      if( !memcmp(pucHeaderBuf + nCount, (void*)DTS_SYNCWORD_CORE,
                  FOURCC_SIGNATURE_BYTES) )
      {
        m_SyncFrameInfo.eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_LEGACY;
        memcpy(m_nFrameSyncMarker, DTS_SYNCWORD_CORE,
               FOURCC_SIGNATURE_BYTES);
      }
      else if( !memcmp(pucHeaderBuf + nCount, (void*)DTS_SYNCWORD_CORE_LE,
                       FOURCC_SIGNATURE_BYTES) )
      {
        m_bIsBigEndian = false;
        m_SyncFrameInfo.eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_LEGACY;
        memcpy(m_nFrameSyncMarker, DTS_SYNCWORD_CORE_LE,
               FOURCC_SIGNATURE_BYTES);
      }
      else if( !memcmp(pucHeaderBuf + nCount, (void*)DTS_SYNCWORD_XCH,
                       FOURCC_SIGNATURE_BYTES) )
      {
        m_SyncFrameInfo.eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_XCH;
        memcpy(m_nFrameSyncMarker, DTS_SYNCWORD_XCH,
               FOURCC_SIGNATURE_BYTES);
      }
      else if( !memcmp(pucHeaderBuf + nCount, (void*)DTS_SYNCWORD_XXCH,
                       FOURCC_SIGNATURE_BYTES) )
      {
        m_SyncFrameInfo.eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_XXCH;
        memcpy(m_nFrameSyncMarker, DTS_SYNCWORD_XXCH,
               FOURCC_SIGNATURE_BYTES);
      }
      else if( !memcmp(pucHeaderBuf + nCount, (void*)DTS_SYNCWORD_X96K,
                       FOURCC_SIGNATURE_BYTES) )
      {
        m_SyncFrameInfo.eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_X96K;
        memcpy(m_nFrameSyncMarker, DTS_SYNCWORD_X96K,
               FOURCC_SIGNATURE_BYTES);
      }
      else if( !memcmp(pucHeaderBuf + nCount, (void*)DTS_SYNCWORD_XBR,
                       FOURCC_SIGNATURE_BYTES) )
      {
        m_SyncFrameInfo.eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_XBR;
        memcpy(m_nFrameSyncMarker, DTS_SYNCWORD_XBR,
               FOURCC_SIGNATURE_BYTES);
      }
      else if( !memcmp(pucHeaderBuf + nCount, (void*)DTS_SYNCWORD_LBR,
                       FOURCC_SIGNATURE_BYTES) )
      {
        m_SyncFrameInfo.eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_LBR;
        memcpy(m_nFrameSyncMarker, DTS_SYNCWORD_LBR,
               FOURCC_SIGNATURE_BYTES);
      }
      else if( !memcmp(pucHeaderBuf + nCount, (void*)DTS_SYNCWORD_LBR_LE,
                       FOURCC_SIGNATURE_BYTES) )
      {
        m_bIsBigEndian = false;
        m_SyncFrameInfo.eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_LBR;
        memcpy(m_nFrameSyncMarker, DTS_SYNCWORD_LBR_LE,
               FOURCC_SIGNATURE_BYTES);
      }
      else if( !memcmp(pucHeaderBuf + nCount, (void*)DTS_SYNCWORD_XLL,
                       FOURCC_SIGNATURE_BYTES) )
      {
        m_SyncFrameInfo.eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_XLL;
        memcpy(m_nFrameSyncMarker, DTS_SYNCWORD_XLL,
               FOURCC_SIGNATURE_BYTES);
      }
      else if( !memcmp(pucHeaderBuf + nCount, (void*)DTS_SYNCWORD_SUBSTREAM,
                       FOURCC_SIGNATURE_BYTES) )
      {
        bIsSubStream = true;;
        memcpy(m_nFrameSyncMarker, DTS_SYNCWORD_SUBSTREAM,
               FOURCC_SIGNATURE_BYTES);
      }
      else if( !memcmp(pucHeaderBuf + nCount, (void*)DTS_SYNCWORD_SUBSTREAM_LE,
                       FOURCC_SIGNATURE_BYTES) )
      {
        m_bIsBigEndian = false;
        bIsSubStream = true;;
        memcpy(m_nFrameSyncMarker, DTS_SYNCWORD_SUBSTREAM_LE,
               FOURCC_SIGNATURE_BYTES);
      }
      else if( !memcmp(pucHeaderBuf + nCount,
                       DTS_SYNCWORD_SUBSTREAM_CORE2, FOURCC_SIGNATURE_BYTES) )
      {
        m_SyncFrameInfo.eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_SUBSTREAM_CORE2;
        memcpy(m_nFrameSyncMarker, DTS_SYNCWORD_SUBSTREAM_CORE2,
               FOURCC_SIGNATURE_BYTES);
      }
      else if(!memcmp(pucHeaderBuf + nCount, "RIFF", FOURCC_SIGNATURE_BYTES) )
      {
        uint32 ulDataSize = 0;
        uint32 tempCount = 0;
        m_nStartOffset = 44;
        nCount = (uint32)m_nStartOffset - 1;
        m_bIsRIFFChunk = true;
        while(tempCount < 4)
        {
          ulDataSize <<= CHAR_SIZE;
          ulDataSize += pucHeaderBuf[nCount - tempCount++];
        }
        nCount++;
        /* In some WAV files other than DATA chunk may present at the end of
           file. DTS decoder will not need that additional information. Update
           file-size to the end of data chunk.
        */
        if(m_fileSize > ulDataSize + m_nStartOffset)
        {
          m_fileSize = ulDataSize + m_nStartOffset;
        }
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseDTSHeader RIFF chunk found, data chunk starts at %llu",
                  m_nStartOffset);
        continue;
      }
      else if(!memcmp(pucHeaderBuf + nCount, DTS_SYNCWORD_PCM,
                      FOURCC_SIGNATURE_BYTES) )
      {
        m_SyncFrameInfo.eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_PCM;
        memcpy(m_nFrameSyncMarker, DTS_SYNCWORD_PCM,
               FOURCC_SIGNATURE_BYTES);
      }
      else if(!memcmp(pucHeaderBuf + nCount, DTS_SYNCWORD_PCM_LE,
                      FOURCC_SIGNATURE_BYTES) )
      {
        m_SyncFrameInfo.eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_PCM;
        memcpy(m_nFrameSyncMarker, DTS_SYNCWORD_PCM_LE,
               FOURCC_SIGNATURE_BYTES);
        m_bIsBigEndian = false;
      }
      else
      {
        eRetError = PARSER_ErrorStreamCorrupt;
      }

      if(PARSER_ErrorNone == eRetError)
      {
        if(FILE_SOURCE_SUB_MN_TYPE_DTS_LBR == m_SyncFrameInfo.eSubType)
        {
          eRetError = ParseFrameHeaderDTSLBR(pucHeaderBuf + nCount);
        }
        else if(true == bIsSubStream)
        {
          eRetError = ParseFrameHeaderDTSSubStream(pucHeaderBuf + nCount);
        }
        else if(FILE_SOURCE_SUB_MN_TYPE_DTS_PCM == m_SyncFrameInfo.eSubType)
        {
          eRetError = ParseFrameHeaderDTSPCM(pucHeaderBuf + nCount);
        }
        else
        {
          eRetError = ParseFrameHeaderDTS(pucHeaderBuf + nCount);
        }
      }

      /* If input stream is validated as DTS bitstream, update class
         variables. */
      if (PARSER_ErrorNone == eRetError)
      {
        m_nStartOffset = m_nOffset= nCount;
        m_bIsDTSFile   = true;
        if((nCount + m_SyncFrameInfo.usFrameSize) > ulReadDataSize )
        {
          ulReadDataSize = FileGetData(m_nOffset, m_SyncFrameInfo.usFrameSize,
                                       DTS_AUDIO_BUF_SIZE, pucHeaderBuf );
          if(m_SyncFrameInfo.usFrameSize > ulReadDataSize)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
              "cDTSFile::ParseDTSHeader read failed %lu", ulReadDataSize);
            eRetError = PARSER_ErrorReadFail;
          }
          nCount = 0;
        }
        if (PARSER_ErrorNone == eRetError)
        {
          (void)ParseRev2AuxData(pucHeaderBuf + nCount);
        }
        break;
      }
      nCount += FOURCC_SIGNATURE_BYTES;
    }
  }
  else
  {
    eRetError = PARSER_ErrorMemAllocFail;
  }// if-else(pucHeaderBuf)
  //free HeaderBuf once done with initial header parsing.
  if (pucHeaderBuf)
  {
    MM_Free(pucHeaderBuf);
    pucHeaderBuf = NULL;
  }
  return eRetError;
}

/*===========================================================================

FUNCTION
  cDTSFile::ParseFrameHeaderDTS

DESCRIPTION
  Parse DTS frame header and update class variable accordingly.

DEPENDENCIES
  None

INPUT PARAMETERS:
  pucFrameBuff: Input Data buffer

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
PARSER_ERRORTYPE cDTSFile::ParseFrameHeaderDTS(uint8* pucFrameBuff)
{
  //Skip Frame Sync marker worth of data
  uint32 ulCurBitOffset = FOURCC_SIGNATURE_BYTES * 8;
  PARSER_ERRORTYPE eRetError = PARSER_ErrorDefault;

  //! Do byte swap if the bit-stream is in little endian format.
  //! This is done to extra DTS frame properties.
  if(false == m_bIsBigEndian)
  {
    BYTESWAP(pucFrameBuff, FRAME_HEADER_SIZE);
  }
  /* Syntax for a frame of DTS legacy is mentioned at the top of the file.*/

  if ( NULL != pucFrameBuff)
  {
    uint8 ucFlag = 0;

    /* Skip 6bits:
       Frame Type          : 1bit
       Deficit Sample Count: 5bits*/
    ulCurBitOffset += 6;

    //1 bit is required for CRC flag
    uint8 ucCRCflag = (uint8)
       GetBitsFromBuffer(1, ulCurBitOffset, pucFrameBuff, DTS_AUDIO_BUF_SIZE);
    ulCurBitOffset += 1;

    // 7 bits are for number of blocks info
    uint8 ucNumBlks = (uint8)
       GetBitsFromBuffer(7, ulCurBitOffset, pucFrameBuff, DTS_AUDIO_BUF_SIZE);
    ucNumBlks++;
    ulCurBitOffset += 7;
    m_SyncFrameInfo.usSamplesperFrame =uint16(ucNumBlks *
                                              AUDIO_DTS_PCM_SAMPLES_PER_BLOCK);

    //14bits are used to store frame Size info. Minimum Frame Size value is 95.
    //As per spec, Frame-size value stored is 1byte less than actual value.
    m_SyncFrameInfo.usFrameSize = (uint16)
       GetBitsFromBuffer(14, ulCurBitOffset, pucFrameBuff, DTS_AUDIO_BUF_SIZE);
    m_SyncFrameInfo.usFrameSize++;
    ulCurBitOffset += 14;

    //6bits are used to store channel configuration.
    m_SyncFrameInfo.ucAudioCodingMode = (uint8)
       GetBitsFromBuffer(6, ulCurBitOffset, pucFrameBuff, DTS_AUDIO_BUF_SIZE);
    ulCurBitOffset += 6;

    //4bits are used to store sampling frequency.
    uint8 SFIndex = (uint8)GetBitsFromBuffer(4, ulCurBitOffset, pucFrameBuff,
                                             DTS_AUDIO_BUF_SIZE);
    m_SyncFrameInfo.ulSamplingRate = DTS_FSCODE_RATE[SFIndex];
    ulCurBitOffset += 4;

    //5bits are used to store bit rate info.
    uint8 BRIndex = (uint8)GetBitsFromBuffer(5, ulCurBitOffset, pucFrameBuff,
                                             DTS_AUDIO_BUF_SIZE);
    m_SyncFrameInfo.ulBitRate = DTS_BIT_RATE[BRIndex];
    ulCurBitOffset += 5;

    //1bit : Fixed bit, This info is not required
    ulCurBitOffset += 1;

    //1bit  used to store embedded dynamic range flag.
    ucFlag = (uint8)
       GetBitsFromBuffer(1, ulCurBitOffset, pucFrameBuff, DTS_AUDIO_BUF_SIZE);
    ulCurBitOffset += 1;
    if (ucFlag)
    {
      m_SyncFrameInfo.bDynamicRangeFlag = true;
    }

    /* Skip remaining bits not required as of now.
       Embedded Time Stamp Flag         -- 1 bit
       Auxiliary Data Flag              -- 1 bit
       HDCD                             -- 1 bit
       Extension Audio Descriptor Flag  -- 3 bit
       Extended Coding Flag             -- 1 bit
       Audio Sync Word Insertion Flag   -- 1 bit
       Low Frequency Effects Flag       -- 1 bit
       Predictor History Flag Switch    -- 2 bit
    */
    ulCurBitOffset += 11;

    //Check whether CRC flag is set or not. If it is set skip another two bytes
    if (ucCRCflag)
    {
      ulCurBitOffset += 16;
    }

    /* Skip Multirate Interpolator Switch which is 1bit. */
    ulCurBitOffset += 1;

    /* Vernum version flag. Curretly decoder supports only version#6 and 7 */
    uint8 ucVernumFlag = (uint8)
      GetBitsFromBuffer(4, ulCurBitOffset, pucFrameBuff, DTS_AUDIO_BUF_SIZE);
    ulCurBitOffset += 4;
    if ((ucVernumFlag == 6) || (ucVernumFlag == 7))
    {
      /* Skip remaining bits not required as of now.
         Copy History                     -- 2 bits
         Source PCM Resolution            -- 3 bits
         Front Sum/Difference Flag        -- 1 bit
         Surrounds Sum/Difference Flag    -- 1 bit
      */
      ulCurBitOffset += 7;

      /* Dialog Normalization Gain Parameter:
         If this value is nonzero, then no DNG is required.
         Currently parser need to indicate whether DNG is required or not. */
      ucFlag = (uint8)
        GetBitsFromBuffer(4, ulCurBitOffset, pucFrameBuff, DTS_AUDIO_BUF_SIZE);
      ulCurBitOffset += 4;
      if (ucFlag)
      {
        m_SyncFrameInfo.bDialnormMetadata = true;
      }
    }

    /* Update frame-rate, frame-size & sample-rate */
    if ( (m_SyncFrameInfo.ulSamplingRate == 0) ||
         (m_SyncFrameInfo.usFrameSize < AUDIO_DTS_MIN_FRAME_SIZE) ||
         (ucNumBlks < AUDIO_DTS_MIN_BLOCKS_PER_FRAME))
    {
      m_SyncFrameInfo.ulBitRate = 0;
      m_SyncFrameInfo.ulSamplingRate = 0;
      m_SyncFrameInfo.usFrameSize = 0;
      eRetError = PARSER_ErrorStreamCorrupt;
    }
    else
    {
      //Update number of channels information.
      if(MAX_DTS_CHANNELS_INDEX > m_SyncFrameInfo.ucAudioCodingMode)
        m_SyncFrameInfo.usNumChannels =
          DTS_CHANNELS[m_SyncFrameInfo.ucAudioCodingMode];

      // Update clip duration
      if ( m_SyncFrameInfo.ulBitRate )
      {
        m_totalDuration =(m_fileSize * 8 * MILLISEC_TIMESCALE_UNIT)/
                         m_SyncFrameInfo.ulBitRate;
      }
      eRetError = PARSER_ErrorNone;
      MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
        "ParseFrameHeaderDTS: SamlingFreq %lu, channels %u,  BitRate %lu",
          m_SyncFrameInfo.ulSamplingRate, m_SyncFrameInfo.usNumChannels,
          m_SyncFrameInfo.ulBitRate);
    }//(validate frameSize and sampleRate)
  }
  else
  {
    eRetError = PARSER_ErrorInvalidParam;
  }
  return (eRetError);
}

/*===========================================================================

FUNCTION
  cDTSFile::ParseFrameHeaderDTSSubStream

DESCRIPTION
  Parse DTS frame header and update class variable accordingly.

DEPENDENCIES
  None

INPUT PARAMETERS:
  pucFrameBuff: Input Data buffer

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
PARSER_ERRORTYPE cDTSFile::ParseFrameHeaderDTSSubStream(uint8* pucFrameBuff)
{
  //Skip Frame Sync marker worth of data
  uint32 ulCurrOffset = FOURCC_SIGNATURE_BYTES * 8;
  PARSER_ERRORTYPE eRetError = PARSER_ErrorDefault;

  /* Syntax for frame of DTS Sub-Stream is as mentioned below.

    // sync info syntax
    "DTS-HD Substream and Decoder Interface Description"
             (Release Version 3.2)
     *************************
     * DTS [section 2.7]*
     *************************
    |-------------|--------|-------|
    |syntax       | size   |offset |
    |             | (bits) |(bits) |
    |-------------|--------|-------|
    |syncinfo()   |        |       |
    |{            |        |       |
    |  syncword;  |  32    |  0    |
    |  userdefined|   8    |  32   |
    |  sampling   |   2    |  40   |
       frequency
    |  header     |   1    |  42   |
       size type
    |  substream  |   8    |  43   |
       header size  (or 12)
    |  frame size |   16   |  51   |
                    (or 20) (or 55)
    |}            |        |       |
    |-------------|--------|-------|
  */

  if(false == m_bIsBigEndian)
  {
    BYTESWAP(pucFrameBuff, 12);
  }
  if ( NULL != pucFrameBuff)
  {
    // User defined bits. Not required, skip.
    (void)GetBitsFromBuffer(8, ulCurrOffset, pucFrameBuff, DTS_AUDIO_BUF_SIZE);
    ulCurrOffset += 8;

    uint32 nuBits4Header = 0;
    uint32 nuBits4ExSSFsize  = 0;

    //2bits are used to store sampling frequency.
    (void)GetBitsFromBuffer(2, ulCurrOffset, pucFrameBuff, DTS_AUDIO_BUF_SIZE);
    ulCurrOffset += 2;

    //1bits is used to indicate number of bits used for header and frame size.
    uint8 bHeaderSizeType = (uint8)GetBitsFromBuffer(1, ulCurrOffset,
                                                     pucFrameBuff,
                                                     DTS_AUDIO_BUF_SIZE);
    ulCurrOffset += 1;
    if(0 == bHeaderSizeType)
    {
      //8bits are used to store header size.
      nuBits4Header =
        GetBitsFromBuffer(8, ulCurrOffset, pucFrameBuff, DTS_AUDIO_BUF_SIZE);
      ulCurrOffset += 8;

      /* 16 bits are used to store SubStream frame Size details. */
      nuBits4ExSSFsize = GetBitsFromBuffer(16, ulCurrOffset, pucFrameBuff,
        DTS_AUDIO_BUF_SIZE);
      ulCurrOffset += 16;
    }
    else
    {
      //8bits are used to store header size.
      nuBits4Header =
        GetBitsFromBuffer(12, ulCurrOffset, pucFrameBuff, DTS_AUDIO_BUF_SIZE);
      ulCurrOffset += 12;

      /* 16 bits are used to store SubStream frame Size details. */
      nuBits4ExSSFsize = GetBitsFromBuffer(20, ulCurrOffset, pucFrameBuff,
        DTS_AUDIO_BUF_SIZE);
      ulCurrOffset += 20;
    }
    nuBits4Header += 1;
    nuBits4ExSSFsize += 1;

    if( !memcmp(pucFrameBuff + nuBits4Header, (void*)DTS_SYNCWORD_CORE,
                 FOURCC_SIGNATURE_BYTES) )
    {
      m_SyncFrameInfo.eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_LEGACY;
      eRetError = ParseFrameHeaderDTS(pucFrameBuff + nuBits4Header);
    }
    else if( !memcmp(pucFrameBuff + nuBits4Header, (void*)DTS_SYNCWORD_LBR,
                     FOURCC_SIGNATURE_BYTES) )
    {
      m_SyncFrameInfo.eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_LBR;
      eRetError = ParseFrameHeaderDTSLBR(pucFrameBuff + nuBits4Header);
    }
    //Update Framesize value
    m_SyncFrameInfo.usFrameSize = (uint16)nuBits4ExSSFsize;
  }
  return (eRetError);
}

/*===========================================================================

FUNCTION
  cDTSFile::ParseFrameHeaderDTSLBR

DESCRIPTION
  Parse DTS frame header and update class variable accordingly.

DEPENDENCIES
  None

INPUT PARAMETERS:
  pucFrameBuff: Input Data buffer

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
PARSER_ERRORTYPE cDTSFile::ParseFrameHeaderDTSLBR(uint8* pucFrameBuff)
{
  //Skip Frame Sync marker worth of data
  uint32 ulCurrOffset = FOURCC_SIGNATURE_BYTES * 8;
  PARSER_ERRORTYPE eRetError = PARSER_ErrorDefault;
  uint8 LBRHeaderType = 0;

  /* Syntax for frame of DTS Sub-Stream is as mentioned below.

    // sync info syntax (Release Version 3.01)
     ****************************************
     * DTS Document No.: 9302F47900B        *
     * Substream LBR Audio Asset Description*
     ****************************************
    |-------------|--------|-------|
    |syntax       | size   |offset |
    |             | (bits) |(bits) |
    |-------------|--------|-------|
    |syncinfo()   |        |       |
    |{            |        |       |
    |  syncword;  |  32    |  0    |
    |  LBR header |   8    |  32   |
       type
    |  sampling   |   8    |  40   |
       frequency
    |  speaker    |   16   |  48   |
       mask
    |  LBR version|   16   |  64   |
    |  compressed |   8    |  80   |
       flags
    |  BitRate    |  8     |  88   |
       MS Nybbles
    |  Original   | 16     |  96   |
       LSW
    |  ScaledBR   | 16     | 112   |
       LSW
    |}            |        |       |
    |-------------|--------|-------|
  */

  if(false == m_bIsBigEndian)
  {
    BYTESWAP(pucFrameBuff, 16);
  }
  if ( NULL != pucFrameBuff)
  {
    //8bits are used to indicate LBR header type
    LBRHeaderType = (uint8)GetBitsFromBuffer(8, ulCurrOffset, pucFrameBuff,
                                             DTS_AUDIO_BUF_SIZE);
    ulCurrOffset += 8;
    if(2 == LBRHeaderType )
    {
      uint16 nLBRBitRateMSnybbles = 0;
      uint16 nLBROriginalBitRate  = 0;

      //8bits are used to store sampling frequency.
      uint8 SFIndex = (uint8)GetBitsFromBuffer(8, ulCurrOffset, pucFrameBuff,
                                               DTS_AUDIO_BUF_SIZE);
      m_SyncFrameInfo.ulSamplingRate = DTS_LBR_FSCODE_RATE[SFIndex];
      ulCurrOffset += 8;

      //16bits are used to store channel configuration.
      m_SyncFrameInfo.ucAudioCodingMode = (uint8)
         GetBitsFromBuffer(16, ulCurrOffset, pucFrameBuff, DTS_AUDIO_BUF_SIZE);
      ulCurrOffset += 16;

      /* 16 bits are used to store LBR Version info. It is not required
         for parser. */
      (void)GetBitsFromBuffer(16, ulCurrOffset, pucFrameBuff,
                              DTS_AUDIO_BUF_SIZE);
      ulCurrOffset += 16;

      /* 8 bits are used to store LBR Compressed Flags info. It is not required
         for parser. */
      (void)GetBitsFromBuffer(8, ulCurrOffset, pucFrameBuff,
                              DTS_AUDIO_BUF_SIZE);
      ulCurrOffset += 8;

      /* 8 bits are used to store LBR BitRate MS nybbles. */
      nLBRBitRateMSnybbles = (uint16)GetBitsFromBuffer(8, ulCurrOffset,
                                                       pucFrameBuff,
                                                       DTS_AUDIO_BUF_SIZE);
      ulCurrOffset += 8;

      /* 16 bits are used to store LBR BitRate MS nybbles. */
      nLBROriginalBitRate = (uint16)GetBitsFromBuffer(16, ulCurrOffset,
                                                      pucFrameBuff,
                                                      DTS_AUDIO_BUF_SIZE);
      ulCurrOffset += 16;

      /* 16 bits are used to store Scaled BitRate MS nybbles. */
      (void)GetBitsFromBuffer(16, ulCurrOffset, pucFrameBuff,
                              DTS_AUDIO_BUF_SIZE);
      ulCurrOffset += 16;

      m_SyncFrameInfo.usFrameSize = AUDIO_DTS_MAX_FRAME_SIZE;
      /* Update frame-rate, frame-size & sample-rate */
      if ( m_SyncFrameInfo.ulSamplingRate == 0 ||
           m_SyncFrameInfo.usFrameSize < AUDIO_DTS_MIN_FRAME_SIZE)
      {
        m_SyncFrameInfo.ulBitRate = 0;
        m_SyncFrameInfo.ulSamplingRate = 0;
        eRetError = PARSER_ErrorStreamCorrupt;
      }
      else
      {
        //Update number of channels information.
        if(MAX_DTS_CHANNELS_INDEX > m_SyncFrameInfo.ucAudioCodingMode)
          m_SyncFrameInfo.usNumChannels =
            DTS_CHANNELS[m_SyncFrameInfo.ucAudioCodingMode];

        m_SyncFrameInfo.ulBitRate = nLBROriginalBitRate |
                                    ((nLBRBitRateMSnybbles & 0x0F) << 16);
        // Update clip duration
        if ( m_SyncFrameInfo.ulBitRate )
        {
          m_totalDuration =(m_fileSize * 8 * MILLISEC_TIMESCALE_UNIT)/
                           m_SyncFrameInfo.ulBitRate;
        }
        MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
        "ParseFrameHeaderDTSLBR: SamlingFreq %lu, channels %u,  BitRate %lu",
          m_SyncFrameInfo.ulSamplingRate, m_SyncFrameInfo.usNumChannels,
          m_SyncFrameInfo.ulBitRate);
        eRetError = PARSER_ErrorNone;
      }//(validate frameSize and sampleRate)
    }
    else
    {
      eRetError = PARSER_ErrorInvalidParam;
    }
  }
  return (eRetError);
}

/*===========================================================================

FUNCTION
  cDTSFile::ParseFrameHeaderDTSPCM

DESCRIPTION
  Parse DTS PCM frame header and update class variable accordingly.

DEPENDENCIES
  None

INPUT PARAMETERS:
  pucFrameBuff: Input Data buffer

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
PARSER_ERRORTYPE cDTSFile::ParseFrameHeaderDTSPCM(uint8* pucInputBuf)
{
  //Skip Frame Sync marker worth of data
  PARSER_ERRORTYPE eRetError = PARSER_ErrorDefault;
  uint8 pucFrameBuff[128];
  bool bEndianNess = m_bIsBigEndian;

  // Function to convert 14bit data into 16bit format.
  Convert14bitBufTo16bitBuf(pucFrameBuff, pucInputBuf, 128, !m_bIsBigEndian);

  /* Reset this as TRUE. It is required as Byte swapping is already taken done
     as part 14bit to 16bit data conversion. This value will be updated
     again with correct value later. */
  m_bIsBigEndian = true;

  eRetError = ParseFrameHeaderDTS(pucFrameBuff);
  m_bIsBigEndian = bEndianNess;

  return (eRetError);
}

/* ======================================================================
FUNCTION:
  cDTSFile::getNextMediaSample

DESCRIPTION:
  gets next sample of the given track.

INPUT/OUTPUT PARAMETERS:
@param[in] ulTrackID  TrackID requested
@param[in] pucDataBuf DataBuffer pointer to fill the frame(s)
@param[in/out]
           pulBufSize Amount of data request /
                      Amount of data filled in Buffer
@param[in] rulIndex   Index

RETURN VALUE:
 PARSER_ErrorNone in Successful case /
 Corresponding error code in failure cases

SIDE EFFECTS:
  None.
======================================================================*/

PARSER_ERRORTYPE cDTSFile::getNextMediaSample(uint32 /*ulTrackID*/,
                                              uint8*  pucDataBuf,
                                              uint32* pulBufSize,
                                              uint32& /*rulIndex*/)
{
  uint32 ulOutDataSize = 0;
  uint64 ullSampleTime = 0;
  uint32 ulNumFrames   = 0;
  uint64 ulSampleDur   = 0;
  uint32 ulFrameSize   = m_SyncFrameInfo.usFrameSize;
  uint64 ullOffset     = m_nOffset;
  uint8  ucBuf[TEMP_BUF_SIZE];
  PARSER_ERRORTYPE eStatus = PARSER_ErrorDefault;

  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "cDTSFile::getNextMediaSample @ %llu", m_nOffset);

  /* Validate input params and class variables */
  if((NULL == m_DTSFilePtr) || (NULL == pulBufSize) ||
     (NULL == pucDataBuf) || (0 == *pulBufSize))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "getNextMediaSample invalid input params!!");
    return PARSER_ErrorInvalidParam;
  }
  _fileErrorCode = PARSER_ErrorNone;

  /* If Parser is configured to give data in bitStream Mode, then provide
     complete data available in the input file. Reset frameSize value so that
     parser will give data till end of the file. Update ReadDataSize param
     with buf-size. Parser reads buffer Size worth of data in one instance.*/
  if(FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM == m_hFrameOutputModeEnum)
  {
    ulFrameSize = 0;
  }

  /* Check whether at least one frame worth of data is available or not.
     This is applicable only in case of Frame mode output.
     In bit-stream mode, Parser provides the data till end of file. */
  if((m_nOffset + ulFrameSize) >= m_fileSize)
  {
    m_endOfFile = true;
    m_nOffset   = m_nStartOffset;
    *pulBufSize = 0;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"cDTSFile::EOS!!");
    _fileErrorCode = PARSER_ErrorEndOfFile;
    m_audsampleinfo.sample = 0;
    return _fileErrorCode;
  }
  if((0 == m_audsampleinfo.sample) &&
     (FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM == m_hFrameOutputModeEnum) &&
     (false == m_bIsRIFFChunk))
  {
    m_nOffset = 0;
  }

  /* Read Buf-Size worth of data if Parser is configured in bit-stream mode
     or if DTS Subtype is not Legacy. Currently Frame by frame is not supported
     in other DTS subtypes. */
  if ((FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM == m_hFrameOutputModeEnum) ||
      (FILE_SOURCE_SUB_MN_TYPE_DTS_LEGACY != m_SyncFrameInfo.eSubType))
  {
    ulOutDataSize = FileGetData(m_nOffset, *pulBufSize,
                                *pulBufSize, pucDataBuf);
    m_nOffset += ulOutDataSize;
  }
  else
  {
    uint64 ullTempOffset   = ullOffset;
    uint32 ulPaddingBytes  = 0;
    uint32 ulDataRead      = 0;
    uint32 ulIndex         = 0;
    bool   bBreakLoop      = false;

    //! Keep reading data into output buffer one frame at a time
    while((ulOutDataSize + ulFrameSize) < *pulBufSize)
    {
      uint32 ulStartIndex = ulOutDataSize;
      /* This flag will be set to true, if previous frame is not completely
        written into output buffer. Use previous sample offset to update
        the amount of data read into o/p buf. */
      if (bBreakLoop)
      {
        ullOffset = ullTempOffset;
        break;
      }
      //! Store sample start offset in local variable
      ullTempOffset = ullOffset;
      //! Read FRAME_HEADER_SIZE bytes worth of data
      //! If read operation failed, break the loop
      ulDataRead = FileGetData(ullOffset, FRAME_HEADER_SIZE,
                               FRAME_HEADER_SIZE,ucBuf);
      if (!ulDataRead)
      {
        break;
      }
      //! Parse DTS Frame header
      eStatus     = ParseFrameHeaderDTS(ucBuf);
      ulFrameSize = m_SyncFrameInfo.usFrameSize;
      ulDataRead  = FileGetData(ullOffset, ulFrameSize, *pulBufSize,
                                pucDataBuf + ulOutDataSize);
      if (ulDataRead != ulFrameSize)
      {
        break;
      }

      //! Update o/p buffer size and read offset params
      ulOutDataSize += ulDataRead;
      ullOffset     += ulDataRead;
      //! Check whether extra data is present after frame or not.
      //! Look for next sync marker, all the data till next sync marker is
      //! considered as part of current frame only.
      do
      {
        ulIndex = 0;
        ulDataRead = FileGetData(ullOffset, FRAME_HEADER_SIZE,
                                 FRAME_HEADER_SIZE, ucBuf);
        while ((ulIndex + FOURCC_SIGNATURE_BYTES) < ulDataRead)
        {
          if( !memcmp(ucBuf + ulIndex, (void*)&m_nFrameSyncMarker,
                      FOURCC_SIGNATURE_BYTES) )
          {
            break;
          }
          ulIndex++;
        }
        //! If memory is not sufficient to read data, then do not copy instead
        //! break the loop.
        if((ulOutDataSize + ulIndex) > *pulBufSize)
        {
          bBreakLoop = true;
          break;
        }
        //! Copy the data into o/p buf, till sync marker
        if(ulIndex)
        {
          memcpy(pucDataBuf + ulOutDataSize, ucBuf, ulIndex);
          ulOutDataSize += ulIndex;
          ullOffset     += ulIndex;
        }
        //! If sync marker is found within limits, then break loop
        if ((ulIndex + FOURCC_SIGNATURE_BYTES) < ulDataRead)
        {
          ulDataRead -= ulIndex;
          break;
        }
      } while(ulDataRead);

      /* Check whether Frame is at 4byte aligned or not. */
      if((!bBreakLoop) && (0 != (ulOutDataSize & 3)))
      {
        ulPaddingBytes = uint32(FOURCC_SIGNATURE_BYTES - (ulOutDataSize & 3));
        //! Add padding bytes before frame start
        //! Ensure each frame is starting at 4byte aligned boundary
        if((ulPaddingBytes + ulOutDataSize) < *pulBufSize)
        {
          memset(pucDataBuf + ulOutDataSize, 0, ulPaddingBytes);
          ulOutDataSize += ulPaddingBytes;
        }
        else
        {
          bBreakLoop = true;
        }
      }
      //! Update frame size based on index jump
      ulFrameSize = ulOutDataSize - ulStartIndex;
    }//! while(ulOutDataSize < *pulBufSize)
    //! Update current offset to read next sample in next iteration
    m_nOffset      = ullOffset;
  }

  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "cDTSFile::read offset %llu, data read %lu, sample %lu!!",
               m_nOffset, ulOutDataSize, m_audsampleinfo.sample);

  *pulBufSize  = ulOutDataSize;
  /* Calculate timestamp only if output is configured to one frame */
  if((m_SyncFrameInfo.ulBitRate) && (1 == m_nOutputFrames) && (*pulBufSize))
  {
    uint64 ullStartOffset   = m_nOffset - ulOutDataSize;
    uint64 ullSampleEndTime = 0;
    if(m_nOffset > ulOutDataSize)
    {
      ullSampleTime = (uint64)(ullStartOffset * 8 * MILLISEC_TIMESCALE_UNIT)/
                               m_SyncFrameInfo.ulBitRate;
    }
    ullSampleEndTime = (uint64)(m_nOffset * 8 * MILLISEC_TIMESCALE_UNIT)/
                                m_SyncFrameInfo.ulBitRate;
    ulSampleDur = ullSampleEndTime - ullSampleTime;
    m_audsampleinfo.btimevalid = true;
  }
  else
  {
    m_audsampleinfo.btimevalid = false;
  }
  m_audsampleinfo.sample    += 1 ;
  m_audsampleinfo.size       = ulOutDataSize;
  m_audsampleinfo.offset     = m_nOffset;
  m_audsampleinfo.time       = ullSampleTime;
  m_audsampleinfo.delta      = ulSampleDur;
  m_audsampleinfo.sync       = 1;
  m_audsampleinfo.num_frames = ulNumFrames;
  if(*pulBufSize)
  {
    return PARSER_ErrorNone;
  }
  else
  {
    return PARSER_ErrorEndOfFile;
  }
}

/* ======================================================================
FUNCTION:
  cDTSFile::resetPlayback

DESCRIPTION:
  resets the playback time to given time(pos) for a track.
  Also tells if we need to goto closest sync sample or not.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 cDTSFile::resetPlayback(uint64 repos_time, uint32 id,
                               bool /*bSetToSyncSample*/, bool* bError,
                               uint64 /*currentPosTimeStamp*/)
{
  uint64 seek_time = 0;
  uint64 coarse_Reposition = 0;
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "cDTSFile::resetPlayback, time %llu, id %lu", repos_time, id);
  if(repos_time == 0)
  {
    m_nOffset   = m_nStartOffset;
    seek_time   = 0;
    m_endOfFile = false;
    *bError = FALSE;
    _fileErrorCode = PARSER_ErrorNone;
  }
  else
  {
    /* Calculate Seek point only if BitRate info is valid and seek time is less
       than total duration value. */
    if((repos_time <= m_totalDuration) && (m_SyncFrameInfo.ulBitRate))
    {
      bool bSyncMarkerFound = false;
      uint8 *pucHeaderBuf = (uint8*)MM_Malloc(DTS_AUDIO_BUF_SIZE);
      m_nOffset   = (repos_time * m_SyncFrameInfo.ulBitRate) /
                    (8 * MILLISEC_TIMESCALE_UNIT);
      coarse_Reposition = m_nOffset % m_SyncFrameInfo.usFrameSize;
      m_nOffset   = m_nOffset - coarse_Reposition + m_nStartOffset;
      seek_time   = (m_nOffset * 8 * MILLISEC_TIMESCALE_UNIT)/
                             m_SyncFrameInfo.ulBitRate;
      m_endOfFile = false;
      *bError = FALSE;
      _fileErrorCode = PARSER_ErrorNone;

      /* If frame size is not fixed, then offset need not be at Frame Sync
         boundary. Following logic will help Parser to find frame sync marker
         in forward direction and give data from that point.
      */
      do
      {
        uint32 nCount = 0;
        uint32 ulReadDataSize = 0;
        if(pucHeaderBuf)
        {
          ulReadDataSize = FileGetData(m_nOffset,
                                       DTS_AUDIO_BUF_SIZE,
                                       DTS_DEFAULT_AUDIO_BUF_SIZE,
                                       pucHeaderBuf );
        }
        else
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                      "cDTSFile::resetPlayback malloc failed ");
          break;
        }
        if(DTS_HEADER_SIZE >= ulReadDataSize)
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
            "cDTSFile::resetPlayback read failed at offset %llu", m_nOffset);
          _fileErrorCode = PARSER_ErrorSeekFail;
          *bError        = true;
          break;
        }
        while((nCount < ulReadDataSize) &&
              (DTS_HEADER_SIZE < (ulReadDataSize - nCount)))
        {
          if( !memcmp(pucHeaderBuf + nCount,
                      m_nFrameSyncMarker,
                      FOURCC_SIGNATURE_BYTES) )
          {
            bSyncMarkerFound = true;
            break;
          }
          nCount++;
        }
        m_nOffset += nCount;
      } while(!bSyncMarkerFound);

      //free HeaderBuf after sync marker is found.
      if (pucHeaderBuf)
      {
        MM_Free(pucHeaderBuf);
        pucHeaderBuf = NULL;
      }
      //calculate approximate sample number.
      m_audsampleinfo.sample = (uint32)
                      (m_nOffset / m_SyncFrameInfo.usFrameSize);
    }
    else
    {
      *bError        = true;
      seek_time      = 0;
      _fileErrorCode = PARSER_ErrorSeekFail;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                   "Seek is not supported for id %lu", id);
    }
  }

  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
               "Seek time is %llu, and seeked to offset %llu, sample num %lu",
               repos_time, m_nOffset, m_audsampleinfo.sample);

  return seek_time;
}

/* ======================================================================
FUNCTION:
  cDTSFile::getTrackAudioSamplingFreq

DESCRIPTION:
  gets audio track's sampling frequency

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 cDTSFile::getTrackAudioSamplingFreq(uint32 /*id*/)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "cDTSFile::getTrackAudioSamplingFreq");
  return m_SyncFrameInfo.ulSamplingRate;
}

/* ======================================================================
FUNCTION:
  cDTSFile::GetNumAudioChannels

DESCRIPTION:
  returns number of audio channels

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 cDTSFile::GetNumAudioChannels(int /*id*/)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "cDTSFile::GetNumAudioChannels");
  return m_SyncFrameInfo.usNumChannels;
}

/* ======================================================================
FUNCTION:
 cDTSFile::getTrackMaxBufferSizeDB

DESCRIPTION:
  gets maximum buffer size to play the track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
int32 cDTSFile::getTrackMaxBufferSizeDB(uint32 /*id*/)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "cDTSFile::getTrackMaxBufferSizeDB");
  int32 bufferSize = AUDIO_DTS_MAX_FRAME_SIZE;
  if(FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM == m_hFrameOutputModeEnum)
  {
    bufferSize = DTS_AUDIO_BUF_SIZE;
  }
  else
  {
    //! For safety allocate 4 bytes extra to the frame size.
    //! This will be helpful in the case where frame size is not multiple of
    //! 4
    bufferSize = (m_SyncFrameInfo.usFrameSize + FOURCC_SIGNATURE_BYTES) *
                 DTS_NUM_FRAMES;
  }
  return bufferSize;
}

/* ======================================================================
FUNCTION:
  cDTSFile::peekCurSample

DESCRIPTION:
  gets information about current sample of a track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE cDTSFile::peekCurSample(uint32 /*trackid*/,
                                        file_sample_info_type *pSampleInfo)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "cDTSFile::peekCurSample");
  *pSampleInfo = m_audsampleinfo;
  return PARSER_ErrorNone;
}

/* ======================================================================
FUNCTION:
  cDTSFile::getTrackWholeIDList

DESCRIPTION:
  gets list of track IDs

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 cDTSFile::getTrackWholeIDList(uint32 *ids)
{
  int32 nTracks = 0;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "cDTSFile::getTrackWholeIDList");
  nTracks = AUDIO_DTS_MAX_TRACKS;
  *ids = nTracks;
  return nTracks;
}

/* ======================================================================
FUNCTION:
  cDTSFile::getTrackAverageBitrate

DESCRIPTION:
  gets track bitrate

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
 int32  cDTSFile::getTrackAverageBitrate(uint32 /*id*/)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "cDTSFile::getTrackAverageBitrate");
  return (int32)m_SyncFrameInfo.ulBitRate;
}

/* ======================================================================
FUNCTION
      cDTSFile::::GetStreamParameter()

  DESCRIPTION
      This function is used to get DTS Audio stream parameter i.e.
      BSMOD,ACMOD,Number of channels etc from specific parser.

  PARAMETERS
      @param[in]  ulTrackId    TrackID of media
      @param[in]  ulParamIndex Parameter Index of the structure to be
                               filled.It is from the FS_MEDIA_INDEXTYPE
                               enumeration.
      @param[in]  pParamStruct Pointer to client allocated structure to
                               be filled by the underlying parser.

  RETURN VALUE
      @return     PARSER_ErrorNone in case of success otherwise Error.
  SIDE EFFECTS    None

 ========================================================================== */
PARSER_ERRORTYPE cDTSFile::GetStreamParameter( uint32 /*ulTrackId*/,
                                               uint32 ulParamIndex,
                                               void* pParamStruct)
{
  PARSER_ERRORTYPE eError = PARSER_ErrorNone;
  if(NULL == pParamStruct)
  {
    eError = PARSER_ErrorInvalidParam;
  }
  else if (ulParamIndex == FS_IndexParamAudioDTS )
  {
    FS_AUDIO_PARAM_DTSTYPE *pDTSInfo = (FS_AUDIO_PARAM_DTSTYPE*)pParamStruct;

    memcpy(pDTSInfo, &m_SyncFrameInfo, sizeof(FS_AUDIO_PARAM_DTSTYPE));
    pDTSInfo->ulSamplingRate    = m_SyncFrameInfo.ulSamplingRate;
    pDTSInfo->ulBitRate         = m_SyncFrameInfo.ulBitRate;
    pDTSInfo->usNumChannels     = m_SyncFrameInfo.usNumChannels;
    pDTSInfo->ucBitStreamMode   = m_SyncFrameInfo.ucBitStreamMode;
    pDTSInfo->ucAudioCodingMode = m_SyncFrameInfo.ucAudioCodingMode;
    pDTSInfo->eSubType          = m_SyncFrameInfo.eSubType;
    pDTSInfo->usFrameSize       = m_SyncFrameInfo.usFrameSize;
    pDTSInfo->usSamplesperFrame = m_SyncFrameInfo.usSamplesperFrame;
    pDTSInfo->bDynamicRangeFlag = m_SyncFrameInfo.bDynamicRangeFlag;
    pDTSInfo->bBroadcastMetadataPresent =
                      m_SyncFrameInfo.bBroadcastMetadataPresent;
  }// if(pParamStruct)
  return eError;
}

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
FileSourceStatus cDTSFile::SetAudioOutputMode(FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  //Default mode is FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM
  //Parser do not support changing output mode during the playback
  switch (henum)
  {
    case FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME:
      if( (m_hFrameOutputModeEnum == FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM) )
      {
        m_hFrameOutputModeEnum = henum;
        m_nOutputFrames = 1;
        status = FILE_SOURCE_SUCCESS;
      }
      break;
    default:
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Unsupported enum value");
  }
  return status;
}

/* ======================================================================
FUNCTION:
  GetAudioOutputMode

DESCRIPTION:
  Called by user to retrieve output mode specified by henum

INPUT/OUTPUT PARAMETERS:
  henum-Output mode

RETURN VALUE:
 FILE_SOURCE_SUCCESS if successful in retrieving output mode
 else returns FILE_SOURCE_FAIL

SIDE EFFECTS:
  None.
======================================================================*/
FileSourceStatus cDTSFile::GetAudioOutputMode(bool* bret,
                                             FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  //Default mode is FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM
  //Parser do not support changing output mode during the playback
  switch (henum)
  {
    case FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME:
      if (m_hFrameOutputModeEnum == henum)
      {
        *bret = true;
        status = FILE_SOURCE_SUCCESS;
      }
      break;
    default:
      {
        *bret = false;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Unsupported enum value");
      }
  }
  return status;
}
/*===========================================================================

FUNCTION
  cDTSFile::Convert14bitBufTo16bitBuf

DESCRIPTION
  This function is to convert 14bit data into 16bit format.

DEPENDENCIES
  None

INPUT PARAMETERS:
  pDest: Destination Buffer
  pSrc:  Input Data Buffer
  ulBufLength : Input Buf Len
  bIsLittleEndian: Flag to indicate whether i/p is LE or BE.

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
void cDTSFile::Convert14bitBufTo16bitBuf(uint8 *pDest, const uint8 *pSrc,
                                         uint32 ulBufLength,
                                         bool bIsLittleEndian )
{
  uint8 ucTempChar, ucPresentChar = 0;
  uint16 usAvailBits, usConsumedBits = 0;
  uint32 nCount, OpBufPos = 0;

  for( nCount = 0; nCount < ulBufLength; nCount++  )
  {
    if( nCount%2 )
    {
      ucTempChar = pSrc[nCount - bIsLittleEndian];
      usAvailBits = CHAR_SIZE;
    }
    else
    {
      ucTempChar = pSrc[nCount + bIsLittleEndian] & 0x3F;
      usAvailBits = CHAR_SIZE - 2;
    }

    if( usConsumedBits < CHAR_SIZE )
    {
      uint16 usBitsToRead = FILESOURCE_MIN( uint16(CHAR_SIZE - usConsumedBits),
                                            usAvailBits );

      // Make way to keep required number of bits in ucPresentChar
      ucPresentChar = uint8(ucPresentChar << usBitsToRead);
      // Read required num of bits from Tempchar variable
      ucPresentChar |= uint8( ucTempChar >> (usAvailBits - usBitsToRead) );
      // Delete the bits that are taken into PresentChar value
      ucTempChar = uint8(ucTempChar &
                         (0xFF >> (CHAR_SIZE - usAvailBits + usBitsToRead)));
      //Update In and Out Bits based on number of  bits read.
      usAvailBits    = uint16(usAvailBits - usBitsToRead);
      usConsumedBits = uint16(usConsumedBits + usBitsToRead);
    }

    //If one complete character is read, then copy it into o/p buf
    if( CHAR_SIZE == usConsumedBits )
    {
      pDest[OpBufPos++] = ucPresentChar;
      ucPresentChar  = 0;
      usConsumedBits = 0;
    }

    //Copy remaining bits in TempChar to PresentChar
    usConsumedBits = uint16(usConsumedBits + usAvailBits);
    ucPresentChar  = uint8(ucPresentChar << usAvailBits);
    ucPresentChar  = uint8(ucPresentChar | ucTempChar);
  }
  return;
}

/*===========================================================================

FUNCTION
  cDTSFile::ParseRev2AuxData

DESCRIPTION
  Parse DTS frame to check whether Rev2 Aux data is present or not..

DEPENDENCIES
  None

INPUT PARAMETERS:
  pucFrameBuff: Input Data buffer

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
PARSER_ERRORTYPE cDTSFile::ParseRev2AuxData(uint8* pucFrameBuff)
{
  uint32 ulIndex   = m_SyncFrameInfo.usFrameSize;
  uint8  ucFlag    = false;
  bool   bAuxData  = false;

  //Skip Frame Sync marker worth of data
  uint32 ulCurBitOffset = FOURCC_SIGNATURE_BYTES * 8;
  PARSER_ERRORTYPE eRetError = PARSER_ErrorInvalidParam;

  if((NULL == pucFrameBuff) || ulIndex < DTS_HEADER_SIZE)
  {
    return eRetError;
  }

  //Check whether Rev2Aux data is present or not at end of the frame
  do
  {
    uint8* pBuf = pucFrameBuff + ulIndex;
    if((!memcmp(pBuf, DTS_REV2_SYNCWORD_LE, FOURCC_SIGNATURE_BYTES)) ||
       ((!memcmp(pBuf, DTS_REV2_SYNCWORD, FOURCC_SIGNATURE_BYTES)) &&
         m_bIsBigEndian))
    {
      bAuxData = true;
      break;
    }
    ulIndex --;
  } while ((int32)ulIndex > 0);
  if (!bAuxData)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "cDTSFile::ParseRev2AuxData Aux Data not found");
    return PARSER_ErrorNone;
  }
  memmove(pucFrameBuff, pucFrameBuff + ulIndex, DTS_AUDIO_BUF_SIZE - ulIndex);

  //Skip 4bytes used for sync marker!!
  ulIndex += 4;

  //7bits are used to store Rev2Aux data Size info. Minimum Frame Size value is 4.
  //As per spec, this value stored is 1byte less than actual value.
  m_SyncFrameInfo.nRev2AUXDataByteSize = (uint8)
     GetBitsFromBuffer(7, ulCurBitOffset, pucFrameBuff, DTS_AUDIO_BUF_SIZE);
  m_SyncFrameInfo.nRev2AUXDataByteSize++;
  ulCurBitOffset += 7;

  /* Currently this CRC check logic was not used. If required, we will add it
     in future. */
  uint16 CalcCRC = CalculateCRC16(pucFrameBuff + 4,
                                  m_SyncFrameInfo.nRev2AUXDataByteSize);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
               "cDTSFile::ParseRev2AuxData CRC value is %u", CalcCRC);

  if ((m_SyncFrameInfo.nRev2AUXDataByteSize < 3) ||
      (m_SyncFrameInfo.nRev2AUXDataByteSize > 128))
  {
    m_SyncFrameInfo.nRev2AUXDataByteSize = 0;
    return PARSER_ErrorNone;
  }

  ucFlag = (uint8)GetBitsFromBuffer(1, ulCurBitOffset, pucFrameBuff,
                                    DTS_AUDIO_BUF_SIZE);
  if (ucFlag)
  {
    m_SyncFrameInfo.bESMetaDataFlag = true;
  }
  ulCurBitOffset += 1;

  if (m_SyncFrameInfo.bESMetaDataFlag)
  {
    uint8 ucDownMixIndex = (uint8)
          GetBitsFromBuffer(8, ulCurBitOffset, pucFrameBuff, DTS_AUDIO_BUF_SIZE);
    if ((ucDownMixIndex < 40) || (ucDownMixIndex > 240))
    {
      m_SyncFrameInfo.bESMetaDataFlag = false;
    }
    ulCurBitOffset += 8;
  }
  m_SyncFrameInfo.bBroadcastMetadataPresent = false;
  if (m_SyncFrameInfo.nRev2AUXDataByteSize > 4)
  {
    ucFlag = (uint8)GetBitsFromBuffer(1, ulCurBitOffset, pucFrameBuff,
                                      DTS_AUDIO_BUF_SIZE);
    if (ucFlag)
    {
      m_SyncFrameInfo.bBroadcastMetadataPresent = true;
    }
    ulCurBitOffset += 1;
  }
  if (m_SyncFrameInfo.bBroadcastMetadataPresent)
  {
    ucFlag = (uint8)GetBitsFromBuffer(1, ulCurBitOffset, pucFrameBuff,
                                      DTS_AUDIO_BUF_SIZE);
    if (ucFlag)
    {
      m_SyncFrameInfo.bDRCMetadataPresent = true;
    }
    ulCurBitOffset += 1;

    ucFlag = (uint8)GetBitsFromBuffer(1, ulCurBitOffset, pucFrameBuff,
                                      DTS_AUDIO_BUF_SIZE);
    if (ucFlag)
    {
      m_SyncFrameInfo.bDialnormMetadata = true;
    }
    ulCurBitOffset += 1;
  }

  return PARSER_ErrorNone;
}
#endif /* FEATURE_FILESOURCE_DTS  */

