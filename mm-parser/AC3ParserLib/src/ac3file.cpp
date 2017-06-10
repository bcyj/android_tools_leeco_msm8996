/* =======================================================================
                         ac3file.cpp
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

Copyright (c) 2009-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AC3ParserLib/main/latest/src/ac3file.cpp#40 $
$DateTime: 2014/03/03 22:26:49 $
$Change: 5382439 $

========================================================================== */

/* ==========================================================================

INCLUDE FILES FOR MODULE

========================================================================== */

/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "filebase.h"
#include "ac3file.h"
#ifdef FEATURE_FILESOURCE_AC3
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
    AC3-bit stream
    syncframe()
    {
      syncinfo();
      bsi();
      for(blk = 0; blk < 6; blk++)
      {
        audblk();
      }
      auxdata();
      errorcheck();
    }

    // sync info syntax (A/52B-2005)
     *************************           *****************************
     * AC3(DD)[section 5.3.1]*           * EAC3(DD+)[Section Annex-E *
     *************************           *****************************
    |-------------|-------|-------|     |-------------|-------|-------|
    |syntax       | size  |offset |     |syntax       | size  |offset |
    |             | (bits)|(bytes)|     |             | (bits)|(bytes)|
    |-------------|-------|-------|     |-------------|-------|-------|
    |syncinfo()   |       |       |     |syncinfo()   |       |       |
    |{            |       |       |     |{            |       |       |
    |  syncword;  |  16   |  0    |     |  syncword;  |  16   |  0    |
    |  crc1;      |  16   |  2    |     |}            |       |       |
    |  fscod;     |   2   |  4    |     |-------------|-------|-------|
    |  frmsizcod; |   6   |  4    |
    |}            |       |       |
    |-------------|-------|-------|

    // bit-stream information syntax (A/52B-2005)
    *************************           *****************************
    * AC3(DD)[section 5.3.1]*           * EAC3(DD+)[Section Annex-E *
    *************************           *****************************
    |-------------|------|-------|     |--------------|------|-------|
    |syntax       |size  |offset |     |syntax        |size  |offset |
    |             |(bits)|(bytes)|     |              |(bits)|(bytes)|
    |-------------|------|-------|     |--------------|------|-------|
    |bsi()        |      |       |     |bsi()         |      |       |
    |{            |      |       |     |{             |      |       |
    |  bsid;      |  5   |  5    |     |  strmtyp;    |  2   |  2    |
    |  bsmod;     |  3   |  5    |     |  substreamid;|  3   |  2    |
    |  acmod;     |  2   |  6    |     |  frmsiz;     | 11   |  2-3  |
    |}            |  2   |  6    |     |  fscod;      |  2   |  4    |
    |             |      |       |     |  fscod2;     |  2   |  4    |
    |             |      |       |     |  acmod;      |  3   |  4    |
    |             |      |       |     |  lfeon;      |  1   |  4    |
    |             |      |       |     |  bsid;       |  5   |  5    |
    |             |      |       |     |  dialnorm    |  5   |  6    |
    |             |      |       |     |  compre      |  1   |  6    |
    |             |      |       |     |  compre-     |  8   |  7    |
    |             |      |       |     |              |      |       |
    |             |      |       |     |}             |      |       |
    |-------------|------|-------|     |--------------|------|-------|

    |---|---|---|---|-----------|-------|-------|
    | 0 | 1 | 2 | 3 |     4     |   5   |   6   |
    |---|---|---|---|-----------|-------|-------|
    | 0x0B77| crc1  |fscod/     | bsid  | acmod |
    |       |       |frmsizcod  | bsmod |       |
    |-------|-------|-----------|-------|-------|
 */


#define BSI_BSID_OFFSET       5
#define AC3_BSI_ACMOD_OFFSET  6
#define AC3_FRAMESIZE_OFFSET  4
#define EAC3_BSI_ACMOD_OFFSET 4
#define EAC3_FRAMSIZE_OFFSET  2
#define SAMPLE_RATE_OFFSET    4
#define FSCOD_RESERVE         2
#define FRAMESIZE_CODE_MAX    37

/* ======================================================================
FUNCTION:
  GetExtraChannelCount

DESCRIPTION:
  This function is used to calculate extra channels available with
  dependent sub-streams. This function will extra number of channels info
  from Channel Map field.

INPUT/OUTPUT PARAMETERS:
  usCustChannelMap         : Extended channel map info

RETURN VALUE:
 total number of channels available

SIDE EFFECTS:
  None.
======================================================================*/
uint16 AC3File::GetExtraChannelCount(uint16 usCustChannelMap)
{
  uint16 usNumChannels = 0;
  uint16 usChannelMask = 0x8000;

  //! Check whether dependent stream has LFE channel or not
  if (usCustChannelMap & 0x1)
  {
    usNumChannels++;
  }
  //! At max 5 channels can be present
  for(int nIndex = 4; nIndex >= 0; nIndex--)
  {
    if (usCustChannelMap & usChannelMask)
    {
      usNumChannels++;
    }
    //! Right shift by "1" to read next bit
    usCustChannelMap = uint16(usCustChannelMap << 1);
  }
  return usNumChannels;
}

/* ======================================================================
FUNCTION:
  AC3File::getFrameSize

DESCRIPTION:
  This function is used to calculate frame Size
INPUT/OUTPUT PARAMETERS:
  pucFrameBuff         : Frame Buffer

RETURN VALUE:
 Frame Size

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AC3File::getFrameSize(uint8 *pucFrameBuff)
{
  uint32 ulFrameSize  = 0;
  uint8  ucOffsetCorr = 0;
  bool   bFrameMarker = false;
  if( ( 0x0B == pucFrameBuff[0] ) && ( 0x77 == pucFrameBuff[1] ) )
  {
    // 8-bit or 16-bit LE stream sync(0x0B 0x77)
    ucOffsetCorr   = 0;
    m_bIsBigEndian = false;
    bFrameMarker   = true;
  }
  else if( ( 0x77 == pucFrameBuff[0] ) && ( 0x0B == pucFrameBuff[1] ) )
  {
    // 16-bit BE stream sync word (0x77 0x0B )
    ucOffsetCorr   = 1;
    m_bIsBigEndian = true;
    bFrameMarker   = true;
  }

  if(AC3_AUDIO == m_OTIType && bFrameMarker)
  {
    uint8 ucFrameSizeCod = pucFrameBuff[SAMPLE_RATE_OFFSET + ucOffsetCorr] & 0x3F;
    uint8 ucFsCod = (pucFrameBuff[SAMPLE_RATE_OFFSET + ucOffsetCorr] & 0xC0) >> 6;
    ulFrameSize = AC3_FRAME_SIZE_CODE[ucFrameSizeCod].ulFrameSize[ucFsCod];
  }
  else if(bFrameMarker)
  {
    // Get frame-size information depending upon LE or BE byte order
    if ( 1== ucOffsetCorr )
    {
      ulFrameSize =
      (uint32)(((uint16)pucFrameBuff[EAC3_FRAMSIZE_OFFSET])|
      ((uint16)(pucFrameBuff[EAC3_FRAMSIZE_OFFSET+1] & 0x7 ) << 8));
    }
    else
    {
      ulFrameSize =
        (uint32)(((uint16)pucFrameBuff[EAC3_FRAMSIZE_OFFSET+1])|
        ((uint16)(pucFrameBuff[EAC3_FRAMSIZE_OFFSET] & 0x7 ) << 8));
    }
    // Frame Size will not include header word. It needs to be added explicitly
    // Sometimes, Frame Size calculated can be less than min header size.
    // Do not consider that as valid frame, ignore it.
    if(ulFrameSize > AC3_HEADER_SIZE)
      ulFrameSize ++;
    else
      ulFrameSize = 0;
  }
  ulFrameSize = ulFrameSize * 2;
  return ulFrameSize;
}

/* ======================================================================
FUNCTION:
  AC3File::FileGetData

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
uint32 AC3File::FileGetData(  uint64 nOffset,
                              uint32 nNumBytesRequest,
                              uint32 nMaxSize,
                              uint8 *pData  )
{
  uint32 nRead = 0;
  if (m_AC3FilePtr != NULL)
  {
    if(!m_bMediaAbort)
    {
      nRead = FileBase::readFile(m_AC3FilePtr, pData, nOffset,
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
  AC3File::AC3File

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
AC3File::AC3File()
{
  InitData();
}

/*===========================================================================

FUNCTION
  AC3File::AC3File

DESCRIPTION
  creates instnace of AC3File instance

DEPENDENCIES
  None

INPUT PARAMETERS:


RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
AC3File:: AC3File(const FILESOURCE_STRING &filename,
                  unsigned char *pFileBuf, uint64 bufSize)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "AC3File:: AC3File");
  InitData();
  if (pFileBuf && bufSize)
  {
    m_pFileBuf = pFileBuf;
    m_FileBufSize = bufSize;
    m_AC3FilePtr = OSCL_FileOpen (pFileBuf, bufSize);
  }
  else
  {
     m_filename = filename;
#ifdef FEATURE_FILESOURCE_FILE_IO_CACHING
     /* Calling with 10K cache  buffer size */
     m_AC3FilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"),
                                  FILE_READ_BUFFER_SIZE_FOR_AC3 );
#else
     m_AC3FilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"));
#endif
     m_fileSize = OSCL_FileSize( m_filename );
  }
  if(m_AC3FilePtr != NULL)
  {
    if(PARSER_ErrorNone == ParseAC3Header())
    {
      _fileErrorCode = PARSER_ErrorNone;
      _success = true;
    }
  }
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
AC3File::AC3File(video::iStreamPort* pport)
{
  InitData();
  m_pPort = pport;
  m_AC3FilePtr = OSCL_FileOpen(pport);
  if(m_AC3FilePtr != NULL)
  {
    if(m_pPort)
    {
      int64 noffset = 0;
      if(video::iStreamPort::DS_SUCCESS == m_pPort->GetContentLength(&noffset))
      {
        m_fileSize = (uint64)noffset;
      }
    }
    if(m_AC3FilePtr != NULL)
    {
      if(PARSER_ErrorNone == ParseAC3Header())
      {
        _fileErrorCode = PARSER_ErrorNone;
        _success = true;
      }
    }
  }
}
#endif

