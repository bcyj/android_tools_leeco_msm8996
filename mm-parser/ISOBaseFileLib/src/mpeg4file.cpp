/* =======================================================================
mpeg4file.cpp
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

Copyright (c) 2008-2014 QUALCOMM Technologies Inc, All Rights Reserved.
QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/mpeg4file.cpp#155 $
$DateTime: 2014/05/13 22:50:58 $
$Change: 5885284 $

========================================================================== */

/* ==========================================================================

INCLUDE FILES FOR MODULE

========================================================================== */

/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "parserdatadef.h"
#include "parserinternaldefs.h"

#include "MMCriticalSection.h"
#include "AEEStdDef.h"    /* Definitions for byte, word, etc.        */
#include "MMDebugMsg.h"

#include <stdio.h>
#include "mpeg4file.h"
#include "ztl.h"
#include "utf8conv.h"

#include "MMCriticalSection.h"

#include "atomdefs.h"

#include "filesourcestring.h"
#include "videofmt_mp4r.h"
#include "MMTimer.h"
#include "zrex_string.h"

/*  Max MDAT atom header size. Typical size is 8 bytes (4Bytes header and size).
    But for version#1 atoms, another 8bytes are required to store Size and first
    4Bytes size field will be equal to "1". */
#define MDAT_ATOM_SIGNATURE 16

//! Max audio Frame buffer size (64kb)
#define MAX_AUDIO_FRAME_SIZE (64*1024)

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
#define QTV_3GPP_MIN_NUM_VIDEO_FRAMES_TO_BUFFER 6
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

#ifdef FEATURE_FILESOURCE_DRM_DCF
#include "IxStream.h"
#include "IxStreamMedia.h"
#endif

#define FRAGMENT_CORRUPT -4
//! Album Art Fixed fields size
#define FIXED_APIC_FIELDS_LENGTH 112
/* ==========================================================================

DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
uint64 AtomUtils::fileSize = 0;

#define ALLOC_MEMORY_HEADER 256
// OMA DRM Encryption Scheme Version
#define OMA_DRM_SCHEME_VERSION 0X0200

//! Marlin DRM System ID
const uint8 MARLIN_DRM_SYSTEMID[MAX_SYSTEMID_SIZE] = { 0x69,0xf9,0x08,0xaf,
                                                       0x48,0x16,0x46,0xea,
                                                       0x91,0x0c,0xcd,0x5d,
                                                       0xcc,0xcb,0x0a,0x3a};

//! PlayReady DRM System ID
const uint8 PLAYREADY_DRM_SYSTEMID[MAX_SYSTEMID_SIZE] = { 0x9a,0x04,0xf0,0x79,
                                                          0x98,0x40,0x42,0x86,
                                                          0xab,0x92,0xe6,0x5b,
                                                          0xe0,0x88,0x5f,0x95};
//! Default text atom header size ( tx3g/stpp)
#define DEFAULT_TEXT_ATOM_HEADER_SIZE 8

//! Get text data type based on sub-stream type
#define GET_TEXT_DATATYPE(_x_) ( ( (_x_) == VIDEO_FMT_STREAM_TEXT_TIMEEDTEXT )\
                                 ? (DATA_ATOM_TX3G) : (DATA_ATOM_STPP) )

#define MAX_SUBTITLE_SAMPLE_SIZE ( 500 * 1024)

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
/*===========================================================================

FUNCTION  Mpeg4File

DESCRIPTION
This is the Mpeg4File class constructor - initializes the class members.
===========================================================================*/
Mpeg4File::Mpeg4File(  FILESOURCE_STRING filename,
                       unsigned char *pFileBuf,
                       uint32 bufSize,
                       bool playVideo,
                       bool playAudio,
                       bool playText
#if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || \
    defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
                       ,bool bHttpStream
                       ,uint32 wBufferOffset
#endif /* defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) */
                     )
{
  // Initialize
  Mpeg4File::InitData();

  m_playAudio = playAudio;
  m_playVideo = playVideo;
  m_playText  = playText;

  m_bAudioPresentInClip = false;
  m_bVideoPresentInClip = false;
  m_bTextPresentInClip  = false;
  m_fileSizeFound       = false;
  if(filename.size() && pFileBuf)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
       "Mpeg4File::Mpeg4File filename and pFileBuf both non NULL");
  }
  if(!(filename.size()) && !pFileBuf)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
       "Mpeg4File::Mpeg4File filename and pFileBuf both NULL");
  }
  m_pFileBuf  = pFileBuf;
  m_fileSize  = bufSize;
  m_filename  = filename;

  if(m_pFileBuf)
  {
    m_parseFilePtr = OSCL_FileOpen(m_pFileBuf, m_fileSize);
  }
  else
  {
    m_parseFilePtr = OSCL_FileOpen(m_filename, (OSCL_TCHAR *) _T("rb"));
    m_fileSize = OSCL_FileSize( m_filename );
  }

  if(!m_parseFilePtr )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Failed to open file");
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
  if(!m_fileSize)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR, "Invalid file size %llu",m_fileSize);
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
  m_fileSizeFound = true;
  AtomUtils::fileSize   = m_fileSize;

#if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
  bHttpStreaming    = bHttpStream;
  m_wBufferOffset   = wBufferOffset;
#endif /* defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) */
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
/* ======================================================================
FUNCTION:
  Mpeg4File::Mpeg4File

DESCRIPTION:
  constructor for supporting playback from StreamPort.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
Mpeg4File::Mpeg4File( video::iStreamPort* pPort,
                      bool bPlayVideo,
                      bool bPlayAudio,
                      bool bPlayText,
                      FileSourceFileFormat eFileFormat)
{
  // Initialize
  Mpeg4File::InitData();
  m_pStreamPort = pPort;
  m_playAudio   = bPlayAudio;
  m_playVideo   = bPlayVideo;
  m_playText    = bPlayText;
  m_eFileFormat = eFileFormat;

  m_bAudioPresentInClip = false;
  m_bVideoPresentInClip = false;
  m_bTextPresentInClip  = false;
  m_fileSizeFound       = false;

#if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
  if(pPort)
  {
    int64 nStreamSize = 0;
    bool bEndOfData = false;
    if(video::iStreamPort::DS_SUCCESS ==
               pPort->GetContentLength(&nStreamSize))
    {
       m_fileSize = (uint64) nStreamSize;
    }
    else
    {
      m_fileSize = MAX_FILE_SIZE;
    }
    /*if bEndofData is true,then OEM has indicated that the entire clip is available in the buffer
    Therefore we can use the atom like MFRA which is present at the end of the clip*/
    m_parseFilePtr = OSCL_FileOpen(pPort);
    int64 nDownloadedBytes = 0;
    m_pStreamPort->GetAvailableOffset(&nDownloadedBytes, &bEndOfData);
    m_wBufferOffset = nDownloadedBytes;
    m_bEndOfData = bEndOfData;
  }
#endif /* defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) */

  if(!m_parseFilePtr )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Failed to create m_parseFilePtr");
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
  if(!m_fileSize)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR, "Invalid file size %llu",m_fileSize);
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
  m_fileSizeFound = true;
  AtomUtils::fileSize   = m_fileSize;

#if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
  bHttpStreaming    = true;
#endif /* defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) */
}
#endif

#ifdef FEATURE_FILESOURCE_DRM_DCF
/*===========================================================================

FUNCTION
  Mpeg4File::Mpeg4File

DESCRIPTION
  Constructor for creating mpeg4file file instance for DCF media

DEPENDENCIES
  None

INPUT PARAMETERS:
->inputStream:IxStream*
->urnType:It should be URN_INPUTSTREAM
->bPlayVideo:Indicates if this is video instance
->bPlayAudio:Indicates if this is audio instance
->bPlayText:Indicates if this is text instance

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
Mpeg4File::Mpeg4File(IxStream* inputStream,
                     bool bPlayVideo,
                     bool bPlayAudio,
                     bool bPlayText
                     )
{
  IxStream* pStream;

  #if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
    boolean bEndOfData = false;
  #endif /* defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) */

  // Initialize
  InitData();
  m_playAudio     = bPlayAudio;
  m_playVideo     = bPlayVideo;
  m_playText      = bPlayText;
  m_inputStream   = inputStream;

  m_bAudioPresentInClip = false;
  m_bVideoPresentInClip = false;
  m_bTextPresentInClip  = false;

  if(inputStream == NULL)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
       "Mpeg4File::Mpeg4File input-stream is NULL");
    return;
  }

  #if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
    pStream = (IxStream* )inputStream;
    if(pStream->IsProgressive())
    {
      bHttpStreaming    = true;
    }
  #endif /* defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) */
    {
      m_parseFilePtr = OSCL_FileOpen(inputStream);
    }

  if(!m_parseFilePtr)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Failed to open file");
    _success = false;
    return;
  }

  // File Size calculation

  pStream = (IxStream* )inputStream;
  if(pStream)
  {
    #if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
    if(pStream->IsProgressive() && (bEndOfData == FALSE))
    {
      m_fileSize = MAX_FILE_SIZE;
    }
    else
    #endif /* defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) */
    {
      pStream->Size(&m_fileSize);
    }
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"Mpeg4File::Mpeg4File size %lu",m_fileSize);
  }

  AtomUtils::fileSize = m_fileSize;
}
#endif

/*===========================================================================

FUNCTION  InitData

DESCRIPTION
  Initialize all members.
===========================================================================*/
void Mpeg4File::InitData()
{

  _success = true;
  fragmentNumber         = 0;
  m_playAudio            = false;
  m_playVideo            = false;
  m_playText             = false;
  m_isFragmentedFile     = false;
  m_parsedEndofFragment  = false;
  m_parsedEndofFile      = false;
  m_corruptFile          = false;
  m_mp4ParseLastStatus = VIDEO_FMT_DONE;
  m_parseIODoneSize = 0;
  m_mp4ParseContinueCb = NULL;

  m_hasAudio             = false;
  m_hasVideo             = false;
  m_hasText              = false;
  m_allSyncVideo         = false;
  m_bTimeToOffsetInvoked = false;
  m_bMOOVPresent         = true;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  m_pStreamPort = NULL;
#endif
  m_defaultCoarseJumpLimit = QTV_MPEG4_COARSE_REPOS_LIMIT;

  memset(&m_videoFmtInfo, 0xFF, sizeof(video_fmt_info_type));
  memset(&m_track[0], 0x0, sizeof(uint32)*VIDEO_FMT_MAX_MEDIA_STREAMS);

  memset(&m_sampleInfo[0], 0x0, sizeof(video_fmt_sample_info_type)*VIDEO_FMT_MAX_MEDIA_STREAMS);
  m_bSampleInfoChanged = false;
  memset(&m_nextSample[0], 0x0, sizeof(uint32)*VIDEO_FMT_MAX_MEDIA_STREAMS);
  m_bOMADRMV2Encrypted = false;
  m_reposStreamPending  = 0;
  m_trackCount          = 0;
  m_mp4ParseEndCb       = NULL;
  m_mp4ParseServerData  = NULL;

  m_pFileBuf            = NULL;
  m_fileSize            = 0;
  m_filename            = NULL;
  m_parseFilePtr        = NULL;
  m_fileSize            = 0;
  AtomUtils::fileSize   = 0;

#ifdef FEATURE_FILESOURCE_DRM_DCF
  m_inputStream         = NULL;
#endif

  _fileErrorCode        = PARSER_ErrorNone;

  textSampleEntryCount  = 0;
  for(int i = 0; i < VIDEO_FMT_MAX_MEDIA_STREAMS;i++)
  {
    memset(m_EncryptionType+i, 0, sizeof(Track_Encryption_Type));
  }
  m_pEncryptedDataBuffer      = NULL;
  m_pDRMClientData            = NULL;
  m_pDRMDecryptFunction       = NULL;
#ifdef FEATURE_MP4_CUSTOM_META_DATA
  _pdcfAtom                   = NULL;
  _dcmdAtom                   = NULL;
  _kddiDRMAtom                = NULL;
  _kddiContentPropertyAtom    = NULL;
  _kddiMovieMailAtom          = NULL;
  _kddiEncoderInformationAtom = NULL;
  _kddiGPSAtom                = NULL;
  _kddiTelopElement           = NULL;
  _pTsmlAtom                  = NULL;
  _midiAtom                   = NULL;
  _linkAtom                   = NULL;
  _ftypAtom                   = NULL;
  m_OffsetinEncryptionTypeArray = 0;
#endif /* FEATURE_MP4_CUSTOM_META_DATA */

  _cprtAtom = NULL;
  _authAtom = NULL;
  _titlAtom = NULL;
  _dscpAtom = NULL;
  _rtngAtom = NULL;
  _clsfAtom = NULL;
  _gnreAtom = NULL;
  _perfAtom = NULL;
  _lociAtom = NULL;
  _kywdAtom = NULL;
  _metaAtom = NULL;
  _albumAtom= NULL;
  pSubsAtom = NULL;
  pYrccAtom = NULL;

  m_nextAVCSeqSample             = 0;
  m_nextAVCPicSample             = 0;
  m_nextMVCSeqSample             = 0;
  m_nextMVCPicSample             = 0;

  /* initialize client data for videoFMT callback */
  for(unsigned int i=0; i<VIDEO_FMT_MAX_MEDIA_STREAMS; i++)
  {
    m_clientData[i].Mp4FilePtr = this;
    m_clientData[i].streamNum  = (int)i;
  }

  bHttpStreaming               = false;
  m_wBufferOffset              = 0;
  m_bEndOfData                 = false; /* Reset EndOfData flag initially */
  bGetMetaDataSize             = FALSE;
  bDataIncomplete              = FALSE;
  bQtvPlayerPaused             = TRUE;
  bsendParseFragmentCmd        = FALSE;
  m_currentParseFragment       = 0;
  m_minOffsetRequired          = 0;
  parserState                  = PARSER_IDLE;
  m_pbTime                     = 0;
  m_startupTime                = 4000;//HTTP_DEFAULT_STARTUP_TIME
  memset(m_bufferedUptoSample, 0, sizeof(m_bufferedUptoSample));

  if(MM_CriticalSection_Create(&videoFMT_Access_CS) != 0)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Failed to create Critical Section.");
    videoFMT_Access_CS = NULL;
  }
  Initialized = FALSE;
  Parsed = FALSE;

  m_bUdtaAtomPresent = false;
  UUIDatomEntryCount = 0;
  m_pBuffer          = NULL;
  m_nBufferSize      = 0;
  m_pSampleInfoBuffer= NULL;
  m_nSampleInfoBufferSize = 0;
  m_bMediaAbort      = false;

  /* 'DASH' related params*/
  m_bIsDashClip     = false;
  m_bFragBoundary   = false;
  m_bSeekDone       = false;
  m_nLastFragOffset = 0;
  m_ulPSSHatomEntryCount = 0;
  m_ulSelectedDRMIdx     = 0;
  m_eFileFormat          = FILE_SOURCE_UNKNOWN;
  memset(&m_baseTimeStamp[0], 0x0, sizeof(uint64)*VIDEO_FMT_MAX_MEDIA_STREAMS);

  /* Buffer to store MOOV atom to complete seek operations faster.
     For long duration clips, Parser repeatedly requests for reloading of
     Sample tables. In case of streaming, it incurs additional delay if data
     needs to be re-downloaded. To avoid these additional delays, this internal
     cache will help. */
  m_ulMoovBufDataRead = 0;
  m_ulMoovBufSize     = 0;
  m_ullMoovAtomOffset = 0;
  m_pMoovAtomBuf      = NULL;
  m_bDataUnderRun     = false;
  m_ulGeoTagSize      = 0;
}

/*===========================================================================
FUNCTION  SetCriticalSection
DESCRIPTION
Updates the critical section field info in File Pointer.
===========================================================================*/
void Mpeg4File::SetCriticalSection(MM_HANDLE pcriticalsection)
{
  if(m_parseFilePtr)
  {
    m_parseFilePtr->pCriticalSection = pcriticalsection;
  }
}

 #ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
/*===========================================================================
FUNCTION  GetOffsetForTime
DESCRIPTION
Returns the absolute file offset associated with the sync sample
with respect to pbtime(either in forward direction or backward direction)
Typically used in case of http streaming to determine if
existing downloaded data can handle Forward or Rewind when current playback time
is pbtime.
===========================================================================*/
bool Mpeg4File::GetOffsetForTime(uint64 pbtime,uint64* fileoffset, uint32 id,
                                 uint64 currentPosTimeStamp, uint64& reposTime)
{
  bool bRet = false;
  //This API is valid only for non-fragmented clips
  if(!isFileFragmented() && fileoffset)
  {
    *fileoffset = 0;
    //validate track-id
    video_fmt_stream_info_type *p_track = getTrackInfoForID(id);
    if ( !p_track )
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR, "GetOffsetForTime, unknown track id = %lu", id);
    }
    else
    {
      uint32 streamNum = p_track->stream_num;
      video_fmt_sample_info_type  sampleInfo;
      memset(&sampleInfo, 0x0, sizeof(video_fmt_sample_info_type));
      uint64 modTS = pbtime;
      m_bTimeToOffsetInvoked = true;
      if ( getTimestampedSampleInfo (p_track, pbtime, &sampleInfo,
                                     &modTS, true, currentPosTimeStamp) )
      {
        //We will return the offset 1 less so that we always attempt to read from the point after data start
        //to avoid failure for HTTP streaming case when we do seek into area which is not buffered.

        *fileoffset = ( getSampleAbsOffset(streamNum,sampleInfo.offset,sampleInfo.size) ) -1;
        /*
        * getSampleAbsOffset API is used when we reposition to a particular sample.
        * After reposition, we need to make sure that there is sufficient data
        * to play the repositioned sample completely.
        * For this reason, offset returned by getSampleAbsOffset API
        * takes into account the size of this sample.
        * In our case, we need to subtract the sample size from the offset returned.
        *
        */
        *fileoffset -= sampleInfo.size;
        reposTime = modTS;
        MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_ERROR,
       "GetOffsetForTime located offset:track id = %lu pbtime %llu offset %llu",
                     id,pbtime,*fileoffset);
        bRet = true;
      }
      else
      {
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
         "GetOffsetForTime failed to locate offset:track id = %lu pbtime %llu",
         id,pbtime);
      }
      m_bTimeToOffsetInvoked = false;
    }
  }//if(!isFileFragmented() && fileoffset)
  return bRet;
}
/*===========================================================================
FUNCTION  Mpeg4File::GetLastRetrievedSampleOffset
DESCRIPTION
Returns the absolute file offset for the current retrieved sample
===========================================================================*/
uint64 Mpeg4File::GetLastRetrievedSampleOffset(uint32 trackid)
{
  uint64 sample_offset = 0;
  if(m_videoFmtInfo.server_data)
  {
    //validate track-id
    video_fmt_stream_info_type *p_track = getTrackInfoForID(trackid);
    if(p_track)
    {
      video_fmt_mp4r_context_type * context =
        (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
      video_fmt_mp4r_stream_type* stream =
        &context->stream_state [p_track->stream_num];
      sample_offset = stream->get_data_src;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
        "GetLastRetrievedSampleOffset sample_offset %llu", sample_offset);
    }
  }
  return sample_offset;
}
#endif
/*===========================================================================

FUNCTION  parseFirstFragment

DESCRIPTION
Private method to parse the first atom inside the MP4 file opened inside
the consturctor.

===========================================================================*/
void Mpeg4File::parseFirstFragment()
{
  if(_success)
  {
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
    if(!initializeVideoFMT())
    {
      _fileErrorCode = PARSER_ErrorReadFail; // Read past EOF
      _success = false;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "Mpeg4File::parseFirstFragment InitVideofmt failure");
    }
#else
    // Parse the file as an MP4 file, building a list of atom information
    // structures.
    video_fmt_open (mp4ParseStatusCallback, this, VIDEO_FMT_MP4,0xff);

    while(  (m_mp4ParseLastStatus != VIDEO_FMT_INFO) &&
            (m_mp4ParseLastStatus != VIDEO_FMT_FAILURE) &&
            (m_mp4ParseLastStatus != VIDEO_FMT_DATA_INCOMPLETE) &&
            (m_mp4ParseLastStatus != VIDEO_FMT_FRAGMENT) &&
            (m_mp4ParseLastStatus != VIDEO_FMT_DATA_CORRUPT)  )
    {
      if( (m_mp4ParseContinueCb == NULL) ||
          (m_mp4ParseServerData == NULL)  )
        break;
      else
        m_mp4ParseContinueCb(m_mp4ParseServerData);
    }
    if(m_mp4ParseLastStatus == VIDEO_FMT_FRAGMENT)
    {
      m_isFragmentedFile = true;
    }
#endif /*FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD */
    // Check for any atoms that may have read past the EOF that were not
    // already caught by any earlier error handling
    if( (m_mp4ParseLastStatus == VIDEO_FMT_FAILURE) ||
        (m_mp4ParseLastStatus == VIDEO_FMT_DATA_CORRUPT)  )
    {
      _fileErrorCode = PARSER_ErrorReadFail; // Read past EOF
      _success = false;
      (void)OSCL_FileClose( m_parseFilePtr );
      m_parseFilePtr = NULL;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "Mpeg4File::parseFirstFragment InitVideofmt failure");
      return;
    }

    if(_success)
    {
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
      if( !bHttpStreaming && (!getNumTracks()) )
#else
      if( !getNumTracks() )
#endif
      {
        _success = false;
        _fileErrorCode = PARSER_ErrorZeroTracks;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
          "Mpeg4File::parseFirstFragment Zero Tracks in i/p file/buffer");
        return;
      }
    }
#ifdef FEATURE_MP4_CUSTOM_META_DATA
    //if parsing was successful, parse KDDI Telop Text
    if (_success && _kddiTelopElement)
    {
      process_kddi_telop_text();
    }
    process_mod_midi_atom();
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
    /* if video track does not have STSS table, then all video frames are SYNC frame */
    if(_success && m_playVideo)
    {
      /* if clip has video and we don't have STSS table, we can't reposition */
      video_fmt_stream_info_type *p_track;
      for(uint32 index = 0; index < m_videoFmtInfo.num_streams; index++)
      {
        p_track = m_videoFmtInfo.streams + index;
        if( p_track->type == VIDEO_FMT_STREAM_VIDEO )
        {
          video_fmt_mp4r_context_type  *context;
          video_fmt_mp4r_stream_type   *stream;
          context = (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
          if(index <= context->num_streams)
          {
            stream = &context->stream_state [index];
            if(!stream->stss.table_size)
            {
              m_allSyncVideo = true;
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "no Video STSS, assuming all frames are sync frames");
            }
          }
        }
      }
    } /* end of if(m_playVideo) */
#ifndef _ANDROID_
    //! This block is not required in LA.
    //! Codec does not require frames per sample property set by Parser.
    if((_success)&& (m_playAudio))
    {
      video_fmt_stream_info_type *p_track=0;
      uint32 framelength=0;
      for(uint32 index = 0; index < m_videoFmtInfo.num_streams; index++)
      {
        p_track = m_videoFmtInfo.streams + index;
        if( (p_track)                                                   &&
          (p_track->type == VIDEO_FMT_STREAM_AUDIO)                   &&
          (p_track->subinfo.audio.format==VIDEO_FMT_STREAM_AUDIO_AMR) )
        {
          video_fmt_sample_info_type sampleInfo;
          memset(&sampleInfo, 0x0, sizeof(video_fmt_sample_info_type));
          bool retStat = false;
          uint32 sampleId = 0;
          do
          {
            //Keep reading untill we get valid sample with non zero DELTA and non zero SIZE
            PARSER_ERRORTYPE retError = getSampleInfo(p_track->stream_num, sampleId,
                                                      1, &sampleInfo);
            if(PARSER_ErrorNone == retError)
            {
              retStat = true;
            }
            sampleId++;
          }while( (retStat) && (m_mp4ReadLastStatus[p_track->stream_num] == VIDEO_FMT_IO_DONE) &&
                  (!sampleInfo.delta || !sampleInfo.size) );


          if( !retStat || (m_mp4ReadLastStatus[p_track->stream_num] != VIDEO_FMT_IO_DONE) )
          {
            _success = false;
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "Mpeg4File::parseFirstFragment getsampleinfo for audio failure");
            return;
          }

          /* for some clips, SamplesPerFrame value is wrongly set to 10 and the correct
          value was one. This check is to fix those clips and we also try to
          minimize the scope of this fix by checking this value in clip and
          size of first AMR sample from a clip in question/given clip*/
          if( ((sampleInfo.size==32) || (sampleInfo.size==13)|| (sampleInfo.size==21) || (sampleInfo.size==18))&&
            (p_track->subinfo.audio.audio_params.frames_per_sample==10) )
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
              "SamplesPerFrame=%d for AMR track, but using 1.",
              p_track->subinfo.audio.audio_params.frames_per_sample);
            p_track->subinfo.audio.audio_params.frames_per_sample = 1;
          }
          if(p_track->subinfo.audio.audio_params.frames_per_sample && p_track->media_timescale)
          {
            framelength= (uint32)
              ((sampleInfo.delta*1000)/(p_track->subinfo.audio.audio_params.frames_per_sample))/(p_track->media_timescale);
          }
          if((framelength<(DURATION_OF_AMR_FRAME_BLOCK-1))||(framelength>(DURATION_OF_AMR_FRAME_BLOCK+1)))
          {
            //CMX validates AMR frame contents so QTV check is redundant.
            //_success = false;
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "Invalid(?) AMR Audio content:Duration of Frame Block=%lu",framelength);
          }
        }
      }//for(uint32 index = 0; index < m_videoFmtInfo.num_streams; index++)
    }//if((_success)&& (m_playAudio))
#endif
  }//if(_success)
}

/*===========================================================================

FUNCTION  initializeVideoFMT

DESCRIPTION
Public method used to initialize VideoFMT

===========================================================================*/
boolean Mpeg4File::initializeVideoFMT ( void )
{
  boolean returnStatus       = TRUE;
  video_fmt_type eFormatType = VIDEO_FMT_MP4;

  // If file format is marked as DASH complaint, update videofmt Type.
  if(FILE_SOURCE_MP4_DASH == m_eFileFormat)
  {
    eFormatType = VIDEO_FMT_MP4_DASH;
  }

  // Parse the file as an MP4/DASH file, building a list of atom information
  // structures.
  video_fmt_open (mp4ParseStatusCallback, this, eFormatType, 0xff);

  while ((m_mp4ParseLastStatus != VIDEO_FMT_INIT)
    && (m_mp4ParseLastStatus != VIDEO_FMT_FAILURE)
    && (m_mp4ParseLastStatus != VIDEO_FMT_DATA_CORRUPT)
    )
  {
    if((m_mp4ParseContinueCb == NULL) ||
      (m_mp4ParseServerData == NULL))
      break;
    else
      m_mp4ParseContinueCb(m_mp4ParseServerData);
  }

  if( (m_mp4ParseLastStatus == VIDEO_FMT_FAILURE) ||
      (m_mp4ParseLastStatus == VIDEO_FMT_DATA_CORRUPT) ||
      (m_corruptFile == true) )
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
      "Mpeg4File::initializeVideoFMT m_mp4ParseLastStatus %d",
      m_mp4ParseLastStatus);
    return FALSE;
  }

  if (m_mp4ParseLastStatus == VIDEO_FMT_INIT && bHttpStreaming)
  {
    m_currentParseFragment = 0;
    bGetMetaDataSize = TRUE;
    bDataIncomplete = TRUE;
    (void)ParseStream();
    if(m_bIsDashClip)
      (void)ParseStream();
    returnStatus = TRUE;
  }
  else if((m_mp4ParseLastStatus == VIDEO_FMT_INIT) && !bHttpStreaming)
  {
    if(ParseStream())
    {
      if(m_bIsDashClip)
      {
        if(ParseStream())
          returnStatus = TRUE;
        else
          returnStatus = FALSE;
      }
      else
        returnStatus = TRUE;
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
        "Mpeg4File::initializeVideoFMT returnStatus is false");
      returnStatus = FALSE;
    }
  }

  return returnStatus;
}
#ifdef FEATURE_FILESOURCE_PSEUDO_STREAM
/*===========================================================================

FUNCTION  getMetaDataSize

DESCRIPTION
Public method used to determine the meta-data size of the fragment.

===========================================================================*/
bool Mpeg4File::getMetaDataSize ( void )
{
  video_fmt_mp4r_context_type *video_fmt_parse_context;
  MM_CriticalSection_Enter(videoFMT_Access_CS);
  m_videoFmtInfo.fragment_size_cb (m_videoFmtInfo.server_data,
    m_currentParseFragment);

  while ((m_mp4ParseLastStatus != VIDEO_FMT_FRAGMENT_SIZE)
    && (m_mp4ParseLastStatus != VIDEO_FMT_FAILURE)
    && (m_mp4ParseLastStatus != VIDEO_FMT_DATA_INCOMPLETE)
    && (m_mp4ParseLastStatus != VIDEO_FMT_DATA_CORRUPT)
     && (m_mp4ParseLastStatus != VIDEO_FMT_INFO))
  {
    if((m_mp4ParseContinueCb == NULL) ||
      (m_mp4ParseServerData == NULL))
      break;
    else
      m_mp4ParseContinueCb (m_mp4ParseServerData);
  }
  MM_CriticalSection_Leave(videoFMT_Access_CS);
  switch(m_mp4ParseLastStatus)
  {
  case VIDEO_FMT_FAILURE:
  case VIDEO_FMT_DATA_CORRUPT:
    return false;

  case VIDEO_FMT_FRAGMENT_SIZE:
    video_fmt_parse_context = (video_fmt_mp4r_context_type *)m_videoFmtInfo.server_data;
    if(m_currentParseFragment == video_fmt_parse_context->fragment_requested)
    {
      if(m_currentParseFragment == 0)
        m_minOffsetRequired = video_fmt_parse_context->fragment_size;
      else
      m_minOffsetRequired += video_fmt_parse_context->fragment_size;
      /* Added this to support 3g2 files */
      mdat_size = video_fmt_parse_context->mdat_size;
      fragmentNumber = video_fmt_parse_context->fragment_requested;
      m_currentParseFragment = fragmentNumber + 1;
      /* Added this to support 3g2 files  */
    }
    bDataIncomplete = FALSE;
    return true;

  case VIDEO_FMT_DATA_INCOMPLETE:
    bDataIncomplete = TRUE;
    return false;

  default:
    break;
  }
  return false;
}
#endif
#if defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) ||defined (FEATURE_FILESOURCE_PSEUDO_STREAM)
/*===========================================================================

FUNCTION  GetBufferedDuration

DESCRIPTION
Public method used to determine the buffered track time in ms for a track

===========================================================================*/
bool Mpeg4File::getBufferedDuration(uint32 id, int64 nBytes, uint64 *pBufferedTime)
{

  bool returnStatus = false;
  video_fmt_stream_info_type *p_track = 0;

  //Fetch the track_info.
  p_track = getTrackInfoForID(id);

  if ( !p_track || !pBufferedTime )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "GetBufferedDuration Bad Argument");
  }
  else
  {
    *pBufferedTime = 0;
    uint32 streamNum = p_track->stream_num;
    video_fmt_sample_info_type sampleInfo, preSampleInfo;
    video_fmt_mp4r_context_type *context = (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
    video_fmt_mp4r_stream_type  *stream = &context->stream_state [streamNum];

    uint64 reqSampleNum = 0;
    if(m_nextSample[streamNum] > m_sampleInfo[streamNum].sample)
    {
      //RW was done recently or this API is called during normal playback scenario
      //or after resuming from under-run
      reqSampleNum = FILESOURCE_MIN(m_sampleInfo[streamNum].sample,
                         m_bufferedUptoSample[streamNum].sample);
    }
    else
    {
      //FD was done recently
      reqSampleNum = FILESOURCE_MAX(m_sampleInfo[streamNum].sample,
                         m_bufferedUptoSample[streamNum].sample);
    }

    uint64 absSampleOffsetInFile = 0;
    /* If Client does not provide offset, then check the Available data offset
       and provide the approximate buffer duration for that offset. */
    if((bHttpStreaming) && (m_pStreamPort) && (MAX_FILE_SIZE == (uint64)nBytes))
    {
       bool bEndOfData = false;
      // if buffered/downloaded bytes is not provided then get the available offset
      m_pStreamPort->GetAvailableOffset((int64*)&nBytes, &bEndOfData);
    }
    //If Data downloaded is not known, then return false
    if (MAX_FILE_SIZE == (uint64)nBytes)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "getBufferedDuration return false as nBytes is -1");
      return false;
    }

    memcpy(&preSampleInfo, &m_bufferedUptoSample[streamNum], sizeof(preSampleInfo));
    memset(&sampleInfo, 0, sizeof(sampleInfo));
    for(; ((uint64)nBytes > absSampleOffsetInFile) &&
           (p_track->frames > reqSampleNum); reqSampleNum++)
    {
      PARSER_ERRORTYPE retError = getSampleInfo(streamNum, reqSampleNum, 1, &sampleInfo);
      if(PARSER_ErrorNone == retError)
      {
        preSampleInfo = sampleInfo;
        /*Fetch the abs file offset for the sample at pbTime.*/
        absSampleOffsetInFile = getSampleAbsOffset (streamNum, sampleInfo.offset, sampleInfo.size);
      }
      else
      {
        break;
      }
    }

    if(preSampleInfo.time != 0)
    {
      *pBufferedTime = (preSampleInfo.time*1000)/p_track->media_timescale;
      returnStatus = true;
      m_bufferedUptoSample[streamNum] = preSampleInfo;
    }
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "getBufferedDuration return %d and time %llu",
                 returnStatus, *pBufferedTime);
  }
  return returnStatus;
}

/*===========================================================================

FUNCTION  CanPlayTracks

DESCRIPTION
Public method used to determine if all the media tracks in this context,
can be played for the specified amouunt of time.

===========================================================================*/
bool Mpeg4File::CanPlayTracks(uint64 pbTime)
{
  //If audio only/video only clip then the m_trackCount
  //inthe other context is zero: so return true.
  if(!m_trackCount)
    return true;

  bool returnStatus = true;
  uint32 reqSampleNum = 0;

  video_fmt_mp4r_context_type *context = (video_fmt_mp4r_context_type *)
                                         m_videoFmtInfo.server_data;
  if(TRUE == context->isDashClip)
  {
    m_bIsDashClip = TRUE;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "CanPlayTracks is not required for DASH");
    return true;
  }

  if(pbTime <= m_pbTime)
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "CanPlayTracks: pbTime=%llu, m_pbTime=%llu", pbTime, m_pbTime);
    return true;
  }

  /*Check if all the tracks are playable for atleast pbTime*/
  for(uint8 index = 0; index < m_trackCount; index++)
  {
    video_fmt_sample_info_type sampleInfo;
    memset(&sampleInfo, 0, sizeof(video_fmt_sample_info_type));

    // Convert timestamp in milli-seconds to timescale
    uint64 timescaledTime = (pbTime*m_track[index]->media_timescale)/1000;

    uint32 streamNum = m_track[index]->stream_num;

    if((m_videoFmtInfo.streams[streamNum].type == VIDEO_FMT_STREAM_VIDEO) && !m_hasVideo)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "CanPlayTracks: No video..m_playVideo=%d", m_playVideo);
      continue;
    }
    else if((m_videoFmtInfo.streams[streamNum].type == VIDEO_FMT_STREAM_AUDIO) && !m_hasAudio)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "CanPlayTracks: No audio..m_playAudio=%d", m_playAudio);
      continue;
    }
    else if((m_videoFmtInfo.streams[streamNum].type == VIDEO_FMT_STREAM_TEXT) && !m_hasText)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "CanPlayTracks: No text..m_playText=%d", m_playText);
      continue;
    }

    uint64 maxFrames  =  m_track[index]->frames;

    if(!m_pbTime)
    {
      m_track[index]->prevReqSampleNum = 0;
    }

    returnStatus = false;

    for (reqSampleNum = m_track[index]->prevReqSampleNum;
      reqSampleNum < maxFrames; ++reqSampleNum )
    {
      PARSER_ERRORTYPE retError = getSampleInfo(streamNum, reqSampleNum, 1, &sampleInfo);
      if(PARSER_ErrorNone == retError)
      {
        returnStatus = true;
        if ((sampleInfo.time + sampleInfo.delta) >= timescaledTime)
        {
          if(m_track[index]->type == VIDEO_FMT_STREAM_VIDEO)
          {
            /*For the video track we should atleast be able to send QTV_3GPP_MIN_NUM_VIDEO_FRAMES_TO_BUFFER
            to videoDec */
            if (reqSampleNum >= (m_sampleInfo[streamNum].sample + QTV_3GPP_MIN_NUM_VIDEO_FRAMES_TO_BUFFER))
            {
              m_track[index]->prevReqSampleNum = reqSampleNum;
              break;
            }
          }
          else
          {
            m_track[index]->prevReqSampleNum = reqSampleNum;
            break;
          }
        }
      }
      else
      {
        returnStatus = false;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "CanPlayTracks: getSampleInfo ERROR");
        break;
      }
    } // for(maxFrames)

    if(!returnStatus)
      break;

    /* Condition: pbTime > playable media time; then
    finish playing the remaining samples */
    if(reqSampleNum == maxFrames)
    {
      m_track[index]->prevReqSampleNum = reqSampleNum-1;
    }

    if(sampleInfo.offset && (sampleInfo.offset >= m_track[index]->header))
    {
      sampleInfo.offset -= m_track[index]->header;
    }

    /*Fetch the abs file offset for the sample at pbTime.
      Validate sample Offset and Size before checking absolute offset..*/
    if (sampleInfo.offset && sampleInfo.size)
    {
      uint64 absFileOffset = getSampleAbsOffset (streamNum, sampleInfo.offset,
                                                 sampleInfo.size);

      if((!absFileOffset) ||
         (m_wBufferOffset>0 && absFileOffset >= m_wBufferOffset))
      {
        returnStatus = false;
        MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "CanPlayTracks: absFileOffset=%llu, m_wBufferOffset=%llu, strNo=%lu",
          absFileOffset, m_wBufferOffset,streamNum);
        break;
      }
    }

  }// for(m_trackCount)

  if(returnStatus)
  {
    //All the tracks can be played for atleast pbTime.
    m_pbTime = pbTime;
  }

  return returnStatus;
}

