#ifndef __FileSource_H__
#define __FileSource_H__
/*=======================================================================
 *                             FileSource.h
 *DESCRIPTION
 * Definiton of interfaces for Filesource Module.
 *
 * Copyright (c) 2008-2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary
 *======================================================================== */

/* =======================================================================
                             Edit History
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileSource/main/latest/inc/filesource.h#64 $$
$DateTime: 2013/08/28 01:52:54 $
$Change: 4345281 $
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "filesourcetypes.h"
/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/*! @brief FileSource Notification Callback datatype */
typedef void (*FileSourceCallbackFuncType)(FileSourceCallBackStatus status,
                                           void * pClientData);

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */


/* =======================================================================
**                        Class Declarations
** ======================================================================= */

/*!
  @brief    FileSource Module.

  @details  FileSource is an API on top of all the core parsers to access each parser in an uniform way.
            Client can use these APIs without worrying about underlying file format.
            User can access individual samples/frame for audio/video/subtitle tracks that may exist in given file/stream.
            It allows capability to seek individual track, retrieve clip meta data such as author,title, copyright etc.

  @note     In order to use the functions provided by this class, it is required
            that the user calls one of the OpenFile APIs and has received successful callback for it.
*/

class FILESOURCE_DLL FileSource
{
public:

  //! constructor with Callback Function as input.This callback is invoked whenever an asynchronous operation is completed
  //! in filesource thread's context.
  FileSource(FileSourceCallbackFuncType callBack, void * pClientData, bool bAsync = true);

  //! Destructor for filesource
  virtual ~FileSource();

  /*!
  @brief  Open a File.

  @details    This is an asynchronous call and needs to be invoked when user wants to play from file.
              FileSource  Module determines the core parser to be instantiated by reading first few bytes from the file.
              Once the core parser is identified, the parser will be instantiated and the callback will be triggered.

  @param[in]  audioFilename name of the audio file with audio track.
  @param[in]  videoFilename name of the video file with video track.
  @param[in]  textFilename  name of the text file with text track.
  @param[in]  format        If file format is already known,caller
                            can pass in the information into FileSource.
                            When format != FILE_SOURCE_UNKNOWN,FileSource
                            will instantiate the given parser without doing any
                            file format check.
  @param[in]  blookforcodecconfig  When true, parser returns OPEN_COMPLETE only when
                                   codec configuration header has been located for audio/video/text track
                                   When false, OPEN_COMPLTE is reported as soon as all valid tracks are located.
  @return     FILE_SOURCE_SUCCESS indicating file name is accepted by filesource for parsing.
              The callback registered in the filesource constructor will be invoked later with either
              FILE_SOURCE_OPEN_COMPLETE or FILE_SOURCE_OPEN_FAIL depending on whether the given file
              belongs to any known file format or not.
  @note       It is expected that when this function is called atleast one of the
              parameters is not NULL. i.e, atleast one filename is passed.
  */
  FileSourceStatus  OpenFile(  wchar_t* audioFilename = NULL,
                               wchar_t* videoFilename = NULL,
                               wchar_t* textFilename  = NULL,
                               FileSourceFileFormat format = FILE_SOURCE_UNKNOWN,
                               bool blookforcodecconfig = true
                            );

