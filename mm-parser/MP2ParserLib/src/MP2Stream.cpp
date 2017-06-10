/* =======================================================================
                              MP2Stream.cpp
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

Copyright (c) 2009-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP2ParserLib/main/latest/src/MP2Stream.cpp#100 $
========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE
========================================================================== */
#include "MP2StreamParser.h"
#include "MP2Stream.h"
#include "MMMalloc.h"
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
                            Function Definitions
** ======================================================================= */

/* ======================================================================
FUNCTION
  MP2StreamCallbakGetData

DESCRIPTION
  Its a callback function from the SDK to get the M2TS data for decoding.
  This is not implemented by the SDK. It should be implemented by the app
  that calls the SDK.

ARGUMENTS
  ullOffset           Offset of the requested data (from beginning),
  ulNumBytesRequest   Size of the requested data (in bytes).
  pucData             Pointer to data buffer.
  ulMaxBufSize        Maximum data buffer size.
  pUserData           Client user data information.

DEPENDENCIES
  Used by M2TS Parser.

RETURN VALUE
  The number of bytes actually being filled in

SIDE EFFECTS
  Detail any side effects.
========================================================================== */
uint32 MP2StreamCallbakGetData (uint64 ullOffset, uint32 ulNumBytesRequest,
                                uint8* pucData, uint32 ulMaxBufSize,
                                void* pUserData )
{
  if(pUserData)
  {
    MP2Stream* pMP2TStream = (MP2Stream*)pUserData;
    /* Just return number of bytes read. Partial data skipping can be done internal to stream parser object*/
    return( pMP2TStream->FileGetData( ullOffset, ulNumBytesRequest,
                                      ulMaxBufSize, pucData ) );
  }
  return 0;
}
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
/* ======================================================================
FUNCTION:
  CheckAvailableDataSize

DESCRIPTION:
  Determines number of bytes available,applicable only in case of HTTP streaming.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
void MP2Stream::CheckAvailableDataSize(uint64* bufDataSize, boolean *pbEndOfData)
{
  if(m_pPort && bufDataSize && pbEndOfData)
  {
#ifdef MPEG2_PARSER_DEBUG
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "CheckAvailableDataSize querying IStreamPort");
#endif
    int64 tmpoffset = 0;
    bool beod = false;
    if(m_pPort->GetAvailableOffset(&tmpoffset,&beod) == video::iSourcePort::DS_SUCCESS)
    {
      if(beod && m_pMP2StreamParser)
      {
        m_pMP2StreamParser->SetEOFFlag((uint64)tmpoffset);
      }
      *bufDataSize = tmpoffset;
      *pbEndOfData = beod;
    }
  }
}

void MP2Stream::GetBufferLowerBound(uint64* pBufLowerOffset, boolean *pbEndOfData)
{
  if(m_pPort && pBufLowerOffset && pbEndOfData)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "GetBufferLowerBound querying IStreamPort");
    int64 sllOffset = 0;
    bool  beod      = false;
    if(m_pPort->GetBufferLowerBound(&sllOffset,&beod) ==
      video::iSourcePort::DS_SUCCESS)
    {
      *pBufLowerOffset = sllOffset;
      *pbEndOfData     = beod;
    }
  }
}
#endif

/* ======================================================================
FUNCTION:
  SetConfiguration

DESCRIPTION:
  Called by user to set output mode specified by henum

INPUT/OUTPUT PARAMETERS:
  henum-Output mode

RETURN VALUE:
 FILE_SOURCE_SUCCESS if successful in setting output mode else returns FILE_SOURCE_FAIL

SIDE EFFECTS:
  None.
======================================================================*/
FileSourceStatus MP2Stream::SetConfiguration(FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;

  if(m_pMP2StreamParser)
  {
    status = m_pMP2StreamParser->SetConfiguration(henum);
  }
  return status;
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
FileSourceStatus MP2Stream::SetAudioOutputMode(FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;

  if(m_pMP2StreamParser)
  {
    status = m_pMP2StreamParser->SetAudioOutputMode(henum);
  }
  return status;
}
/* ======================================================================
FUNCTION:
  GetAudioOutputMode

DESCRIPTION:
  Called by user to check what audio audio output mode is set

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 FILE_SOURCE_SUCCESS if successful in retrieving output mode else returns FILE_SOURCE_FAIL

SIDE EFFECTS:
  None.
======================================================================*/
FileSourceStatus MP2Stream::GetAudioOutputMode(bool* bret, FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pMP2StreamParser)
  {
    status = m_pMP2StreamParser->GetAudioOutputMode(bret,henum);
  }
  return status;
}
/* ======================================================================
FUNCTION:
  MP2Stream::FileGetData

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
uint32 MP2Stream::FileGetData(uint64 nOffset, uint32 nNumBytesRequest,
                               uint32 nMaxSize, uint8* pData  )
{
  uint32 nRead = 0;
  if (m_pFilePtr != NULL)
  {
    if(!m_bMediaAbort)
    {
      nRead = FileBase::readFile(m_pFilePtr, pData, nOffset, FILESOURCE_MIN(nNumBytesRequest,nMaxSize));
    }
    else if(m_pMP2StreamParser)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Breaking, user Abort is true.");
      m_pMP2StreamParser->SetMediaAbortFlag();
    }
  }
  return nRead;
}
/*!===========================================================================
  @brief      Get Audio/Video/Text stream parameter
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
============================================================================*/
PARSER_ERRORTYPE MP2Stream::GetStreamParameter( uint32 ulTrackId,
                                              uint32 ulParamIndex,
                                              void* pParamStruct)
{
  PARSER_ERRORTYPE eError = PARSER_ErrorInvalidParam;

  if ((m_pMP2StreamParser) && (pParamStruct))
  {
      eError = m_pMP2StreamParser -> GetStreamParameter(ulTrackId,
                                              ulParamIndex,
                                              pParamStruct);
  }
  return eError;
}

