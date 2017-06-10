/* =======================================================================
qcpfile.cpp
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

Copyright 2011-2014 Qualcomm Technologies Incorporated, All Rights Reserved.
QUALCOMM Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/QCPParserLib/main/latest/src/qcpfile.cpp#40 $
$DateTime: 2014/02/07 02:53:30 $
$Change: 5240686 $

========================================================================== */


/* ==========================================================================

INCLUDE FILES FOR MODULE

========================================================================== */
#include "parserinternaldefs.h"
#include <stdio.h>
#include "qcpfile.h"
#include "qcpparser.h"
#include "filebase.h"
#include "MMMemory.h"
#ifdef FEATURE_FILESOURCE_DRM_DCF
#include "IxStream.h"
#endif

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


/* ======================================================================
FUNCTION
  QCPCallbakGetData

DESCRIPTION
  Its a callback function from the SDK to get the QCP data for decoding.
  This is not implemented by the SDK. It should be implemented by the app
  that calls the SDK.

ARGUMENTS
  ullOffset           Offset of the requested data (from beginning),
  ulNumBytesRequest   Size of the requested data (in bytes).
  pucData             Pointer to data buffer.
  ulMaxBufSize        Maximum data buffer size.
  pUserData           Client user data information.

DEPENDENCIES
  Used by QCP Parser.

RETURN VALUE
  The number of bytes actually being filled in

SIDE EFFECTS
  Detail any side effects.
========================================================================== */

uint32 QCPCallbakGetData (uint64 ullOffset,
                          uint32 ulNumBytesRequest,
                          uint8* pucData,
                          uint32 ulMaxBufSize,
                          void*  pUserData )
{
  if(pUserData)
  {
    QCPFile *pQCPFile = (QCPFile *)pUserData;
    uint32 nRead = 0;
    nRead = pQCPFile->FileGetData(ullOffset,
                                  ulNumBytesRequest,
                                  ulMaxBufSize,
                                  pucData);
    return(nRead);
  }
  return 0;
}


/* ======================================================================
FUNCTION:
  AVIFile::FileGetData

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
uint32 QCPFile::FileGetData(uint64 nOffset,
                            uint32 nNumBytesRequest,
                            uint32 nMaxSize,
                            uint8 *pData  )
{
  uint32 nRead = 0;
  if (m_QCPFilePtr != NULL)
  {
    if(!m_bMediaAbort)
    {
      nRead = FileBase::readFile(m_QCPFilePtr, pData, nOffset, FILESOURCE_MIN(nNumBytesRequest,nMaxSize));
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
  QCPFile::ParseQCPHeader

DESCRIPTION
  creates instnace of QCPparser and calls start on parsing QCP file

DEPENDENCIES
  None

INPUT PARAMETERS:
->
->

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
QCPFile::QCPFile()
{
  InitData();
}

QCPFile:: QCPFile(const FILESOURCE_STRING &filename,unsigned char *pFileBuf,
                  uint64 bufSize)
{
  InitData();
  if (pFileBuf && bufSize)
  {
    m_pFileBuf = pFileBuf;
    m_FileBufSize = bufSize;
    m_QCPFilePtr = OSCL_FileOpen (pFileBuf, bufSize);
    m_fileSize = bufSize;
  }
  else
  {
     m_filename = filename;
//#ifdef FEATURE_FILESOURCE_FILE_IO_CACHING
#if 0 //Disabling as the buffer siz has to be decided for caching
     /* Calling with 10K cache  buffer size */
     m_QCPFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"),
                                  FILE_READ_BUFFER_SIZE_FOR_QCP );
#else
     m_QCPFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"),
                                  QCP_CACHE_SIZE);
#endif
     m_fileSize = OSCL_FileSize( m_filename );

  }
  if(m_QCPFilePtr != NULL)
  {
    if(ParseQCPHeader())
    {
      _fileErrorCode = PARSER_ErrorNone;
      _success = true;
    }
  }
}

QCPFile::QCPFile(IxStream* pixstream)
{
  InitData();
#ifdef FEATURE_FILESOURCE_DRM_DCF
  m_pIxStream = pixstream;
  m_QCPFilePtr = OSCL_FileOpen(m_pIxStream);
  if(m_QCPFilePtr != NULL)
  {
    if(m_pIxStream)
    {
      (void)m_pIxStream->Size((uint32*)&m_fileSize);
    }
    if(ParseQCPHeader())
    {
      _fileErrorCode = PARSER_ErrorNone;
      _success = true;
    }
  }
#else
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
    "QCPFile::QCPFile (ixstream not implemented) %p", pixstream);