void AC3File::InitData()
{
  m_bHdrParsingDone = false;
  m_bMediaAbort   = false;
  m_pFileBuf      = NULL;
  m_FileBufSize   = 0;
  m_filename      = NULL;
  m_bIsBigEndian  = FALSE;
  m_bStreaming    = FALSE;
  m_fileSize      = 0;
  m_endOfFile     = false;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  m_pPort         = NULL;
#endif
  m_nOffset       = 0;
  m_bIsAC3File    = false;
  m_nStartOffset  = 0;
  m_totalDuration = 0;
  m_OTIType       = AC3_AUDIO;
  _fileErrorCode  = PARSER_ErrorDefault;
  _success        = false;
  memset(&m_audsampleinfo,0,sizeof(m_audsampleinfo));
  memset(&m_SyncFrameInfo, 0, sizeof(FS_AUDIO_PARAM_AC3TYPE));

  /* By default, Parser will provide AC3_NUM_FRAMES frames in single
     getNextMediaSample call */
  m_hFrameOutputModeEnum = FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM;
  m_nOutputFrames = AC3_NUM_FRAMES;
  m_AC3FilePtr    = NULL;
}
void AC3File::SetCriticalSection(MM_HANDLE pcriticalsection)
{
  if(m_AC3FilePtr)
  {
    m_AC3FilePtr->pCriticalSection = pcriticalsection;
  }
}
/*===========================================================================

FUNCTION
  AC3File::~AC3File

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

AC3File::~AC3File()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "AC3File::~AC3File");
  if(m_AC3FilePtr!=NULL)
  {
     OSCL_FileClose(m_AC3FilePtr);
     m_AC3FilePtr = NULL;
  }
}
/*===========================================================================

FUNCTION
  AC3File::ParseAC3Header

DESCRIPTION
  creates instnace of AMRparser and calls start on parsing AMR file

DEPENDENCIES
  None

INPUT PARAMETERS:


RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
PARSER_ERRORTYPE AC3File::ParseAC3Header()
{
  PARSER_ERRORTYPE eRetError = PARSER_ErrorDefault;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "AC3File::ParseAC3Header");
  uint8 ucOffsetCorr     = 0;
  uint32 ulCurrentOffset = 0;
  uint32 ulReadDataSize  = 0;
  uint8 *pucHeaderBuf = (uint8*)MM_Malloc(AC3_AUDIO_BUF_SIZE);
  if ( NULL != pucHeaderBuf )
  {
    uint32 nCount = 0;
    ulReadDataSize = FileGetData(ulCurrentOffset, AC3_AUDIO_BUF_SIZE,
                                 AC3_DEFAULT_AUDIO_BUF_SIZE, pucHeaderBuf );
    if(AC3_HEADER_SIZE > ulReadDataSize)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "AC3File::ParseAC3Header read failed %d", AC3_HEADER_SIZE);
      eRetError = PARSER_ErrorReadFail;
    }

    /* Check whether min AC3_HEADER_SIZE worth of data is available or not. */
    for(nCount = 0; nCount < ulReadDataSize - AC3_HEADER_SIZE; nCount++)
    {
      uint32 ulNextFrameSize = 0;
      if( ( 0x0B == pucHeaderBuf[nCount] ) &&
          ( 0x77 == pucHeaderBuf[nCount + 1] ) )
      {
        // 8-bit or 16-bit LE stream sync(0x0B 0x77)
        ucOffsetCorr = 0;
        eRetError = PARSER_ErrorNone;
        m_bIsBigEndian = false;
      }
      else if( ( 0x77 == pucHeaderBuf[nCount] ) &&
               ( 0x0B == pucHeaderBuf[nCount + 1] ) )
      {
        // 16-bit BE stream sync word (0x77 0x0B )
        ucOffsetCorr = 1;
        eRetError = PARSER_ErrorNone;
        m_bIsBigEndian = true;
      }

      if (PARSER_ErrorNone == eRetError )
      {
        /* bsid(offset = 5), 5-bit (1111xxx)*/
        uint8 ucBSID = uint8(pucHeaderBuf
                       [nCount + BSI_BSID_OFFSET - ucOffsetCorr] >> 3);
        if( IS_DOLBY_DIGITAL(ucBSID) /*AC3*/)
        {
          // Dolby digital audio
          m_OTIType = AC3_AUDIO;
          eRetError = ParseFrameHeaderAC3(pucHeaderBuf + nCount, ucOffsetCorr);
        }
        else if( IS_DOLBY_DIGITAL_PLUS(ucBSID) /*EAC3*/)
        {
          // Dolby digital plus audio
          m_OTIType = EAC3_AUDIO;
          eRetError = ParseFrameHeaderEAC3(pucHeaderBuf + nCount, ucOffsetCorr);
        }
        else
        {
          eRetError = PARSER_ErrorDefault;
        }
      }//if(PARSER_ErrorNone)

      /* If input stream is validated as AC3 bit stream, validate next frame
         also */
      if (PARSER_ErrorNone == eRetError)
      {
        /* if header parsing is not yet completed, then calculate current
           frame size  */
        if (false == m_bHdrParsingDone)
        {
          ulNextFrameSize = getFrameSize(pucHeaderBuf + nCount);
        }
        else if((nCount + m_SyncFrameInfo.usFrameSize) <
                (ulReadDataSize - AC3_HEADER_SIZE))
        {
          ulNextFrameSize = getFrameSize(pucHeaderBuf + nCount +
                                         m_SyncFrameInfo.usFrameSize);
        }
        /* If header parsing is completed, but data available is not sufficient
           to read next frame size, then also use current frame size. */
        else
        {
          ulNextFrameSize = m_SyncFrameInfo.usFrameSize;
        }

        if(ulNextFrameSize > 0)
        {
          //! Update start offset value with first frame offset value
          if (!m_bIsAC3File)
          {
            m_nStartOffset = m_nOffset = nCount;
          }
          m_bIsAC3File = true;
          nCount   +=  (ulNextFrameSize - 1);
          //! If header parsing is already completed, then break loop
          if (m_bHdrParsingDone)
          {
            break;
          }
          eRetError = PARSER_ErrorDefault;
        } //! if(ulNextFrameSize > 0)
        else
        {
          eRetError = PARSER_ErrorStreamCorrupt;
        }
      } //! if (PARSER_ErrorNone == eRetError)
      /* If data read is not sufficient to calculate next frame size, then
         report success. In the first 8192 bytes read, if at least one AC3
         frame is not identified, then report clip as erroneous. */
      if( (ulNextFrameSize) && (ulReadDataSize <= ulNextFrameSize + nCount) )
      {
        if(false == m_bIsAC3File)
        {
          m_nStartOffset = m_nOffset = nCount;
          m_bIsAC3File   = true;
        }
        /* If complete metadata parsing is not completed and data available
           is not sufficient to process next frame, then read data */
        if (!m_bHdrParsingDone)
        {
          ulCurrentOffset += nCount + ulNextFrameSize;
          ulReadDataSize = FileGetData(ulCurrentOffset, AC3_AUDIO_BUF_SIZE,
                                    AC3_DEFAULT_AUDIO_BUF_SIZE, pucHeaderBuf);
          if(AC3_HEADER_SIZE > ulReadDataSize)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
              "AC3File::ParseAC3Header read failed %d", AC3_HEADER_SIZE);
            eRetError = PARSER_ErrorReadFail;
          }
        }
        else
        {
          break;
        }
        nCount = 0;
      } //! if (ulReadDataSize <= m_SyncFrameInfo.usFrameSize + nCount)
    }//! for(nCount = 0; nCount < ulReadDataSize - AC3_HEADER_SIZE; nCount++)
  }//! if ( NULL != pucHeaderBuf )
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

