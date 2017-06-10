/* =======================================================================
                              mp4FragmentFile.cpp
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

  Copyright (c) 2008-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/mp4fragmentfile.cpp#72 $
$DateTime: 2013/09/19 20:43:24 $
$Change: 4465512 $


========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "parserdatadef.h"
#include "parserinternaldefs.h"

#include "AEEStdDef.h"              /* Definitions for byte, word, etc.        */
#include "MMDebugMsg.h"

#include "mp4fragmentfile.h"
#ifdef FEATURE_FILESOURCE_DRM_DCF
 #include "IxStream.h"
#endif

//Define following feature to have lots of debug messages when doing inter fragment repositioning.
//#define FEATURE_INTER_FRAGMENT_REPOS_DEBUG
/*===========================================================================

FUNCTION  Mp4FragmentFile

DESCRIPTION
  This is the Mp4FragmentFile class constructor - when executed is parses and caches
  the meta-data of a fragmented file just opened.

===========================================================================*/
Mp4FragmentFile::Mp4FragmentFile( FILESOURCE_STRING filename,
                                  unsigned char *pFileBuf,
                                  uint32 bufSize,
                                  bool bPlayVideo,
                                  bool bPlayAudio,
                                  bool bPlayText
#if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || \
    defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
                                  ,bool bPseudoStream
                                  ,uint32 wBufferOffset
#endif /* defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) */
                                )
                :Mpeg4File(filename,
                           pFileBuf,
                           bufSize,
                           bPlayVideo,
                           bPlayAudio,
                           bPlayText
#if defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || \
    defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
                           ,bPseudoStream
                           ,wBufferOffset
#endif /* defined (FEATURE_FILESOURCE_PSEUDO_STREAM) || defined (FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) */
                           )
{
  Mp4FragmentFile::InitData();
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
/* ======================================================================
FUNCTION:
  Mp4FragmentFile::Mp4FragmentFile

DESCRIPTION:
  constructor for supporting playback from StreamPort.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
Mp4FragmentFile::Mp4FragmentFile( video::iStreamPort* pPort,
                                  bool bPlayVideo, bool bPlayAudio,
                                  bool bPlayText,
                                  FileSourceFileFormat eFileFormat):
Mpeg4File(pPort, bPlayVideo, bPlayAudio, bPlayText, eFileFormat)
{

  Mp4FragmentFile::InitData();
  m_eFileFormat = eFileFormat;
  if(_success)
  {
     #ifdef FEATURE_FILESOURCE_PSEUDO_STREAM
        if(pPort)
        {
          int64 nDownloadedBytes = 0;
          pPort->GetAvailableOffset(&nDownloadedBytes, &bEndOfData);
          m_bufferOffset = (uint32)nDownloadedBytes;
        }
     #endif
  }
}
#endif
#ifdef FEATURE_FILESOURCE_DRM_DCF
/*===========================================================================

FUNCTION
  Mp4FragmentFile::Mp4FragmentFile

DESCRIPTION
  Constructor for creating mp4fragment file instance for DCF media

DEPENDENCIES
  None

INPUT PARAMETERS:
->inputStream:IxStream*
->bPlayVideo:Indicates if this is video instance
->bPlayAudio:Indicates if this is audio instance
->bPlayText:Indicates if this is text instance

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
Mp4FragmentFile::Mp4FragmentFile(  IxStream* inputStream,
                                   bool bPlayVideo,
                                   bool bPlayAudio,
                                   bool bPlayText
                                   )
                :Mpeg4File(inputStream,
                           bPlayVideo,
                           bPlayAudio,
                           bPlayText)
{
  InitData();
}
#endif

/*===========================================================================

FUNCTION  InitData

DESCRIPTION
  Initialize the members
===========================================================================*/
void Mp4FragmentFile::InitData()
{

  fragmentinfoCount      = 0;
  m_eFileFormat          = FILE_SOURCE_3G2;

  if(_success)
  {
    /*Make room initially for fragmentInfoArray to hold entires for a DEFAULTFRAGMENTCOUNT(5000) through malloc
      This would avoid numerous realloc of fragmentInfoArray during the playback of a large fragmented file
      that may lead to heap fragmentation*/
    int32 retstat = fragmentInfoArray.MakeRoomFor(DEFAULTFRAGMENTCOUNT);
    if(retstat == -1)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                 "Can't allocate memory for initial fragmentInfoArray" );
      _fileErrorCode = PARSER_ErrorMemAllocFail;
    }
  }
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
  m_mp4InitialParseStatus = VIDEO_FMT_STATUS_INVALID;
#endif
}

 /*===========================================================================

FUNCTION  getMovieDuration

DESCRIPTION
  Public method to parse the first fragment inside the MP4 file.

===========================================================================*/
uint64 Mp4FragmentFile::getMovieDuration() const
{
  uint64 Duration     = 0;
  uint64 DurationInMS = 0;

  /* Converting duration into milli-sec units. */
  if(m_isFragmentedFile)
  {
    Duration = m_videoFmtInfo.file_info.fragment_file_total_movie_duration;
  }
  else
  {
    Duration = m_videoFmtInfo.file_info.total_movie_duration;
  }

  DurationInMS = uint64((Duration * MILLISEC_TIMESCALE_UNIT) /
                        m_videoFmtInfo.file_info.movie_timescale);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "Movie Duration value in MS is %llu",
              DurationInMS);
  return DurationInMS;
}

#ifdef FEATURE_FILESOURCE_PSEUDO_STREAM
/*===========================================================================

FUNCTION  parsePseudoStreamLocal

DESCRIPTION
  Public method used to parse the fragment

===========================================================================*/
bool Mp4FragmentFile::parsePseudoStreamLocal ( void )
{
  bool returnStatus = true;
  uint32 numFragment = m_currentParseFragment;
  m_currentParseFragment = 0;
  m_minOffsetRequired = 0;

  for(uint32 i = 0; i <= numFragment; i++)
  {
    m_currentParseFragment = i;
    returnStatus = getMetaDataSize();
  }

  if(returnStatus)
  {
    if((m_wBufferOffset >= m_minOffsetRequired)
        && m_wBufferOffset && m_minOffsetRequired)
    {
       if(!ParseStream())
       {
          returnStatus = false;
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "parsePseudoStreamLocal: parsing failed for fragNo=%lu,\
                   m_mp4ParseLastStatus=%d",
                   (m_currentParseFragment-1),m_mp4ParseLastStatus);
       }
    }
    else
    {
        returnStatus = false;
    }
  }
  else
  {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "parsePseudoStreamLocal: getFragmentSize() failed for \
                   fragNo=%lu, m_mp4ParseLastStatus=%d",
          m_currentParseFragment,m_mp4ParseLastStatus);
  }
  return returnStatus;
}
#endif /* #ifdef FEATURE_FILESOURCE_PSEUDO_STREAM */
/*===========================================================================

FUNCTION  getParseFragmentNum

DESCRIPTION
  Public method used send parser events

===========================================================================*/
uint16 Mp4FragmentFile::getParseFragmentNum( void )
{
    return (uint16)fragmentNumber;
}

/*===========================================================================

FUNCTION  getReadFragmentNum

DESCRIPTION
  Public method used send parser events

===========================================================================*/
uint16 Mp4FragmentFile::getReadFragmentNum( void )
{
    return (uint16)fragmentNumber;
}

uint16 Mp4FragmentFile::m_minTfraRewindLimit = QTV_MPEG4_MIN_TFRA_REW_LIMIT;
/* ======================================================================
FUNCTION
  Mp4FragmentFile::repositionAccessPoint

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
uint64 Mp4FragmentFile::repositionAccessPoint (int32 skipNumber, uint32 id, bool &bError ,uint64 currentPosTimeStampMsec)
{
  bError = FALSE;
  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);
  video_fmt_mp4r_context_type  *context;

  if ( !p_track )
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                 "Mp4Frag::repositionAccessPoint, unknown track id = %lu", id);
    bError = TRUE;
    return 0;
  }

  context = (video_fmt_mp4r_context_type *) (m_videoFmtInfo.server_data);
  if ( !(context->mfra_present) || ((p_track->track_id)!= context->tfra.track_id))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "Mp4Frag::repositionAccessPoint, mfra atom is not present");
    bError = TRUE;
    return 0;
  }

  uint32 streamNum = p_track->stream_num;

  video_fmt_sample_info_type  sampleInfo;
  memset(&sampleInfo, 0x0, sizeof(video_fmt_sample_info_type));

  uint64 modTS; // initial value

  if ( getAccessPointSampleInfo (p_track, skipNumber, &sampleInfo, &modTS,currentPosTimeStampMsec) )
  {
    if ( m_nextSample[streamNum] == sampleInfo.sample )
    {
      // this is to avoid unnecessary disturbance, when no repositioning is needed
      return modTS;
    }

    m_reposStreamPending |= maskByte[streamNum];
    m_nextReposSample[streamNum] = sampleInfo.sample;
    m_sampleInfo[streamNum] = sampleInfo;
    return modTS;
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
               "Mp4Frag::repositionAccessPoint failed for track id = %lu", id);
    // this is to avoid unnecessary disturbance, when repositioning can not be done
    uint64 ullTimeStamp;
    ullTimeStamp = (m_sampleInfo[streamNum].time*1000)/p_track->media_timescale;
    bError = TRUE;
    return ullTimeStamp;
  }
}

/* <EJECT> */
/*===========================================================================

FUNCTION  getAccessPointSampleInfo

DESCRIPTION
  Public method used to get the sample corresponding to a given timestamp.

===========================================================================*/
bool Mp4FragmentFile::getAccessPointSampleInfo(video_fmt_stream_info_type *p_track,
                                               int32                       skipNumber,
                                               video_fmt_sample_info_type *sampleInfo,
                                               uint64                     *newTimeStamp,
                                               uint64                     currentPosTimeStampMsec)
{
  boolean fragmentRepositioned = FALSE;

  uint32 streamNum = p_track->stream_num;

  uint64 reqSampleNum = 0;
  bool  iRewind = false;
  bool retStat = false;
  //get the Random Access point info
  video_fmt_tfra_entry_type   tfraEntry;
  *newTimeStamp = 0;
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;

  uint64 currentPosTimeStamp = (currentPosTimeStampMsec*p_track->media_timescale)/
                                1000;
  if ( skipNumber < 0 )
  {
    iRewind = true;
  }

  if(iRewind)
  {
    if(p_track->fragment_number == 0)
    {
      /*Rewind back to 0th frame*/
      retStat = false;
      retError = getSampleInfo(streamNum, 0, 1, sampleInfo);
      if(PARSER_ErrorNone == retError)
      {
        retStat = true;
      }
      if(!retStat)
      {
         MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
         "getAccesPointSampleInfo: getSampleInfo(0th Sample) failed..!");
      }
      return retStat;
    }

    do
    {
      retStat = getTfraEntryInfo(streamNum, skipNumber, iRewind, &tfraEntry,currentPosTimeStamp);
      if(retStat)
      {
        uint64 tfraTimeMsec = (tfraEntry.access_point_time * 1000) /
                              p_track->media_timescale;

        if(tfraTimeMsec + m_minTfraRewindLimit <= currentPosTimeStampMsec)
        {
          break;
        }
        else
        {
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
      "getAccesPointSampleInfo: Rew to a sync pt < %llu ,currPlayBack=%llu..!",
                tfraTimeMsec,currentPosTimeStampMsec);
          currentPosTimeStamp = tfraEntry.access_point_time-1;
        }
      }
    } while(retStat);

    if((m_iodoneSize[streamNum] == 0) &&
        (m_mp4SyncLastStatus[streamNum] == VIDEO_FMT_IO_DONE))
    {
      /*You have reached the first tfra table entry while rewinding,
        so go back to the 0th frame*/
      retStat = getTimestampedSampleInfo(p_track,0,sampleInfo,newTimeStamp,TRUE,
                                         currentPosTimeStampMsec);
      return retStat;
    }
  }
  else
  {
    retStat = getTfraEntryInfo(streamNum, skipNumber, iRewind, &tfraEntry,currentPosTimeStamp);
  }
  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM, "getAccesPointSampleInfo: iRewind=%d, m_sampleInfo.time=%llu, tfraEntry.time=%llu",
            iRewind, m_sampleInfo[streamNum].time, tfraEntry.access_point_time);

  if(!retStat)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "getAccesPointSampleInfo: getTfraEntryInfo failed..!");
    return false;
  }

  retStat = findSampleFromTfra(p_track, iRewind, reqSampleNum, &tfraEntry,
                                 fragmentRepositioned);
  if(!retStat)
  {
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "getAccesPointSampleInfo: findSampleFromTfra failed..!");
     // if the inter fragment repositioning failed and the fragment has been repositioned
     //Now reposition to the Original Fragment from where it started//
     (void)findiFrameFragment(p_track, m_sampleInfo[streamNum].sample,!iRewind, FALSE, fragmentRepositioned);
     return false;
  }

  retStat = false;
  retError = getSampleInfo(streamNum, reqSampleNum, 1, sampleInfo);
  if(PARSER_ErrorNone == retError)
  {
    retStat = true;
    *newTimeStamp = (sampleInfo->time * 1000)/ p_track->media_timescale;
    if((!iRewind && (*newTimeStamp < currentPosTimeStampMsec))||(iRewind && (*newTimeStamp > currentPosTimeStampMsec)))
    {
      if(fragmentRepositioned)
      {
        // if the inter fragment repositioning failed and the fragment has been repositioned
        //Now reposition to the Original Fragment from where it started//
        iRewind = (m_sampleInfo[streamNum].sample < reqSampleNum)?true:false;
        (void)findiFrameFragment(p_track, m_sampleInfo[streamNum].sample, iRewind, FALSE, fragmentRepositioned);
      }
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "WRONG getAccesPointSampleInfo: getSampleInfo failed..!");

      return false;
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "getAccesPointSampleInfo: getSampleInfo failed..!");
  }

  return retStat;
}