#endif
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
QCPFile::QCPFile(video::iStreamPort* pport)
{
  InitData();
  m_pPort = pport;
  m_QCPFilePtr = OSCL_FileOpen(pport);
  if(m_QCPFilePtr != NULL)
  {
    if(m_pPort)
    {
      int64 noffset = 0;
      if(video::iStreamPort::DS_SUCCESS == m_pPort->GetContentLength(&noffset))
      {
        m_fileSize = (uint64)noffset;
      }
    }
    if(ParseQCPHeader())
    {
      _fileErrorCode = PARSER_ErrorNone;
      _success = true;
    }
  }
}
#endif

/*
* Initialize the class members to the default values.
*/
void QCPFile::InitData()
{
  memset(&m_audsampleinfo,0,sizeof(m_audsampleinfo));
  m_SEEK_DONE = false;
  m_uSeektime = 0;
  _fileErrorCode = PARSER_ErrorDefault;
  _success = false;
  m_bMediaAbort = false;
  m_pIxStream = NULL;
  m_filename = NULL;
  m_QCPFilePtr = NULL;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  m_pPort = NULL;
#endif
  m_fileSize = 0;
  m_pFileBuf = NULL;
  m_FileBufSize = 0;
  m_bStreaming = FALSE;
  m_pqcpParser = NULL;
}

void QCPFile::SetCriticalSection(MM_HANDLE pcriticalsection)
{
  if(m_QCPFilePtr)
  {
    m_QCPFilePtr->pCriticalSection = pcriticalsection;
  }
}
QCPFile::~QCPFile()
{

  if(m_QCPFilePtr!=NULL)
  {
     OSCL_FileClose(m_QCPFilePtr);
     m_QCPFilePtr = NULL;
  }

  if(m_pqcpParser)
  {
     MM_Delete( m_pqcpParser);
     m_pqcpParser = NULL;
  }

}
/*===========================================================================

FUNCTION
  QCPFile::ParseQCPHeader

DESCRIPTION
  creates instnace of QCPparser and calls start on parsing QCP file

DEPENDENCIES
  None

INPUT PARAMETERS:
->
->

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/

bool QCPFile::ParseQCPHeader()
{

  bool ret = false;

  m_pqcpParser = MM_New_Args(qcpParser,(this,m_fileSize,m_QCPFilePtr));

  if(m_pqcpParser)
  {
    if( PARSER_ErrorNone != (m_pqcpParser->StartParsing()) )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "QCPFile::ParseQCPHeadera failed..retError ");
      ret = false;
    }
    else
    {
      ret = true;
    }
  }
  return ret;
}

/* ======================================================================
FUNCTION   : getNextMediaSample
DESCRIPTION: gets next sample of the given track.

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
PARSER_ERRORTYPE QCPFile::getNextMediaSample(uint32 /* ulTrackID */, uint8 *pucDataBuf,
                                             uint32 *pulBufSize, uint32 & /*rulIndex*/)
{
  PARSER_ERRORTYPE retStatus = PARSER_ErrorDefault;
  uint32 nOutDataSize = 0;

  /* Validate input params and class variables */
  if(NULL == m_pqcpParser || NULL == pulBufSize || NULL == pucDataBuf ||
     0 == *pulBufSize)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
               "getNextMediaSample invalid input params!!");
    return PARSER_ErrorInvalidParam;
  }

  qcp_audio_info sqcp_audio_info;

  nOutDataSize = *pulBufSize;
  retStatus = m_pqcpParser->GetCurrentSample(pucDataBuf, *pulBufSize, &nOutDataSize);
  if ( PARSER_ErrorNone == retStatus )
  {
    if ( PARSER_ErrorNone != (m_pqcpParser->GetAudioInfo(&sqcp_audio_info)) )
    {
      retStatus = PARSER_ErrorEndOfFile;
      nOutDataSize = 0;
    }
  }
  //check if parser is configured to output on frame boundary
  bool bframe = false;
  (void)m_pqcpParser->GetAudioOutputMode(&bframe,FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME);
  if(bframe)
  {
    //Each frame will have valid timestamp
    if(m_audsampleinfo.delta == 0)
    {
      //set the delta only once
      m_audsampleinfo.delta = m_pqcpParser->GetCurrentTime() -
                                m_audsampleinfo.time;
    }
    m_audsampleinfo.btimevalid = true;
    m_audsampleinfo.time = m_pqcpParser->GetCurrentTime();
  }
  // if seek operation performed then first frame will start with valid time i.e. 0ms
  if(m_SEEK_DONE == true)
  {
    m_audsampleinfo.btimevalid = true;
  }
  else if ( ( m_SEEK_DONE == false ) && !bframe )
  {
    // if audio output mode is not single frame then mark btimevalid to false
    // for all other sample
    m_audsampleinfo.btimevalid = false;
  }
  m_SEEK_DONE = false;

  *pulBufSize = nOutDataSize;
  return retStatus;
}