/* ======================================================================
FUNCTION:
  AC3File::ParseFrameHeaderAC3

DESCRIPTION:
  gets next sample of the given track.

INPUT/OUTPUT PARAMETERS:
@param[in] pucFrameBuff Data to parse AC3 header
@param[in] ucOffsetCorr Flag to indicate whether bitstream is BE or LE

RETURN VALUE:
 PARSER_ErrorNone in Successful case /
 Corresponding error code in failure cases

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE AC3File::ParseFrameHeaderAC3( uint8* pucFrameBuff,
                                               uint8 ucOffsetCorr )
{
  PARSER_ERRORTYPE eRetError = PARSER_ErrorDefault;
  if ( NULL != pucFrameBuff)
  {
   /* fscod/frmsizcod ( Sample Rate Code & frame-size code): offset = 4
    * |----------|----|-------|
    * |Field     |bits|offset |
    * |----------|----|-------|
    * |FSCOD     | 2  |  4    |
    * |FRMSIZCOD | 4  |  4    |
    * |----------|----|-------|
    */
    uint8 ucFsCod = (pucFrameBuff[SAMPLE_RATE_OFFSET + ucOffsetCorr] & 0xC0) >> 6;
    /* Get frame-size code */
    uint8 ucFrameSizeCod = pucFrameBuff[SAMPLE_RATE_OFFSET + ucOffsetCorr] & 0x3F;

    /* Update frame-rate, frame-size & sample-rate */
    if ( ucFsCod > FSCOD_RESERVE || ucFrameSizeCod > FRAMESIZE_CODE_MAX )
    {
      m_SyncFrameInfo.ulBitRate = 0;
      m_SyncFrameInfo.ulSamplingRate = 0;
      m_SyncFrameInfo.usFrameSize = 0;
      eRetError = PARSER_ErrorStreamCorrupt;
    }
    else
    {
      // Update sample-rate
      m_SyncFrameInfo.ulSamplingRate = AC3_FSCODE_RATE[ucFsCod];
      // Update bit-rate
      m_SyncFrameInfo.ulBitRate =
        AC3_FRAME_SIZE_CODE[ucFrameSizeCod].ulFrameBitRate;
      // Convert frame-size from words to bytes
      m_SyncFrameInfo.usFrameSize =
        AC3_FRAME_SIZE_CODE[ucFrameSizeCod].ulFrameSize[ucFsCod] *2;
      /* BSMOD( bit-stream mode) & ACMOD( audio coding mode)
       * |------|----|-------|
       * |Field |bits|offset |
       * |------|----|-------|
       * |BSMOD | 3  |  5    |
       * |ACMOD | 3  |  6    |
       * |------|----|-------|
       */
      // Update BSMOD information
      m_SyncFrameInfo.ucBitStreamMode =
        ( pucFrameBuff[BSI_BSID_OFFSET - ucOffsetCorr] & 0x07 );
      // Update ACMOD information
      m_SyncFrameInfo.ucAudioCodingMode = uint8(
        pucFrameBuff[AC3_BSI_ACMOD_OFFSET + ucOffsetCorr] >> 5);
      //Update number of channels information.
      m_SyncFrameInfo.usNumChannels =
        (uint16)ACMOD_CHANNELS[m_SyncFrameInfo.ucAudioCodingMode];
      //! Check if lfeon is present or not
      if (pucFrameBuff[AC3_BSI_ACMOD_OFFSET + ucOffsetCorr] & 0x01)
      {
        m_SyncFrameInfo.usNumChannels++;
      }
      // Update clip duration
      if ( m_SyncFrameInfo.ulBitRate )
      {
        m_totalDuration =(m_fileSize * 8)/m_SyncFrameInfo.ulBitRate;
      }
      /* Update bitrate info into Kbps units.
         Initially it is stored in Mbps units. */
      m_SyncFrameInfo.ulBitRate *= MILLISEC_TIMESCALE_UNIT;
      eRetError = PARSER_ErrorNone;
    }//if(fscod & frmsizcod)
  }
  else
  {
    eRetError = PARSER_ErrorInvalidParam;
  }
  //! Set the flag to indicate Header parsing is completed
  //! No dependent streams are available in normal AC3 file case
  m_bHdrParsingDone = true;
  return (eRetError);
}