/*===========================================================================

FUNCTION  getTfraEntryInfo

DESCRIPTION
  Public method used to request a media sample (frame)

===========================================================================*/
bool Mp4FragmentFile::getTfraEntryInfo (uint32 streamNum,
                                        int32  skipNum,
                                        bool   reverse,
                                        video_fmt_tfra_entry_type *buffer,
                                        uint64  currentPosTimeStamp)
{
  int loop = 0;
  uint64 sample_timestamp = currentPosTimeStamp;

  m_videoFmtInfo.access_point_cb( streamNum,
                                  sample_timestamp,
                                  skipNum,
                                  reverse,
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
             "Mp4Frag::getTfraEntryInfo VideoFMT hangs."
             "StreamNum=%lu, skipNum %lu", streamNum, skipNum);
  }

  if( (m_mp4SyncLastStatus[streamNum] == VIDEO_FMT_FAILURE) ||
      (m_mp4SyncLastStatus[streamNum] == VIDEO_FMT_BUSY) ||
      (m_mp4SyncLastStatus[streamNum] == VIDEO_FMT_DATA_CORRUPT))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                 "Mp4Frag::getTfraEntryInfo VideoFMT failed.");
    return FALSE;
  }

  if(m_iodoneSize[streamNum] == 0)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "Mp4Frag::getTfraEntryInfo could not find a sample.");
    return FALSE;
  }
  return TRUE;
}

/*===========================================================================

FUNCTION  findSampleFromTfra

DESCRIPTION
  Public method used to get the sample corresponding to a given timestamp.

===========================================================================*/
bool Mp4FragmentFile::findSampleFromTfra(video_fmt_stream_info_type *input_track,
                                         bool                       iRewind,
                                         uint64                     &reqSampleNum,
                                         video_fmt_tfra_entry_type  *tfraEntry,
                                         boolean                    &fragmentRepositioned)
{
  int32 length = 0, index = 0;
  uint32 i = 0, input_streamnum;
  fragment_info_type *fragment_info = NULL;
  boolean foundFragment = FALSE, continueParsing = TRUE;
  bool returnValue = false, parseReturn = false;
  video_fmt_stream_info_type  *p_stream_info = NULL, *track = NULL;
  video_fmt_mp4r_stream_type  *p_stream = NULL;

  locateStreamData(&p_stream_info,&p_stream,input_track);

  if(!p_stream_info || !p_stream)
     return false;

  input_streamnum = input_track->stream_num;
  fragmentRepositioned = FALSE;

  if(iRewind)
  {
    if((tfraEntry->access_point_time >= input_track->track_frag_info.first_timestamp) &&
       (tfraEntry->access_point_time < input_track->media_duration))
    {
      // The sample is in the same fragment
      reqSampleNum = 0;

      fragment_info = fragmentInfoArray[input_track->fragment_number-1];
      reqSampleNum = fragment_info->frames[input_streamnum];

      if(reqSampleNum > 0)
        --reqSampleNum;
      for(i = 0 ;i < (uint32)(tfraEntry->trun_number - 1); i++)
      {
        reqSampleNum += p_stream->trun[i].table_size;
      }
      reqSampleNum += tfraEntry->sample_number;
      returnValue = true;
    }
    else
    {
      //The required sample is in a different fragment
      length = fragmentInfoArray.GetLength();
      index = length - 1;
      while( TRUE )
      {
        fragment_info = fragmentInfoArray[index];
        /*tfraEntry will allways point to some fragment other than the
        main fragment.*/
        if(fragment_info->timestamp[input_streamnum] <= tfraEntry->access_point_time)
        {
          fragment_info = fragmentInfoArray[index + 1];
          foundFragment = TRUE;
          break;
        }
        index--;
        if(index < 0)
          break;
      }
      fragmentRepositioned = TRUE;
      if(foundFragment == FALSE)
          return false;

      fragment_info = fragmentInfoArray[index];
      reqSampleNum  =  fragment_info->frames[input_streamnum];
      reinitializeFragmentData(input_track, index, reqSampleNum, iRewind);

      if(reqSampleNum > 0)
        --reqSampleNum;


#ifdef FEATURE_FILESOURCE_PSEUDO_STREAM
      if(bHttpStreaming)
      {
        parseReturn = parsePseudoStreamLocal();
      }
      else
#endif /* FEATURE_FILESOURCE_PSEUDO_STREAM */
      {
        parseReturn = parseMetaData();
        if( (parseReturn) && (m_currentParseFragment > 0) )
        {
          //Decrement the current fragment count because we successfully parsed the fragment
          m_currentParseFragment -= 1;
        }
      }
      if(parseReturn)
      {
        //QCUtils::EnterCritSect(&m_trackwrite_CS);
        for(i = 0; i < m_trackCount; i++)
        {
          track = m_track[i];
          locateStreamData(&p_stream_info,&p_stream,track);
          p_stream->fragment_repositioned = TRUE;
        }

        fragmentRepositioned = TRUE;
        returnValue = true;
        //QCUtils::LeaveCritSect(&m_trackwrite_CS);
        locateStreamData(&p_stream_info,&p_stream,input_track);
        for(i = 0 ;i < (uint32)(tfraEntry->trun_number - 1); i++)
        {
          reqSampleNum += p_stream->trun[i].table_size;
        }
        reqSampleNum += tfraEntry->sample_number;
      }
      else
      {
        returnValue = false;
        fragmentRepositioned = FALSE;
      }
    }
   //end of REW
  }
  else
  {
    //FFWD
    if((tfraEntry->access_point_time >= input_track->track_frag_info.first_timestamp) &&
       (tfraEntry->access_point_time < input_track->media_duration))
    {
      // The sample is in the same fragment
      reqSampleNum = 0;
      fragment_info = fragmentInfoArray[input_track->fragment_number-1];
      reqSampleNum = fragment_info->frames[input_streamnum];

      if(reqSampleNum > 0)
        --reqSampleNum;
      for(i = 0 ;i < (uint32)(tfraEntry->trun_number - 1); i++)
      {
        reqSampleNum += p_stream->trun[i].table_size;
      }
      reqSampleNum += tfraEntry->sample_number;
      returnValue = true;
    }
    else
    {
      if(!setMainFragmentBytes())
      {
        return FALSE;
      }

      boolean bDoParse = TRUE;
      //required sample is in a different fragment
      //parse and reposition till the end of the file
      while( TRUE )
      {
        length = fragmentInfoArray.GetLength();
        for(index = 0; index < length; index++)
        {
          fragment_info = fragmentInfoArray[index];
          if(fragment_info->timestamp[input_streamnum] > tfraEntry->access_point_time)
          {
            foundFragment = TRUE;
            continueParsing = FALSE;
            break;
          }
          else
          {
            continueParsing = TRUE;
            foundFragment = FALSE;
          }
        }
        if(continueParsing && !m_parsedEndofFile)
        {
          bDoParse = FALSE;
#ifdef FEATURE_FILESOURCE_PSEUDO_STREAM
          if(bHttpStreaming)
          {
            m_currentParseFragment = fragment_info->fragment_number + 1;
            parseReturn = parsePseudoStreamLocal();
          }
          else
#endif /*FEATURE_FILESOURCE_PSEUDO_STREAM*/
          {
            parseReturn = parseMetaData();
            fragmentRepositioned = TRUE;
            if(parseReturn)
            {
              //Increment the current fragment count because we successfully parsed the fragment
              m_currentParseFragment += 1;
            }
          }
          if(parseReturn != true)
          {
            fragmentRepositioned = FALSE;
            break;
          }
        }
        else
          break;
      }

      if(foundFragment == FALSE)
        return false;

      //Fragment with I Frame exists
      if(!bDoParse)
      {
        uint32 stream_num = 0;
        for(i = 0; i < m_trackCount; i++)
        {
          track = m_track[i];
          stream_num = track->stream_num;
          locateStreamData(&p_stream_info, &p_stream, track);
          reinitializeFragmentStream(p_stream, fragment_info, index,
                                     stream_num, iRewind);
        }
      }
      else
      {
        reinitializeFragmentData(input_track, index, reqSampleNum, iRewind);
#ifdef FEATURE_FILESOURCE_PSEUDO_STREAM
        if(bHttpStreaming)
        {
          m_currentParseFragment = fragment_info->fragment_number;
          parseReturn = parsePseudoStreamLocal();
        }
        else
#endif /*FEATURE_FILESOURCE_PSEUDO_STREAM*/
        {
          parseReturn = parseMetaData();
          if(parseReturn)
          {
             //Increment the current fragment count because we successfully parsed the fragment
             m_currentParseFragment += 1;
          }
        }
        if(!parseReturn)
        {
          fragmentRepositioned  = FALSE;
          return FALSE;
        }
      }

      fragment_info = fragmentInfoArray[index-1];
      reqSampleNum = fragment_info->frames[input_streamnum];

      if(reqSampleNum > 0)
        --reqSampleNum;

      for(i = 0; i < m_trackCount; i++)
      {
        track = m_track[i];
        locateStreamData(&p_stream_info,&p_stream,track);
        p_stream->fragment_repositioned = TRUE;
      }

      fragmentRepositioned = TRUE;
      returnValue = true;
      //QCUtils::LeaveCritSect(&m_trackwrite_CS);
      locateStreamData(&p_stream_info,&p_stream,input_track);
      for(i = 0 ;i < (uint32)(tfraEntry->trun_number - 1); i++)
      {
        reqSampleNum += p_stream->trun[i].table_size;
      }
      reqSampleNum += tfraEntry->sample_number;
    }
    //end of FFWD
  }
  return returnValue;
}

/*===========================================================================

FUNCTION  setMainFragmentBytes

DESCRIPTION
   Private method used set the Main fragment bytes as stsz was not previously
   parsed.

===========================================================================*/
boolean Mp4FragmentFile::setMainFragmentBytes()
{
  for(uint8 trackIndex = 0; trackIndex < m_trackCount; trackIndex++)
  {
    video_fmt_stream_info_type  *p_stream_info = NULL;
    video_fmt_mp4r_stream_type  *p_stream = NULL;
    video_fmt_stream_info_type* track = m_track[trackIndex];

    if( (track->type == VIDEO_FMT_STREAM_VIDEO && m_playVideo) ||
        (track->type == VIDEO_FMT_STREAM_AUDIO && m_playAudio) ||
        (track->type == VIDEO_FMT_STREAM_TEXT && m_playText)
        )
    {
      locateStreamData(&p_stream_info,&p_stream,track);
      if(!p_stream)
      {
        return FALSE;
      }
      //! If main fragment does not contain any frames, do no call
      if(!p_stream->main_fragment_bytes && !m_bIsDashClip &&
         p_stream->main_fragment_frames)
      {
        video_fmt_sample_info_type  lastSampleInfo;
        PARSER_ERRORTYPE retError = getSampleInfo (track->stream_num,
                                                   p_stream->main_fragment_frames-1,
                                                   1, &lastSampleInfo);
        if(PARSER_ErrorNone != retError)
        {
          return FALSE;
        }
        track->bytes = lastSampleInfo.offset + lastSampleInfo.size;
        p_stream->main_fragment_bytes = track->bytes;
        fragmentInfoArray[(uint8)0]->bytes[track->stream_num] = track->bytes;
      }
    }
  }
  return TRUE;
}

/*===========================================================================

FUNCTION  processFragmentBoundary

DESCRIPTION
  Public method used to request a media sample (frame)

===========================================================================*/
PARSER_ERRORTYPE Mp4FragmentFile::processFragmentBoundary(video_fmt_stream_info_type *track)
{
  PARSER_ERRORTYPE  returnVal = PARSER_ErrorDefault;
  uint32 streamNum = track->stream_num;

  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "Mp4FragmentFile::processFragmentBoundary");

  if(!setMainFragmentBytes())
  {
    return PARSER_ErrorEndOfFile;
  }

  if (parseUntilSampleFound(track))
  {
    returnVal = getSampleInfo (streamNum, m_nextSample[streamNum], 1,
                               &m_sampleInfo[streamNum]);
    return returnVal;
  }
  else
  {
    if(m_parsedEndofFile)
    {
       return PARSER_ErrorEndOfFile;
    }
    else
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "Mp4FragmentFile::processFragmentBoundary A:%d V:%d",
        m_playAudio, m_playVideo);
      return PARSER_ErrorDataFragment;
    }
  }
}