/*===========================================================================

FUNCTION  getSampleAbsOffset

DESCRIPTION
Public method used to fetch the absolute file offset for a given
media sample stream offset.

===========================================================================*/
uint64 Mpeg4File::getSampleAbsOffset (uint32 streamNum,
                                      uint64 sampleOffset,
                                      uint32 sampleSize)
{
  int loop = 0;
  bool bDone = false;
  m_absFileOffset[streamNum] = 0;
  m_videoFmtInfo.abs_file_offset_cb(streamNum,
    sampleOffset,
    sampleSize,
    m_videoFmtInfo.server_data,
    mp4ReadStatusCallback,
    &(m_clientData[streamNum]) );

  while ( !bDone )
  {
    while ( (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_ABS_FILE_OFFSET) &&
      (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_FAILURE) &&
      (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_BUSY) &&
      (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_DATA_CORRUPT) &&
      (loop<MPEG4_VIDEOFMT_MAX_LOOP) )
    {
      m_mp4ReadContinueCb[streamNum] (m_mp4ReadServerData[streamNum]);
      loop++;
    }

    if( loop >= MPEG4_VIDEOFMT_MAX_LOOP )
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
        "getSampleAbsOffset: VideoFMT hangs. StreamNum=%lu, startingAt %llu",
        streamNum, sampleOffset);
      bDone = TRUE;
    }

    switch ( m_mp4ReadLastStatus[streamNum] )
    {
    case VIDEO_FMT_BUSY:
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
          "getSampleAbsOffset:BUSY FATAL starting at %llu", sampleOffset);
        return 0;
      }

    case VIDEO_FMT_ABS_FILE_OFFSET:
      bDone = true;
      break;

    case VIDEO_FMT_FAILURE:
    case VIDEO_FMT_DATA_CORRUPT:
      {
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
          "getSampleAbsOffset:FAILURE FATAL starting at %llu, size %lu",
          sampleOffset, sampleSize);
        return 0;
      }
    default:
      break;
    }
  }

  return m_absFileOffset[streamNum];
}

/*===========================================================================

FUNCTION  updateBufferWritePtr

DESCRIPTION
Public method used to update the write buffer offset during Http streaming.

===========================================================================*/
void Mpeg4File::updateBufferWritePtr ( uint64 writeOffset )
{
  //Executing in the UI thread context.
  m_wBufferOffset = writeOffset;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  if(bHttpStreaming)
  {
    if(m_pStreamPort)
    {
       bool bEndOfData = false;
       int64 wBufferOffset = 0;
      //Pull interface so pull download data size from OEM
      m_pStreamPort->GetAvailableOffset(&wBufferOffset, &bEndOfData);
      m_wBufferOffset = wBufferOffset;
      m_bEndOfData = bEndOfData;
    }
  }
  else
    m_bEndOfData = true;
#else
  m_bEndOfData = true;
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  if(((parserState == PARSER_READY) || (parserState == PARSER_RESUME)) && m_wBufferOffset>= m_minOffsetRequired)
  {
    //check if we got sufficient data to start parsing the
    //meta data.
    sendParseHTTPStreamEvent();
  }
}

/*===========================================================================

FUNCTION  sendParseHTTPStreamEvent

DESCRIPTION
Public method used to switch contexts and call the parseHttpStream.

===========================================================================*/
void Mpeg4File::sendParseHTTPStreamEvent(void)
{
  //QTV_PROCESS_HTTP_STREAM_type *pEvent = QCCreateMessage(QTV_PROCESS_HTTP_STREAM, m_pMpeg4Player);

  //if (pEvent)
  //{
  //  pEvent->bHasAudio = (bool) m_playAudio;
  //  pEvent->bHasVideo = (bool) m_playVideo;
  //  pEvent->bHasText = (bool) m_playText;
  //  QCUtils::PostMessage(pEvent, 0, NULL);
  //}
}

/*===========================================================================

FUNCTION  sendHTTPStreamUnderrunEvent

DESCRIPTION
Public method used to switch contexts and notify the player about buffer-underrun.

===========================================================================*/
void Mpeg4File::sendHTTPStreamUnderrunEvent(void)
{
  //QTV_HTTP_STREAM_BUFFER_UNDERRUN_EVENT_type *pEvent = QCCreateMessage(QTV_HTTP_STREAM_BUFFER_UNDERRUN_EVENT, m_pMpeg4Player);

  //if (pEvent)
  //{
  //  pEvent->bAudio = (bool) m_playAudio;
  //  pEvent->bVideo = (bool) m_playVideo;
  //  pEvent->bText = (bool) m_playText;
  //  QCUtils::PostMessage(pEvent, 0, NULL);
  //}
}
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

/*===========================================================================

FUNCTION  parseHTTPStream

DESCRIPTION
Public method used to parse the Http Stream.

===========================================================================*/
bool Mpeg4File::parseHTTPStream ( void )
{
  /* For DASH clips we parse first moof as well along with "moov" atom. */
  if((m_currentParseFragment > 0 && !m_bIsDashClip) ||
     (m_currentParseFragment > 1 && m_bIsDashClip))
  {
    return true;
  }
  if (false == m_bMOOVPresent)
  {
    _fileErrorCode = PARSER_ErrorInHeaderParsing;
    _success       = false;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "Mpeg4File::ParseStream moov atom is not present");
    return false;
  }
  //First Make sure next fragment moov/moof is available.
  if(peekMetaDataSize(m_currentParseFragment) &&
     ((0 == m_wBufferOffset) || (m_wBufferOffset >= m_minOffsetRequired)))
  {
    if(Initialized == FALSE)
    {
     //Then parse it and check for canPlayTracks().
      if(Parsed == FALSE && !parseMetaData())
      {
        //QTV_PS_PARSER_STATUS_PAUSED
        sendParserEvent(PARSER_RESUME);
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
          "Mpeg4File::parseHTTPStream parseMetaData returned false");
        return false;
      }
      Parsed = TRUE;
      if((parserState == PARSER_RESUME || parserState == PARSER_READY))
    {
        /*In DASH, fragments need not always in incremental order. There can be
          jumps as well, so we need to keep info as this is useful further.*/
        if(m_bIsDashClip)
        {
          video_fmt_mp4r_context_type *context =
                    (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
          m_currentParseFragment = context->fragment_available;
        }
        m_bFragBoundary = true;
        m_currentParseFragment++;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
          "Mpeg4File::parseHTTPStream m_currentParseFragment %lu",
          m_currentParseFragment);
        Initialized = TRUE;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
          "Mpeg4File::parseHTTPStream Parser State = Common::PARSER_READY");
        sendParserEvent(PARSER_READY);
        return true;
    }
      return false;
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
        "Mpeg4File::parseHTTPStream Parser State = Common::PARSER_READY");
      sendParserEvent(PARSER_READY);
      return true;
    }
  }
  else
  {
    if(m_bIsDashClip)
    {
      video_fmt_mp4r_context_type *context =
        (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
      context->abs_size_retrieve_pos = m_nLastFragOffset;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "readjusting context offset to last fragment start %llu",
        m_nLastFragOffset);
    }
  }
  return false;
}

/*===========================================================================

FUNCTION  parseMetaData

DESCRIPTION
Public method used to parse the fragment meta-data.

===========================================================================*/
bool Mpeg4File::parseMetaData ( void )
{
  bool returnValue = false;

  if((m_videoFmtInfo.parse_fragment_cb == NULL) ||
    (m_videoFmtInfo.server_data == NULL))
    return false;

  m_videoFmtInfo.parse_fragment_cb (m_videoFmtInfo.server_data);

  while ((m_mp4ParseLastStatus != VIDEO_FMT_INFO)
    && (m_mp4ParseLastStatus != VIDEO_FMT_FAILURE)
    && (m_mp4ParseLastStatus != VIDEO_FMT_FRAGMENT)
    && (m_mp4ParseLastStatus != VIDEO_FMT_DATA_INCOMPLETE)
    && (m_mp4ParseLastStatus != VIDEO_FMT_DATA_CORRUPT)
    && (_fileErrorCode == PARSER_ErrorNone))
  {
    if((m_mp4ParseContinueCb == NULL) ||
      (m_mp4ParseServerData == NULL))
      break;
    else
      m_mp4ParseContinueCb (m_mp4ParseServerData);
  }
  if(_fileErrorCode != PARSER_ErrorNone)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
      "Mpeg4File::parseMetaData _fileErrorCode != PARSER_ErrorNone");
    return false;
  }

  if((m_parsedEndofFile || m_parsedEndofFragment) && !m_corruptFile)
  {
    if(m_mp4ParseLastStatus == VIDEO_FMT_FRAGMENT)
    {
      m_isFragmentedFile = true;
    }
    if (bQtvPlayerPaused)
    {
      //bQtvPlayerPaused: used to initially kickoff playing after the first
      //Http fragment is parsed.
      //QTV_PS_PARSER_STATUS_RESUME
      sendParserEvent(PARSER_RESUME);
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
        "Mpeg4File::parseMetaData Parser State = Common::PARSER_RESUME");
      bQtvPlayerPaused = FALSE;
    }
    bsendParseFragmentCmd = FALSE;

    returnValue = true;
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
          "Mpeg4File::parseMetaData: parse_fragment_cb is failed");
    returnValue = false;
  }

  // Let any waiting process know that we are done with fragment
  return returnValue;
}

/*===========================================================================

FUNCTION  resetPlaybackPos

DESCRIPTION
Public method used to reset the playback position after repositioning was successful.
Called by the player if it later decides not to reposition e.g: when there is
insufficient data to play after reposiitoning in case of 3GPP_Progressive_Dnld.

===========================================================================*/
void Mpeg4File::resetPlaybackPos(uint32 tId)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(tId);

  if ( !p_track )
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR, "resetPlayback, unknown track id = %lu", tId);
    return;
  }

  uint32 streamNum = p_track->stream_num;

  if ( m_reposStreamPending )
  {
    if ( m_reposStreamPending & maskByte[streamNum] )
    {
      m_reposStreamPending &= ~maskByte[streamNum];
    }

    PARSER_ERRORTYPE retError = getSampleInfo(streamNum, m_nextSample[streamNum]-1,
                                              1, &m_sampleInfo[streamNum]);
    if(PARSER_ErrorNone != retError)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "resetPlaybackPos: getSampleInfo ERROR");
    }
  }
  return;
}


/* <EJECT> */
/*===========================================================================

FUNCTION  getSampleInfo

DESCRIPTION
Public method used to request a media sample (frame)

===========================================================================*/
PARSER_ERRORTYPE Mpeg4File::getSampleInfo (uint32 streamNum,
                                           uint64 startingSample,
                                           uint64 sampleCount,
                                           video_fmt_sample_info_type *buffer)
{
  int loop = 0;
  bool bDone = false;
  m_iodoneSize[streamNum] = 0;

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

  video_fmt_mp4r_context_type *context = (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
  video_fmt_mp4r_stream_type  *stream = &context->stream_state [streamNum];
  /* If seek is done successfully, then videofmt cannot use fragment boundary TS.
     So, it marks the flag as false irrespective of whether frame is fragment boundary or
     in the middle of fragment. */
  if(true == m_bSeekDone)
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM, "Resetting Frag Boundary flag for Stream=%lu, Flag value =%d",
                 streamNum, stream->fragment_boundary);
    stream->fragment_boundary = false;
    if(stream->current_trun > 0)
      stream->trun_byte_count = true;
  }
  if(bHttpStreaming)
  {
    if((m_pStreamPort) && (!m_bEndOfData))
    {
       bool bEndOfData = false;
       int64 wBufferOffset = 0;
      //Pull interface so pull download data size from OEM
      m_pStreamPort->GetAvailableOffset((int64*)&wBufferOffset, &bEndOfData);
      m_wBufferOffset = wBufferOffset;
      m_bEndOfData = bEndOfData;
    }
    stream->wBufferOffset = m_wBufferOffset;
  }
  else
  {
    m_bEndOfData = TRUE;
    stream->wBufferOffset = 0;
  }

#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  m_videoFmtInfo.sample_info_cb(  streamNum,
    startingSample,
    sampleCount,
    buffer,
    m_videoFmtInfo.server_data,
    mp4ReadStatusCallback,
    &(m_clientData[streamNum]) );

  while ( !bDone )
  {
    while ( (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_IO_DONE) &&
      (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_FAILURE) &&
      (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_BUSY) &&
      (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_DATA_CORRUPT) &&
      (loop < MPEG4_VIDEOFMT_MAX_LOOP) )
    {
      m_mp4ReadContinueCb[streamNum] (m_mp4ReadServerData[streamNum]);
      loop++;
    }

    if( loop >= MPEG4_VIDEOFMT_MAX_LOOP )
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
        "VideoFMT hangs. Stream=%lu, StartSample=%llu",
        streamNum, startingSample);
      bDone = TRUE;
    }

    switch ( m_mp4ReadLastStatus[streamNum] )
    {
    case VIDEO_FMT_BUSY:
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
          "getSampleInfo FATAL stuck on sample %llu", startingSample);
        return PARSER_ErrorReadFail;
      }
    case VIDEO_FMT_IO_DONE:
      bDone = true;
      break;
    case VIDEO_FMT_FAILURE:
    case VIDEO_FMT_DATA_CORRUPT:
      {
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
          "getSampleInfo FATAL sample %llu, count %llu",
          startingSample, sampleCount);
        return PARSER_ErrorReadFail;
      }
    default:
      break;
    }
  }

  if(m_iodoneSize[streamNum])
  {
    return PARSER_ErrorNone;
  }
  else
  {
    return PARSER_ErrorEndOfFile;
  }
}


/* <EJECT> */
/*===========================================================================

FUNCTION  getSample

DESCRIPTION
Public method used to request a media sample (frame)

===========================================================================*/
PARSER_ERRORTYPE Mpeg4File::getSample (uint32 streamNum,
                                       video_fmt_data_unit_type unitDef,
                                       uint64 startingUnit,
                                       uint64 unitCount,
                                       uint8 *buffer)
{
  int loop = 0;
  bool bDone = false;

  m_iodoneSize[streamNum] = 0;

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

  video_fmt_mp4r_context_type *context =
    (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
  video_fmt_mp4r_stream_type  *stream = &context->stream_state [streamNum];
  if(bHttpStreaming)
  {
    if((m_pStreamPort) && (!m_bEndOfData))
    {
      bool bEndOfData = false;
      int64 wBufferOffset = 0;
      //Pull interface so pull download data size from OEM
      m_pStreamPort->GetAvailableOffset((int64*)&wBufferOffset, &bEndOfData);
      m_wBufferOffset = wBufferOffset;
      m_bEndOfData = bEndOfData;
    }
    stream->wBufferOffset = m_wBufferOffset;
  }
  else
  {
    m_bEndOfData = TRUE;
    stream->wBufferOffset = 0;
  }

#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

  m_videoFmtInfo.read_cb( streamNum,
    unitDef,
    startingUnit,
    unitCount,
    buffer,
    m_videoFmtInfo.server_data,
    mp4ReadStatusCallback,
    &(m_clientData[streamNum]) );

  while ( !bDone )
  {
    while ( (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_IO_DONE) &&
            (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_FAILURE) &&
            (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_BUSY) &&
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
            (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_DATA_INCOMPLETE) &&
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
            (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_DATA_CORRUPT) &&
            (loop<MPEG4_VIDEOFMT_MAX_LOOP)
          )
    {
      m_mp4ReadContinueCb[streamNum] (m_mp4ReadServerData[streamNum]);
      loop++;
    }
    if( loop >= MPEG4_VIDEOFMT_MAX_LOOP )
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
        "Mpeg4File::getSample VideoFMT hangs. StreamNum=%lu, startingAt %llu",
        streamNum, startingUnit);
      bDone = TRUE;
    }

    switch ( m_mp4ReadLastStatus[streamNum] )
    {
    case VIDEO_FMT_BUSY:
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
          "Mpeg4File::getSample getSample FATAL starting at %llu",
          startingUnit);
        _fileErrorCode = PARSER_ErrorEndOfFile;
        bDone = true;
      }
      break;

    case VIDEO_FMT_IO_DONE:
      {
        _fileErrorCode = PARSER_ErrorNone;
        bDone = true;
      }
      break;

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
    case VIDEO_FMT_DATA_INCOMPLETE:
      {
      MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "Mpeg4File::getSample underrun startingUnit=%llu, unitCount=%llu,\
        wBufferOffset=%llu, m_playVideo=%d",
        startingUnit, unitCount,stream->wBufferOffset,m_playVideo);
        _fileErrorCode = PARSER_ErrorDataUnderRun;
        bDone = true;
      }
      break;
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

    case VIDEO_FMT_FAILURE:
    case VIDEO_FMT_DATA_CORRUPT:
      {
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
          "Mpeg4File::getSample FATAL starting at %llu, size %llu",
          startingUnit, unitCount);
        _fileErrorCode = PARSER_ErrorReadFail;
        bDone = true;
      }
      break;
    default:
      break;
    }
  }

  if (m_bDataUnderRun )
  {
    _fileErrorCode  = PARSER_ErrorDataUnderRun;
    m_bDataUnderRun = false;
  }

  /* If End Of the data flag is set to True, then update error code as
     End of file, irrespective of ErrorCode. */
  else if((!m_bMediaAbort && m_bEndOfData) &&
          (PARSER_ErrorNone != _fileErrorCode))
  {
    _fileErrorCode = PARSER_ErrorEndOfFile;
  }

  return _fileErrorCode;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  copyData

DESCRIPTION
Global function to copy data from one buffer to another.
If byteswap is used, we may need to reverse the data.

===========================================================================*/
void copyData(uint8 *dstBuf, uint8 *srcBuf, boolean byteSwap, uint32 amount)
{
  uint32 index;
  if (byteSwap)
  {
    for (index = 0; index < amount; ++index)
    {
        dstBuf [index] = srcBuf[amount - index - 1];
    }
  }
  else
  {
    memcpy (dstBuf, srcBuf, amount);
  }
}

/* ======================================================================
FUNCTION:
  getNextMediaSample

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
PARSER_ERRORTYPE Mpeg4File::getNextMediaSample(uint32 id, uint8 *buf,
                                               uint32 *pulBufSize, uint32 &rulIndex)
{
  PARSER_ERRORTYPE  returnVal = PARSER_ErrorDefault;
  video_fmt_stream_info_type *p_track = 0;
  video_fmt_sample_info_type sCurrentSampleInfo;


  memset(&sCurrentSampleInfo, 0x0, sizeof(video_fmt_sample_info_type));
  //Fetch the track_info.
  p_track = getTrackInfoForID(id);

  /* Validate input params and class variables */
  if(NULL == pulBufSize || NULL == buf || 0 == *pulBufSize)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
               "getNextMediaSample invalid input params!!");
    return PARSER_ErrorInvalidParam;
  }
  uint32 size = *pulBufSize;
  /* track not found */
  if(!p_track)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
      "Try to get data from unknown track ID=%lu", id);
    return PARSER_ErrorDefault;
  }

  for(int i=0; i<VIDEO_FMT_MAX_MEDIA_STREAMS; i++)
  {
    if(( m_EncryptionType[i].track_id == p_track->track_id )
       && (m_EncryptionType[i].encryptionType == ENCRYPT_OMA_DRM_V2))
    {
      /* This track is encrypted with OMA DRM V2 encryption mechanism */
      m_bOMADRMV2Encrypted = true;
      /* Allocate the buffer for encrypted data */
      if((m_pEncryptedDataBuffer == NULL) && size)
      {
        m_pEncryptedDataBuffer = (uint8*) MM_Malloc(size);
      }
      if (m_pEncryptedDataBuffer == NULL)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                    "Failed to allocate memory for m_pEncryptedDataBuffer");
        return PARSER_ErrorMemAllocFail;
      }
      break;
    }
  }

  /* Resetting the Error Code before processing the data */
  _fileErrorCode = PARSER_ErrorNone;

  uint32 streamNum = p_track->stream_num;

  if ( m_reposStreamPending )
  {
    if ( m_reposStreamPending & maskByte[streamNum] )
    {
      m_nextSample[streamNum] = m_nextReposSample[streamNum];
      m_reposStreamPending &= ~maskByte[streamNum];
    }
  }

  returnVal = getSampleInfo (streamNum, m_nextSample[streamNum], 1,
                             &sCurrentSampleInfo);

  // Change in sample info for tx3g atom
  if( ( VIDEO_FMT_STREAM_TEXT == p_track->type ) &&
      ( ( m_sampleInfo[streamNum].sample_desc_index  != sCurrentSampleInfo.sample_desc_index )||
        ( sCurrentSampleInfo.sample == 0) ||
        ( m_bSeekDone == true)) )
  {
    //check if sample description index changed
    m_bSampleInfoChanged = true;
  }

  if ( PARSER_ErrorEndOfFile == returnVal )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "end of fragment is found");
    returnVal = getSampleInfoError(p_track);
    if(PARSER_ErrorNone != returnVal)
    {
      /* If End Of Data flag is set to true, irrespective of failure
         Report EndOfData as status.*/
      if(PARSER_ErrorEndOfFile != returnVal && TRUE == m_bEndOfData)
      {
        return PARSER_ErrorEndOfFile;
      }
      return returnVal;
    }
  }
  else if(PARSER_ErrorNone != returnVal)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "getsampleinfo returned with error %x", returnVal);
    return returnVal;
  }
  else
  {
    //store current sample info
    memcpy(&m_sampleInfo[streamNum], &sCurrentSampleInfo,
           sizeof(sCurrentSampleInfo));
  }

  if(size < (m_sampleInfo[streamNum].size))
  {
    if( m_fileSize && (m_sampleInfo[streamNum].size >= m_fileSize) )
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
                   "Sample size=%lu bigger than File size=%llu",
                   m_sampleInfo[streamNum].size, m_fileSize);
      return PARSER_ErrorReadFail;
    }
    p_track->largest = FILESOURCE_MAX(m_sampleInfo[streamNum].size,
                                      p_track->largest);
    /* If the sample size is greater than the size of the
      m_pEncryptedDataBuffer then free the buffer so that next time
      we can allocate with the size of the audio or video requesting */
    if(m_bOMADRMV2Encrypted && (m_pEncryptedDataBuffer != NULL))
    {
      MM_Free(m_pEncryptedDataBuffer);
    }
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                 "Sample size=%lu bigger than buf size=%lu",
                 m_sampleInfo[streamNum].size, size);
    return PARSER_ErrorInsufficientBufSize;
  }

  if(m_sampleInfo[streamNum].size == 0)
  {
    m_nextSample[streamNum] = m_sampleInfo[streamNum].sample + 1;
    *pulBufSize = 0;
    return PARSER_ErrorZeroSampleSize;
  }

  if( ( VIDEO_FMT_STREAM_VIDEO == p_track->type ) &&
      ( ( VIDEO_FMT_STREAM_VIDEO_H264 == p_track->subinfo.video.format ) ||
        ( VIDEO_FMT_STREAM_VIDEO_HEVC == p_track->subinfo.video.format )))
  {
    //Allocate the buffer if not done already
    if(!m_pBuffer)
    {
      m_pBuffer = (uint8*)MM_Malloc(size);
      //Check for memory allocation failure
      if(!m_pBuffer)
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                    "Failed to allocate memory for m_pBuffer, size %lu", size);
        return PARSER_ErrorMemAllocFail;
      }
      //Remember the size of the buffer allocated.
      m_nBufferSize = size;
    }
    else
    {
      //Make sure current buffer size is sufficient to hold the sample.
      if(m_nBufferSize < size)
      {
        //Need to reallocate the memory
        m_pBuffer = (uint8*)MM_Realloc(m_pBuffer, (size*2));
        if(!m_pBuffer)
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                       "Failed to re-allocate memory for m_pBuffer, size %lu",
                       (2*size) );
          return PARSER_ErrorMemAllocFail;
        }
        //Remember the size of the re-allocated buffer
        m_nBufferSize = (size*2);
      }
    }
  }
 /* If This track is encrypted with OMA DRM V2 encryption mechanism */
  if(m_bOMADRMV2Encrypted)
  {
    returnVal = getSample (streamNum,
      VIDEO_FMT_DATA_UNIT_BYTE,
      m_sampleInfo[streamNum].offset,
      m_sampleInfo[streamNum].size,
      m_pEncryptedDataBuffer);
  }
  else
  {
    returnVal = getSample (streamNum,
      VIDEO_FMT_DATA_UNIT_BYTE,
      m_sampleInfo[streamNum].offset,
      m_sampleInfo[streamNum].size,
      (m_pBuffer!=NULL)?m_pBuffer:buf);
  }
  if ( PARSER_ErrorNone !=  returnVal)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "getsample returned with error %x", returnVal);
    return returnVal;
  }

  // Reset Seek flag once first sample after seek operation is parsed
  m_bSeekDone = false;

  /* This is to call the registered call back to decrypt the audio/video
     sample that we read based on the track_id*/
  if(m_bOMADRMV2Encrypted)
  {
   /* Size of the decrypted sample */
    uint32 dwDecryptedDataSize = size;

    if(m_pEncryptedDataBuffer)
    {
#ifdef FEATURE_FILESOURCE_DRM_DCF
      if(m_inputStream)
      {
        IxErrnoType result = ((IxStreamMedia*)(m_inputStream))->Decrypt(p_track->track_id,
                                                                        m_pEncryptedDataBuffer,
                                                                        dwDecryptedDataSize,
                                                                        buf,
                                                                        &dwDecryptedDataSize);
        if(result != E_SUCCESS)
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR, "IxStreamMedia->Decrypt failed. Result %lu",result);
          return PARSER_ErrorDRMPlaybackError;
        }
      }
      /* call registered decrypt method to decrypt the sample */
      else if(m_pDRMDecryptFunction)
#else
      /* call registered decrypt method to decrypt the sample */
      if(m_pDRMDecryptFunction)
#endif
      {
        if( FALSE == m_pDRMDecryptFunction( p_track->track_id,
                                            m_pEncryptedDataBuffer,
                                            buf,
                                            dwDecryptedDataSize,
                                            &dwDecryptedDataSize,
                                            m_pDRMClientData ) )
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "Failed to Decrypt the sample");
          return PARSER_ErrorDRMPlaybackError;
        }
      }
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation failure. Could not decrypt frame");
      return PARSER_ErrorDRMPlaybackError;
    }
    /* decrypting successful */
    /* after decrypting, size of sample should not change */
    if(dwDecryptedDataSize != m_sampleInfo[streamNum].size)
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
                   "decrypting failed. encrypt size %lu, decrypt size %lu",
                   m_sampleInfo[streamNum].size, dwDecryptedDataSize);
      *pulBufSize = 0;
      return PARSER_ErrorEndOfFile;

    }
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "decrypting successful. encrypt size %lu, decrypt size %lu",
                 m_sampleInfo[streamNum].size, dwDecryptedDataSize);
  }
  uint32 bytesRead = 0;
  /* For H.264 codec frames, we are adding startCode data and subtracting the
     NALUnitLength field info. If NALUnitLength field is not equal to 4 bytes,
     then output buffer size will not be same as actual sample size. To handle
     such case, this change is required */
  uint32 frameSize = m_sampleInfo[streamNum].size;
  if( ( VIDEO_FMT_STREAM_VIDEO == p_track->type ) && (m_pBuffer) &&
      ( ( VIDEO_FMT_STREAM_VIDEO_H264 == p_track->subinfo.video.format ) ||
        ( VIDEO_FMT_STREAM_VIDEO_HEVC == p_track->subinfo.video.format ) ) )
  {
    // For each NALU, copy over the contents to the output buffer,
    int i = 0;
    int NALUnitFieldLen = 0;
    uint32 frameBytes = m_sampleInfo[streamNum].size;
    video_fmt_stream_info_type *pInfo = getTrackInfoForID(id);

    // Update NALU field based on codec configuration type. File container
    // stores NAL Unit Field Length minus one,so we need to add one.
    if( ( NULL != pInfo ) &&
        ( pInfo->dec_specific_info.h264_info.mvcc_info ) &&
        ( pInfo->dec_specific_info.h264_info.vwid_info ) &&
        ( pInfo->dec_specific_info.h264_info.vwid_info->num_views > 1 ) )
    {
      NALUnitFieldLen = \
        pInfo->dec_specific_info.h264_info.mvcc_info->len_minus_one + 1;
    }
    else if( ( NULL != pInfo ) &&
             ( pInfo->dec_specific_info.h264_info.avcc_info ) )
    {
      NALUnitFieldLen = \
        pInfo->dec_specific_info.h264_info.avcc_info->len_minus_one + 1;
    }
    else if( ( NULL != pInfo ) &&
             ( pInfo->dec_specific_info.sHEVCInfo.pHVCCInfo ) )
    {
      NALUnitFieldLen = \
        pInfo->dec_specific_info.sHEVCInfo.pHVCCInfo->ucLenSizeMinusOne + 1;
    }

    // Replace NALU size field with SC in all NALU slices.
    while (i < (int)frameBytes)
    {
      //Write start code for every NALU length for H264
      const unsigned long startCode = 0x01000000;
      // Read NALU length.
      uint32 NALUnitLength = 0;
      int j = 0;

      for (; j < NALUnitFieldLen; ++j)
      {
        NALUnitLength = NALUnitLength << 8;
        NALUnitLength = NALUnitLength | m_pBuffer[i + j];
      }
      if((m_sampleInfo[streamNum].size < NALUnitLength) )
      {
        //NALU size from a given access unit can't be > the access unit size
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
                     "NALUnitLength %lu > Sample Size %lu",
                     NALUnitLength,m_sampleInfo[streamNum].size);
        *pulBufSize = 0;
        m_nextSample[streamNum] = m_sampleInfo[streamNum].sample + 1;
        return PARSER_ErrorZeroSampleSize;

      }
      //! Sometime, ZERO NALU Size occurs in between, ignore those NALs
      if(NALUnitLength)
      {
        (void) memmove (buf+bytesRead, &startCode, FOURCC_SIGNATURE_BYTES);
        bytesRead += FOURCC_SIGNATURE_BYTES;
        // Skip NALU length and copy over NALU contents.
        (void) memmove (buf+bytesRead, &(m_pBuffer [i + j]), NALUnitLength);
        //buffer += NALUnitLength;
        bytesRead += NALUnitLength;
      }
      i += j + NALUnitLength;
    }//while (i < frameBytes)
    //! If the whole frame contains "00 00 00 00", then treat it as EOF
    if (!bytesRead)
    {
      //NALU size from a given access unit can't be > the access unit size
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                   "All the NALUnitLength fields are ZERO, treat it as EOF");
      *pulBufSize = 0;
      m_nextSample[streamNum] = m_sampleInfo[streamNum].sample + 1;
      return PARSER_ErrorZeroSampleSize;
    }

    /* Update frameSize info with actual buffer size,
       which will be used to return*/
    frameSize = bytesRead;
  }//if(p_track->type == VIDEO_FMT_STREAM_VIDEO) && ..)

  m_nextSample[streamNum] = m_sampleInfo[streamNum].sample + 1;

  rulIndex = 0;
  *pulBufSize = frameSize;

  /* It is used to update offset values at the fragment boundary. In case of
     streaming, server will delete the fragments individually when playback
     is completed. So we need to update offset params in Videofmt Context to
     the start of next fragment. For this, we add MDAT atom size to the context
     offset which already points to the start of the MDAT atom. */
  if(m_bFragBoundary && m_bIsDashClip &&
     (0 == m_wBufferOffset ||
      m_wBufferOffset >  m_minOffsetRequired + MDAT_ATOM_SIGNATURE))
  {
    video_fmt_mp4r_context_type *context =
      (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
    boolean byteSwap = context->byte_swap_needed;
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "abs_size_retrieve_pos = %llu",
                 context->abs_size_retrieve_pos);
    uint64 offset = context->abs_size_retrieve_pos;
    uint8  buffer[MDAT_ATOM_SIGNATURE];
    uint32 num_bytes = MDAT_ATOM_SIGNATURE;
    num_bytes = readFile (m_parseFilePtr, buffer, offset, num_bytes);
    if(num_bytes)
    {
      uint32 atomType, atomSize;
      copyData((uint8*)&atomSize, buffer, byteSwap, 4);
      copyData((uint8*)&atomType, buffer + 4, byteSwap, 4);
      if(MDAT_TYPE == atomType)
      {
        /* If atom type is Version1, then size info will be available after
           signature bytes. 8Bytes are used for signature bytes. We have support
           to parse fragments less than 4GB. It means upper 4bytes will be ZERO
           by default. So skipping those 4 bytes directly and reading lower
           4bytes into atomSize param and update offset params accordingly. */
        if(1 == atomSize)
          copyData((uint8*)&atomSize, buffer + 12, byteSwap, 4);
        context->abs_pos = offset + atomSize;
        context->abs_size_retrieve_pos = offset + atomSize;
        m_bFragBoundary = false;
      }
    }
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
      "updated abs_size_retrieve_pos = %llu", context->abs_pos);
  }
  return PARSER_ErrorNone;
}

/* ======================================================================
FUNCTION
Mpeg4File::resetMediaPlayback

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
bool Mpeg4File::resetMediaPlayback(uint32 id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);
  if ( !p_track )
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "resetMediaPlayback, unknown track id = %lu", id);
    return false;
  }

  uint32 streamNum = p_track->stream_num;

  memset(&m_sampleInfo[streamNum], 0x0, sizeof(video_fmt_sample_info_type));
  memset(&m_nextSample[streamNum], 0x0, sizeof(uint32));
  return true;

}

/* <EJECT> */
/*===========================================================================

FUNCTION  getMediaTimestampForCurrentSample

DESCRIPTION
Public method used to request the timestamp for the sample currently processed

===========================================================================*/
uint64 Mpeg4File::getMediaTimestampForCurrentSample(uint32 id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);
  if ( !p_track )
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
              "getMediaTimestampForCurrentSample, unknown track id = %lu", id);
    return 0;
  }
  uint32 streamNum = p_track->stream_num;

  /* Calculate timestamp value in milli-sec units */
  uint64 time = (m_sampleInfo[streamNum].time -
                 m_baseTimeStamp[p_track->stream_num]);

  return time;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getSyncSampleInfo

DESCRIPTION
Public method used to request a media sample (frame)

===========================================================================*/
bool Mpeg4File::getSyncSampleInfo (uint32 streamNum,
                                   uint64 sampleNum,
                                   bool   reverse,
                                   video_fmt_sample_info_type *buffer)
{
  int loop = 0;

  m_videoFmtInfo.sync_sample_cb(  streamNum,
    sampleNum,
    (boolean)reverse,
    buffer,
    m_videoFmtInfo.server_data,
    mp4SyncStatusCallback,
    &(m_clientData[streamNum]));

  while ( (m_mp4SyncLastStatus[streamNum] != VIDEO_FMT_IO_DONE) &&
    (m_mp4SyncLastStatus[streamNum] != VIDEO_FMT_FAILURE) &&
    (m_mp4SyncLastStatus[streamNum] != VIDEO_FMT_BUSY) &&
    (m_mp4SyncLastStatus[streamNum] != VIDEO_FMT_DATA_CORRUPT) &&
    (loop < MPEG4_VIDEOFMT_MAX_LOOP) )
  {
    m_mp4SyncContinueCb[streamNum] (m_mp4SyncServerData[streamNum]);
    loop++;
  }

  if( loop >= MPEG4_VIDEOFMT_MAX_LOOP )
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
        "VideoFMT hangs. StreamNum=%lu, sampleNum %llu", streamNum, sampleNum);
  }

  if( (m_mp4SyncLastStatus[streamNum] == VIDEO_FMT_FAILURE) ||
    (m_mp4SyncLastStatus[streamNum] == VIDEO_FMT_BUSY) ||
    (m_mp4SyncLastStatus[streamNum] == VIDEO_FMT_DATA_CORRUPT))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "VideoFMT get sync sample failed.");
    return FALSE;
  }

  if(m_iodoneSize[streamNum] == 0)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "VideoFMT get sync sample could not find a sample.");
    return FALSE;
  }
  return TRUE;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getSampleAtTimestamp

DESCRIPTION
Public method used to get the sample info for a sample placed at a given
timestamp (in the track timescale).

===========================================================================*/

bool Mpeg4File::getSampleAtTimestamp(video_fmt_stream_info_type *p_track,
                                     uint64                      timestamp,
                                     bool                        lRewind,
                                     video_fmt_sample_info_type *sampleInfo)
{
  bool retStat = false;
  uint32 streamNum = p_track->stream_num;
  int32 maxFrames = (int32)p_track->frames;
  int32 curSample = (int32)m_sampleInfo[streamNum].sample;

  int32 reqSampleNum;
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;

  if ( lRewind )
  {

    for ( reqSampleNum = (int32)m_sampleInfo[streamNum].sample;
      reqSampleNum >= 0; --reqSampleNum )
    {
      retError = getSampleInfo(streamNum, reqSampleNum, 1, sampleInfo);
      if (PARSER_ErrorNone == retError)
      {
        retStat = true;
      }
      if ( sampleInfo->time == timestamp )
      {
        break;
      }
      else if(sampleInfo->time < timestamp)
      {
        /*
        * For text/audio track, we should not pick the sample whose TS >
        * what is being requested.
        * Otherwise, incorrect sample will be displayed for duration
        * which could be > sample's own duration. Example below:
        *
        * Lets say each text sample is of 5 seconds, and user does RW by
        * 2 seconds, when playback time is 5 seconds.
        * Now, if we pick sample#1, since it's TS (5 seconds) is > TS being
        * requested(3 seconds),
        * then sample#1 would be displayed for 7 seconds (5 second from it's
        * own duration + (5-3))
        *
        * Thus, picking sample#0 would make sure that it gets displayed only
        * from 2 seconds.
        */
        if( (curSample > reqSampleNum) &&
            (p_track->type == VIDEO_FMT_STREAM_VIDEO) )
        {
          (void)getSampleInfo(streamNum, reqSampleNum+1, 1, sampleInfo);
        }
        break;
      }
    } // for
  } // rewind
  else
  {   // forward
    for ( reqSampleNum = (int32)m_sampleInfo[streamNum].sample;
      reqSampleNum < maxFrames; ++reqSampleNum )
    {
      retStat = false;
      retError = getSampleInfo(streamNum, reqSampleNum,  1, sampleInfo);
      if(PARSER_ErrorNone == retError)
      {
        retStat = true;
      }
      if (  (sampleInfo->time==timestamp) ||
        /* for enhanced layer of temporal scalability clip we don't need this check
        since we always need to goto higher timestamp, so we make sure this
        check is only for text track */
        ( ((sampleInfo->time+sampleInfo->delta) > timestamp) &&
        (p_track->type == VIDEO_FMT_STREAM_TEXT) ) )
      {
        break;
      }
      else if(sampleInfo->time > timestamp)
      {
        /* don't forword after given time stamp, also don't decrease for forward */
        if((curSample < reqSampleNum) && p_track->type == VIDEO_FMT_STREAM_TEXT)
          (void)getSampleInfo(streamNum, (reqSampleNum-1), 1, sampleInfo);
        break;
      }
    } // for
  } // forward
  return retStat;
}

