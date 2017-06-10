/* =======================================================================
                              FileSource.cpp
DESCRIPTION
  Definiton of interfaces for Filesource Module.

  The Intent of having this module is to Provide a common interface for
  upperlayers to control the FileBase (for both MP4 and ASF file formats)
  and perform basic operations like Play/Pause/Open/Seek based on the TrackId
  independent of the media (Audio/Video/Text) type.


  Copyright (c) 2008-2014 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileSource/main/latest/src/filesource.cpp#118 $$
$DateTime: 2013/08/28 01:52:54 $
$Change: 4345281 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "AEEStdDef.h"
#include "filesource.h"
#include "MMDebugMsg.h"
#include "filesourcehelper.h"
/* ==========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */
/* =======================================================================
**                            Function Definitions
** ======================================================================= */

/* Constructor for FileSource when called with a function call back
 */
FileSource::FileSource(FileSourceCallbackFuncType callBack, void * pClientData, bool bAsync):
m_pHelper(NULL)
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "FileSource::FileSource 0x%x", this);
  m_pHelper = MM_New_Args( FileSourceHelper, (callBack,pClientData, bAsync) );
  MM_Debug_Initialize();
}

/* Destructor for FileSource
 */
FileSource::~FileSource()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "FileSource::~FileSource ");
  if(m_pHelper)
  {
    MM_Delete(m_pHelper);
  }
  m_pHelper = NULL;
  MM_Debug_Deinitialize();
}
/*! ======================================================================
    @brief  Open a File.

    @detail This method is called only in the case of Local file playback.
            FileSource  Module will pass the filenames to lower layer (fileBase) to open
            the files and get information about the tracks, headers etc.

    @param[in]  audioFilename  name of the audio file with audio track.
    @param[in]  videoFilename  name of the video file with video track.
    @param[in]  textFilename   name of the text file with text track.
    @param[in]  format         If file format is already known,caller
                               can pass in the information into FileSource.
                               When format != FILE_SOURCE_UNKNOWN,FileSource
                               will instantiate the given parser without doing any
                               file format check.
    @param[in]  blookforcodecconfig  When true, parser returns OPEN_COMPLETE only when
                                   codec configuration header has been located for audio/video/text track
                                   When false, OPEN_COMPLTE is reported as soon as all valid tracks are located.
    @return     filesource status.

    @note   It is expected that when this function is called atleast one of the
            parameters is not NULL. i.e, atleast one filename is passed.
========================================================================== */

FileSourceStatus  FileSource::OpenFile(wchar_t* audioFilename,
                                       wchar_t* videoFilename,
                                       wchar_t* textFilename,
                                       FileSourceFileFormat format,
                                       bool     blookforcodecconfig)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::OpenFile wchar_t");
  FileSourceStatus status = FILE_SOURCE_NOTAVAILABLE;
  if(m_pHelper)
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                 "FileSource::OpenFile m_pHelper 0x%x filesource 0x%x",
                 m_pHelper, this);
    status = m_pHelper->OpenFile(audioFilename,videoFilename,textFilename,format,blookforcodecconfig);
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::OpenFile wchar_t status %d",status);
  return status;
}
/*! ======================================================================
   @brief  Buffer playback.

    @detail This method is called to play audio/video/text from buffers passed in.
            It's possible to play audio/video/text from separate buffers or user
            can just pass in the same buffer for audio/video/text.
            FileSource  Module will pass the buffers to lower layer (fileBase) to parse
            and get information about the tracks, headers etc.

    @param[in]  pAudioBuf     Buffer to play audio.
    @param[in]  nAudioBufSize Total size of audio buffer
    @param[in]  pVideoBuf     Buffer to play video.
    @param[in]  nAudioBufSize Total size of video buffer
    @param[in]  pTextBuf      Buffer to play text.
    @param[in]  nTextBufSize  Total size of text buffer
    @param[in]  format        If file format is already known,caller
                              can pass in the information into FileSource.
                              When format != FILE_SOURCE_UNKNOWN,FileSource
                              will instantiate the given parser without doing any
                              file format check.
    @return     filesource status.

    @note   It is assumed that these buffers will be valid through out the playback
            and data bytes are stored at contiguous address. FileSource module
            can request the data within the valid offsets range at anytime.

            0 - nAudioBufSize
            0 - nVideoBufSize
            0 - nTextBufSize

            It is expected that when this function is called atleast one of the
            parameters is not NULL. i.e, atleast one buffer is not NULL.
========================================================================== */
FileSourceStatus  FileSource::OpenFile(unsigned char* pAudioBuf,
                                       uint32         nAudioBufSize,
                                       unsigned char* pVideoBuf,
                                       uint32         nVideoBufSize,
                                       unsigned char* pTextBuf,
                                       uint32         nTextBufSize,
                                       FileSourceFileFormat format)
{
  FileSourceStatus status = FILE_SOURCE_NOTAVAILABLE;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::OpenFile unsigned char*");
  if(m_pHelper)
  {
    status = m_pHelper->OpenFile(pAudioBuf,nAudioBufSize,pVideoBuf,nVideoBufSize,pTextBuf,nTextBufSize,format);
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::OpenFile unsigned char* status %d",status);
  return status;
}
/*! ======================================================================
   @brief   Opens a Streaming file

   @detail  It takes iStreamPort as input parameter. iStreamPort will provide interfaces
            to Read/Seek/Close.
            FileSource Module will read the information and get information about the
            tracks, headers etc.

   @param[in]  pInputStream  Input Stream of Audio/video/text tracks
   @param[in]  format        If file format is already known,caller
                             can pass in the information into FileSource.
                             When format != FILE_SOURCE_UNKNOWN,FileSource
                             will instantiate the given parser without doing any
                             file format check.
   @param[in]  blookforcodecconfig  When true, parser returns OPEN_COMPLETE only when
                                   codec configuration header has been located for audio/video/text track
                                   When false, OPEN_COMPLTE is reported as soon as all valid tracks are located.
   @return     filesource status.

   @note    It is expected that pInputStream is  not  NULL.
========================================================================== */

FileSourceStatus FileSource::OpenFile(video::iStreamPort* pInputStream,
                                      FileSourceFileFormat format,
                                      bool     blookforcodecconfig)
{
  FileSourceStatus status = FILE_SOURCE_NOTAVAILABLE;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::OpenFile iStreamPort");

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  if(m_pHelper)
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                 "FileSource::OpenFile m_pHelper 0x%x filesource 0x%x",
                 m_pHelper, this);
    status = m_pHelper->OpenFile(pInputStream,format,blookforcodecconfig);
  }