/*===========================================================================

FUNCTION  parseUntilSampleFound

DESCRIPTION
  Public method used to switch contexts and call the parseFragment event

===========================================================================*/
boolean Mp4FragmentFile::parseUntilSampleFound (video_fmt_stream_info_type *track)
{
  if(!m_parsedEndofFile)
  {
#if defined( FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) || defined(FEATURE_FILESOURCE_PSEUDO_STREAM)
    if(bHttpStreaming)
    {
      //keep parsing the next fragment (if present) until
      //u find audio/video/text frame || u reach the EndOfFile
      //If u dont have sufficeint data to parse then pauseTrack.

      if(track->type == VIDEO_FMT_STREAM_AUDIO)
      {
        do
        {
          //Handling a special case when u had to pause: when
          //u knew the fragment size but did not have sufficient data
          //to continue fragment parsing.
          if(m_mp4ParseLastStatus == VIDEO_FMT_FRAGMENT_SIZE)
            bGetMetaDataSize = FALSE;
          else
            bGetMetaDataSize = TRUE;

          if(!ParseStream())
          {
            m_hasAudio = false;
            if((m_wBufferOffset < m_minOffsetRequired) ||
               (m_wBufferOffset >= m_minOffsetRequired && bDataIncomplete))
            {
              sendParserEvent(PARSER_PAUSE);
            }
            break;
          }
        }while(!m_hasAudio && !m_parsedEndofFile);

        return m_hasAudio;

      }
      else if(track->type == VIDEO_FMT_STREAM_VIDEO)
      {
        do
        {
          if(m_mp4ParseLastStatus == VIDEO_FMT_FRAGMENT_SIZE)
            bGetMetaDataSize = FALSE;
          else
            bGetMetaDataSize = TRUE;

          if(!ParseStream())
          {
            m_hasVideo = false;
            if((m_wBufferOffset < m_minOffsetRequired) ||
               (m_wBufferOffset >= m_minOffsetRequired && bDataIncomplete))
            {
              sendParserEvent(PARSER_PAUSE);
            }
            break;
          }
        }while(!m_hasVideo && !m_parsedEndofFile);

        return m_hasVideo;
      }
      else if(track->type == VIDEO_FMT_STREAM_TEXT)
      {
        do
        {
          if(m_mp4ParseLastStatus == VIDEO_FMT_FRAGMENT_SIZE)
            bGetMetaDataSize = FALSE;
          else
            bGetMetaDataSize = TRUE;

          if(!ParseStream())
          {
            m_hasText = false;
            //Pause the track..insufficient download data
            if((m_wBufferOffset < m_minOffsetRequired) ||
               (m_wBufferOffset >= m_minOffsetRequired && bDataIncomplete))
            {
              sendParserEvent(PARSER_PAUSE);
            }
            break;
          }
        }while(!m_hasText && !m_parsedEndofFile);

        return m_hasText;
      }
    }
    else
#endif /*( FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) || defined(FEATURE_FILESOURCE_PSEUDO_STREAM)*/
    {
      //keep parsing the next fragment until u find
      //audio/video/text frame || u reach the EndOfFile
      if(track->type == VIDEO_FMT_STREAM_AUDIO)
      {
        do
        {
          if(!ParseStream())
          {
            m_hasAudio = false;
            break;
          }
        }while(!m_hasAudio && !m_parsedEndofFile);

        return m_hasAudio;
      }
      else if (track->type == VIDEO_FMT_STREAM_VIDEO)
      {
        do
        {
          if(!ParseStream())
            {
              m_hasVideo = false;
              break;
            }
        }while(!m_hasVideo && !m_parsedEndofFile);

        return m_hasVideo;
      }
      else if (track->type == VIDEO_FMT_STREAM_TEXT)
      {
        do
        {
          if(!ParseStream())
          {
            m_hasText = false;
            break;
          }
        }while(!m_hasText && !m_parsedEndofFile);

        return m_hasText;
      }
    }
  }//!m_parsedEndofFile
  return FALSE;
  //If u return FALSE and if(!m_parsedEndofFile) then the calling thread will come
  //againg after COMMON_FRAGMENT_WAIT_TIMEOUT_MSEC, to check if a new sample is available.
}

/*===========================================================================

FUNCTION  locateStreamData

DESCRIPTION
  Method used to find the stream context

===========================================================================*/
void Mp4FragmentFile::locateStreamData(
                video_fmt_stream_info_type  **p_stream_info,
                video_fmt_mp4r_stream_type  **p_stream,
                video_fmt_stream_info_type  *input_track)
{
  video_fmt_mp4r_context_type *video_fmt_context = NULL;
  uint32 i = 0;

  video_fmt_context = (video_fmt_mp4r_context_type *)m_videoFmtInfo.server_data;

  if(video_fmt_context)
  {
    for(i = 0; i < m_videoFmtInfo.num_streams; i++)
    {
      if((input_track->track_id == video_fmt_context->stream_info[i].track_id)
          && (input_track->stream_num == video_fmt_context->stream_info[i].stream_num)
          && (input_track->type == video_fmt_context->stream_info[i].type))
      {
        *p_stream_info = video_fmt_context->stream_info + i;
        *p_stream = video_fmt_context->stream_state + i;
        break;
      }
    }
  }
}

/* ============================================================================
  @brief  Finds the sample number in the next or previous fragments.

  @details    This function is used to find the fragment number at either start
              of next fragment or the last sample number in previous fragment.
              Once we know sample number in immediate fragment, it will be easy
              to go to that fragment using API "findiFrameFragment".

  @param[in]      pInTrack            Pointer to track properties structure.
  @param[in/out]  rullReqSampleNum    Requested sample number.
  @param[in]      bRewind             Flag to indicate forward or backward.

  @return     True or False depending on whether it is successful or not.
  @note       None.
============================================================================ */
bool Mp4FragmentFile::FindKeyFrameFragmentIndex(video_fmt_stream_info_type *pInTrack,
                                                uint64 &rullReqSampleNum,
                                                bool    bRewind)
{
  int32 ulFragArrayLen = 0, ulIndex = 0;
  uint32 ulStreamNum = 0;
  fragment_info_type *pFragInfo = NULL;
  bool bFoundFragment = false;

  if(!m_isFragmentedFile || !pInTrack)
    return false;

  ulStreamNum = pInTrack->stream_num;

  if(bRewind)
  {
    ulFragArrayLen = fragmentInfoArray.GetLength();
    ulIndex = ulFragArrayLen - 1;
    while( false == bFoundFragment )
    {
      pFragInfo = fragmentInfoArray[ulIndex];
      if (pFragInfo == NULL)
      {
        break;
      }
      if((pFragInfo->fragment_number == 0) &&
        ((rullReqSampleNum + 1) <= pFragInfo->frames[ulStreamNum]))
      {
        rullReqSampleNum  = pFragInfo->frames[ulStreamNum] - 1;
        bFoundFragment = true;
        break;
      }
      else if(pFragInfo->frames[ulStreamNum] < (rullReqSampleNum + 1))
      {
        if((pFragInfo->fragment_i_frame[ulStreamNum]))
        {
          rullReqSampleNum  = pFragInfo->frames[ulStreamNum] - 1;
          bFoundFragment = true;
        }
        else if(fragmentInfoArray[ulIndex]->fragment_number == 0)
        {
          rullReqSampleNum  = pFragInfo->frames[ulStreamNum] - 1;
          bFoundFragment = true;
        }
        if(bFoundFragment)
          break;
      }
      ulIndex--;
      if(ulIndex < 0)
        break;
    }
    if(bFoundFragment == FALSE)
      return false;
  }
  else
  {
    //FFWD
    if(!setMainFragmentBytes())
    {
      return false;
    }

    ulFragArrayLen = fragmentInfoArray.GetLength();
    //parse and reposition till the end of the file
    while(false == bFoundFragment)
    {
      for(ulIndex = 0; ulIndex < ulFragArrayLen; ulIndex++)
      {
        pFragInfo = fragmentInfoArray[ulIndex];
        if((pFragInfo) &&
           (pFragInfo->frames[ulStreamNum] >= (rullReqSampleNum + 1)))
        {
          rullReqSampleNum  = pFragInfo->frames[ulStreamNum];
          bFoundFragment = true;
          break;
        }
      }
      if ((pFragInfo) && (false == bFoundFragment))
      {
        rullReqSampleNum  = pFragInfo->frames[ulStreamNum];
        break;
      }
    }
  }
  return bFoundFragment;
}

