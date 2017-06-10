#ifndef __FileSource_Helper_H__
#define __FileSource_Helper_H__
/*=======================================================================
 *                             filesourcehelper.h
 *DESCRIPTION
 * Helper class for filesource module
 *
 * Copyright (c) 2008-2014 Qualcomm Technologies Inc, All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.rporated.
 *======================================================================== */

/* =======================================================================
                             Edit History
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileSource/main/latest/src/filesourcehelper.h#36 $$
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
#include "filesourcestring.h"
#include "parserdatadef.h"
#include "filebase.h"
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

typedef enum
{
  //! Default state when constructor is invoked.
  FS_STATE_IDLE,
  //! Initialized state. Parser should be in this state to open any file/stream
  FS_STATE_INIT,
  //! Open state, Parser should be in this state to process open command
  FS_STATE_OPENING,
  //! This state is to indicate Parser is in READY state.
  //! Parser will go to this state after open is completed.
  //! Parser should be in this state to process Seek or any other requests
  FS_STATE_READY,
  //! Parser is closing, it will destroy all media handles in this state
  FS_STATE_CLOSING,
}FileSourceState;

/*! @brief FileSource Notification Callback datatype */
typedef void (*FileSourceHelperCallbackFuncType)(
                                          FileSourceCallBackStatus status,    //! status code
                                          void *                   pClientData
                                          );

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
  @brief   FileSource Module.

  @details The Intent of having this module is to Provide a common interface for
          upperlayers to control the FileBase (for both MP4 and ASF file formats)
          and perform basic operations like Play/Pause/Open/Seek based on the TrackId
          independent of the media (Audio/Video/Text) type.


  @note   Inorder to use the functions provided by this class. It is required
          that the User calls OpenFile before calling others
*/

class FileSourceHelper
{
public:

  //! constructor with Callback Function as input
  FileSourceHelper(FileSourceHelperCallbackFuncType callBack, void * pClientData,
                   bool bAsync = true);

  //! So the FileSource destructor gets called when delete the interface
  virtual ~FileSourceHelper();


  /*!
    @brief  Open a File.

    @details This method is called only in the case of Local file playback.
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
    @param[in]  blookforcodecconfig   when true, parser returns OPEN_COMPLETE only when
                               codec configuration header has been located for audio/video/text track
                               When false, OPEN_COMPLTE is reported as soon as all valid tracks are located.
    @return     filesource status.

    @note   It is expected that when this function is called atleast one of the
            parameters is not NULL. i.e, atleast one filename is passed.
   */
  FileSourceStatus  OpenFile(  wchar_t* audioFilename = NULL,
                               wchar_t* videoFilename = NULL,
                               wchar_t* textFilename  = NULL,
                               FileSourceFileFormat format = FILE_SOURCE_UNKNOWN,
                               bool     blookforcodecconfig = true
                              );

  /*!
    @brief  Buffer playback.

    @details This method is called to play audio/video/text from buffers passed in.
            It's possible to play audio/video/text from separate buffers or user
            can just pass in the same buffer for audio/video/text.
            FileSource  Module will pass the buffers to lower layer (fileBase) to parse
            and get information about the tracks, headers etc.

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
    @return     filesource status.

    @note   It is assumed that these buffers will be valid through out the playback
            and data bytes are stored at contiguous address. FileSource module
            can request the data within the valid offsets range at anytime.

            0 - nAudioBufSize
            0 - nVideoBufSize
            0 - nTextBufSize

            It is expected that when this function is called atleast one of the
            parameters is not NULL. i.e, atleast one buffer is not NULL.
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
   @brief   Opens a Streaming source/bytes stream for playback

   @details  It takes iStreamPort as input parameter. iStreamPort will provide interfaces
            to Read/Seek/Close.
            FileSource Module will read the information and get information about the
            tracks, headers etc.

   @param[in]  pInputStream   Input Stream of Audio/video/text tracks
   @param[in]  format         If file format is already known,caller
                              can pass in the information into FileSource.
                              When format != FILE_SOURCE_UNKNOWN,FileSource
                              will instantiate the given parser without doing any
                              file format check.
    @param[in]  blookforcodecconfig  When true, parser returns OPEN_COMPLETE only when
                                   codec configuration header has been located for audio/video/text track
                                   When false, OPEN_COMPLTE is reported as soon as all valid tracks are located.

   @return     filesource status.

   @note    It is expected that pInputStream is  not  NULL.
   */
  FileSourceStatus OpenFile(video::iStreamPort* pInputStream = NULL,
                            FileSourceFileFormat format = FILE_SOURCE_UNKNOWN,
                            bool blookforcodecconfig = true);