#else
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD not enabled");
  status = FILE_SOURCE_NOTAVAILABLE;
#endif/*FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD*/

  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::OpenFile iStreamPort status %d",status);
  return status;
}
/*! ======================================================================
   @brief    Opens IxStream for DCF playback.

   @detail  It takes IxStream* as input parameter. dcf_ixstream_type will provide
            interfaces to Read/Seek/Close.
            FileSource Module will read the information and get information about the
            tracks, headers etc.

   @param[in]  pInputStream   Input DCF Stream of Audio/video/text tracks
   @return     filesource status.

   @note    It is expected that pInputStream is  not  NULL.
========================================================================== */
#ifdef FEATURE_FILESOURCE_DRM_DCF
FileSourceStatus FileSource::OpenFile( IxStream* pInputStream)
#else
FileSourceStatus FileSource::OpenFile( IxStream*)
#endif
{
  FileSourceStatus status = FILE_SOURCE_NOTAVAILABLE;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::OpenFile IxStream");
#ifdef FEATURE_FILESOURCE_DRM_DCF
  if(m_pHelper)
  {
    status = m_pHelper->OpenFile(pInputStream);
  }
#endif
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::OpenFile IxStream* status %d",status);
  return status;
}
/*! ======================================================================
   @brief   Close the existing opened file

   @detail     Please see the note below

   @param[in]  None
   @param[out] None
   @return     filesource status.

   @note       Once close is called, FileSource object will no longer support any
               API except OpenFile.
========================================================================== */
FileSourceStatus FileSource::CloseFile()
{
  FileSourceStatus status = FILE_SOURCE_SUCCESS;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::CloseFile");
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
               "FileSource::CloseFile m_pHelper 0x%x filesource 0x%x",
               m_pHelper, this);
  status = m_pHelper->CloseFile();
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::CloseFile Close is complete");
  return status;
}
/*! ======================================================================
  @brief        method returns the file format.

  @param[out]   fileFormat   type of  FileFormat ( MPEG4/ASF/AVI).
  @return       filesource status.

  @note    It is expected that OpenFile() is called before getFileFormat() is called.
========================================================================== */

FileSourceStatus FileSource::GetFileFormat(FileSourceFileFormat& fileFormat)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  fileFormat = FILE_SOURCE_UNKNOWN;
  if(m_pHelper)
  {
    status = m_pHelper->GetFileFormat(fileFormat);
  }
  return status;
}
/*! ======================================================================
  @brief    API returns the DRM scheme used, if any
  @param[out]  drmtype   type of  DRM. Please refer to FileSourceDrmType enum.
  @return      filesource status.
  @note    It is expected that user has received the successul callback for OpenFile() earlier.
======================================================================  */
FileSourceStatus FileSource::GetDRMType(FileSourceDrmType& drmtype)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  drmtype = FILE_SOURCE_NO_DRM;
  if(m_pHelper)
  {
    status = m_pHelper->GetDRMType(drmtype);
  }
  return status;
}
/*! ======================================================================
  @brief    API to retrieve JANUS DRM information/header.

  @param[in/out]  nsize   Size of the drm header/information.
  @param[out]     pdata   points to memory to be filled in by filesource
  @return         filesource status.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
           Call the API with NULL pdata to get the size needed for storing drm header/info.
           User needs to allocate the memory and call again with valid memory address.
======================================================================  */
FileSourceStatus FileSource::GetJanusDRMInfo(void* pdata,uint32* nsize)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    status = m_pHelper->GetJanusDRMInfo(pdata,nsize);
  }
  return status;
}
/*! ======================================================================
  @brief    method returns Major and Minor Type for the Clip.

  @param[in]   id           track ID
  @param[out]  majorType   Major Media Types ( Audio/Video/Text).
  @param[out]  minorType   Minor Media Types ( Sub media type within Audio/Video text)
                           They are also referred as Codecs
  @return      filesource status.

  @note    It is expected that OpenFile() is called before getMimeType() is called.
========================================================================== */