/*===========================================================================

FUNCTION  findiFrameFragment

DESCRIPTION
  Public method used to get the sample corresponding to a given timestamp.

===========================================================================*/
bool Mp4FragmentFile::findiFrameFragment(video_fmt_stream_info_type *input_track,
                                         uint64                     reqSampleNum,
                                         bool                       iRewind,
                                         boolean                    findiFrame,
                                         boolean                    &fragmentParsed)
{
  int32 length = 0, index = 0;
  uint32 i = 0, input_streamnum;
  fragment_info_type *fragment_info = NULL;
  boolean foundFragment = FALSE, continueParsing = TRUE;
  bool returnValue = false, parseReturn = false;
  video_fmt_stream_info_type  *p_stream_info = NULL, *track = NULL;
  video_fmt_mp4r_stream_type  *p_stream = NULL;

  if(!m_isFragmentedFile)
   return false;

  locateStreamData(&p_stream_info,&p_stream,input_track);

  if(!p_stream_info || !p_stream)
   return false;

  input_streamnum = input_track->stream_num;
  fragmentParsed = FALSE;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  /* There can be a scenario, where data has been downloaded but not updated
     in class variable. The following check will ensure no such case arises.
  */
  if(bHttpStreaming && m_pStreamPort)
  {
    video_fmt_mp4r_context_type *context = (video_fmt_mp4r_context_type *) m_videoFmtInfo.server_data;
    video_fmt_mp4r_stream_type  *stream = &context->stream_state [input_streamnum];

     bool bEndOfData = false;
     int64 wBufferOffset = 0;
    //Pull interface so pull download data size from OEM
    m_pStreamPort->GetAvailableOffset((int64*)&wBufferOffset, &bEndOfData);
    m_wBufferOffset = wBufferOffset;
    m_bEndOfData = bEndOfData;
    stream->wBufferOffset = m_wBufferOffset;
  }
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

  if(iRewind)
  {
    length = fragmentInfoArray.GetLength();
    index = length - 1;
    while( TRUE )
    {
      fragment_info = fragmentInfoArray[index];
      if((fragment_info->fragment_number == 0) &&
        ((reqSampleNum + 1) <= fragment_info->frames[input_streamnum]))
      {
        if(fragment_info->fragment_i_frame[input_streamnum] || !findiFrame)
          foundFragment = TRUE;
        break;
      }
      else if(fragment_info->frames[input_streamnum] < (reqSampleNum + 1))
      {
        fragment_info = fragmentInfoArray[index + 1];
        if((fragment_info) && (fragment_info->fragment_i_frame[input_streamnum] || !findiFrame))
        {
          foundFragment = TRUE;
        }
        else if(fragmentInfoArray[index]->fragment_number == 0)
        {
          reqSampleNum = 0;
          if(fragmentInfoArray[index]->fragment_i_frame[input_streamnum] || !findiFrame)
            foundFragment = TRUE;
        }
        if(foundFragment)
          break;
      }
      index--;
      if(index < 0)
        break;
    }
    if(foundFragment == FALSE)
      return false;

    reinitializeFragmentData(input_track, index, reqSampleNum, iRewind);

#ifdef FEATURE_FILESOURCE_PSEUDO_STREAM
    if(bHttpStreaming)
    {
      parseReturn = parsePseudoStreamLocal();
    }
    else
#endif /*FEATURE_FILESOURCE_PSEUDO_STREAM*/
    {
      parseReturn = parseMetaData();
      if(parseReturn)
      {
        //! Even in Rewind operation scenario, Parser will consumre data in
        //! forward direction only. "m_currentParseFragment" is pointing to the
        //! current fragment number. Increase the counter so that next fragment
        //! will be consumed after the current fragment is completed.
        m_currentParseFragment += 1;
      }
    }
    if(parseReturn)
    {
      fragmentParsed = TRUE;
      for(i = 0; i < m_trackCount; i++)
      {
        track = m_track[i];
        locateStreamData(&p_stream_info,&p_stream,track);
        p_stream->fragment_repositioned = TRUE;
      }
      returnValue = true;
    }
    else
    {
      returnValue = false;
      fragmentParsed = FALSE;
    }
  }
  else
  {
    //FFWD
    if(!setMainFragmentBytes())
    {
        fragmentParsed = FALSE;
        return FALSE;
    }

    boolean bDoParse = TRUE;
    //parse and reposition till the end of the file
    while(TRUE)
    {
      length = fragmentInfoArray.GetLength();
      for(index = 0; index < length; index++)
      {
        fragment_info = fragmentInfoArray[index];
        if(fragment_info->frames[input_streamnum] >= (reqSampleNum + 1))
        {
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "FFWD: fragment_info->frames = %llu reqSampleNum + 1 = %llu",
                  fragment_info->frames[input_streamnum], (reqSampleNum + 1));
          if((fragment_info->fragment_i_frame[input_streamnum]) || (!findiFrame))
          {
             foundFragment = TRUE;
             continueParsing = FALSE;
          }
          else
          {
              continueParsing = TRUE;
              reqSampleNum = fragment_info->frames[input_streamnum];
          }
          break;
        }
        else
        {
          continueParsing = TRUE;
          foundFragment = FALSE;
        }
      }

      if(continueParsing && !m_parsedEndofFile)
      {
        bDoParse = FALSE;
#ifdef FEATURE_FILESOURCE_PSEUDO_STREAM
        if(bHttpStreaming)
        {
          m_currentParseFragment = fragment_info->fragment_number + 1;
          parseReturn = parsePseudoStreamLocal();
        }
        else
#endif /*FEATURE_FILESOURCE_PSEUDO_STREAM*/
        {
          if(m_bIsDashClip)
          {
            uint64 temp_minOffsetRequired= m_minOffsetRequired;

            /* check for normal data underrun */
            if ( m_wBufferOffset && (m_minOffsetRequired > m_wBufferOffset))
            {
              _fileErrorCode=PARSER_ErrorDataUnderRun;
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Data Underrun");
              return false;
            }

            /* Invoke peekMetaDataSize () with next fragment for getting the
               m_minOffsetRequired value of next fragment. if we have received
               next fragment m_minOffsetRequired value which means parser
               has sufficient data*/
            (void)peekMetaDataSize(m_currentParseFragment + 1);

            if( m_minOffsetRequired >= temp_minOffsetRequired)
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                            "Parser has sufficient data: m_currentParseFragment %lu",
                           m_currentParseFragment);
            }
            else
            {
              /* No updatation on minOffset means parser didnt get any data*/
              _fileErrorCode = PARSER_ErrorSeekUnderRunInFragment;
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                           "Data Underrun during seek, m_currentParseFragment %lu",
                          m_currentParseFragment);
              return false;
            }

            // re-assigning m_minOffsetRequired value
            m_minOffsetRequired = temp_minOffsetRequired;
          }

          parseReturn = parseMetaData();
          if(parseReturn)
          {
            //Increment the current fragment count because we successfully parsed the fragment
            m_currentParseFragment += 1;
          }
        }
        if(parseReturn == true)
        {
          fragmentParsed = TRUE;
          if(m_parsedEndofFile && (input_track->track_frag_info.first_frame!=0) && (reqSampleNum >= input_track->frames) )
          {
            reqSampleNum  = input_track->frames - 1;
          }
        }
        else
        {
          break;
        }
      }
      else
        break;
    }

    if(foundFragment == FALSE)
      return false;

    if(!bDoParse)
    {
      uint32 stream_num = 0;
      for(i = 0; i < m_trackCount; i++)
      {
        track = m_track[i];
        stream_num = track->stream_num;
        locateStreamData(&p_stream_info, &p_stream, track);
        reinitializeFragmentStream(p_stream, fragment_info, index,
                                   stream_num, iRewind);
      }
    }
    //Fragment with I Frame exists
    else
    {
      reinitializeFragmentData(input_track, index, reqSampleNum, iRewind);
#ifdef FEATURE_FILESOURCE_PSEUDO_STREAM
      if(bHttpStreaming)
      {
        m_currentParseFragment = fragment_info->fragment_number;
        parseReturn = parsePseudoStreamLocal();
      }
      else
#endif /*FEATURE_FILESOURCE_PSEUDO_STREAM*/
      {
        parseReturn = parseMetaData();
        if(parseReturn)
        {
          //Increment the current fragment count because we successfully parsed the fragment
          m_currentParseFragment += 1;
        }
      }
      if(!parseReturn)
      {
        fragmentParsed = FALSE;
        return FALSE;
      }
    }

    //QCUtils::EnterCritSect(&m_trackwrite_CS);
    for(i = 0; i < m_trackCount; i++)
    {
      track = m_track[i];
      locateStreamData(&p_stream_info,&p_stream,track);
      p_stream->fragment_repositioned = TRUE;
    }
    returnValue = true;
    //QCUtils::LeaveCritSect(&m_trackwrite_CS);
  }
  return returnValue;
}

/*===========================================================================

FUNCTION  reinitializeFragmentData

DESCRIPTION
  Public method used to get the sample corresponding to a given timestamp.

===========================================================================*/
void Mp4FragmentFile::reinitializeFragmentData(video_fmt_stream_info_type *input_track,
                                               uint32                     fragment_infoindex,
                                               uint64                     reqSampleNum,
                                               bool                       iRewind)
{
  fragment_info_type *fragment_info = NULL, *next_fragment_info = NULL, *previous_fragment_info = NULL;
  uint32 i, stream_num = 0;

  video_fmt_stream_info_type  *p_stream_info = NULL, *temp_track = NULL;
  video_fmt_mp4r_stream_type  *p_stream = NULL;

  video_fmt_mp4r_context_type *video_fmt_context = NULL;

  video_fmt_context = (video_fmt_mp4r_context_type *)m_videoFmtInfo.server_data;
  stream_num = input_track->stream_num;

  if(iRewind)
  {
    fragment_info = fragmentInfoArray[fragment_infoindex];

    if((fragment_info->fragment_number == 0) &&
       ((reqSampleNum + 1) <= fragment_info->frames[stream_num]))
    {
      //Rewind to first Fragment
      video_fmt_context->abs_pos = 0;
      video_fmt_context->in_buffer_size = 0;
      video_fmt_context->num_streams = 0;

      for(i = 0; i < m_trackCount; i++)
      {
        temp_track = m_track[i];
        stream_num = temp_track->stream_num;
        locateStreamData(&p_stream_info, &p_stream, temp_track);
        reinitializeFragmentStream(p_stream, NULL, fragment_infoindex, stream_num, iRewind);
        reinitializeFragmentStreamInfo(p_stream_info, NULL, stream_num);
      }

      //! Make this value equal to main fragment value for all use cases.
      //! After Metadata parsing is completed, increment to the next fragment
      m_currentParseFragment = 0;
    }
    else
    {
      next_fragment_info = fragmentInfoArray[fragment_infoindex + 1];
      if(next_fragment_info)
      {
        video_fmt_context->abs_pos = next_fragment_info->fragment_offset;
      }
      video_fmt_context->in_buffer_size = 0;

      for(i = 0; i < m_trackCount; i++)
      {
        temp_track = m_track[i];
        stream_num = temp_track->stream_num;
        locateStreamData(&p_stream_info, &p_stream, temp_track);
        reinitializeFragmentStream(p_stream, fragment_info, fragment_infoindex,
                                   stream_num, iRewind);
        reinitializeFragmentStreamInfo(p_stream_info, fragment_info, stream_num);
      }
      //! Make this value equal to current fragment value for all use cases.
      //! After Metadata parsing is completed, increment to the next fragment
      if(next_fragment_info)
      {
        m_currentParseFragment = next_fragment_info->fragment_number;
      }
    }
  }
  else
  {
    //FFWD
    fragment_info = fragmentInfoArray[fragment_infoindex];
    previous_fragment_info = fragmentInfoArray[fragment_infoindex - 1];
    if(previous_fragment_info && fragment_info)
    {
      for(i = 0; i < m_trackCount; i++)
      {
        temp_track = m_track[i];
        stream_num = temp_track->stream_num;
        locateStreamData(&p_stream_info, &p_stream, temp_track);
        reinitializeFragmentStream(p_stream, fragment_info, fragment_infoindex,
                                   stream_num, iRewind);
        reinitializeFragmentStreamInfo(p_stream_info, previous_fragment_info, stream_num);
      }
      video_fmt_context->abs_pos = fragment_info->fragment_offset;
      video_fmt_context->in_buffer_size = 0;

      //! Make this value equal to current fragment value for all use cases.
      //! After Metadata parsing is completed, increment to the next fragment
      m_currentParseFragment = fragment_info->fragment_number;
    }
  }
}

/*===========================================================================

FUNCTION  reinitializeFragmentStream

DESCRIPTION
  Public method used to get the sample corresponding to a given timestamp.

===========================================================================*/
void Mp4FragmentFile::reinitializeFragmentStream(video_fmt_mp4r_stream_type  *input_stream,
                                           fragment_info_type          *fragment_info,
                                           uint32                      fragment_infoindex,
                                           uint32                      stream_num,
                                           bool                        iRewind)
{
  fragment_info_type          *previous_fragment_info = NULL;

  input_stream->current_trun = 0;

  if(!fragment_info)
  {
    input_stream->last_fragment_frames = 0;
    input_stream->last_fragment_bytes = 0;
    input_stream->last_fragment_timestamp = 0;
    input_stream->sample_byte_offset = 0;
    input_stream->sample_timestamp = 0;
    input_stream->sample_delta_count = 0;
    input_stream->sample_ctts_offset_count = 0;
    input_stream->last_sample_offset = 0;
    // Need to Initialize the tables because the repositioning
    /* in VideoFMT in main fragment is done by counting down the samples
    */
    input_stream->stsz.current_table_pos = 0;
    input_stream->stts.current_table_pos = 0;
    input_stream->stss.current_table_pos = 0;
    input_stream->stsc_info.current_table_pos = 0;
    input_stream->stsc.current_table_pos = 0;
    input_stream->stco.current_table_pos = 0;
    input_stream->stsz.cache_size = 0;
    input_stream->stts.cache_size = 0;
    input_stream->stss.cache_size = 0;
    input_stream->stsc_info.cache_size = 0;
    input_stream->stsc.cache_size = 0;
    input_stream->stco.cache_size = 0;
  }
  else
  {
    if(iRewind)
    {
      if(fragment_info->fragment_number == 0)
      {
        input_stream->last_fragment_frames = 0;
        input_stream->last_fragment_bytes = 0;
        input_stream->last_fragment_timestamp = 0;
        // Need to Initialize the tables because the repositioning
        /* in VideoFMT in main fragment is done by counting down the samples
        */
        input_stream->stsz.current_table_pos = 0;
        input_stream->stts.current_table_pos = 0;
        input_stream->stss.current_table_pos = 0;
        input_stream->stsc_info.current_table_pos = 0;
        input_stream->stsc.current_table_pos = 0;
        input_stream->stco.current_table_pos = 0;
        input_stream->stsz.cache_size = 0;
        input_stream->stts.cache_size = 0;
        input_stream->stss.cache_size = 0;
        input_stream->stsc_info.cache_size = 0;
        input_stream->stsc.cache_size = 0;
        input_stream->stco.cache_size = 0;
      }
      else
      {
        input_stream->last_fragment_frames = fragment_info->frames[stream_num] -
                      input_stream->main_fragment_frames;
        input_stream->last_fragment_bytes = fragment_info->bytes[stream_num] -
                      input_stream->main_fragment_bytes;
        input_stream->last_fragment_timestamp = fragment_info->timestamp[stream_num] -
                      input_stream->main_fragment_timestamp;
      }

      input_stream->sample_byte_offset = fragment_info->bytes[stream_num];
      input_stream->sample_timestamp = fragment_info->timestamp[stream_num];
      /* Normal fragmented clips will have some samples as part of main fragment.
         So, this field will never be ZERO. But for DASH clips, we will not have any
         data as part of main fragment i.e., MOOV atom. So this defensive check will help
         Parser not to go some unknown state. */
      if(fragment_info->frames[stream_num])
        input_stream->last_sample_offset = fragment_info->frames[stream_num] - 1;
      else
        input_stream->last_sample_offset = 0;
    }
    else
    {
      //FFWD
      if(fragment_info->fragment_number == 0)
      {
        input_stream->last_fragment_frames = 0;
        input_stream->last_fragment_bytes = 0;
        input_stream->last_fragment_timestamp = 0;
        input_stream->sample_byte_offset = 0;
        input_stream->sample_timestamp = 0;
        input_stream->last_sample_offset = 0;
      }
      else
      {
        previous_fragment_info = fragmentInfoArray[fragment_infoindex - 1];
        if(previous_fragment_info)
        {
          if(previous_fragment_info->fragment_number == 0)
          {
            input_stream->last_fragment_frames = 0;
            input_stream->last_fragment_bytes = 0;
            input_stream->last_fragment_timestamp = 0;
          }
          else
          {
            input_stream->last_fragment_frames =  previous_fragment_info->frames[stream_num] -
                      input_stream->main_fragment_frames;
            input_stream->last_fragment_bytes =  previous_fragment_info->bytes[stream_num] -
                      input_stream->main_fragment_bytes;
            input_stream->last_fragment_timestamp =  previous_fragment_info->timestamp[stream_num] -
                      input_stream->main_fragment_timestamp;
          }
          input_stream->sample_byte_offset =
                  previous_fragment_info->bytes[stream_num];
          input_stream->sample_timestamp =
                  previous_fragment_info->timestamp[stream_num];
          input_stream->last_sample_offset = 0;
          if(0 != previous_fragment_info->frames[stream_num])
          {
            input_stream->last_sample_offset =
                  previous_fragment_info->frames[stream_num] - 1;
          }
        }
      }
    } //(iRewind)
  }//(!fragment_info)
}

