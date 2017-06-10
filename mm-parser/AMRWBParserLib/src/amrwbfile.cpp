/* =======================================================================
amrwbfile.cpp
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

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AMRWBParserLib/main/latest/src/amrwbfile.cpp#35 $
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

#include "amrwbfile.h"
#include "amrwbparser.h"
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
  AMRWBCallbakGetData

DESCRIPTION
  Its a callback function from the SDK to get the AMR-WB data for decoding.
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
uint32 AMRWBCallbakGetData ( uint64 ullOffset,
                             uint32 ulNumBytesRequest,
                             uint8* pucData,
                             uint32 ulMaxBufSize,
                             void*  pUserData )
{
  if(pUserData)
  {
    AMRWBFile *pAMRWBFile = (AMRWBFile *)pUserData;
    return( pAMRWBFile->FileGetData(ullOffset, ulNumBytesRequest,
                                    ulMaxBufSize, pucData ) );
  }
  return 0;
}

/* ======================================================================
  FUNCTION:
    AMRWBFile::FileGetData

  DESCRIPTION:
    To read the data from the file
  INPUT/OUTPUT PARAMETERS:
    ullOffset         : Offset from which data is being requested
    ulNumBytesRequest : Total number of bytes requested
    ulMaxSize         : Maximum buffer size
    pucData           : Buffer to be used for reading the data


  RETURN VALUE:
   Number of bytes read

  SIDE EFFECTS:
    None.
======================================================================*/
uint32 AMRWBFile::FileGetData(uint64 ullOffset,
                              uint32 ulNumBytesRequest,
                              uint32 ulMaxSize,
                              uint8 *pucData  )
{
  uint32 ulReadBytes = 0;
  if (m_AMRWBFilePtr != NULL)
  {
    if(!m_bMediaAbort)
    {
      ulReadBytes = \
        FileBase::readFile( m_AMRWBFilePtr,
                            pucData,
                            ullOffset,
                            FILESOURCE_MIN(ulNumBytesRequest,ulMaxSize));
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Breaking, user Abort is true.");
    }
  }
  return ulReadBytes;
}

/*===========================================================================

  FUNCTION
    AMRWBFile::AMRWBFile

  DESCRIPTION
     Default constructor

  DEPENDENCIES
    None

  INPUT PARAMETERS:
    None

  RETURN VALUE
    None

  SIDE EFFECTS
    None
===========================================================================*/
AMRWBFile::AMRWBFile()
{
  InitData();
}

/*===========================================================================

  FUNCTION
    AMRWBFile::AMRWBFile

  DESCRIPTION
    Creates instance of AMRWBparser and calls start on parsing AMRWB file

  DEPENDENCIES
    None

  INPUT PARAMETERS:
    None
  RETURN VALUE
    None

  SIDE EFFECTS
    None
===========================================================================*/
AMRWBFile:: AMRWBFile(const FILESOURCE_STRING &filename,
                      unsigned char *pFileBuf,
                      uint64 bufSize)
{
  InitData();
  if (pFileBuf && bufSize)
  {
    m_pFileBuf = pFileBuf;
    m_FileBufSize = bufSize;
    m_AMRWBFilePtr = OSCL_FileOpen (pFileBuf, bufSize);
    m_fileSize = OSCL_FileSize( m_filename );
  }
  else
  {
     m_filename = filename;
//#ifdef FEATURE_FILESOURCE_FILE_IO_CACHING
#if 0 //Disabling as the buffer siz has to be decided for caching
     /* Calling with 10K cache  buffer size */
     m_AMRWBFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"), FILE_READ_BUFFER_SIZE_FOR_AMRWB );
#else
     m_AMRWBFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"),AMRWB_CACHE_SIZE);
#endif
     m_fileSize = OSCL_FileSize( m_filename );
  }

  if(m_AMRWBFilePtr != NULL)
  {
    if(ParseAMRWBHeader())
    {
      _fileErrorCode = PARSER_ErrorNone;
      _success = true;
    }
  }
}
AMRWBFile::AMRWBFile(IxStream* pixstream)
{
  InitData();
#ifdef FEATURE_FILESOURCE_DRM_DCF
  m_pIxStream = pixstream;
  m_AMRWBFilePtr = OSCL_FileOpen(m_pIxStream);
  if(m_AMRWBFilePtr != NULL)
  {
    if(m_pIxStream)
    {
      (void)m_pIxStream->Size(&m_fileSize);
    }
    if(ParseAMRWBHeader())
    {
      _fileErrorCode = PARSER_ErrorNone;
      _success = true;
    }
  }
#else
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
              "AMRWBFile::AMRWBFile (ixstream not implemented) %p", pixstream);