/*!===========================================================================
  @brief      Set Audio/Video/Text stream parameter
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
============================================================================*/
PARSER_ERRORTYPE MP2Stream::SetStreamParameter( uint32 ulTrackId,
                                                uint32 ulParamIndex,
                                                void* pParamStruct)
{
  PARSER_ERRORTYPE eError = PARSER_ErrorInvalidParam;

  if ((m_pMP2StreamParser) && (pParamStruct))
  {
      eError = m_pMP2StreamParser -> SetStreamParameter(ulTrackId,
                                              ulParamIndex,
                                              pParamStruct);
  }
  return eError;
}
/* ======================================================================
FUNCTION:
  MP2Stream::MP2Stream

DESCRIPTION:
  MP2Stream constructor

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
MP2Stream::MP2Stream(const FILESOURCE_STRING filename
                     ,bool bneedcodechdr
                     ,unsigned char* /*pFileBuf*/
                     ,uint32 /*bufSize*/
                     ,bool bPlayVideo
                     ,bool bPlayAudio
                     ,FileSourceFileFormat eFormat
                    )
{
  InitData();
  m_playAudio = bPlayAudio;
  m_playVideo = bPlayVideo;
  m_bHaveTransportRate = false;
  m_pFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"),MP2_TS_CACHE_SIZE);
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
  uint64 size = OSCL_FileSize(filename);
  m_pMP2StreamParser = MM_New_Args(MP2StreamParser,
      (this,size,bneedcodechdr,bHttpStreaming, eFormat));
  m_fileSize = size;
#endif
  (void)ParseMetaData();
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
MP2Stream::MP2Stream(video::iStreamPort* pport,bool bneedcodechdr,
    bool bPlayVideo, bool bPlayAudio, FileSourceFileFormat eFormat)
{
  InitData();
  m_pFilePtr = OSCL_FileOpen(pport);
  m_pPort = pport;
  uint64 size = MAX_FILE_SIZE;
  bHttpStreaming = true;
  m_bHaveTransportRate = false;
  m_playAudio = bPlayAudio;
  m_playVideo = bPlayVideo;
  /* For IStreamPort, we do not need to know the content length.
     Default value MAX_FILE_SIZE will serve the purpose.
  */
  int64 nDownloadedBytes = 0;
  bool bEndOfData = false;
  if(pport)
  {
    video::iSourcePort::DataSourceType eDataSource =
      video::iSourcePort::DS_STREAMING_SOURCE;
    (void)pport->GetSourceType(&eDataSource);
    if (video::iSourcePort::DS_STREAMING_SOURCE != eDataSource)
    {
      bHttpStreaming = false;
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                  "Set from streaming to local playback mode");
    }
    if(pport->GetAvailableOffset(&nDownloadedBytes, &bEndOfData) == video::iSourcePort::DS_SUCCESS)
    {
      if(false == bEndOfData)
      {
        if(video::iSourcePort::DS_SUCCESS != pport->GetContentLength(&nDownloadedBytes))
          nDownloadedBytes = MAX_FILE_SIZE;
      }
      m_fileSize = size = (uint64)nDownloadedBytes;
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "Setting m_fileSize to %llu",m_fileSize);
    }
  }
  else
  {
    _success = false;
    return;
  }
  if(bPlayVideo || bPlayAudio)
  {
    m_pMP2StreamParser = MM_New_Args(MP2StreamParser,
        (this,size,bneedcodechdr,bHttpStreaming, eFormat));
    (void)parseHTTPStream();
  }
}
#endif
/* ======================================================================
FUNCTION:
  MP2Stream::InitData

DESCRIPTION:
  Initializes all the data members to default

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
void MP2Stream::InitData()
{
  m_playAudio   = false;
  m_playVideo   = false;
  m_playText    = false;
  m_corruptFile = false;
  m_bStreaming  = false;
  m_nNumStreams = 0;
  m_audioLargestSize = 0;
  m_videoLargestSize = 0;
  m_nPrevPESPTS = 0;
  m_bMediaAbort = false;

  memset(&m_sampleInfo,0,(FILE_MAX_MEDIA_STREAMS * sizeof(file_sample_info_type)) );
  memset(&m_nDecodedDataSize,0,(FILE_MAX_MEDIA_STREAMS * 4) );
  memset(&m_nLargestFrame,0,(FILE_MAX_MEDIA_STREAMS * 4) );

  m_filename = NULL;
  m_pFileBuf = NULL;
  m_FileBufSize = 0;
  m_fileSize = 0;
  m_pFilePtr = NULL;
  m_pMP2StreamParser = NULL;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  m_pPort = NULL;
#endif
  //m_nSelectedAudioStreamId = -1;
  //m_nSelectedVideoStreamId = -1;
  //m_nSelectedTextStreamId = -1;

  m_pIndTrackIdTable = NULL;
  _success = false;
  _fileErrorCode = PARSER_ErrorDefault;

#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
   m_minOffsetRequired = 0;
   bHttpStreaming = false;
   m_wBufferOffset = 0;
   bGetMetaDataSize = false;
   bIsMetaDataParsed = false;
   m_startupTime = 0;
   parserState = PARSER_IDLE;

   m_HttpDataBufferMinOffsetRequired.Offset = 0;
   m_HttpDataBufferMinOffsetRequired.bValid = FALSE;
#endif //FEATURE_QTV_3GPP_PROGRESSIVE_DNLD
}
void MP2Stream::SetCriticalSection(MM_HANDLE pcriticalsection)
{
  if(m_pFilePtr)
  {
    m_pFilePtr->pCriticalSection = pcriticalsection;
  }
}

/* ======================================================================
FUNCTION:
  MP2Stream::GetLastRetrievedSampleOffset

DESCRIPTION:
  Returns the offset of the last retrieved sample

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 MP2Stream::GetLastRetrievedSampleOffset(uint32 trackid)
{
  uint64 offset = 0;
  if(m_pMP2StreamParser)
  {
    offset = m_pMP2StreamParser->GetLastRetrievedSampleOffset(trackid);
  }
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_LOW, "GetLastRetrievedSampleOffset %llu",
               offset);
  return offset;
}

/* ======================================================================
FUNCTION:
  MP2Stream::~MP2Stream

DESCRIPTION:
  MP2Stream constructor

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
MP2Stream::~MP2Stream()
{
  if(m_pFilePtr!=NULL)
  {
     OSCL_FileClose(m_pFilePtr);
     m_pFilePtr = NULL;
  }
  if(m_pMP2StreamParser)
  {
    MM_Delete(m_pMP2StreamParser);
  }
  if(m_pIndTrackIdTable)
  {
    MM_Free(m_pIndTrackIdTable);
  }
}
/* ======================================================================
FUNCTION:
  MP2Stream::ParseMetaData

DESCRIPTION:
  Starts parsing the MPEG2 transport stream.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
bool MP2Stream::ParseMetaData()
{
  bool bRet = false;
  if(m_pMP2StreamParser)
  {
    if(m_pMP2StreamParser->StartParsing() == MP2STREAM_SUCCESS)
    {
      bRet = true;
      _success = true;
      _fileErrorCode = PARSER_ErrorNone;
      m_nNumStreams = m_pMP2StreamParser->GetTotalNumberOfTracks();
      if(m_nNumStreams)
      {
        uint32* idlist = (uint32*)MM_Malloc(m_nNumStreams * 4 );
        m_pIndTrackIdTable = (TrackIdToIndexTable*)MM_Malloc(m_nNumStreams * sizeof(TrackIdToIndexTable));
        if(m_pIndTrackIdTable && idlist)
        {
          memset(m_pIndTrackIdTable,0,m_nNumStreams * sizeof(TrackIdToIndexTable));
          if(m_pMP2StreamParser->GetTrackWholeIDList(idlist) == m_nNumStreams)
          {
            for(uint32 i = 0; i < m_nNumStreams; i++)
            {
              m_pIndTrackIdTable[i].index = (uint8)i;
              m_pIndTrackIdTable[i].bValid = true;
              m_pIndTrackIdTable[i].trackId = idlist[i];
            }
          }
        }
        // To avoid the resource leak in case m_pIndTrackIdTable is NULL
        if(idlist)
        {
          MM_Free(idlist);
        }
      }
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
      parserState = PARSER_READY;
#endif
    }
  }
  return bRet;
}
/* ======================================================================
FUNCTION:
  MP2Stream::GetFileFormat()

DESCRIPTION:
  Returns if file is VOB or TS

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  FileSourceStatus.

SIDE EFFECTS:
  None.
======================================================================*/
FileSourceStatus MP2Stream::GetFileFormat(FileSourceFileFormat& fileFormat)
{
  FileSourceStatus retStatus = FILE_SOURCE_FAIL;
  if(m_pMP2StreamParser)
  {
    if(MP2STREAM_SUCCESS == m_pMP2StreamParser->GetFileFormat(fileFormat))
    {
      retStatus = FILE_SOURCE_SUCCESS;
    }
  }
  return retStatus;
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
PARSER_ERRORTYPE MP2Stream::getNextMediaSample(uint32 ulTrackID, uint8 *pucDataBuf,
                                               uint32 *pulBufSize, uint32 &index)
{
  int32 nBytes = 0;
  MP2StreamStatus retStatus = MP2STREAM_DEFAULT_ERROR;
  track_type cType;
  media_codec_type codec_type;
  float frameTS = 0;

  /* Validate input params and class variables */
  if(NULL == m_pMP2StreamParser || NULL == pulBufSize || NULL == pucDataBuf ||
     0 == *pulBufSize)
  {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
               "getNextMediaSample invalid input params!!");
    return PARSER_ErrorInvalidParam;
  }

  if (_success == true)
  {
    _fileErrorCode = PARSER_ErrorNone;
  }

#ifdef MPEG2_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"MP2Stream::getNextMediaSample");
#endif

  if(m_pMP2StreamParser->GetTrackType(ulTrackID, &cType,&codec_type) == MP2STREAM_SUCCESS)
  {
    bool   bRet     = false;
    uint32 ulIsSync = 0;
    (void)m_pMP2StreamParser->GetAudioOutputMode(&bRet,
                                  FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME);
    //For audio AAC ADTS, we will output 5 frames at a time for better performance
    if((cType == TRACK_TYPE_AUDIO) && (codec_type == AUDIO_CODEC_AAC) &&
       (true  == bRet))
    {
      retStatus = getNextADTSAudioSample(ulTrackID, pucDataBuf, *pulBufSize,
                                         &nBytes, index,&frameTS);
      if(nBytes)
      {
        retStatus = MP2STREAM_SUCCESS;
      }
    }
    else
    {
      retStatus = m_pMP2StreamParser->GetCurrentSample(ulTrackID, pucDataBuf,
                                                       *pulBufSize, &nBytes,
                                                       &frameTS, &ulIsSync);
    }
    if( (nBytes > 0) && (retStatus == MP2STREAM_SUCCESS))
    {
      bool bError = false;
      uint8 ucIndex = MapTrackIdToIndex(&bError, ulTrackID);
      if(!bError)
      {
        //Clear content protection details, before filling CP
        //related parameter.
        memset(&m_sampleInfo[ucIndex].sSubInfo.sCP, 0,
               sizeof(m_sampleInfo[ucIndex].sSubInfo.sCP));
        //Get PES Pvt data if available.
        if(m_pMP2StreamParser->GetPesPvtData(ulTrackID,
                                m_sampleInfo[ucIndex].sSubInfo.sCP.ucInitVector))
        {
          m_sampleInfo[ucIndex].sSubInfo.sCP.ucIsEncrypted = true;
          m_sampleInfo[ucIndex].sSubInfo.sCP.ucInitVectorSize = PES_EXTN_PVT_DATA_LEN;
        }
        else
        {
          m_sampleInfo[ucIndex].sSubInfo.sCP.ucIsEncrypted = false;
          if((cType == TRACK_TYPE_AUDIO) && (pucDataBuf) &&
            ((codec_type == AUDIO_CODEC_LPCM) || (codec_type == AUDIO_CODEC_HDMV_LPCM))
            )
          {
            if(nBytes % 2)
            {
              //LPCM only 16bits supported
              nBytes--;
            }

            if((uint32)nBytes % 2)
            {
              uint8 tempval = 0;
              for(int i = 0; i< nBytes; i+=2)
              {
                tempval = pucDataBuf[i];
                pucDataBuf[i] = pucDataBuf[i + 1];
                pucDataBuf[i+1] = tempval;
              }
            }
            else
            {
              uint16 *pDst = (uint16*)pucDataBuf;
              uint16 nVal  = 0;
              for(int i = 0; i < (nBytes >> 1); i++)
              {
                nVal = pDst[i];
                pDst[i] = (uint16)( (uint16)(nVal >> 8) | (uint16)(nVal << 8));
              }
            }
          }
        }
        m_sampleInfo[ucIndex].offset = m_pMP2StreamParser->GetSampleOffset();
        m_sampleInfo[ucIndex].num_frames = 1;
        // Forcefully giving zero as there is no perfect way to
        // calculate delta accurately.
        if(cType == TRACK_TYPE_VIDEO)
        {
          m_sampleInfo[ucIndex].delta = 0;
        }
        else if ((cType == TRACK_TYPE_AUDIO))
        {
          if(((uint64)frameTS > m_sampleInfo[ucIndex].time))
          {
            m_sampleInfo[ucIndex].delta = (uint64)frameTS -
              m_sampleInfo[ucIndex].time;
          }
        }

        m_sampleInfo[ucIndex].time = (uint64)frameTS;
        //In case of transport stream, every audio/video frame has valid PTS.
        //In case of program stream, sometime only I frames have valid timestamp.
        //As of now, setting btimevalid to true always.Might have to change for program stream.
        m_sampleInfo[ucIndex].btimevalid = true;
        m_sampleInfo[ucIndex].size = nBytes;
        m_sampleInfo[ucIndex].nBytesLost = m_pMP2StreamParser->GetBytesLost();
        m_pMP2StreamParser->GetRecentPCR(ulTrackID,
                                         &m_sampleInfo[ucIndex].nPCRValue);

        if(cType == TRACK_TYPE_AUDIO)
        {
          m_sampleInfo[ucIndex].sync = 1;
        }
        else
        {
          if(cType == TRACK_TYPE_VIDEO)
          {
            /* Parser checks if Random Access Indicator flag is set to true or
               not. */
            m_sampleInfo[ucIndex].sync = ulIsSync;
          }
        }
        m_sampleInfo[ucIndex].sample++;
        MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_MEDIUM,
          "getNextMediaSample Sample# %lu trackID %lu TIME %llu Size %lu ",
          m_sampleInfo[ucIndex].sample, ulTrackID, m_sampleInfo[ucIndex].time,
          nBytes);
      }
      else
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
          "getNextMediaSample MapTrackIdToIndex failed for trackid %lu", ulTrackID);
      }
    }
    else
    {
      if(retStatus == MP2STREAM_DATA_UNDER_RUN)
        return PARSER_ErrorDataUnderRun;
      else if(retStatus == MP2STREAM_READ_ERROR)
        return PARSER_ErrorReadFail;
      else if(retStatus == MP2STREAM_INSUFFICIENT_MEMORY)
        return PARSER_ErrorInsufficientBufSize;
      else if(retStatus == MP2STREAM_EOF)
        return PARSER_ErrorEndOfFile;
    }
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
      "getNextMediaSample GetTrackType failed for trackid %lu", ulTrackID);
  }