  /*!
  @brief  Buffer playback.

  @details    This is an asynchronous call and needs to be invoked when user wants to play from buffers.
              It's possible to play audio/video/text from separate buffers or user can pass in the same buffer for audio/video/text.
              FileSource  Module determines the core parser to be instantiated by reading first few bytes from the buffer.
              Once the core parser is identified, the parser will be instantiated and the callback will be triggered.

  @param[in]  pAudioBuf     Buffer to play audio.
  @param[in]  nAudioBufSize Total size of audio buffer
  @param[in]  pVideoBuf     Buffer to play video.
  @param[in]  nVideoBufSize Total size of video buffer
  @param[in]  pTextBuf      Buffer to play text.
  @param[in]  nTextBufSize  Total size of text buffer
  @param[in]  format        If file format is already known,caller
                            can pass in the information into FileSource.
                            When format != FILE_SOURCE_UNKNOWN,FileSource
                            will instantiate the given parser without doing any
                            file format check.
  @return     FILE_SOURCE_SUCCESS indicating buffer is accepted by filesource for parsing.
              The callback registered in the filesource constructor will be invoked later with either
              FILE_SOURCE_OPEN_COMPLETE or FILE_SOURCE_OPEN_FAIL depending on whether the bufer content
              belongs to any known file format or not.

  @note       It is assumed that these buffers will be valid through out the playback
              and data bytes are stored at contiguous address. FileSource module
              can request the data within the valid offsets range at anytime.
              It is expected that when this function is called atleast one of the
              parameters is not NULL. i.e, atleast one buffer/size is not NULL.
  */
  FileSourceStatus  OpenFile(  unsigned char* pAudioBuf = NULL,
                               uint32         nAudioBufSize = 0,
                               unsigned char* pVideoBuf = NULL,
                               uint32         nVideoBufSize = 0,
                               unsigned char* pTextBuf  = NULL,
                               uint32         nTextBufSize = 0,
                               FileSourceFileFormat format = FILE_SOURCE_UNKNOWN
                              );
  /*!
  @brief   This is an asynchronous call and needs to be invoked when user wants to play from iStreamPort.
  FileSource Module determines the core parser to be instantiated by reading first few bytes from iStreamPort.
  Once the core parser is identified, the parser will be instantiated and the callback will be triggered.
  Please refer to DataSourcePort.h for iStreamPort details.

  @details    iStreamPort is an interface which provides basic APIs for read/write/seek etc.

  @param[in]  pInputStream   Input iStreamPort of Audio/video/text tracks
  @param[in]  format         If file format is already known,caller
                             can pass in the information into FileSource.
                             When format != FILE_SOURCE_UNKNOWN,FileSource
                             will instantiate the given parser without doing any
                             file format check.
  @param[in]  blookforcodecconfig  When true, parser returns OPEN_COMPLETE only when
                                   codec configuration header has been located for audio/video/text track
                                   When false, OPEN_COMPLTE is reported as soon as all valid tracks are located.
  @return     FILE_SOURCE_SUCCESS indicating istreamport is accepted by filesource for parsing.
              The callback registered in the filesource constructor will be invoked later with either
              FILE_SOURCE_OPEN_COMPLETE or FILE_SOURCE_OPEN_FAIL depending on whether the bufer content
              belongs to any known file format or not.

  @note      It is expected that pInputStream is  not  NULL.
  */
  FileSourceStatus OpenFile(video::iStreamPort* pInputStream = NULL,
                            FileSourceFileFormat format = FILE_SOURCE_UNKNOWN,
                            bool blookforcodecconfig = true);

  /*!
  @brief    This is an asynchronous call and needs to be invoked when user wants to play from IxStream.
  FileSource Module determines the core parser to be instantiated by reading first few bytes from IxStream.
  Once the core parser is identified, the parser will be instantiated and the callback will be triggered.

  @details    IxStream is an interface to support playback of OMA/JANUS DRM content.
              It provides APIs to read/seek/decrypt the content.

  @param[in]  pInputStream   Input IxStream handle
  @return     FILE_SOURCE_SUCCESS indicating ixstream is accepted by filesource for parsing.
              The callback registered in the filesource constructor will be invoked later with either
              FILE_SOURCE_OPEN_COMPLETE or FILE_SOURCE_OPEN_FAIL depending on whether the bufer content
              belongs to any known file format or not.
  @note       It is expected that pInputStream is not NULL.
  */
  FileSourceStatus OpenFile(   IxStream* pInputStream = NULL);

  /*!
  @brief   To be called when user is done with using the filesource object.

  @details     Please see the note below.

  @return     filesource status.

  @note       Once close is called, FileSource object will no longer support any API except OpenFile.
              It's a blocking call (in caller's context) untill CloseFile is processed in FileSource Thread.
  */
  FileSourceStatus  CloseFile();

  /*!
  @brief    API returns the underlying file format.

  @param[out]     fileFormat   type of  FileFormat.
                  Please refer to FileSourceFileFormat enum.
  @return         filesource status.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */
  FileSourceStatus GetFileFormat(FileSourceFileFormat& fileFormat);