  /*!
   @brief   Close the existing opened file

   @details     Please see the note below

   @return     filesource status.

   @note       Once close is called, FileSource object will no longer support any API except OpenFile.
               It's a blocking call (in caller's context) untill CloseFile is processed in FileSource Thread.
   */
  FileSourceStatus  CloseFile();

 /*!
   @brief    Opens IxStream for DCF playback.

   @details  It takes IxStream* as input parameter. dcf_ixstream_type will provide interfaces
            to Read/Seek/Close.
            FileSource Module will read the information and get information about the
            tracks, headers etc.

   @param[in]  pInputStream   Input DCF Stream of Audio/video/text tracks
   @return     filesource status.

   @note    It is expected that pInputStream is  not  NULL.
   */
  FileSourceStatus OpenFile(   IxStream* pInputStream = NULL);

  /*!
  @brief    method returns the file format.

  @param[out]  fileFormat   type of  FileFormat ( MPEG4/ASF/AVI).
  @return      filesource status.

  @note    It is expected that OpenFile() is called before getFileFormat() is called.
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
  @brief    method returns Major and Minor Type for the Clip.

  @param[in]   id           track ID
  @param[out]  majorType   Major Media Types ( Audio/Video/Text).
  @param[out]  minorType   Minor Media Types ( Sub media type within Audio/Video text)
                           They are also referred as Codecs
  @return      filesource status.

  @note    It is expected that OpenFile() is called before getMimeType() is called.
  */
  FileSourceStatus GetMimeType(uint32 id, FileSourceMjMediaType& majorType, FileSourceMnMediaType& minorType);

  /*!
   @brief    Provides Media Samples for requested tracks

   @param[in]  id    The track ID of the track from which the method is to retrieve the samples.
   @param[out] buf   A pointer to the buffer into which to place the sample.
   @param[out] size  The size of the data buffer.
   @param[out] pSampleInfo Provides Information( eg: timestamp etc) about the sample
   @return     The size in bytes of the data placed into the provided buffer.
               If the buffer is not large enough, the return value is the negative
               of the size that is needed .

   @note    It is expected that OpenFile() is called before getNextMediaSample() is called.
  */
  FileSourceMediaStatus GetNextMediaSample(uint32 id, uint8 *buf, uint32 *size, FileSourceSampleInfo& pSampleInfo);


  /*!
   @brief    Proves Current Position in milli seconds

   @details    This function retrieves the current position without you having to call
      the getNextMediaSample().

   @param[in]  id    The track ID of the track
   @return     returns the timestamp in milliseconds of the next valid sample.

   @note     It is expected that OpenFile() is called before getMediaCurrentPosition() is called.
  */
  uint64 GetMediaCurrentPosition(uint32 id);

  /*!
   @brief    Proves Current Position in milli seconds for the entire track

   @details    This function retrieves the current position without you having to call
             the getNextMediaSample(). This function gives preference to video track
       position over audio and audio over text to calculate the current position

   @return     returns the timestamp in milliseconds of the next valid sample.


   @note     It is expected that OpenFile() is called before getMediaCurrentPosition() is called.
  */
  uint64 GetMediaCurrentPosition();

  /*!
   @brief    Provides the buffered duration of a track at any point of time during progressive download

   @param[in]  id     The track ID.
   @param[in]  nBytes Buffered/downloaded bytes
   @param[out] pDuration Function will copy buffered duration into this variable
   @return     FILE_SOURCE_SUCCESS if track is valid else returns FILE_SOURCE_FAIL

   @note    It is expected that OpenFile() is called before getBufferedDuration() is called.
  */
  FileSourceStatus GetBufferedDuration(uint32 id, int64 nBytes, uint64 *pDuration);

  /*!
  @brief      Method to retrieve configuration item set previously via SetConfiguration.

  @param[in]      id              Track id to identify track to which configuration data belongs.
  @param[out]     pItem            Configuration data filled in by parser
  @param[in]      ienumData     Identifies the configuration item.
                                Please refer to FileSourceConfigItemEnum.
  @return         file source status
  @note   It is expected that user has received the successul callback for OpenFile() earlier.
  */

  FileSourceStatus GetConfiguration(uint32 id,FileSourceConfigItem* pItem, FileSourceConfigItemEnum ienumData);

  /*!
  @brief      Method to Set configuration item.

  @param[in]      id              Track id to identify track to which configuration data belongs.
  @param[out]     pItem            Configuration data filled in by caller
  @param[in]      ienumData     Identifies the configuration item.
                                Please refer to FileSourceConfigItemEnum.
  @return         file source status
  @note   It is expected that user has received the successul callback for OpenFile() earlier.
  */

  FileSourceStatus SetConfiguration(uint32 id,FileSourceConfigItem* pItem, FileSourceConfigItemEnum ienumData);