/* ======================================================================
FUNCTION:
  AC3File::ParseFrameHeaderEAC3

DESCRIPTION:
  gets next sample of the given track.

INPUT/OUTPUT PARAMETERS:
@param[in] pucFrameBuff Data to parse EAC3 header
@param[in] ucOffsetCorr Flag to indicate whether bitstream is BE or LE

RETURN VALUE:
 PARSER_ErrorNone in Successful case /
 Corresponding error code in failure cases

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE AC3File::ParseFrameHeaderEAC3( uint8* pucFrameBuff,
                                                uint8 ucOffsetCorr )
{
  PARSER_ERRORTYPE eRetError = PARSER_ErrorDefault;
  uint32 ulSize   = 128;
  uint8 ucBuf[128];

  /* In EAC3 (Enhanced AC3) bit-stream case, file may contain at least one
     independent stream and up to 7 dependent streams. Parser has to include
     number of channels used in those sub-streams in the total channel count.
     One independent or dependent streams will be one frame. So Parser has to
     parse all the streams till one more independent stream is reached.

     There is also possibility of having more than one independent stream in
     EAC3 clip. We are not adding support for that now. In future, if required
     we will add it.
  */

   /*
    * |----------|----|
    * |blocks    |bits|
    * |----------|----|
    * |1         | 00 |
    * |2         | 01 |
    * |3         | 10 |
    * |6         | 11 |
    * |----------|----|
    */
  uint8 ucNumBlksCode[4] = {1,2,3,6};
  uint8 ucNumBlocks = 0;
  //! Bit-offset value
  uint32 usOffset = 16;
  if ( NULL != pucFrameBuff)
  {
    memcpy(ucBuf, pucFrameBuff, ulSize);
    if (ucOffsetCorr)
    {
      BYTESWAP(ucBuf, 128);
    }
    /* EAC3 file format is explained in this file at the line no: 86. */
    uint8 ucStrType = (uint8)GetBitsFromBuffer(2, usOffset, ucBuf, ulSize);
    usOffset += 2;
    //! If stream is independent and already parsed, then return directly
    //! without proceeding further.
    if ((0 == ucStrType) && (true == m_bIsAC3File))
    {
      m_bHdrParsingDone = true;
      return PARSER_ErrorNone;
    }
    uint8 ucSubStrId = (uint8)GetBitsFromBuffer(3, usOffset, ucBuf, ulSize);
    usOffset+= 3;
    uint16 usFrmSize = (uint16)GetBitsFromBuffer(11, usOffset, ucBuf, ulSize);
    usOffset+= 11;
    uint8 ucFsCod = (uint8)GetBitsFromBuffer(2, usOffset, ucBuf, ulSize);
    usOffset+= 2;
    uint8 ucFsCod2 = (uint8)GetBitsFromBuffer(2, usOffset, ucBuf, ulSize);
    usOffset+= 2;
    // Audio Coding mode
    uint8 ucAudCodeMode = (uint8)GetBitsFromBuffer(3, usOffset, ucBuf, ulSize);
    usOffset += 3;
    // LFE ON bit
    uint8 ucLFE = (uint8)GetBitsFromBuffer(1, usOffset, ucBuf, ulSize);
    usOffset += 1;
    //! Skip BSID and DialNorm fields (both are 5bits each)
    usOffset += 10;
    //! Check whether compression bit is set or not
    uint8 ucCompr = (uint8)GetBitsFromBuffer(1, usOffset, ucBuf, ulSize);
    usOffset += 1;
    if (ucCompr)
    {
      usOffset += 8;
    }
    /* if 1+1 mode (dual mono, so some items need a second value) */
    if (0 == ucAudCodeMode)
    {
      //! skip dialnorm2 field
      usOffset += 5;
      //! Check whether compression bit is set or not
      ucCompr = (uint8)GetBitsFromBuffer(1, usOffset, ucBuf, ulSize);
      usOffset += 1;
      if (ucCompr)
      {
        usOffset += 8;
      }
    }
    //! For dependent streams, extended channel map info may be present
    //! Do not update channel count unless independent stream is already parsed
    if ((1 == ucStrType) && (true == m_bIsAC3File) )
    {
      //! Check whether channel map extension bit is set or not
      uint8 ucChnMap = (uint8)GetBitsFromBuffer(1, usOffset, ucBuf, ulSize);
      usOffset += 1;
      if (ucChnMap)
      {
        //! Next 16bits contain channel map info for dependent streams
        uint16 usExtChnMap = (uint16)GetBitsFromBuffer(16, usOffset,
                                                       ucBuf, ulSize);
        m_SyncFrameInfo.usNumChannels = uint16(m_SyncFrameInfo.usNumChannels +
                                         GetExtraChannelCount(usExtChnMap) );
        usOffset += 16;
      }
    }

    //! Update EAC3 header if stream type is Independent type
    if (0 == ucStrType)
    {
      // Convert frame size from words to bytes
      m_SyncFrameInfo.usFrameSize = usFrmSize * 2 + AC3_FILE_SIGNATURE_BYTES;

      m_SyncFrameInfo.ucAudioCodingMode = ucAudCodeMode;
      // Number of channels.
      m_SyncFrameInfo.usNumChannels =(uint16)ACMOD_CHANNELS
                                          [m_SyncFrameInfo.ucAudioCodingMode];
      if (ucLFE)
      {
        m_SyncFrameInfo.usNumChannels++;
      }
      m_SyncFrameInfo.ulSamplingRate = AC3_FSCODE_RATE[ucFsCod];

      if ( 3 == ucFsCod )
      {
        /* In case of reduced sampling frequency no. of blocks are fixed "6" */
        m_SyncFrameInfo.ulSamplingRate = AC3_FSCODE_RATE[ucFsCod2]/2;
        ucNumBlocks = ucNumBlksCode[3];

        /* If Sampling frequency is not known, then update error code. */
        if(3 == ucFsCod2)
          eRetError = PARSER_ErrorStreamCorrupt;
      }
      else
      {
        /* If i/p frequency is not reduced to half value, then following 2bits
           will contain information about Number of blocks per frame. */
        ucNumBlocks = ucNumBlksCode[ucFsCod2];
      }
      /* For EC3 clips, bitrate info is not available in frame header.
         Parser will calc bitrate by using frame size and Sampling rate info.
         This value is average bitrate in case of VBR clips. */
      m_SyncFrameInfo.ulBitRate = (8 * m_SyncFrameInfo.usFrameSize *
                                   m_SyncFrameInfo.ulSamplingRate) /
                                   (ucNumBlocks * 256);
      if(m_SyncFrameInfo.ulBitRate)
      {
        m_totalDuration = (m_fileSize * 8 * MILLISEC_TIMESCALE_UNIT) /
                           m_SyncFrameInfo.ulBitRate;
        eRetError = PARSER_ErrorNone;
      }
    } //! if (0 == ucStrType)
    else
    {
      eRetError = PARSER_ErrorNone;
    }
  } //! if ( NULL != pucFrameBuff)
  else
  {
    eRetError = PARSER_ErrorInvalidParam;
  }
  return (eRetError);
}