#ifdef MPEG2_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
"MP2Stream::getNextMediaSample returing nBytesRead %d",nBytes);
#endif
  *pulBufSize = nBytes;
  return PARSER_ErrorNone;
}
/* ======================================================================
FUNCTION:
  MP2Stream::getNextADTSAudioSample

DESCRIPTION:
  gets next sample of the given track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 size of sample

SIDE EFFECTS:
  None.
======================================================================*/
MP2StreamStatus MP2Stream::getNextADTSAudioSample(uint32 id, uint8 *buf, uint32 size,
                                                  int32 *nBytesRead, uint32& /*index*/,
                                                  float* frameTS)
{
  int32 nBytes = 0;
  uint32 currentSize = 0;
  uint32 ulCount = 0;
  float timestampForSamples = 0;
  bool bRet=false;

  MP2StreamStatus retStatus = MP2STREAM_DEFAULT_ERROR;
  FileSourceStatus status = FILE_SOURCE_SUCCESS;

  while( (ulCount < M2TS_MAX_ADTS_FRAMES_AT_ONCE) &&
         (currentSize < (size)) && (retStatus != MP2STREAM_EOF) )
  {
    *frameTS = 0;
    nBytes = 0;

    retStatus = m_pMP2StreamParser->GetCurrentSample(id,buf+currentSize,(size-currentSize),&nBytes, frameTS);

    if(!ulCount)
    {
      //The timestamp of the first sample of the 5 should be used
      timestampForSamples = *frameTS;
    }

    //Check audio output mode to see if header needs to be stripped
    FileSourceConfigItemEnum outputMode = FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER;
    bRet = false;
    if( (m_pMP2StreamParser->GetAudioOutputMode(&bRet,outputMode) == status) && (bRet))
    {
      uint32 numBytes = 0;
      numBytes = GetNumBytesToStrip(id,(int)AUDIO_CODEC_AAC);
      numBytes = FILESOURCE_MIN((uint32)nBytes,numBytes);
      if((nBytes && numBytes) && (size > currentSize+numBytes))
      {
        memmove(buf+currentSize,buf+currentSize+numBytes,nBytes-numBytes);
        nBytes -= numBytes;
      }
    }
    currentSize += nBytes;
    ulCount++;
    if(MP2STREAM_SUCCESS != retStatus)
      break;
  }

  *frameTS = timestampForSamples;
  *nBytesRead = currentSize;

  return retStatus;
}
/* ======================================================================
FUNCTION:
  MP2Stream::GetNumBytesToStrip()

DESCRIPTION:
  Output number of bytes to strip for codec type

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  MP2StreamStatus.

SIDE EFFECTS:
  None.
======================================================================*/
uint32 MP2Stream::GetNumBytesToStrip(uint32 id, int codec_type)
{
  uint32 bytesToStrip = 0;

  if((media_codec_type)codec_type == AUDIO_CODEC_AAC)
  {
    aac_audio_info sAACInfo;
    memset(&sAACInfo, 0, sizeof(aac_audio_info));
    if( ( NULL!= m_pMP2StreamParser ) &&
        ( (true == m_pMP2StreamParser->GetAACAudioInfo( id, &sAACInfo ) ) &&
        ( AAC_FORMAT_ADTS == m_pMP2StreamParser->GetAACAudioFormat(id) ) ) )
    {
      if(sAACInfo.ucCRCPresent)
      {
        // +2 to account CRC bytes
        bytesToStrip = M2TS_AAC_ADTS_HDR_LEN + 2;
      }
      else
      {
        bytesToStrip = M2TS_AAC_ADTS_HDR_LEN;
      }
    }
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
    "MP2Stream::GetNumBytesToStrip %lu", bytesToStrip);
  return bytesToStrip;
}