/*===========================================================================

FUNCTION  getTimestampedSampleInfo

DESCRIPTION
Public method used to get the sample corresponding to a given timestamp.

===========================================================================*/

bool Mpeg4File::getTimestampedSampleInfo(
  video_fmt_stream_info_type *p_track,
  uint64                      TimeStamp,
  video_fmt_sample_info_type *sampleInfo,
  uint64                     *newTimeStamp,
  bool                        bSetToSyncSample,
  uint64                      currentPosTimeStamp)
{
  // media timescale = number of time units per second
  // if media_timescale == 1000, the time units are miliseconds (1000/sec)
  uint64 timescaledTime = (TimeStamp*p_track->media_timescale)/1000;

  uint32 streamNum = p_track->stream_num;
  uint64 maxFrames = p_track->frames;

  uint32 sampleDelta = 1;
  uint64 reqSampleNum = 0;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  uint64 absFileOffset = 0;
  int64  sllDownloadedOffset = 0;
  bool   bEndOfData = false;
#endif // FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  int64 timeOffset = 0;
  bool  lRewind = false;
  bool retStat = false;
  if(maxFrames == 0)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL, "No valid frames for given track %lu",p_track->track_id);
    return false;
  }

  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;

  if( (m_nextSample[streamNum]==0) && (m_sampleInfo[streamNum].size==0) && (p_track->dec_specific_info.obj_type!=(int)MPEG4_IMAGE) )
  {
    /* we have not read any sample yet. So atleast read first good sample */
    uint32 sampleId = 0;
    do
    {
      retError = getSampleInfo(streamNum, reqSampleNum, 1, &m_sampleInfo[streamNum]);
      if(PARSER_ErrorNone == retError)
      {
        retStat = true;
      }
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "Reading first valid sampleInfo. SampleId=%lu",sampleId);
      sampleId++;
    }while(retStat && (m_mp4ReadLastStatus[streamNum] == VIDEO_FMT_IO_DONE)
      && (!m_sampleInfo[streamNum].delta || !m_sampleInfo[streamNum].size));

    if(!retStat || (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_IO_DONE))
      return false;
  }

  timeOffset = m_sampleInfo[streamNum].time - timescaledTime;

  if ( timeOffset == 0 )
  {
    *sampleInfo = m_sampleInfo[streamNum];
    if(sampleInfo->sync)
      return true;
  }

  /* if all video frames are SYNC frames, we don't have to search for sync frame
  repositioing will work same as audio where all frames are always sync frames */
  if((m_allSyncVideo) || (p_track->type == VIDEO_FMT_STREAM_AUDIO))
    bSetToSyncSample = false;

  if( (p_track->type == VIDEO_FMT_STREAM_VIDEO) &&
    bSetToSyncSample)
  {
    uint64 frameTime;
    bool userRewind;
    uint64 scaledCurPosTime = (currentPosTimeStamp*p_track->media_timescale)/1000;

    /* Special case: if user wants to goto beginning of the case, just jump to first frame */
    if(timescaledTime == 0)
    {
      retError = getSampleInfo(streamNum, 0, 1, sampleInfo);
      if(PARSER_ErrorNone == retError)
      {
        return true;
      }
      else
      {
        return false;
      }
    }

    userRewind = (scaledCurPosTime > timescaledTime)?true:false;

    /* first estimate on which frame we have to go assuming fixed delta between video
    frames. This will take us may be ahead or before the desired time, but from that
    point we can either FF or REW by checking each I-Frame to get the exact reposition
    point (means closest I-Frame). This will reduce time in finding closest I-Frame, if
    user is doing very long seek (say 50 minutes)
    */

    /* if we have to seek more than pre-defined limit (5*60 secs), calculate approximately how many
    sample we have to rewind or forward assuming sample time delta for a track is fixed */
    if( m_sampleInfo[streamNum].delta && ((uint32)abs((int)(TimeStamp-currentPosTimeStamp))>m_defaultCoarseJumpLimit) )
    {
      sampleDelta = (uint32)(abs((int)timeOffset) / m_sampleInfo[streamNum].delta);
      if(timeOffset > 0)
      {
        //rewind
        if(m_sampleInfo[streamNum].sample > sampleDelta)
        {
          reqSampleNum = m_sampleInfo[streamNum].sample - sampleDelta;
        }
        else
        {
          reqSampleNum = 0;
        }
      }
      else
      {
        //forward
        if((m_sampleInfo[streamNum].sample + sampleDelta) >= (p_track->frames-1))
        {
          reqSampleNum = p_track->frames-1;
        }
        else
        {
          reqSampleNum = m_sampleInfo[streamNum].sample + sampleDelta;
        }
      }

      retStat = false;
      retError = getSampleInfo(streamNum, reqSampleNum, 1, sampleInfo);
      if(PARSER_ErrorNone == retError)
      {
        retStat = true;
        if( (sampleInfo->time==timescaledTime) && (sampleInfo->sync!=TRUE) )
        {
          /* if it is not a sync sample, just change the time stamp, so that we continue to
          search for correct SYNC sample. For this if it is REW, we will increase the
          sample. Also if we have reached zero sample, we can only increase it */
          if(userRewind || !sampleInfo->sample)
            (void)getSampleInfo(streamNum, sampleInfo->sample+1, 1, sampleInfo);
          else
            (void)getSampleInfo(streamNum, sampleInfo->sample-1, 1, sampleInfo);
        }
      }
      else
      {
        reqSampleNum = m_sampleInfo[streamNum].sample;
        *sampleInfo = m_sampleInfo[streamNum];
      }
    }
    else
    {
      reqSampleNum = m_sampleInfo[streamNum].sample;
      *sampleInfo = m_sampleInfo[streamNum];
    }

    lRewind = (sampleInfo->time > timescaledTime)?true:false;
    if(sampleInfo->time == timescaledTime)
      lRewind = userRewind;

    frameTime = sampleInfo->time;
    retStat = true;

    /* loop till we get the desired time stamp */
    if(lRewind)
    {
      for( ; frameTime>timescaledTime && retStat ; )
      {
        retStat = getSyncSampleInfo(streamNum, reqSampleNum, lRewind, sampleInfo);
        frameTime = sampleInfo->time;

        /* user has actually pressed FF, but we are rewinding, then we should
        not go beyond current displaty time We also should make sure we are
           beyond the time user has asked, not before */
        if(!userRewind)
        {
          if(scaledCurPosTime >= frameTime ||(timescaledTime > frameTime))
          {
            /* means we have gone before current position, but user has pressed FF,
            so we should look I-Frame in FF direction */
            reqSampleNum = sampleInfo->sample+1;
            lRewind = false;
            break;
          }
        }

        if(sampleInfo->sample > 0)
          reqSampleNum = sampleInfo->sample-1;
        else
        {
          reqSampleNum = 0;
          break;
        }
      }
      /* if in this rewind case, go to the first frame */
      if(!retStat && (m_mp4SyncLastStatus[streamNum]==VIDEO_FMT_IO_DONE))
      {
        reqSampleNum = 0;
        retStat = false;
        retError = getSampleInfo(streamNum, reqSampleNum, 1, sampleInfo);
        if(PARSER_ErrorNone == retError)
        {
          retStat = true;
        }
      }
    }

    /* FF case */
    if(!lRewind)
    {
      for( ; frameTime<timescaledTime && retStat; )
      {
        retStat = getSyncSampleInfo(streamNum, reqSampleNum, lRewind, sampleInfo);
        frameTime = sampleInfo->time;

        /* if we have found the frame, so we don't need to go further */
        if(frameTime>=timescaledTime && retStat)
        {
          break;
        }
        /*if there are no sync samples further then set the time stamp to the
        previous sync sapmple. if the last sample is a sync sample then
        set the time stamp to that sync sample. */
        if((retStat == false && sampleInfo->sync) ||
           (retStat  && (sampleInfo->sample == (maxFrames-1))))
        {
          *newTimeStamp = (sampleInfo->time * 1000) / p_track->media_timescale;
          /* check if we got any sync sample later than current sample */
          if(*newTimeStamp > currentPosTimeStamp)
          {
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
            if( bHttpStreaming)
            {
              if(m_pStreamPort)
              {
                //Pull interface so pull dnld data size from OEM
                m_pStreamPort->GetAvailableOffset(&sllDownloadedOffset,
                                                  &bEndOfData);
                m_wBufferOffset = sllDownloadedOffset;
              }
              /*Fetch the abs file offset for the sample at new time stamp*/
              absFileOffset = getSampleAbsOffset (streamNum, sampleInfo->offset, sampleInfo->size);

              /*
              * When GetOffsetForTime is being invoked,we don't need to make sure that
              * there is sufficient data available corresponding to sample at new timestamp.
              * This is because, GetOffsetForTime API is called
              * to determine how much data should be available for given sample timestamp.
              */

              if( ( absFileOffset && absFileOffset < m_wBufferOffset) ||
                  (m_bTimeToOffsetInvoked) )
              {
                return true;
              }
            }
            else
#endif // FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
            {
              return true;
            }
          }
        }

        if(sampleInfo->sample >= (maxFrames-1))
        {
          retStat = false;
          break;
        }
        reqSampleNum = sampleInfo->sample+1;
      }
    /* in FF case, if we did not find any SYNC sample, then we try to find SYNC sample in reverse
          direction. If we get any SYNC sample and its time stamp is ahead of current position, we
          should FF upto that sample. This is similar logic as above which does not work if we did not
          find any SYNC sample after reqSampleNum frame */
      if((retStat == false) && !sampleInfo->sync )
      {
        /* find SYNC sample in reverse direction */
        retStat = getSyncSampleInfo(streamNum, reqSampleNum, TRUE, sampleInfo);
        if(retStat)
        {
          *newTimeStamp =(sampleInfo->time * 1000) / p_track->media_timescale;
          /* check if we got any sync sample later than current sample */

          //TO DO: AS of now, we don't know how to fail a seek on win mobile.
          //Thus for clips, when there is no I frame in forward direction, we should let it
          //search backward all the way to the first I frame even though it's TS < current playback time.
          //Once we figure out how to fail the seek, we can put back original code.
          //if(*newTimeStamp > currentPosTimeStamp)
          if(1)
          {
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
            if( bHttpStreaming)
            {
              if(m_pStreamPort)
              {
                //Pull interface so pull dnld data size from OEM
                m_pStreamPort->GetAvailableOffset(&sllDownloadedOffset,
                                                  &bEndOfData);
                m_wBufferOffset = (uint64)sllDownloadedOffset;
              }
              /*Fetch the abs file offset for the sample at new time stamp*/
              absFileOffset = getSampleAbsOffset (streamNum, sampleInfo->offset, sampleInfo->size);
              /*
              * When GetOffsetForTime is being invoked,we don't need to make sure that
              * there is sufficient data available corresponding to sample at new timestamp.
              * This is because, GetOffsetForTime API is called
              * to determine how much data should be available for given sample timestamp.
              */
              if( (absFileOffset && absFileOffset < m_wBufferOffset)||
                  (m_bTimeToOffsetInvoked) )
              {
                return true;
              }
            }
            else
#endif // FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
            {
              return true;
            }
          }
          else
            retStat = false;
        }
      }
    }
    /* see if we were successful */
    if(retStat)
    {
      *newTimeStamp = (sampleInfo->time * 1000) / p_track->media_timescale;
      /* if we end up on the same frame, tell repositioning failed, this will prevent
      restarting the same frame again */
      if(*newTimeStamp == currentPosTimeStamp)
        retStat = false;

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
      if( bHttpStreaming && retStat )
      {
        if(m_pStreamPort)
        {
          //Pull interface so pull dnld data size from OEM
          m_pStreamPort->GetAvailableOffset((int64*)&m_wBufferOffset, (bool*)&bEndOfData);
        }

        /*Fetch the abs file offset for the sample at new time stamp*/
        absFileOffset = getSampleAbsOffset (streamNum, sampleInfo->offset, sampleInfo->size);

        /*
        * When GetOffsetForTime is being invoked,we don't need to make sure that
        * there is sufficient data available corresponding to sample at new timestamp.
        * This is because, GetOffsetForTime API is called
        * to determine how much data should be available for given sample timestamp.
        */

        if( (!m_bTimeToOffsetInvoked)  &&
            ( (!absFileOffset) || (absFileOffset >= m_wBufferOffset) ) )
        {
           retStat = false;
        }
      }
#endif // FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

    }
    return retStat;
  }

  /* calculate approximately how many sample we have to rewind or forward
  assuming sample time delta for a track is fixed */
  if (m_sampleInfo[streamNum].delta )
  {
    sampleDelta = (uint32)((abs((int)timeOffset)) / m_sampleInfo[streamNum].delta);
  }

  if ( timeOffset > 0 )
  {
    // rewind processing (current time > desired timestamp)
    //------------------
    lRewind = true;
    if ( m_sampleInfo[streamNum].sample >= sampleDelta )
    {
      reqSampleNum = m_sampleInfo[streamNum].sample - sampleDelta;
    }
    else
    {
      reqSampleNum = 0;
    }
    if(timescaledTime == 0)
    {
      /* if the request is to reposition to 0 ms
      * insure that the clip repositions to the first
      * sample
      */
      reqSampleNum = 0;
    }
  }
  else
  {    // timeOffset < 0
    // fast forward processing (current time < desired timestamp)
    //------------------------
    reqSampleNum = m_sampleInfo[streamNum].sample + sampleDelta;

    if ( reqSampleNum >= maxFrames )
    {
      reqSampleNum = maxFrames - 1;
    }
  }

  if ( bSetToSyncSample )
  {
    retStat = getSyncSampleInfo(streamNum, reqSampleNum, lRewind, sampleInfo);
    /* if in this rewind case, only first frame is sync frame */
    if((timeOffset > 0) && (m_mp4SyncLastStatus[streamNum]==VIDEO_FMT_IO_DONE) &&
       (m_iodoneSize[streamNum]==0) )
    {
      reqSampleNum = 0;
      retStat = false;
      retError = getSampleInfo(streamNum, reqSampleNum, 1, sampleInfo);
      if(PARSER_ErrorNone == retError)
      {
        retStat = true;
      }
    }
  }
  else
  {
    if ( p_track->type == VIDEO_FMT_STREAM_AUDIO )
    {
      retStat = false;
      retError = getSampleInfo(streamNum, reqSampleNum, 1, sampleInfo);
      if(PARSER_ErrorNone == retError)
      {
        retStat = true;
      }
      MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
        "AUDIO CoarseRepos: reqSampleNum=%llu, sampleInfo.sample=%llu,\
        sampleInfo.time=%llu",
        reqSampleNum, sampleInfo->sample, sampleInfo->time);
      if(retStat && (sampleInfo->time!=timescaledTime))
      {
        //perform coarse repositioning..
        m_sampleInfo[streamNum] = *sampleInfo;
        if(sampleInfo->time < timescaledTime)
        {
          lRewind = false;
        }
        else
        {
          lRewind = true;
        }

        retStat = getSampleAtTimestamp (p_track, timescaledTime, lRewind,
                                        sampleInfo);
        MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
                     "AUDIO FineRepos: sampleInfo.sample=%llu, \
                     sampleInfo.time=%llu, lRewind=%d",
                       sampleInfo->sample, sampleInfo->time, lRewind);
      }

    }
    else if( p_track->type == VIDEO_FMT_STREAM_TEXT
      || p_track->type == VIDEO_FMT_STREAM_VIDEO )
    {
      retStat = getSampleAtTimestamp (p_track, timescaledTime, lRewind,
                                      sampleInfo);
    }
  }

  *newTimeStamp = (sampleInfo->time*1000)/ p_track->media_timescale;

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  if( bHttpStreaming && retStat )
  {
    if(m_pStreamPort)
    {
      //Pull interface so pull dnld data size from OEM
      m_pStreamPort->GetAvailableOffset((int64*)&m_wBufferOffset,
                                        (bool*)&bEndOfData);
    }

    /*Fetch the abs file offset for the sample at new time stamp*/
    absFileOffset = getSampleAbsOffset (streamNum, sampleInfo->offset,
                                        sampleInfo->size);

    /*
    * When GetOffsetForTime is being invoked,we don't need to make sure that
    * there is sufficient data available corresponding to sample at new timestamp.
    * This is because, GetOffsetForTime API is called
    * to determine how much data should be available for given sample timestamp.
    */
    if( ((!absFileOffset) || absFileOffset >= m_wBufferOffset) &&
        (!m_bTimeToOffsetInvoked) )
    {
      retStat = false;
    }
  }
#endif // FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

  return retStat;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  resetPlayback

DESCRIPTION
Public method used to get the sample corresponding to a given timestamp.

===========================================================================*/
void Mpeg4File::resetPlayback()
{
#ifdef FEATURE_MP4_CUSTOM_META_DATA
  if (_pTsmlAtom != NULL)
  {
    _pTsmlAtom->ResetTelopVectorIndex();
  }
#endif
  memset(&m_sampleInfo[0], 0x0,
         sizeof(video_fmt_sample_info_type)*VIDEO_FMT_MAX_MEDIA_STREAMS);
  memset(&m_nextSample[0], 0x0, sizeof(uint32)*VIDEO_FMT_MAX_MEDIA_STREAMS);
}

/* ======================================================================
FUNCTION
Mpeg4File::resetPlayback

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
uint64 Mpeg4File::resetPlayback (
                                 uint64 res_time,
                                 uint32 id,
                                 bool bSetToSyncSample,
                                 bool *bError,
                                 uint64 currentPosTimeStamp
                                 )
{
  *bError = FALSE;
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);
  if ( !p_track )
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "resetPlayback, unknown track id = %lu", id);
    *bError = TRUE;
    _fileErrorCode = PARSER_ErrorSeekFail;
    return 0;
  }

  /* Resetting the Error Code before processing the data */
  _fileErrorCode = PARSER_ErrorNone;

  uint32 streamNum = p_track->stream_num;

  video_fmt_sample_info_type  sampleInfo;
  memset(&sampleInfo, 0x0, sizeof(video_fmt_sample_info_type));

  uint64 modTS = res_time; // initial value
  m_bSeekDone = false; //reset the flag  before calling seek function

  bool retValue = getTimestampedSampleInfo (p_track, res_time, &sampleInfo,
                                            &modTS, bSetToSyncSample,
                                            currentPosTimeStamp);
  if(m_bIsDashClip)
  {
    video_fmt_mp4r_context_type *context  =
      (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
      "update both offsets in Videofmt Context:: size retrive pos %llu,\
       pos %llu",
      context->abs_size_retrieve_pos, context->abs_pos);
    context->abs_size_retrieve_pos = context->abs_pos;
    m_bFragBoundary = true;
  }
  if (retValue)
  {
    m_bSeekDone = true; //Mark Seek flag as true
    if(0 == m_nextSample[streamNum] && m_bIsDashClip)
    {
      memset(&m_sampleInfo[streamNum], 0, sizeof(video_fmt_sample_info_type));
    }
    if ( m_nextSample[streamNum] == sampleInfo.sample )
    {

     // re-assign the sampleInfo val in case of Seek because in seek scenario
     // I-Frame could be at the beginning of the fragment. In that case, we were
     // returning 0 timestamp always for Video track instead of actual value.
     // For audio track , there are no issue and returning the correct value.
      if(res_time > 0)
      {
        m_sampleInfo[streamNum] = sampleInfo;
        m_sampleInfo[streamNum].size = 0;
      }
      // this is to avoid unnecessary disturbance, when no repositioning is needed
      return modTS;
    }

    m_reposStreamPending        |= maskByte[streamNum];
    m_nextReposSample[streamNum] = sampleInfo.sample;
    m_sampleInfo[streamNum]      = sampleInfo;
    m_nextSample[streamNum]      = sampleInfo.sample;
    return modTS;
  }
  else
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
                 "Reposition failed for track id = %lu, _fileErrorCode %x",
                 id, _fileErrorCode);
    // this is to avoid unnecessary disturbance, when repositioning can not be done
    *bError = TRUE;

    /* If seek is failed with other errors than data under run or fragment
       underrun ,update fileErrorCode with common SeekFail Error Code */
    if( (_fileErrorCode != PARSER_ErrorDataUnderRun) &&
        (_fileErrorCode != PARSER_ErrorSeekUnderRunInFragment) )
    {
       _fileErrorCode = PARSER_ErrorSeekFail;
    }

    uint64 ullSampleTime;
    ullSampleTime = (m_sampleInfo[streamNum].time * TIMESCALE_BASE) /
                    p_track->media_timescale;
    return ullSampleTime;
  }

}

/* ======================================================================
FUNCTION
Mpeg4File::skipNSyncSamples

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
uint64 Mpeg4File::skipNSyncSamples(int offset, uint32 id, bool *bError, uint64 currentPosTimeStamp)
{

  int64 reqSampleNum = 0;
  int   noOfSyncSamplesSkipped = 0;
  bool result = false;
  uint64 newTimeStamp =0;

  /* Resetting the Error Code before processing the data */
  _fileErrorCode = PARSER_ErrorNone;

  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);
  if ( p_track )
  {
    int streamNum = p_track->stream_num;

    int64 maxFrames = p_track->frames;

    reqSampleNum = m_sampleInfo[streamNum].sample;

    video_fmt_sample_info_type  sampleInfo;
    memset(&sampleInfo, 0, sizeof(video_fmt_sample_info_type));

    if( offset < 0 )
    {
      // rewind case
      for(reqSampleNum = m_sampleInfo[streamNum].sample; reqSampleNum >= 0; --reqSampleNum)
      {

        if(getSyncSampleInfo (streamNum, (uint64)reqSampleNum, true, &sampleInfo))
        {
          noOfSyncSamplesSkipped++;
          if( noOfSyncSamplesSkipped == abs(offset) )
          {
            // In successfull case return the latest sync sample time
            result = true;
            break;
          }
        }
        else
        {
          // Not successfull in skipping desired sync samplese so return the old time.
           break;
        }
      }
    }
    else
    {
      // forward case
      for(reqSampleNum = m_sampleInfo[streamNum].sample; reqSampleNum<maxFrames; ++reqSampleNum)
      {
         if(getSyncSampleInfo( streamNum, (uint64)reqSampleNum, false, &sampleInfo ))
         {
           noOfSyncSamplesSkipped++;
           if( noOfSyncSamplesSkipped==offset )
           {
             // In successfull case return the last sync sample time
             result = true;
             break;
           }
         }
         else
         {
           // Not successfull in skipping desired sync samplese so return the old time.
           break;
         }
      }
    }

    // Check for The result
    if(result)
    {
      *bError =  false;
      newTimeStamp = (sampleInfo.time*1000)/p_track->media_timescale;

      m_reposStreamPending |= maskByte[streamNum];
      m_nextReposSample[streamNum] = sampleInfo.sample;
      m_sampleInfo[streamNum] = sampleInfo;

      MM_MSG_PRIO2( MM_FILE_OPS, MM_PRIO_HIGH,
                   "Time Stamp Returned after Skipping %d Sync Samples= %llu",
                     offset, newTimeStamp );
    }
    else
    {
      *bError =  true;
      _fileErrorCode = PARSER_ErrorSeekFail;
      MM_MSG_PRIO1( MM_FILE_OPS, MM_PRIO_HIGH,
            "seekToSync function Failed In Skipping %d Sync Samples", offset );
      newTimeStamp = currentPosTimeStamp;
    }
  }
  else
  {
    *bError =  true;
    _fileErrorCode = PARSER_ErrorSeekFail;
    MM_MSG_PRIO1( MM_FILE_OPS, MM_PRIO_HIGH,
                  "seekToSync function Failed for track ID %lu", id );
    newTimeStamp = currentPosTimeStamp;
  }
  return newTimeStamp;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  peekCurSample

DESCRIPTION

===========================================================================*/
PARSER_ERRORTYPE Mpeg4File::peekCurSample(uint32 trackid, file_sample_info_type *pSampleInfo)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(trackid);
  if ( !p_track  || !pSampleInfo )
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "getNextMediaSample, unknown track id = %lu", trackid);
    return PARSER_ErrorDefault;
  }

  uint32 ulDataSize = 0;
  uint32 ulOffset = 0;
  FileSourceDrmType drmtype = FILE_SOURCE_NO_DRM;
  uint32 streamNum = p_track->stream_num;

  //Simply return the current sample info without going to VideoFMT

  pSampleInfo->sample               = (uint32)m_sampleInfo[streamNum].sample;
  pSampleInfo->size                 = m_sampleInfo[streamNum].size;
  pSampleInfo->offset               = m_sampleInfo[streamNum].offset;
  pSampleInfo->time                 = m_sampleInfo[streamNum].time;
  //Every sample/AU always have valid timestamp
  pSampleInfo->btimevalid           = true;
  pSampleInfo->delta                = m_sampleInfo[streamNum].delta;
  pSampleInfo->sync                 = m_sampleInfo[streamNum].sync;
  pSampleInfo->sSubInfo.usSampleDescriptionIndex = (uint16)
                        m_sampleInfo[streamNum].sample_desc_index;
  /* mpeg4file returns one sample at a time */
  pSampleInfo->num_frames = 1;

  // Fill in changed sample information
  // Get Text sample entry data (tx3g).In case of timed text, there can be
  // multiple tx3g entry. When ever sample description index changes there
  // will be new set of tx3g entry applicable to group of samples.
  if( ( true == m_bSampleInfoChanged) &&
      ( VIDEO_FMT_STREAM_TEXT == p_track->type ) &&
      ( VIDEO_FMT_STREAM_TEXT_TIMEEDTEXT == p_track->subinfo.text.format ) )
  {
    //sample_desc_index will start from '1'.
    if( m_sampleInfo[p_track->stream_num].sample_desc_index )
    {
      ulOffset = m_sampleInfo[p_track->stream_num].sample_desc_index -1;
    }
    ulDataSize = GetData( DATA_ATOM_TX3G,
                          pSampleInfo->sSubInfo.sSubTitle.ucSubtitleSubInfo,
                          MAX_SUBTITLE_SUB_INFO_SIZE,
                          ulOffset);
    pSampleInfo->sSubInfo.sSubTitle.usSubtitleSubInfoSize = (uint16)ulDataSize;
  }
  // In case of SMPTE-TT, codec configuration is fixed through out the session.
  // If subtitle sample has subsample then, sub-sample box carry sub-sample
  // level information. ucSubtitleSubInfo[] will hold 'subs' box information
  // as it and FS-Client has to parse it to get sub-sample information.
  if( ( NULL != pSubsAtom) &&
      ( VIDEO_FMT_STREAM_TEXT_SMPTE_TIMED_TEXT == p_track->subinfo.text.format ))
  {
    ulDataSize = GetData( DATA_ATOM_SUBS,
                          pSampleInfo->sSubInfo.sSubTitle.ucSubtitleSubInfo,
                          MAX_SUBTITLE_SUB_INFO_SIZE,
                          ulOffset);
    pSampleInfo->sSubInfo.sSubTitle.usSubtitleSubInfoSize = (uint16)ulDataSize;
    pSampleInfo->sSubInfo.sSubTitle.ulSubSampleCount = pSubsAtom->GetSubSampleCount();
    pSampleInfo->sSubInfo.sSubTitle.eSubMnType = FILE_SOURCE_MN_TYPE_PNG;
    pSampleInfo->sSubInfo.sSubTitle.eCharCode = FS_SUBTITLE_CHAR_CODE_UTF8;
  }
  else
  {
    // There is no subsample associated with subtitle sample
    pSampleInfo->sSubInfo.sSubTitle.usSubtitleSubInfoSize = 0;
    pSampleInfo->sSubInfo.sSubTitle.ulSubSampleCount = 0;
    pSampleInfo->sSubInfo.sSubTitle.eSubMnType = FILE_SOURCE_MN_TYPE_UNKNOWN;
    pSampleInfo->sSubInfo.sSubTitle.eCharCode = FS_SUBTITLE_CHAR_CODE_UTF8;
  }
  // Reset flag for next sample
  m_bSampleInfoChanged = false;
  //! If content is DRM protected, update the auxiliary sample info.
  if(IsDRMProtection() &&
    (FILE_SOURCE_SUCCESS == GetDRMType(drmtype)) &&
    (drmtype == FILE_SOURCE_CENC_DRM) )
  {
    return getAuxiliarySampleInfo(trackid,streamNum, pSampleInfo);
  }
  return PARSER_ErrorNone;
}

/* <EJECT> */
/************************************************************************!
  @brief     Retrieve auxiliary sample information of supported
             DRM systems.

  @details   This API will provides  auxiliary sample information
             applicable to supported DRM system in same media
             file.It contains SystemID of the DRM system along with size of
             content protection system specific data for each supported
             DRM in sequential order.

  @param[in]      ulTrackID TrackId identifying the media track.
  @param[in]      streamNum identifying the stream number.
  @param[out]     pSampleInfo: Update the auxiliary sample info details
                  in sample info structure.

  @return         Returns PARSER_ErrorNone if DRM Auxiliary INFO available.
  ****************************************************************************/
PARSER_ERRORTYPE  Mpeg4File::getAuxiliarySampleInfo(uint32 trackid ,
                                                    uint32 streamNum,
                                                    file_sample_info_type *pSampleInfo)
{
  OSCL_FILE* localParseFilePtr = m_parseFilePtr;
  CSinfAtom* pSinfAtom = NULL;
  uint32 bytesConsumed = 0;
  bool byteSwap = TRUE;
  uint32 index = 0;

  if(pSampleInfo == NULL)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
             "getAuxiliarySampleInfo pSampleInfo is NULL");
    return PARSER_ErrorDefault;
  }

  //!Update the default values from SINF/TENC atom entries of the
  //!last stored PSSH Index.
  if((m_ulPSSHatomEntryCount) &&
     (m_aSINFatomEntryArray.GetLength() > m_ulSelectedDRMIdx))
  {
    pSinfAtom = m_aSINFatomEntryArray.ElementAt(m_ulSelectedDRMIdx);
  }

  if(pSinfAtom && pSinfAtom->m_pTencAtom)
  {
    pSampleInfo->sSubInfo.sCP.ucIsEncrypted =
      pSinfAtom->m_pTencAtom->m_usIsEncrypted;
    pSampleInfo->sSubInfo.sCP.ucKeyIDSize = (uint8)
      sizeof(pSinfAtom->m_pTencAtom->m_ucKeyID);
    pSampleInfo->sSubInfo.sCP.ucInitVectorSize =
      pSinfAtom->m_pTencAtom->m_ucIVSize;
    memcpy(pSampleInfo->sSubInfo.sCP.ucDefaultKeyID ,
      pSinfAtom->m_pTencAtom->m_ucKeyID,
      MAX_KID_SIZE);
    memcpy(pSampleInfo->sSubInfo.sCP.ucKeyID ,
      pSinfAtom->m_pTencAtom->m_ucKeyID,
      MAX_KID_SIZE);
  }

  //!Allocated the buffer to process the Auxiliary sample and make sure
  //!current buffer size is sufficient to hold the auxiliary sample.
  if( m_pSampleInfoBuffer)
  {
    //! Need to reallocate the memory
    if(m_nSampleInfoBufferSize < m_sampleInfo[streamNum].aux_sample_info_size)
    {
      m_pSampleInfoBuffer = (uint8*)MM_Realloc(m_pSampleInfoBuffer,
        m_sampleInfo[streamNum].aux_sample_info_size);
      if(!m_pSampleInfoBuffer)
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
          "Failed to re-allocate memory for m_pSampleInfoBuffer, size %lu",
          m_sampleInfo[streamNum].aux_sample_info_size );
        return PARSER_ErrorMemAllocFail;
      }
      //! Remember the size of the re-allocated buffer
      m_nSampleInfoBufferSize = m_sampleInfo[streamNum].aux_sample_info_size;
    }
  }
  else
  {
    m_pSampleInfoBuffer =
      (uint8*)MM_Malloc(m_sampleInfo[streamNum].aux_sample_info_size);
    //! Check for memory allocation failure
    if(!m_pSampleInfoBuffer)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
        "Failed to allocate memory for m_pSampleInfoBuffer, size %lu",
        m_sampleInfo[streamNum].aux_sample_info_size);
      return PARSER_ErrorMemAllocFail;
    }
    //! Remember the size of the buffer allocated.
    m_nSampleInfoBufferSize = m_sampleInfo[streamNum].aux_sample_info_size;
  }

#ifdef FEATURE_FILESOURCE_MP4_PARSER_DEBUG
 MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
               "Sample Number %llu,\
                Auxiliary SampleInfo Offset %llu,\
                Auxiliary SampleInfo Size %lu",
                m_sampleInfo[streamNum].sample,
                m_sampleInfo[streamNum].aux_sample_info_offset,
                m_sampleInfo[streamNum].aux_sample_info_size);
#endif

  //! Copy the Auxiliary Sample data into the m_pSampleInfoBuffer.
  uint32 ulDataRead = readFile (localParseFilePtr,
                                m_pSampleInfoBuffer,
                                m_sampleInfo[streamNum].aux_sample_info_offset,
                                m_sampleInfo[streamNum].aux_sample_info_size);
  if(!ulDataRead)
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
                 "getAuxSampleInfo read failed for sample num %llu at offset %llu",
                 m_sampleInfo[streamNum].sample,
                 m_sampleInfo[streamNum].aux_sample_info_offset);
    return PARSER_ErrorReadFail;
  }

  //! Read Initialization Vector.
  if(pSampleInfo->sSubInfo.sCP.ucInitVectorSize == 16)
  {
    memcpy(pSampleInfo->sSubInfo.sCP.ucInitVector,
           m_pSampleInfoBuffer+bytesConsumed,
           16);
    bytesConsumed += 16;
  }
  else
  {
    memcpy(pSampleInfo->sSubInfo.sCP.ucInitVector,
           m_pSampleInfoBuffer+bytesConsumed,
           8);
    bytesConsumed += 8;
  }
#ifdef FEATURE_FILESOURCE_MP4_PARSER_DEBUG
  uint8 *ucIV = pSampleInfo->sSubInfo.sCP.ucInitVector;
  for(index = 0; index < pSampleInfo->sSubInfo.sCP.ucInitVectorSize;)
  {
    MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_HIGH,
      "InitVector Info %x,%x,%x,%x",
        ucIV[index],ucIV[index+1],ucIV[index+2],ucIV[index+3]);
    index +=4;
  }