/*===========================================================================

FUNCTION  reinitializeFragmentStreamInfo

DESCRIPTION
  Public method used to get the sample corresponding to a given timestamp.

===========================================================================*/
void Mp4FragmentFile::reinitializeFragmentStreamInfo(video_fmt_stream_info_type  *input_streaminfo,
                                               fragment_info_type          *fragment_info,
                                                uint32                      stream_num)
{
  if(!fragment_info)
  {
    input_streaminfo->frames = 0;
    input_streaminfo->bytes = 0;
    input_streaminfo->media_duration = 0;
    input_streaminfo->fragment_number = 0;
    input_streaminfo->fragment_offset = 0;
  }
  else
  {
    input_streaminfo->frames = fragment_info->frames[stream_num];
    input_streaminfo->bytes = fragment_info->bytes[stream_num];
    input_streaminfo->media_duration = fragment_info->timestamp[stream_num];
    input_streaminfo->fragment_number = fragment_info->fragment_number;
    input_streaminfo->fragment_offset = fragment_info->fragment_offset;
  }
}

/*===========================================================================

FUNCTION  getSampleInfoError

DESCRIPTION
  Private method called from getNextMediaSample() in the base class,
  when a fragment boundary is reached.

===========================================================================*/

PARSER_ERRORTYPE Mp4FragmentFile::getSampleInfoError(video_fmt_stream_info_type *p_track)
{
  PARSER_ERRORTYPE returnVal = PARSER_ErrorEndOfFile;
  if (m_isFragmentedFile && !m_corruptFile)
  {
    returnVal = processFragmentBoundary(p_track);
  }
  return returnVal;
}

/*===========================================================================

FUNCTION  process_video_fmt_info

DESCRIPTION
  Private method called from mp4ParseStatus() in the base class.

===========================================================================*/
void Mp4FragmentFile::process_video_fmt_info(video_fmt_status_type status,
                                video_fmt_status_cb_info_type *info)
{
  Mpeg4File::process_video_fmt_info(status,info);

  //Adding fragment information.
    video_fmt_stream_info_type *p_track = NULL;
    fragment_info_type *fragment_info = NULL;
    boolean addfraginfo = TRUE, addData = FALSE;
  uint32 index=0;

  for (index = 0; index < m_videoFmtInfo.num_streams; index++)
  {
    p_track = m_videoFmtInfo.streams + index;
    addfraginfo = TRUE;

    if ((p_track != NULL) &&
      (((p_track->type == VIDEO_FMT_STREAM_VIDEO) && m_playVideo) ||
      ((p_track->type == VIDEO_FMT_STREAM_AUDIO) && m_playAudio) ||
      ((p_track->type == VIDEO_FMT_STREAM_TEXT) && m_playText))
      )
    {
      for (uint32 i = 0; i < fragmentinfoCount; i++)
      {
        fragment_info = fragmentInfoArray[i];
        if(p_track->frames == fragment_info->frames[p_track->stream_num])
        {
          addfraginfo = FALSE;
          break;
        }
      }
      if(addfraginfo)
      {
        addData = TRUE;
        fragment_info_type *frag_info_entry =
          (fragment_info_type*)MM_Malloc(sizeof(fragment_info_type));
        if(frag_info_entry == NULL)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
              "Mp4Frag::process_video_fmt_info frag_info_entry malloc fail" );
          _fileErrorCode = PARSER_ErrorMemAllocFail;
          addData = FALSE;
          break;
        }
        memset(frag_info_entry, 0x0, sizeof(fragment_info_type));
        frag_info_entry->fragment_number = p_track->fragment_number;
        frag_info_entry->fragment_offset = p_track->fragment_offset;
        int32 oldsize = fragmentInfoArray.GetLength();
        fragmentInfoArray += frag_info_entry;
        int32 newsize = fragmentInfoArray.GetLength();
        /*Check for the newsize and the oldsize of the fragmentInfoArray after memory reallocarion*/
        if(newsize<=oldsize)
        {
          MM_Free(frag_info_entry);
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
            "Mp4Frag::process_video_fmt_info realloc failed");
          addData = FALSE;
          _fileErrorCode = PARSER_ErrorMemAllocFail;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
            "Mp4Frag::process_video_fmt_info setting malloc fail error ");
          break;
        }
        fragmentinfoCount++;
        break;
      }
    }
  }

  for (index = 0; index < m_videoFmtInfo.num_streams; index++)
  {
    p_track = m_videoFmtInfo.streams + index;
    if (p_track != NULL)
    {
      switch ( p_track->type )
      {
      case VIDEO_FMT_STREAM_VIDEO:
        if(m_playVideo)
        {
          m_hasVideo = false;
          if(addData)
          {
            fragment_info = fragmentInfoArray[fragmentinfoCount - 1];
            fragment_info->fragment_i_frame[p_track->stream_num] =
              p_track->inter_frames;
            fragment_info->frames[p_track->stream_num] =
              p_track->frames;
            fragment_info->bytes[p_track->stream_num] =
              p_track->bytes;
            fragment_info->timestamp[p_track->stream_num] =
              p_track->media_duration;
            m_hasVideo = true;
          }
          else
          {
            //Check if the fragment is in the fragmentInfoArray
            for (uint32 i = 0; i < fragmentinfoCount; i++)
            {
              fragment_info = fragmentInfoArray[i];
              if((fragment_info->fragment_number == p_track->fragment_number))
              {
                m_hasVideo = true;
                break;
              }
            }
          }
          if(fragmentinfoCount && fragment_info)
          {
            MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
              "FragInfo: fNo=%lu, fFrames=%llu, fIFrame=%d",
              fragment_info->fragment_number,
            fragment_info->frames[p_track->stream_num],
            fragment_info->fragment_i_frame[p_track->stream_num]);
            MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM, "FragInfo: fOffset=%llu,\
              fBytes=%llu, fTs=%llu", fragment_info->fragment_offset,
              fragment_info->bytes[p_track->stream_num],
              fragment_info->timestamp[p_track->stream_num]);
          }
        }
        break;

      case VIDEO_FMT_STREAM_AUDIO:
        if(m_playAudio)
        {
          m_hasAudio = false;
          if(addData)
          {
            fragment_info = fragmentInfoArray[fragmentinfoCount - 1];
            fragment_info->fragment_i_frame[p_track->stream_num] =
              TRUE;
            fragment_info->frames[p_track->stream_num] =
              p_track->frames;
            fragment_info->bytes[p_track->stream_num] =
              p_track->bytes;
            fragment_info->timestamp[p_track->stream_num] =
              p_track->media_duration;
            m_hasAudio = true;
          }
          else
          {
            //Check if the fragment is in the fragmentInfoArray
            for (uint32 i = 0; i < fragmentinfoCount; i++)
            {
              fragment_info = fragmentInfoArray[i];
              if((fragment_info->fragment_number == p_track->fragment_number))
              {
                m_hasAudio = true;
                break;
              }
            }
          }
        }
        break;

      case VIDEO_FMT_STREAM_TEXT:
        if(m_playText)
        {
          m_hasText = false;
          if(addData)
          {
            fragment_info = fragmentInfoArray[fragmentinfoCount - 1];
            fragment_info->fragment_i_frame[p_track->stream_num] = TRUE;
            fragment_info->frames[p_track->stream_num] =
              p_track->frames;
            fragment_info->bytes[p_track->stream_num] =
              p_track->bytes;
            fragment_info->timestamp[p_track->stream_num] =
              p_track->media_duration;
            m_hasText = true;
          }
          else
          {
            //Check if the fragment is in the fragmentInfoArray
            for (uint32 i = 0; i < fragmentinfoCount; i++)
            {
              fragment_info = fragmentInfoArray[i];
              if((fragment_info->fragment_number == p_track->fragment_number))
              {
                m_hasText = true;
                break;
              }
            }
          }
        }
        break;

      default:
        break;
      }
    }
  }
}


/*===========================================================================

FUNCTION  ~Mp4FragmentFile

DESCRIPTION
  Destructor for the Mp4FragmentFile class

===========================================================================*/
Mp4FragmentFile::~Mp4FragmentFile()
{
  uint32 i;

  for (i = 0; i < fragmentinfoCount; i++)
  {
    if((fragmentInfoArray)[i] &&
       (fragmentInfoArray)[i] != NULL)
    {
      MM_Free(fragmentInfoArray[i]);
      (fragmentInfoArray)[i] = NULL;
    }

  }
}

/*===========================================================================

FUNCTION  getSampleAtTimestamp

DESCRIPTION
  Public method used to get the sample info for a sample placed at a given
  timestamp (in the track timescale).

===========================================================================*/

bool Mp4FragmentFile::getSampleAtTimestamp(video_fmt_stream_info_type *p_track,
                                           uint64                      timestamp,
                                           bool                        iRewind,
                                           video_fmt_sample_info_type *sampleInfo)
{
  bool retStat = false;

  uint32 streamNum = p_track->stream_num;
  uint64 curSample = m_sampleInfo[streamNum].sample;

  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  uint64 reqSampleNum;

  boolean fragmentParsed = FALSE;
  video_fmt_sample_info_type PrevSampleInfo;
  memset(&PrevSampleInfo, 0, sizeof(video_fmt_sample_info_type));

  if ( iRewind )
  {
    if(m_sampleInfo[streamNum].sample >= p_track->frames)
    {
      //has been repositioned to an earlier fragment
      //actual frames go from 0 ....; p_track->frames go from 1 ....
      m_sampleInfo[streamNum].sample = p_track->frames - 1;
    }

    for ( reqSampleNum = m_sampleInfo[streamNum].sample;
          reqSampleNum < p_track->frames ; --reqSampleNum )
    {
      if ( m_isFragmentedFile && (reqSampleNum < p_track->track_frag_info.first_frame) )
      {
          //parse to the previous fragment..
          if(!findiFrameFragment(p_track, reqSampleNum, iRewind, FALSE, fragmentParsed))
          {
              break;
          }
          else
          {
              m_parsedEndofFile = false;
          }
      }
      retStat = false;
      retError = getSampleInfo(streamNum, reqSampleNum, 1, sampleInfo);
      if (PARSER_ErrorNone == retError)
      {
        retStat = true;
      }

      if ( sampleInfo->time == timestamp )
      {
        break;
      }
      if (( sampleInfo->time < timestamp ))
      {
#ifdef _ANDROID_
        //! Find the closest entry and seek to that entry
        //! If the previous entry is closest, then seek to that entry
        if ((sampleInfo->time - timestamp) > (timestamp - PrevSampleInfo.time))
        {
          (void)getSampleInfo(streamNum, reqSampleNum + 1, 1, sampleInfo);
        }
#endif
        /* For Video, don't rewind before given time stamp, also in rewind, don't increase samples */
        if( (curSample > reqSampleNum) &&
            (p_track->type == VIDEO_FMT_STREAM_VIDEO) )
        {
          (void)getSampleInfo(streamNum, reqSampleNum+1, 1, sampleInfo);
        }
        break;
      }
    } // for
    //! Copy Current Sample Properties into local variable
    memcpy(&PrevSampleInfo, sampleInfo, sizeof(video_fmt_sample_info_type));
  } // rewind
  else
  {   // forward
    if(m_sampleInfo[streamNum].sample < p_track->track_frag_info.first_frame)
    {
      //has been repositioned to a later fragment
      m_sampleInfo[streamNum].sample = p_track->track_frag_info.first_frame;
    }
    for ( reqSampleNum = m_sampleInfo[streamNum].sample; ; ++reqSampleNum )
    {
      if((m_isFragmentedFile && m_parsedEndofFile) ||
         !m_isFragmentedFile)
      {
        if (reqSampleNum >= p_track->frames)
        {
          break;
        }
      }
      /* It is special scenario. For DASH clips, beyond first fragment Parser will not
         look for frame with required timestamp. It will simply return failure.
         As an enhancement, support to parse beyond first fragment will be added later.
      */
      if ( (m_isFragmentedFile && (reqSampleNum >= p_track->frames)) && !m_bIsDashClip )
      {
        //parse to the next fragment..
        if(!findiFrameFragment(p_track, reqSampleNum, iRewind, FALSE, fragmentParsed))
        {
          break;
        }
      }
      retStat = false;
      retError = getSampleInfo(streamNum, reqSampleNum, 1, sampleInfo);
      if(PARSER_ErrorNone == retError)
      {
        retStat = true;
      }
      else
      {
        retStat = false;
        _fileErrorCode = PARSER_ErrorSeekFail;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                    "Seek is failed as frame with desired TS is not found in first fragment");
        break;
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
#ifdef _ANDROID_
        //! Find the closest entry and seek to that entry
        //! If the previous entry is closest, then seek to that entry
        if ((sampleInfo->time - timestamp) > (timestamp - PrevSampleInfo.time))
        {
          (void)getSampleInfo(streamNum, reqSampleNum - 1, 1, sampleInfo);
        }
#endif
        /* don't forward after given time stamp, also don't decrease for forward */
        if((curSample < reqSampleNum) && p_track->type == VIDEO_FMT_STREAM_TEXT)
          (void)getSampleInfo(streamNum, reqSampleNum-1, 1, sampleInfo);
        break;
      }
      //! Copy Current Sample Properties into local variable
      memcpy(&PrevSampleInfo, sampleInfo, sizeof(video_fmt_sample_info_type));
    } // for
  } // forward

  return retStat;
}