/* ======================================================================
FUNCTION:
  MP2Stream::getMediaTimestampForCurrentSample

DESCRIPTION:
  gets time stamp of current sample of the track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 time stamp in track time scale unit

SIDE EFFECTS:
  None.
======================================================================*/
uint64 MP2Stream::getMediaTimestampForCurrentSample(uint32 id)
{
  bool bError = false;
  uint64 ts = 0;
  uint8 index = MapTrackIdToIndex(&bError,id);
  if(!bError)
  {
    ts = m_sampleInfo[index].time;
  }
  return ts;
}
#ifdef FEATURE_FILESOURCE_REPOSITION_SYNC_FRAME
/* ======================================================================
FUNCTION:
  MP2Stream::skipNSyncSamples

DESCRIPTION:
  Skips specified sync samples in forward or backward direction.

INPUT/OUTPUT PARAMETERS:


RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 MP2Stream::skipNSyncSamples(int nSyncSamplesToSkip,
                                 uint32 id,
                                 bool *bError,
                                 uint64 currentPosTimeStamp)
{
  uint64 newTS = 0;

  bool hasError = false;
  uint8 index = MapTrackIdToIndex(&hasError,id);

  if((!hasError) && (index < FILE_MAX_MEDIA_STREAMS))
  {
    mp2_stream_sample_info sampleInfo;

    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                   "skipNSyncSamples id %lu nSyncSamplesToSkip %d", id,nSyncSamplesToSkip);
    if(bError && m_pMP2StreamParser)
    {
      *bError = true;
      MP2StreamStatus retStatus = MP2STREAM_DEFAULT_ERROR;
      retStatus = m_pMP2StreamParser->SkipNSyncSamples(id,&sampleInfo,nSyncSamplesToSkip);
      if( (retStatus == MP2STREAM_SUCCESS) || (retStatus == MP2STREAM_DATA_UNDER_RUN))
      {
        MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_HIGH,
          "m_pMP2StreamParser->SkipNSyncSamples Successful #sample %lu size %lu ntime %llu noffset %llu",
                   sampleInfo.nsample,sampleInfo.nsize,sampleInfo.ntime,sampleInfo.noffset);
        if(retStatus == MP2STREAM_DATA_UNDER_RUN)
        {
          _fileErrorCode = PARSER_ErrorDataUnderRun;
          *bError = true;
        }
        else
        {
          *bError = false;
          _fileErrorCode = PARSER_ErrorNone;
        }
        track_type ttype = TRACK_TYPE_UNKNOWN;
        media_codec_type medtype = UNKNOWN_AUDIO_VIDEO_CODEC;
        retStatus = m_pMP2StreamParser->GetTrackType(id,&ttype,&medtype);

        if(retStatus == MP2STREAM_SUCCESS)
        {
          m_sampleInfo[index].num_frames = 1;
          if( (ttype == TRACK_TYPE_AUDIO) && (medtype == AUDIO_CODEC_AAC))
          {
            m_pMP2StreamParser->SetAudioSeekRefPTS(sampleInfo.ntime);
          }
          m_sampleInfo[index].time =  (uint64)sampleInfo.ntime;
          m_sampleInfo[index].sample =  sampleInfo.nsample;
          m_sampleInfo[index].size = sampleInfo.nsize;
          m_sampleInfo[index].sync = 0;
          newTS = m_sampleInfo[index].time;
        }
        else
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,"GetTrackType failed, retStatus %d",id);
        }
      }
      else
      {
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                   "skipNSyncSamples id %d nSyncSamplesToSkip %d FAILED!!", id,nSyncSamplesToSkip);
        *bError = true;
        _fileErrorCode = PARSER_ErrorSeekFail;
      }
    }
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,"!!!MapTrackIdToIndex failed for trackid %d!!!",id);
  }
  return newTS;
}
#endif
/* ======================================================================
FUNCTION:
  MP2Stream::resetPlayback

DESCRIPTION:
  resets the playback time/sample to zero.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/

/* ======================================================================
FUNCTION:
  MP2Stream::resetPlayback

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
/* ======================================================================
FUNCTION:
  MP2Stream::resetPlayback

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
uint64 MP2Stream::resetPlayback(uint64 repos_time,
                                uint32 id,
                                bool /*bSetToSyncSample*/,
                                bool *bError,
                                uint64 currentPosTimeStamp)
{
  uint64 newTS = 0;

  bool hasError = false;
  uint8 index = MapTrackIdToIndex(&hasError,id);

  if((!hasError) && (index < FILE_MAX_MEDIA_STREAMS))
  {
    //Use current time for each track instead of common
    uint64 currTimeForTrack = m_sampleInfo[index].time;
    currentPosTimeStamp = currTimeForTrack;

    bool bforward = (repos_time > currentPosTimeStamp)?1:0;
    mp2_stream_sample_info sampleInfo;

    MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
                   "resetPlayback id %lu repos_time %llu currentPosTimeStamp %llu",
                   id,repos_time,currentPosTimeStamp);
    if(bError && m_pMP2StreamParser)
    {
      *bError = true;
      MP2StreamStatus retStatus = MP2STREAM_DEFAULT_ERROR;
      retStatus = m_pMP2StreamParser->Seek(id,repos_time,
                                              currentPosTimeStamp,
                                              &sampleInfo,bforward,false,0);
      if( (retStatus == MP2STREAM_SUCCESS) || (retStatus == MP2STREAM_DATA_UNDER_RUN))
      {
        MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_HIGH,
          "m_pMP2StreamParser->Seek Successful #sample %lu size %lu ntime %f noffset %llu",
                   sampleInfo.nsample,sampleInfo.nsize,sampleInfo.ntime,sampleInfo.noffset);
        if(retStatus == MP2STREAM_DATA_UNDER_RUN)
        {
          _fileErrorCode = PARSER_ErrorDataUnderRun;
          *bError = true;
        }
        else
        {
          *bError = false;
          _fileErrorCode = PARSER_ErrorNone;
        }
        track_type ttype = TRACK_TYPE_UNKNOWN;
        media_codec_type medtype = UNKNOWN_AUDIO_VIDEO_CODEC;
        retStatus = m_pMP2StreamParser->GetTrackType(id,&ttype,&medtype);

        if(retStatus == MP2STREAM_SUCCESS)
        {
          m_sampleInfo[index].num_frames = 1;
          if( (ttype == TRACK_TYPE_AUDIO) && (medtype == AUDIO_CODEC_AAC))
          {
            m_pMP2StreamParser->SetAudioSeekRefPTS(sampleInfo.ntime);
          }
          m_sampleInfo[index].time =  (uint64)sampleInfo.ntime;
          m_sampleInfo[index].sample =  sampleInfo.nsample;
          m_sampleInfo[index].size = sampleInfo.nsize;
          m_sampleInfo[index].sync = 0;
          newTS = m_sampleInfo[index].time;
        }
        else
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,"GetTrackType failed, retStatus %lu",id);
        }
      }
      else
      {
        MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_HIGH,
                   "resetPlayback id %lu repos_time %llu currentPosTimeStamp %llu Failed!!! retStatus=%d",
                   id,repos_time,currentPosTimeStamp,retStatus);
        *bError = true;
        _fileErrorCode = PARSER_ErrorSeekFail;
      }
    }
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,"!!!MapTrackIdToIndex failed for trackid %lu !!!",id);
  }
  return newTS;
}
/* ======================================================================
FUNCTION:
  MP2Stream::resetPlayback

DESCRIPTION:
  resets the playback time to zero for a track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/


/* ======================================================================
FUNCTION:
  MP2Stream::randomAccessDenied

DESCRIPTION:
  gets if repositioning is allowed or not.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/


/* ======================================================================
FUNCTION:
  MP2Stream::getTitle

DESCRIPTION:
  gets title

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/


/* ======================================================================
FUNCTION:
  MP2Stream::getAuthor

DESCRIPTION:
  gets Author

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/


/* ======================================================================
FUNCTION:
  MP2Stream::getDescription

DESCRIPTION:
  gets description

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/


/* ======================================================================
FUNCTION:
  MP2Stream::getRating

DESCRIPTION:
  gets rating

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/


/* ======================================================================
FUNCTION:
  MP2Stream::getCopyright

DESCRIPTION:
  gets Copyright

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/


/* ======================================================================
FUNCTION:
  MP2Stream::getVersion

DESCRIPTION:
  gets version

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/


/* ======================================================================
FUNCTION:
  MP2Stream::getCreationDate

DESCRIPTION:
  gets creation date

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
/* ======================================================================
FUNCTION:
  MP2Stream::getBaseTime

DESCRIPTION:
  gets base time of the clip.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
bool MP2Stream::getBaseTime(uint32 id, uint64* nBaseTime)
{
  bool nRet = false;
  *nBaseTime = 0;
  double baseTime = 0;
  if(m_pMP2StreamParser)
  {
    nRet = m_pMP2StreamParser->GetBaseTime(id, &baseTime);
    *nBaseTime = (uint64)baseTime;
  }
  return nRet;
}
/* ======================================================================
FUNCTION:
  MP2Stream::setBaseTime

DESCRIPTION:
  sets base time of the clip.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
bool MP2Stream::setBaseTime(uint32 id,uint64 nBaseTime)
{
  bool bRet = false;
  if(m_pMP2StreamParser)
  {
    bRet = m_pMP2StreamParser->SetBaseTime(id, (double)nBaseTime);
  }
  return bRet;
}

/* ======================================================================
FUNCTION:
  MP2Stream::getMovieDurationMsec

DESCRIPTION:
  gets movie duration in milli secs

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 MP2Stream::getMovieDurationMsec() const
{
  uint64 nDuration = 0;
  if(!m_pMP2StreamParser)
  {
    return nDuration;
  }
  nDuration = m_pMP2StreamParser->GetClipDurationInMsec();
  return nDuration;
}

/* ======================================================================
FUNCTION:
  AVIFile::getTrackWholeIDList

DESCRIPTION:
  gets list of track IDs

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 MP2Stream::getTrackWholeIDList(uint32 *ids)
{

  if(!m_pMP2StreamParser)
  {
    return 0;
  }
  return (m_pMP2StreamParser->GetTrackWholeIDList(ids));
}

/* ======================================================================
FUNCTION:
  MP2Stream::getTrackVideoFrameRate

DESCRIPTION:
  gets track video (if video) frame rate

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
float MP2Stream::getTrackVideoFrameRate(uint32 id)
{
  float frate = 0.0;
  if(m_pMP2StreamParser)
  {
    frate = m_pMP2StreamParser->GetVideoFrameRate(id);
  }
  return frate;
}
/* ======================================================================
FUNCTION:
  MP2Stream::getTrackVideoFrameWidth

DESCRIPTION:
  returns video track's frame width.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 MP2Stream::getTrackVideoFrameWidth(uint32 id)
{
  uint32 width = 0;
  if(m_pMP2StreamParser)
  {
    width = m_pMP2StreamParser->GetVideoWidth(id);
  }
  return width;
}

/* ======================================================================
FUNCTION:
  AVIFile::getTrackVideoFrameHeight

DESCRIPTION:
  returns video track's frame height.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 MP2Stream::getTrackVideoFrameHeight(uint32 id)
{
  uint32 height = 0;
  if(m_pMP2StreamParser)
  {
    height = m_pMP2StreamParser->GetVideoHeight(id);
  }
  return height;
}

/* ======================================================================
FUNCTION:
  MP2Stream::getTrackAudioSamplingFreq

DESCRIPTION:
  gets audio track's sampling frequency

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 MP2Stream::getTrackAudioSamplingFreq(uint32 ulTrackId)
{
  uint32 ulSampFreq = 0;
  if(m_pMP2StreamParser)
  {
    audio_info sAudioInfo;
    bool bStatus = m_pMP2StreamParser->GetAudioInfo(ulTrackId, &sAudioInfo);
    if (bStatus)
    {
      ulSampFreq = sAudioInfo.SamplingFrequency;
    }
  }
  return ulSampFreq;
}

/* ======================================================================
FUNCTION:
  MP2Stream::GetNumAudioChannels

DESCRIPTION:
  returns number of audio channels

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 MP2Stream::GetNumAudioChannels(int ulTrackId)
{
  uint32 ulNumChannels = 0;
  if(m_pMP2StreamParser)
  {
    audio_info sAudioInfo;
    bool bStatus = m_pMP2StreamParser->GetAudioInfo(ulTrackId, &sAudioInfo);
    if (bStatus)
    {
      ulNumChannels = sAudioInfo.NumberOfChannels;
    }
  }
  return ulNumChannels;
}

/* ======================================================================
FUNCTION:
  MP2Stream::peekCurSample

DESCRIPTION:
  gets information about current sample of a track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE MP2Stream::peekCurSample(uint32 trackid, file_sample_info_type *pSampleInfo)
{
  PARSER_ERRORTYPE reterror = PARSER_ErrorDefault;
  if((!m_pMP2StreamParser) || (!pSampleInfo))
  {
    reterror = PARSER_ErrorDefault;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"MP2Stream::peekCurSample invalid argument");
  }
  else
  {
    bool bError = false;
    uint8 index = MapTrackIdToIndex(&bError,trackid);
    if(!bError)
    {
      *pSampleInfo = m_sampleInfo[index];
      reterror = PARSER_ErrorNone;
    }
  }
  return reterror;
}
/* ======================================================================
FUNCTION:
  MP2Stream::getTrackOTIType

DESCRIPTION:
  gets track OTI value

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
/* ======================================================================
FUNCTION:
  AVIFile::getTrackOTIType

DESCRIPTION:
  gets track OTI value

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 MP2Stream::getTrackOTIType(uint32 id)
{
  uint8 format = 0xFF;
  track_type cType;
  media_codec_type codec_type;

  if(m_pMP2StreamParser)
  {
    if(m_pMP2StreamParser->GetTrackType(id,&cType,&codec_type) == MP2STREAM_SUCCESS)
    {
      switch(cType)
      {
        case TRACK_TYPE_AUDIO:
        {
          if((codec_type == AUDIO_CODEC_AC3) ||
              (codec_type == AUDIO_CODEC_EAC3))
          {
            format = (uint8)AC3_AUDIO;
          }
          else if(codec_type == AUDIO_CODEC_AAC)
          {
            if(m_pMP2StreamParser->GetAACAudioFormat(id) == AAC_FORMAT_ADTS)
            {
              format = (uint8)AAC_ADTS_AUDIO;
            }
            else
            {
              format = (uint8)MPEG4_AUDIO;
            }
          }
          else if(codec_type == AUDIO_CODEC_MP3)
          {
            format = (uint8)MP3_AUDIO;
          }
          else if((codec_type == AUDIO_CODEC_LPCM) ||
                  (codec_type == AUDIO_CODEC_HDMV_LPCM))
          {
            format = (uint8)PCM_AUDIO;
          }
          else if(codec_type == AUDIO_CODEC_MPEG2)
          {
            format = (uint8)MP2_AUDIO;
          }
          else if(codec_type == AUDIO_CODEC_DTS)
          {
            format = (uint8)DTS_AUDIO;
          }
        }
        break;
        case TRACK_TYPE_VIDEO:
        {
          if (m_pMP2StreamParser->IsMPeg1Video())
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                         " MPEG1 Video is not supported");
          }
          else if((codec_type == VIDEO_CODEC_MPEG2))
          {
            format = (uint8)MPEG2_VIDEO;
          }
          else if(codec_type == VIDEO_CODEC_H264)
          {
            format = (uint8)H264_VIDEO;
          }
          else if(codec_type == VIDEO_CODEC_VC1)
          {
            format = (uint8)VC1_VIDEO;
          }
          else if(codec_type == VIDEO_CODEC_MPEG4)
          {
            format = (uint8)MPEG4_VIDEO;
          }
        }
        break;
        default:
        break;
      }
    }
  }
  return format;
}

/* ======================================================================
FUNCTION:
  MP2Stream::getTrackAudioFormat

DESCRIPTION:
  gets track audio format based on VideoFMT enum

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/


/* ======================================================================
FUNCTION:
MP2Stream::getLargestFrameSize

DESCRIPTION:
  gets the largest frame size in the given track
INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/


/* ======================================================================
FUNCTION:
  MP2Stream::getFramesPerSample

DESCRIPTION:
  gets the number frames per sample of given track in video_fmt_stream_audio_subtype type.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/


/* ======================================================================
FUNCTION:
  MP2Stream::getTotalNumberOfFrames

DESCRIPTION:
  gets total number of frames in a given track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/



/* ======================================================================
FUNCTION:
  MP2Stream::getTrackMaxBufferSizeDB

DESCRIPTION:
  gets maximum buffer size to play the track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
int32  MP2Stream::getTrackMaxBufferSizeDB(uint32 id)
{
  track_type type;
  int32 bufsize = 0;
  media_codec_type codec_type;
  if(!m_pMP2StreamParser)
  {
    return 0;
  }
  if(m_pMP2StreamParser->GetTrackType(id,&type,&codec_type)==MP2STREAM_SUCCESS)
  {
    switch(type)
    {
      case TRACK_TYPE_AUDIO:
      {
        bufsize = DEFAULT_AUDIO_BUFF_SIZE;
        if(m_audioLargestSize)
        {
          bufsize = (int32)m_audioLargestSize;
        }
        break;
      }

      case TRACK_TYPE_VIDEO:
      {
          bufsize = DEFAULT_VIDEO_BUFF_SIZE;
          if(m_videoLargestSize)
          {
            bufsize = (int32)m_videoLargestSize;
          }
          else
          {
            uint32 height = getTrackVideoFrameWidth(id);
            uint32 width = getTrackVideoFrameHeight(id);
            if(height && width)
            {
              bufsize = int32(height * width);

              //! If height X Width is less than 720P resolution
              //! Double the buffer size requirement
              if(height <= 720 && width <= 576)
              {
                bufsize <<= 1;
              }
            }
          }

          break;
      }
      default:
        break;
    }
  }
  return bufsize;
}
/* ======================================================================
FUNCTION:
  MP2Stream::getTrackAverageBitrate

DESCRIPTION:
  gets track's average bitrate

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 average bitrate

SIDE EFFECTS:
  None.
======================================================================*/
int32 MP2Stream::getTrackAverageBitrate(uint32 id)
{
  uint32 bitrate = 0;
  if(m_pMP2StreamParser)
  {
    bitrate = m_pMP2StreamParser->GetTrackAverageBitRate(id);
  }
  return bitrate;
}