#endif
}

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
AMRWBFile::AMRWBFile(video::iStreamPort* pport)
{
  InitData();
  m_pPort = pport;
  m_AMRWBFilePtr = OSCL_FileOpen(pport);
  if(m_AMRWBFilePtr != NULL)
  {
    if(m_pPort)
    {
      int64 noffset = 0;
      if(video::iStreamPort::DS_SUCCESS == m_pPort->GetContentLength(&noffset))
      {
        m_fileSize = (uint64)noffset;
      }
    }
    if(ParseAMRWBHeader())
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
void AMRWBFile::InitData()
{
  memset(&m_audsampleinfo,0,sizeof(m_audsampleinfo));
  m_SEEK_DONE = false;
  m_uSeektime = 0;
  _fileErrorCode = PARSER_ErrorDefault;
  _success = false;
  m_bMediaAbort = false;
  m_pIxStream = NULL;
  m_filename = NULL;
  m_AMRWBFilePtr = NULL;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  m_pPort = NULL;
#endif
  m_fileSize = 0;
  m_pFileBuf = NULL;
  m_FileBufSize = 0;
  m_bStreaming = FALSE;
  m_pamrwbParser = NULL;
}

void AMRWBFile::SetCriticalSection(MM_HANDLE pcriticalsection)
{
  if(m_AMRWBFilePtr)
  {
    m_AMRWBFilePtr->pCriticalSection = pcriticalsection;
  }
}

AMRWBFile::~AMRWBFile()
{

  if(m_AMRWBFilePtr!=NULL)
  {
     OSCL_FileClose(m_AMRWBFilePtr);
     m_AMRWBFilePtr = NULL;
  }

  if(m_pamrwbParser)
  {
     MM_Delete( m_pamrwbParser);
     m_pamrwbParser = NULL;
  }


}
/*===========================================================================

  FUNCTION
    AMRWBFile::ParseAMRWBHeader

  DESCRIPTION
    creates instance of AMRWBparser and calls start on parsing AMRWB file

  DEPENDENCIES
    None

  INPUT PARAMETERS:
    None

  RETURN VALUE
    None

  SIDE EFFECTS
    None
===========================================================================*/

bool AMRWBFile::ParseAMRWBHeader()
{
  bool ret = false;

  m_pamrwbParser = MM_New_Args(amrwbParser,(this,m_fileSize,m_AMRWBFilePtr));

  if(NULL != m_pamrwbParser)
  {
    if( PARSER_ErrorNone != ( m_pamrwbParser->StartParsing()) )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "AMRWBFile::ParseAMRWBHeadera failed..retError ");
      ret = false;
    }
    ret = true;
  }
  return ret;
}