/*===========================================================================

FUNCTION  skipNSyncSamples

DESCRIPTION
  Public method used to skip offSet Number of Sync Samples.

===========================================================================*/
uint64 Mp4FragmentFile::skipNSyncSamples(int offset, uint32 id, bool *bError, uint64 currentPosTimeStamp)
{
  uint64 reqSampleNum = 0;
  int    noOfSyncSamplesSkipped = 0;
  bool   result = false;
  uint64 newTimeStamp =0;

  boolean fragmentParsed = FALSE;
  boolean fragmentRepositioned = FALSE;

  /* Resetting the Error Code before processing the data */
  _fileErrorCode = PARSER_ErrorNone;

  video_fmt_stream_info_type *p_track = getTrackInfoForID(id);
  if ( p_track )
  {
    int streamNum = p_track->stream_num;

    reqSampleNum = m_sampleInfo[streamNum].sample;

    video_fmt_sample_info_type  sampleInfo;
    memset(&sampleInfo, 0, sizeof(video_fmt_sample_info_type));

    if( offset < 0 )
    {
      // rewind case
      for(reqSampleNum = m_sampleInfo[streamNum].sample; reqSampleNum < p_track->frames; --reqSampleNum) //penta
      {
        if(getSyncSampleInfo (streamNum, reqSampleNum, true, &sampleInfo))
        {
          noOfSyncSamplesSkipped++;
          reqSampleNum = sampleInfo.sample;
          if( noOfSyncSamplesSkipped == abs(offset) )
          {
            // In successfull case return the latest sync sample time
            result = true;
            break;
          }
        }
        else
        {
          //check are we reached end of the file in case of fragmented file
          if ( m_isFragmentedFile )
          {
            (void)FindKeyFrameFragmentIndex(p_track, reqSampleNum, true);
            //move to previous fragment..
            if(!findiFrameFragment(p_track, reqSampleNum, true, TRUE, fragmentParsed))
            {
              // No more fragments, so move to current sample and break the loop
              if(fragmentRepositioned || fragmentParsed)
              {
                (void)findiFrameFragment(p_track, m_sampleInfo[streamNum].sample, false, FALSE, fragmentParsed);
              }
              break;
            }
            else
            {
              /* for loop will decrement the counter, so to reach last sample
                 in the fragment increment */
              reqSampleNum++;
              // we have got one more fragment so keep searching in this fragment for Sync sample
              fragmentRepositioned = TRUE;
              continue;
            }
          }
          // This indicates no more Sync samples found
          break;
        }
      }
    }
    else
    {
      // forward case
      for(reqSampleNum = m_sampleInfo[streamNum].sample; ; ++reqSampleNum)
      {
        // Fragmented files count samples from 1 to p_track->frames
        if( m_isFragmentedFile && m_parsedEndofFile )
        {
          if (reqSampleNum > p_track->frames)
          {
            break;
          }
        }
        else
        {
          // Un Fragmented files count samples from 0 to p_track->frames -1
          if( !m_isFragmentedFile && (reqSampleNum > p_track->frames -1) )
          {
            break;
          }
        }

        if(getSyncSampleInfo( streamNum, reqSampleNum, false, &sampleInfo ))
        {
          noOfSyncSamplesSkipped++;
          reqSampleNum = sampleInfo.sample;
          if( noOfSyncSamplesSkipped==offset )
          {
            // In successfull case return the last sync sample time
            result = true;
            break;
          }
        }
        else
        {
          if ( m_isFragmentedFile && !m_parsedEndofFile )
          {
            (void)FindKeyFrameFragmentIndex(p_track, reqSampleNum, false);
            // move to the next fragment..
            if(!findiFrameFragment(p_track, reqSampleNum, false, TRUE, fragmentParsed))
            {
              // No more fragments, so move to current sample and break the loop
              if(fragmentRepositioned || fragmentParsed)
              {
                (void)findiFrameFragment(p_track, m_sampleInfo[streamNum].sample, true, FALSE, fragmentParsed);
              }
              break;
            }
            else
            {
              /* for loop will increment the counter, so to reach first sample
                 in the fragment decrement the value */
              reqSampleNum--;

              // we have got one more fragment so keep searching in this fragment for Sync sample
              fragmentRepositioned = TRUE;
              continue;
            }
          }
          // Not success full in skipping desired sync sample so return the old time stamp.
          break;
        }
      }
    }/* if-else offset */

    if( result )
    {
      *bError =  false;
      newTimeStamp =(sampleInfo.time*1000)/p_track->media_timescale;

      m_reposStreamPending |= maskByte[streamNum];
      m_nextReposSample[streamNum] = sampleInfo.sample;
      m_sampleInfo[streamNum] = sampleInfo;

      MM_MSG_PRIO2( MM_FILE_OPS, MM_PRIO_HIGH, "Time Stamp Returned after Skipping %d Sync Samples= %llu",
          offset, newTimeStamp );
    }
    else
    {
      *bError =  true;
      _fileErrorCode = PARSER_ErrorSeekFail;
      MM_MSG_PRIO1( MM_FILE_OPS, MM_PRIO_HIGH, "seekToSync function Failed In Skipping %d Sync Samples", offset );
      newTimeStamp = currentPosTimeStamp;
    }
  }/*if p_track */
  else
  {
    *bError =  true;
    _fileErrorCode = PARSER_ErrorSeekFail;
    MM_MSG_PRIO1( MM_FILE_OPS, MM_PRIO_HIGH, "seekToSync function Failed for track ID %lu", id );
    newTimeStamp = currentPosTimeStamp;
  }
  return newTimeStamp;
}

/*===========================================================================

FUNCTION  getTimestampedSampleInfo

DESCRIPTION
  Public method used to get the sample corresponding to a given timestamp.

===========================================================================*/