/* ======================================================================
 FUNCTION:
   MP2Stream::GetTotalNumberOfAudioStreams()

 DESCRIPTION:
   returns total number of audio streams.

 INPUT/OUTPUT PARAMETERS:
   None.

 RETURN VALUE:
  none

 SIDE EFFECTS:
   None.
 ======================================================================*/
int MP2Stream::GetTotalNumberOfAudioStreams()
{
  int ntracks = 0;
  if(m_pMP2StreamParser)
  {
    ntracks = (int)m_pMP2StreamParser->GetTotalNumberOfAudioTracks();
  }
  return ntracks;
}
 /* ======================================================================
 FUNCTION:
   MP2Stream::GetTotalNumberOfVideoStreams()

 DESCRIPTION:
   returns total number of video streams.

 INPUT/OUTPUT PARAMETERS:
   None.

 RETURN VALUE:
  none

 SIDE EFFECTS:
   None.
 ======================================================================*/
int MP2Stream::GetTotalNumberOfVideoStreams()
{
  int ntracks = 0;
  if(m_pMP2StreamParser)
  {
    ntracks = (int)m_pMP2StreamParser->GetTotalNumberOfVideoTracks();
  }
  return ntracks;
}
 /* ======================================================================
 FUNCTION:
   MP2Stream::GetTotalNumberOfTextStreams()

 DESCRIPTION:
   returns total number of text streams.

 INPUT/OUTPUT PARAMETERS:
   None.

 RETURN VALUE:
  none

 SIDE EFFECTS:
   None.
 ======================================================================*/