  /*!
   @brief      method returns metadata about the Clip.

   @details     In case of MPEG4 file format it retrieves the information from the
               PVUserDataAtom 'pvmm' atom.

   @param[in]  ienumData      Identifies requested metadata type( eg: Title, Author etc).
   @param[out] pLength        Maximum length of pMetadata buffer
   @param[out] pMetaData        MetaData valued gets filled in in this buffer
   @return     file source status

   @note    It is expected that OpenFile() is called before getClipMetaData() is called.
   @note    Pass 0 in pLength to get the size needed, allocate the memory and call again.
  */
  FileSourceStatus GetClipMetaData(wchar_t* pMetaData, uint32* pLength,
                                   FileSourceMetaDataType ienumData,
                                   FS_TEXT_ENCODING_TYPE* peEncodeType = NULL);

  /*!
   @brief      method to retrieve all valid trackID List

   @param[out]  trackIdInfo   a list of TrackIfInfo (
                consisting of trackid's and if a book if they are selected or not).
   @return     number of valid audio, video and text tracks.

   @note    It is expected that OpenFile() is called before getWholeTracksIDList() is called.
  */
  uint32 GetWholeTracksIDList(FileSourceTrackIdInfoType *trackIdInfo );

  /*!
   @brief      method for User to Select a Track.

   @param[in]  id   track Id to select
   @return     file source status.

   @note    It is expected that OpenFile() is called before setSelectedTrackID() is called.
  */
  FileSourceStatus SetSelectedTrackID(uint32 id);

  /*!
   @brief      Provides information about the Track.

   @details    method to retrieve information about the Audio/Video Track. This Interface is provided
              for User to do channel selection OR use it as a criteria to select a particular track.

   @param[in]  id   track Id to select
   @param[out] info Information about the given track id
   @return     FILE_SOURCE_SUCCESS if track is valid else returns FILE_SOURCE_FAIL

   @note    It is expected that OpenFile() is called before getMediaTrackInfo() is called.
  */
  FileSourceStatus GetMediaTrackInfo(uint32 id,MediaTrackInfo* info);

  /*!
   @brief      Provides information about the Track.

   @details    Method to set information about the Audio/Video Track.

   @param[in] info Information about the codec/track
   @return     FILE_SOURCE_SUCCESS if successful in setting the information otherwise returns appropriate error.

   @note       It is expected that user has called OpenFile with FILE_SOURCE_RAW and has
               received OPEN_COMPLETE before invoking this API.

  */
  FileSourceStatus SetMediaTrackInfo(MediaTrackInfo info);


  /*!
   @brief      Maximum Buffer size required for the Track.

   @details    Before we parse a clip we do not know the size of the Frame.
              There are two ways to solve this issue. One is to allocate a huge memory buffer,
              which is in-efficient use of memory OR use this method to retrieve the
              buffer size needed for the frame and then allocate/reallocate memory as needed.

   @param[in]  id   track Id to select.

   @return  largest frame size up to the frame we have parsed.
      (Note:we do not parse/scan all the frames during initial parsing).

   @note    It is expected that OpenFile() is called before getTrackMaxFrameBufferSize() is called.
  */
  int32  GetTrackMaxFrameBufferSize(uint32 id);


  /*!
   @brief      gives the duration of the track in milliseconds.

   @details    This method retrives the duration from  the track.

   @param[in]  id   track Id to select.
   @return     duration of the track in milli seconds

   @note    It is expected that OpenFile() is called before getClipDuration() is called.
  */
  uint64  GetTrackMediaDuration(uint32 id);

  /*!
   @brief      gives the duration of the Clip in milliseconds.

   @details    This method retrives the duration from all the tracks ( audio, video and text).
              and picks the maximum.
   @return     duration of the clip in milli seconds

   @note    It is expected that OpenFile() is called before getClipDuration() is called.
  */
  uint64  GetClipDuration();


  /*!
   @brief      Retrieve the Format Block information about the track.

   @details    method to retrieve the Decoder specific/Format Block information from the track.
              This interface is generic for Audio, Video and Text.
              If buf = NULL, then the function give the size of the required buffer.
              Following is an example of retrieving the format block.

              1.Invoke getFormatBlock API for a given track identifier by passing in NULL for buf.
              2.If a track is valid, *pbufSize will give you the size of format block.
              3.Allocate the memory and invoke getFormatBlock API for a given track identifier
                by passing handle to allocated memory.

   @param[in]   id   track Id to select.
   @param[out]  buf   Buffer provies the format block info to the caller
   @param[out]  pbufSize   Size of the FormatBlock buffer
  @param[in]    bRawCodec  Flag to indicate whether codec data required in input format
                           or converted to proper SPS/PPS format

   @return     file source status.

   @note    It is expected that OpenFile() is called before getFormatBlock() is called.
  */
  FileSourceStatus GetFormatBlock(uint32 id, uint8* buf,
                                  uint32 *pbufSize, bool bRawCodec);