#endif

  if(m_sampleInfo[streamNum].aux_sample_info_size >
    pSampleInfo->sSubInfo.sCP.ucInitVectorSize )
  {
    uint8 NALUnitFieldLen = 0, AdjustedNALUnitFieldLength =0 ,
      AdjustedNALUnitFieldOffset = 0;
    uint32 subSampleOffset = 0;

    //! Read Number of subSample.
    copyData((uint8*)&pSampleInfo->sSubInfo.sCP.ulEncrSubSampleCount,
              m_pSampleInfoBuffer+bytesConsumed,
              byteSwap,
              2);
    bytesConsumed += 2;

    //! Get the NAL Unit length field length.
    NALUnitFieldLen = (uint8)GetSizeOfNALLengthField(trackid);

    //! For H.264 codec frames, we are adding startCode data and subtracting
    //! the NALUnitLength field info. If NALUnitLength field is not equal to
    //! 4 bytes, then output buffer size will not be same as actual sample
    //! size. To handle such case, this change is required.
    if( NALUnitFieldLen < 4 )
    {
      AdjustedNALUnitFieldLength = (uint8)(4 - NALUnitFieldLen);
    }

    for(int ulSubIndex =0;
        (ulSubIndex < pSampleInfo->sSubInfo.sCP.ulEncrSubSampleCount);
         ulSubIndex++)
    {
      FS_ENCRYPTED_SUBSAMPLE_INFOTYPE *pEncSubsampleInfoData =
        &pSampleInfo->sSubInfo.sCP.sEncSubsampleInfo[ulSubIndex];

      //! Copy the clear data size.
      copyData((uint8*)&pEncSubsampleInfoData->usSizeOfClearData,
               m_pSampleInfoBuffer+bytesConsumed,
               byteSwap,
               2);
      bytesConsumed += 2;

      //! Update the clear data offset.
      pEncSubsampleInfoData->ulOffsetClearData = subSampleOffset;
      subSampleOffset += pEncSubsampleInfoData->usSizeOfClearData;

      //!SubSampleOffset will also increase by the AdjustedNALUnitFieldLength.
      subSampleOffset += AdjustedNALUnitFieldLength;

      //! clear data Size will also increase by the AdjustedNALUnitFieldLength.
      pEncSubsampleInfoData->usSizeOfClearData = (uint16)
      (AdjustedNALUnitFieldLength + pEncSubsampleInfoData->usSizeOfClearData);

      //! Copy the encrypted data size.
      copyData((uint8*)&pEncSubsampleInfoData->ulSizeOfEncryptedData,
                 m_pSampleInfoBuffer+bytesConsumed,
                 byteSwap, 4);
      bytesConsumed += 4;

#ifdef FEATURE_FILESOURCE_MP4_PARSER_DEBUG
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
               "AuxSubSample Clear DataSize %u,\
                AuxSubSample Encrypted DataSize %lu",\
                pEncSubsampleInfoData->usSizeOfClearData,
                pEncSubsampleInfoData->ulSizeOfEncryptedData);
#endif
      if(pEncSubsampleInfoData->ulSizeOfEncryptedData)
      {
        //! Update the encrypted data offset.
        pEncSubsampleInfoData->ulOffsetEncryptedData = subSampleOffset;
        subSampleOffset += pEncSubsampleInfoData->ulSizeOfEncryptedData;
      }
    }//for
  }//if(aux_size > IV Size)
  return PARSER_ErrorNone;
}
#ifdef FEATURE_MP4_CUSTOM_META_DATA
/* <EJECT> */
/*===========================================================================

FUNCTION  IsTelopPresent

DESCRIPTION
returns TRUE if telop is present, otherwise FALSE.

===========================================================================*/
bool Mpeg4File::IsTelopPresent()
{
  if (_pTsmlAtom != NULL)
  {
    return true;
  }
  return false;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getTelopTrackDuration

DESCRIPTION
returns the duration of telop track.

===========================================================================*/
uint32 Mpeg4File::getTelopTrackDuration()
{
  if (_pTsmlAtom != NULL)
  {
    return(_pTsmlAtom->GetTelopTrackDuration());
  }
  return 0;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getNextTelopElement

DESCRIPTION
returns next telop element

===========================================================================*/
TelopElement *Mpeg4File::getNextTelopElement()
{
  if (_pTsmlAtom != NULL)
  {
    return(_pTsmlAtom->GetNextTelopElement());
  }
  return NULL;
}
/* <EJECT> */
/*===========================================================================

FUNCTION  getTelopHeader

DESCRIPTION
returns telop header

===========================================================================*/
TelopHeader *Mpeg4File::getTelopHeader()
{
  if (_pTsmlAtom != NULL)
  {
    return(_pTsmlAtom->GetTelopHeader());
  }
  return NULL;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  resetTelopPlayback

DESCRIPTION
resents telop timed text play back.

===========================================================================*/
uint32 Mpeg4File::resetTelopPlayback(uint32 startPos)
{
  if (_pTsmlAtom != NULL)
  {
    (void)_pTsmlAtom->ResetTelopVectorIndexByTime((int32)startPos);
  }
  return startPos;
}
#endif /* FEATURE_MP4_CUSTOM_META_DATA */

#define DEFAULT_UUID_ATOM_HEADER_SIZE 24
#define DEFAULT_UDTA_CHILD_ATOM_HEADER_SIZE 8
/* <EJECT> */
/*===========================================================================

FUNCTION  mp4ParseUUIDAtom

DESCRIPTION
This private function create UUID atom by parsing media file.

===========================================================================*/
void Mpeg4File::mp4ParseUUIDAtom(video_fmt_uuid_data_type *pAtomInfo,OSCL_FILE* localFilePtr)
{
  Atom * entry = NULL ;
  uint64 saveFilePos = OSCL_FileTell (localFilePtr);

  /* videoFMT offset is after atom header, but current parsing needs
     atom header as well, so goback 24 bytes */
  if (pAtomInfo->offset > DEFAULT_UUID_ATOM_HEADER_SIZE)
    (void)OSCL_FileSeek(localFilePtr, pAtomInfo->offset-DEFAULT_UUID_ATOM_HEADER_SIZE, SEEK_SET);

#ifdef FEATURE_MP4_CUSTOM_META_DATA
  switch (pAtomInfo->atom_type)
  {
    case KDDI_DRM_ATOM:
      if(_kddiDRMAtom)
        MM_Delete( _kddiDRMAtom );
      _kddiDRMAtom = MM_New_Args( KDDIDrmAtom, (localFilePtr) );
      break;

    case KDDI_CONTENT_PROPERTY_ATOM:
      if(_kddiContentPropertyAtom)
        MM_Delete( _kddiContentPropertyAtom );
      _kddiContentPropertyAtom = MM_New_Args( KDDIContentPropertyAtom, (localFilePtr) );
      break;

    case KDDI_MOVIE_MAIL_ATOM:
      if(_kddiMovieMailAtom)
        MM_Delete( _kddiMovieMailAtom );
      _kddiMovieMailAtom = MM_New_Args( KDDIMovieMailAtom, (localFilePtr) );
      break;

    case KDDI_ENCODER_INFO_ATOM:
      if(_kddiEncoderInformationAtom)
        MM_Delete( _kddiEncoderInformationAtom );
      _kddiEncoderInformationAtom = MM_New_Args( KDDIEncoderInformationAtom, (localFilePtr) );
      break;

    case KDDI_GPS_ATOM:
      if(_kddiGPSAtom)
        MM_Delete( _kddiGPSAtom );
      _kddiGPSAtom = MM_New_Args( KDDIGPSAtom, (localFilePtr) );
      break;
    case KDDI_TELOP_ATOM:
      //Telop parser lib needs to be called here.
      if(_kddiTelopElement)
        MM_Delete( _kddiTelopElement );
      _kddiTelopElement = MM_New_Args( KDDITelopAtom, (localFilePtr) );
      break;

    case MIDI_TYPE:
      /* for child videoFMT offset is after atom size and type, but current parsing needs
        atom header as well, so goback 8 bytes */
      if (pAtomInfo->offset > DEFAULT_UDTA_CHILD_ATOM_HEADER_SIZE)
        (void)OSCL_FileSeek(localFilePtr, pAtomInfo->offset-DEFAULT_UDTA_CHILD_ATOM_HEADER_SIZE, SEEK_SET);
      if(_midiAtom)
        MM_Delete( _midiAtom );
      _midiAtom = MM_New_Args( UdtaMidiAtom, (localFilePtr) );
      break;

    case LINK_TYPE:
      /* for child videoFMT offset is after atom size and type, but current parsing needs
        atom header as well, so goback 8 bytes */
      if (pAtomInfo->offset > DEFAULT_UDTA_CHILD_ATOM_HEADER_SIZE)
        (void)OSCL_FileSeek(localFilePtr, pAtomInfo->offset-DEFAULT_UDTA_CHILD_ATOM_HEADER_SIZE, SEEK_SET);
      if(_linkAtom)
        MM_Delete( _linkAtom );
      _linkAtom = MM_New_Args( UdtaLinkAtom, (localFilePtr) );
      break;

    default:
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "Unsupp atom %x", (unsigned int)pAtomInfo->atom_type );
      break;
  }
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
  /* videoFMT offset is after atom header, but current parsing needs
     atom header as well, so goback 24 bytes */
  if (pAtomInfo->offset > DEFAULT_UDTA_CHILD_ATOM_HEADER_SIZE)
    (void)OSCL_FileSeek(localFilePtr, pAtomInfo->offset - DEFAULT_UDTA_CHILD_ATOM_HEADER_SIZE, SEEK_SET);

  entry = MM_New_Args( Atom, (localFilePtr) );
  if(entry != NULL)
  {
    if (!entry->FileSuccess())
    {
      MM_Delete( entry );
      /* Here even though the UDTA child is corrupted let the playback continue */
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "UUID atom parsing failed");
    }
    else
    {
      entry->setParent(this);
      UUIDatomEntryArray += entry;
      UUIDatomEntryCount++;
    }
  }
  (void)OSCL_FileSeek(localFilePtr, saveFilePos, SEEK_SET);
}

/*===========================================================================

FUNCTION  getNumUUIDAtom

DESCRIPTION
returns the UUID atom count

===========================================================================*/
uint32 Mpeg4File::getNumUUIDAtom()
{
  return UUIDatomEntryCount;
}

#ifdef FEATURE_MP4_CUSTOM_META_DATA
/*---------------------------------- KDDI Meta Data Related APIs --------------------------------*/

/* <EJECT> */
/*===========================================================================

FUNCTION  getCopyProhibitionFlag

DESCRIPTION
returns Copy Prohibition Flag value from CopyGaurdAtom

===========================================================================*/
uint32 Mpeg4File::getCopyProhibitionFlag()
{
  if (_kddiDRMAtom != NULL)
  {
    return(_kddiDRMAtom->getCopyProhibitionFlag());
  }
  return(0);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getValidityEffectiveDate

DESCRIPTION
returns Validity Start Date from CopyGaurdAtom

===========================================================================*/
uint32 Mpeg4File::getValidityEffectiveDate()
{
  if (_kddiDRMAtom != NULL)
  {
    return(_kddiDRMAtom->getValidityEffectiveDate());
  }
  return(0);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getValidityPeriod

DESCRIPTION
returns Validity Period from Validity Start Date from CopyGaurdAtom

===========================================================================*/
uint32 Mpeg4File::getValidityPeriod()
{
  if (_kddiDRMAtom != NULL)
  {
    return(_kddiDRMAtom->getValidityPeriod());
  }
  return(0);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getNumberofAllowedPlayBacks

DESCRIPTION
returns number of allowed playback from CopyGaurdAtom

===========================================================================*/
uint32 Mpeg4File::getNumberofAllowedPlayBacks()
{
  if (_kddiDRMAtom != NULL)
  {
    return(_kddiDRMAtom->getNumberofAllowedPlayBacks());
  }
  return(0);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getContentPropertyTitle

DESCRIPTION
returns title of content from Content Property Atom

===========================================================================*/
FILESOURCE_STRING Mpeg4File::getContentPropertyTitle()
{
  FILESOURCE_STRING temp(_T(""));
  if (_kddiContentPropertyAtom != NULL)
  {
    return(_kddiContentPropertyAtom->getContentPropertyTitle());
  }
  return(temp);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getContentPropertyCopyRight

DESCRIPTION
returns Copy Right string from Content Property Atom

===========================================================================*/
FILESOURCE_STRING Mpeg4File::getContentPropertyCopyRight()
{
  FILESOURCE_STRING temp(_T(""));
  if (_kddiContentPropertyAtom != NULL)
  {
    return(_kddiContentPropertyAtom->getContentPropertyCopyRight());
  }
  return(temp);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getContentPropertyAuthor

DESCRIPTION
returns content author from Content Property Atom

===========================================================================*/
FILESOURCE_STRING Mpeg4File::getContentPropertyAuthor()
{
  FILESOURCE_STRING temp(_T(""));
  if (_kddiContentPropertyAtom != NULL)
  {
    return(_kddiContentPropertyAtom->getContentPropertyAuthor());
  }
  return(temp);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getContentPropertyMemo

DESCRIPTION
returns content memo from Content Property Atom

===========================================================================*/
FILESOURCE_STRING Mpeg4File::getContentPropertyMemo()
{
  FILESOURCE_STRING temp(_T(""));
  if (_kddiContentPropertyAtom != NULL)
  {
    return(_kddiContentPropertyAtom->getContentPropertyMemo());
  }
  return(temp);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getAuthorDLLVersion

DESCRIPTION
returns DLL version of content authoring tool from Content Property Atom

===========================================================================*/
uint32 Mpeg4File::getAuthorDLLVersion()
{
  if (_kddiContentPropertyAtom != NULL)
  {
    return(_kddiContentPropertyAtom->getAuthorDLLVersion());
  }
  return(0);
}


/* <EJECT> */
/*===========================================================================

FUNCTION  getEditFlags

DESCRIPTION
returns edit flags from Movie Mail Atom

===========================================================================*/
uint32 Mpeg4File::getEditFlags()
{
  if (_kddiMovieMailAtom != NULL)
  {
    return(_kddiMovieMailAtom->getEditFlags());
  }
  return(0);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getRecordingMode

DESCRIPTION
returns recording mode from Movie Mail Atom

===========================================================================*/
uint8 Mpeg4File::getRecordingMode()
{
  if (_kddiMovieMailAtom != NULL)
  {
    return(_kddiMovieMailAtom->getRecordingMode());
  }
  return(0);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getRecordingDate

DESCRIPTION
returns recording date from Movie Mail Atom

===========================================================================*/
uint32 Mpeg4File::getRecordingDate()
{
  if (_kddiMovieMailAtom != NULL)
  {
    return(_kddiMovieMailAtom->getRecordingDate());
  }
  return(0);
}


/* <EJECT> */
/*===========================================================================

FUNCTION  getDeviceName

DESCRIPTION
returns encoding device from Encoder Information Atom

===========================================================================*/
uint8*  Mpeg4File::getDeviceName() const
{
  if (_kddiEncoderInformationAtom != NULL)
  {
    return(_kddiEncoderInformationAtom->getDeviceName());
  }
  return(NULL);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getModelName

DESCRIPTION
returns encoding device model name from Encoder Information Atom

===========================================================================*/
uint8*  Mpeg4File::getModelName() const
{
  if (_kddiEncoderInformationAtom != NULL)
  {
    return(_kddiEncoderInformationAtom->getModelName());
  }
  return(NULL);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getEncoderInformation

DESCRIPTION
returns encoder information from Encoder Information Atom

===========================================================================*/
uint8*  Mpeg4File::getEncoderInformation() const
{
  if (_kddiEncoderInformationAtom != NULL)
  {
    return(_kddiEncoderInformationAtom->getEncoderInformation());
  }
  return(NULL);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getMuxInformation

DESCRIPTION
returns encoding multiplexer information from Encoder Information Atom

===========================================================================*/
uint8*  Mpeg4File::getMuxInformation() const
{
  if (_kddiEncoderInformationAtom != NULL)
  {
    return(_kddiEncoderInformationAtom->getMuxInformation());
  }
  return(NULL);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getTelopInformationString

DESCRIPTION
returns telop information from Telop Atom

===========================================================================*/
uint8*  Mpeg4File::getTelopInformationString() const
{
  if (_kddiTelopElement != NULL)
  {
    return(_kddiTelopElement->getTelopInformationString());
  }
  return(NULL);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getTelopInformationSize

DESCRIPTION
returns telop information size from Telop Atom

===========================================================================*/
uint32  Mpeg4File::getTelopInformationSize()
{
  if (_kddiTelopElement != NULL)
  {
    return(_kddiTelopElement->getTelopInformationSize());
  }
  return(0);
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getGPSByteOrder

DESCRIPTION
returns GPS Byte Order from GPS Atom

===========================================================================*/
uint16 Mpeg4File::getGPSByteOrder()
{
  if (_kddiGPSAtom != NULL)
  {
    return(_kddiGPSAtom->getGPSByteOrder());
  }
  return 0;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getGPSVersionID

DESCRIPTION
returns GPS Version ID from GPS Atom

===========================================================================*/
uint32  Mpeg4File::getGPSVersionID()
{
  if (_kddiGPSAtom != NULL)
  {
    return(_kddiGPSAtom->getVersionID());
  }
  return 0;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getGPSLatitudeRef

DESCRIPTION
returns GPS Latitude Reference from GPS Atom

===========================================================================*/
uint32  Mpeg4File::getGPSLatitudeRef()
{
  if (_kddiGPSAtom != NULL)
  {
    return(_kddiGPSAtom->getLatitudeRef());
  }
  return 0;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getGPSLongitudeRef

DESCRIPTION
returns GPS Longitude Reference from GPS Atom

===========================================================================*/
uint32  Mpeg4File::getGPSLongitudeRef()
{
  if (_kddiGPSAtom != NULL)
  {
    return(_kddiGPSAtom->getLongitudeRef());
  }
  return 0;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getGPSAltitudeRef

DESCRIPTION
returns GPS Altitude Reference from GPS Atom

===========================================================================*/
uint32  Mpeg4File::getGPSAltitudeRef()
{
  if (_kddiGPSAtom != NULL)
  {
    return(_kddiGPSAtom->getAltitudeRef());
  }
  return 0;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getGPSLatitudeArray

DESCRIPTION
returns GPS Latitude Array from GPS Atom

===========================================================================*/
uint64 *Mpeg4File::getGPSLatitudeArray()
{
  if (_kddiGPSAtom != NULL)
  {
    return(_kddiGPSAtom->getGPSLatitudeArray());
  }
  return NULL;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getGPSLongitudeArray

DESCRIPTION
returns GPS Longitude Array from GPS Atom

===========================================================================*/
uint64 *Mpeg4File::getGPSLongitudeArray()
{
  if (_kddiGPSAtom != NULL)
  {
    return(_kddiGPSAtom->getGPSLongitudeArray());
  }
  return NULL;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getGPSAltitude

DESCRIPTION
returns GPS Altitude from GPS Atom

===========================================================================*/
uint64 Mpeg4File::getGPSAltitude()
{
  if (_kddiGPSAtom != NULL)
  {
    return(_kddiGPSAtom->getGPSAltitude());
  }
  return 0;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getGPSTimeArray

DESCRIPTION
returns GPS Time Array from GPS Atom

===========================================================================*/
uint64 *Mpeg4File::getGPSTimeArray()
{
  if (_kddiGPSAtom != NULL)
  {
    return(_kddiGPSAtom->getGPSTimeArray());
  }
  return NULL;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getGPSSurveyData

DESCRIPTION
returns GPS Survey Data from GPS Atom

===========================================================================*/
FILESOURCE_STRING Mpeg4File::getGPSSurveyData()
{
  if (_kddiGPSAtom != NULL)
  {
    return(_kddiGPSAtom->getGPSSurveyData());
  }
  return NULL;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getPositoningMethod

DESCRIPTION
returns GPS Positoning Method from GPS Atom

===========================================================================*/
FILESOURCE_STRING Mpeg4File::getPositoningMethod()
{
  if (_kddiGPSAtom != NULL)
  {
    return(_kddiGPSAtom->getPositoningMethod());
  }
  return NULL;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getPositioningName

DESCRIPTION
returns GPS Positioning Name from GPS Atom

===========================================================================*/
FILESOURCE_STRING Mpeg4File::getPositioningName()
{
  if (_kddiGPSAtom != NULL)
  {
    return(_kddiGPSAtom->getPositioningName());
  }
  return NULL;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getGPSDate

DESCRIPTION
returns GPS Date string from GPS Atom

===========================================================================*/
FILESOURCE_STRING Mpeg4File::getGPSDate()
{
  if (_kddiGPSAtom != NULL)
  {
    return(_kddiGPSAtom->getGPSDate());
  }
  return NULL;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getGPSExtensionMapScaleInfo

DESCRIPTION
returns GPS Extension Map Scale Information from GPS Atom

===========================================================================*/
uint64 Mpeg4File::getGPSExtensionMapScaleInfo()
{
  if (_kddiGPSAtom != NULL)
  {
    return(_kddiGPSAtom->getGPSExtensionMapScaleInfo());
  }
  return 0;
}
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
/* <EJECT> */
/*===========================================================================

FUNCTION  getTextSampleEntryAt

DESCRIPTION

===========================================================================*/
TextSampleEntry *Mpeg4File::getTextSampleEntryAt (uint32 trackid, uint32 index)
{
  uint32 length = 0;

  video_fmt_stream_info_type *p_track = getTrackInfoForID(trackid);
  if ( !p_track )
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR, "getNextMediaSample, unknown track id = %lu", trackid);
    return NULL;
  }

  if( p_track->type == VIDEO_FMT_STREAM_TEXT )
  {
    length = textSampleEntryArray.GetLength();
    if( length >= index )
    {
      return textSampleEntryArray[index-1];
    }
  }

  return NULL;
}


/* ======================================================================
FUNCTION
Mpeg4File::ParseTimedTextAtom

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
void Mpeg4File::ParseTimedTextAtom(video_fmt_text_data_type *text_atom,OSCL_FILE* localFilePtr)
{
  uint64 saveFilePos = OSCL_FileTell (localFilePtr);

  /* videoFMT offset is after atom header, but current parsing needs
     atom header as well, so goback 8 bytes */
  if (text_atom->offset > DEFAULT_TEXT_ATOM_HEADER_SIZE)
    (void)OSCL_FileSeek(localFilePtr, (text_atom->offset-DEFAULT_TEXT_ATOM_HEADER_SIZE), SEEK_SET);
  DataT eTextDataType = GET_TEXT_DATATYPE(text_atom->format);

  TextSampleEntry * entry = MM_New_Args( TextSampleEntry, (localFilePtr,eTextDataType) );

  if(!entry)
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorMemAllocFail;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
      "Mpeg4File::ParseTimedTextAtom Memory allocation failure");
  }
  else if (!(entry->FileSuccess()))
  {
    _fileErrorCode = entry->GetFileError();
    _success = false;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
      "Mpeg4File::ParseTimedTextAtom TextSampleEntry failure");
    MM_Delete( entry );
  }
  else
  {
    entry->setParent(this);
    textSampleEntryArray += entry;
    textSampleEntryCount++;
  }

  (void)OSCL_FileSeek(localFilePtr, saveFilePos, SEEK_SET);
}

/* ======================================================================
FUNCTION
  Mpeg4File::GetNumTX3GAtom

DESCRIPTION

========================================================================== */
uint32 Mpeg4File::GetNumTX3GAtom()
{
  return textSampleEntryCount;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  ~Mpeg4File

DESCRIPTION
Destructor for the Mpeg4File class

===========================================================================*/
Mpeg4File::~Mpeg4File()
{
  uint32 i;

  if (m_mp4ParseEndCb)
  {
    m_mp4ParseEndCb (m_mp4ParseServerData);
    while ( (m_mp4ParseLastStatus != VIDEO_FMT_DONE) &&
      (m_mp4ParseLastStatus != VIDEO_FMT_FAILURE) )
    {
      if((m_mp4ParseContinueCb == NULL) ||
        (m_mp4ParseServerData == NULL))
        break;
      else
        m_mp4ParseContinueCb (m_mp4ParseServerData);
    }
    m_mp4ParseContinueCb = NULL;
    m_mp4ParseServerData = NULL;
    if (m_mp4ParseLastStatus == VIDEO_FMT_FAILURE)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Mpeg4File Destructor: VIDEO_FMT_FAILURE");
    }
  }

  for (i = 0; i < textSampleEntryCount; i++)
  {
    if((textSampleEntryArray)[i] &&
      (textSampleEntryArray)[i] != NULL)
    {
      MM_Delete( (textSampleEntryArray)[i] );
      (textSampleEntryArray)[i] = NULL;
    }
  }
  textSampleEntryCount = 0;
for (i = 0; i < UUIDatomEntryCount; i++)
  {
    if((UUIDatomEntryArray)[i] &&
       (UUIDatomEntryArray)[i] != NULL)
    {
      MM_Delete( (UUIDatomEntryArray)[i] );
      (UUIDatomEntryArray)[i] = NULL;
    }
  }
  UUIDatomEntryCount = 0;
#ifdef FEATURE_MP4_CUSTOM_META_DATA
  if(_dcmdAtom)
  {
    MM_Delete( _dcmdAtom );
  }
  if(_pdcfAtom)
  {
    MM_Delete( _pdcfAtom );
  }
  if (_kddiDRMAtom != NULL)
  {
    MM_Delete( _kddiDRMAtom );
    _kddiDRMAtom = NULL;
  }
  if (_kddiContentPropertyAtom != NULL)
  {
    MM_Delete( _kddiContentPropertyAtom );
    _kddiContentPropertyAtom = NULL;
  }
  if (_kddiMovieMailAtom != NULL)
  {
    MM_Delete( _kddiMovieMailAtom );
    _kddiMovieMailAtom = NULL;
  }
  if (_kddiEncoderInformationAtom != NULL)
  {
    MM_Delete( _kddiEncoderInformationAtom );
    _kddiEncoderInformationAtom = NULL;
  }
  if (_kddiGPSAtom != NULL)
  {
    MM_Delete( _kddiGPSAtom );
    _kddiGPSAtom = NULL;
  }

  if(_pTsmlAtom != NULL)
  {
    MM_Delete( _pTsmlAtom );
    _pTsmlAtom = NULL;
  }
  if (_kddiTelopElement != NULL)
  {
    MM_Delete( _kddiTelopElement );
    _kddiTelopElement = NULL;
  }

  if(_midiAtom)
  {
    MM_Delete( _midiAtom );
    _midiAtom = NULL;
  }
  if(_linkAtom)
  {
    MM_Delete( _linkAtom );
    _linkAtom = NULL;
  }

  if(_ftypAtom)
  {
    MM_Delete( _ftypAtom );
    _ftypAtom = NULL;
  }
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
  if(pSubsAtom)
  {
    MM_Delete(pSubsAtom);
  }
  if(_cprtAtom)
  {
    MM_Delete( _cprtAtom );
  }
  if(_authAtom)
  {
    MM_Delete( _authAtom );
  }
  if(_titlAtom)
  {
    MM_Delete( _titlAtom );
  }
  if(_dscpAtom)
  {
    MM_Delete( _dscpAtom );
  }
  if(_rtngAtom)
  {
    MM_Delete( _rtngAtom );
  }
  if(_kywdAtom)
  {
    MM_Delete( _kywdAtom );
  }
  if(_clsfAtom)
  {
    MM_Delete( _clsfAtom );
  }
  if(_lociAtom)
  {
    MM_Delete( _lociAtom );
  }
  if(_gnreAtom)
  {
    MM_Delete( _gnreAtom );
  }
  if(_perfAtom)
  {
    MM_Delete( _perfAtom );
  }

  if(_metaAtom)
  {
    MM_Delete( _metaAtom );
  }

  if(_albumAtom)
  {
    MM_Delete( _albumAtom);
  }
  if (pYrccAtom)
  {
    MM_Delete(pYrccAtom);
  }

  for (i = 0; i < m_ulPSSHatomEntryCount; i++)
  {
    if((m_aPSSHatomEntryArray)[i] &&
      (m_aPSSHatomEntryArray)[i] != NULL)
    {
      MM_Delete( (m_aPSSHatomEntryArray)[i] );
      (m_aPSSHatomEntryArray)[i] = NULL;
    }
  }

  for (i = 0; i < m_ulPSSHatomEntryCount; i++)
  {
    if((m_aSINFatomEntryArray)[i] &&
      (m_aSINFatomEntryArray)[i] != NULL)
    {
      MM_Delete( (m_aSINFatomEntryArray)[i] );
      (m_aSINFatomEntryArray)[i] = NULL;
    }
  }
  m_ulPSSHatomEntryCount = 0;

  /* free the m_pEncryptedDataBuffer*/
  if(m_pEncryptedDataBuffer)
  {
    MM_Free(m_pEncryptedDataBuffer);
  }

  /* now close the parse file pointer */

  if(m_parseFilePtr)
  {
    (void)OSCL_FileClose( m_parseFilePtr );
    m_parseFilePtr = NULL;

  }
  if(m_pBuffer)
  {
    MM_Free(m_pBuffer);
    m_pBuffer = NULL;
  }
  if(m_pSampleInfoBuffer)
  {
    MM_Free(m_pSampleInfoBuffer);
    m_pSampleInfoBuffer = NULL;
  }

  if(videoFMT_Access_CS)
  {
    MM_CriticalSection_Release(videoFMT_Access_CS);
  }

  if (m_pMoovAtomBuf)
  {
    MM_Free(m_pMoovAtomBuf);
    m_pMoovAtomBuf      = NULL;
    m_ulMoovBufDataRead = 0;
    m_ulMoovBufSize     = 0;
  }

  m_nBufferSize = 0;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getMovieDuration

DESCRIPTION
Get total movie duration

===========================================================================*/
uint64 Mpeg4File::getMovieDuration() const
{
  /* Convert duration value into Micro-sec units. */
  return ((m_videoFmtInfo.file_info.total_movie_duration *
            MICROSEC_TIMESCALE_UNIT) /
           m_videoFmtInfo.file_info.movie_timescale);
}

/*===========================================================================

FUNCTION  isFileFragmented

DESCRIPTION
Get total movie duration in msec

===========================================================================*/
bool Mpeg4File::isFileFragmented(void)
{
  return m_isFragmentedFile;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getTrackVideoFrameRate

DESCRIPTION
Get total movie duration

===========================================================================*/
float  Mpeg4File::getTrackVideoFrameRate(uint32 id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);
  if ( p_track && p_track->type == VIDEO_FMT_STREAM_VIDEO )
  {
    return p_track->subinfo.video.frame_rate;
  }
  return 0;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  trackRandomAccessDenied

DESCRIPTION
Get total movie duration

===========================================================================*/
uint8 Mpeg4File::trackRandomAccessDenied(uint32 id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  if ( p_track )
  {
    return p_track->user_data.no_rand_access;
  }

  return 0; // by default allow random access

}// from 'rand' atom at track level

/*===========================================================================

FUNCTION  getEncryptionType

DESCRIPTION
Return the type of encryption if any of the track is encrypted.

===========================================================================*/
EncryptionTypeT Mpeg4File::getEncryptionType()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Mpeg4File::getEncryptionType");
  for(int i=0; i<VIDEO_FMT_MAX_MEDIA_STREAMS; i++)
  {
    if( m_EncryptionType[i].encryptionType != ENCRYPT_NONE )
    {
      return m_EncryptionType[i].encryptionType;
    }
  }
  return ENCRYPT_NONE;
}

/*===========================================================================

FUNCTION  getEncryptionType

DESCRIPTION
Return the type of encryption if it is encrypted.

===========================================================================*/
EncryptionTypeT Mpeg4File::getEncryptionType(uint32 track_id)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Mpeg4File::getEncryptionType");
  for(int i=0; i<VIDEO_FMT_MAX_MEDIA_STREAMS; i++)
  {
    if( m_EncryptionType[i].track_id == track_id )
    {
      return m_EncryptionType[i].encryptionType;
    }
  }
  return ENCRYPT_NONE;
}

/* ======================================================================
FUNCTION
  Mpeg4Player::RegisterDRMDecryptMethod

DESCRIPTION
  Registers DRM decryption method.

DEPENDENCIES
  key can be set only in IDLE state.

RETURN VALUE
  TRUE if successful, else FALSE

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool Mpeg4File::RegisterDRMDecryptMethod(DRMDecryptMethodT pDRMDecryptFunction, void *pClientData)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Mpeg4File::RegisterDRMDecryptMethod");
  m_pDRMClientData = pClientData;
  m_pDRMDecryptFunction = (DRMDecryptMethodT)pDRMDecryptFunction;
  return true;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  trackDependsOn

DESCRIPTION
Get total movie duration

===========================================================================*/
// From TrackReference
uint32  Mpeg4File::trackDependsOn(uint32 id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  if ( p_track )
  {
    if ( p_track->ref_track.ref_atom == DPND_TYPE /* 'dpnd' */)
    {
      return p_track->ref_track.track_id[0];
    }
  }

  return 0;

}

/* ======================================================================
FUNCTION
Mpeg4File::getTrackMediaDuration

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
// From MediaHeader
uint64 Mpeg4File::getTrackMediaDuration(uint32 id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);
  uint64 ulDuration = 0;

  if ( p_track )
  {
    /* In non-DASH clips, main fragment duration will be provided as duration.
       But for DASH clips, main fragment will not contain media data so duration
       will always be ZERO. Instead total fragment's duration value can be used
       for the same purpose. Give duration as it is, FS will convert into
       microsecond units later.*/
    /* Duration should not be "-1". If the duration value is "-1", then use
       Total movie duration */
    if(p_track->media_duration && !m_isFragmentedFile &&
       ((uint32)-1!= p_track->media_duration) &&
       ((uint64)-1 != p_track->media_duration) )
    {
      ulDuration = (p_track->media_duration);
    }
    else
    {
      video_fmt_mp4r_context_type *context =
        (video_fmt_mp4r_context_type *) m_mp4ParseServerData;
      uint64 totalDur =
         context->file_level_data.fragment_file_total_movie_duration;
      //! Convert movie duration into Track's timescale units
      ulDuration = (uint64)(((totalDur * p_track->media_timescale) /
                         p_track->media_timescale));
    }
  }
  return ulDuration;
}
/* <EJECT> */
/*===========================================================================

FUNCTION  getTrackMediaTimescale

DESCRIPTION
Get total movie duration

===========================================================================*/
uint32 Mpeg4File::getTrackMediaTimescale(uint32 id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  if ( p_track )
  {
    /* Return Track timescale directly, FS will convert timestamp into
       micro seconds units. */
    return p_track->media_timescale;
  }
  else
    return 0;
}

/*===========================================================================

FUNCTION  getTrackAudioSamplingFreq

DESCRIPTION
Get total movie duration

===========================================================================*/
uint32 Mpeg4File::getTrackAudioSamplingFreq(uint32 id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  if ( p_track )
  {
    return p_track->subinfo.audio.sampling_frequency;
  }
  return 0;
}

/*===========================================================================

FUNCTION  getAudioTrackLanguage

DESCRIPTION
Get tarck language

===========================================================================*/
FILESOURCE_STRING Mpeg4File::getAudioTrackLanguage(uint32 id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);
  if ( p_track )
  {
    FILESOURCE_STRING Language;
    char *str =(char *)MM_Malloc(VIDEO_MAX_LANGUAGE_BYTES);
    if (!str)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                        "Mpeg4File::getAudioTrackLanguage() failed to allocate memory");
      return 0;
    }
    str[0] = (char)((p_track->media_language >> 10) + '`');
    str[1] = (char)(((p_track->media_language >> 5) & 0x1F) + '`');
    str[2] = (char)((p_track->media_language & 0x1F) + '`');
    str[3] = '\0';
    Language = (wchar_t*)str;
    MM_Free(str);
    return Language;
  }
  else
    return 0;
}

/* <EJECT> */

/*===========================================================================

FUNCTION  getAudioSamplesPerFrame

DESCRIPTION
Get total number of audio (codec) samples per MPEG4 sample entry
(which may contain one or more MPEG4 frames).

NOTE: Each MPEG4 frame contains a number of codec samples

===========================================================================*/
uint32 Mpeg4File::getAudioSamplesPerFrame(uint32 id)
{
  uint32 samplesPerFrame = 0;
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  if ( p_track && p_track->type == VIDEO_FMT_STREAM_AUDIO )
  {
    switch ( p_track->subinfo.audio.format )
    {
    case VIDEO_FMT_STREAM_AUDIO_EVRC:
      samplesPerFrame =
        (uint32)(p_track->subinfo.audio.audio_params.frames_per_sample * EVRC_SAMPLES_PER_FRAME);
      break;

    case VIDEO_FMT_STREAM_AUDIO_EVRC_PV:
      samplesPerFrame = (uint32)(EVRC_SAMPLES_PER_FRAME);
      break;

    case VIDEO_FMT_STREAM_AUDIO_PUREVOICE:
      samplesPerFrame =
        (uint32)(p_track->subinfo.audio.audio_params.frames_per_sample * QCELP_SAMPLES_PER_FRAME);
      break;

    case VIDEO_FMT_STREAM_AUDIO_AMR:
      {
        video_fmt_sample_info_type sampleInfo;
        uint32 streamNum = p_track->stream_num;

        if ( PARSER_ErrorNone == getSampleInfo(streamNum, 0, 1, &sampleInfo))
        {
          /* for some clips, SamplesPerFrame value is wrongly set to 10 and the correct
          value was one. This check is to fix those clips and we also try to
          minimize the scope of this fix by checking this value in clip and
          size of first AMR sample from a clip in question/given clip*/
          if( ((sampleInfo.size==32) || (sampleInfo.size==13) || (sampleInfo.size==21)|| (sampleInfo.size==18) )&&
              (p_track->subinfo.audio.audio_params.frames_per_sample==10) )
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR, "SamplesPerFrame=%d for AMR track, but using 1.",
                          p_track->subinfo.audio.audio_params.frames_per_sample);
            p_track->subinfo.audio.audio_params.frames_per_sample = 1;
          }
        }
      }
      samplesPerFrame = (uint32)(p_track->subinfo.audio.audio_params.frames_per_sample *
        AMR_SAMPLES_PER_FRAME);
      break;

    case VIDEO_FMT_STREAM_AUDIO_QCELP13K_FULL:
      samplesPerFrame = (uint32)(QCELP_SAMPLES_PER_FRAME);
      break;

    case VIDEO_FMT_STREAM_AUDIO_MPEG1_L3:
    case VIDEO_FMT_STREAM_AUDIO_MPEG2_L3:
      samplesPerFrame = (uint32)(MP3_SAMPLES_PER_FRAME);
      break;
    case VIDEO_FMT_STREAM_AUDIO_AMR_WB:
          samplesPerFrame = AMR_WB_SAMPLES_PER_FRAME;
          break;
    case VIDEO_FMT_STREAM_AUDIO_AMR_WB_PLUS:
          samplesPerFrame = AMR_WB_PLUS_SAMPLES_PER_FRAME;
          break;
    case VIDEO_FMT_STREAM_AUDIO_EVRC_B:
       samplesPerFrame = EVRC_NB_SAMPLES_PER_FRAME;
       break;
    case VIDEO_FMT_STREAM_AUDIO_EVRC_WB:
       samplesPerFrame = EVRC_WB_SAMPLES_PER_FRAME;
       break;
    default:
      break;
    }
  }

  return samplesPerFrame;

}

/* <EJECT> */
/*===========================================================================

FUNCTION  Mpeg4File::getTrackDecoderSpecificInfoContent

DESCRIPTION
===========================================================================*/
PARSER_ERRORTYPE Mpeg4File::getTrackDecoderSpecificInfoContent(uint32 id, uint8* buf, uint32 *pbufSize)
{
  uint32 bufsize = 0;
  uint32 headersize=0;
  uint8 *poutputtemp = NULL;
  PARSER_ERRORTYPE readstatus = PARSER_ErrorNone;

  if(pbufSize == NULL)
  {
    return PARSER_ErrorInvalidParam;
  }

  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  /* Sanity check for pointer*/
  if(p_track == NULL)
  {
    return PARSER_ErrorDefault;
  }

  /*Initialize Parameters*/
  poutputtemp = buf;
  bufsize = *pbufSize;

  /*Check if the input media type is H264*/
  if( (p_track->type == VIDEO_FMT_STREAM_VIDEO) &&
    (p_track->subinfo.video.format == VIDEO_FMT_STREAM_VIDEO_H264) )
  {
    int32 nBytes = 0;
    uint8* psize;
    uint8  *ptempbuf = NULL;

    if(m_bRawCodecData)
    {
      uint8* pInbuf = NULL;
      video_fmt_h264_dec_info_type *pH264Info = &p_track->dec_specific_info.h264_info;
      uint32 ulSize = 0;
      if(NULL != pH264Info->avcc_info)
      {
        ulSize = pH264Info->avcc_alloc.allocated_size;
        pInbuf = pH264Info->avcc_alloc.memory_ptr;
      }
      else
      {
        ulSize = pH264Info->mvcc_alloc.allocated_size;
        pInbuf = pH264Info->mvcc_alloc.memory_ptr;
      }
      if((0 == *pbufSize) || (NULL == buf) )
      {
        *pbufSize = ulSize;
      }
      else if(*pbufSize < ulSize)
      {
        *pbufSize = ulSize;
        return PARSER_ErrorInsufficientBufSize;
      }
      else
      {
        memcpy(buf, pInbuf, ulSize);
        *pbufSize = ulSize;
      }
      return PARSER_ErrorNone;
    }
    /*Allocate temp buffer*/
    ptempbuf = (uint8 *) MM_Malloc (ALLOC_MEMORY_HEADER);

    /*Check for allocation failure*/
    if(ptempbuf == NULL)
    {
      /*set header size to 0*/
      *pbufSize = 0;
      return PARSER_ErrorMemAllocFail;
    }
    psize = (uint8 *) &nBytes;

    /*Reset the NAL parameters */
    resetParamSetNAL(id);

    do
    {
      /*read the next NAL unit in the file*/
      nBytes = getNextParamSetNAL(id, ptempbuf, ALLOC_MEMORY_HEADER);
      if(nBytes == FRAGMENT_CORRUPT)
      {
        MM_Free(ptempbuf);
        return PARSER_ErrorDefault;
      }

      if( (poutputtemp != NULL) &&  /*Check if ouput buffer is valid*/
          (nBytes > 0)          &&  /*Check if we have reached end of header*/
          (nBytes <= ALLOC_MEMORY_HEADER) &&
          (bufsize >= (uint32)(nBytes + 4))/* Check if enough space in ip buf*/
        )
      {
        //ARM is big-endian, and decoder expects size in little-endian
        for( int i = 0; i < 4; i++ )
        {
          poutputtemp[i] = psize[ 3 - i ];
        }
        //Need to prefix each parameter with following start-code
        const unsigned long startCode = 0x01000000;
        (void) memmove (poutputtemp, &startCode, 4);

        /*increment pointer*/
        poutputtemp += 4;
        /*Copy the header related stuff*/
        memcpy (poutputtemp,ptempbuf,(uint8)nBytes);
        /*Increment the Source pointer*/
        poutputtemp += nBytes;
        /*Decrement the input buffer size*/
        bufsize -= (nBytes + 4);
      }
      /*
      Buffer that client has supplied is not enough
      Hence Flag error
      */
      else if( ( poutputtemp == NULL )  || (bufsize < (uint32)(nBytes +4)) )
      {
        readstatus = PARSER_ErrorInsufficientBufSize;
      }
      /*If nothing is read avoid incrementing headersize*/
      if(nBytes)
      {
        /*Increment Header Size*/
        headersize += (nBytes+4);
      }
    }
    while (nBytes);
    /*
    Not enough memory to add the nal length information
    then return error
    */
    if ( poutputtemp == NULL ||
      bufsize < 2 )
    {
      readstatus = PARSER_ErrorInsufficientBufSize;
    }
    else
    {
      //This is not part of header but output of below two methods is needed
      //to configure VDEC object for decoding H.264 clip.
      //Means, it will be input parameter for vdec_h264_init_decoder_specific_info
      poutputtemp[0]   = GetSizeOfNALLengthField(id);

      //We are going to work with Stored media, so fix this one as zero.
      poutputtemp[1] = 0;

      headersize+=2;
    }

    /*Reset the NAL parameters */
    resetParamSetNAL(id);

    /*
    Check if Buffer is allocated
    Free if allocated
    */
    MM_Free (ptempbuf);

  } /*Video format is H.264*/
  else if( ( VIDEO_FMT_STREAM_VIDEO == p_track->type ) &&
           ( VIDEO_FMT_STREAM_VIDEO_HEVC == p_track->subinfo.video.format ) )
  {
    video_fmt_hevc_dec_info_type *pHEVCInfo = &p_track->dec_specific_info.sHEVCInfo;
    // Advertise NAL data buffer size required.
    if( ( NULL!= pHEVCInfo) &&
        ( NULL!= pHEVCInfo->pHVCCInfo) &&
        ( NULL!= pHEVCInfo->pHVCCInfo->pArrayNALU ) &&
        ( NULL!= pHEVCInfo->pHVCCInfo->pArrayNALU->pNALU ))
    {
      if( 0 == *pbufSize || NULL == buf)
      {
        headersize = FILESOURCE_MAX(pHEVCInfo->pHVCCInfo->ulNaluDataLength,
                                    pHEVCInfo->sHVCCAtomAlloc.allocated_size);
      }
      else if(m_bRawCodecData)
      {
        headersize = pHEVCInfo->sHVCCAtomAlloc.allocated_size;
        memcpy(buf, pHEVCInfo->sHVCCAtomAlloc.memory_ptr, headersize);
      }
      else
      {
        uint32 ulArrIdx = 0;
        uint32 ulNalIdx = 0;
        uint32 ulWriteIdx = 0;
        const unsigned long ulSC = 0x01000000;
        for ( ulArrIdx = 0;
              ulArrIdx < pHEVCInfo->pHVCCInfo->ucNumOfNALArray;
              ulArrIdx++)
        {
          for( ulNalIdx = 0;
               ulNalIdx < pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].usNumNALU;
               ulNalIdx++)
          {
            //Copy start-code
            (void) memmove (buf+ulWriteIdx, &ulSC, 4);
            ulWriteIdx += 4;
            //Copy NAL data
            (void) memcpy( buf+ulWriteIdx,
                  pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].data,
                  pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].len);
            ulWriteIdx += \
              pHEVCInfo->pHVCCInfo->pArrayNALU[ulArrIdx].pNALU[ulNalIdx].len;
          }//for(ulNalIdx)
        }//for(ulArrIdx)
      }//if(buf)
    }//if(pHEVCInfo
  }
  else if( (p_track->type == VIDEO_FMT_STREAM_TEXT) &&
    ( ( p_track->subinfo.text.format == VIDEO_FMT_STREAM_TEXT_SMPTE_TIMED_TEXT )||
      ( p_track->subinfo.text.format == VIDEO_FMT_STREAM_TEXT_TIMEEDTEXT ) ) )
  {
    DataT eDataType = GET_TEXT_DATATYPE(p_track->subinfo.text.format);
    uint32 ulOffset = 0;
    //sample_desc_index will start from '1'.
    if( m_sampleInfo[p_track->stream_num].sample_desc_index )
    {
      ulOffset = m_sampleInfo[p_track->stream_num].sample_desc_index -1;
    }
    // Get codec configuration specific data size
        if( buf && *pbufSize )
    {
      headersize = GetData( eDataType,
                            buf,
                            *pbufSize,
                            ulOffset);
    }
    else if( 0 == *pbufSize )
    {
      headersize = GetDataSize( eDataType, ulOffset );
      headersize -= DEFAULT_TEXT_ATOM_HEADER_SIZE;
    }
  }
  else
  {
    /*Check if the input buffer has enough space*/
    if( (bufsize >= p_track->header) && (poutputtemp) )
    {
      /*Copy the header*/
      memcpy (poutputtemp,p_track->dec_specific_info.info,p_track->header);
      /*Update header size*/
      headersize = p_track->header;
    }
    else
    {
      /*
      Don't have enough space hence update the Status.
      Send actual header size
      */
      readstatus = PARSER_ErrorInsufficientBufSize;
      headersize = p_track->header;
    }
  }
  /*Update the size of header*/
  *pbufSize = headersize;
  return readstatus;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getTrackOTIType

DESCRIPTION
Get total movie duration

===========================================================================*/
// Based on OTI (ObjectTypeIndication) value from the 'esds' atom
uint8  Mpeg4File::getTrackOTIType(uint32 id)
{
  uint8 format = 0xFF;
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  if ( p_track )
  {
    format = p_track->dec_specific_info.obj_type;
    if (p_track->dec_specific_info.obj_type == MPEG4_AUDIO)
    {   /* Verify if the atom has BSAC specific information */
      if (p_track->subinfo.audio.aac_params.audio_object_type == 22)
      {
        format = MPEG4_AUDIO_BSAC;
      }
    }

    if (p_track->type == VIDEO_FMT_STREAM_VIDEO)
    {
      switch(p_track->subinfo.video.format)
      {
      case VIDEO_FMT_STREAM_VIDEO_H263:
        format = (uint8)H263_VIDEO;
        break;

      case VIDEO_FMT_STREAM_VIDEO_H264:
        format = (uint8)H264_VIDEO;
        break;

      case VIDEO_FMT_STREAM_VIDEO_HEVC:
        format = (uint8)HEVC_VIDEO;
        break;

      default:
        break;
      }
    }
    if ( (p_track->dec_specific_info.obj_type == MPEG4_VIDEO ||
          p_track->dec_specific_info.obj_type == MPEG4_IMAGE) &&
         (p_track->frames == 1) )
    {
      /* this is Mpeg4 Video CODEC with just one video frame. We will treat
       it as STILL IMAGE frame, so that audio track repositioning can work */
       format = MPEG4_IMAGE;
    }
    /* Audio special treatment
    */
    if (p_track->type == VIDEO_FMT_STREAM_AUDIO)
    {
      switch(p_track->subinfo.audio.format)
      {
      case VIDEO_FMT_STREAM_AUDIO_AMR:
        format = (uint8)AMR_AUDIO;
        break;

      case VIDEO_FMT_STREAM_AUDIO_EVRC:
      case VIDEO_FMT_STREAM_AUDIO_EVRC_PV:
        format = (uint8)EVRC_AUDIO;
        break;

      case VIDEO_FMT_STREAM_AUDIO_PUREVOICE:
        format = (uint8)PUREVOICE_AUDIO; // 3GPP2 QCELP
        break;

      case VIDEO_FMT_STREAM_AUDIO_QCELP13K_FULL:
        format = (uint8)PUREVOICE_AUDIO_2; // 3GPP2 QCELP
        break;

      case VIDEO_FMT_STREAM_AUDIO_MPEG1_L3:
      case VIDEO_FMT_STREAM_AUDIO_MPEG2_L3:
        format  = (uint8)MP3_AUDIO;
        break;

      case VIDEO_FMT_STREAM_AUDIO_AMR_WB:
        format = AMR_WB_AUDIO;
        break;

      case VIDEO_FMT_STREAM_AUDIO_AMR_WB_PLUS:
        format = AMR_WB_PLUS_AUDIO;
        break;

      case VIDEO_FMT_STREAM_AUDIO_EVRC_B:
        format = EVRC_NB_AUDIO;
        break;

      case VIDEO_FMT_STREAM_AUDIO_EVRC_WB:
        format = EVRC_WB_AUDIO;
        break;

      case VIDEO_FMT_STREAM_AUDIO_AC3:
        format = AC3_AUDIO;
        break;

      case VIDEO_FMT_STREAM_AUDIO_EAC3:
        format = EAC3_AUDIO;
        break;

      case VIDEO_FMT_STREAM_AUDIO_EAC3_JOC:
        format = EAC3_JOC_AUDIO;
        break;

      case VIDEO_FMT_STREAM_AUDIO_DTS_CORE:
      case VIDEO_FMT_STREAM_AUDIO_DTS_HD:
      case VIDEO_FMT_STREAM_AUDIO_DTS_HD_LOSSLESS:
      case VIDEO_FMT_STREAM_AUDIO_DTS_LBR:
        format = DTS_AUDIO;
        break;

      default:
        break;
      }
    }// if(type == VIDEO_FMT_STREAM_AUDIO)

    if (p_track->type == VIDEO_FMT_STREAM_TEXT)
    {
      switch( p_track->subinfo.text.format)
      {
      case VIDEO_FMT_STREAM_TEXT_TIMEEDTEXT:
        format = (uint8)TIMED_TEXT;
        break;

      case VIDEO_FMT_STREAM_TEXT_SMPTE_TIMED_TEXT:
        format = (uint8)SMPTE_TIMED_TEXT;
        break;

      default:
        break;
      }
    }
  }

  return format;

}

/* <EJECT> */
/*===========================================================================

FUNCTION  getTracAudioFormat

DESCRIPTION
Gets the audio format of given track in video_fmt_stream_audio_subtype type.

===========================================================================*/
uint8 Mpeg4File::getTrackAudioFormat(uint32 id)
{
  uint8 format = 0xFF;
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  if ( p_track && (p_track->type == VIDEO_FMT_STREAM_AUDIO) )
  {
    return (uint8)(p_track->subinfo.audio.format);
  }

  return format;
}


/* <EJECT> */
/*===========================================================================

FUNCTION  getFramesPerSample

DESCRIPTION
Gets the number frames per sample of given track in video_fmt_stream_audio_subtype type.

===========================================================================*/
uint8 Mpeg4File::getFramesPerSample(uint32 id)
{
  uint8 num_frames = 0x00;
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  if ( p_track && (p_track->type == VIDEO_FMT_STREAM_AUDIO) )
  {
    return (uint8)(p_track->subinfo.audio.audio_params.frames_per_sample);
  }

  return num_frames;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getTotalNumberOfFrames

DESCRIPTION
Gets the total number of frames in a given track.

===========================================================================*/
uint16 Mpeg4File::getTotalNumberOfFrames(uint32 id)
{
  uint8 total_num_frames = 0x00;
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  if ( p_track && (p_track->type == VIDEO_FMT_STREAM_AUDIO) )
  {
    return (uint16)(p_track->frames);
  }

  return total_num_frames;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getTrackMaxBufferSizeDB

DESCRIPTION
Get total movie duration
This function takes care of long overflow by taking FILESOURCE_MAX.

===========================================================================*/
int32 Mpeg4File::getTrackMaxBufferSizeDB(uint32 id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  if (p_track )
  {
    if(!p_track->largest_found)
    {
      /* In case video we need to have maxframe size as height * width * 1.5 because in DS or OMX
         there is no way to reallocate the buffers. so we are defaulting this to height * width * 1.5
         instead of 10K
      */
      if(p_track->type == VIDEO_FMT_STREAM_VIDEO)
      {
        //Reducing parser input buffer size by 2 as i/p buffer size becoming
        //greater than then YUV frame size for 720p/1080p content
        p_track->largest = (uint32)(p_track->subinfo.video.height  *
                                    p_track->subinfo.video.width );

        //Check whether frame size is minimum allowed size or not.
        p_track->largest = FILESOURCE_MAX(p_track->largest, (uint32)
                                          MIN_MEDIA_FRAME_SIZE);
      }
      else
      {
        p_track->largest = MAX_AUDIO_FRAME_SIZE; //Defaulting to 64K.
      }
    }

      /*  Max bitrate indicates amount of data to be transmitted in 1sec time.
          Audio frames will never be more than ~50ms in most of the cases.
          To be on safer side, calculate max buffer size required to store
          200ms worth of data which means 1/5th of max bitrate value. */
      uint32 ulMaxBitRate = getTrackMaxBitrate(id);
      //! Parser need not to use bitrate field for AAC codec
    if((VIDEO_FMT_STREAM_AUDIO == p_track->type) &&
       (VIDEO_FMT_STREAM_AUDIO_MPEG4_AAC != p_track->subinfo.audio.format))
    {
      uint32 ulMaxBufSize = p_track->largest;
      if (ulMaxBitRate)
      {
        ulMaxBufSize = FILESOURCE_MIN( ulMaxBitRate/5, ulMaxBufSize);
      }
      return (int32)ulMaxBufSize;
    }

    else if ( (p_track->type == VIDEO_FMT_STREAM_VIDEO) &&
      (p_track->subinfo.video.format == VIDEO_FMT_STREAM_VIDEO_H263) )

    {

      //Return the largest sample entry size in sample table and add 3 bytes in case
      // decoder looks forward 3 bytes since the H.263 track atom does not have any decoder specific info
      return (int32)FILESOURCE_MAX( (p_track->largest+p_track->header+3), p_track->largest );

    }
    else if ( (p_track->type == VIDEO_FMT_STREAM_VIDEO) &&
      (p_track->subinfo.video.format == VIDEO_FMT_STREAM_VIDEO_H264) )

    {
      //Return the largest sample entry size in sample table and add 1 byte to make it NAL.
      return (int32)FILESOURCE_MAX( (p_track->largest+p_track->header+1), p_track->largest );
    }
    else if ( ( p_track->type == VIDEO_FMT_STREAM_TEXT ) &&
              ( p_track->subinfo.text.format ==
                  VIDEO_FMT_STREAM_TEXT_SMPTE_TIMED_TEXT ) )
    {
      //Ref: CFF-Section 6.4.3
      //Return the largest possible sample size.CFF-TT subtitle samples
      //has following constraint.
      // 1. CFF-document size <= 200kb
      // 2. Reference image size <= 100kb
      // 3. Subtitle fragment/sample size including image <= 500 kb
      return (int32) MAX_SUBTITLE_SAMPLE_SIZE;
    }
    else
    {
      return (int32)FILESOURCE_MAX( (p_track->largest+p_track->header), p_track->largest );
    }
  }
  else
    return 0;
}

/*===========================================================================

FUNCTION  getLargestFrameSize

DESCRIPTION
  Public method used to find the largest frame size in a given track.

===========================================================================*/
uint32 Mpeg4File::getLargestFrameSize ( uint32 id )
{
  uint32 nretVal = 0;
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);
  if (p_track)
  {
    if (!p_track->largest_found)
    {
      /* call this call back to find the largest frame size
          in the given track. */
      m_videoFmtInfo.largest_frame_size_cb (m_videoFmtInfo.server_data,
                                            p_track->stream_num);
      /* If the largest is found */
      if(p_track->largest_found)
      {
        /* Return the largest */
        nretVal = p_track->largest;
      }
    }
    else
    {
      nretVal = p_track->largest;
    }
  }
  return nretVal;
}

/*===========================================================================

FUNCTION  getRotationDegrees

DESCRIPTION
  Public method used to find the rotation degree in a given track.

===========================================================================*/
uint32 Mpeg4File::getRotationDegrees( uint32 ulTrackId )
{
  uint32 ulRotationDegree = 0;
  video_fmt_stream_info_type *p_track = getTrackInfoForID(ulTrackId);
  if (p_track && (VIDEO_FMT_STREAM_VIDEO == p_track->type ))
  {
    ulRotationDegree = p_track->rotation_degrees;
  }
  return ulRotationDegree;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getTrackAverageBitrate

DESCRIPTION
Get total movie duration

===========================================================================*/
int32 Mpeg4File::getTrackAverageBitrate(uint32 id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);
  int32 bitRate = 0;

  if ( p_track )
  {
    /* Special case for AMR audio tracks */
    if ((p_track->type == VIDEO_FMT_STREAM_AUDIO)
      && (p_track->subinfo.audio.format == VIDEO_FMT_STREAM_AUDIO_AMR))
    {
      for (uint32 i = 0; i < 8; i++)
      {
        if ( p_track->subinfo.audio.audio_params.mode_set & AMRModeSetMask[i])
        {
          bitRate = AMRBitRates[i];
        }
      }
    }
    else if( ( VIDEO_FMT_STREAM_AUDIO == p_track->type ) &&
             ( ( VIDEO_FMT_STREAM_AUDIO_DTS_CORE == \
                 p_track->subinfo.audio.format ) ||
               ( VIDEO_FMT_STREAM_AUDIO_DTS_HD == \
                 p_track->subinfo.audio.format ) ||
               ( VIDEO_FMT_STREAM_AUDIO_DTS_HD_LOSSLESS == \
                 p_track->subinfo.audio.format ) ||
               ( VIDEO_FMT_STREAM_AUDIO_DTS_LBR == \
                 p_track->subinfo.audio.format ) ) )
    {
      bitRate = (int32) (p_track->subinfo.audio.dts_params.ulAvgBitRate);
    }
    else
    { /* If the bitrate is encoded, then use that value */
      if( p_track->dec_specific_info.avgbitrate != 0 )
      {
        bitRate = (int32)(p_track->dec_specific_info.avgbitrate);
      }
      else
      {
        bitRate = 0;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "Avg bitRate set to 0: p_track->bytes not available");
      }
    }
  }

  return bitRate;
}
/* <EJECT> */
/*===========================================================================

FUNCTION  getTrackMaxBitrate

DESCRIPTION
Get total movie duration

===========================================================================*/
int32 Mpeg4File::getTrackMaxBitrate(uint32 id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  if ( p_track )
  {
    if ((p_track->type == VIDEO_FMT_STREAM_AUDIO)
      && (p_track->subinfo.audio.format == VIDEO_FMT_STREAM_AUDIO_AMR))
    {
      int32 bitRate = 0;
      for (uint32 i = 0; i < 8; i++)
      {
        if (p_track->subinfo.audio.audio_params.mode_set & AMRModeSetMask[i])
        {
          bitRate = AMRBitRates[i];
        }
      }
      return bitRate;
    }
    else if( ( VIDEO_FMT_STREAM_AUDIO == p_track->type ) &&
             ( ( VIDEO_FMT_STREAM_AUDIO_DTS_CORE == \
                 p_track->subinfo.audio.format ) ||
               ( VIDEO_FMT_STREAM_AUDIO_DTS_HD == \
                 p_track->subinfo.audio.format ) ||
               ( VIDEO_FMT_STREAM_AUDIO_DTS_HD_LOSSLESS == \
                 p_track->subinfo.audio.format ) ||
               ( VIDEO_FMT_STREAM_AUDIO_DTS_LBR == \
                 p_track->subinfo.audio.format ) ) )
    {
      return (int32) (p_track->subinfo.audio.dts_params.ulMaxBitRate );
    }
    else
      return (int32)(p_track->dec_specific_info.maxbitrate);
  }
  else
    return 0;
}
/* <EJECT> */
/*===========================================================================

FUNCTION  getMovieTimescale

DESCRIPTION
Get total movie duration

===========================================================================*/
uint32 Mpeg4File::getMovieTimescale() const
{
  /* In mpeg4file object itself, we are converting movie duration value into
     micro-sec units.  */
  return MILLISEC_TIMESCALE_UNIT;

}

/* <EJECT> */
/*===========================================================================

FUNCTION  getTrackVideoFrameWidth

DESCRIPTION
Get total movie duration

===========================================================================*/
uint32 Mpeg4File::getTrackVideoFrameWidth(uint32 id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  if ( p_track )
  {
    return p_track->subinfo.video.width;
  }
  return 0;
}
/* <EJECT> */
/*===========================================================================

FUNCTION  getTrackVideoFrameHeight

DESCRIPTION
Get total movie duration

===========================================================================*/
uint32 Mpeg4File::getTrackVideoFrameHeight(uint32 id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  if ( p_track )
  {
    return p_track->subinfo.video.height;
  }
  return 0;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  GetNumAudioChannels

DESCRIPTION
returns the number of audio channels

==========================================================================*/
uint32 Mpeg4File::GetNumAudioChannels(int id)
{
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

  if ( p_track )
  {
    /*Allowing specific clips given by the customer to work by
    setting the  num_channels to one even when the info in the clip is zero*/
    return FILESOURCE_MAX(1,p_track->subinfo.audio.num_channels);
  }
  return 0;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  GetAACAudioProfile

DESCRIPTION
returns the AAC Audio profile information

==========================================================================*/

uint32 Mpeg4File::GetAACAudioProfile(uint32 id)
{
   video_fmt_stream_info_type *p_track = getTrackInfoForID(id);

   if ( p_track )
   {
      return p_track->subinfo.audio.aac_params.audio_object_type;
   }
   return 0;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  randomAccessDenied

DESCRIPTION
Get total movie duration

===========================================================================*/
uint8 Mpeg4File::randomAccessDenied()
{
  uint8 nRet = FALSE;
  if(m_videoFmtInfo.file_info.no_rand_access)
  {
    nRet = TRUE;
  }
  return nRet;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getNumTracks

DESCRIPTION
Get total movie duration

===========================================================================*/
int32 Mpeg4File::getNumTracks()
{
  int32 totalTracks = 0;
  video_fmt_stream_info_type *p_track = 0;

  /* Errors may happen in mp4ParseStatus, i.e., VIDEO_FMT_ALLOC may fail,
     so need to check errors before obtaining tracks info
  */
  /*Sync Errors may happen due to Invalid AMR audio content */
  if(!_success)
  {
     return 0;
  }

  for ( uint32 index = 0; index < m_videoFmtInfo.num_streams; index++)
  {
    p_track = m_videoFmtInfo.streams + index;

    if ( (p_track->type == VIDEO_FMT_STREAM_AUDIO) ||
         (p_track->type == VIDEO_FMT_STREAM_TEXT)  ||
         (p_track->type == VIDEO_FMT_STREAM_VIDEO) )
    {
      if(p_track->type == VIDEO_FMT_STREAM_AUDIO)
        m_bAudioPresentInClip = true;
      else if(p_track->type == VIDEO_FMT_STREAM_VIDEO)
        m_bVideoPresentInClip = true;
      else if(p_track->type == VIDEO_FMT_STREAM_TEXT)
        m_bTextPresentInClip = true;
      totalTracks++;
    }
  }

  /* Added this to correct the play information sent in constructor*/
  if(m_playAudio)
  {
    if(!m_bAudioPresentInClip)
      m_playAudio = false;
  }
  if(m_playVideo)
  {
    if(!m_bVideoPresentInClip)
      m_playVideo = false;
  }
  if(m_playText)
  {
    if(!m_bTextPresentInClip)
      m_playText = false;
  }

  return totalTracks;
}

/* <EJECT> */

/*===========================================================================

FUNCTION  getTrackWholeIDList

DESCRIPTION
Get total movie duration

===========================================================================*/
uint32 Mpeg4File::getTrackWholeIDList(uint32 *ids)
{
  uint32 count = 0;
  video_fmt_stream_info_type *p_track = 0;

  if (ids == NULL)
  {
    return 0;
  }

  if (m_videoFmtInfo.num_streams > VIDEO_FMT_MAX_MEDIA_STREAMS )
  {
    m_videoFmtInfo.num_streams = VIDEO_FMT_MAX_MEDIA_STREAMS;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "Clip has more tracks than supported");
  }

  for (uint32 i = 0; i < m_videoFmtInfo.num_streams; i++)
  {
    p_track = m_videoFmtInfo.streams + i;

    /* some clips have erorr where we don't have time scale information.
    In that case, we will not play that track */
    if( !p_track->media_timescale )
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR, "Non Playable Track=%lu, TimeScale is zero", p_track->stream_num);
      continue;
    }

    if ( (p_track->type == VIDEO_FMT_STREAM_AUDIO) ||
      (p_track->type == VIDEO_FMT_STREAM_TEXT)  ||
      (p_track->type == VIDEO_FMT_STREAM_VIDEO) )
    {
      (*ids) = p_track->track_id;
      ids++;
      count++;
    }
  }
  return count;
}


/* <EJECT> */
/*===========================================================================

FUNCTION  getTrackInfoForID

DESCRIPTION
Public method providing the capability to read data from a MP4 file.
It fetches the media track whose track_id matches the input parameter 'id'

===========================================================================*/
video_fmt_stream_info_type * Mpeg4File::getTrackInfoForID (uint32 id)
{
  for (uint32 index = 0; index < m_trackCount; index++)
  {
    if ( m_track[index]->track_id == id )
    {
      return m_track[index];
    }
  }
  return 0;
}

/*===========================================================================

FUNCTION  getTrackIdFromStreamNum

DESCRIPTION
Public method providing the capability to read trackId corresponding to the
given input parameter streamNum.

===========================================================================*/
uint32 Mpeg4File::getTrackIdFromStreamNum (uint32 streamNum)
{
  for (uint32 index = 0; index < m_trackCount; index++)
  {
    if ( m_track[index]->stream_num == streamNum )
    {
      return m_track[index]->track_id;
    }
  }
  return 0;
}

/*===========================================================================

FUNCTION  process_video_fmt_info

DESCRIPTION
Private method called from mp4ParseStatus().

===========================================================================*/
void Mpeg4File::process_video_fmt_info(video_fmt_status_type status,
                                       video_fmt_status_cb_info_type *info)
{
  status = status; //variable not in use, required to remove compile time warning

  if(status == VIDEO_FMT_INFO)
  {
    m_parsedEndofFile = true;
  }
  else if(status == VIDEO_FMT_FRAGMENT)
  {
    m_parsedEndofFragment = true;
    m_parsedEndofFile = false;
  }

  memcpy ( &m_videoFmtInfo, (void *)&info->info, sizeof(video_fmt_info_type) );

  // Save stream reading callback.
  m_mp4ParseServerData = info->info.server_data;

  // Calculate the frame rate for all tracks
  uint32 index;
  video_fmt_stream_info_type *p_track = NULL;

  //Recalculate the toatal number of tracks in this new parsed fragment.
  m_trackCount = 0;

  for (index = 0; index < m_videoFmtInfo.num_streams; index++)
  {
    p_track = m_videoFmtInfo.streams + index;

    if (p_track != NULL)
    {
#ifdef FEATURE_MP4_CUSTOM_META_DATA
      if(m_OffsetinEncryptionTypeArray < VIDEO_FMT_MAX_MEDIA_STREAMS)
      {
        if(( (p_track->subinfo.video.pdcf_info.scheme_type == ODKM_TYPE)
              && (p_track->subinfo.video.pdcf_info.scheme_version == OMA_DRM_SCHEME_VERSION))
           || ((p_track->subinfo.audio.pdcf_info.scheme_type == ODKM_TYPE)
              && (p_track->subinfo.audio.pdcf_info.scheme_version == OMA_DRM_SCHEME_VERSION)))
        {
          m_EncryptionType[m_OffsetinEncryptionTypeArray].track_id = p_track->track_id;
          m_EncryptionType[m_OffsetinEncryptionTypeArray].encryptionType = ENCRYPT_OMA_DRM_V2;
          m_OffsetinEncryptionTypeArray++;
        }
      }
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
      switch ( p_track->type )
      {
      case VIDEO_FMT_STREAM_VIDEO:
        if(m_playVideo)
        {
          if(!p_track->frames)
          {
            m_hasVideo = false;
          }
          else
          {
            m_hasVideo = true;
          }

          if ( p_track->subinfo.video.frame_rate == 0 )
          {
            // frame_rate = frames / (track duration / track timescale)
            if ( p_track->media_duration > 0 )
            {
              /* temporarily typecast to float to prevent uint32 overflow for large clips */
              p_track->subinfo.video.frame_rate = (float)
                ((p_track->frames * p_track->media_timescale) /
                 p_track->media_duration);
              /* since we return only uint16, make sure frame rate is at least one */
              p_track->subinfo.video.frame_rate =
                          FILESOURCE_MAX(1, p_track->subinfo.video.frame_rate);
            }
            else
            {
              p_track->subinfo.video.frame_rate =
                (float)(p_track->frames * p_track->media_timescale);
            }
          }
        }
        /*
        * m_track is declared as
        * video_fmt_stream_info_type* m_track[VIDEO_FMT_MAX_MEDIA_STREAMS];
        * Make sure (m_trackCount+1) is within the correct limits before attempting to write to it.
        */
        if((m_trackCount+1)<VIDEO_FMT_MAX_MEDIA_STREAMS)
        {
          m_track[m_trackCount++] = p_track;
        }
        else
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
            "VIDEO_FMT_STREAM_VIDEO:m_trackCount OVERFLOW %lu", m_trackCount);
        }
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                     "VIDEO: p_track->frames = %llu", p_track->frames);
        break;

      case VIDEO_FMT_STREAM_AUDIO:
        if(m_playAudio)
        {
          if(!p_track->frames)
          {
            m_hasAudio = false;
          }
          else
          {
            m_hasAudio = true;
          }
        }
         /*
        * m_track is declared as
        * video_fmt_stream_info_type* m_track[VIDEO_FMT_MAX_MEDIA_STREAMS];
        * Make sure (m_trackCount+1) is within the correct limits before attempting to write to it.
        */
        if((m_trackCount+1)<VIDEO_FMT_MAX_MEDIA_STREAMS)
        {
          m_track[m_trackCount++] = p_track;
        }
        else
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
            "VIDEO_FMT_STREAM_AUDIO:m_trackCount OVERFLOW %lu", m_trackCount);
        }
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                     "AUDIO: p_track->frames = %llu", p_track->frames);
        break;

      case VIDEO_FMT_STREAM_TEXT:
        if(m_playText)
        {
          if(!p_track->frames)
          {
            m_hasText = false;
          }
          else
          {
            m_hasText = true;
          }
        }
        /*
        * m_track is declared as
        * video_fmt_stream_info_type* m_track[VIDEO_FMT_MAX_MEDIA_STREAMS];
        * Make sure (m_trackCount+1) is within the correct limits before attempting to write to it.
        */
        if((m_trackCount+1)<VIDEO_FMT_MAX_MEDIA_STREAMS)
        {
          m_track[m_trackCount++] = p_track;
        }
        else
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
            "VIDEO_FMT_STREAM_TEXT:m_trackCount OVERFLOW %lu", m_trackCount);
        }
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                     "TEXT: p_track->frames = %llu", p_track->frames);
        break;

      default:
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                     "Unsupported VideoFMT track=%lu", p_track->stream_num);
        break;
      }
    }
  }
}

/* ======================================================================
FUNCTION
Mpeg4File::mp4ParseStatus

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
void Mpeg4File::mp4ParseStatus (video_fmt_status_type status,
                                video_fmt_status_cb_info_type *info,
                                video_fmt_end_cb_func_type end)
{
 // Store the status code for later inspection.
  m_mp4ParseLastStatus = status;
  m_parsedEndofFragment = false;

  // Store session end callback function.
  m_mp4ParseEndCb = end;

  /*
  * m_parseFilePtr is valid only for initial parsing.
  * For subsequent fragment parsing, we will use one of the already opened
  * file pointer for playing track because we want to limit
  * opened file pointers to one per stream.
  * Some OEM have limited file pointers and for some OEM opening and
  * closing file pointers are very CPU expensive.
  *
  * Validation of localParseFilePtr is done below before use.
  */
  OSCL_FILE* localParseFilePtr = m_parseFilePtr;
  // Branch according to the status code received.
  switch (status)
  {
  case VIDEO_FMT_ALLOC:
    info->alloc.ptr = (char*) MM_Malloc(info->alloc.size);
    if ( info->alloc.ptr == NULL )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "Mpeg4File::mp4ParseStatus failed to allocate memory");
    }
    else
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "Mpeg4File::mp4ParseStatus alloc");
    break;

  case VIDEO_FMT_FREE:
    MM_Free(info->free.ptr);
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Mpeg4File::mp4ParseStatus free");
    break;

  case VIDEO_FMT_FILESIZE:
    info->fSize.fileSize = m_fileSize;
    break;

  case VIDEO_FMT_GET_DATA:
    {
      if(localParseFilePtr == NULL)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                    "Mpeg4File::mp4ParseStatus localParseFilePtr is NULL");
        break;
      }
      MM_ASSERT(localParseFilePtr);
      m_mp4ParseContinueCb = info->get_data.callback_ptr;
      m_mp4ParseServerData = info->get_data.server_data;

      /* This flag will be set by the UI task if the user terminates the
         playback. By setting EOD flag as true, Videofmt will treat this
         as read failure and abort the playback. If EOD flag is not set to
         true, then Videofmt will consider it as Under-run, rather than
         read failure. */
        if(m_bMediaAbort)
        {
          info->get_data.num_bytes  = 0;
          info->get_data.bEndOfData = TRUE;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
            "Mpeg4File::mp4ParseStatus read fail, User aborted playback..!!");
          _fileErrorCode = PARSER_ErrorReadFail;
          break;
        }

      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
         "Mpeg4File::mp4ParseStatus Read offset %llu Read bytes %llu",
          info->get_data.offset, info->get_data.num_bytes);
      memset(info->get_data.buffer, 0, (uint32)info->get_data.num_bytes);
      // Read the given number of bytes from the given offset in the
      // file.
      info->get_data.num_bytes
        = (uint32)FILESOURCE_MIN (info->get_data.num_bytes,
        (m_fileSize - FILESOURCE_MIN (m_fileSize, info->get_data.offset)));

      /* In case of streaming scenario, m_bEndOfData flag will be used.
         Where as in case of local playback, EndOfData flag will be updated
         as TRUE, if no data is available to read from input file. */
      info->get_data.bEndOfData = m_bEndOfData;

      /* If number of bytes to be read is ZERO, mark EndOfData flag as TRUE
         in case of local file playback */
      if (!info->get_data.num_bytes
#if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
          && (!bHttpStreaming)
#endif
          )
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                    "Mpeg4File::mp4ParseStatus EndOfData flag is set to true");
        info->get_data.bEndOfData = TRUE;
      }

      uint32 ulDataEndOffset = (uint32)(info->get_data.offset +
                                        info->get_data.num_bytes);
      /* If local cache is available and data requested is within range,
         then copy data directly from cache instead of reading from file. */
      if ((m_pMoovAtomBuf) &&
          ((m_ulMoovBufDataRead + m_ullMoovAtomOffset) >= ulDataEndOffset) &&
          (m_ullMoovAtomOffset <= info->get_data.offset))
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "Mpeg4File::mp4ParseStatus copied from cache @ offset %llu",
                  info->get_data.offset);
        memcpy(info->get_data.buffer,
               m_pMoovAtomBuf + (info->get_data.offset - m_ullMoovAtomOffset),
               (uint32)info->get_data.num_bytes);
        break;
      }

      if ((m_ulMoovBufSize) && (!m_pMoovAtomBuf))
      {
        m_pMoovAtomBuf = (uint8*) MM_Malloc(m_ulMoovBufSize);
        if (!m_pMoovAtomBuf)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
            "Mpeg4File::mp4ParseStatus malloc failed for m_pMoovAtomBuf");
        }
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                     "Mpeg4File::mp4ParseStatus malloc for m_pMoovAtomBuf %x",
                     m_pMoovAtomBuf);
      }

      if(bHttpStreaming && !m_parsedEndofFile && m_mp4ParseServerData)
      {
        video_fmt_mp4r_context_type *context =
          (video_fmt_mp4r_context_type *) m_mp4ParseServerData;
        if(context->get_data_src_in_mdat &&
          ((info->get_data.offset + info->get_data.num_bytes -1) >= m_wBufferOffset))
        {
          //eg: h.263
          bsendParseFragmentCmd = FALSE;
          memset(info->get_data.buffer, 0x0, (uint32)info->get_data.num_bytes);
          /* Update the minimum offset required */
          m_minOffsetRequired = info->get_data.offset + info->get_data.num_bytes;
           info->get_data.num_bytes = 0;
          break;
        }
      }

      if ( info->get_data.num_bytes )
      {
        if(bHttpStreaming)
        {
          /* If offset requested is greater than the available offset, check
             once if data is available or not and updatem_wBufferOffset and
             m_bEndOfData parameters accordingly. As this check will be done
             only when data is not available, it wont result into calling of
             same API multiple times unless data under-run occurs frequently.*/
          if(info->get_data.offset >= m_wBufferOffset)
          {
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
            if(bHttpStreaming)
            {
              if(m_pStreamPort)
              {
                 bool bEndOfData = FALSE;
                 int64 wBufferOffset = 0;
                //Pull interface so pull download data size from OEM
                m_pStreamPort->GetAvailableOffset(&wBufferOffset, &bEndOfData);
                m_wBufferOffset = wBufferOffset;
                m_bEndOfData = bEndOfData;
              }
            }
            else
              m_bEndOfData = TRUE;
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
          }
          info->get_data.bEndOfData = m_bEndOfData;
          if(info->get_data.offset >= m_wBufferOffset)
          {
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
             "Mpeg4File::mp4ParseStatus Read offset %llu m_wBufferOffset %llu",
             info->get_data.offset, m_wBufferOffset);
            info->get_data.num_bytes = 0;
            memset(info->get_data.buffer, 0, (uint32)info->get_data.num_bytes);
            break;
          }
          else if((info->get_data.offset + info->get_data.num_bytes -1) >=
                  m_wBufferOffset)
          {
            info->get_data.num_bytes = m_wBufferOffset - info->get_data.offset;
          }
        }

        if( (info->get_data.offset+info->get_data.num_bytes) > m_fileSize )
        {
           /*
           * This can happen if file has some junk data at the end
           * or in case of encrypted file, there is some padding at the end.
           * Make sure we don't read beyond the file size.
           */
           MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "Mpeg4File::mp4ParseStatus Trying to read beyond file size: \
                  offset %llu nRead %llu file_size %llu",
                  info->get_data.offset, info->get_data.num_bytes, m_fileSize);
           /*
           * Try to adjust number of bytes to read but make sure delta is non
           * zero as info->get_data.num_bytes is unsigned int thus -ve number
           * will overflow.
           */
           if((signed) (m_fileSize - info->get_data.offset) >=(signed) 0)
           {
             info->get_data.num_bytes =
                      (uint32)(m_fileSize - info->get_data.offset);
           }
           else
           {
             info->get_data.num_bytes = 0;
           }
           MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                        "Mpeg4File::mp4ParseStatus Adjusted read bytes %llu",
                        info->get_data.num_bytes);
        }
        if(info->get_data.num_bytes > 0)
        {
          /* Read MOOV atom worth of data into local cache */
          if ((m_ulMoovBufDataRead == 0) && (m_pMoovAtomBuf) &&
              (m_minOffsetRequired) &&
              (info->get_data.offset >= m_ullMoovAtomOffset))
          {
            m_ulMoovBufSize = FILESOURCE_MIN(m_ulMoovBufSize, (uint32)
                                           (m_fileSize - m_ullMoovAtomOffset));
            do
            {
              m_ulMoovBufDataRead = readFile (localParseFilePtr,
                                              m_pMoovAtomBuf,
                                              m_ullMoovAtomOffset,
                                              m_ulMoovBufSize,
                                              &m_bDataUnderRun, m_bMediaAbort);
              if((!m_bDataUnderRun) || (m_ulMoovBufDataRead) ||
                 (FILE_SOURCE_MP4_DASH == m_eFileFormat))
              {
                MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                          "mp4ParseStatus read into cache size %lu @ %llu",
                          m_ulMoovBufSize, m_ullMoovAtomOffset);
                break;
              }
              MM_Timer_Sleep(200);
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                        "mp4ParseStatus read underrun cache size %lu",
                        m_ulMoovBufSize);
            } while(m_bDataUnderRun);
          }
          m_bDataUnderRun = false;
          /* If local cache is available and data requested is within range,
             then copy data directly from cache instead of reading from file. */
          if ((m_pMoovAtomBuf) && (m_ulMoovBufDataRead) &&
              (m_ulMoovBufDataRead + m_ullMoovAtomOffset >= ulDataEndOffset) &&
              (m_ullMoovAtomOffset <= info->get_data.offset))
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "Mpeg4File::mp4ParseStatus copied from cache @ offset %llu",
                   info->get_data.offset);
            memcpy(info->get_data.buffer,
               m_pMoovAtomBuf + (info->get_data.offset - m_ullMoovAtomOffset),
               (uint32)info->get_data.num_bytes);
            break;
          }
          /* If sufficient data is not available in the cache, then read from
             file. */
          do
          {
            uint32 ulDataRead = 0;
            ulDataRead = readFile (localParseFilePtr,
                                   info->get_data.buffer,
                                   info->get_data.offset,
                                   (uint32)info->get_data.num_bytes,
                                   &m_bDataUnderRun, m_bMediaAbort);
            //! Do not continue the loop if file is 'DASH' complaint
            if((!m_bDataUnderRun) || (ulDataRead) ||
               (FILE_SOURCE_MP4_DASH == m_eFileFormat))
            {
              info->get_data.num_bytes = ulDataRead;
              if (m_bMediaAbort)
              {
                info->get_data.bEndOfData = TRUE;
              }
              break;
            }
            MM_Timer_Sleep(100);
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
              "Mpeg4File::mp4ParseStatus read underrun @ %llu, size %llu",
              info->get_data.offset, info->get_data.num_bytes);
          } while(m_bDataUnderRun);
          m_bDataUnderRun = false;
          if (!info->get_data.num_bytes)
          {
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
              "Mpeg4File::mp4ParseStatus Failed to readFile @ %llu, size %llu",
              info->get_data.offset, info->get_data.num_bytes);