int MP2Stream::GetTotalNumberOfTextStreams()
{
  int ntracks = 0;
  return ntracks;
}
/* ======================================================================
FUNCTION:
  MP2Stream::getAllowAudioOnly

DESCRIPTION:
  gets if audio only playback is allowed.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/


/* ======================================================================
FUNCTION:
  MP2Stream::getAllowVideoOnly

DESCRIPTION:
  gets if video only playback is allowed.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/

/* ======================================================================
FUNCTION:
  MP2Stream::getTrackDecoderSpecificInfoContent

DESCRIPTION:
  this returns Sequence (VOL) Header/size for given track id

INPUT/OUTPUT PARAMETERS:
  @in id: Track identifier
  @in buf: Buffer to copy VOL header
  @in/@out: pbufSize: Size of VOL header
  When buf is NULL, function returns the size of VOL header into pbufSize.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/

/* ======================================================================
FUNCTION:
  MP2Stream::getTrackDecoderSpecificInfoContent

DESCRIPTION:
  this returns Sequence (VOL) Header.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/

PARSER_ERRORTYPE MP2Stream::getTrackDecoderSpecificInfoContent(uint32 id, uint8* buf, uint32 *pbufSize)
{
  PARSER_ERRORTYPE retvalue = PARSER_ErrorDefault;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "getTrackDecoderSpecificInfoContent");
  track_type cType;
  media_codec_type codec_type;

  if((m_pMP2StreamParser) && ((m_pMP2StreamParser->GetTrackType(id,&cType,&codec_type)) == MP2STREAM_SUCCESS) && pbufSize)
  {
    if (codec_type== VIDEO_CODEC_H264)
    {
      *pbufSize = 0;
    }
    else if(codec_type== AUDIO_CODEC_AAC)
    {
      if(MP2STREAM_SUCCESS == m_pMP2StreamParser->GetTrackDecoderSpecificInfoContent(id, buf,pbufSize) )
      {
        retvalue = PARSER_ErrorNone;
      }
    }
  }
  return retvalue;
}

/* ======================================================================
FUNCTION:
  MP2Stream::getTrackDecoderSpecificInfoSize

DESCRIPTION:
  this returns Sequence (VOL) Header size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/

/* ======================================================================
FUNCTION:
  MP2Stream::SetTimeStampedSample

DESCRIPTION:
  gets closest sample's info of the closest frame of given time.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
/* ======================================================================
FUNCTION:
  MP2Stream::MapTrackIdToIndex

DESCRIPTION:
  Maps given track-id to track index.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 MP2Stream::MapTrackIdToIndex(bool* bError,uint32 trackid)
{
  uint8 index = 0;
  if(bError)
  {
    *bError = true;
    for(uint32 i = 0; i < m_nNumStreams; i++)
    {
      if( (m_pIndTrackIdTable[i].trackId == trackid) &&
        (m_pIndTrackIdTable[i].bValid) )
      {
        index = m_pIndTrackIdTable[i].index;
        *bError = false;
        break;
      }
    }

    if(*bError)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,"MP2Stream::MapTrackIdToIndex failed for trackid %lu",trackid);
    }
  }
  return index;
}
/* ======================================================================
FUNCTION:
  MP2Stream::GetAACAudioProfile

DESCRIPTION:
  Returns AAC audio profile.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 MP2Stream::GetAACAudioProfile(uint32 ulTrackId)
{
  uint32 ulAudioObject = 0;
  if(m_pMP2StreamParser)
  {
    audio_info sAudioInfo;
    bool bStatus = m_pMP2StreamParser->GetAudioInfo(ulTrackId, &sAudioInfo);
    if (bStatus)
    {
      ulAudioObject = sAudioInfo.AudioObjectType;
    }
  }
  return ulAudioObject;
}

/* ======================================================================
FUNCTION:
  MP2Stream::GetAACAudioFormat

DESCRIPTION:
  returns AAC Audio format

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 MP2Stream::GetAACAudioFormat(uint32 ulTrackId)
{
  uint32 ulAudioFormat = AAC_FORMAT_UNKNOWN;
  if(m_pMP2StreamParser)
  {
    audio_info sAudioInfo;
    bool bStatus = m_pMP2StreamParser->GetAudioInfo(ulTrackId, &sAudioInfo);
    if (bStatus && AUDIO_CODEC_AAC == sAudioInfo.Audio_Codec )
    {
      ulAudioFormat = AAC_FORMAT_ADTS;
    }
  }
  return ulAudioFormat;
}

#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)

/*===========================================================================

FUNCTION  updateBufferWritePtr

DESCRIPTION
  Public method used to update the write buffer offset during Http streaming.

===========================================================================*/
void MP2Stream::updateBufferWritePtr ( uint64 writeOffset )
{
  m_wBufferOffset = writeOffset;
}

