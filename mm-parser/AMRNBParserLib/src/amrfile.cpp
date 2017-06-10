/* =======================================================================
  amrfile.cpp
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

  Copyright (c) 2009-2014 QUALCOMM Technologies Inc, All Rights Reserved.
  QUALCOMM Technologies Proprietary and Confidential.

========================================================================== */

/* =======================================================================
Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AMRNBParserLib/main/latest/src/amrfile.cpp#37 $
$DateTime: 2014/02/07 02:53:30 $
$Change: 5240686 $

========================================================================== */

/* ==========================================================================

INCLUDE FILES FOR MODULE

========================================================================== */
#include "parserinternaldefs.h"
#include "parserdatadef.h"
#include "MMDebugMsg.h"
#include <stdio.h>

#include "amrfile.h"
#include "amrparser.h"
#include "filebase.h"
#include "MMMemory.h"

#ifdef FEATURE_FILESOURCE_DRM_DCF
#include "IxStream.h"
#endif

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
#define QTV_3GPP_MIN_NUM_VIDEO_FRAMES_TO_BUFFER 6
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

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
  AMRCallbakGetData

DESCRIPTION
  Its a callback function from the SDK to get the AMR-NB data for decoding.
  This is not implemented by the SDK. It should be implemented by the app
  that calls the SDK.

ARGUMENTS
  ullOffset           Offset of the requested data (from beginning),
  ulNumBytesRequest   Size of the requested data (in bytes).
  pucData             Pointer to data buffer.
  ulMaxBufSize        Maximum data buffer size.
  pUserData           Client user data information.

DEPENDENCIES
  Used by AMR-NB Parser.

RETURN VALUE
  The number of bytes actually being filled in

SIDE EFFECTS
  Detail any side effects.
========================================================================== */