FileSourceStatus FileSource::GetMimeType (uint32 id,
                                          FileSourceMjMediaType& majorType,
                                          FileSourceMnMediaType& minorType )
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    status = m_pHelper->GetMimeType(id,majorType,minorType);
  }
  return status;
}

/*! ======================================================================
   @brief    Provides Media Samples for requested tracks

   @param[in]  id    The track ID of the track from which the method is to retrieve the samples.
   @param[out] buf   A pointer to the buffer into which to place the sample.
   @param[out] size  The size of the data buffer.
   @param[out] pSampleInfo Provides Information( eg: timestamp etc) about the sample
   @return     The size in bytes of the data placed into the provided buffer.
               If the buffer is not large enough, the return value is the negative
               of the size that is needed .

   @note    It is expected that OpenFile() is called before getNextMediaSample() is called.
========================================================================== */

FileSourceMediaStatus FileSource::GetNextMediaSample(uint32 id, uint8 *buf,
                                                     uint32 *size,
                                                     FileSourceSampleInfo& pSampleInfo)
{
  FileSourceMediaStatus status = FILE_SOURCE_DATA_ERROR;
  if(m_pHelper)
  {
    status = m_pHelper->GetNextMediaSample(id,buf,size,pSampleInfo);
  }
  return status;
}
/*! ======================================================================================================
   @brief    Provides the buffered duration of a track at any point of time during progressive download

   @param[in]  id    The track ID.
   @param[out] pDuration Function will copy buffered duration into this variable
   @return     FILE_SOURCE_SUCCESS if track is valid else returns FILE_SOURCE_FAIL

   @note    It is expected that OpenFile() is called before getBufferedDuration() is called.
=========================================================================================================*/
FileSourceStatus FileSource::GetBufferedDuration(uint32 id, uint64 *pDuration)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  if(m_pHelper)
  {
    int64 nDownloadedBytes = -1;
    status = m_pHelper->GetBufferedDuration(id,nDownloadedBytes,pDuration);
  }
#else
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD not enabled");
  status = FILE_SOURCE_NOTAVAILABLE;
#endif
  return status;
}

/*! ======================================================================================================
   @brief     Provides the playback time for buffered/downloaded bytes of particular track during PD

   @param[in]      nTrackID  The track ID.
   @param[in]      nBytes    Buffered/Downloaded bytes
   @param[out]     pDuration Filled in by FileSource to provided playtime for buffered/downloaded bytes
   @return    FILE_SOURCE_SUCCESS if track is valid else returns FILE_SOURCE_FAIL

   @note      It is expected that OpenFile() is called before getBufferedDuration() is called.
=========================================================================================================*/
FileSourceStatus FileSource::GetBufferedDuration(uint32 nTrackID, int64 nBytes, uint64 *pDuration)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  if(m_pHelper)
  {
    status = m_pHelper->GetBufferedDuration(nTrackID, nBytes, pDuration);
  }
#else
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "FileSource::FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD not enabled");
  status = FILE_SOURCE_NOTAVAILABLE;
#endif
  return status;
}

/*! ======================================================================
   @brief    Proves Current Position in milli seconds

   @detail    This function retrieves the current position without you having to call
      the getNextMediaSample().

   @param[in]  id    The track ID of the track
   @return     returns the timestamp in milliseconds of the next valid sample.

   @note     It is expected that OpenFile() is called before getMediaCurrentPosition() is called.
========================================================================== */

uint64 FileSource::GetMediaCurrentPosition(uint32 id)
{
  uint64 StartTime = 0;
  if(m_pHelper)
  {
    StartTime = m_pHelper->GetMediaCurrentPosition(id);
  }
  return StartTime;
}


/*! ======================================================================
  @brief    Proves Current Position in milli seconds for the entire track

   @detail    This function retrieves the current position without you having to call
             the getNextMediaSample(). This function gives preference to video track
       position over audio and audio over text to calculate the current position

   @return     returns the timestamp in milliseconds of the next valid sample.


   @note     It is expected that OpenFile() is called before getMediaCurrentPosition() is called.
  ========================================================================== */

uint64 FileSource::GetMediaCurrentPosition()
{
  uint64 time = 0;

  if(m_pHelper)
  {
    time = m_pHelper->GetMediaCurrentPosition();
  }
  return time;
}
/*! ======================================================================
  @brief      Method to retrieve Stream specific parameters.

  @param[in]      ulTrackId     Track id to identify stream.
  @param[in]      ulParamIndex  Parameter Index to identify structure'
  @param[in]      pParamStruct  Pointer to the structure.

  @return         file source status
  @note   It is expected that user has received the successul callback for
          OpenFile() earlier.
========================================================================== */

FileSourceStatus FileSource::GetStreamParameter(uint32 ulTrackId,
                                                uint32 ulParamIndex,
                                                void* pParamStruct)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    status = m_pHelper->GetStreamParameter(ulTrackId, ulParamIndex,
                                           pParamStruct);
  }
  return status;
}
/*! ======================================================================
  @brief      Method to retrieve configuration item set previously via SetConfiguration.

  @param[in]      id              Track id to identify track to which configuration data belongs.
  @param[out]     pItem            Configuration data filled in by parser
  @param[in]      ienumData     Identifies the configuration item.
                                Please refer to FileSourceConfigItemEnum.
  @return         file source status
  @note   It is expected that user has received the successul callback for OpenFile() earlier.
========================================================================== */