  /*!
  @brief    API returns the DRM scheme used, if any

  @param[out]  drmtype   type of  DRM
               Please refer to FileSourceDrmType enum.
  @return      filesource status.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */
  FileSourceStatus GetDRMType(FileSourceDrmType& drmtype);

  /*!
  @brief    API to retrieve JANUS DRM information/header.

  @param[in/out]  nsize   Size of the drm header/information.
  @param[out]     pdata   points to memory to be filled in by filesource
  @return         filesource status.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
           Call the API with NULL pdata to get the size needed for storing drm header/info.
           User needs to allocate the memory and call again with valid memory address.
  */
  FileSourceStatus GetJanusDRMInfo(void* pdata,uint32* nsize);

  /*!
  @brief    method returns Major and Minor Type for the given track-id.

  @param[in]      id              Track identifier for which mime type is needed
  @param[out]     majorType   Major Media Types ( Audio/Video/Text).
  @param[out]     minorType   Minor Media Types ( Sub media type within Audio/Video text)
                              They are also referred as Codecs.
                              Please refer to FileSourceMjMediaType and FileSourceMnMediaType enums.
  @return         filesource status.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */
  FileSourceStatus GetMimeType(uint32 id, FileSourceMjMediaType& majorType, FileSourceMnMediaType& minorType);

  /*!
  @brief    Provides Media Samples for requested tracks

  @param[in]      id    Identifies the track for which sample is to be retrieved.
  @param[out]     buf   A pointer to the buffer into which to place the sample.
  @param[out]     size  The size of the data buffer.
  @param[out]           pSampleInfo Provides Information( eg: timestamp etc) about the sample

  @return         FILE_SOURCE_DATA_OK is successful in retrieving the sample otherwise,
                  returns appropriate error code. Please refer to FileSourceMediaStatus.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */
  FileSourceMediaStatus GetNextMediaSample(uint32 id, uint8 *buf, uint32 *size, FileSourceSampleInfo& pSampleInfo);


  /*!
  @brief    Retrieves current Position/playback time( in milli seconds) for the given track.

  @details    This function retrieves the current position/playback time for the given track.

  @param[in]      id    The track identifier
  @return         returns the current Position/playback time in milliseconds

  @note     It is expected that user has received the successul callback for OpenFile() earlier.
  */
  uint64 GetMediaCurrentPosition(uint32 id);

  /*!
  @brief    Retrieves Current Position/playback time(in milli seconds)for the entire clip.

  @details    This function retrieves the current position/playback time for the entire clip.
            This function gives preference to video track over audio and
                  audio over text to calculate the current position.

  @return         returns the Current Position/playback time in milliseconds


  @note     It is expected that user has received the successul callback for OpenFile() earlier.
  */
  uint64 GetMediaCurrentPosition();

  /*!
  @brief    Provides the buffered duration for the given track.

  @param[in]      id    The track ID.
  @param[out]     pDuration Filled in by FileSourec
  @return         FILE_SOURCE_SUCCESS if successful in retrieving buffered duration else returns FILE_SOURCE_FAIL.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
           In case of local file playback, value returned is same as track duration.
  */
  FileSourceStatus GetBufferedDuration(uint32 id, uint64 *pDuration);

 /*!
  @brief    Provides the playback time for buffered/downloaded bytes of particular track.

  @param[in]      nTrackID  The track ID.
  @param[in]      nBytes    Buffered/Downloaded bytes (Always provide positive integer)
  @param[out]     pDuration Filled in by FileSource to provided playtime for buffered/downloaded bytes
  @return         FILE_SOURCE_SUCCESS if successful in retrieving buffered duration else returns FILE_SOURCE_FAIL.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
           In case of local file playback, value returned is same as track duration.
  */
  FileSourceStatus GetBufferedDuration(uint32 nTrackID, int64 nBytes, uint64 *pDuration);