/* ======================================================================
FUNCTION:
  AC3File::getNextMediaSample

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

PARSER_ERRORTYPE AC3File::getNextMediaSample(uint32 /*ulTrackID*/,
                                             uint8*  pucDataBuf,
                                             uint32* pulBufSize,
                                             uint32& /*rulIndex*/)
{
  uint32 nOutDataSize = 0;
  uint64 nTimeStamp = 0;
  uint32 num_frames = 0;
  uint32 frameSize  = 0;
  uint32 nBytesSkipped = 0;
  uint64 new_timestamp = 0;
  uint64 duration = 0;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "AC3File::getNextMediaSample @ %llu", m_nOffset);

  /* Validate input params and class variables */
  if(NULL == m_AC3FilePtr || NULL == pulBufSize || NULL == pucDataBuf ||
     0 == *pulBufSize)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
               "getNextMediaSample invalid input params!!");
    return PARSER_ErrorInvalidParam;
  }

  if( m_nOffset >= m_fileSize || m_endOfFile)
  {
    m_endOfFile = true;
    m_nOffset = m_nStartOffset;
    *pulBufSize = 0;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"AC3File::EOS!!");
    return PARSER_ErrorEndOfFile;
  }
  if((m_AC3FilePtr) && (!m_endOfFile))
  {
    uint32 ReadDataSize = *pulBufSize;
    if(1 == m_nOutputFrames)
      ReadDataSize = m_SyncFrameInfo.usFrameSize;

    /* Read "ReadDataSize" worth of data. If output is configured to provide
       one frame at a time, then read data equivalent to last frame Size.
       In case of multiple frames, read Buffer Size (*pulBufSize) worth of
       data in one instance. */
    nOutDataSize = FileGetData(m_nOffset, ReadDataSize, *pulBufSize,
                               pucDataBuf);

    //Reset output buffer Size value.
    *pulBufSize = 0;

    /* Run the loop for "m_nOutputFrames" times. Make sure at least one frame
       worth of data is available. Available data size should be greater than
       AC3_HEADER_SIZE (8bytes). */
    for(uint32 count = 0; count < m_nOutputFrames &&
        nOutDataSize > AC3_HEADER_SIZE; count++)
    {
      if(nBytesSkipped)
        break;
      frameSize = getFrameSize(pucDataBuf + *pulBufSize);
      if(!frameSize && !*pulBufSize)
      {
        for(uint32 nCount = 0; nCount < nOutDataSize && !frameSize; nCount++)
        {
          nBytesSkipped++;
          //! Read more data if data read is not sufficient.
          //! Compare if there is only one byte left out.
          if(!frameSize && nBytesSkipped == nOutDataSize - 1)
          {
            m_nOffset += nBytesSkipped;
            nOutDataSize = FileGetData(m_nOffset, ReadDataSize, ReadDataSize,
                                       pucDataBuf);
            nBytesSkipped = 0;
            nCount = 0;
          }
          frameSize = getFrameSize(pucDataBuf + nBytesSkipped);
        }
        m_nOffset += nBytesSkipped;
      }
      if(!frameSize)
        break;

      /* Update class variable accordingly */
      m_SyncFrameInfo.usFrameSize = frameSize;

      /* If Parser reads less than one frame Size, then read more data from
         the input file/buffer */
      if(!nBytesSkipped && nOutDataSize < frameSize && 0 == *pulBufSize)
      {
        uint32 dataRead = 0;
        dataRead = FileGetData(m_nOffset + nOutDataSize,
                               frameSize - nOutDataSize,
                               frameSize, pucDataBuf + nOutDataSize);
        nOutDataSize += dataRead;
      }
      else if(nBytesSkipped)
      {
        uint32 dataRead = 0;
        dataRead = FileGetData(m_nOffset, frameSize,
                               frameSize, pucDataBuf);
        nOutDataSize = dataRead;
      }

      //Validate whether we have min frame size worth of data or not
      if( nOutDataSize < frameSize)
      {
        /* If there is no single frame available, then update offset value
           to end of the file. This will be required to report End Of File.*/
        if(!*pulBufSize)
          m_nOffset += nOutDataSize;
        break;
      }

      /* In case frame data is not in bigEndian convension, convert it back
         to bigEndian convension. Here AC3 frames are word aligned, so frames
         with odd length are not possible.*/
      if(false == m_bIsBigEndian && nOutDataSize >= frameSize)
      {
        uint32 nCount  = 0;
        uint8* tempBuf = pucDataBuf + *pulBufSize;
        while(nCount < frameSize)
        {
          uint8 ucChar = tempBuf[nCount];
          tempBuf[nCount]     = tempBuf[nCount + 1];
          tempBuf[nCount + 1] = ucChar;
          nCount += 2;
        }
      }
      /* Update output buffer size and available data size parameters
         accordingly */
      *pulBufSize  += frameSize;
      nOutDataSize -= frameSize;
    }
    nOutDataSize = *pulBufSize;
    m_nOffset   += nOutDataSize;
  }

  /* Calculate timestamp only if output is configured to one frame */
  if(m_SyncFrameInfo.ulBitRate && 1 == m_nOutputFrames)
  {
    uint64 offset = m_nOffset - nOutDataSize;
    if(offset > 0)
    {
      nTimeStamp = (uint64)(offset * 8 * MILLISEC_TIMESCALE_UNIT)/
                            m_SyncFrameInfo.ulBitRate;
      if(m_SyncFrameInfo.ulBitRate)
      {
        num_frames = nOutDataSize/m_SyncFrameInfo.usFrameSize;
      }
    }
    new_timestamp = (uint64)(m_nOffset * 8 * MILLISEC_TIMESCALE_UNIT)/
                             m_SyncFrameInfo.ulBitRate;
    duration = new_timestamp - nTimeStamp;
    m_audsampleinfo.btimevalid = true;
  }
  else
  {
    m_audsampleinfo.btimevalid = false;
  }
  m_audsampleinfo.sample += 1 ;
  m_audsampleinfo.size = nOutDataSize;
  m_audsampleinfo.offset = m_nOffset;
  m_audsampleinfo.time = nTimeStamp;
  m_audsampleinfo.delta = duration;
  m_audsampleinfo.sync = 1;
  m_audsampleinfo.num_frames = num_frames;
  if(*pulBufSize)
  {
    return PARSER_ErrorNone;
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
               "getNextMediaSample End Of the File is reached!!");
    return PARSER_ErrorEndOfFile;
  }
}