bool Mp4FragmentFile::getTimestampedSampleInfo(
  video_fmt_stream_info_type *p_track,
  uint64                      TimeStamp,
  video_fmt_sample_info_type *sampleInfo,
  uint64                     *newTimeStamp,
  bool                        bSetToSyncSample,
  uint64                      currentPosTimeStamp)
{
  boolean fragmentRepositioned = FALSE;
  boolean fragmentParsed = FALSE;

  // media timescale = number of time units per second
  // if media_timescale == 1000, the time units are miliseconds (1000/sec)
  uint64 timescaledTime = (TimeStamp * p_track->media_timescale)/1000;
  bool userRewind;
  uint64 scaledCurPosTime = (currentPosTimeStamp *p_track->media_timescale)/1000;

  uint32 streamNum = p_track->stream_num;
  uint64 maxFrames = p_track->frames;

  uint32 minFrames = 0;

  uint64 sampleDelta = 1;
  uint64 reqSampleNum = 0;
  int64 timeOffset = 0;
  bool  iRewind = false;
  bool retStat = false;
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  if( (!m_isFragmentedFile) && (maxFrames == 0) )
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
       "Mp4Frag::getTimestampedSampleInfo No valid frames for given track %lu",
       p_track->track_id);
    return false;
  }

  if( (m_nextSample[streamNum]==0) && (m_sampleInfo[streamNum].size==0) &&
      (p_track->dec_specific_info.obj_type!=MPEG4_IMAGE) )
  {
    /* we have not read any sample yet. So atleast read first good sample */
    uint32 sampleId = 0;
    do
    {
      retError = getSampleInfo(streamNum, sampleId, 1, &m_sampleInfo[streamNum]);
      if(PARSER_ErrorNone == retError)
      {
        retStat = true;
      }
      else if (PARSER_ErrorEndOfFile == retError)
      {
        retError = getSampleInfoError(p_track);
        if (PARSER_ErrorNone != retError)
        {
          break;
        }
        //! Mark the flag as true as sample properties are fetched successfully
        else
          retStat = true;
      }
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "Reading first valid sampleInfo. SampleId=%lu",sampleId);
      sampleId++;
    }
    while((retStat) && (m_mp4ReadLastStatus[streamNum] == VIDEO_FMT_IO_DONE) &&
          (!m_sampleInfo[streamNum].delta || !m_sampleInfo[streamNum].size));

    if(!retStat || (m_mp4ReadLastStatus[streamNum] != VIDEO_FMT_IO_DONE))
      return false;
  }

  timeOffset = m_sampleInfo[streamNum].time - timescaledTime;
  if ( p_track->type == VIDEO_FMT_STREAM_AUDIO )
  {
    userRewind = (timeOffset > 0)? true:false;
  }
  else
  {
    userRewind = (scaledCurPosTime > timescaledTime)?true:false;
  }

  if ( timeOffset == 0 )
  {
    *sampleInfo = m_sampleInfo[streamNum];
    if(sampleInfo->sync)
      return true;
  }
  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
               "CurrentMpeg4Sample=%llu, ReqdTS=%llu, curPosTS=%llu",
               m_sampleInfo[streamNum].sample, TimeStamp, currentPosTimeStamp);

  if( (p_track->type == VIDEO_FMT_STREAM_VIDEO) && bSetToSyncSample)
  {
    uint64 frameTime;

    /* Special case: if user wants to goto beginning of the case, just jump to
       first frame */
    if((timescaledTime == 0) && (!m_isFragmentedFile))
    {
      retStat = false;
      retError = getSampleInfo(streamNum, 0, 1, sampleInfo);
      if(PARSER_ErrorNone == retError)
      {
        retStat = true;
      }
      return retStat;
    }


    /* first estimate on which frame we have to go assuming fixed delta between
       video frames. This will take us may be ahead or before the desired time,
       but from that point we can either FF or REW by checking each I-Frame to
       get the exact reposition point (means closest I-Frame). This will reduce
       time in finding closest I-Frame, if user is doing very long seek
       (say 50 minutes)
    */

    /* if we have to seek more than pre-defined limit (5*60 secs), calculate
       approximately how many sample we have to rewind or forward assuming
       sample time delta for a track is fixed */
    if( (!m_isFragmentedFile) && (m_sampleInfo[streamNum].delta &&
        ((uint64)abs((int)(TimeStamp-currentPosTimeStamp))>
          m_defaultCoarseJumpLimit) ))
    {
      sampleDelta = (abs((int)timeOffset) / m_sampleInfo[streamNum].delta);
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
        if(!m_isFragmentedFile &&
          ((m_sampleInfo[streamNum].sample + sampleDelta) >= (p_track->frames-1))
          )
        {
          reqSampleNum = p_track->frames-1;
        }
        else
        { //for fragmented files we dont know the final sample number which
          // we would only know after parsing to the last fragment.
          reqSampleNum = m_sampleInfo[streamNum].sample + sampleDelta;
        }
      }

      /* If the required sample is not in the current fragment skip the fragment
       * to go to the required fragment. Also insures that the target fragment
       * has an I Frame or else the repositioning will fail
      */

      if(((reqSampleNum < p_track->track_frag_info.first_frame) ||
          ((reqSampleNum >= maxFrames) && (!m_parsedEndofFile))) &&
          (m_isFragmentedFile))
      {
        iRewind = (reqSampleNum < m_sampleInfo[streamNum].sample)?true:false;
        if(findiFrameFragment(p_track, reqSampleNum, iRewind, FALSE, fragmentParsed))
        {
            fragmentRepositioned = TRUE;
            maxFrames = p_track->frames;
        }
        else
        {
          if((fragmentParsed) && !iRewind)
          {
            // if reqSampleNum is beyond the total number of frames in the clip.
            //then rewind to the correct fragment Position.
            m_parsedEndofFile = false;
            (void)findiFrameFragment(p_track, m_sampleInfo[streamNum].sample,
                                     true, FALSE, fragmentParsed);
          }
          //could not find any fragment containing reqSampleNum.
          *newTimeStamp = 0;
          return false;
        }
      }

      retStat = false;
      retError = getSampleInfo(streamNum, reqSampleNum, 1, sampleInfo);
      /* If this sample timestamp is same as seek time, go to either next or
         previous entry. With this Parser will ensure to seek to sync sample */
      if(PARSER_ErrorNone == retError)
      {
        uint64 ullSampleTimeinMS;
        retStat = true;
        /* Convert timestamp into msec units. Sometimes due to fractional
           values, below comparision is not working properly.
           In order to avoid this, Sample time is converted into ms units. */
        ullSampleTimeinMS = (uint64)((double)sampleInfo->time * 1000.0) /
                            p_track->media_timescale;
        if( (ullSampleTimeinMS == TimeStamp) && (TRUE != sampleInfo->sync) )
        {
          /* if it is not a sync sample, just change the time stamp, so that we
             continue to search for correct SYNC sample. For this if it is REW,
             we will increase the sample. Also if we have reached zero sample,
             we can only increase it */
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

    iRewind = (sampleInfo->time > timescaledTime)?true:false;
    if(sampleInfo->time == timescaledTime)
      iRewind = userRewind;

    frameTime = sampleInfo->time;
    retStat = true;

    /* If file is DASH complaint, always do forward seek.
       update frameTime to less than seek time. This is to do forward seek at
       least once. */
    if ((FILE_SOURCE_MP4_DASH == m_eFileFormat) && (true == iRewind) &&
        (timescaledTime))
    {
      iRewind   = false;
      frameTime = timescaledTime - 1;
    }
    /* loop till we get the desired time stamp */
    if(iRewind)
    {
      /* Convert timestamp into msec units. Sometimes due to fractional values,
         Parser is going to one sample extra in backward direction. In order
         to avoid this, Sample time is also converted into milli-sec units. */
      uint64 ullSampleTimeinMS = (frameTime * 1000) /p_track->media_timescale;
      for( ; (retStat) && (ullSampleTimeinMS > TimeStamp); )
      {
        /*reqSampleNum is always within the current parsed fragment at this point*/
        retStat = getSyncSampleInfo(streamNum, reqSampleNum, iRewind, sampleInfo);

        MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH, "Rewind1: SynchSample=%llu, \
                     frag_info.first_frame=%llu, eof=%d", sampleInfo->sample,
                     p_track->track_frag_info.first_frame,m_parsedEndofFile);

        bool iFrameOnFirstSample = false;
        iFrameOnFirstSample = retStat && sampleInfo->sample
            && (sampleInfo->sample == p_track->track_frag_info.first_frame)
            && (sampleInfo->time > timescaledTime) && m_isFragmentedFile;

        MM_MSG_PRIO3 (MM_FILE_OPS, MM_PRIO_HIGH, "SynchSample=>offset=%llu, \
                      =>size=%lu, iFrameOnFirstSample=%d"
                  , sampleInfo->offset, sampleInfo->size,iFrameOnFirstSample);

        if((!retStat && m_isFragmentedFile) || iFrameOnFirstSample )
        {
          if(p_track->track_frag_info.first_frame)
          {
            //not the first fragment
            //move to the previous fragment.
            reqSampleNum = FILESOURCE_MIN(reqSampleNum,p_track->track_frag_info.first_frame - 1);
            if(findiFrameFragment(p_track, reqSampleNum, iRewind, TRUE, fragmentParsed))
            {
              m_parsedEndofFile = false;
              maxFrames = p_track->frames;
              fragmentRepositioned = TRUE;
              reqSampleNum = FILESOURCE_MIN(reqSampleNum,p_track->frames -1);
              retStat = true;
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                           "Rewind2..found iFrameFragment MaxFrames=%llu",
                           maxFrames);
              continue;
            }
            else
            {
              //Note:Rewind should always succeed.
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                          "Rewind2..did not find an iFrameFragment");
              if((fragmentParsed) || (fragmentRepositioned))
              {
                // if the inter fragment repositioning failed and the fragment
                // has been repositioned. Now reposition to the correct fragment
                iRewind = (m_sampleInfo[streamNum].sample < reqSampleNum)?true:false;
                (void)findiFrameFragment(p_track,
                                         m_sampleInfo[streamNum].sample,
                                         iRewind, FALSE, fragmentParsed);
              }
              *newTimeStamp = 0;
              retStat = false;
              break;
            }
          }
        }
        frameTime = sampleInfo->time;
        ullSampleTimeinMS = (frameTime * 1000) /p_track->media_timescale;

        /* user has actually pressed FF, but we are rewinding, then we should
           not go beyond current displaty time. We also should make sure we are
           beyond the time user has asked, not before */
        if(!userRewind)
        {
          if( (scaledCurPosTime >= frameTime) ||(timescaledTime > frameTime) )
          {
            /* means we have gone before current position, but user has pressed FF,
               so we should look I-Frame in FF direction */
            iRewind = false;
            reqSampleNum = sampleInfo->sample+1;
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                      "Forward0: !userRewind reqSampleNum=%llu",reqSampleNum);
            if((sampleInfo->sample >= (maxFrames-1))&& m_isFragmentedFile)
            {
              findiFrameFragment(p_track, reqSampleNum, FALSE, FALSE,
                                 fragmentParsed);
            }
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
        if(m_isFragmentedFile)
          findiFrameFragment(p_track, reqSampleNum, TRUE, FALSE, fragmentParsed);
          retStat = false;
          retError = getSampleInfo(streamNum, reqSampleNum, 1, sampleInfo);
          if(PARSER_ErrorNone == retError)
          {
            retStat = true;
          }
      }
    }

  /* Flag to mark the seek operation has been done. This flag is useful for
     DASH clips. With the help of this flag, Parser will not update same TFDT
     value both in Seek and playback APIs two times. Moved this update after
     rewind seek logic to minimise the flag usage only for forward seek loigc.
  */
  m_bSeekDone = true;

    /* FF case */
    if(!iRewind)
    {
      video_fmt_sample_info_type PrevSyncSampleInfo;
      memset(&PrevSyncSampleInfo, 0, sizeof(video_fmt_sample_info_type));
      for( ; frameTime<=timescaledTime && retStat; )
      {
        /*reqSampleNum is always within the current parsed fragment at this point*/
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                     "invoking getSyncSampleInfo with reqSampleNum %llu",
                     reqSampleNum);
#endif
        retStat = getSyncSampleInfo(streamNum, reqSampleNum, iRewind, sampleInfo);

        MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_HIGH,
          "Forward1: SynchSample=%llu =>offset=%llu =>size=%lu time %llu",
          sampleInfo->sample, sampleInfo->offset, sampleInfo->size,
          sampleInfo->time);
        if ((sampleInfo->sample >= reqSampleNum) && retStat)
        {
          memcpy(&PrevSyncSampleInfo, sampleInfo,
                 sizeof(video_fmt_sample_info_type));
        }

        /* It is a special scenario for DASH standard compliant clips.
           Parser will return Failure, if I-frame is not available in the
           first fragment.
        */
        if(m_bIsDashClip && !retStat)
        {
          _fileErrorCode = PARSER_ErrorSeekFail;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "Seek is failed as I frame is not found in first fragment");
          return retStat;
        }
        if(!retStat  && m_isFragmentedFile)
        {
          /* If Sync Sample could not be found on this fragment FFWD to next fragment
          */
          //move to the next fragment.
          reqSampleNum = FILESOURCE_MAX(reqSampleNum,p_track->frames);
          if(!m_parsedEndofFile &&
             findiFrameFragment(p_track, reqSampleNum, iRewind, TRUE, fragmentParsed))
          {
            maxFrames = p_track->frames;
            fragmentRepositioned = TRUE;
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
              "came back in getTimestampedSampleInfo from findiFrameFragment");
            MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
              "reqSampleNum %llu p_track->track_frag_info.first_frame %llu \
              maxFrames %llu",reqSampleNum,
              p_track->track_frag_info.first_frame, maxFrames);
#endif
            reqSampleNum = FILESOURCE_MAX(reqSampleNum,
                                         p_track->track_frag_info.first_frame);
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
              "updated reqsample by MAX reqSampleNum %llu p_track->track_frag_\
              info.first_frame %llu",reqSampleNum,
              p_track->track_frag_info.first_frame);
#endif
            retStat = true;
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                        "Forward2.. found iFrameFragment");
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                        "Forward2.. found iFrameFragment,continuing..");
#endif
            continue;
          }
          else
          {
            bool dataUnderRun = false;
            if( sampleInfo->sync)
            /* Sync to the previous sync sample*/
            {
              *newTimeStamp = (sampleInfo->time * 1000) /
                              p_track->media_timescale;
              /* check if we got any sync sample later than current sample */
              if(*newTimeStamp > currentPosTimeStamp)
              {
                /* check if the sync sample is in the same fragment*/
                if(fragmentParsed || fragmentRepositioned)
                {
                  iRewind = (sampleInfo->sample < reqSampleNum)?true:false;
                  findiFrameFragment(p_track, sampleInfo->sample, iRewind,
                                     FALSE, fragmentParsed);
                }
                retStat=false;
                retError= getSampleInfo(streamNum, sampleInfo->sample, 1,
                                        sampleInfo);
                if(PARSER_ErrorNone == retError)
                {
                retStat = true;
                }
                break;
              }
            }
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                        "Forward2..did not find an iFrameFragment");
#ifdef _ANDROID_
            //go to last valid sync sample property
            if ((PrevSyncSampleInfo.sample) &&
                (FILE_SOURCE_MP4_DASH != m_eFileFormat))
            {
              m_sampleInfo[streamNum].sample = PrevSyncSampleInfo.sample;
            }
#endif
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
            MM_MSG_PRIO7(MM_FILE_OPS, MM_PRIO_HIGH,
              "Forward2..sampleInfo->sample %llu (maxFrames-1)%llu \
              m_parsedEndofFile %d fragmentParsed %d fragmentRepositioned %d \
              reqSampleNum %llu retStat %d",
              sampleInfo->sample, (maxFrames-1), m_parsedEndofFile,
              fragmentParsed, fragmentRepositioned, reqSampleNum,retStat);
#endif
            /* Even though complete data is available, Videofmt cannot exit
               properly except in DATA_INCOMPLETE state while parsing of
               fragment. So, along with checking of videofmt status, we also
               need to check whether complete data has been downloaded or not.
            */
            if( (VIDEO_FMT_DATA_INCOMPLETE == m_mp4ParseLastStatus) &&
                (FALSE == m_bEndOfData) &&
                (_fileErrorCode != PARSER_ErrorSeekUnderRunInFragment) )
            {
              dataUnderRun = true;
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                "Reposition failed for track id %lu, due to underrun!!",
                p_track->track_id);
            }
            if((fragmentParsed) || (fragmentRepositioned))
            {
              // If the inter fragment repositioning failed and the fragment has
              // been repositioned. Now reposition to the correct fragment
              m_parsedEndofFile = false;
              iRewind = (m_sampleInfo[streamNum].sample < reqSampleNum)?true:false;
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
              "Forward2..resetting m_parsedEndofFile and determining iRewind?");
              MM_MSG_PRIO7(MM_FILE_OPS, MM_PRIO_HIGH,
                "Forward2..sampleInfo->sample %llu (maxFrames-1)%llu \
                m_parsedEndofFile %d fragmentParsed %d fragmentRepositioned \
                %d reqSampleNum %llu iRewind %d", sampleInfo->sample,
                (maxFrames-1), m_parsedEndofFile, fragmentParsed,
                fragmentRepositioned, reqSampleNum, iRewind);
#endif
              (void)findiFrameFragment(p_track, m_sampleInfo[streamNum].sample,
                                       iRewind, FALSE, fragmentParsed);
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                "Forward2..came back in getTimestampedSampleInfo from \
                findiFrameFragment with frg. repositioned to \
                m_sampleInfo[streamNum].sample %llu",
                m_sampleInfo[streamNum].sample);
#endif
            }
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
              "Forward2..m_sampleInfo[streamNum].sample+1 %llu \
              p_track->frames %llu",
              (m_sampleInfo[streamNum].sample+1),p_track->frames);
#endif
            //If this was the last sample in the current fragment, then reset
            // the stream offset, as this fragment will not be processed in
            // the videoFMT reader.
            if(m_sampleInfo[streamNum].sample+1 == p_track->frames)
            {
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "Forward2..inside (m_sampleInfo[streamNum].sample+1 == \
                p_track->frames)");
#endif
                /* Since we fail the repositioning, we had parsed the current
                   fragment again and videoFMT will be pointing to the start of
                   fragment. By calling getSampleInfo, VideoFMT will move its
                   current pointer again to the current sample just like it was
                   before repositioning started..
                */
                getSampleInfo(streamNum, m_sampleInfo[streamNum].sample, 1,
                              &m_sampleInfo[streamNum]);
            }

            /* If a fragment is not available in forward direction due to
               data-underrun, m_mp4ParseLastStatus will be DATA_INCOMPLETE.
               There is no need to parse previous fragment properties. Sothat
               "m_mp4ParseLastStatus" value will not be modified. By using that
               lastStatus value, Parser can report Data_Underrun to FS. */
            if(true == dataUnderRun)
            {
              _fileErrorCode = PARSER_ErrorDataUnderRun;
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                          "updating _fileErrorCode to Data-UnderRun!!");
            }
            *newTimeStamp = 0;
            retStat = false;
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                      "Forward2..making *newTimeStamp 0 and retStat as false");
#endif
#ifdef _ANDROID_
            if ((PrevSyncSampleInfo.sample) &&
                (FILE_SOURCE_MP4_DASH != m_eFileFormat))
            {
              *newTimeStamp = PrevSyncSampleInfo.time;
              retStat = true;
            }