uint32 AMRCallbakGetData (uint64 ullOffset,
                          uint32 ulNumBytesRequest,
                          uint8* pucData,
                          uint32 ulMaxBufSize,
                          void*  pUserData )
{
  if(pUserData)
  {
    AMRFile *pAMRFile = (AMRFile *)pUserData;
    return ( pAMRFile->FileGetData(ullOffset,
                                   ulNumBytesRequest,
                                   ulMaxBufSize,
                                   pucData ));
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
uint32 AMRFile::FileGetData(  uint64 nOffset,
                              uint32 nNumBytesRequest,
                              uint32 nMaxSize,
                              uint8 *pData  )
{
  uint32 nRead = 0;
  if (m_AMRFilePtr != NULL)
  {
    if(!m_bMediaAbort)
    {
      nRead = FileBase::readFile(m_AMRFilePtr, pData, nOffset,
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
  AMRFile::AMRFile

DESCRIPTION
  default constructer

DEPENDENCIES
  None

INPUT PARAMETERS:
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
AMRFile:: AMRFile()
{
  InitData();
}
/*===========================================================================

FUNCTION
  AMRFile::AMRFile

DESCRIPTION
  creates instnace of AMRparser and calls start on parsing AMR file

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

AMRFile:: AMRFile(const FILESOURCE_STRING &filename,
                  unsigned char *pFileBuf, uint64 bufSize)
{
  InitData();
  if (pFileBuf && bufSize)
  {
    m_pFileBuf = pFileBuf;
    m_FileBufSize = bufSize;
    m_AMRFilePtr = OSCL_FileOpen (pFileBuf, bufSize);
    m_fileSize = OSCL_FileSize( m_filename );
  }
  else
  {
     m_filename = filename;
//#ifdef FEATURE_FILESOURCE_FILE_IO_CACHING
#if 0 //Disabling as the buffer siz has to be decided for caching
     /* Calling with 10K cache  buffer size */
     m_AMRFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"),
                                  FILE_READ_BUFFER_SIZE_FOR_AMR );
#else
     m_AMRFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"),
                                  AMR_CACHE_SIZE);
#endif
     m_fileSize = OSCL_FileSize( m_filename );

  }
  if(ParseAMRHeader())
  {
    _fileErrorCode = PARSER_ErrorNone;
    _success = true;
  }
}

AMRFile::AMRFile(IxStream* pixstream)
{
  InitData();
#ifdef FEATURE_FILESOURCE_DRM_DCF
  m_pIxStream = pixstream;
  m_AMRFilePtr = OSCL_FileOpen(m_pIxStream);
  if(m_AMRFilePtr != NULL)
  {
    if(m_pIxStream)
    {
      (void)m_pIxStream->Size(&m_fileSize);
    }
    if(ParseAMRHeader())
    {
      _fileErrorCode = PARSER_ErrorNone;
      _success = true;
    }
  }
#else
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
     "AMRBFile::AMRBFile (ixstream not implemented) %p", pixstream);
#endif
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
AMRFile::AMRFile(video::iStreamPort* pport)
{
  InitData();
  m_pPort = pport;
  m_AMRFilePtr = OSCL_FileOpen(pport);
  if(m_AMRFilePtr != NULL)
  {
    if(m_pPort)
    {
      int64 noffset = 0;
      if(video::iStreamPort::DS_SUCCESS == m_pPort->GetContentLength(&noffset))
      {
        m_fileSize = (uint64)noffset;
      }
    }
    if(ParseAMRHeader())
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
void AMRFile::InitData()
{
  memset(&m_audsampleinfo,0,sizeof(m_audsampleinfo));
  m_SEEK_DONE = false;
  m_uSeektime = 0;
  _fileErrorCode = PARSER_ErrorDefault;
  _success = false;
  m_bMediaAbort = false;
  m_pIxStream = NULL;
  m_filename = NULL;
  m_AMRFilePtr = NULL;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  m_pPort = NULL;
#endif
  m_fileSize = 0;
  m_pFileBuf = NULL;
  m_FileBufSize = 0;
  m_bStreaming = FALSE;
  m_pamrParser = NULL;
}
void AMRFile::SetCriticalSection(MM_HANDLE pcriticalsection)
{
  if(m_AMRFilePtr)
  {
    m_AMRFilePtr->pCriticalSection = pcriticalsection;
  }
}

AMRFile::~AMRFile()
{

  if(m_AMRFilePtr!=NULL)
  {
     OSCL_FileClose(m_AMRFilePtr);
     m_AMRFilePtr = NULL;
  }

  if(m_pamrParser)
  {
     MM_Delete( m_pamrParser);
     m_pamrParser = NULL;
  }


}
/*===========================================================================

FUNCTION
  AMRFile::ParseAMRHeader

DESCRIPTION
  creates instnace of AMRparser and calls start on parsing AMR file

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

bool AMRFile::ParseAMRHeader()
{
  PARSER_ERRORTYPE retError;
  bool ret;

  m_pamrParser = MM_New_Args(amrParser,(this,m_fileSize,m_AMRFilePtr));

  if(m_pamrParser)
  {
    if( (retError = m_pamrParser->StartParsing()) != PARSER_ErrorNone)
    {

      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                  "AMRFile::ParseAMRHeader failed..retError %d",retError);

      ret = false;

      return ret;
    }
   }
  else
  {
     ret = false;

     return ret;
  }

  ret = true;

  return ret;
}

/* ======================================================================
FUNCTION:
  AMRFile::getNextMediaSample

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
PARSER_ERRORTYPE AMRFile::getNextMediaSample(uint32 /*ulTrackID*/,
                                             uint8*  pucDataBuf,
                                             uint32* pulBufSize,
                                             uint32& /*rulIndex*/)
{
  PARSER_ERRORTYPE retStatus = PARSER_ErrorDefault;
  uint32 nOutDataSize = 0;
  if(NULL == pucDataBuf || NULL == m_pamrParser || NULL == pulBufSize ||
     0 == *pulBufSize)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
               "getNextMediaSample invalid input params!!");
    return PARSER_ErrorInvalidParam;
  }

  nOutDataSize = *pulBufSize;
  retStatus = m_pamrParser->GetCurrentSample(pucDataBuf, *pulBufSize,
                                             &nOutDataSize);
  if ( PARSER_ErrorNone == retStatus)
  {
    bool bModeStatus = false;

    (void)m_pamrParser->GetAudioOutputMode(&bModeStatus,
      FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME);
    if( bModeStatus )
    {
      if( 0 == m_audsampleinfo.delta)
      {
        m_audsampleinfo.delta = \
          m_pamrParser->GetCurrentPbTime()- m_audsampleinfo.time;
      }
      m_audsampleinfo.btimevalid = true;
      m_audsampleinfo.time = m_pamrParser->GetCurrentPbTime();

    }
  }
  if(m_SEEK_DONE == true)
  {
    m_audsampleinfo.btimevalid = true;
  }
  else
  {
    m_audsampleinfo.btimevalid = false;
  }
  m_SEEK_DONE = false;
  *pulBufSize = nOutDataSize;
  return retStatus;
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
uint64 AMRFile::getMediaTimestampForCurrentSample(uint32 /*id*/)
{
  uint64 nTimeStamp = 0;
  if(m_pamrParser)
  {
    nTimeStamp = m_audsampleinfo.time;
  }
  return nTimeStamp;
}

/* ======================================================================
FUNCTION:
  AMRFile::resetPlayback

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

uint64 AMRFile::resetPlayback(  uint64 repos_time, uint32 id,
                                bool /*bSetToSyncSample*/,
                                bool *bError, uint64 currentPosTimeStamp)
{
  if(m_pamrParser)
  {
    m_uSeektime =m_pamrParser->Seek(repos_time);
    if(!m_uSeektime)
    {
      MM_MSG_PRIO3(
        MM_FILE_OPS,
        MM_PRIO_MEDIUM,
        " AMRFile::resetPlayback %ld repos_time %llu current TS %llu",
        id,repos_time,currentPosTimeStamp);

      m_pamrParser->init_file_position();
    }
    m_audsampleinfo.time = m_uSeektime;
    m_SEEK_DONE = true;
    *bError	= FALSE;
    _fileErrorCode = PARSER_ErrorNone;
  }
  return m_uSeektime;
}

/* ======================================================================
FUNCTION:
  AMRFile::getMovieDuration

DESCRIPTION:
  gets movie duration in movie timescale unit.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 AMRFile::getMovieDuration() const
{
  uint64 nDuration = 0;

  if(!m_pamrParser)
  {
    return 0;
  }

  nDuration = m_pamrParser->GetClipDurationInMsec();

  return nDuration;
}

/* ======================================================================
FUNCTION:
  AMRFile::getTrackWholeIDList

DESCRIPTION:
  gets list of track IDs

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AMRFile::getTrackWholeIDList(uint32 *ids)
{
  int32 nTracks = 0;

  if((!m_pamrParser)||(!ids))
  {
    return 0;
  }

  nTracks = AUDIO_AMR_MAX_TRACKS;

  *ids = nTracks;

  return nTracks;

}

/* ======================================================================
FUNCTION:
  AMRFile::getTrackAudioSamplingFreq

DESCRIPTION:
  gets audio track's sampling frequency

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AMRFile::getTrackAudioSamplingFreq(uint32 /*id*/)
{

  amr_header_amrh samr_header_amrh;
  uint32 nSamplingFreq = 0;
  if(!m_pamrParser)
  {
    return nSamplingFreq;
  }

  if(m_pamrParser->GetAMRHeader(&samr_header_amrh)==PARSER_ErrorNone)
  {
    nSamplingFreq = samr_header_amrh.nSampleRate;
  }
  return nSamplingFreq;
}