///*===========================================================================
//
//FUNCTION  getMetaDataSize
//
//DESCRIPTION
//  Public method used to determine the meta-data size of the fragment.
//
//===========================================================================*/
//tWMCDecStatus MP2Stream::getMetaDataSize ( void )
//{
//  tWMCDecStatus wmerr = WMCDec_Fail;
//  uint32 nHttpDownLoadBufferOffset = 0;
//  boolean bHttpDownLoadBufferOffsetValid = GetHTTPStreamDownLoadedBufferOffset(&nHttpDownLoadBufferOffset);
//  WMFDecoderEx *pDecoder = (WMFDecoderEx *)m_hASFDecoder;
//  U32_WMC nAsfHeaderSize = 0;
//
//  if( pDecoder && bHttpDownLoadBufferOffsetValid && (nHttpDownLoadBufferOffset > (MIN_OBJECT_SIZE + sizeof(U32_WMC) + 2*sizeof(U8_WMC)) ) )
//  {
//    wmerr = GetAsfHeaderSize(&m_hASFDecoder,&nAsfHeaderSize );
//  }
//  if(wmerr == WMCDec_Succeeded)
//  {
//    m_HttpDataBufferMinOffsetRequired.Offset = nAsfHeaderSize;
//    m_HttpDataBufferMinOffsetRequired.bValid = TRUE;
//    bGetMetaDataSize = FALSE;
//    return wmerr;
//  }
//  else
//  {
//    bGetMetaDataSize = TRUE;
//    return wmerr;
//  }
//
//  return WMCDec_Fail;
//}
//
/*===========================================================================

FUNCTION  parseHTTPStream

DESCRIPTION
  Public method used to parse the Http Stream.

===========================================================================*/
bool MP2Stream::parseHTTPStream ( void )
{
  bool returnStatus = false;

  if(!m_playAudio && !m_playVideo)
  {
    return true;
  }

  m_minOffsetRequired = TS_PKT_SIZE;
  if((m_wBufferOffset >= m_minOffsetRequired)
    && m_wBufferOffset && m_minOffsetRequired)
  {
    //Parse the fragment here
    if(!ParseMetaData())
    {
      parserState = PARSER_PAUSE;
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Parser State = PARSER_PAUSE, m_playVideo=%d",m_playVideo);
      returnStatus = false;
    }

    if ( ((parserState == PARSER_RESUME)||(parserState == PARSER_READY)) )
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Parser State = PARSER_READY, m_playVideo=%d",m_playVideo);
      parserState = PARSER_READY;
      returnStatus = true;
    }
    else
    {
      returnStatus = false;
    }
  }
  else
  {
    parserState = PARSER_PAUSE;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "Parser State = PARSER_PAUSE, m_playVideo=%d",m_playVideo);
    return false;
  }

  return returnStatus;
}

/* ======================================================================
FUNCTION:
  MP2Stream::IsDRMProtection

DESCRIPTION:
  returns clip DRM protection status

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  TRUE   Clip DRM protected
  FALSE  Clip not protected by any DRM scheme

SIDE EFFECTS:
  None.
======================================================================*/
bool MP2Stream::IsDRMProtection()
{
  if(!m_pMP2StreamParser)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "MP2Stream::IsDRMProtection Error,returning FALSE");
    return false;
  }
  return m_pMP2StreamParser->IsDRMProtection();
}