/* ======================================================================
  FUNCTION:
    AMRWBFile::getNextMediaSample

  DESCRIPTION:
    Gets next sample of the given track.

  INPUT/OUTPUT PARAMETERS:
  @param[in]      ulTrackID  TrackID requested
  @param[in]      pucDataBuf DataBuffer pointer to fill the frame(s)
  @param[in/out]  pulBufSize Amount of data request amount of data filled in Buffer
  @param[in]      rulIndex   Index

  RETURN VALUE:
   PARSER_ErrorNone in Successful case /
   Corresponding error code in failure cases

  SIDE EFFECTS:
    None.
======================================================================*/
PARSER_ERRORTYPE AMRWBFile::getNextMediaSample(uint32 /*ulTrackID*/,
                                               uint8*  pucDataBuf,
                                               uint32* pulBufSize,
                                               uint32& /*rulIndex*/)
{
  PARSER_ERRORTYPE retStatus = PARSER_ErrorDefault;
  uint32 nOutDataSize = 0;

  /* Validate input params and class variables */
  if(NULL == m_pamrwbParser || NULL == pulBufSize || NULL == pucDataBuf ||
     0 == *pulBufSize)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
               "AMRWBFile::getNextMediaSample invalid input params!!");
    return PARSER_ErrorInvalidParam;
  }

  nOutDataSize = *pulBufSize;
  retStatus = m_pamrwbParser->GetCurrentSample(pucDataBuf,*pulBufSize,&nOutDataSize);
  if ( PARSER_ErrorNone == retStatus)
  {
    bool bModeStatus = false;
    (void)m_pamrwbParser->GetAudioOutputMode(&bModeStatus,
                                  FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME);
    if( bModeStatus )
    {
      if( 0 == m_audsampleinfo.delta)
      {
        m_audsampleinfo.delta = \
          m_pamrwbParser->GetCurrentPbTime()- m_audsampleinfo.time;
      }
      m_audsampleinfo.btimevalid = true;
      m_audsampleinfo.time = m_pamrwbParser->GetCurrentPbTime();
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
uint64 AMRWBFile::getMediaTimestampForCurrentSample(uint32 /*id*/)
{
  uint64 nTimeStamp = 0;
  if(m_pamrwbParser)
  {
    nTimeStamp = m_audsampleinfo.time;
  }
  return nTimeStamp;
}

/* ======================================================================
  FUNCTION:
    AMRWBFile::resetPlayback

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
uint64 AMRWBFile::resetPlayback(uint64 repos_time, uint32 id,
                                bool /*bSetToSyncSample*/,
                                bool *bError, uint64 currentPosTimeStamp)
{

  if(m_pamrwbParser)
  {
    m_uSeektime =m_pamrwbParser->Seek(repos_time);
    if(!(m_uSeektime))
    {
      MM_MSG_PRIO3(
        MM_FILE_OPS,
        MM_PRIO_HIGH,
        " AMRWBFile::resetPlayback %ld repos_time %llu current TS %llu",
        id,repos_time,currentPosTimeStamp);

      m_pamrwbParser->init_file_position();
    }
    m_audsampleinfo.time = m_uSeektime;
    m_SEEK_DONE = true;
    *bError = FALSE;
    _fileErrorCode = PARSER_ErrorNone;
  }
  return m_uSeektime;
}

/* ======================================================================
  FUNCTION:
    AMRWBFile::getMovieDuration

  DESCRIPTION:
    gets movie duration in movie timescale unit.

  INPUT/OUTPUT PARAMETERS:
    None.

  RETURN VALUE:
   none

  SIDE EFFECTS:
    None.
======================================================================*/
uint64 AMRWBFile::getMovieDuration() const
{
  uint64 nDuration = 0;

  if(m_pamrwbParser)
  {
    nDuration = m_pamrwbParser->GetClipDurationInMsec();
  }

  return nDuration;
}

/* ======================================================================
  FUNCTION:
    AMRWBFile::getTrackWholeIDList

  DESCRIPTION:
    gets list of track IDs

  INPUT/OUTPUT PARAMETERS:
    None.

  RETURN VALUE:
   none

  SIDE EFFECTS:
    None.
======================================================================*/
uint32 AMRWBFile::getTrackWholeIDList(uint32 *ids)
{
  uint32 nTracks = 0;

  if((m_pamrwbParser)&&(ids))
  {
    nTracks = AUDIO_AMRWB_MAX_TRACKS;
    *ids = nTracks;
  }
  return nTracks;
}

/* ======================================================================
  FUNCTION:
    AMRWBFile::getTrackAudioSamplingFreq

  DESCRIPTION:
    gets audio track's sampling frequency

  INPUT/OUTPUT PARAMETERS:
    None.

  RETURN VALUE:
   none

  SIDE EFFECTS:
    None.
======================================================================*/
uint32 AMRWBFile::getTrackAudioSamplingFreq(uint32 /*id*/)
{

  amrwb_header_amrwbh samrwb_header_amrwbh;
  uint32 nSamplingFreq = 0;

  if(m_pamrwbParser)
  {
    if(m_pamrwbParser->GetAMRWBHeader(&samrwb_header_amrwbh)==PARSER_ErrorNone)
    {
      nSamplingFreq = samrwb_header_amrwbh.nSampleRate;
    }
  }

  return nSamplingFreq;
}

/* ======================================================================
  FUNCTION:
    AMRWBFile::GetNumAudioChannels

  DESCRIPTION:
    returns number of audio channels

  INPUT/OUTPUT PARAMETERS:
    None.

  RETURN VALUE:
   none

  SIDE EFFECTS:
    None.
======================================================================*/
uint32 AMRWBFile::GetNumAudioChannels(int /*id*/)
{
  uint32 nChannels = 0;
  amrwb_header_amrwbh samrwb_header_amrwbh;

  if(m_pamrwbParser)
  {
    if(m_pamrwbParser->GetAMRWBHeader(&samrwb_header_amrwbh)==PARSER_ErrorNone)
    {
      nChannels = samrwb_header_amrwbh.nChannels;
    }
  }

return nChannels;
}

/* ======================================================================
  FUNCTION:
   AMRWBFile::getTrackMaxBufferSizeDB

  DESCRIPTION:
    gets maximum buffer size to play the track

  INPUT/OUTPUT PARAMETERS:
    None.

  RETURN VALUE:
   none

  SIDE EFFECTS:
    None.
======================================================================*/
int32 AMRWBFile::getTrackMaxBufferSizeDB(uint32 /*id*/)
{
  amrwb_audio_info audioInfo;
  int32 bufferSize = 0;

  if(m_pamrwbParser)
  {
    if((m_pamrwbParser->GetAudioInfo(&audioInfo))== PARSER_ErrorNone)
    {
      bufferSize = audioInfo.dwSuggestedBufferSize;
    }
    else
    {
      bufferSize = AMRWB_DEFAULT_AUDIO_BUF_SIZE;
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
PARSER_ERRORTYPE AMRWBFile::peekCurSample(uint32 /*trackid*/,
                                          file_sample_info_type *pSampleInfo)
{
  PARSER_ERRORTYPE ret = PARSER_ErrorDefault;
  if(m_pamrwbParser)
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
    Called by user to set output mode specified by eConfigParam

  INPUT/OUTPUT PARAMETERS:
    eConfigParam-Output mode

  RETURN VALUE:
   FILE_SOURCE_SUCCESS if successful in setting output mode
   else returns FILE_SOURCE_FAIL

  SIDE EFFECTS:
    None.
======================================================================*/
FileSourceStatus AMRWBFile::SetAudioOutputMode(
                          FileSourceConfigItemEnum eConfigParam)
{
  FileSourceStatus eStatus = FILE_SOURCE_FAIL;
  if( ( NULL != m_pamrwbParser )&&
      ( PARSER_ErrorNone == m_pamrwbParser->SetAudioOutputMode(eConfigParam) ) )
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
FileSourceStatus AMRWBFile::GetAudioOutputMode(
                            bool* pbConfigStatus,
                            FileSourceConfigItemEnum eConfigParam)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if( ( NULL != m_pamrwbParser)&&
      ( PARSER_ErrorNone == \
        m_pamrwbParser->GetAudioOutputMode(pbConfigStatus,eConfigParam ) ) )
  {
    status = FILE_SOURCE_SUCCESS;
  }
  return status;
}