FileSourceStatus FileSource::GetConfiguration(uint32 id,FileSourceConfigItem* pItem, FileSourceConfigItemEnum ienumData)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    status = m_pHelper->GetConfiguration(id,pItem,ienumData);
  }
  return status;
}
/*! ======================================================================
  @brief      Method to Set configuration item.

  @param[in]      id              Track id to identify track to which configuration data belongs.
  @param[out]     pItem            Configuration data filled in by caller
  @param[in]      ienumData     Identifies the configuration item.
                                Please refer to FileSourceConfigItemEnum.
  @return         file source status
  @note   It is expected that user has received the successul callback for OpenFile() earlier.
========================================================================== */

FileSourceStatus FileSource::SetConfiguration(uint32 id,FileSourceConfigItem* pItem, FileSourceConfigItemEnum ienumData)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    status = m_pHelper->SetConfiguration(id,pItem,ienumData);
  }
  return status;
}
/*! ======================================================================
   @brief      method returns metadata about the Clip.

   @details    In case of MPEG4 file format it retrieves the information from the
               PVUserDataAtom 'pvmm' atom.

   @param[in]  ienumData      Identifies requested metadata type( eg: Title, Author etc).
   @param[out] oMetaData        returns a string associated with requested metadata
   @return     file source status

   @note    It is expected that OpenFile() is called before getClipMetaData() is called.
========================================================================== */

FileSourceStatus FileSource::GetClipMetaData(wchar_t* pMetaData,
                                             uint32* pLength,
                                             FileSourceMetaDataType ienumData,
                                             FS_TEXT_ENCODING_TYPE* peEncodeType)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    status = m_pHelper->GetClipMetaData(pMetaData,pLength,ienumData, peEncodeType);
  }
  return status;
}
/*! ======================================================================
   @brief      method to retrieve all valid trackID List

   @param[out]  trackIdInfo   a list of TrackIfInfo (
                consisting of trackid's and if a book if they are selected or not).
   @return     number of valid audio, video and text tracks.

   @note    It is expected that OpenFile() is called before getWholeTracksIDList() is called.
========================================================================== */

uint32 FileSource::GetWholeTracksIDList(FileSourceTrackIdInfoType *trackIdInfo )
{
  uint32 totalNumTracks = 0;
  if(m_pHelper)
  {
    totalNumTracks = m_pHelper->GetWholeTracksIDList(trackIdInfo);
  }
  return totalNumTracks;
}
/*! ======================================================================
   @brief      method for User to Select a Track.

   @param[in]  id   track Id to select
   @return     file source status.

   @note    It is expected that OpenFile() is called before setSelectedTrackID() is called.
========================================================================== */

FileSourceStatus FileSource::SetSelectedTrackID(uint32 id)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    status = m_pHelper->SetSelectedTrackID(id);
  }
  return status;
}

/*! ======================================================================
  @brief      Provides information about the Track.

   @detail    method to retrieve information about the Audio/Video Track. This Interface is provided
              for User to do channel selection OR use it as a criteria to select a particular track.

   @param[in]  id   track Id to select
   @param[out] info Information about the given track id
   @return     FILE_SOURCE_SUCCESS if track is valid else returns FILE_SOURCE_FAIL

   @note    It is expected that OpenFile() is called before getMediaTrackInfo() is called.
========================================================================== */
FileSourceStatus FileSource::GetMediaTrackInfo(uint32 id,MediaTrackInfo* info)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    status = m_pHelper->GetMediaTrackInfo(id,info);
  }
  return status;
}
/*! ======================================================================
   @brief      Provides information about the Track.

   @details    Method to set information about the Audio/Video Track.

   @param[in] info Information about the codec/track
   @return     FILE_SOURCE_SUCCESS if successful in setting the information otherwise returns appropriate error.

   @note       It is expected that user has called OpenFile with FILE_SOURCE_RAW and has
               received OPEN_COMPLETE before invoking this API.

========================================================================== */
FileSourceStatus FileSource::SetMediaTrackInfo(MediaTrackInfo info)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    status = m_pHelper->SetMediaTrackInfo(info);
  }
  return status;
}
/*! ======================================================================
   @brief      Maximum Buffer size required for the Track.

   @detail    Before we parse a clip we do not know the size of the Frame.
              There are two ways to solve this issue. One is to allocate a huge memory buffer,
              which is in-efficient use of memory OR use this method to retrieve the
              buffer size needed for the frame and then allocate/reallocate memory as needed.

   @param[in]  id   track Id to select.

   @return  largest frame size up to the frame we have parsed.
      (Note:we do not parse/scan all the frames during initial parsing).

   @note    It is expected that OpenFile() is called before getTrackMaxFrameBufferSize() is called.
========================================================================== */

int32  FileSource::GetTrackMaxFrameBufferSize(uint32 id)
{
  int32 n = 0;
  if(m_pHelper)
  {
    n = m_pHelper->GetTrackMaxFrameBufferSize(id);
  }
  return n;
}