  /*!
   @brief      Reposition to an absolute position.

   @details   The routine will seek/repositon to next valid sample based on the timestamp
              provided. This function Uses Video track as the primary source to find the time stamp and
              then sync's the Audio and Text accordingly. However, if Audio is not present it
              will use Audio as the reference.

   @param[in]  tAbsoluteTime seek to the absolute position(ms).
   @param[in]  bSeekToSync   When set to false, parser can seek to non sync frame
   @param[in]  nCurrPlayTime Current playback time.(-1 indicates time unknown)
   @return     file source status.

   @note    It is expected that OpenFile() is called before SeekAbsolutePosition() is called.
            bSeekToSync is only applicable if there is video track as for audio,
            every frame is a sync frame.
  */
  FileSourceStatus SeekAbsolutePosition( const int64 tAbsoluteTime,
                                         bool bSeekToSync=true,
                                         int64 nCurrPlayTime=-1);

  /*!
   @brief     Reposition given track to an absolute position

   @details   This API will try to seek the track identified via trackid to
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
  */
  FileSourceStatus SeekAbsolutePosition( int64 trackid ,
                                         const int64 tAbsoluteTime,
                                         bool bSeekToSync=true,
                                         int64 nCurrPlayTime=-1);

  /*!
   @brief      Reposition to a relative Sync Sample/number of sync sample.

   @details    The routine will seek/repositon to next sync sample based on the timestamp
              provided. The direction can be both positive and negative.

   @param[in]   currentPlaybacktime current playback time(ms) from the point of view of the User/Caller.
   @param[in]   numSync number of sync sample to jump. The value can be both positive and negative
               , specifiying the direction to search for the Sync Sample.
   @return     file source status.

   @note    It is expected that OpenFile() is called before SeekRelativeSyncPoint() is called.
  */

  FileSourceStatus SeekRelativeSyncPoint( uint64 currentPlaybacktime, const int numSync);

  /*!
    @brief   Returns current source error, if any.

    @return        error code status of underlying file source

    @note    It is expected that OpenFile() is called before GetFileError() is called.
   */
  FileSourceParserError GetFileError();

  /*!
   @brief      Retrieves clip specifc DRM information

   @param[in,out]   pDrmInfo pointer to retrieve the Clip specific DRM information.
                    Memory should be allocated before the pointer is passed to this function.

   @return  FILE_SOURCE_SUCCESS if successful in retrieving DRM information otherwise,
            returns appropriate error code.

   @note    It is expected that OpenFile() is called before GetClipDrmInfo() is called.
  */

  FileSourceStatus GetClipDrmInfo(ClipDrmInfoT* pDrmInfo);

  /*!
   @brief   function to findout if the clip is DRM protected or not.

   @return    True if file is protected else returns false.

   @note    It is expected that OpenFile() is called before IsDRMProtection() is called.
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

  @return    True if successful in retrieving the code else returns false.

  @note    This API needs to be called in 2 step process.
           Call with NULL as first parameter and API will return the size needed for registration code.
           Allocate the memory and call again to get the registration code.
  */
  bool GetRegistrationCode(char*,int*);

  /*!
    @brief   Commit the DivX Playback.

    @param[in]    majorType Identifies audio/video/text file base instance.
    @return        true if commit is successful otherwise returns false

    @note    It is expected that OpenFile() is called before CommitDivXPlayback() is called.
   */
  bool CommitDivXPlayback(FileSourceMjMediaType majorType);

  /*!
  @brief Retrieve the wma codec data needed to configure WMA decoder

  @param[in] id   Identifies the WMA track for which codec data needs to be retrieved
  @param[in,out] pCodecData filled in by FileSource

  @return true is successful in retrieving the codec data else returns false
  */
  bool GetWMACodecData(uint32 id,WmaCodecData* pCodecData);

  /*!
  @brief Retrieve the wav codec data needed to configure wav decoder

  @param[in] id   Identifies the wav track for which codec data needs to be retrieved
  @param[in,out] pCodecData filled in by FileSource

  @return true is successful in retrieving the codec data else returns false
  */
  bool GetWavCodecData(uint32 id,WavFormatData* pCodecData);

  /*!
  @brief Retrieve the AAC codec data needed to configure AAC decoder

  @param[in] id   Identifies the AAC track for which codec data needs to be retrieved
  @param[in,out] pCodecData filled in by FileSource

  @return true is successful in retrieving the codec data else returns false
  */
  bool GetAACCodecData(uint32 id,AacCodecData* pCodecData);