#if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
            if(!bHttpStreaming)
#endif
            {
              //This should be done for local playback as
              //for PD,PS, it is possible to have 0 bytes being read.
              //Set the error code to let player know the exact failure.
              //This will help to stop/abort the playback.
              _fileErrorCode = PARSER_ErrorReadFail;
            }
          }
        }
      }
    }
    break;

  case VIDEO_FMT_PUT_DATA:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "Mpeg4File::mp4ParseStatus VIDEO_FMT_PUT_DATA");
    break;

  case VIDEO_FMT_CONTINUE:
    {
      m_mp4ParseContinueCb = info->cont.callback_ptr;
      m_mp4ParseServerData = info->cont.server_data;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "Mpeg4File::mp4ParseStatus VIDEO_FMT_CONTINUE");
    }
    break;

  case VIDEO_FMT_INFO:
  case VIDEO_FMT_FRAGMENT:
    {
      process_video_fmt_info(status,info);
    }
    break;

  case VIDEO_FMT_UUID:
    mp4ParseUUIDAtom(&(info->uuid_atom),localParseFilePtr);
    break;

  case VIDEO_FMT_UDTA_CHILD:
    mp4ParseUUIDAtom(&(info->uuid_atom),localParseFilePtr);
    break;

  case VIDEO_FMT_TEXT:
    if(localParseFilePtr == NULL)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
        "Mpeg4File::mp4ParseStatus VIDEO_FMT_TEXT localParseFilePtr is NULL");
      break;
    }
    ParseTimedTextAtom(&(info->text_atom),localParseFilePtr);
    break;

  case VIDEO_FMT_IO_DONE:
    {
      m_parseIODoneSize = info->io_done.bytes;
    }
    break;

  case VIDEO_FMT_DONE:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "Mpeg4File::mp4ParseStatus VIDEO_FMT_DONE");
    break;

  case VIDEO_FMT_HINT:
    {
      video_fmt_mp4_atom_type* pAtomInfo = info->hint.mp4;
#ifdef FEATURE_MP4_CUSTOM_META_DATA
      if( pAtomInfo->type == SCHM_TYPE || pAtomInfo->type == FRMA_TYPE
        || pAtomInfo->type == OHDR_TYPE || pAtomInfo->type == ODAF_TYPE
        || pAtomInfo->type == MDRI_TYPE)
      {
        DataT atom_type = DATA_ATOM_NONE;
        DataT atom_offset_type = DATA_ATOM_NONE;
        switch(pAtomInfo->type)
        {
          case SCHM_TYPE:
            atom_type = DATA_ATOM_SCHM;
            atom_offset_type = DATA_ATOM_SCHM_OFFSET;
            break;
          case FRMA_TYPE:
            atom_type = DATA_ATOM_FRMA;
            atom_offset_type = DATA_ATOM_FRMA_OFFSET;
            break;
          case OHDR_TYPE:
            atom_type = DATA_ATOM_OHDR;
            atom_offset_type = DATA_ATOM_OHDR_OFFSET;
            break;
          case ODAF_TYPE:
            atom_type = DATA_ATOM_ODAF;
            atom_offset_type = DATA_ATOM_ODAF_OFFSET;
            break;
          case MDRI_TYPE:
            atom_type = DATA_ATOM_MDRI;
            atom_offset_type = DATA_ATOM_MDRI_OFFSET;
            break;
          default:
            break;
        }
        if((m_playAudio && (info->hint.stream_info->type == VIDEO_FMT_STREAM_AUDIO)) ||
           (m_playVideo && (info->hint.stream_info->type == VIDEO_FMT_STREAM_VIDEO)))
        {
          if(_pdcfAtom == NULL)
          {
            _pdcfAtom = MM_New( PdcfAtom );
          }
          if(!_pdcfAtom->saveAtom(atom_type, atom_offset_type,
                                  info->hint.stream_info->track_id,
                                  pAtomInfo->size, pAtomInfo->offset)
            )
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                    "Mpeg4File::mp4ParseStatus failed to store the pdcfAtom");
          }
        }
      }