#endif
            break;
          }
        }
        frameTime = sampleInfo->time;
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
        MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
          "updated frameTime to sampleInfo->time %llu timescaledTime %llu \
          retStat %d", frameTime, timescaledTime, retStat);
#endif

        /* if we have found the frame, so we don't need to go further */
        if(frameTime>=timescaledTime && retStat)
        {
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH, "breaking in (frameTime>=timescaledTime && retStat)frameTime %llu timescaledTime %llu",
            frameTime,timescaledTime);
          break;
        }
        /*If the last sample is a sync sample then set the time stamp to that sync sample. */
         if((retStat && m_parsedEndofFile && (sampleInfo->sample == (p_track->frames-1))))
         {
           *newTimeStamp = (sampleInfo->time * 1000) / p_track->media_timescale;
          /*check if we got any sync sample later than current sample */
          if(*newTimeStamp > currentPosTimeStamp)
           return true;
        }

         /**If the serach in the current Fragment for Sync frames completed, then move
          **to next available I frame fragment, if not available come out of loop
          */

        if((sampleInfo->sample >= (maxFrames-1)))
        {
          if(m_parsedEndofFile)
          {
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
            MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
              "(sampleInfo->sample >= (maxFrames-1)), sampleInfo->sample %llu \
               (maxFrames-1)%llu m_parsedEndofFile %d", sampleInfo->sample,
               (maxFrames-1), m_parsedEndofFile);
#endif
            retStat = false;
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
            "breaking (sampleInfo->sample >= (maxFrames-1)), as \
            m_parsedEndofFile is TRUE sampleInfo->sample %llu (maxFrames-1)%llu",
            sampleInfo->sample, (maxFrames-1));
            break;
          }
          else
          {
           /**This scenario normally happens, if the last sample in the fragment is a sync sample
           */
            reqSampleNum = FILESOURCE_MAX(reqSampleNum,p_track->frames);
            if( findiFrameFragment(p_track, reqSampleNum, iRewind, TRUE,
                                   fragmentParsed) )
            {
              maxFrames = p_track->frames;
              fragmentRepositioned = TRUE;
              reqSampleNum = FILESOURCE_MAX(reqSampleNum,
                                        p_track->track_frag_info.first_frame);
              retStat = true;
              continue;
            }
            else
            {
              retStat = false;
              break;
            }
          }
        }
        reqSampleNum = sampleInfo->sample+1;
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
          "sampleInfo->sample %llu reqSampleNum(increase sample by 1) %llu",
          sampleInfo->sample,reqSampleNum);
#endif
      }

#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
      MM_MSG_PRIO6(MM_FILE_OPS, MM_PRIO_HIGH,
        "out of loop for loop retStat %d sampleInfo->sync %lu \
        m_parsedEndofFile %d sampleInfo->sample %llu, \
        (p_track->frames-1)%llu, sampleInfo->time %llu",
        retStat,sampleInfo->sync, m_parsedEndofFile, sampleInfo->sample,
        (p_track->frames-1), sampleInfo->time);
#endif

      if(!m_isFragmentedFile && retStat == false)
      {
        if(sampleInfo->sync)
        {
       /* Sync to the previous sync sample*/
          retStat=false;
          retError= getSampleInfo(streamNum, sampleInfo->sample, 1, sampleInfo);
          if(PARSER_ErrorNone == retError)
          {
            retStat = true;
          }
        }
        else
      /* in FF case, if we did not find any SYNC sample, then we try to find
        SYNC sample in reverse direction. If we get any SYNC sample and its time
        stamp is ahead of current position, we should FF upto that sample*/
        retStat = getSyncSampleInfo(streamNum, reqSampleNum, TRUE, sampleInfo);
        if(retStat)
        {
          *newTimeStamp = (sampleInfo->time * 1000) /p_track->media_timescale;
          /* check if we got any sync sample later than current sample */

          //TO DO: AS of now, we don't know how to fail a seek on win mobile.
          //Thus for clips, when there is no I frame in forward direction,
          // we should let it search backward all the way to the first I frame
          // even though it's TS < current playback time. Once we figure out
          // how to fail the seek, we can put back original code.

          /*if(*newTimeStamp > currentPosTimeStamp)
            return true;
          else
          {
            retStat = false;
          }*/
          retStat = true;
        }
      }
    }
    /* see if we were successful */
    if(retStat)
    {
      *newTimeStamp = (sampleInfo->time * 1000) / p_track->media_timescale;
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
      MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH, "out of for loop when retStat is \
                   TRUE updated *newTimeStamp to %llu,sampleInfo->time %llu, \
                   p_track->media_timescale %lu",
                  (*newTimeStamp),sampleInfo->time,p_track->media_timescale);
#endif
    }
#ifdef FEATURE_INTER_FRAGMENT_REPOS_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "returning retStat as %d", retStat);
#endif
    return retStat;
  }

  /* Flag to mark the seek operation has been done. This flag is useful for
     DASH clips. With the help of this flag, Parser will not update same TFDT
     value both in Seek and playback APIs two times.
  */
  m_bSeekDone = true;

  /* calculate approximately how many sample we have to rewind or forward
     assuming sample time delta for a track is fixed */
  if (m_sampleInfo[streamNum].delta )
  {
    sampleDelta = (uint64)(abs((int) timeOffset) / m_sampleInfo[streamNum].delta);
  }

  if (userRewind )
  {
    bool bHasFrames= false;
    // rewind processing (current time > desired timestamp)
    //------------------
    iRewind = true;


    /* Check if current fragment has frames or not
     By Checking if the current fragment is in the fragmentInfoArray  */
    for (uint32 i = 0; i < fragmentinfoCount; i++)
    {
      fragment_info_type *fragment_info = fragmentInfoArray[i];
      if((fragment_info->fragment_number) == (p_track->fragment_number))
      {
        bHasFrames = true;
        break;
      }
    }
    if(p_track->type != VIDEO_FMT_STREAM_TEXT)
    {
      if ( m_sampleInfo[streamNum].sample >= sampleDelta )
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
      reqSampleNum = m_sampleInfo[streamNum].sample;
    }
    if ( reqSampleNum <= minFrames )
    {
      reqSampleNum = minFrames;
    }

    if( ( (reqSampleNum < p_track->track_frag_info.first_frame)||(!bHasFrames) ) &&
        (m_isFragmentedFile))
    {
      if(findiFrameFragment(p_track, reqSampleNum, iRewind, TRUE, fragmentParsed))
      {
        fragmentRepositioned = TRUE;
      }
      else
      {
        *newTimeStamp = 0;
        return false;
      }
    }

  }
  else
  {    // timeOffset < 0
    // fast forward processing (current time < desired timestamp)
    //------------------------
    if(p_track->type != VIDEO_FMT_STREAM_TEXT)
    {
      reqSampleNum = m_sampleInfo[streamNum].sample + sampleDelta;
    }
    else
    {
      reqSampleNum = m_sampleInfo[streamNum].sample;
    }
    if((reqSampleNum >= maxFrames) && (!m_parsedEndofFile) &&
      (m_isFragmentedFile))

    {
      if(findiFrameFragment(p_track, reqSampleNum, iRewind, TRUE, fragmentParsed))
      {
        maxFrames = p_track->frames;
        fragmentRepositioned = TRUE;
      }
      else
      {
        if(fragmentParsed)
        {
          if(p_track->type == VIDEO_FMT_STREAM_AUDIO)
          {

            /*
            * This could happen if we are requesting more than the duarion of audio track.
            * In this case, we should not fail the repositioning for audio track.
            * Instead, we should take audio to the last sample.
            *
            * This will reduce the duration of video freeze for A+V clip as CheckFrame would be
            * smaller than the case if we would have failed the repositioning.
            */
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
              "findiframefragment returned FALSE for VIDEO_FMT_STREAM_AUDIO for\
              reqSampleNum %llu",reqSampleNum);

            int32 length = 0, index = 0;
            fragment_info_type *fragment_info = NULL;

            /*
            * By this time, all fragments that have audio sample > what we are
            * playing right now, would have been added when we invoked findiFrameFragment above
            * and 'fragmentParsed' is set to TRUE.
            *
            * fragmentInfoArray.GetLength() will help to identify the correct fragment
            * which has the last audio sample. It could be in the same fragment which has
            * current audio sample being played.
            */
            length = fragmentInfoArray.GetLength();
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                         "fragmentInfoArray.GetLength() returned %ld",length);

            /*
            * Adjust the index of target fragment
            */
            index = length -1;

            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "adjusted index to %ld",index);

            /*
            * Get the target fragment_info to identify the
            * fragment which has lat audio sample.
            */
            fragment_info = fragmentInfoArray[index];


            if(fragment_info)
            {
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                "got fragment_info for index %lu and \
                fragment_info->frames[p_track->stream_num] %llu",
                index,fragment_info->frames[p_track->stream_num]);

              /*
              * Adjust the 'reqSampleNum' if we are asking
              * more than what's in the audio track
              */
              if(reqSampleNum >= fragment_info->frames[p_track->stream_num])
              {
                /*
                * p_track numbering runs from 1..., so subtract 1.
                */
                reqSampleNum = fragment_info->frames[p_track->stream_num] -1;
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                             "adjusted reqSampleNum to %llu",reqSampleNum);

                /*
                * Invoke findiFrameFragment with adjusted 'reqSampleNum' to
                * bring the correct fragment which has the last audio sample,
                * so that we can invoke getSampleInfo on 'reqSampleNum'
                */

                /*
                * In this case reqSampleNum is the last available audio sample
                * we must search for the iFrameFragment for this reqSampleNum
                * in the backward direction so rewind must be true
                */

                iRewind= TRUE;
                if(findiFrameFragment(p_track, reqSampleNum, iRewind, TRUE,
                                      fragmentParsed))
                {
                  maxFrames = p_track->frames;
                  fragmentRepositioned = TRUE;
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                               "current timescaledTime %llu ",timescaledTime);


                  if(PARSER_ErrorNone == getSampleInfo(streamNum, reqSampleNum,
                                                       1, sampleInfo) )
                  {
                    /*
                    * Update the timescaledTime to match with the
                    * TS of the last valid audio sample
                    */
                    timescaledTime = (sampleInfo->time * p_track->media_timescale)
                                     / 1000 ;
                    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                                 "Updated timescaledTime to %llu ",timescaledTime);
                  }

                }
                else
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "findiFrameFragment returned FALSE for adjusted reqSampleNum\
                    %llu",reqSampleNum);
                }
              }
            }//if(fragment_info)
            else
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                           "got NULL fragment_info for index %lu",index);
            }
          }//if(p_track->type == VIDEO_FMT_STREAM_AUDIO)
          else
          {
            // if the inter fragment FFWD failed and the fragment has been repositioned
            //Now Rewind to the correct fragment Position
            (void)findiFrameFragment(p_track, m_sampleInfo[streamNum].sample, true, FALSE, fragmentParsed);
          }
        }//if(fragmentParsed)
        if(p_track->type != VIDEO_FMT_STREAM_AUDIO)
        {
          *newTimeStamp = 0;
          return false;
        }
      }
    }

    if ( reqSampleNum >= maxFrames )
    {
      reqSampleNum = maxFrames - 1;
    }
  }

  if ( p_track->type == VIDEO_FMT_STREAM_AUDIO )
  {
    retStat=false;
    retError= getSampleInfo(streamNum, reqSampleNum, 1, sampleInfo);
    if(PARSER_ErrorNone == retError)
    {
      retStat = true;
    }
     MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
       "AUDIO CoarseRepos: reqSampleNum=%llu, sampleInfo.sample=%llu, \
        sampleInfo.time=%llu", reqSampleNum, sampleInfo->sample,
        sampleInfo->time);
    if(retStat && (sampleInfo->time!=timescaledTime))
    {
      //perform coarse repositioning..
      m_sampleInfo[streamNum] = *sampleInfo;
      if(sampleInfo->time < timescaledTime)
      {
        iRewind = false;
      }
      else
      {
        iRewind = true;
      }

      retStat = getSampleAtTimestamp (p_track, timescaledTime, iRewind, sampleInfo);
      MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
        "AUDIO FineRepos: sampleInfo.sample=%llu, sampleInfo.time=%llu, \
        iRewind=%d", sampleInfo->sample, sampleInfo->time, iRewind);
    }
  }
  else if( (p_track->type == VIDEO_FMT_STREAM_TEXT) ||
           (p_track->type == VIDEO_FMT_STREAM_VIDEO) )
  {
    retStat = getSampleAtTimestamp (p_track, timescaledTime, iRewind, sampleInfo);
  }

  *newTimeStamp = (sampleInfo->time*1000)/p_track->media_timescale;
  MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_HIGH,
            "newTimeStamp=%llu, m_playVideo=%d, m_playAudio=%d, m_playText=%d",
                *newTimeStamp,m_playVideo,m_playAudio,m_playText);

  return retStat;
}