  /*!
  @brief      Method to retrieve Stream specific parameters.

  @param[in]      ulTrackId     Track id to identify stream.
  @param[in]      ulParamIndex  Parameter Index to identify structure'
  @param[in]      pParamStruct  Pointer to the structure.

  @return         file source status
  @note   It is expected that user has received the successul callback for
          OpenFile() earlier.
  */

  FileSourceStatus GetStreamParameter(uint32 ulTrackId, uint32 ulParamIndex,
                                      void* pParamStruct);
  /*!
  @brief      Method to retrieve configuration item set previously via SetConfiguration.

  @param[in]      id              Track id to identify track to which configuration data belongs.
  @param[out]     pItem            Configuration data filled in by parser
  @param[in]      ienumData     Identifies the configuration item. Please refer to FileSourceConfigItemEnum.
  @return         file source status
  @note   It is expected that user has received the successul callback for OpenFile() earlier.
          If ienumData passed in was set earlier via SetConfiguration, pItem->nresult will be set to true by parser.
          If ienumData passed in is not set earlier, pItem->nresult will be set to false by parser.

  */
  FileSourceStatus GetConfiguration(uint32 id, FileSourceConfigItem* pItem, FileSourceConfigItemEnum ienumData);

  /*!
  @brief      Method to Set configuration item.

  @param[in]      id            Track id to identify track to which configuration data belongs.
  @param[out]     pItem          Configuration data filled in by caller
  @param[in]      ienumData   Identifies the configuration item.
                              Please refer to FileSourceConfigItemEnum.
  @return         file source status
  @note   It is expected that user has received the successul callback for OpenFile() earlier.
          If ienumData passed in is not set, pItem->nresult will be set to false by parser.
          If ienumData passed in is set successfully, pItem->nresult will be set to true by parser.
          Every parser may or may not support ienumData being passed in.
          Caller needs to check the return value for success/failure.
          We do not support changing the output mode in the middle of playback.
          Thus, once changed, the output mode won't be allowed to change.
  */
  FileSourceStatus SetConfiguration(uint32 id, FileSourceConfigItem* pItem,FileSourceConfigItemEnum ienumData);

  /*!
  @brief          Method returns metadata such as author,title,copyright information etc.

  @param[out]     pMetaData     MetaData valued gets filled in this buffer
  @param[out]     pLength       Maximum length of pMetadata buffer
  @param[in]      ienumData     Identifies requested metadata type( eg: Title, Author etc).
                                Please refer to FileSourceMetaDataType enum.
                  peEncodeType  Data Encode type pointer

  @details        Pass 0 for pLength to get the size needed, allocate the memory and call again.

  @return         file source status

  @note   It is expected that user has received the successul callback for OpenFile() earlier.
  */
  FileSourceStatus GetClipMetaData(wchar_t* pMetaData, uint32* pLength,
                                   FileSourceMetaDataType ienumData,
                                   FS_TEXT_ENCODING_TYPE* peEncodeType = NULL);

  /*!
  @brief      method to retrieve all valid track identifiers

  @param[out]     trackIdInfo   Pointer to FileSourceTrackIdInfoType, filled in by filesource.

  @details        Pass NULL for trackIdInfo to get total number of valid tracks.
                  Allocate the memory and call again with non NULL pointer.

  @return         number of valid tracks.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */
  uint32 GetWholeTracksIDList(FileSourceTrackIdInfoType *trackIdInfo );

  /*!
  @brief      method for user to select a given track for playback

  @param[in]      id   track identifier to select
  @return         file source status.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */
  FileSourceStatus SetSelectedTrackID(uint32 id);

  /*!
  @brief      Retrieves information about the given Track.

  @details    method to retrieve information about the audio/video/text Track. This Interface is provided
               for User to do channel selection OR use it as a criteria to select a particular track.

  @param[in]      id   track identifier
  @param[out]     info Information about the given track id, filled in by filesource
  @return         FILE_SOURCE_SUCCESS if track is valid else returns FILE_SOURCE_FAIL

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */
  FileSourceStatus GetMediaTrackInfo(uint32 id,MediaTrackInfo* info);