#endif /* FEATURE_MP4_CUSTOM_META_DATA */

      if( pAtomInfo->type == FTYP_TYPE || pAtomInfo->type == CPRT_TYPE ||
          pAtomInfo->type == AUTH_TYPE || pAtomInfo->type == TITL_TYPE ||
          pAtomInfo->type == DSCP_TYPE || pAtomInfo->type == DCMD_DRM_ATOM||
          pAtomInfo->type == RTNG_TYPE || pAtomInfo->type == GNRE_TYPE||
          pAtomInfo->type == PERF_TYPE || pAtomInfo->type == CLSF_TYPE||
          pAtomInfo->type == KYWD_TYPE || pAtomInfo->type == LOCI_TYPE||
          pAtomInfo->type == META_TYPE || pAtomInfo->type == UDTA_TYPE||
          pAtomInfo->type == ALBM_TYPE || pAtomInfo->type == SUBS_TYPE||
          pAtomInfo->type == PSSH_TYPE || pAtomInfo->type == SINF_TYPE||
          pAtomInfo->type == GEOT_TYPE || pAtomInfo->type == YRRC_TYPE
        )
      {
        uint64 saveFilePos = OSCL_FileTell (localParseFilePtr);
        /* for child videoFMT offset is after atom size and type, but
        current parsing needs atom header as well, so goback 8 bytes */
        if (pAtomInfo->offset >= DEFAULT_UDTA_CHILD_ATOM_HEADER_SIZE)
          (void)OSCL_FileSeek(localParseFilePtr,
          (pAtomInfo->offset-DEFAULT_UDTA_CHILD_ATOM_HEADER_SIZE),
          SEEK_SET);

        switch(pAtomInfo->type)
        {
        case UDTA_TYPE:
          m_bUdtaAtomPresent=true;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "3GPP Udta atom present..");
          break;
        case CPRT_TYPE: /* 'cprt' */
          if(_cprtAtom)
            MM_Delete( _cprtAtom );
          _cprtAtom = MM_New_Args( UdtaCprtAtom, (localParseFilePtr) );
          if(_cprtAtom != NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "CopyRight Atom Size = %lu",
                        _cprtAtom->getUdtaCprtDataSize());
          }
          break;
        case AUTH_TYPE: /* 'auth' */
          if(_authAtom)
            MM_Delete( _authAtom );
          _authAtom = MM_New_Args( UdtaAuthAtom, (localParseFilePtr) );
          if(_authAtom != NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "Author Atom Size = %lu",
                        _authAtom->getUdtaAuthDataSize());
          }
          break;
        case TITL_TYPE: /* 'titl' */
          if(_titlAtom)
            MM_Delete( _titlAtom );
          _titlAtom = MM_New_Args( UdtaTitlAtom, (localParseFilePtr) );
          if(_titlAtom != NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "Title Atom Size = %lu",
                         _titlAtom->getUdtaTitlDataSize());
          }
          break;
        case DSCP_TYPE: /* 'dscp' */
          if(_dscpAtom)
            MM_Delete( _dscpAtom );
          _dscpAtom = MM_New_Args( UdtaDscpAtom, (localParseFilePtr) );
          if(_dscpAtom != NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
              "Description Atom Size = %lu", _dscpAtom->getUdtaDscpDataSize());
          }
          break;
#ifdef FEATURE_MP4_CUSTOM_META_DATA
        case FTYP_TYPE: /* 'ftyp' */
          if(_ftypAtom)
            MM_Delete( _ftypAtom );
          _ftypAtom = MM_New_Args( FtypAtom, (localParseFilePtr) );
          if(_ftypAtom != NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "FileType Atom Size = %lu",
              _ftypAtom->getFtypDataSize());
          }
          break;
        case  DCMD_DRM_ATOM: /* 'dcmd' */
          if(_dcmdAtom)
            MM_Delete( _dcmdAtom );
          _dcmdAtom = MM_New_Args( DcmdDrmAtom, (localParseFilePtr) );
          if(_dcmdAtom != NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "DCMD DRM Atom Size = %lu",
                        _dcmdAtom->getDcmdDataSize());
          }
          break;
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
        case  RTNG_TYPE:
          if(_rtngAtom)
            MM_Delete( _rtngAtom );
          _rtngAtom= MM_New_Args( UdtaRtngAtom, (localParseFilePtr) );
          if(_rtngAtom != NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "Rating Atom Size = %lu",
                         _rtngAtom->getUdtaRtngDataSize());
          }
          break;
        case  GNRE_TYPE:
          if(_gnreAtom)
            MM_Delete( _gnreAtom );
          _gnreAtom= MM_New_Args( UdtaGnreAtom, (localParseFilePtr) );
          if(_gnreAtom != NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "Genre Atom Size = %lu",
                         _gnreAtom->getUdtaGnreDataSize());
          }
          break;
        case  PERF_TYPE:
          if(_perfAtom)
            MM_Delete( _perfAtom );
          _perfAtom= MM_New_Args( UdtaPerfAtom, (localParseFilePtr) );
           if(_perfAtom != NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
              "Performance Atom Size = %lu", _perfAtom->getUdtaPerfDataSize());
          }
          break;
        case  CLSF_TYPE:
          if(_clsfAtom)
            MM_Delete( _clsfAtom );
          _clsfAtom= MM_New_Args( UdtaClsfAtom, (localParseFilePtr) );
         if(_clsfAtom != NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
            "Classification Atom Size = %lu", _clsfAtom->getUdtaClsfDataSize());
          }
          break;
        case  KYWD_TYPE:
          if(_kywdAtom)
            MM_Delete( _kywdAtom );
          _kywdAtom= MM_New_Args( UdtaKywdAtom, (localParseFilePtr) );
          if(_kywdAtom != NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "Keyword Atom Size = %lu",
                        _kywdAtom->getUdtaKywdDataSize());
          }
          break;
        case  LOCI_TYPE:
          if(_lociAtom)
            MM_Delete( _lociAtom );
          _lociAtom= MM_New_Args( UdtaLociAtom, (localParseFilePtr) );
           if(_lociAtom != NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "Location Atom Size = %lu",
                         _lociAtom->getUdtaLociDataSize());
          }
          break;
        case YRRC_TYPE:
          if (pYrccAtom)
            MM_Delete(pYrccAtom);
          pYrccAtom = MM_New_Args( UdtaYrrcAtom, (localParseFilePtr) );
          break;
        case  ALBM_TYPE:
          if(_albumAtom)
            MM_Delete( _albumAtom );
          _albumAtom= MM_New_Args( UdtaAlbumAtom, (localParseFilePtr) );
           if(_albumAtom != NULL)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "Album Atom Size = %lu",
                        _albumAtom->getUdtaAlbumDataSize());
          }
          break;
        case  META_TYPE:
          if(_metaAtom)
            MM_Delete( _metaAtom );
          _metaAtom= MM_New_Args( UdtaMetaAtom, (localParseFilePtr) );
          if(_metaAtom)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "Meta Atom Size = %lu",
                         _metaAtom->getUdtaMetaDataSize());
          }
          break;

        case  SUBS_TYPE:
          if(pSubsAtom)
            MM_Delete( pSubsAtom );
          pSubsAtom= MM_New_Args( CSubsAtom, (localParseFilePtr) );
          if(pSubsAtom)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "subs Atom size = %lu",
                        pSubsAtom->GetSubsDataSize());
          }
          break;
        case PSSH_TYPE:
          {
            CPsshAtom *pPsshAtom = MM_New_Args( CPsshAtom, (localParseFilePtr) );
            if(pPsshAtom != NULL)
            {
              if (!pPsshAtom->FileSuccess())
              {
                MM_Delete( pPsshAtom );
                //! Here even though the PSSH is corrupted let playback continue.
                MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "Mpeg4File::mp4ParseStatus PSSH atom parsing failed");
                pPsshAtom = NULL;
              }
              else
              {
                pPsshAtom->setParent(this);
                m_aPSSHatomEntryArray += pPsshAtom;
                m_ulPSSHatomEntryCount++;
              }
            }
          }
          break;
        case SINF_TYPE:
          {
            CSinfAtom *pSinfAtom = MM_New_Args( CSinfAtom, (localParseFilePtr) );
            if(pSinfAtom != NULL)
            {
              if (!pSinfAtom->FileSuccess())
              {
                MM_Delete( pSinfAtom );
                //! Here even though the SINF is corrupted let playback continue.
                MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "Mpeg4File::mp4ParseStatus SINF atom parsing failed");
              }
              else
              {
                /*! Parse the SINF atom details.
                  ! If input file is encrypted, then both SINF and PSSH atoms
                  ! together. So one count variable will be sufficient to
                  ! handle both PSSH and SINF atom counts.
                */
                pSinfAtom->Parse();
                pSinfAtom->setParent(this);
                m_aSINFatomEntryArray += pSinfAtom;
              }
            }
          }
          break;

        case  GEOT_TYPE:
          {
            uint32 ulAtomSize    = pAtomInfo->size;
            uint64 ullAtomOffset = pAtomInfo->offset;
            bool bError          = false;

            /* This Metadata atom contains Geo Tag Location.
               Atom Structure:
               uint16   --  Data String Length
               uint16   --  Language Code
               uint8    --  0x2B ['+']
               uint8[]  --  Geo Data String
               uint8    --  0x2E [Forward slash '/']
               Worst case the location string length would be 18,
               For Example, +90.0000-180.0000 without '/'.
            */
            if((ulAtomSize < 8) || ((ulAtomSize - 5) >= sizeof(m_ucGeoTagLoc)))
            {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                          "GEOT Atom is corrupted");
              break;
            }
            /* First 4 bytes at start and 1byte at end are not required for
               Location info.*/
            ullAtomOffset += FOURCC_SIGNATURE_BYTES;
            ulAtomSize    -= 5;

            /* Read Geo Tag data from internal cache. */
            if ((m_pMoovAtomBuf) &&
                ((m_ulMoovBufDataRead+m_ullMoovAtomOffset) >= ullAtomOffset) &&
                (m_ullMoovAtomOffset <= ullAtomOffset))
            {
              memcpy(m_ucGeoTagLoc,
                     m_pMoovAtomBuf + (ullAtomOffset - m_ullMoovAtomOffset),
                     ulAtomSize);
            }
            else
            {
              ulAtomSize = readFile (localParseFilePtr, m_ucGeoTagLoc,
                                     ullAtomOffset, ulAtomSize,
                                     &m_bDataUnderRun);
            }
            m_ucGeoTagLoc[ulAtomSize] = '\0';
            m_ulGeoTagSize = ulAtomSize + 1;
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                         "GEO Tag info is present, Str is %s",
                         (char*)m_ucGeoTagLoc);
          }
          break;

        default:
          break;
        }
        (void)OSCL_FileSeek(localParseFilePtr, saveFilePos, SEEK_SET);
      }
      //MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "VIDEO_FMT_HINT");
      break;
    }

  case VIDEO_FMT_FAILURE:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "Mpeg4File::mp4ParseStatus VIDEO_FMT_FAILURE");
    if(!m_parsedEndofFile)
      m_parsedEndofFile = true;
    m_corruptFile = true;
    break;

  case VIDEO_FMT_BUSY:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "Mpeg4File::mp4ParseStatus VIDEO_FMT_BUSY");
    break;

  case VIDEO_FMT_INIT:
    m_trackCount = 0;
    memcpy ( &m_videoFmtInfo, (void *)&info->info, sizeof(video_fmt_info_type) );
    // Save stream reading callback.
    m_mp4ParseServerData = info->info.server_data;
    break;

  case VIDEO_FMT_DATA_CORRUPT:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "Mpeg4File::mp4ParseStatus VIDEO_FMT_DATA_CORRUPT");
    if(!m_parsedEndofFile)
      m_parsedEndofFile = true;
    m_corruptFile = true;
    break;

#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)

  case VIDEO_FMT_FRAGMENT_SIZE:
    memcpy ( &m_videoFmtInfo, (void *)&info->info, sizeof(video_fmt_info_type) );
    // Save stream reading callback.
    m_mp4ParseServerData = info->info.server_data;
    break;

  case VIDEO_FMT_FRAGMENT_PEEK:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "Mpeg4File::mp4ParseStatus VIDEO_FMT_FRAGMENT_PEEK");
    break;

  case VIDEO_FMT_DATA_INCOMPLETE:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "Mpeg4File::mp4ParseStatus underrun occurred in MOOV/MOOF");
    break;
#endif //FEATURE_FILESOURCE_PSEUDO_STREAM

  case VIDEO_FMT_STATUS_INVALID:
  default:
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "Mpeg4File::mp4ParseStatus Invalid status = %d", status);
    break;
  }
}

/* ======================================================================
FUNCTION
Mpeg4File::mp4ParseStatusCallback

DESCRIPTION
This function is registered with the video format services as the status
callback function.  It receives any output data coming from that module.

IN video_fmt_status_type
The current status of the video format services.

IN void *client_data
This is actually a pointer to the Mpeg4File object which is the
client of the video format services.

INOUT video_fmt_status_cb_info_type *info
This points to a union containing information relating to the current
video format services status.  Depending on the status, the video
format services may require the callback function perform some
function with this data, and possibly modify the data.

IN video_fmt_end_cb_func_type end
This points to the function to be called when the current session of
the video format services should be closed.

DEPENDENCIES
List any dependencies for this function, global variables, state,
resource availability, etc.

RETURN VALUE
Enumerate possible return values

SIDE EFFECTS
Detail any side effects.

========================================================================== */
void Mpeg4File::mp4ParseStatusCallback (video_fmt_status_type status,
                                        void *client_data,
                                        void *info,
                                        video_fmt_end_cb_func_type end)
{
  Mpeg4File * Mp4FilePtr = (Mpeg4File *) (client_data);
  Mp4FilePtr->mp4ParseStatus (status, (video_fmt_status_cb_info_type*)info, end);
}

/* ======================================================================
FUNCTION
Mpeg4File::mp4ReadStatus

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
void Mpeg4File::mp4ReadStatus (uint32 streamNum,
                               video_fmt_status_type status,
                               video_fmt_status_cb_info_type *info,
                               video_fmt_end_cb_func_type)
{
  // Store the status code for later inspection.
  m_mp4ReadLastStatus[streamNum] = status;

  // Store session end callback function.

  // Branch according to the status code received.
  switch (status)
  {
  case VIDEO_FMT_ALLOC:
    info->alloc.ptr = (char*)MM_Malloc(info->alloc.size);
    if ( info->alloc.ptr == NULL )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "Mpeg4File::mp4ReadStatus malloc failed");
    }
    else
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Mpeg4File::mp4ReadStatus alloc");
    break;

  case VIDEO_FMT_FREE:
    MM_Free(info->free.ptr);
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Mpeg4File::mp4ReadStatus free");
    break;

  case VIDEO_FMT_GET_DATA:
    {

      /* This flag will be set by the UI task if the user terminates the
         playback. By setting EOD flag as true, Videofmt will treat this
         as read failure and abort the playback. If EOD flag is not set to
         true, then Videofmt will consider it as Under-run, rather than
         read failure. */
        if(m_bMediaAbort)
        {
          info->get_data.num_bytes  = 0;
          info->get_data.bEndOfData = TRUE;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                      "Mpeg4File::mp4ReadStatus Failed to readFile"
                      ": User aborted playback..!!");
          _fileErrorCode = PARSER_ErrorReadFail;
          break;
        }
      // Read the given number of bytes from the given offset in the
      // file.
      info->get_data.num_bytes
        = (uint32)FILESOURCE_MIN (info->get_data.num_bytes,
        (m_fileSize - FILESOURCE_MIN (m_fileSize, info->get_data.offset)));

      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "Mpeg4File::mp4ReadStatus sample offset %llu, size %llu",
                   info->get_data.offset, info->get_data.num_bytes);

      uint32 ulDataEndOffset = (uint32)(info->get_data.offset +
                                        info->get_data.num_bytes);
      /* If Internal cache is available and data requested was within range*/
      if ((m_pMoovAtomBuf) &&
          ((m_ulMoovBufDataRead + m_ullMoovAtomOffset) >= ulDataEndOffset) &&
          (m_ullMoovAtomOffset <= info->get_data.offset))
      {
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
          "mp4ReadStatus copied from local buf @ offset %llu, bytes %llu",
          info->get_data.offset, info->get_data.num_bytes);
        memcpy(info->get_data.buffer,
               m_pMoovAtomBuf + (info->get_data.offset - m_ullMoovAtomOffset),
               (uint32)info->get_data.num_bytes);
        if(info->get_data.num_bytes)
        {
          m_mp4ReadContinueCb[streamNum] = info->get_data.callback_ptr;
          m_mp4ReadServerData[streamNum] = info->get_data.server_data;
        }
        //Break switch case
        break;
      }

      if ( info->get_data.num_bytes )
      {
        info->get_data.num_bytes = readFile (m_parseFilePtr,
          info->get_data.buffer,
          info->get_data.offset,
          (uint32)info->get_data.num_bytes, &m_bDataUnderRun);
        if(info->get_data.num_bytes)
        {
          m_mp4ReadContinueCb[streamNum] = info->get_data.callback_ptr;
          m_mp4ReadServerData[streamNum] = info->get_data.server_data;
        }
        else
        {
          info->get_data.bEndOfData = m_bEndOfData;
          if (false == m_bDataUnderRun)
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                        "Mpeg4File::mp4ReadStatus Failed to readFile");
          }
          else
          {
            info->get_data.bEndOfData = FALSE;
          }

#if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
          if(!bHttpStreaming)
#endif
          {
            //This should be done for local playback as
            //for PD,PS, it is possible to have 0 bytes being read.
            //Set the error code to let player know the exact failure.
            //This will help to stop/abort the playback.
            _fileErrorCode = PARSER_ErrorReadFail;
          }
        }
      }
    }
    break;

  case VIDEO_FMT_PUT_DATA:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "VIDEO_FMT_PUT_DATA");
    break;

  case VIDEO_FMT_CONTINUE:
    {
      m_mp4ReadContinueCb[streamNum] = info->cont.callback_ptr;
      m_mp4ReadServerData[streamNum] = info->cont.server_data;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "VIDEO_FMT_CONTINUE");
    }
    break;

  case VIDEO_FMT_IO_DONE:
    {
      m_iodoneSize[streamNum] = info->io_done.bytes;
    }
    break;

  case VIDEO_FMT_DONE:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "VIDEO_FMT_DONE");
    break;

  case VIDEO_FMT_HINT:
    // MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "VIDEO_FMT_HINT");
    break;

  case VIDEO_FMT_FAILURE:
    {
      if(m_videoFmtInfo.streams[streamNum].type == VIDEO_FMT_STREAM_VIDEO)
      {
        m_hasVideo = false;
      }
      else if(m_videoFmtInfo.streams[streamNum].type == VIDEO_FMT_STREAM_AUDIO)
      {
        m_hasAudio = false;
      }
      else if(m_videoFmtInfo.streams[streamNum].type == VIDEO_FMT_STREAM_TEXT)
      {
        m_hasText = false;
      }
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "Mpeg4File::mp4ReadStatus VIDEO_FMT_FAILURE");
      break;
    }

  case VIDEO_FMT_BUSY:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "Mpeg4File::mp4ReadStatus VIDEO_FMT_BUSY");
    break;

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  case VIDEO_FMT_DATA_INCOMPLETE:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "Mpeg4File::mp4ReadStatus underrun in media data");
    break;
  case VIDEO_FMT_ABS_FILE_OFFSET:
    m_absFileOffset[streamNum] = info->info.abs_file_offset;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "VIDEO_FMT_ABS_FILE_OFFSET");
    break;
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

  case VIDEO_FMT_STATUS_INVALID:
  default:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
      "Mpeg4File::mp4ReadStatus VIDEO_FMT_STATUS_INVALID");
    break;
  }
}

/* ======================================================================
FUNCTION
Mpeg4File::mp4ReadStatusCallback

DESCRIPTION
This function is registered with the video format services as the status
callback function when reading stream data.  It receives any output data
coming from that module.

IN video_fmt_status_type
The current status of the video format services.

IN void *client_data
This is actually a pointer to the CMP4parseDoc object which is the
client of the video format services.

INOUT video_fmt_status_cb_info_type *info
This points to a union containing information relating to the current
video format services status.  Depending on the status, the video
format services may require the callback function perform some
function with this data, and possibly modify the data.

IN video_fmt_end_cb_func_type end
This points to the function to be called when the current session of
the video format services should be closed.

DEPENDENCIES
List any dependencies for this function, global variables, state,
resource availability, etc.

RETURN VALUE
Enumerate possible return values

SIDE EFFECTS
Detail any side effects.

========================================================================== */
void Mpeg4File::mp4ReadStatusCallback (video_fmt_status_type status,
                                       void *client_data,
                                       void *info,
                                       video_fmt_end_cb_func_type end)
{
  videoFMTClientData * clientData = (videoFMTClientData *) (client_data);
  clientData->Mp4FilePtr->mp4ReadStatus
                         ((uint32)(clientData->streamNum), status,
                          (video_fmt_status_cb_info_type*)info, end);
}

/* ======================================================================
FUNCTION
Mpeg4File::mp4SyncStatus

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
void Mpeg4File::mp4SyncStatus (uint32 streamNum,
                               video_fmt_status_type status,
                               video_fmt_status_cb_info_type *info,
                               video_fmt_end_cb_func_type)
{
  // Store the status code for later inspection.
  m_mp4SyncLastStatus[streamNum] = status;
  uint32 ulDataEndOffset         = 0;
  // Store session end callback function.

  // Branch according to the status code received.
  switch (status)
  {

  case VIDEO_FMT_GET_DATA:
      /* This flag will be set by the UI task if the user terminates the
         playback. By setting EOD flag as true, Videofmt will treat this
         as read failure and abort the playback. If EOD flag is not set to
         true, then Videofmt will consider it as Under-run, rather than
         read failure. */
      if(m_bMediaAbort)
      {
        info->get_data.num_bytes  = 0;
        info->get_data.bEndOfData = TRUE;
        _fileErrorCode            = PARSER_ErrorReadFail;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                    "Failed to readFile (mp4SyncStatus):"
                    "User aborted playback..!!");
        break;
      }
    // Read the given number of bytes from the given offset in the
    // file.
    info->get_data.num_bytes
      = (uint32)FILESOURCE_MIN (info->get_data.num_bytes,
      (m_fileSize - FILESOURCE_MIN (m_fileSize, info->get_data.offset)));

    ulDataEndOffset = (uint32)(info->get_data.offset +
                                      info->get_data.num_bytes);
    /* If Internal cache is available and data requested was within range*/
    if ((m_pMoovAtomBuf) &&
        ((m_ulMoovBufDataRead + m_ullMoovAtomOffset) >= ulDataEndOffset) &&
        (m_ullMoovAtomOffset <= info->get_data.offset))
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "mp4SyncStatus copied from local buf @ offset %llu, bytes %llu",
        info->get_data.offset, info->get_data.num_bytes);
      memcpy(info->get_data.buffer,
             m_pMoovAtomBuf + (info->get_data.offset - m_ullMoovAtomOffset),
             (uint32)info->get_data.num_bytes);
      if(info->get_data.num_bytes)
      {
        m_mp4SyncContinueCb[streamNum] = info->get_data.callback_ptr;
        m_mp4SyncServerData[streamNum] = info->get_data.server_data;
      }
      //Break switch case
      break;
    }

    if ( info->get_data.num_bytes )
    {
      info->get_data.num_bytes = readFile (m_parseFilePtr,
        info->get_data.buffer,
        info->get_data.offset,
        (uint32)info->get_data.num_bytes);
      if (info->get_data.num_bytes)
      {
        m_mp4SyncContinueCb[streamNum] = info->get_data.callback_ptr;
        m_mp4SyncServerData[streamNum] = info->get_data.server_data;
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                    "Mpeg4File::mp4SyncStatus Failed to readFile");
#if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
          if(!bHttpStreaming)
#endif
          {
            //This should be done for local playback as
            //for PD,PS, it is possible to have 0 bytes being read.
            //Set the error code to let player know the exact failure.
            //This will help to stop/abort the playback.
            _fileErrorCode = PARSER_ErrorReadFail;
          }
      }
    }
    break;

  case VIDEO_FMT_CONTINUE:
    {
      m_mp4SyncContinueCb[streamNum] = info->cont.callback_ptr;
      m_mp4SyncServerData[streamNum] = info->cont.server_data;
    }
    break;

  case VIDEO_FMT_IO_DONE:
    {
      m_iodoneSize[streamNum] = info->io_done.bytes;
    }
    break;

  case VIDEO_FMT_DONE:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "VIDEO_FMT_DONE");
    break;

  case VIDEO_FMT_HINT:
    // MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "VIDEO_FMT_HINT");
    break;

  case VIDEO_FMT_FAILURE:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "Mpeg4File::mp4SyncStatus VIDEO_FMT_FAILURE");
    break;

  case VIDEO_FMT_BUSY:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "Mpeg4File::mp4SyncStatus VIDEO_FMT_BUSY");
    break;

  case VIDEO_FMT_STATUS_INVALID:
  default:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "Mpeg4File::mp4SyncStatus VIDEO_FMT_STATUS_INVALID");
    break;
  }
}

/* ======================================================================
FUNCTION
Mpeg4File::mp4SyncStatusCallback

DESCRIPTION
This function is registered with the video format services as the status
callback function when reading stream data.  It receives any output data
coming from that module.

IN video_fmt_status_type
The current status of the video format services.

IN void *client_data
This is actually a pointer to the CMP4parseDoc object which is the
client of the video format services.

INOUT video_fmt_status_cb_info_type *info
This points to a union containing information relating to the current
video format services status.  Depending on the status, the video
format services may require the callback function perform some
function with this data, and possibly modify the data.

IN video_fmt_end_cb_func_type end
This points to the function to be called when the current session of
the video format services should be closed.

DEPENDENCIES
List any dependencies for this function, global variables, state,
resource availability, etc.

RETURN VALUE
Enumerate possible return values

SIDE EFFECTS
Detail any side effects.

========================================================================== */
void Mpeg4File::mp4SyncStatusCallback (video_fmt_status_type status,
                                       void *client_data,
                                       void *info,
                                       video_fmt_end_cb_func_type end)
{
  videoFMTClientData * clientData = (videoFMTClientData *) (client_data);
  clientData->Mp4FilePtr->mp4SyncStatus ((uint32)(clientData->streamNum), status,
                                         (video_fmt_status_cb_info_type*)info, end);
}