/*! ======================================================================
   @brief      gives the duration of the track in milliseconds.

   @detail    This method retrives the duration from  the track.

   @param[in]  id   track Id to select.
   @return     duration of the track in milli seconds

   @note    It is expected that OpenFile() is called before getClipDuration() is called.
========================================================================== */

uint64  FileSource::GetTrackMediaDuration(uint32 id)
{
  uint64 trackDuration = 0;
  if(m_pHelper)
  {
    trackDuration =  m_pHelper->GetTrackMediaDuration(id);
  }
  return trackDuration;
}

/*! ======================================================================
   @brief      gives the duration of the Clip in milliseconds.

   @detail    This method retrives the duration from all the tracks ( audio, video and text).
              and picks the maximum.
   @return     duration of the clip in milli seconds

   @note    It is expected that OpenFile() is called before getClipDuration() is called.
========================================================================== */

uint64  FileSource::GetClipDuration()
{
  uint64 clipDuration =0;
  if(m_pHelper)
  {
    clipDuration = m_pHelper->GetClipDuration();
  }
  /* return duration in ms */
  return clipDuration;
}


/*! ==============================================================================================
   @brief      retrieve the Format Block information about the track.

   @detail    method to retrieve the Decoder specific/Format Block information from the track.
              This interface is generic for Audio, Video and Text.
              If buf = NULL, then the function give the size of the required buffer.
              Following is an example of retrieving the format block.

              1.Invoke getFormatBlock API for a given track identifier by passing in NULL for buf.
              2.If a track is valid, *pbufSize will give you the size of format block.
              3.Allocate the memory and invoke getFormatBlock API for a given track identifier
                by passing handle to allocated memory.

   @param[in]   id   track Id to select.
   @param[out]  buf   Buffer provies the format block info to the caller
   @param[out]  pBufSize   Size of the FormatBlock buffer
  @param[in]    bRawCodec  Flag to indicate whether codec data required in input format
                           or converted to proper SPS/PPS format

   @return     file source status.

   @note    It is expected that OpenFile() is called before getFormatBlock() is called.
  =================================================================================================*/

FileSourceStatus FileSource::GetFormatBlock(uint32 id,uint8* buf,
                                            uint32 *pbufSize, bool bRawCodec)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    status = m_pHelper->GetFormatBlock(id, buf, pbufSize, bRawCodec);
  }
  return status;
}
/*! ======================================================================
   @brief      Reposition to an absolute position.

   @detail    The routine will seek/repositon to next valid sample based on the timestamp
              provided. This function Uses Video track as the primary source to find the time stamp and
              then sync's the Audio and Text accordingly. However, if Audio is not present it
              will use Audio as the reference.

   @param[in]  tAbsoluteTime Seek to the absolute position(ms).
   @param[in]  bSeekToSync   When set to false, parser can seek to non sync frame
   @param[in]  nCurrPlayTime Current playback time.(-1 indicates time unknown)
   @return     file source status.

   @note    It is expected that OpenFile() is called before SeekAbsolutePosition() is called.
            bSeekToSync is only applicable if there is video track as for audio,
            every frame is a sync frame.
========================================================================== */

FileSourceStatus FileSource::SeekAbsolutePosition( const int64 tAbsoluteTime,
                                                  bool bSeekToSync,
                                                  int64 nCurrPlayTime)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    status = m_pHelper->SeekAbsolutePosition(tAbsoluteTime,bSeekToSync,nCurrPlayTime);
  }
  return status;
}
/*! ======================================================================
   @brief      Reposition given track to an absolute position.

   @detail    This API will try to seek the track identified via trackid to
              given absolute time provided there is an I frame at that time.
              If there are no I frames in forward direction, forward seek
              request can fail. To allow parser to seek to non key frame,
              bSeekToSync can be set to false.

   @param[in]  trackid       Track-Id to specify which track to seek
   @param[in]  tAbsoluteTime Seek to the absolute position(ms).
   @param[in]  bSeekToSync   When set to false, parser can seek to non sync frame
   @param[in]  nCurrPlayTime Current playback time.(-1 indicates time unknown)
   @return     file source status.

   @note    It is expected that OpenFile() is called before SeekAbsolutePosition() is called.
            bSeekToSync is only applicable if there is video track as for audio,
            every frame is a sync frame.
========================================================================== */

FileSourceStatus FileSource::SeekAbsolutePosition( int64 trackid,
                                                   const int64 tAbsoluteTime,
                                                   bool bSeekToSync,
                                                   int64 nCurrPlayTime)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    status = m_pHelper->SeekAbsolutePosition(trackid,tAbsoluteTime,bSeekToSync,nCurrPlayTime);
  }
  return status;
}
/*! ======================================================================
   @brief      Reposition to a relative Sync Sample/number of sync sample.

   @detail    The routine will seek/repositon to next sync sample based on the timestamp
              provided. The direction can be both positive and negative.

   @param[in]   currentPlaybacktime current playback time(ms) from the point of view of the User/Caller.
   @param[in]   numSync number of sync sample to jump. The value can be both positive and negative
               , specifiying the direction to search for the Sync Sample.
   @return     file source status.

   @note    It is expected that OpenFile() is called before SeekRelativeSyncPoint() is called.
========================================================================== */