  /*!
  @brief Retrieve the Flac codec data needed to configure FLAC decoder

  @param[in] id   Identifies the FLAC track for which codec data needs to be retrieved
  @param[in,out] pCodecData filled in by FileSource

  @return true is successful in retrieving the codec data else returns false
  */
  bool GetFlacCodecData(uint32 id,flac_format_data* pCodecData);

  /*!
  @brief To determine whether Seek operation is supported or not
  @return true(1) indicates seek is not allowed and false(0) indicates seek is allowed.
  @note   It is expected that OpenFile() is called successfully before calling SeekDenied
  */
  uint8 IsSeekDenied();

  /*!
  @brief Returns absolute file offset(in bytes) associated with time stamp 'pbtime'(in milliseconds).

  @param[in]  pbtime Timestamp(in milliseconds) of the sample that user wants to play/seek
  @param[out] offset Absolute file offset(in bytes) corresponding to 'pbtime'

  @return true if successful in retrieving the absolute offset(in bytes) else returns false

  @note When there are multiple tracks in a given clip(audio/video/text),API returns
   minimum absolute offset(in bytes)among audio/video/text tracks.
   API is valid only for non-fragmented clips.
  */
  bool GetOffsetForTime(uint64 pbtime, uint64* offset);

  /*!
  @brief Returns absolute file offset(in bytes)
         corresponding to current sample being retrieved via getNextMediaSample.

  @param[in]  trackid TrackId identifying the media track
  @param[out] bError  Indicates if error occured while retrieving file offset.

  @note       bError is set to true when error occurs otherwise, set to false.
  @return Returns absolute file offset(in bytes) that corresponds to
          current sample retrieved via getNextMediaSample.
  */
  uint64 GetLastRetrievedSampleOffset(uint32 trackid, bool* bError);

  /*!
  @brief Returns total number of views associated with given trackid.
         Default value is always 1 for any given track unless the track has MVC(MultiViewCoding).

  @param[in]  trackid TrackId identifying the media track
  @return     Returns total number of views for given trackid
  @note       It is expected that user has received the successul callback for OpenFile() earlier.
  */
  uint16 GetNumberOfViews(uint32 trackid);