/* ======================================================================
FUNCTION
      Mpeg4File::GetStreamParameter()

  DESCRIPTION
      This function is used to get Audio/Video stream parameter i.e.
      codec configuration, profile, level from specific parser.

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
PARSER_ERRORTYPE Mpeg4File::GetStreamParameter( uint32 ulTrackId,
                                               uint32 ulParamIndex,
                                               void* pParamStruct)
{
  PARSER_ERRORTYPE eError = PARSER_ErrorDefault;
  OSCL_FILE* localParseFilePtr = m_parseFilePtr;

  if(pParamStruct == NULL)
  {
    eError = PARSER_ErrorInvalidParam;
  }
  else
  {
    video_fmt_stream_info_type *pTrackInfo = getTrackInfoForID(ulTrackId);
    if( pTrackInfo != NULL )
    {
      switch(ulParamIndex)
      {
      case FS_IndexParamVideoH264:
        {
          FS_VIDEO_PARAM_H264TYPE *pH264Info = (FS_VIDEO_PARAM_H264TYPE*)pParamStruct;
          // Update number of views information
          if(pTrackInfo->dec_specific_info.h264_info.vwid_info)
          {
            pH264Info->ulNumberOfViews =
              pTrackInfo->dec_specific_info.h264_info.vwid_info->num_views;
          }
          else
          {
            // For AVC there will be only single view always
            pH264Info->ulNumberOfViews = 1;
          }
          //Update profile & level information
          if( ( pH264Info->ulNumberOfViews > 1) &&
            ( pTrackInfo->dec_specific_info.h264_info.mvcc_info!= NULL ) )
          {
            // MVC case
            pH264Info->ucH264ProfileInfo =
              pTrackInfo->dec_specific_info.h264_info.mvcc_info->MVCProfileIndication;
            pH264Info->ucH264LevelInfo =
              pTrackInfo->dec_specific_info.h264_info.mvcc_info->MVCLevelIndication;
            pH264Info->ucCompleteRepresentation =
              pTrackInfo->dec_specific_info.h264_info.mvcc_info->complete_representation;
            pH264Info->ucExplicitAuTrack =
              pTrackInfo->dec_specific_info.h264_info.mvcc_info->explicit_au_track;
          }
          else if( pTrackInfo->dec_specific_info.h264_info.avcc_info != NULL )
          {
            // AVC case
            pH264Info->ucH264ProfileInfo =
              pTrackInfo->dec_specific_info.h264_info.avcc_info->AVCProfileIndication;
            pH264Info->ucH264LevelInfo =
              pTrackInfo->dec_specific_info.h264_info.avcc_info->AVCLevelIndication;
          }
          eError = PARSER_ErrorNone;
        }
        break;//case FS_IndexParamVideoH264

      case FS_IndexParamOtherMediaTrackInfo:
        {
          if ( VIDEO_FMT_STREAM_TEXT == pTrackInfo->type )
          {
            FileSourceTextInfo *pTextTrackInfo = (FileSourceTextInfo *)pParamStruct;
            pTextTrackInfo->frameHeight = pTrackInfo->tkhd_height >> 16;
            pTextTrackInfo->frameWidth = pTrackInfo->tkhd_width >> 16;
            pTextTrackInfo->duration = getTrackMediaDuration(ulTrackId);
            pTextTrackInfo->timeScale = getTrackMediaTimescale(ulTrackId);
          }
          eError = PARSER_ErrorNone;
        }
        break;//case FS_IndexParamOtherMediaTrackInfo

      case FS_IndexParamOtherPSSHInfo:
        {
          //! Update the PSSHInfo Structure.
          FS_PROTECTION_SYSTEM_SPECIFIC_INFOTYPE *pPSSHInfo =
            (FS_PROTECTION_SYSTEM_SPECIFIC_INFOTYPE*)pParamStruct;
          for (uint32 i = 0; i < m_ulPSSHatomEntryCount; i++)
          {
            CPsshAtom *pPsshAtom = NULL;
            CSinfAtom *pSinfAtom = NULL;
            if(m_aPSSHatomEntryArray.GetLength() > i)
            {
              pPsshAtom = m_aPSSHatomEntryArray.ElementAt(i);
            }
            // uint32 lenght =m_aSINFatomEntryArray.GetLength();
            if(m_aSINFatomEntryArray.GetLength() > i)
            {
              pSinfAtom = m_aSINFatomEntryArray.ElementAt(i);
            }
            pPSSHInfo[i].ePSDRMType = FILE_SOURCE_DRM_UNKNOWN;
            if(pSinfAtom && pSinfAtom->m_pSchmAtom)
            {
              pPSSHInfo[i].ePSDRMType = pSinfAtom->m_pSchmAtom->m_drmType;
            }
            if(pPsshAtom)
            {
              pPSSHInfo[i].ulDRMIndex = i;
              memcpy ((uint8 *) &pPSSHInfo[i].ucSystemID,
                      pPsshAtom->m_ucSystemID,
                      MAX_SYSTEMID_SIZE);
              pPSSHInfo[i].ulKIDCount     = pPsshAtom->m_ulKidCount;
              pPSSHInfo[i].ulKIDDataSize  = pPsshAtom->m_ulKidDataSize;
              pPSSHInfo[i].ulPSSHDataSize = pPsshAtom->m_ulPsshDataSize;

              //! Update the drmType if System ID matches with Marlin SystemID.
              if( !memcmp(pPSSHInfo[i].ucSystemID,
                          MARLIN_DRM_SYSTEMID,
                          MAX_SYSTEMID_SIZE) )
              {
                pPSSHInfo[i].ePSDRMType = FILE_SOURCE_MARLIN_DRM;
                //! In case of Marlin DRM,PSSH Data Size will be size of complete
                //! PSSH Atom and data Offset will be start offset of PSSH Atom.
                pPSSHInfo[i].ulPSSHDataSize = pPsshAtom->m_ulPsshDataBufferSize;
              }
              else if( !memcmp(pPSSHInfo[i].ucSystemID,
                               PLAYREADY_DRM_SYSTEMID,
                               MAX_SYSTEMID_SIZE) )
              {
                pPSSHInfo[i].ePSDRMType = FILE_SOURCE_PLAYREADY_DRM;
                //! In case of Marlin DRM,PSSH Data Size will be size of complete
                //! PSSH Atom and data Offset will be start offset of PSSH Atom.
                pPSSHInfo[i].ulPSSHDataSize = pPsshAtom->m_ulPsshDataBufferSize;
              }
            }
          }//for
          eError = PARSER_ErrorNone;
        }//case
        break;
      case FS_IndexParamOtherPSSHData:
        {
          FS_PROTECTION_SYSTEM_SPECIFIC_DATATYPE *pPSSHData =
            (FS_PROTECTION_SYSTEM_SPECIFIC_DATATYPE*)pParamStruct;

          if( pPSSHData->ulDRMIndex >= m_ulPSSHatomEntryCount)
          {
            return PARSER_ErrorInvalidParam;
          }
          CPsshAtom *pPsshAtom = NULL;
          CSinfAtom *pSinfAtom = NULL;
          m_ulSelectedDRMIdx = pPSSHData->ulDRMIndex;

          if(m_aPSSHatomEntryArray.GetLength() > pPSSHData->ulDRMIndex)
          {
            pPsshAtom = m_aPSSHatomEntryArray.ElementAt(pPSSHData->ulDRMIndex);
          }
          if(m_aSINFatomEntryArray.GetLength() > pPSSHData->ulDRMIndex)
          {
            pSinfAtom = m_aSINFatomEntryArray.ElementAt(pPSSHData->ulDRMIndex);
          }

          //! Copy default KID from SINF/TENC.
          if(pSinfAtom && pSinfAtom->m_pTencAtom)
          {
            memcpy ((uint8 *) &pPSSHData->ucDefaultKeyID,
                    pSinfAtom->m_pTencAtom->m_ucKeyID,
                    MAX_KID_SIZE);
          }

          if(pPsshAtom)
          {
            if((!memcmp(pPsshAtom->m_ucSystemID, MARLIN_DRM_SYSTEMID,
                        MAX_SYSTEMID_SIZE) ) ||
               (!memcmp(pPsshAtom->m_ucSystemID, PLAYREADY_DRM_SYSTEMID,
                        MAX_SYSTEMID_SIZE) ) )
            {
              //! In case of Marlin DRM,PSSH Data Buf Size will be size of complete
              //! PSSH Atom and Data buffer will have complete PSSH Atom.
              pPSSHData->ulPSSHDataBufSize = pPsshAtom->m_ulPsshDataBufferSize;
              (void)readFile (localParseFilePtr, pPSSHData->pucPSSHDataBuf,
                              pPsshAtom->m_ulPsshDataBufferOffset ,
                              pPsshAtom->m_ulPsshDataBufferSize);
            }
            else
            {
              pPSSHData->ulPSSHDataBufSize = pPsshAtom->m_ulPsshDataSize;
              //! Read 'PSSH' atom data from the offset.
              (void)readFile (localParseFilePtr, pPSSHData->pucPSSHDataBuf,
                              pPsshAtom->m_ulPsshDataOffset ,
                              pPsshAtom->m_ulPsshDataSize);
            }

            if(0 == pPsshAtom->m_ulKidCount)
            {
              pPSSHData->ulKIDDataBufSize= 0;
              memset(pPSSHData->pucKIDDataBuf,0,pPsshAtom->m_ulKidDataSize);
            }
            else
            {
              pPSSHData->ulKIDDataBufSize= pPsshAtom->m_ulKidDataSize;
              (void)readFile (localParseFilePtr, pPSSHData->pucKIDDataBuf,
                              pPsshAtom->m_ulKidDataOffset ,
                              pPsshAtom->m_ulKidDataSize);
            }
          }
          eError = PARSER_ErrorNone;
        }//case
        break;

      case FS_IndexParamAudioAC3:
        {
          uint8 ucIdx = 0;
          FS_AUDIO_PARAM_AC3TYPE *pAC3Param = \
            (FS_AUDIO_PARAM_AC3TYPE *)pParamStruct;
          pAC3Param->ulSamplingRate = \
            pTrackInfo->subinfo.audio.sampling_frequency;
          pAC3Param->ulBitRate = \
            pTrackInfo->subinfo.audio.dd_params.usBitRate;
          pAC3Param->ucNumOfIndSubs = \
            pTrackInfo->subinfo.audio.dd_params.ucNumIndSubs;
          pAC3Param->ucEc3ExtTypeA = \
            pTrackInfo->subinfo.audio.dd_params.ucEC3ExtTypeA;
          if(pAC3Param->ucProgramID)
          {
            // ProgramID is base 1 i.e. 1,2, 8 etc.
            ucIdx = pAC3Param->ucProgramID - 1;
            pAC3Param->ucAudioCodingMode = \
              pTrackInfo->subinfo.audio.dd_params.sBSIInfo[ucIdx].ucAcmod;
            pAC3Param->ucBitStreamMode = \
              pTrackInfo->subinfo.audio.dd_params.sBSIInfo[ucIdx].ucBsmod;
            pAC3Param->usChannelLocation = \
              pTrackInfo->subinfo.audio.dd_params.sBSIInfo[ucIdx].usChLocation;
            pAC3Param->usNumChannels = \
              pTrackInfo->subinfo.audio.dd_params.sBSIInfo[ucIdx].ucNumChannels;
          }
          eError = PARSER_ErrorNone;
        }
        break;//case FS_IndexParamAudioAC3

      case FS_IndexParamAudioDTS:
        {
          FS_AUDIO_PARAM_DTSTYPE *pDTSParam = \
            (FS_AUDIO_PARAM_DTSTYPE *)pParamStruct;
          pDTSParam->usNumChannels = \
            pTrackInfo->subinfo.audio.num_channels;
          pDTSParam->ulSamplingRate = \
            pTrackInfo->subinfo.audio.sampling_frequency;
          pDTSParam->ulBitRate = \
            pTrackInfo->subinfo.audio.dts_params.ulAvgBitRate;
          if( VIDEO_FMT_STREAM_AUDIO_DTS_CORE == \
              pTrackInfo->subinfo.audio.format )
          {
            pDTSParam->eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_LEGACY;
          }
          else if(VIDEO_FMT_STREAM_AUDIO_DTS_HD == \
                  pTrackInfo->subinfo.audio.format)
          {
            pDTSParam->eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_HD;
          }
          else if(VIDEO_FMT_STREAM_AUDIO_DTS_HD_LOSSLESS == \
                  pTrackInfo->subinfo.audio.format)
          {
            pDTSParam->eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_XLL;
          }
          else if(VIDEO_FMT_STREAM_AUDIO_DTS_LBR == \
                  pTrackInfo->subinfo.audio.format)
          {
            pDTSParam->eSubType = FILE_SOURCE_SUB_MN_TYPE_DTS_LBR;
          }
          eError = PARSER_ErrorNone;
        }
        break;//case FS_IndexParamAudioDTS

      default:
        {
          eError = PARSER_ErrorNotImplemented;
        }
        break;
      }//switch(ulParamIndex)
    }//if(p_track && VIDEO_FMT_STREAM_VIDEO)
  }// if(pParamStruct)
  return eError;
}

/* ======================================================================
FUNCTION
Mpeg4File::getNumPicParamSet

DESCRIPTION
returns number of H264 picture parameter sets in the clip.

DEPENDENCIES
List any dependencies for this function, global variables, state,
resource availability, etc.

RETURN VALUE
number of H264 picture parameter sets.

SIDE EFFECTS
Detail any side effects.

========================================================================== */
uint32 Mpeg4File::getNumPicParamSet(uint32 trackId)
{
  uint32 numParam = 0;
  video_fmt_stream_info_type *pInfo = getTrackInfoForID(trackId);

  if(pInfo && (pInfo->type==VIDEO_FMT_STREAM_VIDEO) && (pInfo->subinfo.video.format==VIDEO_FMT_STREAM_VIDEO_H264) )
  {
    //if number of views greater then 1 then update MVCC picture param number else AVCC picture param number
    if( pInfo->dec_specific_info.h264_info.vwid_info &&
       (pInfo->dec_specific_info.h264_info.vwid_info->num_views > 1 ))
    {
      numParam = pInfo->dec_specific_info.h264_info.mvcc_info->num_pic_param;
    }
    else
    {
      numParam = pInfo->dec_specific_info.h264_info.avcc_info->num_pic_param;
    }
  }
  return numParam;
}

/* ======================================================================
FUNCTION
Mpeg4File::getNumSeqParamSet

DESCRIPTION
returns number of H264 sequence parameter sets in the clip.

DEPENDENCIES
List any dependencies for this function, global variables, state,
resource availability, etc.

RETURN VALUE
number of H264 picture parameter sets.

SIDE EFFECTS
Detail any side effects.

========================================================================== */
uint32 Mpeg4File::getNumSeqParamSet(uint32 trackId)
{
  uint32 numParam = 0;
  video_fmt_stream_info_type *pInfo = getTrackInfoForID(trackId);

  if(pInfo && (pInfo->type==VIDEO_FMT_STREAM_VIDEO) && (pInfo->subinfo.video.format==VIDEO_FMT_STREAM_VIDEO_H264) )
  {
    if( pInfo->dec_specific_info.h264_info.vwid_info &&
      (pInfo->dec_specific_info.h264_info.vwid_info->num_views > 1 ))
    {
      numParam = pInfo->dec_specific_info.h264_info.mvcc_info->num_seq_param;
    }
    else
    {
      numParam = pInfo->dec_specific_info.h264_info.avcc_info->num_seq_param;
    }
  }
  return numParam;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getNextParamSetNAL

DESCRIPTION
Public method used to request next parameter set.

DESCRIPTION
copies pic parameter set NAL in given buffer.

PARAMETERS:
idx     INPUT   index of video track.
buf     OUTPUT  buffer for data to be copied.
size    INPUT   size of buffer.

DEPENDENCIES
First it will return all Sequence parameters, then picture parameters.

RETURN VALUE
size of parameter set NAL
zero if no more parameters to be sent.

SIDE EFFECTS
First it will return all Sequence parameters, then picture parameters.
ASSERTS if buffer size is smaller than size of NAL.

===========================================================================*/
int32 Mpeg4File::getNextParamSetNAL(uint32 trackId, uint8 *buf, uint32 size)
{
  int32  nBytes = 0;
  video_fmt_stream_info_type *pInfo = getTrackInfoForID(trackId);

  if( pInfo &&
     ( VIDEO_FMT_STREAM_VIDEO == pInfo->type ) &&
     ( VIDEO_FMT_STREAM_VIDEO_H264 == pInfo->subinfo.video.format ) )
  {
    video_fmt_h264_dec_info_type *h264Info = &pInfo->dec_specific_info.h264_info;
    /* Get sequence parameter set & picture parameter set of both MVC & AVC configuration.
     * SPS/PPS sets will be send in following way
     * <sc><SPS(MVC)V-0><sc><SPS(MVC)V-1><sc><PPS(MVC)V-0><sc><PPS(MVC)V-1><sc><SPS(AVC)><sc><PPS(AVC)>
     */
    /* First send all sequence parameters(SPS)for MVC Configuration */
    if(h264Info && h264Info->mvcc_info && (m_nextMVCSeqSample < h264Info->mvcc_info->num_seq_param))
    {
      nBytes = h264Info->mvcc_info->seq_param_set[m_nextMVCSeqSample].len;

      if (!(nBytes < (int32)size))
      {
        nBytes = FRAGMENT_CORRUPT;
        return nBytes;
      }
      memcpy(buf, h264Info->mvcc_info->seq_param_set[m_nextMVCSeqSample].data, (uint32)nBytes);
      m_nextMVCSeqSample++;
      return nBytes ;
    }
    /* Send all picture parameters(PPS) for MVC Configuration*/
    else if( h264Info && h264Info->mvcc_info && (m_nextMVCPicSample < h264Info->mvcc_info->num_pic_param))
    {
      nBytes = h264Info->mvcc_info->pic_param_set[m_nextMVCPicSample].len;

      if (!(nBytes < (int32)size))
      {
        nBytes = FRAGMENT_CORRUPT;
        return nBytes;
      }
      memcpy(buf, h264Info->mvcc_info->pic_param_set[m_nextMVCPicSample].data, (uint32)nBytes);
      m_nextMVCPicSample++;
      return nBytes;
    }
    /* Second send all sequence parameters(SPS)for AVC Configuration */
    else if( h264Info && h264Info->avcc_info && (m_nextAVCSeqSample < h264Info->avcc_info->num_seq_param))
    {
      nBytes = h264Info->avcc_info->seq_param_set[m_nextAVCSeqSample].len;

      if (!(nBytes < (int32)size))
      {
        nBytes = FRAGMENT_CORRUPT;
        return nBytes;
      }
      memcpy(buf, h264Info->avcc_info->seq_param_set[m_nextAVCSeqSample].data, (uint32)nBytes);
      m_nextAVCSeqSample++;
      return nBytes ;
    }
    /* Send all picture parameters(PPS) for AVC Configuration*/
    else if( h264Info && h264Info->avcc_info && (m_nextAVCPicSample < h264Info->avcc_info->num_pic_param))
    {
      nBytes = h264Info->avcc_info->pic_param_set[m_nextAVCPicSample].len;

      if (!(nBytes < (int32)size))
      {
        nBytes = FRAGMENT_CORRUPT;
        return nBytes;
      }
      memcpy(buf, h264Info->avcc_info->pic_param_set[m_nextAVCPicSample].data, (uint32)nBytes);
      m_nextAVCPicSample++;
      return nBytes;
    }
  }
  return nBytes;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  resetParamSetNAL

DESCRIPTION
Public method used to reset parameter set counters.

PARAMETERS:
idx     INPUT   index of video track.

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/
void Mpeg4File::resetParamSetNAL(uint32)
{
  m_nextAVCSeqSample = 0;
  m_nextAVCPicSample = 0;
  m_nextMVCSeqSample = 0;
  m_nextMVCPicSample = 0;
}

/* <EJECT> */
/*===========================================================================

FUNCTION  GetSizeOfNALLengthField

DESCRIPTION
Public method used to obtain the size of NAL length field before NAL in
video frames.

PARAMETERS:
idx     INPUT   index of video track.

DEPENDENCIES

RETURN VALUE
size of NAL length field before NAL in video frames in bytes.

SIDE EFFECTS

===========================================================================*/
int Mpeg4File::GetSizeOfNALLengthField(uint32 trackId)
{
  int32  nBytes = 0;
  video_fmt_stream_info_type *pInfo = getTrackInfoForID(trackId);

  if(pInfo && (pInfo->type==VIDEO_FMT_STREAM_VIDEO) && (pInfo->subinfo.video.format==VIDEO_FMT_STREAM_VIDEO_H264) )
  {
    if( pInfo->dec_specific_info.h264_info.vwid_info &&
       (pInfo->dec_specific_info.h264_info.vwid_info->num_views > 1 ))
    {
      nBytes = pInfo->dec_specific_info.h264_info.mvcc_info->len_minus_one + 1;
    }
    else
    {
      nBytes = pInfo->dec_specific_info.h264_info.avcc_info->len_minus_one + 1;
    }
  }
  return nBytes;
}
#ifdef FEATURE_MP4_CUSTOM_META_DATA
/* ======================================================================
FUNCTION:
Mpeg4File::IsMidiDataPresent

DESCRIPTION:
if SKT specific MIDI data is present or not.

INPUT/OUTPUT PARAMETERS:
None.

RETURN VALUE:
TRUE - if MIDI data is present
FALSE - MIDI data not present.

SIDE EFFECTS:
None.
======================================================================*/
bool Mpeg4File::IsMidiDataPresent()
{
  if ( _midiAtom )
  {
    return true;
  }
  return false;
}

/* ======================================================================
FUNCTION:
Mpeg4File::GetMidiDataSize

DESCRIPTION:
returns MIDI data size.

INPUT/OUTPUT PARAMETERS:
None.

RETURN VALUE:
size of MIDI data
0 if MIDI not present.

SIDE EFFECTS:
None.
======================================================================*/
uint32 Mpeg4File::GetMidiDataSize()
{
  if(_midiAtom)
  {
    return _midiAtom->getUdtaMidiDataSize();
  }
  return 0;
}

/* ======================================================================
FUNCTION:
Mpeg4File::GetMidiData

DESCRIPTION:
copies the MIDI data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
pBuf  - INPUT/OUTPUT  - buffer for data to be copied.
size  - INPUT         - size of buffer and max data to be copied.
offset- INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
actual bytes copied into buffer
0 if no MIDI data is available

SIDE EFFECTS:
None.
======================================================================*/
uint32 Mpeg4File::GetMidiData(uint8 *pBuf, uint32 size, uint32 offset)
{
  if(_midiAtom)
  {
    return _midiAtom->getUdtaMidiData(pBuf, size, offset);
  }
  return 0;
}

/* ======================================================================
FUNCTION:
Mpeg4File::IsLinkDataPresent

DESCRIPTION:
if SKT specific LINK atom is present or not.

INPUT/OUTPUT PARAMETERS:
None.

RETURN VALUE:
TRUE - if LINK atom is present
FALSE - LINK atom not present.

SIDE EFFECTS:
None.
======================================================================*/
bool Mpeg4File::IsLinkDataPresent()
{
  if ( _linkAtom )
  {
    return true;
  }
  return false;
}

/* ======================================================================
FUNCTION:
Mpeg4File::GetLinkDataSize

DESCRIPTION:
returns LINK size.

INPUT/OUTPUT PARAMETERS:
None.

RETURN VALUE:
size of LINK
0 if LINK not present.

SIDE EFFECTS:
None.
======================================================================*/
uint32 Mpeg4File::GetLinkDataSize()
{
  if(_linkAtom)
  {
    return _linkAtom->getUdtaLinkDataSize();
  }
  return 0;
}

/* ======================================================================
FUNCTION:
Mpeg4File::GetLinkData

DESCRIPTION:
copies the LINK into supplied buffer.

INPUT/OUTPUT PARAMETERS:
pBuf  - INPUT/OUTPUT  - buffer for data to be copied.
size  - INPUT         - size of buffer and max data to be copied.

RETURN VALUE:
actual bytes copied into buffer
0 if no LINK data is available

SIDE EFFECTS:
None.
======================================================================*/
uint32 Mpeg4File::GetLinkData(uint8 *pBuf, uint32 size)
{
  if(_linkAtom)
  {
    return _linkAtom->getUdtaLinkData(pBuf, size);
  }
  return 0;
}

/* ======================================================================
FUNCTION:
Mpeg4File::IsDataPresent

DESCRIPTION:
Checks if the specified data-type data is present or not.

INPUT/OUTPUT PARAMETERS:
None.

RETURN VALUE:
TRUE - if specified data-type data is present
FALSE - if specified data-type data not present.

SIDE EFFECTS:
None.
======================================================================*/
bool Mpeg4File::IsDataPresent(DataT dType, uint32 track_id)
{
  track_id = track_id; /* This is to remove the comiler warning */
  switch(dType)
  {
  case DATA_ATOM_FTYP:       /* File Type Atom*/
    if(_ftypAtom)
    {
      return true;
    }
    return false;
  case DATA_ATOM_UDTA_CPRT:        /* Copyright Atom*/
    if ( _cprtAtom )
    {
      return true;
    }
    return false;
  case DATA_ATOM_UDTA_AUTH:  /* Author Atom*/
    if ( _authAtom )
    {
      return true;
    }
    return false;
  case DATA_ATOM_UDTA_TITL:    /* Title Atom*/
    if ( _titlAtom )
    {
      return true;
    }
    return false;
  case DATA_ATOM_UDTA_DSCP:  /* Description Atom*/
    if ( _dscpAtom )
    {
      return true;
    }
    return false;
  case DATA_ATOM_DCMD:      /* DCMD DRM Atom*/
    if ( _dcmdAtom )
    {
      return true;
    }
    return false;
  case DATA_ATOM_UDTA_RTNG:  /* Rating Atom*/
  if ( _rtngAtom )
  {
   return true;
  }
  return false;
  case DATA_ATOM_UDTA_PERF:  /* Performance Atom*/
  if ( _perfAtom )
  {
    return true;
  }
  return false;
  case DATA_ATOM_UDTA_CLSF:  /* Classification Atom*/
  if ( _clsfAtom )
  {
    return true;
  }
  return false;
  case DATA_ATOM_UDTA_KYWD:  /* Keyword Atom*/
  if ( _kywdAtom )
  {
    return true;
  }
  return false;
  case DATA_ATOM_UDTA_LOCI:  /* Location Atom*/
  if ( _lociAtom )
  {
    return true;
  }
  return false;

  case DATA_ATOM_UDTA_META:  /* Meta Atom*/
  if ( _metaAtom )
  {
    return true;
  }
  return false;

  case DATA_ATOM_UDTA_MIDI:
    if ( _midiAtom )
    {
      return true;
    }
    return false;
  case DATA_ATOM_UDTA_LINK:
    if ( _linkAtom )
    {
      return true;
    }
    return false;

  case DATA_ATOM_UDTA_ALBUM:  /* Album Atom*/
  if ( _albumAtom )
  {
    return true;
  }
  return false;
  case DATA_TEXT_TKHD_ORIGIN_X:    /* Text track origin_x*/
  case DATA_TEXT_TKHD_ORIGIN_Y:    /* Text track origin_y*/
  case DATA_TEXT_TKHD_WIDTH:       /* Text track width*/
  case DATA_TEXT_TKHD_HEIGHT:       /* Text track height*/
    /*Check if the text track is present*/
    for (uint32 index = 0; index < m_trackCount; index++)
    {
      if ( m_track[index]->type == VIDEO_FMT_STREAM_TEXT )
      {
        return true;
      }
    }
    return false;
  case DATA_ATOM_SCHM:
  case DATA_ATOM_FRMA:
  case DATA_ATOM_OHDR:
  case DATA_ATOM_ODAF:
  case DATA_ATOM_MDRI:
  case DATA_ATOM_SCHM_OFFSET:
  case DATA_ATOM_FRMA_OFFSET:
  case DATA_ATOM_OHDR_OFFSET:
  case DATA_ATOM_ODAF_OFFSET:
  case DATA_ATOM_MDRI_OFFSET:
    if(_pdcfAtom)
    {
      return _pdcfAtom->isAtomPresent(dType, track_id);
    }
    break;
  default:
    break;
  }
  return false;
}
#endif /* FEATURE_MP4_CUSTOM_META_DATA */

/* ======================================================================
FUNCTION:
Mpeg4File::GetDataSize

DESCRIPTION:
returns the specified data-type data size.

INPUT/OUTPUT PARAMETERS:
dType - data-type.
offset - used as a track_id if the data is in multiple tracks.

RETURN VALUE:
size of the specified data-type.
0 if specified data not present.

SIDE EFFECTS:
None.
======================================================================*/
uint32 Mpeg4File::GetDataSize(DataT dType, uint32 offset )
{
  offset = offset; //required to remove compile time warning
  uint32 ulTrackID = 0;
  TextSampleEntry *textAtom = NULL;
  switch(dType)
  {
  case DATA_ATOM_UDTA_CPRT:        /* Copyright Atom*/
    if(_cprtAtom)
    {
      return _cprtAtom->getUdtaCprtDataSize();
    }
    return 0;
  case DATA_ATOM_UDTA_AUTH:  /* Author Atom*/
    if(_authAtom)
    {
      return _authAtom->getUdtaAuthDataSize();
    }
    return 0;
  case DATA_ATOM_UDTA_TITL:    /* Title Atom*/
    if(_titlAtom)
    {
      return _titlAtom->getUdtaTitlDataSize();
    }
    return 0;
  case DATA_ATOM_UDTA_DSCP:  /* Description Atom*/
    if(_dscpAtom)
    {
      return _dscpAtom->getUdtaDscpDataSize();
    }
    return 0;

  case DATA_ATOM_UDTA_RTNG:      /* Rating*/
    if(_rtngAtom)
    {
        return _rtngAtom->getUdtaRtngDataSize();
    }
    return 0;
  case DATA_ATOM_UDTA_PERF:      /* Performance Atom*/
    if(_perfAtom)
    {
        return _perfAtom->getUdtaPerfDataSize();
    }
    return 0;
  case DATA_ATOM_UDTA_KYWD:      /* Keyword Atom*/
    if(_kywdAtom)
    {
        return _kywdAtom->getUdtaKywdDataSize();
    }
    return 0;
  case DATA_ATOM_UDTA_CLSF:      /* Classification Atom*/
    if(_clsfAtom)
    {
        return _clsfAtom->getUdtaClsfDataSize();
    }
    return 0;
  case DATA_ATOM_UDTA_LOCI:      /* Location Atom*/
    if(_lociAtom)
    {
        return _lociAtom->getUdtaLociDataSize();
    }
    return 0;
  case DATA_ATOM_UDTA_YEAR:
    {
      if (pYrccAtom)
      {
        return pYrccAtom->getUdtaYrrcDataSize();
      }
      return 0;
    }
  case DATA_ATOM_UDTA_ALBUM:      /* Location Atom*/
    if(_albumAtom)
    {
        return _albumAtom->getUdtaAlbumDataSize();
    }
    return 0;

  case DATA_ATOM_UDTA_META:
    if(_metaAtom)
    {
        return _metaAtom->getUdtaMetaDataSize();
    }
    return 0;

  case DATA_TEXT_TKHD_ORIGIN_X:    /* Text track origin_x*/
  case DATA_TEXT_TKHD_ORIGIN_Y:    /* Text track origin_y*/
  case DATA_TEXT_TKHD_WIDTH:       /* Text track width*/
  case DATA_TEXT_TKHD_HEIGHT:       /* Text track height*/
    for (uint32 index = 0; index < m_trackCount; index++)
    {
      if ( m_track[index]->type == VIDEO_FMT_STREAM_TEXT )
      {
        return FOURCC_SIGNATURE_BYTES;
      }
    }
    return 0;

  case DATA_ATOM_TX3G:
  case DATA_ATOM_STPP:
    for (uint32 ulIndex = 0; ulIndex < m_trackCount; ulIndex++)
    {
      if ( m_track[ulIndex]->type == VIDEO_FMT_STREAM_TEXT )
        ulTrackID = m_track[ulIndex]->track_id;
    }
    textAtom = this->getTextSampleEntryAt(ulTrackID, (offset+1));
    if(textAtom)
    {
      return textAtom->getSize();
    }
    return 0;

  case DATA_ATOM_SUBS:
    if(pSubsAtom)
    {
      return pSubsAtom->GetSubsDataSize();
    }
    return 0;
#ifdef FEATURE_MP4_CUSTOM_META_DATA
  case DATA_ATOM_FTYP:       /* File Type Atom*/
    if(_ftypAtom)
    {
      return _ftypAtom->getFtypDataSize();
    }
    return 0;
  case DATA_ATOM_UDTA_MIDI:
    if(_midiAtom)
    {
      return _midiAtom->getUdtaMidiDataSize();
    }
    return 0;
  case DATA_ATOM_UDTA_LINK:
    if(_linkAtom)
    {
      return _linkAtom->getUdtaLinkDataSize();
    }
    return 0;
  case DATA_ATOM_DCMD:      /* DCMD DRM Atom*/
    if(_dcmdAtom)
    {
      return _dcmdAtom->getDcmdDataSize();
    }
    return 0;
  case DATA_ATOM_OHDR:
  case DATA_ATOM_ODAF:
  case DATA_ATOM_MDRI:
    if(_pdcfAtom)
    {
      return _pdcfAtom->getAtomSize(dType, offset);
    }
    break;
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
  default:
    break;
  }
  return 0;
}

/* ======================================================================
FUNCTION:
Mpeg4File::GetData

DESCRIPTION:
copies the specified data-type data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
dType - data-type.
pBuf  - INPUT/OUTPUT  - buffer for data to be copied.
size  - INPUT         - size of buffer and max data to be copied.
offset- INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
actual bytes copied into buffer
0 if no data is available

SIDE EFFECTS:
None.
======================================================================*/
uint32 Mpeg4File::GetData(DataT dType, uint8 *pBuf, uint32 size, uint32 offset)
{
  switch(dType)
  {
  case DATA_ATOM_UDTA_CPRT:        /* Copyright Atom*/
    if(_cprtAtom)
    {
      return _cprtAtom->getUdtaCprtData(pBuf, size, offset);
    }
    return 0;
  case DATA_ATOM_UDTA_AUTH:  /* Author Atom*/
    if(_authAtom)
    {
      return _authAtom->getUdtaAuthData(pBuf, size, offset);
    }
    return 0;
  case DATA_ATOM_UDTA_TITL:    /* Title Atom*/
    if(_titlAtom)
    {
      return _titlAtom->getUdtaTitlData(pBuf, size, offset);
    }
    return 0;
  case DATA_ATOM_UDTA_DSCP:  /* Description Atom*/
    if(_dscpAtom)
    {
      return _dscpAtom->getUdtaDscpData(pBuf, size, offset);
    }
    return 0;

  case DATA_ATOM_UDTA_RTNG:  /* Rating Atom*/
    if(_rtngAtom)
    {
        return _rtngAtom->getUdtaRtngData(pBuf, size, offset);
    }
    return 0;
  case DATA_ATOM_UDTA_PERF:  /* Performance Atom*/
    if(_perfAtom)
    {
        return _perfAtom->getUdtaPerfData(pBuf, size, offset);
    }
    return 0;
  case DATA_ATOM_UDTA_CLSF:  /* Classification Atom*/
    if(_clsfAtom)
    {
        return _clsfAtom->getUdtaClsfData(pBuf, size, offset);
    }
    return 0;
  case DATA_ATOM_UDTA_KYWD:  /* Keyword Atom*/
    if(_kywdAtom)
    {
        return _kywdAtom->getUdtaKywdData(pBuf, size, offset);
    }
    return 0;
  case DATA_ATOM_UDTA_LOCI:  /* Location Atom*/
    if(_lociAtom)
    {
        return _lociAtom->getUdtaLociData(pBuf, size, offset);
    }
    return 0;
  case DATA_ATOM_UDTA_YEAR:
    if(pYrccAtom)
    {
      return pYrccAtom->getUdtaYrrcData(pBuf, size, offset);
    }
    break;
  case DATA_ATOM_UDTA_ALBUM:  /* Album Atom*/
    if(_albumAtom)
    {
        return _albumAtom->getUdtaAlbumData(pBuf, size, offset);
    }
    return 0;
  case DATA_ATOM_UDTA_META:  /* Meta Atom*/
    if(_metaAtom)
    {
        return _metaAtom->getUdtaMetaData(pBuf, size, offset);
    }
    return 0;
  case DATA_TEXT_TKHD_ORIGIN_X:    /* Text track origin_x*/
    if(size >= FOURCC_SIGNATURE_BYTES)
    {
      for (uint32 index = 0; index < m_trackCount; index++)
      {
        if ( m_track[index]->type == VIDEO_FMT_STREAM_TEXT )
        {
          *((uint32 *)pBuf) = m_track[index]->tkhd_origin_x >> 16;
          return FOURCC_SIGNATURE_BYTES;
        }
      }
    }
    return 0;

  case DATA_TEXT_TKHD_ORIGIN_Y:    /* Text track origin_y*/
    if(size >= FOURCC_SIGNATURE_BYTES)
    {
      for (uint32 index = 0; index < m_trackCount; index++)
      {
        if ( m_track[index]->type == VIDEO_FMT_STREAM_TEXT )
        {
          *((uint32 *)pBuf) = m_track[index]->tkhd_origin_y >> 16;
          return FOURCC_SIGNATURE_BYTES;
        }
      }
    }
    return 0;

  case DATA_TEXT_TKHD_WIDTH:       /* Text track width*/
    if(size >= FOURCC_SIGNATURE_BYTES)
    {
      for (uint32 index = 0; index < m_trackCount; index++)
      {
        if ( m_track[index]->type == VIDEO_FMT_STREAM_TEXT )
        {
          *((uint32 *)pBuf) = m_track[index]->tkhd_width >> 16;
          return FOURCC_SIGNATURE_BYTES;
        }
      }
    }
    return 0;

  case DATA_TEXT_TKHD_HEIGHT:       /* Text track height*/
    if(size >= FOURCC_SIGNATURE_BYTES)
    {
      for (uint32 index = 0; index < m_trackCount; index++)
      {
        if ( m_track[index]->type == VIDEO_FMT_STREAM_TEXT )
        {
          *((uint32 *)pBuf) = m_track[index]->tkhd_height >> 16;
          return FOURCC_SIGNATURE_BYTES;
        }
      }
    }
    return 0;

case DATA_ATOM_UUID:
  {
    if(m_parseFilePtr)
    {
      uint64 saveFilePos = OSCL_FileTell (m_parseFilePtr);
      uint32 copiedSize = 0;
      Atom *UUIDatom = NULL;
      uint32 length = this->UUIDatomEntryArray.GetLength();
      if( length >= offset )
        UUIDatom = UUIDatomEntryArray[offset];
      else
        UUIDatom = NULL;

      if (UUIDatom != NULL)
      {
        (void)OSCL_FileSeek(m_parseFilePtr, UUIDatom->getOffsetInFile(), SEEK_SET);
        uint32 uuidatomSize = UUIDatom->getSize();
        copiedSize = FILESOURCE_MIN(size, uuidatomSize);
        if ( OSCL_FileRead (pBuf, copiedSize,
                          1,  m_parseFilePtr) == 0 )
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Failed to readFile");
          copiedSize = 0;
        }
      }
      (void)OSCL_FileSeek(m_parseFilePtr, saveFilePos, SEEK_SET);
      return copiedSize;
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_ERROR,"Failed to find open file pointer");
    }
    break;
  }

  case DATA_ATOM_TX3G:
  case DATA_ATOM_STPP:
  {
    if(m_parseFilePtr)
    {
      uint64 saveFilePos = OSCL_FileTell (m_parseFilePtr);
      uint32 trackid = 0;
      for (uint32 index = 0; index < m_trackCount; index++)
      {
        if ( m_track[index]->type == VIDEO_FMT_STREAM_TEXT )
          trackid = m_track[index]->track_id;
      }
      TextSampleEntry *textAtom = this->getTextSampleEntryAt(trackid, (offset+1));
      uint32 ulReadDataSize = 0;
      if (textAtom != NULL)
      {
        uint64 ullAtomOffset  = textAtom->getOffsetInFile();
        uint32 ulTextAtomSize = textAtom->getSize();
        /* In LA, framework is expecting complete TX3G atom including header.
           Where as in other frameworks, Parser has to remove Atom header.*/
#ifndef _ANDROID_
        ullAtomOffset  += DEFAULT_TEXT_ATOM_HEADER_SIZE;
        ulTextAtomSize -= DEFAULT_TEXT_ATOM_HEADER_SIZE;
#endif
        (void)OSCL_FileSeek(m_parseFilePtr, ullAtomOffset, SEEK_SET);
        ulReadDataSize = FILESOURCE_MIN(size, ulTextAtomSize);
        if ( OSCL_FileRead (pBuf, ulReadDataSize,
                            1,  m_parseFilePtr) == 0 )
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Failed to readFile");
          ulReadDataSize = 0;
        }
      }
      (void)OSCL_FileSeek(m_parseFilePtr, saveFilePos, SEEK_SET);
      return ulReadDataSize;
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_ERROR,"Failed to find open file pointer");
    }
    break;
  }

  case DATA_ATOM_SUBS:
    if(pSubsAtom)
    {
      return pSubsAtom->GetSubsData(pBuf, size,offset);
    }
    return 0;