FileSourceStatus FileSource::SeekRelativeSyncPoint( uint64 currentPlaybacktime, const int numSync)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    status = m_pHelper->SeekRelativeSyncPoint(currentPlaybacktime,numSync);
  }
  return status;
}
/*! ======================================================================
   @brief      Retrieves clip specifc DRM information

   @param[in/out]   pointer to retrieve the Clip specific DRM information. Memory should
          be allocated before the pointer is passed to this function.

   @note    It is expected that OpenFile() is called before GetClipDrmInfo() is called.
========================================================================== */
#ifdef FEATURE_FILESOURCE_DIVX_DRM
FileSourceStatus FileSource::GetClipDrmInfo(ClipDrmInfoT* pDrmInfo)
#else
FileSourceStatus FileSource::GetClipDrmInfo(ClipDrmInfoT*)
#endif
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
#ifdef FEATURE_FILESOURCE_DIVX_DRM
  if(m_pHelper)
  {
    status =  m_pHelper->GetClipDrmInfo(pDrmInfo);
  }
#endif
  return status;
}
/*! ======================================================================
   @brief   function to findout if the clip is DRM protected or not.

   @return    True if file is protected else returns false.

   @note    It is expected that OpenFile() is called before IsDRMProtection() is called.
========================================================================== */

bool FileSource::IsDRMProtection()
{
  bool status = false;
  if(m_pHelper)
  {
    status = m_pHelper->IsDRMProtection();
  }
  return status;
}

/*! ======================================================================
   @brief   function to register DRM Decryption function

   @return  True if successful else returns false.

   @note    It is expected that OpenFile() is called before
            RegisterDRMDecryptMethod is called.
========================================================================== */

 bool FileSource::RegisterDRMDecryptMethod(DRMDecryptMethodT pDecrypt,
                                           void*             pClientData)
 {
   bool status = false ;

   if (m_pHelper)
   {
     status = m_pHelper->RegisterDRMDecryptMethod(pDecrypt, pClientData);
   }

   return status;
 }
/*!
  @brief   Function to get the device registration code.

  @return    True if successful in retrieving the code else returns false.

  @note    This API needs to be called in 2 step process.
           Call with NULL as first parameter and API will return the size needed for registration code.
           Allocate the memory and call again to get the registration code.
*/
bool FileSource::GetRegistrationCode(char* ptr,int* plength)
{
  bool status = false;
  if(m_pHelper)
  {
    status = m_pHelper->GetRegistrationCode(ptr,plength);
  }
  return status;
}
/*! ======================================================================
  @brief Retrieve the wma codec data needed to configure WMA decoder.

  @param[in] id   Identifies the WMA track for which codec data needs to be retrieved
  @param[in,out] pBlock filled in by FileSource

  @return true is successful in retrieving the codec data else returns false
========================================================================== */
bool FileSource::GetWMACodecData(uint32 id,WmaCodecData* pCodecData)
{
  bool bRet = false;
#ifdef FEATURE_FILESOURCE_WINDOWS_MEDIA
  if(m_pHelper)
  {
    bRet = m_pHelper->GetWMACodecData(id,pCodecData);
  }
#endif
  return bRet;
}
/*! ======================================================================
  @brief Retrieve the wav codec data needed to configure wav decoder.

  @param[in] id   Identifies the wav track for which codec data needs to be retrieved
  @param[in,out] pBlock filled in by FileSource

  @return true is successful in retrieving the codec data else returns false
========================================================================== */
bool FileSource::GetWavCodecData(uint32 id,WavFormatData* pCodecData)
{
  bool bRet = false;
  if(m_pHelper)
  {
    bRet = m_pHelper->GetWavCodecData(id,pCodecData);
  }
  return bRet;
}
/*!
@brief Retrieve the AAC codec data needed to configure AAC decoder

@param[in] id   Identifies the AAC track for which codec data needs to be retrieved
@param[in,out] pCodecData filled in by FileSource

@return true is successful in retrieving the codec data else returns false
*/
bool FileSource::GetAACCodecData(uint32 id,AacCodecData* pCodecData)
{
   bool bRet = false;
   if (m_pHelper)
   {
     bRet = m_pHelper->GetAACCodecData(id,pCodecData);
   }
   return (bRet);
}
/*!
@brief Retrieve the Flac codec data needed to configure FLAC decoder

@param[in] id   Identifies the FLAC track for which codec data needs to be retrieved
@param[in,out] pCodecData filled in by FileSource

@return true is successful in retrieving the codec data else returns false
*/
bool FileSource::GetFlacCodecData(uint32 id,FlacFormatData* pCodecData)
{
   bool bRet = false;
   if (m_pHelper)
   {
     bRet = m_pHelper->GetFlacCodecData(id,pCodecData);
   }
   return (bRet);
}

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
/*! ======================================================================
@brief Returns absolute file offset(in bytes) associated with time stamp 'pbtime'(in milliseconds).

  @param[in]  pbtime Timestamp(in milliseconds) of the sample that user wants to play/seek
  @param[out] offset Absolute file offset(in bytes) corresponding to 'pbtime'

  @return true if successful in retrieving the absolute offset(in bytes) else returns false

  @note When there are multiple tracks in a given clip(audio/video/text),API returns
   minimum absolute offset(in bytes)among audio/video/text tracks.
   API is valid only for non-fragmented clips.
========================================================================== */
bool FileSource::GetOffsetForTime(uint64 pbtime, uint64* offset)
{
  bool bRet = false;
  if(m_pHelper)
  {
    bRet = m_pHelper->GetOffsetForTime(pbtime,offset);
  }
  return bRet;
}
#endif
/*! ======================================================================
  @brief Returns absolute file offset(in bytes)
         corresponding to current sample being retrieved via getNextMediaSample.

  @param[in]  trackid TrackId identifying the media track
  @param[out] bError  Indicates if error occured while retrieving file offset.

  @note       bError is set to true when error occurs otherwise, set to false.
  @return Returns absolute file offset(in bytes) that corresponds to
          current sample retrieved via getNextMediaSample.
========================================================================== */
uint64 FileSource::GetLastRetrievedSampleOffset(uint32 trackid,bool* bError)
{
  uint64 soffset = 0;
  if(m_pHelper)
  {
    soffset = m_pHelper->GetLastRetrievedSampleOffset(trackid,bError);
  }
  return soffset;
}
/*! ======================================================================
  @brief Returns total number of views associated with given trackid.
         Default value is always 1 for any given track unless the track has MVC(MultiViewCoding).

  @param[in]  trackid TrackId identifying the media track
  @return     Returns total number of views for given trackid
  @note       It is expected that user has received the successul callback for OpenFile() earlier.
========================================================================== */
uint16 FileSource::GetNumberOfViews(uint32 trackid)
{
  uint16 nviews = 0;
  if(m_pHelper)
  {
    nviews = m_pHelper->GetNumberOfViews(trackid);
  }
  return nviews;
}
/*! ======================================================================
    @brief   Commit the DivX Playback.

    @param[in]    Major media type
    @return        true if commit is successful

    @note    It is expected that OpenFile() is called before CommitDivXPlayback() is called.
========================================================================== */
#ifdef FEATURE_FILESOURCE_DIVX_DRM
bool FileSource::CommitDivXPlayback(FileSourceMjMediaType majorType)
#else
bool FileSource::CommitDivXPlayback(FileSourceMjMediaType)
#endif
{
  bool status = false;
#ifdef FEATURE_FILESOURCE_DIVX_DRM
  if(m_pHelper)
  {
    status = m_pHelper->CommitDivXPlayback(majorType);
  }
#endif
  return status;
}
/*! ======================================================================
    @brief   Returns current source error, if any.

    @return        error code status of underlying file source

    @note    It is expected that OpenFile() is called before GetFileError() is called.
========================================================================== */
FileSourceParserError FileSource::GetFileError()
{
  FileSourceParserError status = FILE_SOURCE_PARSER_UNKNOWN_ERROR;
  if(m_pHelper)
  {
    status = m_pHelper->GetFileError();
  }
  return status;
}