  /*!==========================================================================
  @brief      Get Audio/Video stream parameter

  @details    This function is used to get Audio/Video stream parameter i.e.
              codec configuration, profile, level from specific parser.

  @param[in]  ulTrackId    TrackID of media
  @param[in]  ulParamIndex Parameter Index of the structure to be
                           filled.It is from the FS_MEDIA_INDEXTYPE
                           enumeration.
  @param[out] pParamStruct Pointer to client allocated structure to
                           be filled by the underlying parser.

  @return     PARSER_ErrorNone in case of success otherwise Error.
  @note
  =============================================================================*/
  FileSourceStatus GetStreamParameter(uint32 ulTrackId,
                                      uint32 ulParamIndex,
                                      void* pParamStruct);

/*! ===========================================================================
  @brief          Validated file format based on file format specific signature
                  without creating any filesource instance.
                  This function will only run check for desired file format.

  @param[in]      pInputBuffer buffer containing data from the beginning of the
                  file/clip.If NULL, function will put required number of bytes
                  for particular format check in pBufferSize.
  @param[in/out]  pBufferSize user will pass size of pInputBuffer. Function will
                  updated required size if given size if not enough

                  @return Returns FILE_SOURCE_SUCCESS if format check passes
                  Returns FILE_SOURCE_FAIL if format check fails
                  Returns FILE_SOURCE_NOTAVAILABLE if check for this format is not available
                  Returns FILE_SOURCE_DATA_NOTAVAILABLE if pInputBuffer does not have
                  enough data. Then pBufferSize will be updated to have required size.
                  Returns FILE_SOURCE_INVALID if arguments are not valid

  @note           Utility function. No need to create FileSource instance.
 ===============================================================================*/
static FileSourceStatus ValidateAACFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateAC3File(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateDTSFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateAMRFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateAMRWBFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateASFFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateAVIFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateEVRCBFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateEVRCWBFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateFLACFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateMP4File(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus Validate3GPP2File(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateVOBFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateTSFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateMKVFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateMP3File(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateOGGFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateQCPFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateWAVFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
static FileSourceStatus ValidateFLVFile(uint8 *pucDataBuffer,
                                        uint32 *pulBufferSize);
protected:
  //! local function to initialize all data members.
  void BaseInitData();

  //! Open a file
  void HandleOpenFileCommand();

  //! close a file
  void HandleCloseFileCommand();

  //! seek the file which was opened successfully via OpenFile
  void HandleSeekFileCommand();

  //! Fill the information related to Audio tracks
  void FillAudioTrackInfo();

  //! Fill the information related to Video tracks
  void FillVideoTrackInfo();

  //! Fill the information related to Text tracks
  void FillTextTrackInfo();

  //! select the first track as the default in audio/video/text tracks
  void SelectDefaultTracks();

  //! retrieve the Major Media type ( Audio/Video/Text)
  FileSourceMjMediaType GetFileSourceMajorType(uint32 id);

  //! retrieve the Minor Media type
  FileSourceMnMediaType GetFileSourceMinorType(uint32 id);

  //! function to map codec to Minor type
  FileSourceMnMediaType MapCodecToMinorType(uint8 codec);

  void MapParserError2FileSourceStatus(PARSER_ERRORTYPE retError, FileSourceMediaStatus &status);
  //! function to read the next audio sample
  FileSourceMediaStatus GetNextAudioSample(uint32 id, uint8 *buf, uint32 *size, int *numSample);

  //! function to read the next audio frame(at frame boundary)
  FileSourceMediaStatus GetNextAudioFrame(uint32 id, uint8 *buf, uint32 *size, int *numSample);

  //! function to locate frame boundary for audio
  uint32 LocateAudioFrameBoundary(uint32 trackid,audio_data_buffer* pAudBuff,bool bprocesstimestamp);

  //! function to get MP3 header and validate it
  uint32 GetMP3AudioInfo(uint8* buf,void* phdr,uint32 trackid);

  //! function to parse MP3 header from given buffer
  bool   FillMP3TechHdr(uint8* buf,void* phdr,uint32 trackid);

  //! function to parse AAC header from given buffer
  uint32 GetAACAudioInfo(uint8* buf,float* frameTime);

  //! function to read the next video sample
  FileSourceMediaStatus GetNextVideoSample(uint32 id, uint8 *buf, uint32 *size, int *numSample);

  //! function to read the next text sample
  FileSourceMediaStatus GetNextTextSample(uint32 id, uint8 *buf, uint32 *size, int *numSample);

  //! get information about the current sample from a given track
  FileSourceMediaStatus GetCurrentSampleInfo(uint32 id, FileSourceSampleInfo &pSampleInfo, FileBase* pFileBase);

  //! Function to get the index of the given track from media(audio/video)tracks array.
  int GetIndexInMediaTrackInfo(uint32 id);

  //! Function to get the index of the given track from overall tracks array.
  int GetIndexInTrackIdInfo(uint32 id);

  //! destroys the media handles associated with the file being opened
  void DestroyMediaHandles();

  //! To determine if audio/video/text tracks exist in given clip
  void IdentifyTracksMajorType(FileBase*, bool*,bool*,bool*);

  //! Function to open media handle through IStreamPort
  void OpenFileStreamPort(void);

  //! Function to open media handle through Buffer
  void OpenFileBuffer(void);

  //! Function to open media handle through IxStream
  void OpenFileIxStream(void);

  //! Function to open media handle when file name is provided
  void OpenLocalFile(void);

  //! Function to dump text track, works on WIN 32 only
  void DumpTextTrack(void);

  /*!
  @brief Notify each media handle about the amount of data being downloaded.

  @details In case of http streaming, each media handle needs to get an update
           to figure out the maximum amount of data downloaded so far.
           This helps is determining if playback for given track can begin or not.

  @param[out] bEndOfData Set to true/false based on video::iStreamPort::GetAvailableOffset

  @note This API is valid only when video::iStreamPort is being passed to FileSource
  */

  void UpdateMediaBufferOffset(bool* bEndOfData);

  //! Pointer to the Audio instances of FileBase
  FileBase* m_pAudioFileHandle;

  //! Pointer to the Video instances of FileBase
  FileBase* m_pVideoFileHandle;

  //! Pointer to the Text instances of FileBase
  FileBase* m_pTextFileHandle;

  //! Pointer to video::iStreamPort which is used to read/seek the required data
  video::iStreamPort* m_pIStreamPort;

  //! Pointer to IxStream for DRM playback
  IxStream*           m_pIxStream;

  //! Function Pointer to hold the callback function provided by the 'User'
  //! FileSource will call this when an OPEN is complete/failed OR a SEEK is complete/failed
  FileSourceHelperCallbackFuncType m_fileSourceHelperCallBackFunc;

  //! To store client data passed by caller in FileSource constructor
  void* m_pClientData;

  //! Array of all the valid track along with the information about their media type and selectivity.
  FileSourceTrackIdInfoType m_trackIdInfo[FILE_SOURCE_MAX_NUM_TRACKS];

  //! Number of Audio tracks
  uint32 m_nAudioTracks;

  //! Number of Video tracks
  uint32 m_nVideoTracks;

  //! Number of Text tracks
  uint32 m_nTextTracks;

  //!Stores current playback time
  uint64 m_nCurrentPlaybacktimeForSeek;

  //!Stores number of sync frmes to skip
  int m_nNumSync;

  //!Stores the absolute time to seek to
  uint64 m_nSeekAbsoluteTime;

  //!Specify whether to seek to key frame only
  bool m_bSeekToSync;

  //!Stores the playback time passed in during seek request
  int64 m_nPlaybackTime;

  //!Stores the track id to seek
  int64 m_nTrackIdToSeek;

  //! Type of seek requested
  FileSourceSeekType m_eSeekType;

  //! Selected TrackId for Audio track
  int m_audioSelectedTrackId;

  //! Selected TrackId for Video track
  int m_videoSelectedTrackId;

  //! Selected TrackId for Text track
  int m_textSelectedTrackId;

  //! Stores Track Information about all the valid Audio tracks.
  FileSourceAudioInfo m_audioTrackInfo[FILE_SOURCE_MAX_NUM_TRACKS];
  //! Stores Track Information about all the valid Video tracks.
  FileSourceVideoInfo m_videoTrackInfo[FILE_SOURCE_MAX_NUM_TRACKS];
  //! Stores Track Information about all the valid Text tracks.
  FileSourceTextInfo  m_textTrackInfo[FILE_SOURCE_MAX_NUM_TRACKS];

  AC3HeaderInfo    sAC3HdrInfo;
  //! Current position TimeStamp
  uint64 m_currentPosTimeStamp;

  //! Stop position TimeStamp
  uint64 m_stopPosTimeStamp;

  //! Use in case of http streaming to check periodically
  //! whether media handles are ready or not.
  bool m_bOpenPending;

  //! CloseFile API will set a signal for file source to process Close command.
  //! Close command will be handled in file source thread while caller will be
  //! blocked untill Close is processed.
  bool m_bClosePending;

  //!Set to true when filesource is able to start it's thread and initialize
  //!all the data members. By default, it's set to true in constructor and gets reset to false
  //!if some error occurs in constructor
  bool m_bEveryThingOK;

  //!When true, instructs individual parser to locate codec configuration header
  //for each track before reporting initial parsing done.
  //!When false, initial parsing is done as soon as tracks are detected..
  bool m_bLookForCodecHdr;

  //!When true, instructs file source to work in asynchronous mode.
  // It means FileSouce will create thread and accepts OPEN, CLOSE and SEEK
  // commands as events and FileSource will work on these commands internally.
  //!When false, instructs file source to work in synchronous mode.
  // In this mode, FileSource will not create any specific thread.
  // All the operations will be executed in client thread.
  bool m_bFSAsync;

private:

  //! Audio is played from the file pointed by m_audioFilename
  FILESOURCE_STRING m_audioFilename;
  //! Video is played from the file pointed by m_videoFilename
  FILESOURCE_STRING m_videoFilename;
  //! Text is played from the file pointed by m_textFilename
  FILESOURCE_STRING m_textFilename;

  //! Audio is played from the buffer pointed by m_pAudioBuffer
  unsigned char*    m_pAudioBuffer;
  //! Video is played from the buffer pointed by m_pVideoBuffer
  unsigned char*    m_pVideoBuffer;
  //! Text is played from the buffer pointed by m_pTextBuffer
  unsigned char*    m_pTextBuffer;

  //! Maximum Audio buffer size
  uint32            m_nAudioBufSize;
  //! Maximum Video buffer size
  uint32            m_nVideoBufSize;
  //! Maximum Text buffer size
  uint32            m_nTextBufSize;

  //! The signal Q for the file source working thread to wait on.
  MM_HANDLE m_pSignalQ;

  //! The signal Q to wait on close done in CloseFile API.
  MM_HANDLE m_pCloseDoneSignalQ;

  //! The signal associated with the open file command.
  MM_HANDLE m_pOpenFileSignal;

  //! The signal associated with the close file command.
  MM_HANDLE m_pCloseFileSignal;

  //! The signal when set indicates that CloseFile command is executed successfully.
  MM_HANDLE m_pCloseFileDoneSignal;

  //! The signal associated with the seek command.
  MM_HANDLE m_pSeekFileSignal;

  //! The thread exit signal. This signal is used to exit the file source working thread.
  MM_HANDLE m_pExitSignal;

  //! The file source working handle.
  MM_HANDLE m_pSourceThreadHandle;

  //!Critical section to be used
  MM_HANDLE m_pCriticalSection;

  FileSourceFileFormat m_hFileFormatRequested;

  //! The file source working thread entry function.
  static int SourceThreadEntry( void* ptr );

  //! The file source working thread.
  void SourceThread( void );

  //! The file source working thread priority.
  static const int SOURCE_THREAD_PRIORITY;

  //! The file source working thread stack size
  static const unsigned int SOURCE_THREAD_STACK_SIZE;

  /*!
  * Following event is set when user calls OpenFile.
  * The event is handled in FileSource Thread Context.
  */
  static const uint32 OPEN_FILE_EVENT;

  /*!
  * Following event is set when FileSource Thread exits.
  */
  static const uint32 THREAD_EXIT_EVENT;

  /*!
  * Following event is set when user calls CloseFile.
  * The event is handled in FileSource Thread Context.
  */
  static const uint32 CLOSE_FILE_EVENT;

  /*!
  * Following event is set when user calls SeekAbsolutePosition/SeekRelativeSyncPoint.
  * The event is handled in FileSource Thread Context.
  */
  static const uint32 SEEK_FILE_EVENT;

  /*!
  * Following event is set FileSource Thread is done processing CloseFile event.
  */
  static const uint32 CLOSE_FILE_DONE_EVENT;

  //! Copy the DRM contect information from the video track to Audio and text tracks
  FileSourceStatus  CopyDRMContextInfo();

  //! Points to the free index in m_trackIdInfo to store track information
  uint32 m_nTrackIdInfoIndexToUse;

  //!Deletes the respective media handle if audio/video/text does not exist
  void ValidateMediaHandles();

  //! checks if parser has encountered DRM related error.
  //!If yes, cleans up the track information so that OPEN_FAIL will be reported.
  void CheckDRMError();

  //!Returns the media handle based on the trackID input
  FileBase* GetMediaHandleForTrackID(uint32 id);
  //!Returns the critical section pointer based on the trackID input
  void*     GetCriticalSectionPtrForTrackID(uint32 ulTrackID);
  /* ==========================================================================
    @brief      Function to seek based on timestamp.

    @details    This function is used to do Seek operation on timestamp value.
                For Video track, Seek will be done to Sync sample if requested.
                For non-Video tracks, Seek will be done to non-Sync Samples.

    @param[in]      pFileBase         FileBase Handle.
    @param[in]      ulSelectedTrackId Track ID selected.
    @param[in]      slTrackIDtoSeek   Track ID on which Seek is requested.
    @param[in/out]  pbError           Error Flag Pointer.
    @param[in]      bSeektoSync       Flag to indicate whether sync sample based seek
                                      requested or not.
    @param[in/out]  ullReposTime      Reposition timestamp.
    @param[in]      pCriticalSection  Critical Section for handle.

    @return     Parser_ErrorNone if Seek is successful,
                else corresponding error.
    @note       None.
  ========================================================================== */
  PARSER_ERRORTYPE SeekToTime(FileBase* pFileBase, int ulSelectedTrackId,
                              int64 slTrackIDtoSeek, bool* pbError,
                              bool bSeektoSync, uint64& ullReposTime,
                              void* pCriticalSection);
  /* ==========================================================================
  @brief      Function to seek based on timestamp.

  @details    This function is used to do Seek operation on timestamp value.
              For Video track, Seek will be done to Sync sample if requested.
              For non-Video tracks, Seek will be done to non-Sync Samples.

  @param[in]      ullReposTime  Reposition Seek time.

  @return     Parser_ErrorNone if Seek is successful,
              else corresponding error.
  @note       None.
  ========================================================================== */
  PARSER_ERRORTYPE SeekAbsoluteAPI(uint64& ullReposTime);
  /* ==========================================================================
  @brief      Function to seek based on Sync Sample.

  @details    This function is used to do Seek operation on Sync Sample count.
              For Video track, Seek will be done to Sync sample if requested.
              For non-Video tracks, Seek will be done to non-Sync Samples.

  @param[in]      ullReposTime  Reposition Seek time.

  @return     Parser_ErrorNone if Seek is successful,
              else corresponding error.
  @note       None.
  ========================================================================== */
  PARSER_ERRORTYPE SeekRelativeAPI(uint64& ullReposTime);
  /* ==========================================================================
  @brief      Function to return Seek status to FS client.

  @details    This function is used to return status through registered
              callback function.

  @param[in]      eRetStatus    Parser seek status.
  @param[in]      ullReposTime  Reposition Seek time.

  @return     Function will return Seek Callback status.

  @note       None.
  ========================================================================== */
  void             ReturnSeekStatus(PARSER_ERRORTYPE eRetStatus,
                                    uint64 ullReposTime);
  FileSourceState m_eState;//Internal state. Used to check if we are in good state before processing any command/API.

  //!Critical section to be used while retrieving/seeking audio/video/text tracks.
  MM_HANDLE m_pAudioCriticalSection;
  MM_HANDLE m_pVideoCriticalSection;
  MM_HANDLE m_pTextCriticalSection;

  audio_data_buffer* m_pAudioDataBuffer;
};


#endif /* __FileSource_Helper_H__  */