#ifdef FEATURE_MP4_CUSTOM_META_DATA
  case DATA_ATOM_FTYP:       /* File Type Atom*/
    if(_ftypAtom)
    {
      return _ftypAtom->getFtypData(pBuf, size, offset);
    }
    return 0;
  case DATA_ATOM_UDTA_MIDI:
    if(_midiAtom)
    {
      return _midiAtom->getUdtaMidiData(pBuf, size, offset);
    }
    return 0;
  case DATA_ATOM_UDTA_LINK:
    if(_linkAtom)
    {
      return _linkAtom->getUdtaLinkData(pBuf, size);
    }
    return 0;
  case DATA_ATOM_DCMD:      /* DCMD DRM Atom*/
    if(_dcmdAtom)
    {
      return _dcmdAtom->getDcmdData(pBuf, size, offset);
    }
    return 0;
  case DATA_ATOM_OHDR:
  case DATA_ATOM_ODAF:
  case DATA_ATOM_MDRI:
    if(_pdcfAtom)
    {
      if( m_parseFilePtr == NULL )
      {
        m_parseFilePtr = OSCL_FileOpen(m_filename, (OSCL_TCHAR *) _T("rb"));
      }
      if(m_parseFilePtr)
      {
        uint32 copiedSize = 0;
        copiedSize = _pdcfAtom->getAtomData(m_parseFilePtr, dType, pBuf, size, offset);
        OSCL_FileClose(m_parseFilePtr);
        m_parseFilePtr = NULL;
        return copiedSize;
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Failed to open file");
      }
    }
    break;
  case DATA_ATOM_SCHM_OFFSET:
  case DATA_ATOM_FRMA_OFFSET:
  case DATA_ATOM_OHDR_OFFSET:
  case DATA_ATOM_ODAF_OFFSET:
  case DATA_ATOM_MDRI_OFFSET:
    if(_pdcfAtom)
    {
      return _pdcfAtom->getAtomOffset(dType,pBuf, size, offset);
    }
    break;
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
  default:
    break;
  }
  return 0;
}
#ifdef FEATURE_MP4_CUSTOM_META_DATA
void Mpeg4File::process_kddi_telop_text()
{
  if (_kddiTelopElement)
  {
    uint8* telopString = _kddiTelopElement->getTelopInformationString();

    uint32 telopStringSize = _kddiTelopElement->getTelopInformationSize();

    int32 telopParseResult = true;

    _pTsmlAtom = TsmlAtom::ParseTelopElementText( (char*)telopString,
      (int32)telopStringSize,
      (int32)getMovieDuration(),
      &telopParseResult);
    if (telopParseResult != SUCCESSFUL || _pTsmlAtom == NULL)
    {
      /* for telop parsing error, we only drop telop playback.
      other tracks (audio/video) can still play */
      if(_pTsmlAtom != NULL)
      {
        MM_Delete( _pTsmlAtom );
        _pTsmlAtom = NULL;
      }
      if (_kddiTelopElement != NULL)
      {
        MM_Delete( _kddiTelopElement );
        _kddiTelopElement = NULL;
      }
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Telop Atom Parsing Failed." );
    }
    else
    {
      _pTsmlAtom->ResetTelopVectorIndex();
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "_kddiTelopElement pointer invalid");
  }
}

void Mpeg4File::process_mod_midi_atom()
{
  if( _midiAtom && (_midiAtom->getUdtaMidiDataSize()==0) )
  {
    MM_Delete( _midiAtom );
    _midiAtom = NULL;
  }
  if( _linkAtom && (_linkAtom->getUdtaLinkDataSize()==0) )
  {
    MM_Delete( _linkAtom );
    _linkAtom = NULL;
  }
}
#endif
/*===========================================================================

FUNCTION  peekMetaDataSize

DESCRIPTION
Public method used to determine the meta-data size of the fragment.

===========================================================================*/
bool Mpeg4File::peekMetaDataSize (uint32 fragment_num)
{
  //If the buffer size is zero while streaming, no need to check videofmt.
  if(m_wBufferOffset==0 && bHttpStreaming &&
     ((m_videoFmtInfo.parse_fragment_cb == NULL) ||
      (m_videoFmtInfo.server_data == NULL)))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "Mpeg4File::peekMetaDataSize invalid parameters");
    return false;
  }
  MM_CriticalSection_Enter(videoFMT_Access_CS);

  if(m_bIsDashClip)
  {
    video_fmt_mp4r_context_type *context =
      (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
    m_nLastFragOffset = context->abs_size_retrieve_pos;
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "storing last fragment Offset value %llu", m_nLastFragOffset);
  }
  m_videoFmtInfo.fragment_size_peek_cb (m_videoFmtInfo.server_data,
    fragment_num);

  while ((m_mp4ParseLastStatus != VIDEO_FMT_FAILURE)
    && (m_mp4ParseLastStatus != VIDEO_FMT_DATA_INCOMPLETE)
    && (m_mp4ParseLastStatus != VIDEO_FMT_DATA_CORRUPT)
    && (m_mp4ParseLastStatus != VIDEO_FMT_INFO)
    && (m_mp4ParseLastStatus != VIDEO_FMT_FRAGMENT_PEEK))
  {
    if((m_mp4ParseContinueCb == NULL) ||
      (m_mp4ParseServerData == NULL))
      break;
    else
      m_mp4ParseContinueCb (m_mp4ParseServerData);
  }
  MM_CriticalSection_Leave(videoFMT_Access_CS);

  video_fmt_mp4r_context_type *ctxt= (video_fmt_mp4r_context_type *)m_videoFmtInfo.server_data;
  switch(m_mp4ParseLastStatus)
  {
  case VIDEO_FMT_FAILURE:
  case VIDEO_FMT_DATA_CORRUPT:
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
      "Mpeg4File::peekMetaDataSize fragment_size_peek_cb failed ");
    return false;

  case VIDEO_FMT_FRAGMENT_PEEK:
    m_minOffsetRequired = ctxt->fragment_size + ctxt->fragment_offset;
    //! Fragment number 0 means MOOV. MOOV atom is mandatory.
    //! If MOOV atom is not present, then report failure.
    if ((0 == fragment_num) && (fragment_num != ctxt->fragment_available))
    {
      bDataIncomplete = FALSE;
      m_bMOOVPresent  = false;
      return false;
    }

    /* Update MOOV atom buffer size with min offset required value.
       This value always points to end of either MOOV/MOOF atoms.
       First time, this will points to end of MOOV atom. Update buffer size
       with that value. */
    if (0 == m_ulMoovBufSize)
    {
      m_ullMoovAtomOffset = ctxt->fragment_offset;
      m_ulMoovBufSize     = (uint32)ctxt->fragment_size + DEFAULT_ATOM_SIZE;
      m_ulMoovBufSize     = (uint32)FILESOURCE_MIN((uint64)m_ulMoovBufSize,
                                                   m_fileSize);
    }

    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "Mpeg4File::peekMetaDataSize m_minOffsetRequired = %llu",
                 m_minOffsetRequired);
    bDataIncomplete = FALSE;
    return true;

  case VIDEO_FMT_DATA_INCOMPLETE:
    bDataIncomplete = TRUE;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                 "Mpeg4File::peekMetaDataSize underrun ");
    return false;

  case VIDEO_FMT_INFO:
    return true;

  default:
    break;
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "Mpeg4File::peekMetaDataSize m_minOffsetRequired = %llu",
               m_minOffsetRequired);
  return false;
}

/*===========================================================================

FUNCTION  ParseStream

DESCRIPTION
Public method called at the end of every fragment and at the beginning.This is
to be used irrespective of whether PD or PS is in use.

===========================================================================*/
bool Mpeg4File::ParseStream ()
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "current ParseFragment %lu",
               m_currentParseFragment);
  if (false == m_bMOOVPresent)
  {
    sendParserEvent(PARSER_RESUME);
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "Mpeg4File::ParseStream moov atom is not present");
    return false;
  }

  //First Make sure next fragment moov/moof is available.
  if(!peekMetaDataSize(m_currentParseFragment))
  {
    sendParserEvent(PARSER_RESUME);
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "Mpeg4File::ParseStream peekMetaDataSize failed");
    return false;
  }

  //Check for data present.
  if((m_wBufferOffset==0) || (m_wBufferOffset >= m_minOffsetRequired))
  {
    /* In DASH, fragments need not always in incremental order. There can be
       jumps as well, so we need to keep info as this is useful further. */
    if(m_bIsDashClip)
    {
      video_fmt_mp4r_context_type *context =
                  (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
      m_currentParseFragment = context->fragment_available;
    }
    m_currentParseFragment++;
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "ParseStream m_currentParseFragment %lu",
                 m_currentParseFragment);
    bool isParsingDone = parseMetaData();
    video_fmt_mp4r_context_type *context =
      (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
    if(TRUE == context->isDashClip)
    {
      m_bIsDashClip = TRUE;
    }

    //Then parse it and check for canPlayTracks().
    if(!isParsingDone)
    {
      //QTV_PS_PARSER_STATUS_PAUSED
      sendParserEvent(PARSER_RESUME);
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "Mpeg4File::ParseStream parseMetaData failed");
      return false;
    }
    else
    {
      m_bFragBoundary = true;
      if(!m_bIsDashClip || m_currentParseFragment > 1)
      {
        Parsed = TRUE;
      }
      if((parserState == PARSER_RESUME || parserState == PARSER_READY))
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
          "Mpeg4File::ParseStream Parser State = PARSER_READY");
        sendParserEvent(PARSER_READY);
        return true;
      }
      else
      {
        return true;
      }
    }
  }
  else
  {
    if(m_bIsDashClip)
    {
      video_fmt_mp4r_context_type *context =
              (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
      context->abs_size_retrieve_pos = m_nLastFragOffset;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "readjusting context offset to last fragment start %llu",
                   m_nLastFragOffset);
    }
    sendParserEvent(PARSER_RESUME);
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "Mpeg4File::ParseStream Parser State = PARSER_RESUME, \
                 m_wBufferOffset %llu",
                 m_wBufferOffset);
    return false;
  }
}
/* ======================================================================
FUNCTION:
  Mpeg4File::getBaseTime

DESCRIPTION:
  gets base time of the clip.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
bool Mpeg4File::getBaseTime(uint32 id, uint64* nBaseTime)
{
  video_fmt_mp4r_context_type *context =
    (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
  video_fmt_mp4r_stream_type  *stream  = 0;
  video_fmt_stream_info_type  *p_track = 0;

  //Fetch the track_info.
  p_track = getTrackInfoForID(id);
  if(NULL != p_track)
  {
    stream = &context->stream_state [p_track->stream_num];
  }

  if(NULL != stream && stream->fragment_boundary)
  {
    *nBaseTime = stream->cur_fragment_timestamp;
    //Don't update baseTime value in this API, update in setBaseTime API only.
    //m_baseTimeStamp[p_track->stream_num] = *nBaseTime;
    return true;
  }
  else
    return false;
}
/* ======================================================================
FUNCTION:
  Mpeg4File::setBaseTime

DESCRIPTION:
  sets base time of the clip.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
bool Mpeg4File::setBaseTime(uint32 id, uint64 nBaseTime)
{
  video_fmt_stream_info_type *p_track = 0;

  //Fetch the track_info.
  p_track = getTrackInfoForID(id);
  if(NULL != p_track)
  {
    m_baseTimeStamp[p_track->stream_num] = nBaseTime;
    return true;
  }
  return false;
}

/************************************************************************!
  @brief   Function to return the DRM protection status of content.

  @param[in,out]  None.

  @return     TRUE: content is DRM protected.
              FALSE: content is clear content.
  ****************************************************************************/
bool Mpeg4File::IsDRMProtection()
{
  if(m_bOMADRMV2Encrypted || m_ulPSSHatomEntryCount)
    return true;
  else
    return false;
}
/************************************************************************!
  @brief   Function to get the number of DRM Scheme supported.

  @param[in,out]   pulNoOfDRMSupported will provide the number of DRM scheme
                   supported by media.

  @note    It is expected that user has received the successul callback
           for OpenFile() earlier.
  ****************************************************************************/
 FileSourceStatus Mpeg4File::GetNumberOfDRMSupported(uint32* pulNoOfDRMSupported)
{
  FileSourceStatus retStatus = FILE_SOURCE_FAIL;
  if(NULL != pulNoOfDRMSupported)
  {
     *pulNoOfDRMSupported = m_ulPSSHatomEntryCount;
      retStatus = FILE_SOURCE_SUCCESS;
  }
  else
  {
    retStatus = FILE_SOURCE_FAIL;
  }
  return retStatus;
}
 /************************************************************************!
  @brief   Function to get the DRM Scheme.

  @param[in,out]   drmtype specify the DRM Scheme.

  @return         Returns FILE_SOURCE_SUCCESS if DRM INFO available.
  ****************************************************************************/
 FileSourceStatus Mpeg4File::GetDRMType(FileSourceDrmType& drmtype)
 {
   MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "Mpeg4File::GetDRMType");
   drmtype = FILE_SOURCE_NO_DRM;

   if(m_ulPSSHatomEntryCount > 0 &&
      m_aSINFatomEntryArray.GetLength() >= m_ulSelectedDRMIdx)
   {
     CSinfAtom *pSinfAtom = m_aSINFatomEntryArray.ElementAt(m_ulSelectedDRMIdx);
     if (pSinfAtom && pSinfAtom->m_pSchmAtom )
     {
       if(FILE_SOURCE_CENC_DRM == pSinfAtom->m_pSchmAtom->m_drmType )
       {
         drmtype = FILE_SOURCE_CENC_DRM;
         MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "Mpeg4File::GetDRMType FILE_SOURCE_CENC_DRM");
       }
     }
   }
   return FILE_SOURCE_SUCCESS;
 }

/* ============================================================================
  @brief  GetClipMetaData.

  @details    Checks if required metadata is present or not and provides
              data if it is available

  @param[in/out]  pucDataBuf          O/p Metadata Buffer
  @param[in/out]  pulDatabufLen       O/p Metadata size
  @param[in]      ienumData           FileSource Metadata type Enum

  @return  PARSER_ErrorNone if successful
           else returns corresponding error.
  @note       None.
=============================================================================*/
PARSER_ERRORTYPE Mpeg4File::GetClipMetaData(wchar_t* pucDataBuf,
                                            uint32*  pulDatabufLen,
                                            FileSourceMetaDataType ienumData)
{
  bool             bFound     = false;
  uint32           ulDataSize = 0;
  PARSER_ERRORTYPE eRetType   = PARSER_ErrorDefault;
  if((pulDatabufLen == NULL) || (FILE_SOURCE_MD_UNKNOWN == ienumData))
  {
    return PARSER_ErrorInvalidParam;
  }

  //! Reset this flag every time.
  // If metadata is in UTF8 format, this flag will be set to true
  m_eEncodeType = FS_ENCODING_TYPE_UNKNOWN;

  /* Creation date is available as part of 'MVHD' atom.
     Convert data into string format and give it to client. */
  if (FILE_SOURCE_MD_CREATION_DATE == ienumData)
  {
    if (pucDataBuf)
    {
      uint8 ucDate[128];
      memset(ucDate, 0, 128);
      if (*pulDatabufLen < 128)
      {
        *pulDatabufLen = 128;
        return PARSER_ErrorInsufficientBufSize;
      }
      memset(pucDataBuf, 0, *pulDatabufLen);
#ifdef _ANDROID_
      snprintf((char*)ucDate, sizeof(ucDate), (char*)"%llu",
               m_videoFmtInfo.file_info.creation_time);
#else
      std_strlprintf((char*)ucDate, sizeof(ucDate), (char*)"%llu",
                     m_videoFmtInfo.file_info.creation_time);
#endif
      m_eEncodeType = FS_TEXT_ENC_UTF8;
    }
    *pulDatabufLen = 128;
    eRetType       = PARSER_ErrorNone;
    bFound         = true;
  }
  else if ((FILE_SOURCE_MD_GEOTAG == ienumData) && (m_ulGeoTagSize))
  {
    if (pucDataBuf)
    {
      memset(pucDataBuf, 0, *pulDatabufLen);
      if (*pulDatabufLen < m_ulGeoTagSize)
      {
        *pulDatabufLen = m_ulGeoTagSize;
        return PARSER_ErrorInsufficientBufSize;
      }
      memcpy(pucDataBuf, m_ucGeoTagLoc, m_ulGeoTagSize);
      m_eEncodeType = FS_TEXT_ENC_UTF8;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "GEO Tag info is present, Str is %s",
                   (char*)pucDataBuf);
    }
    *pulDatabufLen = m_ulGeoTagSize + 1;
    eRetType       = PARSER_ErrorNone;
    bFound         = true;
  }
  else if(_metaAtom )
  {
    ItunesMetaData* pMetaData = NULL;
    bFound = IsMetaDataPresentiniLst(ienumData, &pMetaData);

    if (pMetaData)
    {
      if (pucDataBuf)
      {
        eRetType = ReadMetaDataFromiLst(ienumData, pucDataBuf,
                                        pulDatabufLen, pMetaData);
      }
      else
      {
        eRetType       = PARSER_ErrorNone;
        *pulDatabufLen = pMetaData->ulMetaDataLen;
      }
    }
    else
    {
      eRetType = IsMetaDataPresentinID3(ienumData, pucDataBuf, pulDatabufLen);
      if (PARSER_ErrorInvalidParam != eRetType)
      {
        bFound = true;
      }
    }
  }
  // If Meta/iLst are not available, check if user data atoms present or not
  if(false == bFound)
  {
    DataT eDataType = DATA_ATOM_NONE;
    MapFileSourceEnumtoDataEnum(ienumData, eDataType);

    /* If Data Type is known, then read metadata into o/p buffer. */
    if(DATA_ATOM_NONE != eDataType)
    {
      eRetType = ReadMetaDataFromUDTA(eDataType, pucDataBuf, pulDatabufLen);
    }
    //! Track Number is not a separate UDTA atom. It is part of Album atom.
    //! So, this routine is not kept in general function. API used to extract
    //! Track number is different from general API.
    else if (FILE_SOURCE_MD_TRACK_NUM == ienumData && _albumAtom)
    {
      uint8* pucTemp = _albumAtom->getUdtaTrackNumStr();
      if(pucDataBuf && *pulDatabufLen >= 4)
        memcpy(pucDataBuf, pucTemp, 4);
      m_eEncodeType     = FS_TEXT_ENC_UTF8;
      *pulDatabufLen    = 4;
      eRetType          = PARSER_ErrorNone;
    }
  }
  return eRetType;
}

/* ============================================================================
  @brief  ReadMetaDataFromUDTA.

  @details    This function used to read data from UDTA meta atoms.
              This function also calculates the size of metadata

  @param[in]      eDataType           UDTA atom type
  @param[in/out]  pucDataBuf          O/p buffer to read metadata.
  @param[in/out]  pulDatabufLen       O/p Buffer size

  @return  PARSER_ErrorNone if successful
           else returns corresponding error.
  @note       None.
=============================================================================*/
PARSER_ERRORTYPE Mpeg4File::ReadMetaDataFromUDTA(DataT    eDataType,
                                                 wchar_t* pucDataBuf,
                                                 uint32*  pulDatabufLen)
{
  //Validate input param
  if (NULL == pulDatabufLen)
  {
    return PARSER_ErrorInvalidParam;
  }
  /*  UDTA atom structures will have following format:
      Size   : 4 bytes
      Type   : 4 bytes
      Version: 1 byte
      Flags  : 3 bytes
      Pad    : 1 bit
      Lang   : 15 bits (Packed ISO-639-2/T language code)
      Data   : Metadata string either in UTF-8 or UTF-16 format.
      0x FF FE (Byte Order Mark) at first two bytes will help to
      differentiate between UTF8 and UTF16.

      Presently, Parser is giving both UTF8 and UTF16 format data fields,
      as it is to upper client. It is tested that UTF8 will not have any
      issues. If there is any problem with UTF16 format metadata, then
      we will convert this into UTF32 format.
  */
  uint32 ulDataSize = GetDataSize(eDataType, 0);
  if((ulDataSize) && (NULL == pucDataBuf) && (DATA_ATOM_UDTA_YEAR != eDataType))
  {
    uint8* pucTemp = (uint8*)MM_Malloc(ulDataSize);
    //! First 2 bytes are of no use, they are for Language field
    ulDataSize -= 2;
    //! Below logic is to check, whether there are any valid characters apart
    //! from mandatory NULL char. If there is no data except for NULL character
    //! ignore the data field.
    if (pucTemp)
    {
      GetData(eDataType, (uint8*)pucTemp, ulDataSize, 2);
      //! If first two bytes are 0xFEFF, then its UTF16
      if (0xFE == pucTemp[0] || 0xFF == pucTemp[1])
      {
        //! 2 bytes are BOM and 2 bytes for NULL character
        //! This is done to count actual metadata size
        ulDataSize -= 2;
        ulDataSize -= 2;
      }
      else
      {
        ulDataSize -= 1;
      }
      MM_Free(pucTemp);
    }
  }
  if ((pucDataBuf) && (ulDataSize))
  {
    /* Skip first 2bytes of data. 2bytes has language field which is not
       required now. Third parameter in the below function indicates start
       offset field, from which data to be copied. */
    uint32 ulOffset = 2;
    //! In case of recording YEAR (YRCC), language field is not available.
    if (DATA_ATOM_UDTA_YEAR == eDataType)
    {
      ulOffset = 0;
    }

    //Check if buffer size is sufficient to copy the whole metadata
    if (*pulDatabufLen < (ulDataSize + FOURCC_SIGNATURE_BYTES))
    {
      *pulDatabufLen = ulDataSize + FOURCC_SIGNATURE_BYTES;
      return PARSER_ErrorInsufficientBufSize;
    }
    memset(pucDataBuf, 0, ulDataSize + FOURCC_SIGNATURE_BYTES);

    GetData(eDataType, (uint8*)pucDataBuf, ulDataSize, ulOffset);
    if( (((uint8*)pucDataBuf)[0] == 0xFE && ((uint8*)pucDataBuf)[1] == 0xFF) ||
        (((uint8*)pucDataBuf)[0] == 0xFF && ((uint8*)pucDataBuf)[1] == 0xFE) )
    {
      m_eEncodeType = FS_TEXT_ENC_UTF16;
    }
    else
    {
      m_eEncodeType = FS_TEXT_ENC_UTF8;
    }
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "MetaData str is %s and MetaData Len %lu",
                 (char*)pucDataBuf, ulDataSize);
    //! For safety allocate 4 bytes extra
    ulDataSize += FOURCC_SIGNATURE_BYTES;
  }
  else if(ulDataSize)
  {
    //! Update Metadata Size to include NULL characters and double to support
    //! UTF8 format.
    ulDataSize ++;
    ulDataSize *= 4;
    //! For safety allocate 4 bytes extra
    ulDataSize += FOURCC_SIGNATURE_BYTES;
  }
  //! Update output size parameter
  *pulDatabufLen = ulDataSize;
  return PARSER_ErrorNone;
}

/* ============================================================================
  @brief  MapFileSourceEnumtoDataEnum.

  @details    This function used to Map FileSource Metadata Enums with UDTA .
              metadata atom Enums

  @param[in]      ienumData           FileSource Metadata Enum
  @param[in/out]  eDataType           UDTA atom type
  @param[in/out]  pulDatabufLen       O/p Buffer size

  @return     None
  @note       None.
=============================================================================*/
void Mpeg4File::MapFileSourceEnumtoDataEnum(FileSourceMetaDataType ienumData,
                                            DataT &eDataType)
{
  switch(ienumData)
  {
    case FILE_SOURCE_MD_TITLE:
    {
      eDataType = DATA_ATOM_UDTA_TITL;
      break;
    }
    case FILE_SOURCE_MD_AUTHOR:
    {
      eDataType = DATA_ATOM_UDTA_AUTH;
      break;
    }
    case FILE_SOURCE_MD_ALBUM:
    {
      eDataType = DATA_ATOM_UDTA_ALBUM;
      break;
    }
    case FILE_SOURCE_MD_COPYRIGHT:
    {
      eDataType = DATA_ATOM_UDTA_CPRT;
      break;
    }
    case FILE_SOURCE_MD_CLASSIFICATION:
    {
      eDataType = DATA_ATOM_UDTA_CLSF;
      break;
    }
    case FILE_SOURCE_MD_DESCRIPTION:
    {
      eDataType = DATA_ATOM_UDTA_DSCP;
      break;
    }
    case FILE_SOURCE_MD_RATING:
    {
      eDataType = DATA_ATOM_UDTA_RTNG;
      break;
    }
    case FILE_SOURCE_MD_PERFORMANCE:
    case FILE_SOURCE_MD_ARTIST:
    {
      eDataType = DATA_ATOM_UDTA_PERF;
      break;
    }
    case FILE_SOURCE_MD_REC_YEAR:
    {
      eDataType = DATA_ATOM_UDTA_YEAR;
      break;
    }
    case FILE_SOURCE_MD_KEYWORD:
    {
      eDataType = DATA_ATOM_UDTA_KYWD;
      break;
    }
    case FILE_SOURCE_MD_LOCATION:
    {
      eDataType = DATA_ATOM_UDTA_LOCI;
      break;
    }
    case FILE_SOURCE_MD_GENRE:
    {
      eDataType = DATA_ATOM_UDTA_GNRE;
      break;
    }
    default:
    {
      eDataType = DATA_ATOM_NONE;
    }
    break;
  }
}

/* ============================================================================
  @brief  IsMetaDataPresentiniLst.

  @details    Checks if the required metadata type info is present in
              iLst atom or not

  @param[in]      ienumData           Metadata type requested
  @param[in/out]  pMetaData           Metadata structure Pointer

  @return  "true" if required metadata field is available
           else returns "false".
  @note       None.
=============================================================================*/
bool Mpeg4File::IsMetaDataPresentiniLst(FileSourceMetaDataType ienumData,
                                        ItunesMetaData**       dpMetaData)
{
  bool bFound = false;
  if(NULL == dpMetaData)
  {
    return bFound;
  }
  uint32 ulCount            = _metaAtom->m_uliLstAtomCount;
  ItunesMetaData *pMetaData = NULL;
  for (uint32 ulIndex = 0; ulIndex < ulCount; ulIndex++)
  {
    UdtaiLstAtom *piLstAtom = _metaAtom->m_aiLstAtomEntryArray[ulIndex];
    uint32 ulMetaCount      = 0;
    uint32 ulMetaDataIndex  = 0;
    if (piLstAtom)
    {
      ulMetaCount = piLstAtom->m_ulMetaAtomCount;
    }
    else
    {
      continue;
    }

    for(;ulMetaDataIndex < ulMetaCount; ulMetaDataIndex++)
    {
      pMetaData = piLstAtom->m_aMetaAtomEntryArray[ulMetaDataIndex];

      if ((pMetaData) && ((pMetaData->eMetaType == ienumData) ||
          (FILE_SOURCE_MD_PADDING_DELAY == ienumData &&
           FILE_SOURCE_MD_ENC_DELAY     == pMetaData->eMetaType)))
      {
        bFound = true;
        *dpMetaData = pMetaData;
        break;
      }
    }
  }
  return bFound;
}

/* ============================================================================
  @brief  ReadMetaDataFromiLst.

  @details    Read Metadata from iLst Atom

  @param[in]      ienumData           Metadata type requested
  @param[in/out]  pucDataBuf          O/p Metadata Buffer
  @param[in/out]  pulDatabufLen       O/p Metadata size
  @param[in]      pMetaData           Metadata structure Pointer

  @return  PARSER_ErrorNone if successful
           else returns corresponding error.
  @note       This will be called only if "IsMetaDataPresentiniLst" function
              returns True.
=============================================================================*/
PARSER_ERRORTYPE Mpeg4File::ReadMetaDataFromiLst(
                                              FileSourceMetaDataType ienumData,
                                              wchar_t*        pucDataBuf,
                                              uint32*         pulDatabufLen,
                                              ItunesMetaData* pMetaData)
{
  uint32 ulBufIndex    = 0;
  uint32 ulMetaDataLen = pMetaData->ulMetaDataLen;
  /* Encoder and Padding delays will be available in same metadata
     section :iTunSMPB". One example of iTunSMPB metadata:
     00000000 00000840 000001CA 00000000003F31F6 00000000 00000000
     00000000 00000000 00000000 00000000 00000000 00000000.
     In the above data,
     0x840 indicates Encoder delay and
     0x1CA indicates Padding delay
     0x3F31F6 indicates total number of samples
     <Space><8bytes ZERO Padding><Space><8bytes Encoder Delay>
     <Space><8bytes Padding delay><Space>
  */
  if (FILE_SOURCE_MD_ENC_DELAY == ienumData)
  {
    ulBufIndex    = ENCODER_DELAY_OFFSET;
    ulMetaDataLen = FOURCC_SIGNATURE_BYTES * 2;
    memcpy(pucDataBuf, pMetaData->pucMetaData + ulBufIndex,
           ulMetaDataLen);
  }
  else if (FILE_SOURCE_MD_PADDING_DELAY == ienumData)
  {
    ulBufIndex    = PADDING_DELAY_OFFSET;
    ulMetaDataLen = FOURCC_SIGNATURE_BYTES * 2;
    memcpy(pucDataBuf, pMetaData->pucMetaData + ulBufIndex,
           ulMetaDataLen);
  }
  //If metadata requested is not Album Artist info
  else
  {
    // Check if buffer size sufficient to read data or not
    if (*pulDatabufLen < ulMetaDataLen)
    {
      *pulDatabufLen = ulMetaDataLen;
      return PARSER_ErrorInsufficientBufSize;
    }
    memset(pucDataBuf, 0, *pulDatabufLen);

   /* Check whether text string is in UTF-8 or UTF-16 format.
      Copy both UTF-8 and UTF-16 metadata formats as it is to upper
      component.
   */
    memcpy(pucDataBuf, pMetaData->pucMetaData, ulMetaDataLen);
    m_eEncodeType = FS_TEXT_ENC_UTF8;

    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "MetaData str is %s and MetaData Len %lu",
                 (char*)pucDataBuf, ulMetaDataLen);
  }
  return PARSER_ErrorNone;
}

/* ============================================================================
  @brief  getAlbumArt.

  @details    Checks if Album art/cover art info is present or not and provides
              data if it is available

  @param[in/out]  pucDataBuf          O/p Metadata Buffer
  @param[in/out]  pulDatabufLen       O/p Metadata size

  @return  PARSER_ErrorNone if successful
           else returns corresponding error.
  @note       This will be called only if "IsMetaDataPresentiniLst" function
              returns True.
=============================================================================*/
PARSER_ERRORTYPE Mpeg4File::getAlbumArt(wchar_t *pucDataBuf, uint32 *pulDatabufLen)
{
  ItunesMetaData*  pMetaData  = NULL;
  PARSER_ERRORTYPE eRetError  = PARSER_ErrorDefault;
  uint32 ulMetaDataLen = 0;
  bool   bAvailable    = false;
  //! Validate input params
  if((NULL == pulDatabufLen) || (NULL == _metaAtom))
  {
    return PARSER_ErrorInvalidParam;
  }
  //! Reset buffer size, if Buffer is not yet allocated by client
  else if (NULL == pucDataBuf)
  {
    *pulDatabufLen = 0;
  }

  //! Check if album art is present in iLst or not
  bAvailable = IsMetaDataPresentiniLst(FILE_SOURCE_MD_ALBUM_ART, &pMetaData);
  if((true == bAvailable) && (pMetaData) && (pMetaData->pucMetaData))
  {
    ulMetaDataLen = pMetaData->ulMetaDataLen;
    if (pucDataBuf)
    {
      FS_ALBUM_ART_METADATA *pAlbArt = (FS_ALBUM_ART_METADATA*)pucDataBuf;

      if(*pulDatabufLen < (ulMetaDataLen + FIXED_APIC_FIELDS_LENGTH))
      {
        return PARSER_ErrorInsufficientBufSize;
      }
      memset(pucDataBuf, 0, FIXED_APIC_FIELDS_LENGTH);
      pAlbArt->ucTextEncodeType = pMetaData->eDataType;
      pAlbArt->picType          = PIC_TYPE_COVER_FRONT;
      pAlbArt->ulPicDataLen     = ulMetaDataLen;
      //! Update Image format
      switch (pMetaData->eDataType)
      {
        case ITUNES_JPEG:
          pAlbArt->imgFormat = IMG_JPG;
          break;
        case ITUNES_PNG:
          pAlbArt->imgFormat = IMG_PNG;
          break;
        case ITUNES_BMP:
          pAlbArt->imgFormat = IMG_BMP;
          break;
        case ITUNES_GIF:
          pAlbArt->imgFormat = IMG_GIF;
          break;
        default:
          pAlbArt->imgFormat = IMG_UNK;
          break;
      }

      //Copy locale indicator data into description
      memcpy(pAlbArt->ucDesc, &pMetaData->ulLocaleIndicator,
             sizeof(uint32));
      memcpy(pAlbArt->pucPicData, pMetaData->pucMetaData, ulMetaDataLen);
      //! Image format string is not available in 3gp Metadata
      memset(pAlbArt->ucImgFormatStr, 0, MAX_IMG_FORMAT_LEN);
    } //if (pucDataBuf)
    *pulDatabufLen = ulMetaDataLen + FIXED_APIC_FIELDS_LENGTH;
    eRetError      = PARSER_ErrorNone;
  } //if((true == bAvailable) && (*pulDatabufLen) && (pMetaData))
  else if (_metaAtom->m_ulID3AtomCount)
  {
    for (uint32 ulCount = 0; ulCount < _metaAtom->m_ulID3AtomCount; ulCount++)
    {
      metadata_id3v2_type* pID3V2 = _metaAtom->m_aID3AtomArray[ulCount];
      //! Parse album art info and update o/p buffer
      eRetError = ParseAlbumArtFromID3V2(pID3V2, pucDataBuf, pulDatabufLen);
      if (PARSER_ErrorInvalidParam != eRetError)
      {
        break;
      }
    }
  }
  return eRetError;
}

/* ============================================================================
  @brief  IsMetaDataPresentinID3.

  @details    Checks if the required metadata type info is present in
              ID3 atom or not

  @param[in]      ienumData           Metadata type requested
  @param[in/out]  pucDataBuf          Buffer to store Metadata
  @param[in/out]  pulDatabufLen       Buffer size

  @return  "PARSER_ErrorNone" if successful
           else returns corresponding error.
  @note       None.
=============================================================================*/
PARSER_ERRORTYPE Mpeg4File::IsMetaDataPresentinID3(
                                     FileSourceMetaDataType ienumData,
                                     wchar_t*  pucDataBuf,
                                     uint32*   pulDatabufLen)
{
  PARSER_ERRORTYPE eRet     = PARSER_ErrorInvalidParam;
  uint32 ulTotalID3Entries  = _metaAtom->m_ulID3AtomCount;

  for(uint32 ulIndex = 0; ulIndex < ulTotalID3Entries; ulIndex++)
  {
    metadata_id3v2_type *pID3v2Info = _metaAtom->m_aID3AtomArray[ulIndex];

    //! Check the given entry is available current ID3 metadata entry
    eRet = ParseID3V2MetaData(pID3v2Info, ienumData, pucDataBuf, pulDatabufLen,
                              m_eEncodeType);
    if (PARSER_ErrorInvalidParam != eRet)
    {
      break;
    }
  }//!for(ulIndex = 0; ulIndex < ulTotalID3Entries && !pTextFrame; ulIndex++)

  return eRet;
}