/*! ======================================================================
  @brief  local function to check if Seek is allowed in the clip

  @param[in]  none
  @return     TRUE or FALSE
  @note   It is expected that OpenFile() is called before getIndexInTrackIdInfo() is called.
========================================================================== */

uint8 FileSource::IsSeekDenied()
{
  uint8 ret = 0;
  if(m_pHelper)
  {
    ret = m_pHelper->IsSeekDenied();
  }
  return ret;
}

/*! ==================================================================================================================
  @brief          Quick way of validating if given File belongs to certain format without creating FileSource instance.
                  To speed up the checking, this function will only run check for desired file format.

  @param[in]      FormatToCheck Enum specifying format to be checked
  @param[in]      pInputBuffer buffer containing data from the beginning of the file/clip
                  If NULL, function will put required number of bytes for particular format check in pBufferSize.
  @param[in/out]  pBufferSize user will pass size of pInputBuffer. Function will updated required size if given size if not enough
  @return         Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have enough data. Then pBufferSize will
                  be updated to have required size.
                  Returns FILE_SOURCE_INVALID if arguments are not valid
  @note           Utility function. No need to create FileSource instance.
======================================================================================================================== */
FileSourceStatus FileSource::CheckFileFormat(FileSourceFileFormat FormatToCheck,
                                             uint8 *pInputBuffer,
                                             uint32 *pBufferSize)
{
  FileSourceStatus ReturnVal = FILE_SOURCE_NOTAVAILABLE;

  if( !pBufferSize )
  {
    return FILE_SOURCE_INVALID;
  }

  switch(FormatToCheck)
  {
    case FILE_SOURCE_RAW:      //Special case. File might be having raw pcm/aac etc.
      ReturnVal = FILE_SOURCE_NOTAVAILABLE;
      break;

    case   FILE_SOURCE_AC3:      //.ac3
      ReturnVal = FileSourceHelper::ValidateAC3File(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_DTS:      //.dts, .dtshd, .cpt
      ReturnVal = FileSourceHelper::ValidateDTSFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_AAC:      //.aac
      ReturnVal = FileSourceHelper::ValidateAACFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_AMR_NB:   //.amr
      ReturnVal = FileSourceHelper::ValidateAMRFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_AMR_WB:   //.awb
      ReturnVal = FileSourceHelper::ValidateAMRWBFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_ASF:      //.wma,.wmv,.asf
      ReturnVal = FileSourceHelper::ValidateASFFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_AVI:      //.avi,.divx
      ReturnVal = FileSourceHelper::ValidateAVIFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_EVRCB:    //.evb
      ReturnVal = FileSourceHelper::ValidateEVRCBFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_EVRC_WB:  //.ewb
      ReturnVal = FileSourceHelper::ValidateEVRCWBFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_FLAC:     //.flac
      ReturnVal = FileSourceHelper::ValidateFLACFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_MPEG4:    //.3gp,.mp4,.k3g,.skm,.mp4a
      ReturnVal = FileSourceHelper::ValidateMP4File(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_3G2:      //.3g2
      ReturnVal = FileSourceHelper::Validate3GPP2File(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_MP2PS:      //.vob, .mp2, .ps, .mpg
      ReturnVal = FileSourceHelper::ValidateVOBFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_MP2TS:    //.ts,.m2ts
    case   FILE_SOURCE_WFD_MP2TS:
    case   FILE_SOURCE_DASH_MP2TS:
      ReturnVal = FileSourceHelper::ValidateTSFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_MKV:      //.mkv
      ReturnVal = FileSourceHelper::ValidateMKVFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_MP3:      //.mp3
      ReturnVal = FileSourceHelper::ValidateMP3File(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_OGG:      //.ogg
      ReturnVal = FileSourceHelper::ValidateOGGFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_QCP:      //.qcp
      ReturnVal = FileSourceHelper::ValidateQCPFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_WAV:      //.wav
      ReturnVal = FileSourceHelper::ValidateWAVFile(pInputBuffer,pBufferSize);
      break;

    case   FILE_SOURCE_FLV:      //.mkv
      ReturnVal = FileSourceHelper::ValidateFLVFile(pInputBuffer,pBufferSize);
      break;

    default:
      ReturnVal = FILE_SOURCE_NOTAVAILABLE;
      break;
  };
  return ReturnVal;
}

 /*!===========================================================================
  @brief     Retrieve protection system specific information of supported
             DRM systems.

  @details        This API will provides content protection system specific
                  information applicable to supported DRM system in same media
                  file.It contains SystemID of the DRM system along with size of
                  content protection system specific data for each supported
                  DRM in sequential order.

  @param[in]      ulTrackID TrackId identifying the media track
  @param[out]     pPSSHInfo  Array of protection system specific information
                  Reference : FS_PROTECTION_SYSTEM_SPECIFIC_INFOTYPE

  @return         Returns FILE_SOURCE_SUCCESS if PSSH INFO available
                  Returns FILE_SOURCE_FAIL if error i.e. bad input parameter
                  /internal error.
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if PSSH info not available.

  @note           It is expected that user has received the successful callback
                  for OpenFile().
                  GetConfiguration()API return number of supported count if
                  quired index is FILE_SOURCE_MEDIA_NUM_DRM_SYSTEM_SUPPORTED.
                  FS client has to allocate memory to hold pPSSHInfo of size
                  (NoOfDRM x sizeof(FS_PROTECTION_SYSTEM_SPECIFIC_INFOTYPE).
                  FileSource fill data sequentially in it for all supported DRM.
  ==============================================================================*/
FileSourceStatus FileSource::GetProtectionSystemSpecificInfo(
  uint32 ulTrackID,
  FS_PROTECTION_SYSTEM_SPECIFIC_INFOTYPE* pPSSHInfo
 )
{
  FileSourceStatus eStatus = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    eStatus = m_pHelper->GetStreamParameter(ulTrackID,
                                            FS_IndexParamOtherPSSHInfo,
                                            pPSSHInfo);
  }
  return eStatus;
}

 /*!===========================================================================
  @brief     Retrieve protection system specific data supported by particular DRM

  @details        This API will provides content protection system specific
                  data needed to playback content. Data encapsulated in the
                  PSSHData/KIDData field will enable DRM system for decryption
                  key acquisition and decryption of media data.

  @param[in]      ulTrackID TrackId identifying the media track
  @param[out]     pPSSHData Content protection system specific data. Reference:
                  FS_PROTECTION_SYSTEM_SPECIFIC_DATATYPE

  @return         Returns FILE_SOURCE_SUCCESS if PSSH INFO available
                  Returns FILE_SOURCE_FAIL if error i.e. bad input parameter
                  /internal error.
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if PSSH data not available.


  @note      It is expected that user has received the successul callback for OpenFile().
             GetProtectionSystemSpecificInfo() API called prior calling this
             API to get DRM system data associated with DRMIndex.
             FS Client need to pass proper DRMIndex to get particular DRM
             system specific data.
  ==============================================================================*/
FileSourceStatus FileSource::GetProtectionSystemSpecificData(
  uint32 ulTrackID,
  FS_PROTECTION_SYSTEM_SPECIFIC_DATATYPE* pPSSHData
 )
{
  FileSourceStatus eStatus = FILE_SOURCE_FAIL;
  if(m_pHelper)
  {
    eStatus = m_pHelper->GetStreamParameter(ulTrackID,
                                            FS_IndexParamOtherPSSHData,
                                            pPSSHData);
  }
  return eStatus;
}