/* =============================================================================
FUNCTION:
 ASFFile::GetDRMType

DESCRIPTION:
Returns DRM scheme used by underlying file, if any.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
FileSourceStatus MP2Stream::GetDRMType(FileSourceDrmType& drmtype)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "MP2Stream::GetDRMType");
  drmtype = FILE_SOURCE_NO_DRM;
  if(IsDRMProtection())
  {
    //We only support HDCP DRM as of now. As HDCP DRM has two
    //version HDCP2.0 & HDCP2.1, but at media level there is no
    //way to get this information, so making type as FILE_SOURCE_HDCP_DRM
    drmtype = FILE_SOURCE_HDCP_DRM;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "MP2Stream::DRM Type FILE_SOURCE_HDCP_DRM");
  }
  return FILE_SOURCE_SUCCESS;
}
///*===========================================================================
//
//FUNCTION  sendHTTPStreamUnderrunEvent
//
//DESCRIPTION
//  Public method used to switch contexts and notify the player about buffer-underrun.
//
//===========================================================================*/
//void MP2Stream::sendHTTPStreamUnderrunEvent(void)
//{
//  QTV_HTTP_STREAM_BUFFER_UNDERRUN_EVENT_type *pEvent = QCCreateMessage(QTV_HTTP_STREAM_BUFFER_UNDERRUN_EVENT, m_pMpeg4Player);
//
//  if (pEvent)
//  {
//    pEvent->bAudio = (bool) m_playAudio;
//    pEvent->bVideo = (bool) m_playVideo;
//    pEvent->bText = (bool) m_playText;
//    QCUtils::PostMessage(pEvent, 0, NULL);
//  }
//}
///*===========================================================================
//
//FUNCTION  GetHTTPStreamDownLoadedBufferOffset
//
//DESCRIPTION
//  Public method used to switch contexts and notify the player about buffer-underrun.
//
//===========================================================================*/
//boolean MP2Stream::GetHTTPStreamDownLoadedBufferOffset(U32_WMC * pOffset)
//{
//  WMFDecoderEx *pDecoder = (WMFDecoderEx *)m_hASFDecoder;
//
//  if(pDecoder && pOffset)
//  {
//    if(m_fpFetchBufferedDataSize)
//    {
//      //Pull interface so pull dnld data size from OEM
//      m_fpFetchBufferedDataSize( 0, &(pDecoder->wHttpDataBuffer.Offset, m_QtvInstancehandle) );
//      pDecoder->wHttpDataBuffer.bValid = TRUE;
//    }
//    if( pDecoder->wHttpDataBuffer.bValid )
//    {
//      *pOffset = pDecoder->wHttpDataBuffer.Offset;
//      return TRUE;
//    }
//  }
//  return FALSE;
//}
//

/*! =======================================================================
@brief        Returns absolute file offset(in bytes) associated
              with time stamp 'pbtime'(in milliseconds).

@param[in]    pbtime:Timestamp(in milliseconds) of the sample
                       that user wants to play/seek
              ulTrackId: The track ID for which API is called
@param[out]   pullOffset:Absolute file offset(in bytes) corresponding
              to 'pbtime'

@return       true if successful in retrieving the absolute
              offset(in bytes) else returns false

@note         It is expected that user has received the successul
              callback for OpenFile() earlier. When there are
              multiple tracks in a given clip(audio/video/text),
              API returns minimum absolute offset(in bytes)among
              audio/video/text tracks. API is valid only for
              non-fragmented clips.
==========================================================================  */
bool MP2Stream::GetOffsetForTime(uint64 ullPBTime, uint64* pullOffset,
                                 uint32 ulTrackId,
                                 uint64 /*ullCurrentPosTimeStamp*/,
                                 uint64& /*pullReposTime*/)

{
  bool bRet = false;
  uint64 ullStartOffset=0,ullStartOffset1=0;
  uint64 ullEndOffset=0,ullEndOffset1=0;
  uint64 ullPTS2=0,ullPTS1=0;
  uint64 ullStartPos=0;
  uint64 ullTempCurrOffset = m_pMP2StreamParser->GetCurrOffset();

  if(!m_bHaveTransportRate)
  {
    m_pMP2StreamParser->GetPTSFromNextPES(ulTrackId,
      ullStartPos, &ullStartOffset, &ullEndOffset, &ullPTS1);
    ullStartPos = ullEndOffset;
    while(ullPTS2 <= ullPTS1)
    {
      m_pMP2StreamParser->GetPTSFromNextPES(ulTrackId,
        ullStartPos, &ullStartOffset1, &ullEndOffset1, &ullPTS2);
      ullStartPos = ullEndOffset1;
    }
    fpTransportRate = (float)(((ullStartOffset1 - ullStartOffset)*1000)/
                             (ullPTS2 - ullPTS1));
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "fpTransportRate=%f Bytes/Second", fpTransportRate);
    m_bHaveTransportRate = true;
  }
  if(m_bHaveTransportRate)
  {
    // We have transport ratio already
    *pullOffset = (uint64)(((uint64)fpTransportRate*ullPBTime)/1000);
    if(m_fileSize && (*pullOffset > m_fileSize))
    {
        *pullOffset = m_fileSize;
    }
    bRet = true;
  }
  // Restore m_nCurrOffset to its initial value
  m_pMP2StreamParser->SetCurrOffset(ullTempCurrOffset);

  return bRet;
}
/*! =======================================================================
@brief    Provides the playback time for buffered/downloaded bytes of
          particular track.

@param[in]      nTrackID  The track ID.
@param[in]      nBytes    Buffered/Downloaded bytes (Always provide
                          positive integer)
@param[out]     pDuration Filled in by FileSource to provided playtime
                for buffered/downloaded bytes
@return         true  if successful in retrieving buffered duration
                else false.

@note         It is expected that user has received the successul
              callback for OpenFile() earlier. When there are
              multiple tracks in a given clip(audio/video/text),
              API returns minimum absolute offset(in bytes)among
              audio/video/text tracks. API is valid only for
              non-fragmented clips.
==========================================================================  */
bool MP2Stream::getBufferedDuration(uint32 nTrackID, int64 nBytes,
    uint64 *pDuration)
{
  bool bRet = false;
  MP2StreamStatus eRetError = MP2STREAM_FAIL;

  eRetError = m_pMP2StreamParser->GetPTSFromLastPES(nTrackID,
      (uint64)nBytes, pDuration);
  if(eRetError == MP2STREAM_SUCCESS)
  {
    bRet = true;
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
        "getBufferedDuration Failed eRetError=%d", eRetError);
  }
  return bRet;
}
/*===========================================================================

FUNCTION  CanPlayTracks

DESCRIPTION
  Public method used to switch contexts and notify the player about buffer-underrun.

===========================================================================*/
//bool MP2Stream::CanPlayTracks(uint32 nTotalPBTime)
//{
//    return true;
//}

/* ======================================================================
FUNCTION:
  MP2Stream::GetMediaMaxTimeStampPlayable

DESCRIPTION:
  gets time stamp of current sample of the track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 time stamp in track time scale unit

SIDE EFFECTS:
  None.
======================================================================*/
//tWMCDecStatus MP2Stream::GetMediaMaxPlayableTime(U32_WMC *nMaxPBTime)
//{
//  uint32 nMaxPlayableTime = 0;  // default max playable time
//
//  if( (m_pStreamDecodePattern == NULL) || (nMaxPBTime == NULL) )
//  {
//    return WMCDec_InValidArguments;
//  }
//  for(uint16 i=0; i<(int)m_nNumStreams; i++)
//  {
//    if( m_maxPlayableTime[i] && (m_pStreamDecodePattern[i].tPattern != Discard_WMC) )
//    {
//      if(!nMaxPlayableTime)
//      {
//        /* initialize with valid track sample time */
//        nMaxPlayableTime = m_maxPlayableTime[i];
//        continue;
//      }
//      /* Take the MIN value to make sure all tracks are playable atleast nMaxPlayableTime */
//      nMaxPlayableTime = MIN(m_maxPlayableTime[i],nMaxPlayableTime);
//    }
//  }
//
//    *nMaxPBTime = nMaxPlayableTime;
//
//  return WMCDec_Succeeded;
//}
#endif //  FEATURE_QTV_3GPP_PROGRESSIVE_DNLD