  /*!
  @brief      Sets the information for the given track identifier.

  @details    Method to set information for the given audio/video track.

  @param[in]      info Track information to be set
  @return     FILE_SOURCE_SUCCESS if successful in setting the information otherwise returns appropriate error.

  @note       It is expected that user has called OpenFile with FILE_SOURCE_RAW and has
                  received OPEN_COMPLETE before invoking this API.

  */
  FileSourceStatus SetMediaTrackInfo(MediaTrackInfo info);

  /*!
  @brief      Maximum Buffer size required for the Track to retrieve samples.

  @details        Without parsing, we don't really buffer size needed to retrieve samples for the given track.
                  There are two ways to solve this issue. One is to allocate a huge memory buffer,
                  which is in-efficient use of memory OR use this API to retrieve the
                  buffer size needed for the given track.
                  In some cases, even parser may not know the exact buffer size, so user might
                  have to reallocate if GetNextMediaSample returns FILE_SOURCE_DATA_REALLOCATE_BUFFER.

  @param[in]      id   track identifier

  @return         largest frame size based on parsed clip.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */
  int32  GetTrackMaxFrameBufferSize(uint32 id);


  /*!
  @brief      Retrieves the duration of the track in milliseconds.

  @details        This method retrives the duration for the given track in milliseconds.

  @param[in]      id   track identifier
  @return         duration of the track in milli seconds

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */
  uint64  GetTrackMediaDuration(uint32 id);

  /*!
  @brief      Retrieves the duration of the Clip in milliseconds.

  @details        This method retrives the duration from all the tracks ( audio, video and text)
                  and returns the maximum.
  @return         duration of the clip in milli seconds.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */
  uint64  GetClipDuration();

  /*!
  @brief      Retrieve the Format Block information for the given track.

  @details        method retrieves the Decoder specific/Format Block information for the given track.
                  This interface is generic for Audio, Video and Text.
                  If buf = NULL, then the function give the size of the required buffer/format block.
                  Following is an example of retrieving the format block.

                  1.Invoke getFormatBlock for a given track identifier by passing in NULL for buf.
                  2.If a track is valid, *pbufSize will give you the size of format block that exist.
                  3.Allocate the memory and invoke getFormatBlock API for a given track identifier
                  by passing handle to allocated memory.

  @param[in]      id   track Id to select.
  @param[out]     buf   Buffer provies the format block info to the caller
  @param[out]     pbufSize   Size of the FormatBlock buffer
  @param[in]      bRawCodec  Flag to indicate whether codec data required in input format
                             or converted to proper SPS/PPS format

  @return         file source status.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */
  FileSourceStatus GetFormatBlock(uint32 id, uint8* buf, uint32 *pbufSize,
                                  bool bRawCodec = false);

  /*!
  @brief      Reposition to an absolute position.

  @details        The routine will seek/repositon to next valid sample based on the timestamp
                  provided. This function Uses Video track as the primary source to find the time stamp and
                  then sync's the Audio and Text accordingly. However, if Video is not present it
                  will use Audio as the reference to seek text track.

  @param[in]      tAbsoluteTime seek to the absolute position(ms).
  @param[in]      bSeekToSync   When set to false, parser can seek to non sync frame
  @param[in]      nCurrPlayTime Current playback time.(-1 indicates time unknown)
  @return         file source status.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
                  bSeekToSync is only applicable if there is video track
                  as for audio every frame is a sync frame.
  */
  FileSourceStatus SeekAbsolutePosition( const int64 tAbsoluteTime,
                                         bool bSeekToSync=true,
                                         int64 nCurrPlayTime=-1);

  /*!
  @brief     Reposition given track to an absolute position

  @details        This API will try to seek the track identified via trackid to
                  given absolute time provided there is an I frame at that time.
                  If there are no I frames in forward direction, forward seek
                  request can fail. To allow parser to seek to non key frame,
                  bSeekToSync can be set to false.

  @param[in]      trackid       Track-Id to specify which track to seek
  @param[in]      tAbsoluteTime Seek to the absolute position(ms).
  @param[in]      bSeekToSync   When set to false, parser can seek to non sync frame
  @param[in]      nCurrPlayTime Current playback time.(-1 indicates time unknown)
  @return         file source status.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
                  IsSeekDenied() API can be used to determine if seek will be supported or not.
                  bSeekToSync is only applicable if there is video track as for audio,
                  every frame is a sync frame.
  */
  FileSourceStatus SeekAbsolutePosition( int64 trackid ,
                                         const int64 tAbsoluteTime,
                                         bool bSeekToSync=true,
                                         int64 nCurrPlayTime=-1);