/* ======================================================================
FUNCTION:
  AVIFile::getMediaTimestampForCurrentSample

DESCRIPTION:
  gets time stamp of current sample of the track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 time stamp in track time scale unit

SIDE EFFECTS:
  None.
======================================================================*/
uint64 AC3File::getMediaTimestampForCurrentSample(uint32 /*id*/)
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
    "AC3File::getMediaTimestampForCurrentSample: %llu", m_audsampleinfo.time);
  return m_audsampleinfo.time;
}

/* ======================================================================
FUNCTION:
  AC3File::resetPlayback

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
uint64 AC3File::resetPlayback(  uint64 repos_time, uint32 id,
                                bool /*bSetToSyncSample*/, bool *bError,
                                uint64 /*currentPosTimeStamp*/)
{
  uint64 seek_time = 0;
  uint64 coarse_Reposition = 0;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "AC3File::resetPlayback");

  if(repos_time <= m_totalDuration)
  {
    uint64 nSeekOffset = 0;
    if(m_totalDuration)
    {
      nSeekOffset = (repos_time * m_fileSize) / m_totalDuration;
    }
    coarse_Reposition = nSeekOffset % m_SyncFrameInfo.usFrameSize;
    m_nOffset = nSeekOffset - coarse_Reposition + m_nStartOffset;
    seek_time = repos_time;
    m_endOfFile = false;
    *bError = false;
    _fileErrorCode  = PARSER_ErrorNone;
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "Reposition failed for track id = %lu", id);
    *bError = true;
    _fileErrorCode  = PARSER_ErrorSeekFail;
  }
  return seek_time;
}