/* ======================================================================
FUNCTION:
  AMRFile::GetNumAudioChannels

DESCRIPTION:
  returns number of audio channels

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AMRFile::GetNumAudioChannels(int /*id*/)
{
  uint32 nChannels = 0;
  amr_header_amrh samr_header_amrh;

  if((m_pamrParser) &&
     (m_pamrParser->GetAMRHeader(&samr_header_amrh)==PARSER_ErrorNone) )
  {
    nChannels = samr_header_amrh.nChannels;
  }

  return nChannels;
}

/* ======================================================================
FUNCTION:
 AMRFile::getTrackMaxBufferSizeDB

DESCRIPTION:
  gets maximum buffer size to play the track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
int32 AMRFile::getTrackMaxBufferSizeDB(uint32 /*id*/)
{
  amr_audio_info audioInfo;
  int32 bufferSize = AMR_DEFAULT_AUDIO_BUF_SIZE;

  if(!m_pamrParser)
  {
    return 0;
  }

  if((m_pamrParser->GetAudioInfo(&audioInfo))== PARSER_ErrorNone)
   {
     bufferSize = audioInfo.dwSuggestedBufferSize;
   }
  else
   {
     bufferSize = AMR_DEFAULT_AUDIO_BUF_SIZE;
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
PARSER_ERRORTYPE AMRFile::peekCurSample(uint32 /*trackid*/,
                                        file_sample_info_type *pSampleInfo)
{
  if(!m_pamrParser)
  {
    return PARSER_ErrorDefault;
  }
  else
  {
    *pSampleInfo = m_audsampleinfo;
    return PARSER_ErrorNone;
  }
}

/* ======================================================================
  FUNCTION:
    SetAudioOutputMode

  DESCRIPTION:
    Called by user to set output mode specified by eConfigParam

  INPUT/OUTPUT PARAMETERS:
    eConfigParam-Output mode

  RETURN VALUE:
   FILE_SOURCE_SUCCESS if successful in setting output mode
   else returns FILE_SOURCE_FAIL

  SIDE EFFECTS:
    None.
======================================================================*/
FileSourceStatus AMRFile::SetAudioOutputMode(
                          FileSourceConfigItemEnum eConfigParam)
{
  FileSourceStatus eStatus = FILE_SOURCE_FAIL;
  if( ( NULL != m_pamrParser )&&
      ( PARSER_ErrorNone == m_pamrParser->SetAudioOutputMode(eConfigParam) ) )
  {
    eStatus = FILE_SOURCE_SUCCESS;
  }
  return eStatus;
}

/* ======================================================================
  FUNCTION:
    GetAudioOutputMode

  DESCRIPTION:
    Called by user to query output mode specified by eConfigParam

  INPUT/OUTPUT PARAMETERS:
    pbConfigStatus- Output mode query status
    eConfigParam-Output mode to query

  RETURN VALUE:
   FILE_SOURCE_SUCCESS if successful in retrieving output mode
   else returns FILE_SOURCE_FAIL

  SIDE EFFECTS:
    None.
======================================================================*/
FileSourceStatus AMRFile::GetAudioOutputMode(
                            bool* pbConfigStatus,
                            FileSourceConfigItemEnum eConfigParam)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if( ( NULL != m_pamrParser)&&
      ( PARSER_ErrorNone == \
        m_pamrParser->GetAudioOutputMode(pbConfigStatus,eConfigParam ) ) )
  {
    status = FILE_SOURCE_SUCCESS;
  }
  return status;
}