  /*!
  @brief      Reposition to a relative Sync Sample/number from current playback position.

  @details        The routine will seek in forward or backward direction based on the timestamp and numSync.
                  The direction can be both positive or negative.

  @param[in]      currentPlaybacktime current playback time(ms) from the point of view of the User/Caller.
  @param[in]      numSync number of sync sample to jump. The value can be both positive or negative.
                  + value will trigger seeking in forward direction for given number of sync samples.
                  - value will trigger seeking in backward direction for given number of sync samples.
  @return         file source status.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */

  FileSourceStatus SeekRelativeSyncPoint( uint64 currentPlaybacktime, const int numSync);

  /*!
  @brief   Returns current parser error, if any.

  @return              error code from underlying file parser. Please refer to FileSourceParserError enumeration.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */
  FileSourceParserError GetFileError();

  /*!
  @brief           Retrieves clip specifc DRM information.

  @param[in,out]   pDrmInfo pointer to retrieve the Clip specific DRM information.
                   Memory should be allocated before the pointer is passed to this function.

  @return          FILE_SOURCE_SUCCESS if successful in retrieving DRM information otherwise,
                   returns appropriate error code.

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */

  FileSourceStatus GetClipDrmInfo(ClipDrmInfoT* pDrmInfo);

  /*!
  @brief   function to findout if the clip is DRM protected or not.

  @return             True if file is protected else returns false.

  @note    It is expected that user has received the successful callback for OpenFile() earlier.
  */
  bool IsDRMProtection();

  /*!
   @brief   Function to register DRM Decrypt Method with parser.

   @return  True if successful in registering the method else returns false.

   @note    It is expected that the Filesource client has received a successful OPEN
            callback
   */
  bool RegisterDRMDecryptMethod(DRMDecryptMethodT,void*);
  /*!
  @brief   Function to get the device registration code.

  @return             True if successful in retrieving the code else returns false.

  @note    This API needs to be called in 2 step process.
                   Call with NULL as first parameter and API will return the size needed for registration code.
                   Allocate the memory and call again to get the registration code.
  */
  bool GetRegistrationCode(char*,int*);

  /*!
  @brief   Commit the DivX Playback.

  @param[in]         majorType Identifies audio/video/text track which should process the commit.
  @return               true if commit is successful otherwise returns false

  @note    It is expected that user has received the successul callback for OpenFile() earlier.
  */
  bool CommitDivXPlayback(FileSourceMjMediaType majorType);

  /*!
  @brief Retrieve the wma codec data needed to configure WMA decoder

  @param[in] id    Identifies the WMA track for which codec data needs to be retrieved
  @param[in,out]   pCodecData filled in by FileSource

  @return          true is successful in retrieving the codec data else returns false
  @note It is expected that user has received the successul callback for OpenFile() earlier.
  */
  bool GetWMACodecData(uint32 id,WmaCodecData* pCodecData);

  /*!
  @brief Retrieve the wav codec data needed to configure wav decoder

  @param[in] id    Identifies the wav track for which codec data needs to be retrieved
  @param[in,out]   pCodecData filled in by FileSource

  @return          true is successful in retrieving the codec data else returns false
  @note It is expected that user has received the successul callback for OpenFile() earlier.
  */
  bool GetWavCodecData(uint32 id,WavFormatData* pCodecData);

  /*!
  @brief Retrieve the AAC codec data needed to configure AAC decoder

  @param[in] id    Identifies the AAC track for which codec data needs to be retrieved
  @param[in,out]   pCodecData filled in by FileSource

  @return          true is successful in retrieving the codec data else returns false
  @note It is expected that user has received the successul callback for OpenFile() earlier.
  */
  bool GetAACCodecData(uint32 id,AacCodecData* pCodecData);