/* ======================================================================
FUNCTION:
  AC3File::getMovieDuration

DESCRIPTION:
  gets movie duration in movie timescale unit.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 AC3File::getMovieDuration() const
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "AC3File::getMovieDuration: %llu", m_totalDuration);
  return m_totalDuration;
}


/* ======================================================================
FUNCTION:
  AC3File::getMovieTimescale

DESCRIPTION:
  gets movie timescale

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AC3File::getMovieTimescale() const
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "AC3File::getMovieTimescale: %d",
               MILLISEC_TIMESCALE_UNIT);
  return MILLISEC_TIMESCALE_UNIT;
}

/* ======================================================================
FUNCTION:
  AC3File::getTrackAudioSamplingFreq

DESCRIPTION:
  gets audio track's sampling frequency

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AC3File::getTrackAudioSamplingFreq(uint32 /*id*/)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "AC3File::getTrackAudioSamplingFreq");
  return m_SyncFrameInfo.ulSamplingRate;
}

/* ======================================================================
FUNCTION:
  AC3File::GetNumAudioChannels

DESCRIPTION:
  returns number of audio channels

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AC3File::GetNumAudioChannels(int /*id*/)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "AC3File::GetNumAudioChannels");
  return m_SyncFrameInfo.usNumChannels;
}