/* ======================================================================
FUNCTION:
  QCPFile::getTrackDecoderSpecificInfoContent

DESCRIPTION:
  gets information about current sample of a track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE QCPFile::getTrackDecoderSpecificInfoContent(uint32 /*id*/, uint8* buf, uint32 *pbufSize)
{
  qcp_decode_info sqcp_decode_info;
  PARSER_ERRORTYPE ret = PARSER_ErrorDefault;

  if(m_pqcpParser)
  {
    if(m_pqcpParser->GetQCPDecodeInfo(&sqcp_decode_info)==PARSER_ErrorNone)
    {
      uint8 *p_qcp_decode_info =  reinterpret_cast<uint8 *>(&sqcp_decode_info);
      if(buf != NULL)
      {
        (void)memcpy(buf,p_qcp_decode_info,sizeof(sqcp_decode_info));
      }
      *pbufSize = (uint32)sizeof(sqcp_decode_info);
      ret = PARSER_ErrorNone;
    }
  }

  return ret;
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
uint64 QCPFile::getMediaTimestampForCurrentSample(uint32 /*id*/)
{
  uint64 nTimeStamp = 0;
  if(m_pqcpParser)
  {
    nTimeStamp = m_audsampleinfo.time;
  }
  return nTimeStamp;
}


/* ======================================================================
FUNCTION:
  QCPFile::resetPlayback

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

uint64 QCPFile::resetPlayback(  uint64 repos_time, uint32 /*id*/, bool /*bSetToSyncSample*/,
                                bool* /*bError*/, uint64 /*currentPosTimeStamp*/)
{
   if(m_pqcpParser)
   {
      m_uSeektime =m_pqcpParser->Seek(repos_time);
      if(!m_uSeektime)
      {
         /*MM_MSG_PRIO3(MM_FILE_OPS,
                  MM_PRIO_HIGH,
                  " QCPFile::resetPlayback %ld nSyncSamplesToSkip %ld current TS %ld",
                  id,nSyncSamplesToSkip,currentPosTimeStamp);*/
         m_pqcpParser->init_file_position();
      }
    m_audsampleinfo.time = m_uSeektime;
    m_SEEK_DONE = true;
    _fileErrorCode = PARSER_ErrorNone;
   }
   return m_uSeektime;
}
/* ======================================================================
FUNCTION:
  QCPFile::randomAccessDenied

DESCRIPTION:
  Returns non zero is seek is not supported else returns 0(seek is supported)
INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none
SIDE EFFECTS:
  None.
======================================================================*/
uint8 QCPFile::randomAccessDenied()
{
  uint8 nSeekDenied = 1;
  if(m_pqcpParser)
  {
    nSeekDenied = m_pqcpParser->RandomAccessDenied();
  }
  return nSeekDenied;
}
/* ======================================================================
FUNCTION:
  QCPFile::getMovieDuration

DESCRIPTION:
  gets movie duration in movie timescale unit.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 QCPFile::getMovieDuration() const
{
  uint64 nDuration = 0;
  if( m_pqcpParser )
  {
    nDuration = m_pqcpParser->GetClipDurationInMsec();
  }
  return nDuration;
}


/* ======================================================================
FUNCTION:
  QCPFile::getMovieTimescale

DESCRIPTION:
  gets movie timescale

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 QCPFile::getMovieTimescale() const
{
  uint32 uTime = 0;

  if(m_pqcpParser)
  {
    uTime = QCP_STREAM_TIME_SCALE;
  }
  return uTime;
}


/* ======================================================================
FUNCTION:
  QCPFile::getTrackWholeIDList

DESCRIPTION:
  gets list of track IDs

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 QCPFile::getTrackWholeIDList(uint32 *ids)
{
  uint32 nTracks = 0;
  if((m_pqcpParser)&&(ids))
  {
    nTracks = AUDIO_QCP_MAX_TRACKS;
    *ids = nTracks;
  }
  return nTracks;
}



/* ======================================================================
FUNCTION:
  QCPFile::getTrackMediaDuration

DESCRIPTION:
  gets track duration in track time scale unit

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 QCPFile::getTrackMediaDuration(uint32 /*id*/)
{
  uint64 nTrackDuration = 0;
  if(m_pqcpParser)
  {
    nTrackDuration = m_pqcpParser->GetClipDurationInMsec();
  }
  return nTrackDuration;
}