  /*!
  @brief Retrieve the Flac codec data needed to configure FLAC decoder

  @param[in] id    Identifies the FLAC track for which codec data needs to be retrieved
  @param[in,out]   pCodecData filled in by FileSource

  @return          true is successful in retrieving the codec data else returns false
  @note It is expected that user has received the successul callback for OpenFile() earlier.
  */
  bool GetFlacCodecData(uint32 id,flac_format_data* pCodecData);

  /*!
  @brief To determine whether Seek operation is supported or not.
  @return          true(1) indicates seek is not allowed and false(0) indicates seek is allowed.
  @note   It is expected that user has received the successul callback for OpenFile() earlier.
  */
  uint8 IsSeekDenied();

  /*!
  @brief Returns absolute file offset(in bytes) associated with time stamp 'pbtime'(in milliseconds).

  @param[in]       pbtime Timestamp(in milliseconds) of the sample that user wants to play/seek
  @param[out]      offset Absolute file offset(in bytes) corresponding to 'pbtime'

  @return          true if successful in retrieving the absolute offset(in bytes) else returns false

  @note
        It is expected that user has received the successul callback for OpenFile() earlier.
        When there are multiple tracks in a given clip(audio/video/text),API returns
                   minimum absolute offset(in bytes)among audio/video/text tracks.
                   API is valid only for non-fragmented clips.
  */
  bool GetOffsetForTime(uint64 pbtime,uint64* offset);

  /*!
  @brief Returns absolute file offset(in bytes)
         corresponding to last retrieved sample for the given track identifier.

  @param[in]      trackid TrackId identifying the media track
  @param[out]     bError  Set to true if error occurs while retrieving file offset.
  @return         Returns absolute file offset(in bytes) that corresponds to
                  current sample retrieved via getNextMediaSample.
  @note       It is expected that user has received the successul callback for OpenFile() earlier.
  */
  uint64 GetLastRetrievedSampleOffset(uint32 trackid,bool* bError);

  /*!
  @brief Returns total number of views associated with given trackid.
         Default value is always 1 for any given track unless the track has MVC(MultiViewCoding).

  @param[in]  trackid TrackId identifying the media track
  @return     Returns total number of views for given trackid
  @note       It is expected that user has received the successul callback for OpenFile() earlier.
  */
  uint16 GetNumberOfViews(uint32 trackid);

  /*!
  @brief          Quick way of validating if given File belongs to certain format without creating FileSource instance.
                  To seppd up the checking, this function will only run check for desired file format.

  @param[in]      FormatToCheck Enum specifying format to be checked
  @param[in]      pInputBuffer buffer containing data from the biginning of the file/clip
                        If NULL, function will put required number of bytes for particular format check in pBufferSize.
  @param[in/out]  pBufferSize user will pass size of pInputBuffer. Function will updated required size if given size if not enough
  @return         Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have enough data. Then pBufferSize will be updated to have required size.
                  Returns FILE_SOURCE_INVALID if arguments are not valid
  @note           Utility function. No need to create FileSource instance.
  */
  static FileSourceStatus CheckFileFormat(FileSourceFileFormat FormatToCheck, uint8 *pInputBuffer, uint32 *pBufferSize);

 /*!
  @brief     Retrieve protection system specific information of supported
             DRM systems.

  @details        This API will provides content protection system specific
                  information applicable to supported DRM system in same media
                  file.It contains SystemID of the DRM system along with size of
                  content protection system specific data for each supported
                  DRM in sequential order.

  @param[in]      ulTrackID TrackId identifying the media track.
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
  */
  FileSourceStatus GetProtectionSystemSpecificInfo(
    uint32 ulTrackID,
    FS_PROTECTION_SYSTEM_SPECIFIC_INFOTYPE* pPSSHInfo);

 /*!
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
  */
  FileSourceStatus GetProtectionSystemSpecificData(
    uint32 ulTrackID,
    FS_PROTECTION_SYSTEM_SPECIFIC_DATATYPE* pPSSHData);

private:
 FileSourceHelper* m_pHelper;

};

#endif /* __FileSource_H__ */