/* ======================================================================
FUNCTION:
 AC3File::getTrackMaxBufferSizeDB

DESCRIPTION:
  gets maximum buffer size to play the track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
int32 AC3File::getTrackMaxBufferSizeDB(uint32 /*id*/)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "AC3File::getTrackMaxBufferSizeDB");
  int32 bufferSize = AC3_FRAME_SIZE_CODE[37].ulFrameSize[2] * AC3_NUM_FRAMES;
  return bufferSize;
}

/* ======================================================================
FUNCTION:
  AC3File::peekCurSample

DESCRIPTION:
  gets information about current sample of a track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE AC3File::peekCurSample(uint32 /*trackid*/,
                                        file_sample_info_type *pSampleInfo)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "AC3File::peekCurSample");
  *pSampleInfo = m_audsampleinfo;
  return PARSER_ErrorNone;
}

/* ======================================================================
FUNCTION:
  AC3File::getTrackWholeIDList

DESCRIPTION:
  gets list of track IDs

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AC3File::getTrackWholeIDList(uint32 *ids)
{
  int32 nTracks = 0;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "AC3File::getTrackWholeIDList");
  nTracks = AUDIO_AC3_MAX_TRACKS;
  *ids = nTracks;
  return nTracks;
}

/* ======================================================================
FUNCTION:
  AC3File::getTrackMediaDuration

DESCRIPTION:
  gets track duration in track time scale unit

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 AC3File::getTrackMediaDuration(uint32 /*id*/)
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "AC3File::getTrackMediaDuration %llu", m_totalDuration);
  return m_totalDuration;
}

/* ======================================================================
FUNCTION:
  AC3File::getTrackMediaTimescale

DESCRIPTION:
  gets track time scale

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AC3File::getTrackMediaTimescale(uint32 /*id*/)
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
              "AC3File::getTrackMediaTimescale: %d", MILLISEC_TIMESCALE_UNIT);
  return MILLISEC_TIMESCALE_UNIT;
}

/* ======================================================================
FUNCTION:
  AC3File::getTrackAverageBitrate

DESCRIPTION:
  gets track bitrate

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
 int32  AC3File::getTrackAverageBitrate(uint32 /*id*/)
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
             "AC3File::getTrackAverageBitrate: %lu",m_SyncFrameInfo.ulBitRate);
  return m_SyncFrameInfo.ulBitRate;
}
/* ======================================================================
FUNCTION
      AC3File::::GetStreamParameter()

  DESCRIPTION
      This function is used to get AC3 Audio stream parameter i.e.
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
PARSER_ERRORTYPE AC3File::GetStreamParameter( uint32 /*ulTrackId*/,
                                               uint32 ulParamIndex,
                                               void* pParamStruct)
{
  PARSER_ERRORTYPE eError = PARSER_ErrorNone;
  if(pParamStruct == NULL)
  {
    eError = PARSER_ErrorInvalidParam;
  }
  else if (FS_IndexParamAudioAC3 == ulParamIndex )
  {
    FS_AUDIO_PARAM_AC3TYPE *pAC3Info = (FS_AUDIO_PARAM_AC3TYPE*)pParamStruct;

    pAC3Info->ulSamplingRate    = m_SyncFrameInfo.ulSamplingRate;
    pAC3Info->ulBitRate         = m_SyncFrameInfo.ulBitRate;
    pAC3Info->usNumChannels     = m_SyncFrameInfo.usNumChannels;
    pAC3Info->ucBitStreamMode   = m_SyncFrameInfo.ucBitStreamMode;
    pAC3Info->ucAudioCodingMode = m_SyncFrameInfo.ucAudioCodingMode;
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
FileSourceStatus AC3File::SetAudioOutputMode(FileSourceConfigItemEnum henum)
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
FileSourceStatus AC3File::GetAudioOutputMode(bool* bret,
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

#endif /* FEATURE_FILESOURCE_AC3  */