/* ======================================================================
FUNCTION:
  QCPFile::getTrackMediaTimescale

DESCRIPTION:
  gets track time scale

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 QCPFile::getTrackMediaTimescale(uint32 /*id*/)
{
  uint32 uTime = 0;
  if(m_pqcpParser)
  {
    uTime = QCP_STREAM_TIME_SCALE;
  }
  return uTime;
}



/* ======================================================================
FUNCTION:
  QCPFile::getTrackAudioSamplingFreq

DESCRIPTION:
  gets audio track's sampling frequency

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 QCPFile::getTrackAudioSamplingFreq(uint32 /*id*/)
{
  qcp_header_qcph sqcp_header_qcph;
  uint32 nSamplingFreq = 0;
  if(m_pqcpParser)
  {
    if(PARSER_ErrorNone == m_pqcpParser->GetQCPHeader(&sqcp_header_qcph))
    {
      nSamplingFreq = sqcp_header_qcph.nSampleRate;
    }
  }
  return nSamplingFreq;
}

/* ======================================================================
FUNCTION:
  QCPFile::GetNumAudioChannels

DESCRIPTION:
  returns number of audio channels

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 QCPFile::GetNumAudioChannels(int /*id*/)
{
  uint32 nChannels = 0;
  qcp_header_qcph sqcp_header_qcph;
  if(m_pqcpParser)
  {
    if(PARSER_ErrorNone == m_pqcpParser->GetQCPHeader(&sqcp_header_qcph))
    {
      nChannels = sqcp_header_qcph.nChannels;
    }
  }
  return nChannels;
}

/* ======================================================================
FUNCTION:
 QCPFile::getTrackOTIType

DESCRIPTION:
  gets format sub type

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/

uint8  QCPFile::getTrackOTIType(uint32 /*id*/)
{
  uint8 t_subtype;
  if(m_pqcpParser->get_qcp_subtype())
  {
    t_subtype = (uint8) EVRC_AUDIO;// Based on OTI value
  }
  else
  {
    t_subtype = (uint8) QCP_QLCM_AUDIO;// Based on OTI value
  }
  return t_subtype;
}


/* ======================================================================
FUNCTION:
 QCPFile::getTrackMaxBufferSizeDB

DESCRIPTION:
  gets maximum buffer size to play the track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
int32 QCPFile::getTrackMaxBufferSizeDB(uint32 /*id*/)
{
  qcp_audio_info audioInfo;
  uint32 bufferSize = 0;

  if(m_pqcpParser)
  {
    if( PARSER_ErrorNone ==(m_pqcpParser->GetAudioInfo(&audioInfo)) )
    {
      bufferSize = audioInfo.dwSuggestedBufferSize;
    }
    else
    {
      bufferSize = QCP_DEFAULT_AUDIO_BUF_SIZE;
    }
  }
  return bufferSize;
}

/* ======================================================================
FUNCTION:
  AVIFile::peekCurSample

DESCRIPTION:
  gets information about current sample of a track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE QCPFile::peekCurSample(uint32 /*trackid*/,
                                        file_sample_info_type *pSampleInfo)
{
  PARSER_ERRORTYPE ret = PARSER_ErrorDefault;
  if(m_pqcpParser)
  {
    *pSampleInfo = m_audsampleinfo;
    ret = PARSER_ErrorNone;
  }
  return ret;
}
/* ======================================================================
FUNCTION:
  SetAudioOutputMode

DESCRIPTION:
  Called by user to set output mode specified by henum

INPUT/OUTPUT PARAMETERS:
  henum-Output mode

RETURN VALUE:
 FILE_SOURCE_SUCCESS if successful in setting output mode else returns FILE_SOURCE_FAIL

SIDE EFFECTS:
  None.
======================================================================*/
FileSourceStatus QCPFile::SetAudioOutputMode(FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pqcpParser)
  {
    status = m_pqcpParser->SetAudioOutputMode(henum);
  }
  return status;
}
/* ======================================================================
FUNCTION:
  GetAudioOutputMode

DESCRIPTION:
  Called by user to to check if henum passed in is was set earlier

INPUT/OUTPUT PARAMETERS:
  henum-Output mode

RETURN VALUE:
 FILE_SOURCE_SUCCESS if successful in retrieving the mode else returns FILE_SOURCE_FAIL

SIDE EFFECTS:
  None.
======================================================================*/
FileSourceStatus QCPFile::GetAudioOutputMode(bool* bret,FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pqcpParser)
  {
    status = m_pqcpParser->GetAudioOutputMode(bret,henum);
  }
  return status;
}

